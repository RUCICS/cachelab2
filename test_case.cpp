#include "cachelab.h"

void test_case(int m, int n, int p, void (*gemm_case)(ptr_reg, ptr_reg, ptr_reg, ptr_reg)) {
    int *rawA, *rawB, *rawC;
    int* initA = new int[m * n];
    int* initB = new int[n * p];
    int* ansC = new int[m * p];
    {
        auto [A, B, C, buffer] = init(m, n, p);
        rawA = A.ptr_;
        rawB = B.ptr_;
        rawC = C.ptr_;
        for (int i = 0; i < m * n; i++) {
            initA[i] = rawA[i];
        }
        for (int i = 0; i < n * p; i++) {
            initB[i] = rawB[i];
        }
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < p; j++) {
                ansC[i * p + j] = 0;
                for (int k = 0; k < n; k++) {
                    ansC[i * p + j] += rawA[i * n + k] * rawB[k * p + j];
                }
            }
        }
        gemm_case(std::move(A), std::move(B), std::move(C), std::move(buffer));
    }

    if (!is_same(ansC, rawC, m, p) || !is_same(initA, rawA, m, n) || !is_same(initB, rawB, n, p)) {
        throw std::runtime_error("Incorrect result");
    } else {
        std::cerr << "Pass using " << get_max_reg_count() << " regs" << std::endl;
    }

    print_log();
    destroy();
    delete[] initA;
    delete[] initB;
    delete[] ansC;
}

void case0() {
    test_case(case0_m,
              case0_n,
              case0_p,
              gemm_case0);
}

void case1() {
    test_case(case1_m,
              case1_n,
              case1_p,
              gemm_case1);
}

void case2() {
    test_case(case2_m,
              case2_n,
              case2_p,
              gemm_case2);
}

void case3() {
    test_case(case3_m,
              case3_n,
              case3_p,
              gemm_case3);
}

void case0_baseline() {
    test_case(case0_m,
              case0_n,
              case0_p,
              gemm_case0_baseline);
}

void case1_baseline() {
    test_case(case1_m,
              case1_n,
              case1_p,
              gemm_case1_baseline);
}

void case2_baseline() {
    test_case(case2_m,
              case2_n,
              case2_p,
              gemm_case2_baseline);
}

void case3_baseline() {
    test_case(case3_m,
              case3_n,
              case3_p,
              gemm_case3_baseline);
}