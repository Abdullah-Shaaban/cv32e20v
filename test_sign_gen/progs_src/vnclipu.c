// Copyright 2021 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Author: Muhammad Ijaz

#include "vector_macros.h"

void TEST_CASE1() {
  VSET(4, e8, m1);
  VLOAD_16(v2, 0x320, 0x5879, 0xFCE, 0x19);
  VLOAD_8(v4, 7, 3, 6, 5);
  __asm__ volatile("csrw vxrm, 0");
  __asm__ volatile("vnclipu.wv v1, v2, v4");
  // VCMP_U8(3, v1, 0x06, 0xff, 0x3f, 0x01); // v1 = 0x013fff06
  // uint32_t vxsat;
  // read_vxsat(vxsat);
  // check_vxsat(1, vxsat, 1);  // vxsat = 1
  reset_vxsat;
}

void TEST_CASE2() {
  VSET(4, e8, m1);
  VLOAD_16(v2, 0x320, 0x6, 0xD30E, 0x19);
  int8_t scalar = 4;
  __asm__ volatile("csrw vxrm, 1");
  __asm__ volatile("vnclipu.wx v1, v2, %[A]" ::[A] "r"(scalar));
  // VCMP_U8(2, v1, 0x32, 0x00, 0xff, 0x02); // v1 = 0x02ff0032 
  // uint32_t vxsat;
  // read_vxsat(vxsat);
  // check_vxsat(1, vxsat, 1);  // vxsat = 1
  reset_vxsat;
}


void TEST_CASE3() {
  VSET(4, e8, m1);
  VLOAD_16(v2, 0x320, 0x19f, 0x3CE, 0x19);
  __asm__ volatile("csrw vxrm, 3");
  __asm__ volatile("vnclipu.wi v1, v2, 7");
  // VCMP_U8(3, v1, 0x07, 0x03, 0x07, 0x01); // v1 = 0x01070307
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