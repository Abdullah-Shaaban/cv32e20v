// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

/*
 * Module Description: this module is the top level that instantiates and connects 
 * the scalar unit CVE2 and the vector unit Spatz.
 * 
 * NOTE: this module is adapted from cve2_top_tracing to be used in the UVM TB from CORE-V-VERIF,
 * but it should be changed before the synthesis and power flows.
*/

module cve2v_top_tracing import cve2_pkg::*; #(
    parameter int unsigned MHPMCounterNum   = 0,
    parameter int unsigned MHPMCounterWidth = 40,
    parameter bit          RV32E            = 1'b0,
    parameter rv32m_e      RV32M            = RV32MFast,
    parameter bit          BranchPredictor  = 1'b0,
    parameter int unsigned DmHaltAddr       = 32'h1A110800,
    parameter int unsigned DmExceptionAddr  = 32'h1A110808
  ) (
    // Clock and Reset
    input  logic                         clk_i,
    input  logic                         rst_ni,
  
    input  logic                         test_en_i,     // enable all clock gates for testing
    input  prim_ram_1p_pkg::ram_1p_cfg_t ram_cfg_i,
  
  
    input  logic [31:0]                  hart_id_i,
    input  logic [31:0]                  boot_addr_i,
  
    // Instruction memory interface
    output logic                         instr_req_o,
    input  logic                         instr_gnt_i,
    input  logic                         instr_rvalid_i,
    output logic [31:0]                  instr_addr_o,
    input  logic [31:0]                  instr_rdata_i,
    input  logic                         instr_err_i,
  
    // Data memory interface
    output logic                         data_req_o,
    input  logic                         data_gnt_i,
    input  logic                         data_rvalid_i,
    output logic                         data_we_o,
    output logic [3:0]                   data_be_o,
    output logic [31:0]                  data_addr_o,
    output logic [31:0]                  data_wdata_o,
    input  logic [31:0]                  data_rdata_i,
    input  logic                         data_err_i,
  
    // Interrupt inputs
    input  logic                         irq_software_i,
    input  logic                         irq_timer_i,
    input  logic                         irq_external_i,
    input  logic [15:0]                  irq_fast_i,
    input  logic                         irq_nm_i,       // non-maskeable interrupt
  
    // Debug Interface
    input  logic                         debug_req_i,
    output crash_dump_t                  crash_dump_o,
  
    // CPU Control Signals
    input  logic                         fetch_enable_i,
    output logic                         core_sleep_o
  
  );

  // cve2_tracer relies on the signals from the RISC-V Formal Interface
  `ifndef RVFI
    // $fatal(1,"Fatal error: RVFI needs to be defined globally.");
  `endif
  logic        rvfi_valid;
  logic [63:0] rvfi_order;
  logic [31:0] rvfi_insn;
  logic        rvfi_trap;
  logic        rvfi_halt;
  logic        rvfi_intr;
  logic [ 1:0] rvfi_mode;
  logic [ 1:0] rvfi_ixl;
  logic [ 4:0] rvfi_rs1_addr;
  logic [ 4:0] rvfi_rs2_addr;
  logic [ 4:0] rvfi_rs3_addr;
  logic [31:0] rvfi_rs1_rdata;
  logic [31:0] rvfi_rs2_rdata;
  logic [31:0] rvfi_rs3_rdata;
  logic [ 4:0] rvfi_rd_addr;
  logic [31:0] rvfi_rd_wdata;
  logic [31:0] rvfi_pc_rdata;
  logic [31:0] rvfi_pc_wdata;
  logic [31:0] rvfi_mem_addr;
  logic [ 3:0] rvfi_mem_rmask;
  logic [ 3:0] rvfi_mem_wmask;
  logic [31:0] rvfi_mem_rdata;
  logic [31:0] rvfi_mem_wdata;
  logic [31:0] rvfi_ext_mip;
  logic        rvfi_ext_nmi;
  logic        rvfi_ext_debug_req;
  logic [63:0] rvfi_ext_mcycle;

  logic [31:0] unused_rvfi_ext_mip;
  logic        unused_rvfi_ext_nmi;
  logic        unused_rvfi_ext_debug_req;
  logic [63:0] unused_rvfi_ext_mcycle;

  // Tracer doesn't use these signals, though other modules may probe down into tracer to observe
  // them.
  assign unused_rvfi_ext_mip = rvfi_ext_mip;
  assign unused_rvfi_ext_nmi = rvfi_ext_nmi;
  assign unused_rvfi_ext_debug_req = rvfi_ext_debug_req;
  assign unused_rvfi_ext_mcycle = rvfi_ext_mcycle;

  cve2v_top #(
    .MHPMCounterNum   (MHPMCounterNum),
    .MHPMCounterWidth (MHPMCounterWidth),
    .RV32E            (RV32E),
    .RV32M            (RV32M),
    .XIF              (1),
    .DmHaltAddr       (DmHaltAddr),
    .DmExceptionAddr  (DmExceptionAddr)
  ) u_cve2v_top (
    .clk_i,
    .rst_ni,

    .test_en_i,
`ifndef GLS
    .ram_cfg_i,
`else
    .ram_cfg_i,
    // .ram_cfg_i_ram_cfg_cfg_en('0), .ram_cfg_i_ram_cfg_cfg_3_('0),
    // .ram_cfg_i_ram_cfg_cfg_2_('0), .ram_cfg_i_ram_cfg_cfg_1_('0),
    // .ram_cfg_i_ram_cfg_cfg_0_('0), .ram_cfg_i_rf_cfg_cfg_en('0),
    // .ram_cfg_i_rf_cfg_cfg_3_('0), .ram_cfg_i_rf_cfg_cfg_2_('0),
    // .ram_cfg_i_rf_cfg_cfg_1_('0), .ram_cfg_i_rf_cfg_cfg_0_('0),
`endif
    .hart_id_i,
    .boot_addr_i,

    .instr_req_o,
    .instr_gnt_i,
    .instr_rvalid_i,
    .instr_addr_o,
    .instr_rdata_i,
    .instr_err_i,

    .data_req_o,
    .data_gnt_i,
    .data_rvalid_i,
    .data_we_o,
    .data_be_o,
    .data_addr_o,
    .data_wdata_o,
    .data_rdata_i,
    .data_err_i,

    .irq_software_i,
    .irq_timer_i,
    .irq_external_i,
    .irq_fast_i,
    .irq_nm_i,

    .debug_req_i,
`ifndef GLS
    .crash_dump_o,
`endif
`ifdef RVFI
    .rvfi_valid,
    .rvfi_order,
    .rvfi_insn,
    .rvfi_trap,
    .rvfi_halt,
    .rvfi_intr,
    .rvfi_mode,
    .rvfi_ixl,
    .rvfi_rs1_addr,
    .rvfi_rs2_addr,
    .rvfi_rs3_addr,
    .rvfi_rs1_rdata,
    .rvfi_rs2_rdata,
    .rvfi_rs3_rdata,
    .rvfi_rd_addr,
    .rvfi_rd_wdata,
    .rvfi_pc_rdata,
    .rvfi_pc_wdata,
    .rvfi_mem_addr,
    .rvfi_mem_rmask,
    .rvfi_mem_wmask,
    .rvfi_mem_rdata,
    .rvfi_mem_wdata,
    .rvfi_ext_mip,
    .rvfi_ext_nmi,
    .rvfi_ext_debug_req,
    .rvfi_ext_mcycle,
`endif
    .fetch_enable_i,
    .core_sleep_o
  );

  // cve2_tracer u_cve2_tracer (
  //   .clk_i,
  //   .rst_ni,

  //   .hart_id_i,

  //   .rvfi_valid,
  //   .rvfi_order,
  //   .rvfi_insn,
  //   .rvfi_trap,
  //   .rvfi_halt,
  //   .rvfi_intr,
  //   .rvfi_mode,
  //   .rvfi_ixl,
  //   .rvfi_rs1_addr,
  //   .rvfi_rs2_addr,
  //   .rvfi_rs3_addr,
  //   .rvfi_rs1_rdata,
  //   .rvfi_rs2_rdata,
  //   .rvfi_rs3_rdata,
  //   .rvfi_rd_addr,
  //   .rvfi_rd_wdata,
  //   .rvfi_pc_rdata,
  //   .rvfi_pc_wdata,
  //   .rvfi_mem_addr,
  //   .rvfi_mem_rmask,
  //   .rvfi_mem_wmask,
  //   .rvfi_mem_rdata,
  //   .rvfi_mem_wdata
  // );

  ///////////////////////////////
  ////// Benchmarking Logic /////
  ///////////////////////////////
`ifdef BENCHMARK
  enum logic [2:0] {
    BENCHMARK_IDLE,
    BENCMARK_READY,
    BENCHMARK_RUNNING,
    WAITING_FOR_SPATZ,
    BENCHMARK_DONE
  } benchmark_state;
  logic fence_insn_flag;
  logic [31:0] benchmark_counter;

  // Raise flag when fence reaches decode stage
  assign fence_insn_flag = u_cve2v_top.u_cve2_top.u_cve2_core.id_stage_i.instr_rdata_i == 32'h0ff0000f &&
                           u_cve2v_top.u_cve2_top.u_cve2_core.id_stage_i.instr_valid_i == 1'b1;

  // FSM
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      benchmark_state <= BENCHMARK_IDLE;
    end else begin
      case (benchmark_state)
        BENCHMARK_IDLE: begin
          if (fence_insn_flag) begin
            benchmark_state <= BENCMARK_READY;
          end
        end
        BENCMARK_READY: begin // Needed in case fence is preceded by branch
          if (fence_insn_flag) begin
            benchmark_state <= BENCHMARK_RUNNING;
          end else begin
            benchmark_state <= BENCHMARK_IDLE;
          end
        end
        BENCHMARK_RUNNING: begin
`ifdef BNCH_VECTOR
          if (fence_insn_flag) begin
            benchmark_state <= WAITING_FOR_SPATZ;
          end
`else
          if (fence_insn_flag) begin
            benchmark_state <= BENCHMARK_DONE;
          end
`endif
        end
        WAITING_FOR_SPATZ: begin
          if (u_cve2v_top.u_spatz.i_controller.running_insn_d == '0 ) begin
            benchmark_state <= BENCHMARK_DONE;
          end
        end
        BENCHMARK_DONE: begin
          benchmark_state <= BENCHMARK_IDLE;
        end
      endcase
    end
  end

  // Counter
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      benchmark_counter <= 32'h0;
    end else begin
      if (benchmark_state == BENCHMARK_RUNNING || benchmark_state == WAITING_FOR_SPATZ) begin
        benchmark_counter <= benchmark_counter + 32'h1;
      end
    end
  end

  // Print the counter value
  realtime start_time, end_time;
  int fd; // File handle
  initial begin
    // Dump VCD. Change the `vopt` visibility options from `+acc` to `-debug` if necessary.
    // $dumpfile("waves.vcd");
    // $dumpvars(0, uvmt_cv32e20_tb.dut_wrap.cv32e20_top_i);
    // $fsdbDumpfile("waves.fsdb");
    // $fsdbDumpvars(0, u_cve2v_top.u_cve2_top.u_cve2_core, "+all");

    // Wait for the benchmark to start
    // $dumpoff;
    // $fsdbDumpoff;
    wait(benchmark_state == BENCHMARK_RUNNING);

    // Start Dumping the VCD
    // $dumpon;
    // $fsdbDumpon;
    start_time = $realtime;
    $display("========================================================================");
    $display("====================     Benchmarking start time: %t    ================", start_time);
    $display("========================================================================");
`ifdef BNCH_VECTOR    
    wait(benchmark_state == WAITING_FOR_SPATZ)
    // Force cve2 to stall in decode stage until the vector unit finishes the benchmark
    force u_cve2v_top.u_cve2_top.u_cve2_core.id_stage_i.stall_id = 1;
    
    wait(benchmark_state == BENCHMARK_DONE);
    // Release the stall
    release u_cve2v_top.u_cve2_top.u_cve2_core.id_stage_i.stall_id;
`else
    wait(benchmark_state == BENCHMARK_DONE);
`endif
    // End the benchmark
    // $dumpoff;
    // $fsdbDumpoff;
    end_time = $realtime;
    $display("==========================================================================");
    $display("======================     Benchmark end time: %t    =====================", end_time);
    $display("======================     Benchmark counter: %d    ======================", benchmark_counter);
    $display("==========================================================================");
    // Store the benchmark results in a file
    fd = $fopen("benchmark_results.log", "w");
    $fdisplay(fd, "Benchmark counter: %d", benchmark_counter);
    $fdisplay(fd, "Benchmark start time: %t", start_time);
    $fdisplay(fd, "Benchmark end time: %t", end_time);
    $fclose(fd);
  end

`endif


  
  //////////////////////////////
  //////// Assertions //////////
  //////////////////////////////
  
  // // Default Clocking Block, avoids the need for @(posedge clk_i) in the assertions
  // default clocking @(posedge clk_i); endclocking
  
  // // Property 1: for each valid issue request, there is a ready response. 
  // // NOTE: "or" allows CVE2 to retract its issue request, which is valid behavior.
  // assert property ($rose(xif_if.issue_valid) |-> ##[1:$] xif_if.issue_ready or $fell(xif_if.issue_valid))
  //     else $error("xif_if.issue_ready is not asserted for a valid issue request");
  
  // // Property 2: The issue_req signals are valid when issue_valid is 1
  // assert property (xif_if.issue_valid |-> !$isunknown(xif_if.issue_req))
  //     else $error("xif_if.issue_req is unknown when xif_if.issue_valid is HIGH");
  
  // // Property 3: The signals in issue_resp are valid when issue_valid and issue_ready are both 1
  // assert property (xif_if.issue_valid && xif_if.issue_ready |-> !$isunknown(xif_if.issue_resp))
  //     else $error("xif_if.issue_resp is unknown during an XIF issue transaction when xif_if.issue_valid and xif_if.issue_ready are both HIGH");
  
  // // Property 4: The register.hartid, register.id, register.ecs_valid and register.rs_valid signals are valid when register_valid is 1.
  // assert property (xif_if.register_valid |-> !$isunknown(xif_if.register.hartid) && !$isunknown(xif_if.register.id) && !$isunknown(xif_if.register.ecs_valid) && !$isunknown(xif_if.register.rs_valid))
  //     else $error("xif_if.register signals are unknown when xif_if.register_valid is HIGH");
  
  // Property 5: The register.rs signals required to be stable during the register.rs_valid bits to be 1.
  // NOTE: using overlapping implication "|->" requires that rs does not change from its value just before rs_valid goes HIGH!!
  // assert property ($rose(xif_if.register.rs_valid[0]) |=> $stable(xif_if.register.rs[0]) until xif_if.register_ready)
  //     else $error("xif_if.register.rs[0] is not stable during a XIF register transaction");
  // assert property ($rose(xif_if.register.rs_valid[1]) |=> $stable(xif_if.register.rs[1]) until xif_if.register_ready)
  //     else $error("xif_if.register.rs[1] is not stable during a XIF register transaction");
  // assert property (xif_if.register.rs_valid[2] |=> $stable(xif_if.register.rs[2])); // NOTE: not used
  
  // // Property 6: The signals in result are valid when result_valid is 1. 
  // assert property (xif_if.result_valid |-> !$isunknown(xif_if.result))
  //     else $error("xif_if.result is unknown when xif_if.result_valid is HIGH");
  
  // Property 7: The signals in result shall remain stable during a result transaction.
  // assert property ($rose(xif_if.result_valid) |=> $stable(xif_if.result) until xif_if.result_ready) 
  //     else $error("xif_if.result is not stable during a XIF result transaction");
  // Alternative to the above. This assertion means that the <<condition>> "result is stable" holds throughout 
  // the <<sequence>> of "result_valid being HIGH for 1 cycle or more".
  // assert property ($rose(xif_if.result_valid) |=> $stable(xif_if.result) throughout xif_if.result_valid [*1:$])
  //     else $error("xif_if.result is not stable during a XIF result transaction");


endmodule