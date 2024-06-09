// Copyright 2021 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Author: Matheus Cavalcante <matheusd@iis.ee.ethz.ch>
//         Basile Bougenot <bbougenot@student.ethz.ch>

#include "vector_macros.h"

void TEST_CASE1() {
  uint32_t vxsat;
  VSET(3, e8, m1);
  VLOAD_8(v2, 127, 127, -128);
  VLOAD_8(v3, 127, 10, -128);
  __asm__ volatile("csrw vxrm, 2");
  __asm__ volatile("vsmul.vv v4, v2, v3");
//   VCMP_I8(1, v4, 126, 9, 127);
//   read_vxsat(vxsat);
//   check_vxsat(1, vxsat, 1);
  reset_vxsat;
}

void TEST_CASE2() {
  uint32_t vxsat;
  VSET(3, e8, m1);
  VLOAD_8(v2, 127, 63, -50);
  int8_t scalar = 55;
  __asm__ volatile("csrw vxrm, 1");
  __asm__ volatile("vsmul.vx v4, v2, %[A]" ::[A] "r"(scalar));
//   VCMP_I8(2, v4, 54, 27, -22);   // v4 = 0x00eb1b37
//   read_vxsat(vxsat);
//   check_vxsat(1, vxsat, 0);  // vxsat = 0
  reset_vxsat;
}

void TEST_CASE3() {
  uint32_t vxsat;
  VSET(3, e8, m1);
  VLOAD_8(v2, 1, -73, 50);
  int8_t scalar = 55;
  __asm__ volatile("csrw vxrm, 3");
  __asm__ volatile("vsmul.vx v4, v2, %[A]" ::[A] "r"(scalar));
//   VCMP_I8(3, v4, 54, 27, -22); // v4 = 0x0015e101
//   read_vxsat(vxsat);
//   check_vxsat(1, vxsat, 0);  // vxsat = 0
  reset_vxsat;
}


int main(void) {
  INIT_CHECK();
//   enable_vec();
  TEST_CASE1();
  TEST_CASE2();
  TEST_CASE3();
  EXIT_CHECK();
}