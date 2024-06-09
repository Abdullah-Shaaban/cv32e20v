// Copyright 2021 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Author: Muhammad Ijaz

#include "vector_macros.h"

void TEST_CASE1() {
  VSET(4, e8, m1);
  VLOAD_16(v2, 0x320, 0x5879, 0xFFCE, 0x19);
  VLOAD_8(v4, 7, 3, 6, 5);
  __asm__ volatile("csrw vxrm, 0");
  __asm__ volatile("vnclip.wv v1, v2, v4");
  // VCMP_I8(1, v1, 0x06, 0x7f, 0xff, 0x01); // v1 = 0x01ff7f06
  // uint32_t vxsat;
  // read_vxsat(vxsat);
  // check_vxsat(1, vxsat, 1);  // vxsat = 1
  reset_vxsat;
}

void TEST_CASE2() {
  VSET(4, e8, m1);
  VLOAD_16(v2, 0x320, 0x6, 0xD30E, 0x19);
  int8_t scalar = 3;
  __asm__ volatile("csrw vxrm, 1");
  __asm__ volatile("vnclip.wx v1, v2, %[A]" ::[A] "r"(scalar));
  // VCMP_I8(2, v1, 0x64, 0x01, 0x80, 0x03); // v1 = 0x03800164 
  // uint32_t vxsat;
  // read_vxsat(vxsat);
  // check_vxsat(1, vxsat, 1);  // vxsat = 1
  reset_vxsat;
}


void TEST_CASE3() {
  VSET(4, e8, m1);
  VLOAD_16(v2, 0x320, 0x19f, 0xFFCE, 0x19);
  __asm__ volatile("csrw vxrm, 3");
  __asm__ volatile("vnclip.wi v1, v2, 7");
  // VCMP_I8(3, v1, 0x07, 0x03, 0xff, 0x01); // v1 = 0x01ff0307
  // uint32_t vxsat;
  // read_vxsat(vxsat);
  // check_vxsat(1, vxsat, 0);  // vxsat = 0
  reset_vxsat;
}

int main(void) {
  INIT_CHECK();
  // enable_vec();
  TEST_CASE1();
  TEST_CASE2();
  TEST_CASE3();
  EXIT_CHECK();
}
