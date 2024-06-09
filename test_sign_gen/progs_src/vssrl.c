// Copyright 2021 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Author: Matheus Cavalcante <matheusd@iis.ee.ethz.ch>
//         Basile Bougenot <bbougenot@student.ethz.ch>

#include "vector_macros.h"

void TEST_CASE1() {
  VSET(4, e8, m1);
  VLOAD_8(v2, 0xff, 0x00, 0xf0, 0x0f);
  VLOAD_8(v3, 1, 2, 3, 4);
  __asm__ volatile("csrw vxrm, 1");
  __asm__ volatile("vssrl.vv v4, v2, v3");
//   VCMP_U8(1, v2, 0x7f, 0, 0x1e, 0x00);
}

void TEST_CASE3() {
  VSET(4, e8, m1);
  VLOAD_8(v2, 0xff, 0x00, 0xf0, 0x0f);
  __asm__ volatile("csrw vxrm, 3"); // set the rounding mode to round-to-odd
  __asm__ volatile("vssrl.vi v4, v2, 5");
//   VCMP_U8(3, v2, 7, 0, 7, 0);
}

void TEST_CASE5() {
  VSET(4, e8, m1);
  VLOAD_8(v2, 0xff, 0x00, 0xf0, 0x0f);
  uint32_t scalar = 5;
  __asm__ volatile("csrw vxrm, 0");
  __asm__ volatile("vssrl.vx v4, v2, %[A]" ::[A] "r"(scalar));
//   VCMP_U8(5, v4, 7, 0, 7, 0);
}

void TEST_CASE6() {
  VSET(4, e8, m1);
  VLOAD_8(v2, 0xff, 0x00, 0xf0, 0x0f);
  uint32_t scalar = 5;
  __asm__ volatile("csrw vxrm, 2");
  __asm__ volatile("vssrl.vx v4, v2, %[A]" ::[A] "r"(scalar));
//   VCMP_U8(5, v4, 7, 0, 7, 0);
}

int main(void) {
  INIT_CHECK();
//   enable_vec();
// Enable V Unit -- Set mstatus.VS (bits 9 and 10) to 2 (Clean)
  // __asm__ (
  //   "csrr t4, mstatus;"
  //   "li t5, 0x200;" 
  //   "or t4, t4, t5;"
  //   "csrw mstatus, t4;"
  //   );
  TEST_CASE1();
  TEST_CASE3();
  TEST_CASE5();
  TEST_CASE6();
  EXIT_CHECK();
}