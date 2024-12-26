#include "matrix.h"

namespace {
std::vector<int*> ptrs;
}

std::tuple<ptr_reg, ptr_reg, ptr_reg, ptr_reg> init(int m, int n, int p) {
    int* mem_pool = new int[m * n + n * p + m * p + BUFFER_SIZE];
    int* rawA = mem_pool;
    int* rawB = rawA + m * n;
    int* rawC = rawB + n * p;
    int* buffer = rawC + m * p;
    ptr_reg::base = mem_pool;
    ptr_reg::base_offset = (int*)0x30000000;
    ptrs.push_back(mem_pool);
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            rawA[i * n + j] = i * n + j;
        }
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < p; j++) {
            rawB[i * p + j] = i * p + j;
        }
    }
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < p; j++) {
            rawC[i * p + j] = 0;
        }
    }
    ptr_reg A(rawA);
    ptr_reg B(rawB);
    ptr_reg C(rawC);
    ptr_reg D(buffer);
    return std::make_tuple(std::move(A), std::move(B), std::move(C), std::move(D));
}

bool correct_check(ptr_reg A, ptr_reg B, ptr_reg C, int m, int n, int p) {
    int* rawA = A.ptr_;
    int* rawB = B.ptr_;
    int* rawC = C.ptr_;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < p; j++) {
            int sum = 0;
            for (int k = 0; k < n; k++) {
                sum += rawA[i * n + k] * rawB[k * p + j];
            }
            if (sum != rawC[i * p + j]) {
                return false;
            }
        }
    }
    return true;
}

bool correct_check(int* rawA, int* rawB, int* rawC, int m, int n, int p) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < p; j++) {
            int sum = 0;
            for (int k = 0; k < n; k++) {
                sum += rawA[i * n + k] * rawB[k * p + j];
            }
            if (sum != rawC[i * p + j]) {
                return false;
            }
        }
    }
    return true;
}

bool is_same(int* ans, int* out, int m, int n){
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            if (ans[i * n + j] != out[i * n + j]) {
                return false;
            }
        }
    }
    return true;
}

void destroy() {
    for (int* ptr : ptrs) {
        delete[] ptr;
    }
}