#ifndef NN_MATH_H
#define NN_MATH_H

/**
 * Generic neural network math utilities optimized for auto-vectorization.
 */

/**
 * Performs a dense (fully connected) layer forward pass: output = activation(input * weights + biases)
 * Note: weights are expected in shape [out_dim, in_dim] for better cache locality in the inner loop.
 */
void nn_dense_forward(const float* restrict input, 
                      const float* restrict weights, 
                      const float* restrict biases, 
                      float* restrict output, 
                      int in_dim, int out_dim);

/**
 * Rectified Linear Unit: x = max(0, x)
 */
void nn_relu(float* data, int size);

/**
 * Softmax activation: e^x_i / sum(e^x_j)
 */
void nn_softmax(float* data, int size);

/**
 * Sigmoid activation: 1 / (1 + e^-x)
 */
void nn_sigmoid(float* data, int size);

#endif // NN_MATH_H
