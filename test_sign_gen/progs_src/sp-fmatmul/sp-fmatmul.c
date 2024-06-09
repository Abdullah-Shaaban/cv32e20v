// Copyright 2023 ETH Zurich and University of Bologna.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: Matheus Cavalcante, ETH Zurich

#include "data_64_64_64.h"
#include "sp-fmatmul.h"
#include <stddef.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void matmul(float *c, const float *a, const float *b, const unsigned int M,
            const unsigned int N, const unsigned int P) {
  if (M <= 4) {
    matmul_2xVL(c, a, b, 0, M, N, P, 0, P);
  } else if (M <= 8) {
    matmul_4xVL(c, a, b, 0, M, N, P, 0, P);
  } else {
    matmul_8xVL(c, a, b, 0, M, N, P, 0, P);
  }
}

// ---------------
// 2xVL
// ---------------

void matmul_2xVL(float *c, const float *a, const float *b,
                 const unsigned int m_start, const unsigned int m_end,
                 const unsigned int N, const unsigned int P,
                 const unsigned int p_start, const unsigned int p_end) {

  unsigned int p = p_start;
  while (p < p_end) {
    // Calculate the vl
    size_t gvl;
    asm volatile("vsetvli %[gvl], %[vl], e32, m8, ta, ma"
                 : [gvl] "=r"(gvl)
                 : [vl] "r"(p_end - p));

    const float *b_ = b + p;
    float *c_ = c + p;

    for (unsigned int m = m_start; m < m_end; m += 2) {
      const float *a_ = a + m * N;
      const float *a__ = a_;

      asm volatile("vle32.v v16, (%0);" ::"r"(b_));
      const float *b__ = b_ + P;

      float *c__ = c_ + m * P;

      float t0, t1;

      t0 = *a__;
      a__ += N;
      t1 = *a__;

      unsigned int n = 0;

      while (n < N) {
        a__ = a_ + ++n;

        asm volatile("vle32.v v24, (%0);" ::"r"(b__));
        b__ += P;

        if (n == 1) {
          asm volatile("vfmul.vf v0, v16, %0" ::"f"(t0));
          t0 = *a__;
          a__ += N;
          asm volatile("vfmul.vf v8, v16, %0" ::"f"(t1));
          t1 = *a__;
        } else {
          asm volatile("vfmacc.vf v0, %0, v16" ::"f"(t0));
          t0 = *a__;
          a__ += N;
          asm volatile("vfmacc.vf v8, %0, v16" ::"f"(t1));
          t1 = *a__;
        }

        a__ = a_ + ++n;

        if (n == N)
          break;

        asm volatile("vle32.v v16, (%0);" ::"r"(b__));
        b__ += P;

        asm volatile("vfmacc.vf v0, %0, v24" ::"f"(t0));
        t0 = *a__;
        a__ += N;
        asm volatile("vfmacc.vf v8, %0, v24" ::"f"(t1));
        t1 = *a__;
      }

      asm volatile("vfmacc.vf v0, %0, v24" ::"f"(t0));
      asm volatile("vse32.v v0, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vfmacc.vf v8, %0, v24" ::"f"(t1));
      asm volatile("vse32.v v8, (%0);" ::"r"(c__));
    }

    p += gvl;
  }
}

// ---------------
// 4xVL
// ---------------

void matmul_4xVL(float *c, const float *a, const float *b,
                 const unsigned int m_start, const unsigned int m_end,
                 const unsigned int N, const unsigned int P,
                 const unsigned int p_start, const unsigned int p_end) {

  unsigned int p = p_start;
  while (p < p_end) {
    
    // Find pointer to the submatrices
    const float *b_ = b + p;
    float *c_ = c + p;

    // Calculate the vl
    size_t gvl;
    asm volatile("vsetvli %[gvl], %[vl], e32, m4, ta, ma" : [gvl] "=r"(gvl) : [vl] "r"(p_end - p));

    // Iterate over the rows
    for (unsigned int m = m_start; m < m_end; m += 4) {
      // Find pointer to the submatrices
      const float *a_ = a + m * N;
      float *c__ = c_ + m * P;
      
      // Temporary variables
      float t0, t1, t2, t3;
      
      // Original pointer
      const float *a__ = a_;

      // Prefetch one row of matrix B
      asm volatile("vle32.v v16, (%0);" ::"r"(b_));
      const float *b__ = b_ + P;

      // Prefetch one row of scalar values
      t0 = *a__;
      a__ += N;
      t1 = *a__;
      a__ += N;
      t2 = *a__;
      a__ += N;
      t3 = *a__;

      unsigned int n = 0;

      while (n < N) {
        
        // Calculate pointer to the matrix A
        a__ = a_ + ++n;
        
        // Load one row of B
        asm volatile("vle32.v v20, (%0);" ::"r"(b__));
        b__ += P;
    
        if (n == 1) {
          asm volatile("vfmul.vf v0, v16, %0" ::"f"(t0));
          t0 = *a__;
          a__ += N;
          asm volatile("vfmul.vf v4, v16, %0" ::"f"(t1));
          t1 = *a__;
          a__ += N;
          asm volatile("vfmul.vf v8, v16, %0" ::"f"(t2));
          t2 = *a__;
          a__ += N;
          asm volatile("vfmul.vf v12, v16, %0" ::"f"(t3));
          t3 = *a__;
        } else {
          asm volatile("vfmacc.vf v0, %0, v16" ::"f"(t0));
          t0 = *a__;
          a__ += N;
          asm volatile("vfmacc.vf v4, %0, v16" ::"f"(t1));
          t1 = *a__;
          a__ += N;
          asm volatile("vfmacc.vf v8, %0, v16" ::"f"(t2));
          t2 = *a__;
          a__ += N;
          asm volatile("vfmacc.vf v12, %0, v16" ::"f"(t3));
          t3 = *a__;
        }

        a__ = a_ + ++n;

        if (n == N)
          break;

        // Load one row of B
        asm volatile("vle32.v v16, (%0);" ::"r"(b__));
        b__ += P;

        asm volatile("vfmacc.vf v0, %0, v20" ::"f"(t0));
        t0 = *a__;
        a__ += N;
        asm volatile("vfmacc.vf v4, %0, v20" ::"f"(t1));
        t1 = *a__;
        a__ += N;
        asm volatile("vfmacc.vf v8, %0, v20" ::"f"(t2));
        t2 = *a__;
        a__ += N;
        asm volatile("vfmacc.vf v12, %0, v20" ::"f"(t3));
        t3 = *a__;
      }

      // Last iteration: store results
      asm volatile("vfmacc.vf v0, %0, v20" ::"f"(t0));
      asm volatile("vse32.v v0, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vfmacc.vf v4, %0, v20" ::"f"(t1));
      asm volatile("vse32.v v4, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vfmacc.vf v8, %0, v20" ::"f"(t2));
      asm volatile("vse32.v v8, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vfmacc.vf v12, %0, v20" ::"f"(t3));
      asm volatile("vse32.v v12, (%0);" ::"r"(c__));
    }

    p += gvl;
  }
}

// ---------------
// 8xVL
// ---------------

void matmul_8xVL(float *c, const float *a, const float *b,
                 const unsigned int m_start, const unsigned int m_end,
                 const unsigned int N, const unsigned int P,
                 const unsigned int p_start, const unsigned int p_end) {

  unsigned int p = p_start;
  while (p < p_end) {
    // Calculate the vl
    size_t gvl;
    asm volatile("vsetvli %[gvl], %[vl], e32, m2, ta, ma"
                 : [gvl] "=r"(gvl)
                 : [vl] "r"(p_end - p));

    const float *b_ = b + p;
    float *c_ = c + p;

    for (unsigned int m = m_start; m < m_end; m += 8) {
      const float *a_ = a + m * N;
      const float *a__ = a_;

      asm volatile("vle32.v v18, (%0);" ::"r"(b_));
      const float *b__ = b_ + P;

      float *c__ = c_ + m * P;

      float t0, t1, t2, t3, t4, t5, t6, t7;

      t0 = *a__;
      a__ += N;
      t1 = *a__;
      a__ += N;
      t2 = *a__;
      a__ += N;
      t3 = *a__;
      a__ += N;
      t4 = *a__;
      a__ += N;
      t5 = *a__;
      a__ += N;
      t6 = *a__;
      a__ += N;
      t7 = *a__;

      unsigned int n = 0;

      while (n < N) {
        a__ = a_ + ++n;

        asm volatile("vle32.v v20, (%0);" ::"r"(b__));
        b__ += P;

        if (n == 1) {
          asm volatile("vfmul.vf v0, v18, %0" ::"f"(t0));
          t0 = *a__;
          a__ += N;
          asm volatile("vfmul.vf v2, v18, %0" ::"f"(t1));
          t1 = *a__;
          a__ += N;
          asm volatile("vfmul.vf v4, v18, %0" ::"f"(t2));
          t2 = *a__;
          a__ += N;
          asm volatile("vfmul.vf v6, v18, %0" ::"f"(t3));
          t3 = *a__;
          a__ += N;
          asm volatile("vfmul.vf v8, v18, %0" ::"f"(t4));
          t4 = *a__;
          a__ += N;
          asm volatile("vfmul.vf v10, v18, %0" ::"f"(t5));
          t5 = *a__;
          a__ += N;
          asm volatile("vfmul.vf v12, v18, %0" ::"f"(t6));
          t6 = *a__;
          a__ += N;
          asm volatile("vfmul.vf v14, v18, %0" ::"f"(t7));
          t7 = *a__;
        } else {
          asm volatile("vfmacc.vf v0, %0, v18" ::"f"(t0));
          t0 = *a__;
          a__ += N;
          asm volatile("vfmacc.vf v2, %0, v18" ::"f"(t1));
          t1 = *a__;
          a__ += N;
          asm volatile("vfmacc.vf v4, %0, v18" ::"f"(t2));
          t2 = *a__;
          a__ += N;
          asm volatile("vfmacc.vf v6, %0, v18" ::"f"(t3));
          t3 = *a__;
          a__ += N;
          asm volatile("vfmacc.vf v8, %0, v18" ::"f"(t4));
          t4 = *a__;
          a__ += N;
          asm volatile("vfmacc.vf v10, %0, v18" ::"f"(t5));
          t5 = *a__;
          a__ += N;
          asm volatile("vfmacc.vf v12, %0, v18" ::"f"(t6));
          t6 = *a__;
          a__ += N;
          asm volatile("vfmacc.vf v14, %0, v18" ::"f"(t7));
          t7 = *a__;
        }

        a__ = a_ + ++n;

        if (n == N)
          break;

        asm volatile("vle32.v v18, (%0);" ::"r"(b__));
        b__ += P;

        asm volatile("vfmacc.vf v0, %0, v20" ::"f"(t0));
        t0 = *a__;
        a__ += N;
        asm volatile("vfmacc.vf v2, %0, v20" ::"f"(t1));
        t1 = *a__;
        a__ += N;
        asm volatile("vfmacc.vf v4, %0, v20" ::"f"(t2));
        t2 = *a__;
        a__ += N;
        asm volatile("vfmacc.vf v6, %0, v20" ::"f"(t3));
        t3 = *a__;
        a__ += N;
        asm volatile("vfmacc.vf v8, %0, v20" ::"f"(t4));
        t4 = *a__;
        a__ += N;
        asm volatile("vfmacc.vf v10, %0, v20" ::"f"(t5));
        t5 = *a__;
        a__ += N;
        asm volatile("vfmacc.vf v12, %0, v20" ::"f"(t6));
        t6 = *a__;
        a__ += N;
        asm volatile("vfmacc.vf v14, %0, v20" ::"f"(t7));
        t7 = *a__;
      }

      asm volatile("vfmacc.vf v0, %0, v20" ::"f"(t0));
      asm volatile("vse32.v v0, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vfmacc.vf v2, %0, v20" ::"f"(t1));
      asm volatile("vse32.v v2, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vfmacc.vf v4, %0, v20" ::"f"(t2));
      asm volatile("vse32.v v4, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vfmacc.vf v6, %0, v20" ::"f"(t3));
      asm volatile("vse32.v v6, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vfmacc.vf v8, %0, v20" ::"f"(t4));
      asm volatile("vse32.v v8, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vfmacc.vf v10, %0, v20" ::"f"(t5));
      asm volatile("vse32.v v10, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vfmacc.vf v12, %0, v20" ::"f"(t6));
      asm volatile("vse32.v v12, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vfmacc.vf v14, %0, v20" ::"f"(t7));
      asm volatile("vse32.v v14, (%0);" ::"r"(c__));
    }

    p += gvl;
  }
}


float *a;
float *b;
float c[64*64];

// Verify the matrices
int verify_matrix(float *matrix, const float *checksum,
                  const unsigned int num_rows, const unsigned int num_columns) {
  for (unsigned int i = 0; i < num_rows; ++i) {
    float sum = 0;
    for (unsigned int j = 0; j < num_columns; ++j) {
      sum += (float)matrix[i * num_columns + j];
    }

    float diff = sum - (float)checksum[i];
    if (diff < 0)
      diff = -diff;
    if (diff > 0.001) {
      return i == 0 ? -1 : (int)i;
    }
  }
  return 0;
}

void print_matrix(float const *matrix, uint32_t num_rows, uint32_t num_columns) {
  printf("0x%8X\n", (unsigned int)matrix);
  for (uint32_t i = 0; i < num_rows; ++i) {
    for (uint32_t j = 0; j < num_columns; ++j) {
      printf("%5f ", matrix[i * num_columns + j]);
    }
    printf("\n");
  }
}

int main() {
  const unsigned int num_cores = 1;
  const unsigned int cid = 0;

  unsigned int m_start, m_end;
  unsigned int p_start, p_end;
  unsigned int kernel_size;

  // Set matrix dimension
  kernel_size = 4;

  // Work over complete P dimension
  p_start = 0;
  p_end = gemm_l.N;
  m_start = 0;
  m_end = gemm_l.M;

  a = gemm_A_dram;
  b = gemm_B_dram;

  // Calculate matmul
  if (kernel_size == 2) {
    matmul_2xVL(c, a, b, m_start, m_end, gemm_l.K, gemm_l.N, p_start, p_end);
  } else if (kernel_size == 4) {
    matmul_4xVL(c, a, b, m_start, m_end, gemm_l.K, gemm_l.N, p_start, p_end);
  } else if (kernel_size == 8) {
    matmul_8xVL(c, a, b, m_start, m_end, gemm_l.K, gemm_l.N, p_start, p_end);
  } else {
    return -2;
  }
  
  int error = verify_matrix(c, (const float *)gemm_checksum, gemm_l.M, gemm_l.N);

  if (error != 0) {
    printf("\nError!!!\n");
  } else{
    printf("\nSuccess!!!\n");
  }

  // Output
  print_matrix(c, 10, 10);
  // Expected
  print_matrix(gemm_C_dram, 10, 10);

  return 0;
}
