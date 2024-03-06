#include "uintN_t.h"
#include "fir_interp_names.h"

// Names for base components
#define base_fir_name   fir_interp_fir_name(fir_interp_name)

// Allow user to provide stream type
// TODO default to stream(data_t etc)
#ifndef fir_interp_data_stream_t
#define fir_interp_data_stream_t fir_in_data_stream_type(base_fir_name)
#endif
#ifndef fir_interp_out_stream_t
#define fir_interp_out_stream_t fir_out_data_stream_type(base_fir_name)
#endif

// Declare FIR for after inserting zeros to smooth interpolate
// Base FIR instance
#define fir_name        base_fir_name
#define FIR_N_TAPS      FIR_INTERP_N_TAPS
#define FIR_LOG2_N_TAPS FIR_INTERP_LOG2_N_TAPS
#define fir_data_t      fir_interp_data_t
#define fir_data_stream_t fir_interp_data_stream_t
#define fir_coeff_t     fir_interp_coeff_t
#define fir_accum_t     fir_interp_accum_t
#define fir_out_t       fir_interp_out_t
#define fir_out_stream_t fir_interp_out_stream_t
#define FIR_OUT_SCALE   FIR_INTERP_OUT_SCALE
#define FIR_POW2_DN_SCALE  FIR_INTERP_POW2_DN_SCALE
#define FIR_COEFFS      FIR_INTERP_COEFFS
#include "dsp/fir.h"

// Insert zeros
#define fir_interp_insert_n_zeros fir_interp_insert_n_zeros_func(fir_interp_name)
PRAGMA_MESSAGE(FUNC_WIRES fir_interp_insert_n_zeros) // Not worried about delay of this func
fir_interp_data_stream_t fir_interp_insert_n_zeros(fir_interp_data_stream_t in_sample){
  static uint8_t zero_counter;  // TODO size counter?
  fir_interp_data_stream_t o;
  o.valid = 0;
  if(in_sample.valid){
    // restart zeros
    zero_counter = FIR_INTERP_FACTOR - 1;
    // output real original sample pulse
    o.valid = 1;
    o.data = in_sample.data;
  }else{
    // output zeros
    if(zero_counter > 0){
      o.valid = 1;
      o.data = 0;
      zero_counter -= 1;
    }
  }
  return o;
}

// The FIR pipeline
fir_interp_out_stream_t fir_interp_name(fir_interp_data_stream_t in_sample){
  // Insert zeros between samples per interp ratio
  fir_interp_data_stream_t samples_w_zeros = fir_interp_insert_n_zeros(in_sample);

  // Smooth the samples pulses with FIR
  fir_interp_out_stream_t interp_out = base_fir_name(samples_w_zeros);

  return interp_out;
}

#undef base_fir_name
#undef fir_interp_name
#undef FIR_INTERP_N_TAPS
#undef FIR_INTERP_LOG2_N_TAPS
#undef FIR_INTERP_FACTOR
#undef fir_interp_data_t
#undef fir_interp_coeff_t
#undef fir_interp_accum_t
#undef fir_interp_out_t
#ifdef FIR_INTERP_OUT_SCALE
#undef FIR_INTERP_OUT_SCALE
#endif
#undef FIR_INTERP_POW2_DN_SCALE
#undef FIR_INTERP_COEFFS
#undef fir_interp_insert_n_zeros
#undef fir_interp_data_t
#undef fir_interp_out_stream_t
