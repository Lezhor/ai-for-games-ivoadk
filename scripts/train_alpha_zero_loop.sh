#!/bin/bash

# Configuration & Defaults
RESUME_EPOCH=-1
INSTANCES=4
TARGET_ROWS=10000
PAST_EPOCHS=3
EPOCH_0_ROW_MULTIPLIER=5
ENABLE_LOG=false

# MCTS Defaults
MCTS_ITERATIONS=800
TEMPERATURE=1.0
C_PUCT=1.414

# Training Defaults
TRAIN_EPOCHS=100  # Total Alpha Zero cycles
NN_EPOCHS=10      # Iterations in train.py
BATCH_SIZE=1024
LR=0.001

# Paths
DATA_DIR="data/train_alpha_zero"
MODEL_DIR="models/alpha_zero"
MCTS_BIN="./build/headless-release/train-alpha_zero"
TRAIN_SCRIPT="models/train/train.py"

# Help
usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  --resume-epoch N       Resume from epoch N (default: -1, bootstrap)"
    echo "  --instances N          Number of parallel MCTS processes (default: $INSTANCES)"
    echo "  --target-rows N        Target rows per epoch (default: $TARGET_ROWS)"
    echo "  --past-epochs N        Number of past epochs to include in training (default: $PAST_EPOCHS)"
    echo "  --mcts-iterations N    MCTS iterations per move (default: $MCTS_ITERATIONS)"
    echo "  --temperature F        MCTS temperature (default: $TEMPERATURE)"
    echo "  --c-puct F             MCTS C-PUCT (default: $C_PUCT)"
    echo "  --train-epochs N       Total Alpha Zero cycles to run (default: $TRAIN_EPOCHS)"
    echo "  --nn-epochs N          NN training iterations per cycle (default: $NN_EPOCHS)"
    echo "  --batch-size N         NN training batch size (default: $BATCH_SIZE)"
    echo "  --lr F                 NN training learning rate (default: $LR)"
    echo "  --log                  Enable writing instance logs to $DATA_DIR"
    exit 1
}

# Parse Arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --resume-epoch) RESUME_EPOCH="$2"; shift 2 ;;
        --instances) INSTANCES="$2"; shift 2 ;;
        --target-rows) TARGET_ROWS="$2"; shift 2 ;;
        --past-epochs) PAST_EPOCHS="$2"; shift 2 ;;
        --mcts-iterations) MCTS_ITERATIONS="$2"; shift 2 ;;
        --temperature) TEMPERATURE="$2"; shift 2 ;;
        --c-puct) C_PUCT="$2"; shift 2 ;;
        --train-epochs) TRAIN_EPOCHS="$2"; shift 2 ;;
        --nn-epochs) NN_EPOCHS="$2"; shift 2 ;;
        --batch-size) BATCH_SIZE="$2"; shift 2 ;;
        --lr) LR="$2"; shift 2 ;;
        --log) ENABLE_LOG=true; shift ;;
        -h|--help) usage ;;
        *) echo "Unknown option: $1"; usage ;;
    esac
done

mkdir -p "$DATA_DIR"
mkdir -p "$MODEL_DIR"

CURRENT_EPOCH=$((RESUME_EPOCH + 1))
END_EPOCH=$((CURRENT_EPOCH + TRAIN_EPOCHS))

while [ $CURRENT_EPOCH -lt $END_EPOCH ]; do
    EPOCH_STR=$(printf "%02d" $CURRENT_EPOCH)
    echo "=== Starting Epoch $EPOCH_STR ==="

    # Determine target rows for this epoch
    CALC_TARGET_ROWS=$TARGET_ROWS
    if [ $CURRENT_EPOCH -eq 0 ]; then
        CALC_TARGET_ROWS=$((TARGET_ROWS * EPOCH_0_ROW_MULTIPLIER))
    fi

    # Launch MCTS instances
    PIDS=()
    FILES=()
    for i in $(seq 0 $((INSTANCES - 1))); do
        INSTANCE_STR=$(printf "%02d" $i)
        OUTPUT_FILE="$DATA_DIR/epoch$EPOCH_STR.$INSTANCE_STR.csv"
        FILES+=("$OUTPUT_FILE")
        
        # Build command
        CMD=("$MCTS_BIN" "--training-data-output" "$OUTPUT_FILE" "--mcts-iterations" "$MCTS_ITERATIONS" "--temperature" "$TEMPERATURE" "--c-puct" "$C_PUCT" "--num-games" "-1")
        
        if [ $CURRENT_EPOCH -gt 0 ]; then
            PREV_EPOCH_STR=$(printf "%02d" $((CURRENT_EPOCH - 1)))
            CMD+=("--use-nn" "--model-path" "$MODEL_DIR/epoch$PREV_EPOCH_STR.bin")
        fi

        # Run in background
        LOG_TARGET="/dev/null"
        if [ "$ENABLE_LOG" = true ]; then
            LOG_TARGET="$DATA_DIR/instance_$INSTANCE_STR.log"
        fi
        
        "${CMD[@]}" > "$LOG_TARGET" 2>&1 &
        PIDS+=($!)
    done

    START_TIME=$(date +%s)
    
    # Progress Bar Monitoring
    while true; do
        TOTAL_ROWS=0
        ROWS_PER_INSTANCE=()
        
        for f in "${FILES[@]}"; do
            if [ -f "$f" ]; then
                R=$(wc -l < "$f" | tr -d ' ')
            else
                R=0
            fi
            ROWS_PER_INSTANCE+=($R)
            TOTAL_ROWS=$((TOTAL_ROWS + R))
        done

        # Print progress bars
        # Move cursor up for each instance + 1 for total
        for ((i=0; i<INSTANCES+1; i++)); do
            echo -ne "\033[K" # Clear line
        done
        # Return cursor
        echo -ne "\033[$((INSTANCES+1))A"

        for i in "${!ROWS_PER_INSTANCE[@]}"; do
            R=${ROWS_PER_INSTANCE[$i]}
            INSTANCE_TARGET=$((CALC_TARGET_ROWS / INSTANCES))
            PERCENT=$((R * 100 / INSTANCE_TARGET))
            if [ $PERCENT -gt 100 ]; then PERCENT=100; fi
            
            # Simple bar
            BAR_LEN=30
            FILLED=$((PERCENT * BAR_LEN / 100))
            BAR=$(printf "%${FILLED}s" | tr ' ' '#')
            EMPTY=$(printf "%$((BAR_LEN - FILLED))s" | tr ' ' '-')
            
            printf "Instance %02d: [%s%s] %d%% (%d rows)\n" $i "$BAR" "$EMPTY" $PERCENT $R
        done
        
        TOTAL_PERCENT=$((TOTAL_ROWS * 100 / CALC_TARGET_ROWS))
        if [ $TOTAL_PERCENT -gt 100 ]; then TOTAL_PERCENT=100; fi
        BAR_LEN=30
        FILLED=$((TOTAL_PERCENT * BAR_LEN / 100))
        BAR=$(printf "%${FILLED}s" | tr ' ' '=')
        EMPTY=$(printf "%$((BAR_LEN - FILLED))s" | tr ' ' '-')
        printf "Total:       [%s%s] %d%% (%d/%d rows)\n" "$BAR" "$EMPTY" $TOTAL_PERCENT $TOTAL_ROWS $CALC_TARGET_ROWS

        if [ $TOTAL_ROWS -ge $CALC_TARGET_ROWS ]; then
            break
        fi
        sleep 2
    done

    # Stop instances
    echo "Target reached. Stopping MCTS processes..."
    for pid in "${PIDS[@]}"; do
        kill "$pid" 2>/dev/null
    done
    wait "${PIDS[@]}" 2>/dev/null

    # Cleanup CSV files (remove last potentially partial line)
    # Using sed -i '' for macOS compatibility
    for f in "${FILES[@]}"; do
        if [ -f "$f" ]; then
            sed -i '' '$ d' "$f"
        fi
    done

    END_TIME=$(date +%s)
    DURATION=$(( (END_TIME - START_TIME) / 60 ))
    echo "$TOTAL_ROWS rows of data generated for epoch $EPOCH_STR in $DURATION minutes."

    # NN Training
    echo "Starting NN training for epoch $EPOCH_STR..."
    
    DATA_PATHS=()
    for ((e=0; e<=CURRENT_EPOCH; e++)); do
        if [ $e -ge $((CURRENT_EPOCH - PAST_EPOCHS)) ]; then
            E_STR=$(printf "%02d" $e)
            # Match various naming conventions: epoch1.csv, epoch01.csv, epoch01.00.csv, etc.
            for pattern in "epoch$e.csv" "epoch$E_STR.csv" "epoch$e.*.csv" "epoch$E_STR.*.csv"; do
                # Use a temporary array to expand the glob safely
                MATCHES=($DATA_DIR/$pattern)
                for f in "${MATCHES[@]}"; do
                    if [ -f "$f" ]; then
                        DATA_PATHS+=("$f")
                    fi
                done
            done
        fi
    done

    if [ ${#DATA_PATHS[@]} -eq 0 ]; then
        echo "No data files found for epoch $EPOCH_STR. Exiting."
        exit 1
    fi

    # Use the python executable from the .env virtual environment if it exists, else fallback to python3
    PYTHON_EXEC="./.env/bin/python3"
    if [ ! -f "$PYTHON_EXEC" ]; then
        PYTHON_EXEC="python3"
    fi

    TRAIN_CMD=($PYTHON_EXEC "$TRAIN_SCRIPT" \
        --data_paths "${DATA_PATHS[@]}" \
        --output_model "$MODEL_DIR/epoch$EPOCH_STR.bin" \
        --epochs "$NN_EPOCHS" \
        --batch_size "$BATCH_SIZE" \
        --lr "$LR")

    if [ $CURRENT_EPOCH -gt 0 ]; then
        PREV_EPOCH_STR=$(printf "%02d" $((CURRENT_EPOCH - 1)))
        TRAIN_CMD+=("--input_model" "$MODEL_DIR/epoch$PREV_EPOCH_STR.bin")
    fi

    "${TRAIN_CMD[@]}"

    if [ $? -ne 0 ]; then
        echo "Training failed for epoch $EPOCH_STR. Exiting."
        exit 1
    fi

    echo "Finished epoch $EPOCH_STR. Model saved to $MODEL_DIR/epoch$EPOCH_STR.bin"
    
    CURRENT_EPOCH=$((CURRENT_EPOCH + 1))
done
