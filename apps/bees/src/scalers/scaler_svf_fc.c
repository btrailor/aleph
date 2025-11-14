// std
#include <string.h>
// asf
#include "print_funcs.h"
// aleph-avr32
#include "memory.h"
// bees
#include "files.h"
#include "param_scaler.h"
#include "scaler_note.h"
#include "scaler_svf_fc.h"

// table size
static const u8 tabBits = 10;
static const u32 tabSize = 1024;
// shift from io_t size to index
static const u8 inRshift = 5;

static const s32* tabVal;

static u8 initFlag = 0;

//-------------------
//--- static funcs

//-----------------------
//---- extern funcs

s32 scaler_svf_fc_val(void* scaler, io_t in) {
  /* print_dbg("\r\n requesting svf_fc_scaler value for input: 0x"); */
  /* print_dbg_hex((u32)in); */
  if(in < 0) { in = 0; }
  return tabVal[(u16)((u16)in >> inRshift)];
}

void scaler_svf_fc_str(char* dst, void* scaler, io_t in) {
  //  u16 uin = BIT_ABS_16((s16)in) >> inRshift;
  // use note scaler class for representation
  scaler_note_str(dst, scaler, in);
}

// init function
void scaler_svf_fc_init(void* scaler) {
  ParamScaler* sc = (ParamScaler*)scaler;
  print_dbg("\r\n [DEBUG] entering scaler_svf_fc_init");
  print_dbg("\r\n initializing svf_fc scaler for param, label: ");
  print_dbg(sc->desc->label);
  // check descriptor
  if( sc->desc->type != eParamTypeSvfFreq) {
    print_dbg("\r\n !!! warning: wrong param type for svf_fc scaler");
  }
  
  print_dbg("\r\n [DEBUG] about to check initFlag");
  // init flag for static data
  if(initFlag) { 
    print_dbg("\r\n [DEBUG] initFlag already set, skipping static init");
  } else {
    print_dbg("\r\n [DEBUG] setting initFlag and initializing static data");
    initFlag = 1;

    print_dbg("\r\n [DEBUG] about to call scaler_get_nv_data");
    // assign
    tabVal = scaler_get_nv_data(eParamTypeSvfFreq);
    print_dbg("\r\n [DEBUG] scaler_get_nv_data returned tabVal=");
    print_dbg_hex((u32)tabVal);
    
    // Test if we can access the first element
    if (tabVal != NULL) {
      print_dbg("\r\n [DEBUG] testing access to tabVal[0]");
      volatile s32 test_val = tabVal[0];
      print_dbg("\r\n [DEBUG] tabVal[0] = ");
      print_dbg_hex(test_val);
      print_dbg("\r\n [DEBUG] testing access to tabVal[511]");
      test_val = tabVal[511];
      print_dbg("\r\n [DEBUG] tabVal[511] = ");
      print_dbg_hex(test_val);
    }
    print_dbg("\r\n [DEBUG] scaler_get_nv_data completed successfully");
  }

  // hack:
  // use the 16.16 table from hz scaler for representation.
  // make sure note scaler is also initialized.
  //.... why doesn't this always crash??? (NULL arg)
  print_dbg("\r\n [DEBUG] about to call scaler_note_init");
  scaler_note_init(NULL);
  print_dbg("\r\n [DEBUG] scaler_note_init completed");

  print_dbg("\r\n [DEBUG] setting inMin and inMax");
  sc->inMin = 0;
  sc->inMax = (tabSize - 1) << inRshift;
  print_dbg("\r\n [DEBUG] scaler_svf_fc_init completed successfully");

  //// FIXME: add tuning functions (???)
  //  sc->tune = NULL;
  //  sc->numTune = 0;  
}

// get input given DSP value (use sparingly)
io_t scaler_svf_fc_in(void* scaler, s32 x) {
  print_dbg("\r\n [DEBUG] entering scaler_svf_fc_in with x=");
  print_dbg_hex(x);
  print_dbg("\r\n [DEBUG] checking if tabVal is initialized");
  if (tabVal == NULL) {
    print_dbg("\r\n [DEBUG] ERROR: tabVal is NULL, initializer never called!");
    return 0;
  }
  print_dbg("\r\n [DEBUG] tabVal is initialized, proceeding with binary search");
  print_dbg("\r\n [DEBUG] tabSize = ");
  print_dbg_ulong(tabSize);
  
  // value table is monotonic, can binary search
  s32 jl = 0;
  s32 ju = tabSize - 1;
  s32 jm = 0;

  print_dbg("\r\n [DEBUG] jl=");
  print_dbg_ulong(jl);
  print_dbg(", ju=");
  print_dbg_ulong(ju);

  print_dbg("\r\n scaler_svf_fc_in, x: 0x");
  print_dbg_hex(x);

  // binary tree search
  s32 iterations = 0;
  while(ju - jl > 1) {
    iterations++;
    if (iterations > 20) {
      print_dbg("\r\n [DEBUG] ERROR: binary search stuck in infinite loop!");
      return 0;
    }
    jm = (ju + jl) >> 1;
    print_dbg("\r\n [DEBUG] iteration ");
    print_dbg_ulong(iterations);
    print_dbg(", jm=");
    print_dbg_ulong(jm);
    
    if (jm >= tabSize) {
      print_dbg("\r\n [DEBUG] ERROR: jm >= tabSize, array bounds violation!");
      return 0;
    }
    
    // value table is always ascending
    if(x >= tabVal[jm]) {
      jl = jm;
    } else {
      ju = jm;
    }
    print_dbg(", new jl=");
    print_dbg_ulong(jl);
    print_dbg(", ju=");
    print_dbg_ulong(ju);
  }

  return (u16)jm << inRshift;
}


// increment input by pointer, return value
s32 scaler_svf_fc_inc(void* scaler, io_t* pin, io_t inc ) {
  ParamScaler* sc = (ParamScaler*)scaler;
  // this speeds up the knob a great deal.
#if 0
  s32 sinc;
  // scale up to smallest significant abscissa:
  // check for 16b overflow
  sinc = (s32)inc << inRshift;
  if(sinc > 0x3fff) { 
    inc = (io_t)0x3fff;
  } 
  else if (sinc < (s32)0xffffc000) { 
    inc = (io_t)0xc000;
  }
#endif
  
  // use saturation
  *pin = op_sadd(*pin, inc);

  if(*pin < sc->inMin) { *pin = sc->inMin; }
  if(*pin > sc->inMax) { *pin = sc->inMax; }

  // scale and return.
  // ignoring ranges in descriptor at least for now.
  return scaler_svf_fc_val(sc, *pin);
}
