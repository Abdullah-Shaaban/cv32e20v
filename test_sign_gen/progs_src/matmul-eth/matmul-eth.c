// Copyright 2021 ETH Zurich and University of Bologna.
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

// Author: Domenic Wüthrich, ETH Zurich

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void matmul_single(int8_t *c, const int8_t *a, const int8_t *b, const unsigned int N, const unsigned int P, unsigned int vl) {
  // Set VL
  asm volatile("vsetvli zero, %0, e8, m2, ta, ma" ::"r"(vl));

  // Temporary variables
  int8_t t0, t1, t2, t3, t4, t5, t6, t7;
  int8_t *a_ = (int8_t *)a;
  int8_t *b_ = (int8_t *)b;
  int8_t *c_ = (int8_t *)c;

  int8_t *a__ = a_;

  // Compute the multiplication
  unsigned int n = 0;

  t0 = *a__, a__ += N;
  t1 = *a__, a__ += N;
  t2 = *a__, a__ += N;
  t3 = *a__, a__ += N;
  t4 = *a__, a__ += N;
  t5 = *a__, a__ += N;
  t6 = *a__, a__ += N;
  t7 = *a__;

  // Calculate pointer to the matrix A
  a__ = a_ + ++n;

  asm volatile("vle8.v v16, (%0);" ::"r"(b_));
  b_ += P;
  asm volatile("vmv.v.i v0,  0");
  asm volatile("vmacc.vx v0, %0, v16" ::"r"(t0));
  t0 = *a__, a__ += N;
  asm volatile("vmv.v.i v2,  0");
  asm volatile("vmacc.vx v2, %0, v16" ::"r"(t1));
  t1 = *a__, a__ += N;
  asm volatile("vmv.v.i v4,  0");
  asm volatile("vmacc.vx v4, %0, v16" ::"r"(t2));
  t2 = *a__, a__ += N;
  asm volatile("vmv.v.i v6,  0");
  asm volatile("vmacc.vx v6, %0, v16" ::"r"(t3));
  t3 = *a__, a__ += N;

  // Load one row of B
  asm volatile("vle8.v v18, (%0);" ::"r"(b_));
  b_ += P;

  asm volatile("vmv.v.i v8,  0");
  asm volatile("vmacc.vx v8, %0, v16" ::"r"(t4));
  t4 = *a__, a__ += N;
  asm volatile("vmv.v.i v10,  0");
  asm volatile("vmacc.vx v10, %0, v16" ::"r"(t5));
  t5 = *a__, a__ += N;
  asm volatile("vmv.v.i v12,  0");
  asm volatile("vmacc.vx v12, %0, v16" ::"r"(t6));
  t6 = *a__, a__ += N;
  asm volatile("vmv.v.i v14,  0");
  asm volatile("vmacc.vx v14, %0, v16" ::"r"(t7));
  t7 = *a__;

  // Calculate pointer to the matrix A
  a__ = a_ + ++n;

  while (n < N) {
    // Load one row of B
    asm volatile("vle8.v v16, (%0);" ::"r"(b_));
    b_ += P;

    asm volatile("vmacc.vx v0, %0, v18" ::"r"(t0));
    t0 = *a__, a__ += N;
    asm volatile("vmacc.vx v2, %0, v18" ::"r"(t1));
    t1 = *a__, a__ += N;
    asm volatile("vmacc.vx v4, %0, v18" ::"r"(t2));
    t2 = *a__, a__ += N;
    asm volatile("vmacc.vx v6, %0, v18" ::"r"(t3));
    t3 = *a__, a__ += N;
    asm volatile("vmacc.vx v8, %0, v18" ::"r"(t4));
    t4 = *a__, a__ += N;
    asm volatile("vmacc.vx v10, %0, v18" ::"r"(t5));
    t5 = *a__, a__ += N;
    asm volatile("vmacc.vx v12, %0, v18" ::"r"(t6));
    t6 = *a__, a__ += N;
    asm volatile("vmacc.vx v14, %0, v18" ::"r"(t7));
    t7 = *a__;

    // Calculate pointer to the matrix A
    a__ = a_ + ++n;

    // Load one row of B
    asm volatile("vle8.v v18, (%0);" ::"r"(b_));
    b_ += P;

    asm volatile("vmacc.vx v0, %0, v16" ::"r"(t0));
    t0 = *a__, a__ += N;
    asm volatile("vmacc.vx v2, %0, v16" ::"r"(t1));
    t1 = *a__, a__ += N;
    asm volatile("vmacc.vx v4, %0, v16" ::"r"(t2));
    t2 = *a__, a__ += N;
    asm volatile("vmacc.vx v6, %0, v16" ::"r"(t3));
    t3 = *a__, a__ += N;
    asm volatile("vmacc.vx v8, %0, v16" ::"r"(t4));
    t4 = *a__, a__ += N;
    asm volatile("vmacc.vx v10, %0, v16" ::"r"(t5));
    t5 = *a__, a__ += N;
    asm volatile("vmacc.vx v12, %0, v16" ::"r"(t6));
    t6 = *a__, a__ += N;
    asm volatile("vmacc.vx v14, %0, v16" ::"r"(t7));
    t7 = *a__;

    // Calculate pointer to the matrix A
    a__ = a_ + ++n;
  }

  asm volatile("vmacc.vx v0, %0, v18" ::"r"(t0));
  asm volatile("vse8.v v0, (%0);" ::"r"(c_));
  c_ += P;
  asm volatile("vmacc.vx v2, %0, v18" ::"r"(t1));
  asm volatile("vse8.v v2, (%0);" ::"r"(c_));
  c_ += P;
  asm volatile("vmacc.vx v4, %0, v18" ::"r"(t2));
  asm volatile("vse8.v v4, (%0);" ::"r"(c_));
  c_ += P;
  asm volatile("vmacc.vx v6, %0, v18" ::"r"(t3));
  asm volatile("vse8.v v6, (%0);" ::"r"(c_));
  c_ += P;
  asm volatile("vmacc.vx v8, %0, v18" ::"r"(t4));
  asm volatile("vse8.v v8, (%0);" ::"r"(c_));
  c_ += P;
  asm volatile("vmacc.vx v10, %0, v18" ::"r"(t5));
  asm volatile("vse8.v v10, (%0);" ::"r"(c_));
  c_ += P;
  asm volatile("vmacc.vx v12, %0, v18" ::"r"(t6));
  asm volatile("vse8.v v12, (%0);" ::"r"(c_));
  c_ += P;
  asm volatile("vmacc.vx v14, %0, v18" ::"r"(t7));
  asm volatile("vse8.v v14, (%0);" ::"r"(c_));
  c_ += P;
}

void matmul_2xVL(int8_t *c, const int8_t *a, const int8_t *b,
                 const unsigned int m_start, const unsigned int m_end,
                 const unsigned int N, const unsigned int P,
                 const unsigned int p_start, const unsigned int p_end) {

  unsigned int p = p_start;
  while (p < p_end) {
    // Calculate the vl
    size_t gvl;
    asm volatile("vsetvli %[gvl], %[vl], e8, m8, ta, ma"
                 : [gvl] "=r"(gvl)
                 : [vl] "r"(p_end - p));

    const int8_t *b_ = b + p;
    int8_t *c_ = c + p;

    for (unsigned int m = m_start; m < m_end; m += 2) {
      const int8_t *a_ = a + m * N;
      const int8_t *a__ = a_;

      asm volatile("vle8.v v16, (%0);" ::"r"(b_));
      const int8_t *b__ = b_ + P;

      int8_t *c__ = c_ + m * P;

      int8_t t0, t1;

      t0 = *a__;
      a__ += N;
      t1 = *a__;

      unsigned int n = 0;

      while (n < N) {
        a__ = a_ + ++n;

        asm volatile("vle8.v v24, (%0);" ::"r"(b__));
        b__ += P;

        if (n == 1) {
          asm volatile("vmul.vx v0, v16, %0" ::"r"(t0));
          t0 = *a__;
          a__ += N;
          asm volatile("vmul.vx v8, v16, %0" ::"r"(t1));
          t1 = *a__;
        } else {
          asm volatile("vmacc.vx v0, %0, v16" ::"r"(t0));
          t0 = *a__;
          a__ += N;
          asm volatile("vmacc.vx v8, %0, v16" ::"r"(t1));
          t1 = *a__;
        }

        a__ = a_ + ++n;

        if (n == N)
          break;

        asm volatile("vle8.v v16, (%0);" ::"r"(b__));
        b__ += P;

        asm volatile("vmacc.vx v0, %0, v24" ::"r"(t0));
        t0 = *a__;
        a__ += N;
        asm volatile("vmacc.vx v8, %0, v24" ::"r"(t1));
        t1 = *a__;
      }

      asm volatile("vmacc.vx v0, %0, v24" ::"r"(t0));
      asm volatile("vse8.v v0, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v8, %0, v24" ::"r"(t1));
      asm volatile("vse8.v v8, (%0);" ::"r"(c__));
    }

    p += gvl;
  }
}

void matmul_4xVL(int8_t *c, const int8_t *a, const int8_t *b,
                 const unsigned int m_start, const unsigned int m_end,
                 const unsigned int N, const unsigned int P,
                 const unsigned int p_start, const unsigned int p_end) {

  unsigned int p = p_start;

  // Set the vector configuration
  uint32_t block_size_p;
  asm volatile("vsetvli %0, %1, e8, m4, ta, ma" : "=r"(block_size_p) : "r"(P));

  while (p < p_end) {
    // Calculate the vl
    uint32_t gvl;
    const uint32_t p_ = MIN(P - p, block_size_p);
    asm volatile("vsetvli %[gvl], %[vl], e8, m4, ta, ma"
                 : [gvl] "=r"(gvl)
                 : [vl] "r"(p_));
                //  : [vl] "r"(p_end - p));

    const int8_t *b_ = b + p;
    int8_t *c_ = c + p;

    for (unsigned int m = m_start; m < m_end; m += 4) {

      // Slice init
      asm volatile("vmv.v.i v0,  0");
      asm volatile("vmv.v.i v4,  0");
      asm volatile("vmv.v.i v8,  0");
      asm volatile("vmv.v.i v12, 0");

      const int8_t *a_ = a + m * N;
      const int8_t *a__ = a_;

      asm volatile("vle8.v v16, (%0);" ::"r"(b_));
      const int8_t *b__ = b_ + P;

      int8_t *c__ = c_ + m * P;

      int8_t t0, t1, t2, t3;

      t0 = *a__;
      a__ += N;
      t1 = *a__;
      a__ += N;
      t2 = *a__;
      a__ += N;
      t3 = *a__;

      unsigned int n = 0;

      while (n < N) {
        asm volatile("vle8.v v20, (%0);" ::"r"(b__));
        b__ += P;

        a__ = a_ + ++n;

        // if (n == 1) {
        //   asm volatile("vmul.vx v0, v16, %0" ::"r"(t0));
        //   t0 = *a__;
        //   a__ += N;
        //   asm volatile("vmul.vx v4, v16, %0" ::"r"(t1));
        //   t1 = *a__;
        //   a__ += N;
        //   asm volatile("vmul.vx v8, v16, %0" ::"r"(t2));
        //   t2 = *a__;
        //   a__ += N;
        //   asm volatile("vmul.vx v12, v16, %0" ::"r"(t3));
        //   t3 = *a__;
        // } else {
          asm volatile("vmacc.vx v0, %0, v16" ::"r"(t0));
          t0 = *a__;
          a__ += N;
          asm volatile("vmacc.vx v4, %0, v16" ::"r"(t1));
          t1 = *a__;
          a__ += N;
          asm volatile("vmacc.vx v8, %0, v16" ::"r"(t2));
          t2 = *a__;
          a__ += N;
          asm volatile("vmacc.vx v12, %0, v16" ::"r"(t3));
          t3 = *a__;
        // }

        a__ = a_ + ++n;

        if (n == N)
          break;

        asm volatile("vle8.v v16, (%0);" ::"r"(b__));
        b__ += P;

        asm volatile("vmacc.vx v0, %0, v20" ::"r"(t0));
        t0 = *a__;
        a__ += N;
        asm volatile("vmacc.vx v4, %0, v20" ::"r"(t1));
        t1 = *a__;
        a__ += N;
        asm volatile("vmacc.vx v8, %0, v20" ::"r"(t2));
        t2 = *a__;
        a__ += N;
        asm volatile("vmacc.vx v12, %0, v20" ::"r"(t3));
        t3 = *a__;
      }

      asm volatile("vmacc.vx v0, %0, v20" ::"r"(t0));
      asm volatile("vse8.v v0, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v4, %0, v20" ::"r"(t1));
      asm volatile("vse8.v v4, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v8, %0, v20" ::"r"(t2));
      asm volatile("vse8.v v8, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v12, %0, v20" ::"r"(t3));
      asm volatile("vse8.v v12, (%0);" ::"r"(c__));
    }

    p += gvl;
  }
}

void matmul_8xVL(int8_t *c, const int8_t *a, const int8_t *b,
                 const unsigned int m_start, const unsigned int m_end,
                 const unsigned int N, const unsigned int P,
                 const unsigned int p_start, const unsigned int p_end) {

  unsigned int p = p_start;
  while (p < p_end) {
    // Calculate the vl
    size_t gvl;
    asm volatile("vsetvli %[gvl], %[vl], e8, m2, ta, ma"
                 : [gvl] "=r"(gvl)
                 : [vl] "r"(p_end - p));

    const int8_t *b_ = b + p;
    int8_t *c_ = c + p;

    for (unsigned int m = m_start; m < m_end; m += 8) {
      const int8_t *a_ = a + m * N;
      const int8_t *a__ = a_;

      asm volatile("vle8.v v18, (%0);" ::"r"(b_));
      const int8_t *b__ = b_ + P;

      int8_t *c__ = c_ + m * P;

      int8_t t0, t1, t2, t3, t4, t5, t6, t7;

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

        asm volatile("vle8.v v20, (%0);" ::"r"(b__));
        b__ += P;

        if (n == 1) {
          asm volatile("vmul.vx v0, v18, %0" ::"r"(t0));
          t0 = *a__;
          a__ += N;
          asm volatile("vmul.vx v2, v18, %0" ::"r"(t1));
          t1 = *a__;
          a__ += N;
          asm volatile("vmul.vx v4, v18, %0" ::"r"(t2));
          t2 = *a__;
          a__ += N;
          asm volatile("vmul.vx v6, v18, %0" ::"r"(t3));
          t3 = *a__;
          a__ += N;
          asm volatile("vmul.vx v8, v18, %0" ::"r"(t4));
          t4 = *a__;
          a__ += N;
          asm volatile("vmul.vx v10, v18, %0" ::"r"(t5));
          t5 = *a__;
          a__ += N;
          asm volatile("vmul.vx v12, v18, %0" ::"r"(t6));
          t6 = *a__;
          a__ += N;
          asm volatile("vmul.vx v14, v18, %0" ::"r"(t7));
          t7 = *a__;
        } else {
          asm volatile("vmacc.vx v0, %0, v18" ::"r"(t0));
          t0 = *a__;
          a__ += N;
          asm volatile("vmacc.vx v2, %0, v18" ::"r"(t1));
          t1 = *a__;
          a__ += N;
          asm volatile("vmacc.vx v4, %0, v18" ::"r"(t2));
          t2 = *a__;
          a__ += N;
          asm volatile("vmacc.vx v6, %0, v18" ::"r"(t3));
          t3 = *a__;
          a__ += N;
          asm volatile("vmacc.vx v8, %0, v18" ::"r"(t4));
          t4 = *a__;
          a__ += N;
          asm volatile("vmacc.vx v10, %0, v18" ::"r"(t5));
          t5 = *a__;
          a__ += N;
          asm volatile("vmacc.vx v12, %0, v18" ::"r"(t6));
          t6 = *a__;
          a__ += N;
          asm volatile("vmacc.vx v14, %0, v18" ::"r"(t7));
          t7 = *a__;
        }

        a__ = a_ + ++n;

        if (n == N)
          break;

        asm volatile("vle8.v v18, (%0);" ::"r"(b__));
        b__ += P;

        asm volatile("vmacc.vx v0, %0, v20" ::"r"(t0));
        t0 = *a__;
        a__ += N;
        asm volatile("vmacc.vx v2, %0, v20" ::"r"(t1));
        t1 = *a__;
        a__ += N;
        asm volatile("vmacc.vx v4, %0, v20" ::"r"(t2));
        t2 = *a__;
        a__ += N;
        asm volatile("vmacc.vx v6, %0, v20" ::"r"(t3));
        t3 = *a__;
        a__ += N;
        asm volatile("vmacc.vx v8, %0, v20" ::"r"(t4));
        t4 = *a__;
        a__ += N;
        asm volatile("vmacc.vx v10, %0, v20" ::"r"(t5));
        t5 = *a__;
        a__ += N;
        asm volatile("vmacc.vx v12, %0, v20" ::"r"(t6));
        t6 = *a__;
        a__ += N;
        asm volatile("vmacc.vx v14, %0, v20" ::"r"(t7));
        t7 = *a__;
      }

      asm volatile("vmacc.vx v0, %0, v20" ::"r"(t0));
      asm volatile("vse8.v v0, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v2, %0, v20" ::"r"(t1));
      asm volatile("vse8.v v2, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v4, %0, v20" ::"r"(t2));
      asm volatile("vse8.v v4, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v6, %0, v20" ::"r"(t3));
      asm volatile("vse8.v v6, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v8, %0, v20" ::"r"(t4));
      asm volatile("vse8.v v8, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v10, %0, v20" ::"r"(t5));
      asm volatile("vse8.v v10, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v12, %0, v20" ::"r"(t6));
      asm volatile("vse8.v v12, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v14, %0, v20" ::"r"(t7));
      asm volatile("vse8.v v14, (%0);" ::"r"(c__));
    }

    p += gvl;
  }
}

// Matrices defined in data.S
extern int AVL __attribute__((aligned(4))); // Dimensions of the matrices
extern int8_t matrix1[] __attribute__((aligned(4)));
extern int8_t matrix2[] __attribute__((aligned(4)));
extern int8_t golden_o[] __attribute__((aligned(4)));
extern int8_t output[] __attribute__((aligned(4)));

void print_matrix(int8_t const *matrix, uint32_t num_rows, uint32_t num_columns) {
  printf("0x%8X\n", (unsigned int)matrix);
  for (uint32_t i = 0; i < num_rows; ++i) {
    for (uint32_t j = 0; j < num_columns; ++j) {
      printf("%2X ", (uint8_t)matrix[i * num_columns + j]);
    }
    printf("\n");
  }
}

void matmul(int8_t *c, const int8_t *a, const int8_t *b,
            const unsigned int M, const unsigned int N, const unsigned int P) {
  if (M <= 4) {
    matmul_2xVL(c, a, b, 0, M, N, P, 0, P);
  } else if (M <= 8) {
    matmul_4xVL(c, a, b, 0, M, N, P, 0, P);
  } else {
    matmul_8xVL(c, a, b, 0, M, N, P, 0, P);
  }
}


// Ara's Versions
void imatmul_4x4(int8_t *c, const int8_t *a, const int8_t *b,
                 const uint32_t M, const uint32_t N,
                 const uint32_t P) {
  // We work on 4 rows of the matrix at once
  const uint32_t block_size = 4;
  uint32_t block_size_p;

  // Set the vector configuration
  asm volatile("vsetvli %0, %1, e8, m4, ta, ma" : "=r"(block_size_p) : "r"(P));

  // Slice the matrix into a manageable number of columns p_
  for (uint32_t p = 0; p < P; p += block_size_p) {
    // Set the vector length
    const int32_t p_ = MIN(P - p, block_size_p);

    // Find pointers to the submatrices
    const int8_t *b_ = b + p;
    int8_t *c_ = c + p;

    asm volatile("vsetvli zero, %0, e8, m4, ta, ma" ::"r"(p_));

    // Iterate over the rows
    for (uint32_t m = 0; m < M; m += block_size) {
      // Find pointer to the submatrices
      const int8_t *a_ = a + m * N;
      int8_t *c__ = c_ + m * P;

      // imatmul_vec_4x4_slice_init();
      asm volatile("vmv.v.i v0,  0");
      asm volatile("vmv.v.i v4,  0");
      asm volatile("vmv.v.i v8,  0");
      asm volatile("vmv.v.i v12, 0");
      // imatmul_vec_4x4(c__, a_, b_, N, P);
      // Temporary variables
      int8_t t0, t1, t2, t3;

      // Original pointer
      const int8_t *a__ = a_;

      // Prefetch one row of matrix B
      asm volatile("vle8.v v16, (%0);" ::"r"(b_));
      b_ += P;

      // Prefetch one row of scalar values
      t0 = *a_, a_ += N;
      t1 = *a_, a_ += N;
      t2 = *a_, a_ += N;
      t3 = *a_;

      // Compute the multiplication
      uint32_t n = 0;

      while (n < N) {
        // Calculate pointer to the matrix A
        a_ = a__ + ++n;

        asm volatile("vmacc.vx v0, %0, v16" ::"r"(t0));
        t0 = *a_, a_ += N;

        // Load one row of B
        asm volatile("vle8.v v20, (%0);" ::"r"(b_));
        b_ += P;

        asm volatile("vmacc.vx v4, %0, v16" ::"r"(t1));
        t1 = *a_, a_ += N;
        asm volatile("vmacc.vx v8, %0, v16" ::"r"(t2));
        t2 = *a_, a_ += N;
        asm volatile("vmacc.vx v12, %0, v16" ::"r"(t3));
        t3 = *a_;

        a_ = a__ + ++n;

        if (n == N)
          break;

        asm volatile("vmacc.vx v0, %0, v20" ::"r"(t0));
        t0 = *a_, a_ += N;

        // Load one row of B
        asm volatile("vle8.v v16, (%0);" ::"r"(b_));
        b_ += P;

        asm volatile("vmacc.vx v4, %0, v20" ::"r"(t1));
        t1 = *a_, a_ += N;
        asm volatile("vmacc.vx v8, %0, v20" ::"r"(t2));
        t2 = *a_, a_ += N;
        asm volatile("vmacc.vx v12, %0, v20" ::"r"(t3));
        t3 = *a_;
      }

      // Last iteration: store results
      asm volatile("vmacc.vx v0, %0, v20" ::"r"(t0));
      asm volatile("vse8.v v0, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v4, %0, v20" ::"r"(t1));
      asm volatile("vse8.v v4, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v8, %0, v20" ::"r"(t2));
      asm volatile("vse8.v v8, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v12, %0, v20" ::"r"(t3));
      asm volatile("vse8.v v12, (%0);" ::"r"(c__));

    }
  }
}
void imatmul_8x8(int8_t *c, const int8_t *a, const int8_t *b,
                 const uint32_t M, const uint32_t N,
                 const uint32_t P) {
  // We work on 8 rows of the matrix at once
  const uint32_t block_size = 8;
  uint32_t block_size_p;

  // Set the vector configuration
  asm volatile("vsetvli %0, %1, e8, m2, ta, ma" : "=r"(block_size_p) : "r"(P));

  // Slice the matrix into a manageable number of columns p_
  for (uint32_t p = 0; p < P; p += block_size_p) {
    // Set the vector length
    const uint32_t p_ = MIN(P - p, block_size_p);

    // Find pointers to the submatrices
    const int8_t *b_ = b + p;
    int8_t *c_ = c + p;

    asm volatile("vsetvli zero, %0, e8, m2, ta, ma" ::"r"(p_));

    // Iterate over the rows
    for (uint32_t m = 0; m < M; m += block_size) {
      // Find pointer to the submatrices
      const int8_t *a_ = a + m * N;
      int8_t *c__ = c_ + m * P;

      // imatmul_vec_8x8_slice_init();
      asm volatile("vmv.v.i v0,  0");
      asm volatile("vmv.v.i v2,  0");
      asm volatile("vmv.v.i v4,  0");
      asm volatile("vmv.v.i v6,  0");
      asm volatile("vmv.v.i v8,  0");
      asm volatile("vmv.v.i v10, 0");
      asm volatile("vmv.v.i v12, 0");
      asm volatile("vmv.v.i v14, 0");
      // imatmul_vec_8x8(c__, a_, b_, N, P);
      // Temporary variables
      int8_t t0, t1, t2, t3, t4, t5, t6, t7;

      // Original pointer
      const int8_t *a__ = a_;

      // Prefetch one row of matrix B
      asm volatile("vle8.v v18, (%0);" ::"r"(b_));
      b_ += P;

      // Prefetch one row of scalar values
      t0 = *a_, a_ += N;
      t1 = *a_, a_ += N;
      t2 = *a_, a_ += N;
      t3 = *a_, a_ += N;
      t4 = *a_, a_ += N;
      t5 = *a_, a_ += N;
      t6 = *a_, a_ += N;
      t7 = *a_;

      // Compute the multiplication
      uint32_t n = 0;

      while (n < N) {
        // Calculate pointer to the matrix A
        a_ = a__ + ++n;

        asm volatile("vmacc.vx v0, %0, v18" ::"r"(t0));
        t0 = *a_, a_ += N;

        // Load one row of B
        asm volatile("vle8.v v20, (%0);" ::"r"(b_));
        b_ += P;

        asm volatile("vmacc.vx v2, %0, v18" ::"r"(t1));
        t1 = *a_, a_ += N;
        asm volatile("vmacc.vx v4, %0, v18" ::"r"(t2));
        t2 = *a_, a_ += N;
        asm volatile("vmacc.vx v6, %0, v18" ::"r"(t3));
        t3 = *a_, a_ += N;
        asm volatile("vmacc.vx v8, %0, v18" ::"r"(t4));
        t4 = *a_, a_ += N;
        asm volatile("vmacc.vx v10, %0, v18" ::"r"(t5));
        t5 = *a_, a_ += N;
        asm volatile("vmacc.vx v12, %0, v18" ::"r"(t6));
        t6 = *a_, a_ += N;
        asm volatile("vmacc.vx v14, %0, v18" ::"r"(t7));
        t7 = *a_;

        a_ = a__ + ++n;

        if (n == N)
          break;

        asm volatile("vmacc.vx v0, %0, v20" ::"r"(t0));
        t0 = *a_, a_ += N;

        // Load one row of B
        asm volatile("vle8.v v18, (%0);" ::"r"(b_));
        b_ += P;

        asm volatile("vmacc.vx v2, %0, v20" ::"r"(t1));
        t1 = *a_, a_ += N;
        asm volatile("vmacc.vx v4, %0, v20" ::"r"(t2));
        t2 = *a_, a_ += N;
        asm volatile("vmacc.vx v6, %0, v20" ::"r"(t3));
        t3 = *a_, a_ += N;
        asm volatile("vmacc.vx v8, %0, v20" ::"r"(t4));
        t4 = *a_, a_ += N;
        asm volatile("vmacc.vx v10, %0, v20" ::"r"(t5));
        t5 = *a_, a_ += N;
        asm volatile("vmacc.vx v12, %0, v20" ::"r"(t6));
        t6 = *a_, a_ += N;
        asm volatile("vmacc.vx v14, %0, v20" ::"r"(t7));
        t7 = *a_;
      }

      // Last iteration: store results
      asm volatile("vmacc.vx v0, %0, v20" ::"r"(t0));
      asm volatile("vse8.v v0, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v2, %0, v20" ::"r"(t1));
      asm volatile("vse8.v v2, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v4, %0, v20" ::"r"(t2));
      asm volatile("vse8.v v4, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v6, %0, v20" ::"r"(t3));
      asm volatile("vse8.v v6, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v8, %0, v20" ::"r"(t4));
      asm volatile("vse8.v v8, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v10, %0, v20" ::"r"(t5));
      asm volatile("vse8.v v10, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v12, %0, v20" ::"r"(t6));
      asm volatile("vse8.v v12, (%0);" ::"r"(c__));
      c__ += P;
      asm volatile("vmacc.vx v14, %0, v20" ::"r"(t7));
      asm volatile("vse8.v v14, (%0);" ::"r"(c__));
    }
  }
}


int main() {
  
  // Print inputs
  printf("Matrix size: %dx%d\n", AVL, AVL);
  print_matrix(matrix1, AVL, AVL);
  print_matrix(matrix2, AVL, AVL);

  // gcc sometimes uses vector instructions to assign the date. So, we allow time before starting the benchmark 
  printf("Starting the benchmark...\n");
  
  // Issue fence instruction to start benchmark counter
  asm volatile("fence");
  asm volatile("fence");

  // Perform the operation
  // matmul_single(output, matrix1, matrix2, AVL, AVL, AVL);
  // matmul(output, matrix1, matrix2, AVL, AVL, AVL);
  // imatmul_8x8(output, matrix1, matrix2, AVL, AVL, AVL);
  // imatmul_4x4(output, matrix1, matrix2, AVL, AVL, AVL);
  matmul_4xVL(output, matrix1, matrix2, 0, AVL, AVL, AVL, 0, AVL);
  
  // Issue fence instruction to stop benchmark counter
  asm volatile("fence");

  // Check the result
  int error = 0;
  for (int indx = 0; indx < AVL*AVL; indx++) {
    if (output[indx] != golden_o[indx]) {
      printf("Error at index %d: Expected %X, Got %X\n", indx, (uint8_t)golden_o[indx], (uint8_t)output[indx]);
      error++;
    }
  }

  if (error>0) {
    printf("\n\nTest failed. Number of errors: %d\n\n", error);
    return 1;
  } else {
    printf("\n\nTest succeeded!!\n\n");
    return 0;
  }
}
