import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader
import numpy as np
import argparse
import os
import struct
import glob

class GameDataset(Dataset):
    def __init__(self, data_paths):
        all_files = []
        for path in data_paths:
            if os.path.isdir(path):
                all_files.extend(glob.glob(os.path.join(path, "*.csv")))
            else:
                all_files.append(path)
        
        if not all_files:
            raise ValueError(f"No CSV files found in paths: {data_paths}")

        data_list = []
        for f in all_files:
            print(f"Loading {f}...")
            # Fast loading for large CSVs
            try:
                d = np.loadtxt(f, delimiter=',').astype(np.float32)
                if len(d.shape) == 1: 
                    d = d.reshape(1, -1)
                data_list.append(d)
            except Exception as e:
                print(f"Warning: Could not load {f}: {e}")
        
        if not data_list:
            raise ValueError("No valid data could be loaded.")
            
        self.data = np.vstack(data_list)
        print(f"Total samples loaded: {len(self.data)}")

    def __len__(self):
        return len(self.data)

    def __getitem__(self, idx):
        row = self.data[idx]
        features = row[:82]
        policy = row[82:102]
        value = row[102:]
        return features, policy, value

class AlphaZeroMLP(nn.Module):
    def __init__(self, input_dim=82, hidden_dims=[256, 256, 128], policy_dim=20, value_dim=3):
        super(AlphaZeroMLP, self).__init__()
        
        self.hidden_layers = nn.ModuleList()
        current_dim = input_dim
        for h_dim in hidden_dims:
            self.hidden_layers.append(nn.Linear(current_dim, h_dim))
            current_dim = h_dim
        
        self.policy_head = nn.Linear(current_dim, policy_dim)
        self.value_head = nn.Linear(current_dim, value_dim)

    def forward(self, x):
        for layer in self.hidden_layers:
            x = F.relu(layer(x))
        
        policy = F.softmax(self.policy_head(x), dim=1)
        # Value corresponds to normalized tourney points [0, 1]
        value = torch.sigmoid(self.value_head(x))
        return policy, value

def train(args):
    dataset = GameDataset(args.data_paths)
    dataloader = DataLoader(dataset, batch_size=args.batch_size, shuffle=True)

    model = AlphaZeroMLP()
    
    # Device selection
    if torch.cuda.is_available():
        model = model.cuda()
    elif torch.backends.mps.is_available():
        model = model.to("mps")

    optimizer = optim.Adam(model.parameters(), lr=args.lr)
    
    # Policy head uses KL Divergence between MCTS distribution and NN distribution
    criterion_policy = nn.KLDivLoss(reduction='batchmean')
    # Value head uses MSE against final game outcomes
    criterion_value = nn.MSELoss()

    model.train()
    for epoch in range(args.epochs):
        total_loss = 0
        for features, target_policy, target_value in dataloader:
            if torch.cuda.is_available():
                features, target_policy, target_value = features.cuda(), target_policy.cuda(), target_value.cuda()
            elif torch.backends.mps.is_available():
                device = torch.device("mps")
                features, target_policy, target_value = features.to(device), target_policy.to(device), target_value.to(device)

            optimizer.zero_grad()
            
            pred_policy, pred_value = model(features)
            
            loss_policy = criterion_policy(torch.log(pred_policy + 1e-8), target_policy)
            loss_value = criterion_value(pred_value, target_value)
            
            loss = loss_policy + loss_value
            loss.backward()
            optimizer.step()
            
            total_loss += loss.item()
        
        print(f"Epoch {epoch+1}/{args.epochs}, Loss: {total_loss/len(dataloader):.6f}")

    export_to_bin(model.to("cpu"), args.output_model)

def export_to_bin(model, output_path):
    model.eval()
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with torch.no_grad():
        with open(output_path, 'wb') as f:
            # Header
            num_hidden = len(model.hidden_layers)
            f.write(struct.pack('I', num_hidden))
            
            dims = [model.hidden_layers[0].in_features]
            for layer in model.hidden_layers:
                dims.append(layer.out_features)
            
            for d in dims:
                f.write(struct.pack('I', d))
            
            f.write(struct.pack('I', model.policy_head.out_features))
            f.write(struct.pack('I', model.value_head.out_features))
            
            # Weights & Biases
            for layer in model.hidden_layers:
                f.write(layer.weight.data.numpy().astype(np.float32).tobytes())
                f.write(layer.bias.data.numpy().astype(np.float32).tobytes())
            
            f.write(model.policy_head.weight.data.numpy().astype(np.float32).tobytes())
            f.write(model.policy_head.bias.data.numpy().astype(np.float32).tobytes())
            
            f.write(model.value_head.weight.data.numpy().astype(np.float32).tobytes())
            f.write(model.value_head.bias.data.numpy().astype(np.float32).tobytes())

    print(f"Model exported to {output_path}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--data_paths', nargs='+', required=True, help='Paths to CSV files or directories')
    parser.add_argument('--output_model', default='models/alpha_zero/model.bin', help='Output binary path')
    parser.add_argument('--epochs', type=int, default=10)
    parser.add_argument('--batch_size', type=int, default=1024)
    parser.add_argument('--lr', type=float, default=0.001)
    
    args = parser.parse_args()
    train(args)
