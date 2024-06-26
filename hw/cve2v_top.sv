// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

/*
 * Module Description: this module is the top level that instantiates and connects 
 * the scalar unit CVE2 and the vector unit Spatz.
*/

module cve2v_top import cve2_pkg::*; #(
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

      // RISC-V Formal Interface
`ifdef RVFI
        output logic                         rvfi_valid,
        output logic [63:0]                  rvfi_order,
        output logic [31:0]                  rvfi_insn,
        output logic                         rvfi_trap,
        output logic                         rvfi_halt,
        output logic                         rvfi_intr,
        output logic [ 1:0]                  rvfi_mode,
        output logic [ 1:0]                  rvfi_ixl,
        output logic [ 4:0]                  rvfi_rs1_addr,
        output logic [ 4:0]                  rvfi_rs2_addr,
        output logic [ 4:0]                  rvfi_rs3_addr,
        output logic [31:0]                  rvfi_rs1_rdata,
        output logic [31:0]                  rvfi_rs2_rdata,
        output logic [31:0]                  rvfi_rs3_rdata,
        output logic [ 4:0]                  rvfi_rd_addr,
        output logic [31:0]                  rvfi_rd_wdata,
        output logic [31:0]                  rvfi_pc_rdata,
        output logic [31:0]                  rvfi_pc_wdata,
        output logic [31:0]                  rvfi_mem_addr,
        output logic [ 3:0]                  rvfi_mem_rmask,
        output logic [ 3:0]                  rvfi_mem_wmask,
        output logic [31:0]                  rvfi_mem_rdata,
        output logic [31:0]                  rvfi_mem_wdata,
        output logic [31:0]                  rvfi_ext_mip,
        output logic                         rvfi_ext_nmi,
        output logic                         rvfi_ext_debug_req,
        output logic [63:0]                  rvfi_ext_mcycle,
`endif  

    // CPU Control Signals
    input  logic                         fetch_enable_i,
    output logic                         core_sleep_o
);
  
  // The data memory interface signals pass  through the arbiter first
  // Scalar signals
  logic          sdata_req;
  logic          sdata_gnt;
  logic          sdata_rvalid;
  logic          sdata_we;
  logic [3:0]    sdata_be;
  logic [31:0]   sdata_addr;
  logic [31:0]   sdata_wdata;
  logic [31:0]   sdata_rdata;
  logic          sdata_err;
  // Vector signals
  logic          vdata_we;
  logic [3:0]    vdata_be;
  logic [31:0]   vdata_addr;
  logic [31:0]   vdata_wdata;
  logic          vdata_req;
  logic          vdata_gnt;
  logic          vdata_rvalid;
  logic          vdata_err;
  logic [31:0]   vdata_rdata;

  // eXtension interface instance to connect CVE2 and Spatz
  cve2_if_xif xif_if();

  // Data memory arbiter
  data_mem_arbiter u_data_mem_arbiter (
    .clk_i                  (clk_i),
    .rst_ni                 (rst_ni),
    
    // Scalar interface
    .sdata_req_i            (sdata_req),
    .sdata_we_i             (sdata_we),
    .sdata_be_i             (sdata_be),
    .sdata_addr_i           (sdata_addr),
    .sdata_wdata_i          (sdata_wdata),
    .sdata_gnt_o            (sdata_gnt),
    .sdata_rvalid_o         (sdata_rvalid),
    .sdata_err_o            (sdata_err),
    .sdata_rdata_o          (sdata_rdata),

    // Vector interface
    .vdata_we_i             (vdata_we),
    .vdata_be_i             (vdata_be),
    .vdata_addr_i           (vdata_addr),
    .vdata_wdata_i          (vdata_wdata),
    .vdata_req_i            (vdata_req),
    .vdata_gnt_o            (vdata_gnt),
    .vdata_rvalid_o         (vdata_rvalid),
    .vdata_err_o            (vdata_err),
    .vdata_rdata_o          (vdata_rdata),

    // Memory interface
    .data_gnt_i,
    .data_rvalid_i,
    .data_err_i,
    .data_rdata_i,
    .data_req_o,
    .data_we_o,
    .data_addr_o,
    .data_be_o,
    .data_wdata_o
  );
  

  //////////////////////////////
  /////////// Spatz ////////////
  //////////////////////////////
  // Parameters
  localparam unsigned NumMemPortsPerSpatz = spatz_pkg::N_FU; // NOTE: The number of memory ports needs to be equal to the number of FUs
  localparam NumSpatzOutstandingLoads = 1;
  localparam RegisterRsp = 0; // Don't register the XIF result interface of Spatz
  // Typedefs
  // NOTE: The types for the data memory interface for Spatz are taken from the following 
  //       file: spatz/hw/ip/tcdm_interface/include/tcdm_interface/typedef.svh
  typedef struct packed { 
      logic [31:0] addr;  
      logic        write; // Write enable
      logic [3:0]  amo;   // Stub, not used
      logic [31:0] data;  
      logic [3:0]  strb;  // Byte enable
      logic        user;  // Stub, not used  
  } spatz_mem_req_t;
  typedef struct packed { 
      logic [31:0] data;  
  } spatz_mem_rsp_t;
  // NOTE: the XIF interface types for Spatz are taken from the following file:
  //       spatz/hw/system/spatz_cluster/src/spatz_cluster.sv
  typedef struct packed {
      logic [5:0]   id;           // Spatz expects to contains "rd" field from the instruction.
      logic [31:0]  data_op;      // Instr
      logic [31:0]  data_arga;    // RS1
      logic [31:0]  data_argb;    // RS2
      logic [31:0]  data_argc;    // Stub, won't be used
  } acc_issue_req_t;
  typedef struct packed {
  logic accept;
  logic writeback;
  logic loadstore;    // Stub, won't be used
  logic exception;    // Should be part of Result Interface
  logic isfloat;      // Stub, won't be used
  } acc_issue_rsp_t;
  typedef struct packed {
  logic [5:0]  id;    // contains "rd" field from the instruction
  logic        we;    // Write value back to the scalar core
  logic        error; // Stub, won't be used
  logic [31:0] data;
  } acc_rsp_t;

  // "Gluing" XIF interfaces if CVE2 and Spatz
  acc_issue_req_t spatz_issue_req;
  acc_issue_rsp_t spatz_issue_resp;
  acc_rsp_t spatz_result;
  always_comb begin : glue_xif
    // NOTE: unused fields are tied to 0 for now
    spatz_issue_req ='{
        id : xif_if.issue_req.instr[11:7],  // rd field
        data_op : xif_if.issue_req.instr,
        data_arga : xif_if.register.rs[0],  // Assumes the register handshake is done with the instruction issue
        data_argb : xif_if.register.rs[1],
        data_argc : '0
    };
    xif_if.issue_resp ='{
        accept : spatz_issue_resp.accept,
        writeback : spatz_issue_resp.writeback,
        register_read : 1'b0, // NOTE: Spatz does not have to assert this signal because CVE2 provides the register values speculatively anyway
        ecswrite : 1'b0
    };
    xif_if.register_ready = xif_if.issue_ready; // NOTE: Spatz reads the register values during the issue handshake
    xif_if.result ='{
        id : '0,
        hartid : '0,
        data : spatz_result.data,
        rd : spatz_result.id, // NOTE: id in Spatz contains rd field from the instruction
        we : spatz_result.we,
        ecsdata : '0,
        ecswe : '0,
        exc : 1'b0,
        exccode : '0,
        err : spatz_result.error,
        dbg : 1'b0
    };
  end : glue_xif

  // Connect Spatz data memory interface to the arbiter
  spatz_mem_req_t spatz_mem_req;
  spatz_mem_rsp_t spatz_mem_rsp;
  always_comb begin
    vdata_addr = spatz_mem_req.addr;
    vdata_wdata = spatz_mem_req.data;
    vdata_we = spatz_mem_req.write;
    // TODO: handle the byte enable signal for loads in a better way. Spatz does not use it for loads (keeps it '0), 
    // but the memory model in the UVM TB requires it to be valid for both loads and stores. One of Spatz's README files
    // says: "Sub-word accesses are not allowed. Fewer words can be written by using the `strb` signal. The entire bus word is accessed during reads."
    vdata_be = vdata_we? spatz_mem_req.strb : '1;
    spatz_mem_rsp.data = vdata_rdata;
  end

  spatz #(
    .NrMemPorts         (NumMemPortsPerSpatz),
    .NumOutstandingLoads(NumSpatzOutstandingLoads),
    .RegisterRsp        (RegisterRsp),
    .spatz_mem_req_t    (spatz_mem_req_t),
    .spatz_mem_rsp_t    (spatz_mem_rsp_t),
    .spatz_issue_req_t  (acc_issue_req_t),
    .spatz_issue_rsp_t  (acc_issue_rsp_t),
    .spatz_rsp_t        (acc_rsp_t)
  ) u_spatz (
    .clk_i                   (clk_i),
    .rst_ni                  (rst_ni),
    .testmode_i              (1'b0),
    .hart_id_i               (hart_id_i),
    
    // Interface to CVE2
    .issue_valid_i           (xif_if.issue_valid),
    .issue_ready_o           (xif_if.issue_ready),
    .issue_req_i             (spatz_issue_req),
    .issue_rsp_o             (spatz_issue_resp),
    .rsp_valid_o             (xif_if.result_valid),
    .rsp_ready_i             (xif_if.result_ready),
    .rsp_o                   (spatz_result),
    
    // Data memory interface
    .spatz_mem_req_o         (spatz_mem_req),
    .spatz_mem_req_valid_o   (vdata_req),
    .spatz_mem_req_ready_i   (vdata_gnt),
    .spatz_mem_rsp_i         (spatz_mem_rsp),
    .spatz_mem_rsp_valid_i   (vdata_rvalid),
    .spatz_mem_finished_o    (/*not connected*/),
    .spatz_mem_str_finished_o(/*not connected*/),

    // Floating-point, not enabled here
    .fp_lsu_mem_req_o        (/*not connected*/),
    .fp_lsu_mem_rsp_i        ('0),
    .fpu_rnd_mode_i          (/*not connected*/),
    .fpu_fmt_mode_i          (/*not connected*/),
    .fpu_status_o            (/*not connected*/)
  );


  //////////////////////////////
  /////////// CVE2 /////////////
  //////////////////////////////
  cve2_top #(
    .MHPMCounterNum   (MHPMCounterNum),
    .MHPMCounterWidth (MHPMCounterWidth),
    .RV32E            (RV32E),
    .RV32M            (RV32M),
    .XIF              (1),
    .DmHaltAddr       (DmHaltAddr),
    .DmExceptionAddr  (DmExceptionAddr)
  ) u_cve2_top (
    .clk_i (clk_i),
    .rst_ni(rst_ni),

    .test_en_i,
    .ram_cfg_i,

    .hart_id_i,
    .boot_addr_i,

    .instr_req_o,
    .instr_gnt_i,
    .instr_rvalid_i,
    .instr_addr_o,
    .instr_rdata_i,
    .instr_err_i,

    .data_req_o       (sdata_req),
    .data_gnt_i       (sdata_gnt),
    .data_rvalid_i    (sdata_rvalid),
    .data_we_o        (sdata_we),
    .data_be_o        (sdata_be),
    .data_addr_o      (sdata_addr),
    .data_wdata_o     (sdata_wdata),
    .data_rdata_i     (sdata_rdata),
    .data_err_i       (sdata_err),

    .irq_software_i,
    .irq_timer_i,
    .irq_external_i,
    .irq_fast_i,
    .irq_nm_i,

    .debug_req_i,
    .crash_dump_o,

    // eXtension interface
    .xif_issue_if     (xif_if),
    .xif_register_if  (xif_if),
    .xif_commit_if    (xif_if),
    .xif_result_if    (xif_if),
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

endmodule