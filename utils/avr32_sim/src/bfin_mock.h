/* bfin_mock.h
   aleph-avr32 simulator

   Mock Blackfin DSP for Phase 1 simulation.

   When MOCK_BFIN is defined, bfin.c delegates all DSP queries to this module
   instead of the real SPI hardware. This allows BEES operators that depend on
   DSP parameters (param names, types, ranges) to function without real hardware.

   The mock maintains:
     - A table of ParamDesc descriptors populated by bfin_mock_load()
     - A table of ParamValue values (one per param, initialised to 0)
     - A module name string (derived from the loaded filename)

   Usage:
     Define MOCK_BFIN at compile time (e.g. -DMOCK_BFIN in CFLAGS).
     The real bfin.c wraps every function body with #ifdef MOCK_BFIN so the
     stub path still compiles when MOCK_BFIN is not set.
*/

#ifndef _BFIN_MOCK_H_
#define _BFIN_MOCK_H_

#ifdef MOCK_BFIN

#include "module_common.h"
#include "param_common.h"
#include "types.h"

/* Maximum number of parameters the mock will track. */
#define BFIN_MOCK_MAX_PARAMS 128

/* --------------------------------------------------------------------------
   bfin_mock_load
   Called instead of real SPI boot when MOCK_BFIN is set.

   'name' is the filename BEES is trying to load (e.g. "aleph-waves.ldr").
   The mock:
     1. Strips the path/extension to derive a short module name like "waves".
     2. Builds a generic set of ParamDesc entries based on hard-coded
        per-module tables (see bfin_mock.c).  If the module name is unknown,
        an empty (0-param) descriptor set is installed.
     3. Resets all param values to 0.

   Returns 1 on success, 0 on failure (unknown module still returns 1 with
   0 params so BEES can continue gracefully).
-------------------------------------------------------------------------- */
int bfin_mock_load(const char* name);

/* Return the number of parameters currently loaded. */
u32  bfin_mock_get_num_params(void);

/* Copy the descriptor at paramIdx into *pDesc.
   Does nothing if paramIdx is out of range. */
void bfin_mock_get_param_desc(u16 paramIdx, volatile ParamDesc* pDesc);

/* Copy the module name into buf (MODULE_NAME_LEN bytes, NUL-terminated). */
void bfin_mock_get_module_name(volatile char* buf);

/* Store a parameter value. idx must be < numParams. */
void bfin_mock_set_param(u8 idx, s32 val);

/* Retrieve a parameter value. Returns 0 for out-of-range idx. */
s32  bfin_mock_get_param(u8 idx);

/* Dump all param names + values to print_dbg (for test observability). */
void bfin_mock_dump_params(void);

#endif /* MOCK_BFIN */

#endif /* _BFIN_MOCK_H_ */
