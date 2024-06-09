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
 
  int8_t *i_cpy = i;  

  // Set the vector configuration
  int32_t block_size_c;
  asm volatile("vsetvli %0, %1, e8, m2, ta, ma" : "=r"(block_size_c) : "r"(num_columns));

  for (int32_t c = 0; c < num_columns; c += block_size_c) {

    int32_t c_ = MIN(num_columns - c, block_size_c);

    // First iteration round, r = 0
    int8_t *i = i_cpy + c;
    int8_t *o_ = o + c;

    int32_t lwi = N + F - 1;
    int32_t lwf = F;
    

    // Iterate over the output rows
    for (int32_t r = 0; r < num_rows; r += 1) {
      
      // Initialize output Row with 0 (for accumlation)
      asm volatile("vmv.v.i v0,  0"); 

      // Load 1st Filter Column
      int8_t t0, t1, t2;
      t0 = *f;        // t0 now holds filter(0,0) --> 1st Row, 1st Column
      t1 = *(f+F);    // t1 now holds filter(1,0) --> 2nd Row, 1st Column
      t2 = *(f+2*F);  // t2 now holds filter(2,0) --> 3rd Row, 1st Column

      // Load first image row and multiply it by filter(0,0)
      int8_t *i__ = i;
      asm volatile("vle8.v v8,  (%0)" :: "r"(i__)); // v8 holds 1st Row of image
      asm volatile("vmacc.vx v0, %0, v8" ::"r"(t0)); // v0+v1 hold 1st Row of output

      // Load second image row and multiply it by filter(1,0)
      i__ += lwi;
      asm volatile("vle8.v v10, (%0)" :: "r"(i__)); // v10 holds 2nd Row of image
      asm volatile("vmacc.vx v0, %0, v10" ::"r"(t1)); // Multiply filter(1,0) by the whole 2nd Row of Image, AND ACCUMULATE 
      

      // Load third image row and multiply it by filter(2,0)      
      i__ += lwi;
      asm volatile("vle8.v v12, (%0)" :: "r"(i__)); // v12 holds 3rd Row of image
      asm volatile("vmacc.vx v0, %0, v12" ::"r"(t2)); // Multiply filter(2,0) by the whole 3rd Row of Image, AND ACCUMULATE
      
      
      // Helper variables
      int8_t *f_ = f + (lwf << 1);
      int8_t *o__ = o_;

      // Fetch the middle column of the filter, and start calculating its contributions on the output rows.
      f_ = f + 1;
      t0 = *f_;   // t0 now holds filter(0,1)
      f_ += lwf;
      t1 = *f_;   // t1 now holds filter(1,1)
      f_ += lwf;
      t2 = *f_;   // t2 now holds filter(2,1)
      
      // To do so, slide down the Image rows by one
      int8_t sld_elem;
      int8_t *i_ = i + c_;
      sld_elem = *i_;
      asm volatile("vslide1down.vx v20, v8, %0" ::"r"(sld_elem));
      asm volatile("vmacc.vx v0, %0, v20" ::"r"(t0));
      
      i_ += lwi;
      sld_elem = *i_;
      asm volatile("vslide1down.vx v22, v10, %0" ::"r"(sld_elem));
      asm volatile("vmacc.vx v0, %0, v22" ::"r"(t1));

      i_ += lwi;
      sld_elem = *i_;
      asm volatile("vslide1down.vx v24, v12, %0" ::"r"(sld_elem));
      asm volatile("vmacc.vx v0, %0, v24" ::"r"(t2));

      // Repeat for the last filter column, and then store the output rows
      f_ = f + 2;
      t0 = *f_;
      f_ += lwf;
      t1 = *f_;
      f_ += lwf;
      t2 = *f_;

      // To do so, slide down the Image rows by one
      i_ = i + c_ + 1;
      sld_elem = *i_;
      asm volatile("vslide1down.vx v8, v20, %0" ::"r"(sld_elem));
      asm volatile("vmacc.vx v0, %0, v8" ::"r"(t0));

      i_ += lwi;
      sld_elem = *i_;
      asm volatile("vslide1down.vx v10, v22, %0" ::"r"(sld_elem));
      asm volatile("vmacc.vx v0, %0, v10" ::"r"(t1));
      
      i_ += lwi;
      sld_elem = *i_;
      asm volatile("vslide1down.vx v12, v24, %0" ::"r"(sld_elem));
      asm volatile("vmacc.vx v0, %0, v12" ::"r"(t2));


      // Store the output row
      asm volatile("vse8.v  v0, (%0)" :: "r"(o__));
      
      // Prepare for the next iteration
      i += N + F - 1;
      o_ += N;
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
  // printf("Filter:\n");
  // print_matrix(filter, F, F);
  // printf("Image:\n");
  // print_matrix(image, M+2, N+2);
  
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