// Code adapted from a SystemVerilog file with the following credentials:
// Copyright TU Wien
// Licensed under the Solderpad Hardware License v2.1, see LICENSE.txt for details
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1


///////////////////////////////////////////////////////////////////////////////////////////
// Brief description: 
// Data arbiter for main core and vector unit. Assumes 32-bit memory interface
// Generally, the scalar/vector request is propagated to the memory, and it is acknowledged if the memory interface is ready.
// The vector requests are prioritized over the scalar requests at all times (static priority).
// A 5-bit counter is used to make sure each granted request has an associated response transaction. However, we don't have any form of request identification.
// Therefore, all transactions are assumed to be in order, and we cannot concurrently serve a vector and a scalar request.
// The scalar requests are not propagated to memory nor acknowledged if the vector unit is requesting and/or a vector access is ongoing.
// Because we can't distinguish between requests, an ongoing scalar access will stop a vector request from being propagated/acknowledged; however, when the memory completes the respnse transaction, further scalar requests will be stopped, and the vector requests will be served until all vector accesses are completed.
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

    logic ongoing_scalar_access_q; 
    logic ongoing_vector_access_q;
    logic [4:0] ongoing_access_count_q;
    
    always_comb begin
        // Memory side
        data_req_o   = (vdata_req_i && !ongoing_scalar_access_q) || (sdata_req_i && !(ongoing_vector_access_q || vdata_req_i));
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

        // Cores grants
        sdata_gnt_o = sdata_req_i && data_gnt_i && !(ongoing_vector_access_q || vdata_req_i);
        vdata_gnt_o = vdata_req_i && data_gnt_i && !ongoing_scalar_access_q;
        
        // We propagate the memory response to core with ongoing access
        // CVE2-specific: LSU expects rvalid to be asserted even for store operations. LSU also expects a response only if it has sent a request.
        sdata_rvalid_o = ongoing_scalar_access_q && data_rvalid_i;
        vdata_rvalid_o = ongoing_vector_access_q && data_rvalid_i;
        sdata_rdata_o = data_rdata_i;
        vdata_rdata_o = data_rdata_i;
        sdata_err_o = data_err_i;
        vdata_err_o = data_err_i;
    end
    
    always_ff @(posedge clk_i or negedge rst_ni) begin
        if (~rst_ni) begin
            ongoing_scalar_access_q   <= 1'b0;
            ongoing_vector_access_q   <= 1'b0;
            ongoing_access_count_q    <= 'd0;
        end else begin

            // Increment the ongoing accesses counter for each granted request and decrement it for each response.
            // If both a grant and a response happen concurrently, the counter will remain the same.
            if ((sdata_gnt_o || vdata_gnt_o) && !(sdata_rvalid_o || vdata_rvalid_o)) begin
                ongoing_access_count_q <= ongoing_access_count_q + 1'b1;
            end else if (!(sdata_gnt_o || vdata_gnt_o) && (sdata_rvalid_o || vdata_rvalid_o)) begin
                ongoing_access_count_q <= ongoing_access_count_q - 1'b1;
            end

            // The flags for ongoing scalar/vector accesse are set on the first granted request and reset only when all requests are served.
            // There cannot be concurrent scalar and vector granted requests, so it is safe to separate the 2 if statements below.
            if (sdata_gnt_o) begin
                ongoing_scalar_access_q <= 1'b1;
            end else if (ongoing_access_count_q == 'd0) begin
                ongoing_scalar_access_q <= 1'b0;
            end

            if (vdata_gnt_o) begin
                ongoing_vector_access_q <= 1'b1;
            end else if (ongoing_access_count_q == 'd0) begin
                ongoing_vector_access_q <= 1'b0;
            end
        end
    end
    
endmodule
