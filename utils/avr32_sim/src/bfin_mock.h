/* bfin_mock.h
 * avr32_sim
 * aleph
 *
 * Mock Blackfin DSP for simulator builds.
 *
 * Provides static parameter descriptors and state so BEES can
 * load/query/set params without a real BF533 connected.
 *
 * Activate by defining MOCK_BFIN=1 at compile time.
 */

#ifndef _BFIN_MOCK_H_
#define _BFIN_MOCK_H_

#include "types.h"
#include "param_common.h"
#include "module_common.h"

//-------------------------------------
//--- init / lifecycle

void bfin_mock_init(void);
void bfin_mock_deinit(void);

//-------------------------------------
//--- API (mirrors real bfin.c public interface)

void bfin_mock_load_buf(void);
void bfin_mock_set_param(u8 idx, fix16_t x);
s32  bfin_mock_get_param(u8 idx);
void bfin_mock_get_num_params(volatile u32* num);
void bfin_mock_get_param_desc(u16 paramIdx, volatile ParamDesc* pDesc);
void bfin_mock_get_module_name(volatile char* buf);
void bfin_mock_get_module_version(ModuleVersion* vers);
void bfin_mock_enable(void);
void bfin_mock_disable(void);
void bfin_mock_wait(void);
void bfin_mock_wait_ready(void);
void bfin_mock_report_params(void);

#endif // _BFIN_MOCK_H_
