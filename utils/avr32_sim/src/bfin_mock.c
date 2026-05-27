/* bfin_mock.c
 * avr32_sim
 * aleph
 *
 * Mock Blackfin DSP for simulator builds.
 *
 * Hardcodes a simple 4-parameter module descriptor so BEES
 * can load scenes and exercise operators that depend on DSP params.
 *
 * No binary .dsc parsing — static descriptor table for rapid testing.
 */

#include <string.h>
#include "bfin_mock.h"
#include "print_funcs.h"
#include "module_common.h"

//-------------------------------------
//--- static descriptor table

#define MOCK_NUM_PARAMS 4

static ParamDesc s_mockDesc[MOCK_NUM_PARAMS] = {
  {
    .label = "param_0",
    .type = eParamTypeFix,
    .min = 0,
    .max = 0x00010000,  // fix16 1.0
    .radix = 16
  },
  {
    .label = "param_1",
    .type = eParamTypeFix,
    .min = 0,
    .max = 0x00010000,
    .radix = 16
  },
  {
    .label = "param_2",
    .type = eParamTypeFix,
    .min = 0,
    .max = 0x00010000,
    .radix = 16
  },
  {
    .label = "param_3",
    .type = eParamTypeFix,
    .min = 0,
    .max = 0x00010000,
    .radix = 16
  }
};

static fix16_t s_mockValues[MOCK_NUM_PARAMS] = { 0, 0, 0, 0 };
static char    s_mockName[MODULE_NAME_LEN] = "mock";
static u8      s_mockLoaded = 0;

//-------------------------------------
//--- lifecycle

void bfin_mock_init(void) {
  s_mockLoaded = 0;
  memset(s_mockValues, 0, sizeof(s_mockValues));
  print_dbg("\r\n [bfin_mock] init");
}

void bfin_mock_deinit(void) {
  s_mockLoaded = 0;
}

//-------------------------------------
//--- public API

void bfin_mock_load_buf(void) {
  s_mockLoaded = 1;
  print_dbg("\r\n [bfin_mock] load_buf (hardcoded 'mock' module, ");
  print_dbg_ulong(MOCK_NUM_PARAMS);
  print_dbg(" params)");
}

void bfin_mock_set_param(u8 idx, fix16_t x) {
  if (idx < MOCK_NUM_PARAMS) {
    s_mockValues[idx] = x;
    print_dbg("\r\n [bfin_mock] set_param idx=");
    print_dbg_ulong(idx);
    print_dbg(" val=0x");
    print_dbg_hex((u32)x);
  }
}

s32 bfin_mock_get_param(u8 idx) {
  if (idx < MOCK_NUM_PARAMS) {
    return (s32)s_mockValues[idx];
  }
  return 0;
}

void bfin_mock_get_num_params(volatile u32* num) {
  *num = s_mockLoaded ? MOCK_NUM_PARAMS : 0;
}

void bfin_mock_get_param_desc(u16 paramIdx, volatile ParamDesc* pDesc) {
  if (paramIdx < MOCK_NUM_PARAMS) {
    memcpy((void*)pDesc, &s_mockDesc[paramIdx], sizeof(ParamDesc));
  }
}

void bfin_mock_get_module_name(volatile char* buf) {
  memcpy((void*)buf, s_mockName, MODULE_NAME_LEN);
}

void bfin_mock_get_module_version(ModuleVersion* vers) {
  vers->maj = 0;
  vers->min = 1;
  vers->rev = 0;
}

void bfin_mock_enable(void) {
  print_dbg("\r\n [bfin_mock] enable");
}

void bfin_mock_disable(void) {
  print_dbg("\r\n [bfin_mock] disable");
}

void bfin_mock_wait(void) {
  // no-op: don't wait for hardware busy pin
}

void bfin_mock_wait_ready(void) {
  // no-op
}

void bfin_mock_report_params(void) {
  u8 i;
  print_dbg("\r\n [bfin_mock] params:");
  for (i = 0; i < MOCK_NUM_PARAMS; i++) {
    print_dbg("\r\n  ");
    print_dbg_ulong(i);
    print_dbg(": ");
    print_dbg_hex((u32)s_mockValues[i]);
  }
}
