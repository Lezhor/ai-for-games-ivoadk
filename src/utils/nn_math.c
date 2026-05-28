#include "nn_math.h"
#include <math.h>
#include <float.h>

void nn_dense_forward(const float* restrict input, 
                      const float* restrict weights, 
                      const float* restrict biases, 
                      float* restrict output, 
                      int in_dim, int out_dim) {
    for (int i = 0; i < out_dim; i++) {
        float sum = biases[i];
        const float* restrict w_row = &weights[i * in_dim];
        
        // This inner loop is the hot path. 
        // -O3 -march=native will turn this into SIMD instructions.
        for (int j = 0; j < in_dim; j++) {
            sum += input[j] * w_row[j];
        }
        output[i] = sum;
    }
}

void nn_relu(float* data, int size) {
    for (int i = 0; i < size; i++) {
        if (data[i] < 0.0f) {
            data[i] = 0.0f;
        }
    }
}

void nn_softmax(float* data, int size) {
    float max_val = -FLT_MAX;
    for (int i = 0; i < size; i++) {
        if (data[i] > max_val) max_val = data[i];
    }

    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        data[i] = expf(data[i] - max_val);
        sum += data[i];
    }

    float inv_sum = 1.0f / sum;
    for (int i = 0; i < size; i++) {
        data[i] *= inv_sum;
    }
}

void nn_sigmoid(float* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = 1.0f / (1.0f + expf(-data[i]));
    }
}
