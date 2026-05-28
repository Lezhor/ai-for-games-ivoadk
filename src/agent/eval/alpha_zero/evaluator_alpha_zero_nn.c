#include "evaluator_alpha_zero.h"
#include "nn_features.h"
#include "utils/nn_math.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int in_dim;
    int out_dim;
    float* weights;
    float* biases;
} Layer;

typedef struct {
    int num_hidden_layers;
    Layer* hidden_layers;
    Layer policy_head;
    Layer value_head;
    
    float* all_weights_buffer; // Single allocation for all weights/biases
    
    // Ping-pong buffers for hidden layer activations
    float* buffer1;
    float* buffer2;
    int max_hidden_dim;
} NNModel;

static void az_nn_evaluate(AlphaZeroEvaluator* self, const GameSettings* settings, const GameState* state, AlphaZeroEvaluation* out_eval) {
    NNModel* model = (NNModel*)self->state;

    float input[FEATURE_COUNT];
    extract_features(settings, state, input);

    float* current_in = input;
    float* current_out = model->buffer1;

    // 1. Hidden Layers
    int in_dim = FEATURE_COUNT;
    for (int i = 0; i < model->num_hidden_layers; i++) {
        Layer* l = &model->hidden_layers[i];
        nn_dense_forward(current_in, l->weights, l->biases, current_out, in_dim, l->out_dim);
        nn_relu(current_out, l->out_dim);
        
        in_dim = l->out_dim;
        current_in = current_out;
        current_out = (current_in == model->buffer1) ? model->buffer2 : model->buffer1;
    }

    // 2. Policy Head
    float policy_out[20];
    nn_dense_forward(current_in, model->policy_head.weights, model->policy_head.biases, policy_out, in_dim, model->policy_head.out_dim);
    nn_softmax(policy_out, model->policy_head.out_dim);
    for (int i = 0; i < 20; i++) out_eval->policy[i] = (double)policy_out[i];

    // 3. Value Head
    float value_out[3];
    nn_dense_forward(current_in, model->value_head.weights, model->value_head.biases, value_out, in_dim, model->value_head.out_dim);
    nn_sigmoid(value_out, model->value_head.out_dim);
    for (int i = 0; i < 3; i++) out_eval->value[i] = (double)value_out[i];
}

static void az_nn_free(AlphaZeroEvaluator* self) {
    if (!self) return;
    NNModel* model = (NNModel*)self->state;
    if (model) {
        if (model->hidden_layers) free(model->hidden_layers);
        if (model->all_weights_buffer) free(model->all_weights_buffer);
        if (model->buffer1) free(model->buffer1);
        if (model->buffer2) free(model->buffer2);
        free(model);
    }
    free(self);
}

static int az_nn_load(AlphaZeroEvaluator* self, const char* filepath) {
    (void)self; (void)filepath;
    // Loading is handled in creator for this implementation
    return 0;
}

static int az_nn_save(AlphaZeroEvaluator* self, const char* filepath) {
    (void)self; (void)filepath;
    return 0;
}

AlphaZeroEvaluator* evaluator_create_alpha_zero_nn(const char* model_path) {
    FILE* f = fopen(model_path, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open model file: %s\n", model_path);
        return NULL;
    }

    NNModel* model = calloc(1, sizeof(NNModel));
    
    // Read Header
    if (fread(&model->num_hidden_layers, sizeof(int), 1, f) != 1) goto error;
    
    int* dims = malloc(sizeof(int) * (size_t)(model->num_hidden_layers + 1));
    if (fread(dims, sizeof(int), (size_t)(model->num_hidden_layers + 1), f) != (size_t)(model->num_hidden_layers + 1)) {
        free(dims);
        goto error;
    }

    if (fread(&model->policy_head.out_dim, sizeof(int), 1, f) != 1) {
        free(dims);
        goto error;
    }
    if (fread(&model->value_head.out_dim, sizeof(int), 1, f) != 1) {
        free(dims);
        goto error;
    }

    model->hidden_layers = malloc(sizeof(Layer) * (size_t)model->num_hidden_layers);
    
    // Calculate total weights size
    size_t total_floats = 0;
    model->max_hidden_dim = 0;
    for (int i = 0; i < model->num_hidden_layers; i++) {
        total_floats += (size_t)dims[i] * (size_t)dims[i+1]; // weights
        total_floats += (size_t)dims[i+1]; // biases
        if (dims[i+1] > model->max_hidden_dim) model->max_hidden_dim = dims[i+1];
    }
    // Policy head
    total_floats += (size_t)dims[model->num_hidden_layers] * (size_t)model->policy_head.out_dim;
    total_floats += (size_t)model->policy_head.out_dim;
    // Value head
    total_floats += (size_t)dims[model->num_hidden_layers] * (size_t)model->value_head.out_dim;
    total_floats += (size_t)model->value_head.out_dim;

    model->all_weights_buffer = malloc(sizeof(float) * total_floats);
    if (fread(model->all_weights_buffer, sizeof(float), total_floats, f) != total_floats) {
        free(dims);
        goto error;
    }
    fclose(f);

    // Slice pointers
    float* cursor = model->all_weights_buffer;
    for (int i = 0; i < model->num_hidden_layers; i++) {
        model->hidden_layers[i].in_dim = dims[i];
        model->hidden_layers[i].out_dim = dims[i+1];
        model->hidden_layers[i].weights = cursor;
        cursor += (size_t)dims[i] * (size_t)dims[i+1];
        model->hidden_layers[i].biases = cursor;
        cursor += (size_t)dims[i+1];
    }

    model->policy_head.in_dim = dims[model->num_hidden_layers];
    model->policy_head.weights = cursor;
    cursor += (size_t)model->policy_head.in_dim * (size_t)model->policy_head.out_dim;
    model->policy_head.biases = cursor;
    cursor += (size_t)model->policy_head.out_dim;

    model->value_head.in_dim = dims[model->num_hidden_layers];
    model->value_head.weights = cursor;
    cursor += (size_t)model->value_head.in_dim * (size_t)model->value_head.out_dim;
    model->value_head.biases = cursor;

    model->buffer1 = malloc(sizeof(float) * (size_t)model->max_hidden_dim);
    model->buffer2 = malloc(sizeof(float) * (size_t)model->max_hidden_dim);

    free(dims);

    AlphaZeroEvaluator* eval = malloc(sizeof(AlphaZeroEvaluator));
    eval->state = model;
    eval->evaluate = az_nn_evaluate;
    eval->free = az_nn_free;
    eval->load = az_nn_load;
    eval->save = az_nn_save;

    return eval;

error:
    if (f) fclose(f);
    if (model) {
        if (model->hidden_layers) free(model->hidden_layers);
        if (model->all_weights_buffer) free(model->all_weights_buffer);
        free(model);
    }
    return NULL;
}
