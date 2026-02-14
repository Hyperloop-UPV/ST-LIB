#pragma once
#include "../ControlBlock.hpp"

template <size_t U, size_t V, size_t W> class MatrixMultiplier;

template <size_t U, size_t V, size_t W>
    requires(U > 1 && V > 1 && W > 1)
class MatrixMultiplier<U, V, W> : public ControlBlock<float (*)[U][V], float (*)[U][W]> {
    float (&A)[U][V], (&B)[V][W], (&result)[U][W];

public:
    MatrixMultiplier(float (&A)[U][V], float (&B)[V][W], float (&result)[U][W])
        : A(A), B(B), result(result) {
        this->input_value = &A;
        this->output_value = &result;
    }

    void execute() override {
        for (size_t i = 0; i < U; i++) {
            for (size_t j = 0; j < W; j++) {
                result[i][j] = 0;
                for (size_t k = 0; k < V; k++)
                    result[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    void reset() {
        for (size_t i = 0; i < U; i++) {
            for (size_t j = 0; j < W; j++)
                result[i][j] = 0.0;
        }
    }
};

// CTAD
#if __cpp_deduction_guides >= 201606
template <size_t U, size_t V, size_t W>
    requires(U > 1 && V > 1 && W > 1)
MatrixMultiplier(float (&)[U][V], float (&)[V][W], float (&)[U][W]) -> MatrixMultiplier<U, V, W>;
#endif

template <size_t U, size_t V>
class MatrixMultiplier<U, V, 1> : public ControlBlock<float (*)[U][V], float (*)[U]> {
    float (&A)[U][V], (&B)[V], (&result)[U];

public:
    MatrixMultiplier(float (&A)[U][V], float (&B)[V], float (&result)[U])
        : A(A), B(B), result(result) {
        this->input_value = &A;
        this->output_value = &result;
    }

    void execute() override {
        for (size_t i = 0; i < U; i++) {
            result[i] = 0;
            for (size_t k = 0; k < V; k++)
                result[i] += A[i][k] * B[k];
        }
    }

    void reset() {
        for (size_t i = 0; i < U; i++) {
            result[i] = 0.0;
        }
    }
};

// CTAD
#if __cpp_deduction_guides >= 201606
template <size_t U, size_t V>
    requires(U > 1 && V > 1)
MatrixMultiplier(float (&)[U][V], float (&)[V], float (&)[U]) -> MatrixMultiplier<U, V, 1>;
#endif

template <size_t U, size_t V>
class MatrixMultiplier<1, U, V> : public ControlBlock<float (*)[U], float (*)[V]> {
    float (&A)[U], (&B)[U][V], (&result)[V];

public:
    MatrixMultiplier(float (&A)[U], float (&B)[U][V], float (&result)[V])
        : A(A), B(B), result(result) {
        this->input_value = &A;
        this->output_value = &result;
    }

    void execute() override {
        for (size_t j = 0; j < V; j++) {
            result[j] = 0;
            for (size_t k = 0; k < U; k++)
                result[j] += A[k] * B[k][j];
        }
    }

    void reset() {
        for (size_t i = 0; i < V; i++) {
            result[i] = 0.0;
        }
    }
};

// CTAD
#if __cpp_deduction_guides >= 201606
template <size_t U, size_t V>
    requires(U > 1 && V > 1)
MatrixMultiplier(float (&)[U], float (&)[U][V], float (&)[V]) -> MatrixMultiplier<1, U, V>;
#endif

template <size_t U> class MatrixMultiplier<1, U, 1> : public ControlBlock<float (*)[U], float(*)> {
    float (&A)[U], (&B)[U], &result;

public:
    MatrixMultiplier(float (&A)[U], float (&B)[U], float(&result)) : A(A), B(B), result(result) {
        this->input_value = &A;
        this->output_value = &result;
    }

    void execute() override {
        result = 0;
        for (size_t k = 0; k < U; k++)
            result += A[k] * B[k];
    }

    void reset() { result = 0; }
};

// CTAD
#if __cpp_deduction_guides >= 201606
template <size_t U>
    requires(U > 1)
MatrixMultiplier(float (&)[U], float (&)[U], float(&)) -> MatrixMultiplier<1, U, 1>;
#endif
