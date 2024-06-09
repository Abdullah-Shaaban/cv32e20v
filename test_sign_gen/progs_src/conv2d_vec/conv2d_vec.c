// Copyright 2020 ETH Zurich and University of Bologna.
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

// Author: Matteo Perotti

#include <stdint.h>
#include <stdio.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

extern int M;
extern int N;
extern int F;

void conv2d_3x3(int8_t *o, int8_t *i, int8_t *f, int32_t num_rows, int32_t num_columns) {
  // We work on 4 rows of the output matrix at once
  int32_t block_size_o = 4;
  // We work on block_size_o + F - 1 rows of the input matrix at once

  int32_t block_size_c;
  int8_t *i_cpy = i;

  // Set the vector configuration
  asm volatile("vsetvli %0, %1, e8, m2, ta, ma" : "=r"(block_size_c) : "r"(num_columns));

  for (int32_t c = 0; c < num_columns; c += block_size_c) {

    int32_t c_ = MIN(num_columns - c, block_size_c);

    // First iteration round, r = 0
    int8_t *i = i_cpy + c;
    int8_t *o_ = o + c;

    int32_t lwi = N + F - 1;
    int32_t lwf = F;
    int32_t lwo = N;

    // Preload t0 and t1
    int8_t t0, t1, t2;
    t0 = *f;  // t0 now holds filter(0,0) -->  1st Row, 1st Column
    t1 = *(f+F);  // t0 now holds filter(1,0) --> 2nd Row, 1st Column

    // Preload the first two input rows -> This is not needed in the other rounds
    int8_t *i__ = i;
    asm volatile("vle8.v v8,  (%0)" :: "r"(i__)); // v8 holds 1st Row of image
    i__ += lwi;
    asm volatile("vmv.v.i v0,  0"); // vmv is used to Initialize output Rows with 0 (for accumlation)
    asm volatile("vmacc.vx v0, %0, v8" ::"r"(t0)); // v0+v1 hold 1st Row of output
                                                   // Here, filter(0,0) is multiplied by the whole 1st Row of Image
    asm volatile("vle8.v v10, (%0)" :: "r"(i__)); // v10 holds 2nd Row of image
    asm volatile("vmv.v.i v2,  0");
    asm volatile("vmacc.vx v2, %0, v10" ::"r"(t0)); // v2+v3 hold 2nd Row of output
                                                    // Here, filter(0,0) is multiplied by the whole 2nd Row of Image

    // Iterate over the output rows
    for (int32_t r = 0; r < num_rows; r += block_size_o) {
      // Helper variables
      int8_t *f_ = f + (lwf << 1);
      int8_t *o__ = o_;
      int8_t sld_elem;

      // Instructions in first part are ordered in a messy way, so
      // that we are able to hide latency of vector loads and stores

      int8_t *i__ = i + (F - 1) * (N + F - 1);
      asm volatile("vle8.v v12, (%0)" :: "r"(i__));
      i__ += lwi;

      asm volatile("vmacc.vx v0, %0, v10" ::"r"(t1)); // Multiply filter(1,0) by the whole 2nd Row of Image, AND ACCUMULATE 
      int8_t *i_ = i + c_;
      sld_elem = *i_;
      asm volatile("vmv.v.i v4,  0");
      asm volatile("vmacc.vx v4, %0, v12" ::"r"(t0));

      asm volatile("vle8.v v14, (%0)" :: "r"(i__));
      i__ += lwi;

      t2 = *f_;
      asm volatile("vslide1down.vx v20, v8, %0" ::"r"(sld_elem));
      i_ += lwi;
      sld_elem = *i_;

      asm volatile("vmacc.vx v2, %0, v12" ::"r"(t1));
      asm volatile("vmacc.vx v0, %0, v12" ::"r"(t2)); // Multiply filter(2,0) by the whole 3rd Row of Image, AND ACCUMULATE
      asm volatile("vmv.v.i v6,  0");

      asm volatile("vmacc.vx v6, %0, v14" ::"r"(t0));

      asm volatile("vle8.v v16, (%0)" :: "r"(i__));
      i__ += lwi;

      asm volatile("vmacc.vx v4, %0, v14" ::"r"(t1));
      asm volatile("vmacc.vx v2, %0, v14" ::"r"(t2));
      asm volatile("vmacc.vx v6, %0, v16" ::"r"(t1));

      asm volatile("vle8.v v18, (%0)" :: "r"(i__));

      f_ = f + 1;
      t0 = *f_; // t0 now holds filter(0,1)
      f_ += lwf;
      asm volatile("vmacc.vx v4, %0, v16" ::"r"(t2));

      // Fetch the middle column of the filter, and start calculating its
      // contributions on the output rows. To do so, slide down the Image rows by one
      t1 = *f_;
      f_ += lwf;
      asm volatile("vslide1down.vx v22, v10, %0" ::"r"(sld_elem));
      i_ += lwi;
      sld_elem = *i_;
      asm volatile("vmacc.vx v0, %0, v20" ::"r"(t0));
      asm volatile("vmacc.vx v6, %0, v18" ::"r"(t2));

      t2 = *f_;
      asm volatile("vslide1down.vx v24, v12, %0" ::"r"(sld_elem));
      i_ += lwi;
      sld_elem = *i_;
      asm volatile("vmacc.vx v0, %0, v22" ::"r"(t1));
      asm volatile("vmacc.vx v2, %0, v22" ::"r"(t0));

      asm volatile("vslide1down.vx v26, v14, %0" ::"r"(sld_elem));
      i_ += lwi;
      sld_elem = *i_;
      asm volatile("vmacc.vx v0, %0, v24" ::"r"(t2));
      asm volatile("vmacc.vx v2, %0, v24" ::"r"(t1));
      asm volatile("vmacc.vx v4, %0, v24" ::"r"(t0));

      asm volatile("vslide1down.vx v28, v16, %0" ::"r"(sld_elem));
      i_ += lwi;
      sld_elem = *i_;
      asm volatile("vmacc.vx v2, %0, v26" ::"r"(t2));
      asm volatile("vmacc.vx v4, %0, v26" ::"r"(t1));
      asm volatile("vmacc.vx v6, %0, v26" ::"r"(t0));

      asm volatile("vslide1down.vx v30, v18, %0" ::"r"(sld_elem));
      i_ = i + c_ + 1;
      sld_elem = *i_;
      asm volatile("vmacc.vx v4, %0, v28" ::"r"(t2));
      asm volatile("vmacc.vx v6, %0, v28" ::"r"(t1));

      f_ = f + 2;
      t0 = *f_;
      f_ += lwf;
      asm volatile("vslide1down.vx v8, v20, %0" ::"r"(sld_elem));
      i_ += lwi;
      sld_elem = *i_;
      asm volatile("vmacc.vx v6, %0, v30" ::"r"(t2));

      // Repeat for the last filter column, and then store the output rows
      t1 = *f_;
      f_ += lwf;
      asm volatile("vslide1down.vx v10, v22, %0" ::"r"(sld_elem));
      i_ += lwi;
      sld_elem = *i_;
      asm volatile("vmacc.vx v0, %0, v8" ::"r"(t0));

      t2 = *f_;
      asm volatile("vslide1down.vx v12, v24, %0" ::"r"(sld_elem));
      i_ += lwi;
      sld_elem = *i_;
      asm volatile("vmacc.vx v0, %0, v10" ::"r"(t1));
      asm volatile("vmacc.vx v0, %0, v12" ::"r"(t2));
      i_ += lwi;
      int8_t sld_elem_tmp = *i_;
      asm volatile("vse8.v  v0, (%0)" :: "r"(o__));
      o__ += lwo;
      asm volatile("vmv.v.v v8, v16");
      asm volatile("vmacc.vx v2, %0, v10" ::"r"(t0));
      asm volatile("vmv.v.v v10, v18");

      asm volatile("vmacc.vx v2, %0, v12" ::"r"(t1));
      asm volatile("vmacc.vx v4, %0, v12" ::"r"(t0));

      asm volatile("vslide1down.vx v14, v26, %0" ::"r"(sld_elem));
      asm volatile("vmacc.vx v2, %0, v14" ::"r"(t2));
      asm volatile("vslide1down.vx v16, v28, %0" ::"r"(sld_elem_tmp));
      i_ += lwi;
      sld_elem = *i_;
      asm volatile("vse8.v  v2, (%0)" :: "r"(o__));
      o__ += lwo;
      asm volatile("vmacc.vx v4, %0, v14" ::"r"(t1));
      asm volatile("vmacc.vx v6, %0, v14" ::"r"(t0));

      asm volatile("vmacc.vx v4, %0, v16" ::"r"(t2));
      f_ = f;
      t0 = *f_;
      f_ += lwf;
      asm volatile("vse8.v  v4, (%0)" :: "r"(o__));
      o__ += lwo;
      asm volatile("vmacc.vx v6, %0, v16" ::"r"(t1));
      asm volatile("vslide1down.vx v18, v30, %0" ::"r"(sld_elem));

      asm volatile("vmacc.vx v6, %0, v18" ::"r"(t2));
      t1 = *f_;
      asm volatile("vse8.v  v6, (%0);" : "+r"(o__));

      asm volatile("vmv.v.i v0,  0");
      asm volatile("vmacc.vx v0, %0, v8" ::"r"(t0));
      i += block_size_o * (N + F - 1);
      o_ += block_size_o * N;
      asm volatile("vmv.v.i v2,  0");
      asm volatile("vmacc.vx v2, %0, v10" ::"r"(t0));
    }
  }
}


// Define Matrix dimensions:
// o = i ° f, with i=[MxN], f=[FxF], o=[MxN]
// The filter is a square matrix, and F is odd

// Matrices defined in data.S
extern int8_t image[] __attribute__((aligned(4)));    // [ (M+2*floor(F/2)) * (N+2*floor(F/2)) ] = [ (M+1) * (N+1) ]
extern int8_t filter[] __attribute__((aligned(4)));   // [ F*F ]
extern int8_t output[] __attribute__((aligned(4)));   // [ M*N ]
extern int8_t golden_o[] __attribute__((aligned(4))); // [ M*N ]

void print_matrix(int8_t const *matrix, uint32_t num_rows, uint32_t num_columns) {
  printf("0x%8X\n", (unsigned int)matrix);
  for (uint32_t i = 0; i < num_rows; ++i) {
    for (uint32_t j = 0; j < num_columns; ++j) {
      printf("%2X ", (uint8_t)matrix[i * num_columns + j]);
    }
    printf("\n");
  }
}

int main(){

  // Print input matrices
  printf("Filter:\n");
  print_matrix(filter, F, F);
  printf("Image:\n");
  print_matrix(image, M+2, N+2);
  
  // Issue fence instruction to start benchmark counter
  asm volatile("fence");
  asm volatile("fence");

  // Execute convolution. Dimensions are given for output matrix.
  conv2d_3x3(output, image, filter, M, N);
  
  // Issue fence instruction to stop benchmark counter
  asm volatile("fence");

  // Print output matrix
  printf("\nOutput:\n");
  print_matrix(output, M, N);

  // Verify correctness
  printf("Verifying result...\n");
  int error = 0;
  for (int indx = 0; indx < M * N; indx++) {
    if (output[indx] != golden_o[indx]) {
      printf("Error at index %d: Expected %X, Got %X\n", indx, (uint8_t)golden_o[indx], (uint8_t)output[indx]);
      error +=1;
    }
  }
  if (error == 0) {
    printf("\n\nTest passed!\n\n");
  } else {
    printf("\n\nTest failed with %d errors!\n\n", error);
    return 1;
  }
  return 0;
}