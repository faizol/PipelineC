
#pragma PART "xc7a100tcsg324-1"
#include "intN_t.h"
#include "uintN_t.h"
#include "arrays.h"
#include "examples/cordic.h"

typedef struct float2{
  float data[2];
}float2;

typedef struct float3{
  float data[3];
}float3;

/*```py
def unified(p0, p1, fi, T):
    #Input
    #P0 = [x, y]
    #P1 = [y, y]
    #FI = x
    #T = [x, y]
    #Returns
    #h = [
    #        [a, b, c],
    #        [d, e, f],
    #        [g, h, i]
    #    ]
    #b = [a, b, c]
    def u(p0, p1, fi, T):
        sin_fi = np.sin(fi)
        cos_fi = np.cos(fi)
        R = np.array((
            [cos_fi, -sin_fi],
            [sin_fi, cos_fi]
        ))
        sin_fi_p10 = sin_fi * p1[0]
        cos_fi_p11 = cos_fi * p1[1]
        cos_fi_p10 = cos_fi * p1[0]
        sin_fi_p11 = sin_fi * p1[1]
        n_sin_fi_p10_n_fi_p11 = -sin_fi_p10 - cos_fi_p11
        cos_fi_p10_n_sin_fi_p11 = cos_fi_p10 - sin_fi_p11

        p1_rotated = np.array([R[0,0]*p1[0]+R[0,1]*p1[1], R[1,0]*p1[0]+R[1,1]*p1[1]])
        e = np.array([p1_rotated[0]+T[0]-p0[0], p1_rotated[1]+T[1]-p0[1]])

        h = np.array([
            [1, 0, n_sin_fi_p10_n_fi_p11],
            [0, 1, cos_fi_p10_n_sin_fi_p11],
            [n_sin_fi_p10_n_fi_p11, cos_fi_p10_n_sin_fi_p11, n_sin_fi_p10_n_fi_p11**2 + cos_fi_p10_n_sin_fi_p11**2]
        ])
        b = np.array([e[0], e[1], n_sin_fi_p10_n_fi_p11*e[0]+cos_fi_p10_n_sin_fi_p11*e[1]])
        return h, b
    h, b = u(p0, p1, fi, T)
    return h, b
```*/

/*    #Input
    #P0 = [x, y]
    #P1 = [y, y]
    #FI = x
    #T = [x, y]
    #Returns
    #h = [
    #        [a, b, c],
    #        [d, e, f],
    #        [g, h, i]
    #    ]
    #b = [a, b, c]*/
//#pragma MAIN unified
typedef struct unified_out_t{
  float h[3][3];
  float3 b;
  uint1_t valid;
}unified_out_t;
unified_out_t unified(
  float2 p0, float2 p1, float fi, float2 T,
  uint1_t valid
){
  //float sin_fi = sin(fi);
  //float cos_fi = cos(fi);
  cordic_float_t cordic = cordic_float_fixed32_n32(fi);
  float sin_fi = cordic.s;
  float cos_fi = cordic.c;
  float R[2][2];
  R[0][0] = cos_fi;
  R[0][1] = -sin_fi;
  R[1][0] = sin_fi;
  R[1][1] = cos_fi;
  float sin_fi_p10 = sin_fi * p1.data[0];
  float cos_fi_p11 = cos_fi * p1.data[1];
  float cos_fi_p10 = cos_fi * p1.data[0];
  float sin_fi_p11 = sin_fi * p1.data[1];
  float n_sin_fi_p10_n_fi_p11 = -sin_fi_p10 - cos_fi_p11;
  float cos_fi_p10_n_sin_fi_p11 = cos_fi_p10 - sin_fi_p11;
  float p1_rotated[2];
  p1_rotated[0] = R[0][0]*p1.data[0]+R[0][1]*p1.data[1];
  p1_rotated[1] = R[1][0]*p1.data[0]+R[1][1]*p1.data[1];
  float e[2];
  e[0] = p1_rotated[0]+T.data[0]-p0.data[0];
  e[1] = p1_rotated[1]+T.data[1]-p0.data[1];
  unified_out_t rv;
  rv.valid = valid;
  rv.h[0][0] = 1.0;
  rv.h[0][1] = 0.0;
  rv.h[0][2] = n_sin_fi_p10_n_fi_p11;
  rv.h[1][0] = 0.0;
  rv.h[1][1] = 1.0;
  rv.h[1][2] = cos_fi_p10_n_sin_fi_p11;
  rv.h[2][0] = n_sin_fi_p10_n_fi_p11;
  rv.h[2][1] = cos_fi_p10_n_sin_fi_p11;
  rv.h[2][2] = (n_sin_fi_p10_n_fi_p11*n_sin_fi_p10_n_fi_p11) + (cos_fi_p10_n_sin_fi_p11*cos_fi_p10_n_sin_fi_p11);
  rv.b.data[0] = e[0];
  rv.b.data[1] = e[1];
  rv.b.data[2] = n_sin_fi_p10_n_fi_p11*e[0]+cos_fi_p10_n_sin_fi_p11*e[1];
  return rv;
}

// Globally visible pipeline instances
// TODO convert to shared resource bus pipeline instance
//  1 host 1 dev but the flow control fifos etc will be needed

// Unified pipeline
uint1_t unified_pipeline_in_valid;
float2 unified_pipeline_in_p0;
float2 unified_pipeline_in_p1;
uint1_t unified_pipeline_ready_for_in;
float unified_pipeline_in_fi;
float2 unified_pipeline_in_T;
unified_out_t unified_pipeline_out;
uint1_t unified_pipeline_ready_for_out;
#pragma MAIN_MHZ unified_instance 50.0
void unified_instance()
{
  unified_pipeline_ready_for_in = 1;
  // TODO use unified_pipeline_ready_for_out

  // Use input and output registers for now
  // so unpipelined sim shows some 2 cycle delay for calc
  static unified_out_t unified_pipeline_out_r;
  static uint1_t unified_pipeline_in_valid_r;
  static float2 unified_pipeline_in_p0_r;
  static float2 unified_pipeline_in_p1_r;
  static float unified_pipeline_in_fi_r;
  static float2 unified_pipeline_in_T_r;
  // OUT REG
  unified_pipeline_out = unified_pipeline_out_r;

  unified_pipeline_out_r = unified(
    unified_pipeline_in_p0_r, unified_pipeline_in_p1_r,
    unified_pipeline_in_fi_r, unified_pipeline_in_T_r,
    unified_pipeline_in_valid_r
  );

  // IN REG
  unified_pipeline_in_p0_r = unified_pipeline_in_p0;
  unified_pipeline_in_p1_r = unified_pipeline_in_p1;
  unified_pipeline_in_fi_r =  unified_pipeline_in_fi;
  unified_pipeline_in_T_r = unified_pipeline_in_T;
  unified_pipeline_in_valid_r = unified_pipeline_in_valid;
}

// TODO parameterize by N points at once read from RAM being processed instead of 1

// Globally visible RAMs
// TODO how to init AXI DDR RAM? Needs runtime load?
//#include "ram_init.h"
#define N_POINTS 128

// Starting with one AXI BRAM shared to look like multiple independent RAMs
#define SHARED_AXI_RAM_DEPTH 1024 // 1024 u32's
#define SHARED_AXI_RAM_NUM_HOST_PORTS 2
#define SHARED_AXI_RAM_HOST_CLK_MHZ 50.0
#define SHARED_AXI_RAM_N_DEV_PORTS 1 // BRAM ports
#define SHARED_AXI_RAM_DEV_CLK_MHZ 50.0
#include "examples/shared_resource_bus/shared_axi_bram.c"

// Logic for how to start and finish a read from RAM
// (generic already, not type specific, just need sizeof(type))
typedef struct axi_shared_bus_sized_read_start_t{
  uint1_t ready_for_inputs;
  uint1_t done;
  axi_shared_bus_t_read_req_t bus_req;
}axi_shared_bus_sized_read_start_t;
axi_shared_bus_sized_read_start_t axi_shared_bus_sized_read_start(
  uint1_t rd_valid,
  uint32_t rd_index, // Not to be confused with axi addr
  uint32_t rd_size,
  uint1_t ready_for_outputs,
  uint32_t bus_base_addr, 
  uint1_t bus_req_ready
){
  // Begin N u32 reads using bus helper FSM
  uint8_t NUM_WORDS = rd_size >> 2; // / sizeof(uint32_t);
  static uint8_t word_counter = 0xFF; //max rd_size / sizeof(uint32_t);
  static uint32_t bus_addr;
  axi_shared_bus_sized_read_start_t rv;
  rv.ready_for_inputs = 0;
  rv.done = 0;
  if(ready_for_outputs)
  {
    // Active state
    if(word_counter < NUM_WORDS){
      // Try to start a u32 read
      axi_read_req_t req = axi_addr_to_read(bus_addr);
      axi_shared_bus_t_read_start_logic_outputs_t read_start = 
        axi_shared_bus_t_read_start_logic(
          req,
          1,
          bus_req_ready
        );
      rv.bus_req = read_start.req;
      // Move to next u32 when done
      if(read_start.done){
        // Last word?
        if(word_counter==(NUM_WORDS-1)){
          rv.done = 1;
        }      
        bus_addr += sizeof(uint32_t);
        word_counter += 1;
      }
    }else{
      // IDLE state, wait for valid input
      rv.ready_for_inputs = 1;
      if(rd_valid){
        word_counter = 0;
        bus_addr = bus_base_addr + (rd_index * rd_size);
      }
    }
  }
  return rv;
}


// p0,p1
typedef struct points_t{
  float2 p[2];
}points_t;
#include "points_t_bytes_t.h"
typedef struct p0p1_ram_read_finish_t{
  points_t data;
  uint1_t done;
  uint1_t bus_data_ready;
}p0p1_ram_read_finish_t;
p0p1_ram_read_finish_t p0p1_ram_axi_shared_bus_read_finish(
  uint1_t ready_for_outputs,
  axi_shared_bus_t_read_data_t bus_read_data
){
  // End N u32 reads using bus helper FSM
  // Assemble read responses into output data
  static uint8_t word_counter;
  static uint8_t bytes[sizeof(points_t)];
  static uint1_t done;
  uint8_t NUM_WORDS = sizeof(points_t) / sizeof(uint32_t);
  p0p1_ram_read_finish_t rv;
  rv.done = done;
  rv.data = bytes_to_points_t(bytes);
  rv.bus_data_ready = 0;
  // Active state
  if(word_counter < NUM_WORDS){
    // Try to finish a u32 read
    axi_shared_bus_t_read_finish_logic_outputs_t read_finish = 
      axi_shared_bus_t_read_finish_logic(
        1,
        bus_read_data
      );
    rv.bus_data_ready = read_finish.data_ready;
    if(read_finish.done){
      // Put u32 data into byte buffer
      uint32_t rd_data = axi_read_to_data(read_finish.data);
      uint8_t rd_bytes[sizeof(uint32_t)];
      uint32_t i;
      for(i = 0; i < sizeof(uint32_t); i+=1)
      {
        rd_bytes[i] = rd_data >> (i*8);
      }
      ARRAY_SHIFT_INTO_TOP(bytes, sizeof(points_t), rd_bytes, sizeof(uint32_t))
      // Last word?
      if(word_counter==(NUM_WORDS-1)){
        done = 1;
      }      
      word_counter += 1;
    }
  }else{
    // IDLE/DONE state
    // Ready for output clears done
    if(ready_for_outputs){
      done = 0;
    }
    // Restart resets word counter
    if(~done){
      word_counter = 0;
    }
  }
  return rv;
}
uint32_t p0p1_ram_in_addr;
uint1_t p0p1_ram_in_valid;
uint1_t p0p1_ram_ready_for_in;
points_t p0p1_ram_out_data;
uint1_t p0p1_ram_out_valid;
uint1_t p0p1_ram_ready_for_out;
#pragma MAIN p0p1_ram_ctrl
void p0p1_ram_ctrl()
{
  // Streams of reads starting and finishing at same time

  // Start
  axi_shared_bus_sized_read_start_t read_start = axi_shared_bus_sized_read_start(
    p0p1_ram_in_valid,
    p0p1_ram_in_addr,
    sizeof(points_t),
    1,
    0, // TODO BASE ADDR
    shared_axi_ram_dev_to_host_wire_on_host_clk.read.req_ready
  );
  p0p1_ram_ready_for_in = read_start.ready_for_inputs;
  shared_axi_ram_host_to_dev_wire_on_host_clk.read.req = read_start.bus_req;

  // Finish
  p0p1_ram_read_finish_t read_finish = p0p1_ram_axi_shared_bus_read_finish(
    p0p1_ram_ready_for_out,
    shared_axi_ram_dev_to_host_wire_on_host_clk.read.data
  );
  p0p1_ram_out_data = read_finish.data;
  p0p1_ram_out_valid = read_finish.done;
  shared_axi_ram_host_to_dev_wire_on_host_clk.read.data_ready = read_finish.bus_data_ready; 
}

// fi, T
typedef struct fi_T_t{
  float fi;
  float2 T;
}fi_T_t;
#include "fi_T_t_bytes_t.h"
typedef struct fit_ram_read_finish_t{
  fi_T_t data;
  uint1_t done;
  uint1_t bus_data_ready;
}fit_ram_read_finish_t;
fit_ram_read_finish_t fit_ram_axi_shared_bus_read_finish(
  uint1_t ready_for_outputs,
  axi_shared_bus_t_read_data_t bus_read_data
){
  // End N u32 reads using bus helper FSM
  // Assemble read responses into output data
  static uint8_t word_counter;
  static uint8_t bytes[sizeof(fi_T_t)];
  static uint1_t done;
  uint8_t NUM_WORDS = sizeof(fi_T_t) / sizeof(uint32_t);
  fit_ram_read_finish_t rv;
  rv.done = done;
  rv.data = bytes_to_fi_T_t(bytes);
  rv.bus_data_ready = 0;
  // Active state
  if(word_counter < NUM_WORDS){
    // Try to finish a u32 read
    axi_shared_bus_t_read_finish_logic_outputs_t read_finish = 
      axi_shared_bus_t_read_finish_logic(
        1,
        bus_read_data
      );
    rv.bus_data_ready = read_finish.data_ready;
    if(read_finish.done){
      // Put u32 data into byte buffer
      uint32_t rd_data = axi_read_to_data(read_finish.data);
      uint8_t rd_bytes[sizeof(uint32_t)];
      uint32_t i;
      for(i = 0; i < sizeof(uint32_t); i+=1)
      {
        rd_bytes[i] = rd_data >> (i*8);
      }
      ARRAY_SHIFT_INTO_TOP(bytes, sizeof(fi_T_t), rd_bytes, sizeof(uint32_t))
      // Last word?
      if(word_counter==(NUM_WORDS-1)){
        done = 1;
      }      
      word_counter += 1;
    }
  }else{
    // IDLE/DONE state
    // Ready for output clears done
    if(ready_for_outputs){
      done = 0;
    }
    // Restart resets word counter
    if(~done){
      word_counter = 0;
    }
  }
  return rv;
}
uint32_t fit_ram_in_addr;
uint1_t fit_ram_in_valid;
uint1_t fit_ram_ready_for_in;
uint1_t fit_ram_out_valid;
uint1_t fit_ram_ready_for_out;
fi_T_t fit_ram_out_data;
#pragma MAIN fit_ram_ctrl
void fit_ram_ctrl()
{
  // Streams of reads starting and finishing at same time

  // Start
  axi_shared_bus_sized_read_start_t read_start = axi_shared_bus_sized_read_start(
    fit_ram_in_valid,
    fit_ram_in_addr,
    sizeof(fi_T_t),
    1,
    0, // TODO BASE ADDR
    shared_axi_ram_dev_to_host_wire_on_host_clk.read.req_ready
  );
  fit_ram_ready_for_in = read_start.ready_for_inputs;
  shared_axi_ram_host_to_dev_wire_on_host_clk.read.req = read_start.bus_req;

  // Finish
  fit_ram_read_finish_t read_finish = fit_ram_axi_shared_bus_read_finish(
    fit_ram_ready_for_out,
    shared_axi_ram_dev_to_host_wire_on_host_clk.read.data
  );
  fit_ram_out_data = read_finish.data;
  fit_ram_out_valid = read_finish.done;
  shared_axi_ram_host_to_dev_wire_on_host_clk.read.data_ready = read_finish.bus_data_ready; 
}

// Globally visible instances
// TODO add valid+ready interface

// H,B accumulator
unified_out_t hb_accum_in_data;
uint1_t hb_accum_in_last;
uint1_t hb_accum_ready_for_in;
unified_out_t hb_accum_out_data;
uint1_t hb_accum_out_done;
uint1_t hb_accum_ready_for_out;
#pragma MAIN hb_accum
void hb_accum()
{
  static unified_out_t out_reg;
  hb_accum_out_data = out_reg;
  hb_accum_out_done = out_reg.valid;
  out_reg.valid = 0;
  hb_accum_ready_for_in = 1;
  // TODO use hb_accum_ready_for_out
  if(hb_accum_in_data.valid){
    // Do accumulate // TODO right logic?
    uint32_t i;
    for (i = 0; i < 3; i+=1)
    {
      out_reg.b.data[i] += hb_accum_in_data.b.data[i];
      uint32_t j;
      for (j = 0; j < 3; j+=1)
      {
        out_reg.h[i][j] += hb_accum_in_data.h[i][j];
      }
    }
    if(hb_accum_in_last){
      out_reg.valid = 1;
    }
  }
}


typedef enum state_t{
  DO_UNIFIED,
  DONE
}state_t;
#pragma MAIN control_fsm
unified_out_t control_fsm() // Output some data so doesnt synthesize away
{
  // State regs
  static state_t state; // FSM state
  static uint32_t address; // For reading from RAMs
  static uint32_t unified_in_counter; // For sim debug
  static uint32_t hb_accum_in_counter; // For sim debug and signal of done accumulating
  static uint32_t cycle_counter; // For sim debug

  // Default no valid data flowing into anywhere
  p0p1_ram_in_valid = 0;
  fit_ram_in_valid = 0;
  unified_pipeline_in_valid = 0;
  hb_accum_in_data.valid = 0;

  // The state machine logic
  if(state==DO_UNIFIED)
  {
    // Stream many P0,P1,FI,T through unified pipeline into H,B accumulator
    // Simultaneously begin reads from P0P1 ram and FI,T RAM
    // Drive incrementing addr and read enable into rams
    uint1_t valid = address<N_POINTS;
    p0p1_ram_in_addr = address;
    fit_ram_in_addr = address;
    if(p0p1_ram_ready_for_in & fit_ram_ready_for_in){
      p0p1_ram_in_valid = valid;
      fit_ram_in_valid = valid;
      if(valid){
        printf("Cycle %d: Starting read of P0,P1,FI,T [%d]...\n", cycle_counter, address); 
        address += 1;
      }
    }

    // At the same time receive output from reads from RAM some cycles delayed
    // combine read stream into a unified pipeline input stream
    unified_pipeline_in_p0 = p0p1_ram_out_data.p[0];
    unified_pipeline_in_p1 = p0p1_ram_out_data.p[1];
    unified_pipeline_in_fi = fit_ram_out_data.fi;
    unified_pipeline_in_T = fit_ram_out_data.T;
    p0p1_ram_ready_for_out = 0;
    fit_ram_ready_for_out = 0;
    if((p0p1_ram_out_valid & fit_ram_out_valid) & unified_pipeline_ready_for_in)
    {
      p0p1_ram_ready_for_out = 1;
      fit_ram_ready_for_out = 1;
      unified_pipeline_in_valid = 1;
      printf("Cycle %d: P0,P1,FI,T [%d] read data into unified pipeline...\n", cycle_counter, unified_in_counter); 
      unified_in_counter += 1;
    }

    // Outputs from unified pipeline flow into hb accum
    hb_accum_in_data.valid = unified_pipeline_out.valid;
    hb_accum_in_last = hb_accum_in_counter == (N_POINTS-1);
    hb_accum_in_data.b = unified_pipeline_out.b;
    hb_accum_in_data.h = unified_pipeline_out.h;
    unified_pipeline_ready_for_out = hb_accum_ready_for_in;
    if(unified_pipeline_out.valid & hb_accum_ready_for_in){
      printf("Cycle %d: unified pipeline output for P0,P1,FI,T [%d] into H,B accumulator...\n", cycle_counter, hb_accum_in_counter); 
      hb_accum_in_counter += 1;
    }

    // Always ready for hb accum output
    hb_accum_ready_for_out = 1;
    // Last element out of unified pipeline produces accumulator done flag
    if(hb_accum_out_done){
      printf("Cycle %d: All points done. Result ready.\n", cycle_counter);
      state = DONE;
    }
  }

  cycle_counter += 1; // For sim printfs
  return unified_pipeline_out; // For not synth away
}