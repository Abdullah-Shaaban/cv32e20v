// Code adapted from a SystemVerilog file with the following credentials:
// Copyright TU Wien
// Licensed under the Solderpad Hardware License v2.1, see LICENSE.txt for details
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1


///////////////////////////////////////////////////////////////////////////////////////////
////// Data arbiter for main core and vector unit. Assumes 32-bit memory interface ////////
///////////////////////////////////////////////////////////////////////////////////////////
module data_mem_arbiter (
    input logic clk_i, 
    input logic rst_ni,
    
    // Scalar interface
    input logic             sdata_req_i,
    input logic             sdata_we_i, 
    input logic [3:0]       sdata_be_i,
    input logic [31:0]      sdata_addr_i,
    input logic [31:0]      sdata_wdata_i,
    output logic            sdata_gnt_o, 
    output logic            sdata_rvalid_o, 
    output logic            sdata_err_o,
    output logic [31:0]     sdata_rdata_o,

    // Vector interface
    input logic             vdata_we_i,
    input logic [3:0]       vdata_be_i,
    input logic [31:0]      vdata_addr_i,
    input logic [31:0]      vdata_wdata_i,
    input logic             vdata_req_i,
    input logic             vect_pending_store_i, 
    input logic             vect_pending_load_i,
    output logic            vdata_gnt_o, 
    output logic            vdata_rvalid_o, 
    output logic            vdata_err_o,
    output logic [31:0]     vdata_rdata_o,

    // Memory interface
    input logic             data_gnt_i,
    input logic             data_rvalid_i,
    input logic             data_err_i,
    input logic  [31:0]     data_rdata_i,
    output logic            data_req_o,
    output logic            data_we_o,
    output logic [31:0]     data_addr_o,
    output logic [3:0]      data_be_o,
    output logic [31:0]     data_wdata_o
);

    // Favor the vector core. Block the scalar core's requests
    logic sdata_hold;
    // TODO: get the equivalent of vect_pending_store and vect_pending_load from Spatz
    assign sdata_hold = (vdata_req_i | vect_pending_store_i | (vect_pending_load_i & sdata_we_i));

    // Data request and assignment logic
    always_comb begin
        data_req_o   = vdata_req_i | (sdata_req_i & ~sdata_hold);
        data_addr_o  = sdata_addr_i;
        data_we_o    = sdata_we_i;
        data_be_o    = sdata_be_i;
        data_wdata_o = sdata_wdata_i;
        if (vdata_req_i) begin
            data_addr_o  = vdata_addr_i;
            data_we_o    = vdata_we_i;
            data_be_o    = vdata_be_i;
            data_wdata_o = vdata_wdata_i;
        end
    end

    // Grant assignment for scalar and vector cores
    assign sdata_gnt_o = data_gnt_i && sdata_req_i && ~sdata_hold;
    assign vdata_gnt_o = data_gnt_i && vdata_req_i;

    // Remember the granted core
    logic sdata_waiting; 
    logic vdata_waiting;
    always_ff @(posedge clk_i or negedge rst_ni) begin
        if (~rst_ni) begin
            sdata_waiting   <= 1'b0;
            vdata_waiting   <= 1'b0;
        end else begin
            if (sdata_gnt_o) sdata_waiting <= 1'b1;
            else if (sdata_rvalid_o) sdata_waiting <= 1'b0;

            if (vdata_gnt_o) vdata_waiting <= 1'b1;
            else if (vdata_rvalid_o) vdata_waiting <= 1'b0;
        end
    end

    // Valid assertion for the granted core
    assign sdata_rvalid_o = sdata_waiting && data_rvalid_i;
    assign vdata_rvalid_o = vdata_waiting && data_rvalid_i;

    // Data propagation
    assign sdata_rdata_o  = data_rdata_i;
    assign vdata_rdata_o  = data_rdata_i;

    // Error propagation
    assign sdata_err_o    = data_err_i;
    assign vdata_err_o    = data_err_i;
    
endmodule
