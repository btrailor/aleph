/* bfin_mock.c
   aleph-avr32 simulator

   Mock Blackfin DSP – Phase 1 implementation.

   See bfin_mock.h for API documentation.

   Design notes
   ============
   The real Blackfin firmware responds to SPI queries with parameter
   descriptors that it builds in its own moduleData struct at startup.
   We cannot run Blackfin ELF files on the host, so we replicate the
   descriptors here using data derived from the module params.h files.

   For each known module we provide a static table of ParamDesc entries.
   Unknown modules get 0 params so BEES can still boot and show the DSP
   page without crashing.

   The tables below are intentionally minimal – enough to make BEES operators
   functional.  Add more entries as needed following the existing pattern.

   Compile guard: entire file is compiled only when MOCK_BFIN is defined.
*/

#ifdef MOCK_BFIN

#include <string.h>
#include <stdio.h>

#include "module_common.h"
#include "param_common.h"
#include "types.h"
#include "print_funcs.h"
#include "bfin_mock.h"

/* -------------------------------------------------------------------------
   Internal state
-------------------------------------------------------------------------- */

/* Loaded module name (set by bfin_mock_load). */
static char mockModuleName[MODULE_NAME_LEN];

/* Parameter count for the currently loaded module. */
static u32 mockNumParams = 0;

/* Descriptor table. */
static ParamDesc mockParamDesc[BFIN_MOCK_MAX_PARAMS];

/* Live parameter values. */
static s32 mockParamValue[BFIN_MOCK_MAX_PARAMS];

/* -------------------------------------------------------------------------
   Convenience macro for filling one descriptor entry.
-------------------------------------------------------------------------- */
#define FILL_DESC(idx, lbl, ptype, pmin, pmax, prad) \
  do { \
    strncpy(mockParamDesc[(idx)].label, (lbl), PARAM_LABEL_LEN - 1); \
    mockParamDesc[(idx)].label[PARAM_LABEL_LEN - 1] = '\0'; \
    mockParamDesc[(idx)].type   = (ptype); \
    mockParamDesc[(idx)].min    = (pmin); \
    mockParamDesc[(idx)].max    = (pmax); \
    mockParamDesc[(idx)].radix  = (prad); \
  } while(0)

/* Handy range constants (mirror those in module params.h / osc units). */
#define AMP_MAX       (FRACT32_MAX >> 1)
#define OSC_HZ_MIN    0x040000        /* 4 Hz in 16.16 */
#define OSC_HZ_MAX    0x40000000      /* 16384 Hz */
#define OSC_HZ_RADIX  15
#define RATIO_MIN     0x4000          /* 1/4 in 16.16 */
#define RATIO_MAX     0x40000         /* 4 in 16.16 */
#define RATIO_RADIX   3
#define SLEW_MIN      0x2000          /* 1/8 */
#define SLEW_MAX      0x400000        /* 64  */
#define SLEW_RADIX    7
#define MIX_MIN       0
#define MIX_MAX       FRACT32_MAX
#define MIX_RADIX     1
#define CUT_MAX       0x7fffffff
#define RQ_MAX        0x0000ffff
#define DELAY_MAX     0x003c0000      /* ~60 s in 16.16 */
#define DELAY_RADIX   7

/* -------------------------------------------------------------------------
   Module: "waves" (dual-oscillator + SVF + I/O mix)
   Matches modules/waves/params.h eParamNumParams ordering.
-------------------------------------------------------------------------- */
static void install_waves(void) {
  u32 i = 0;

  /* Slew smoothers (processed first on hardware) */
  FILL_DESC(i++, "amp0Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "amp1Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "hz0Slew",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "hz1Slew",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "wave0Slew",   eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "wave1Slew",   eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "pm10Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "pm01Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "wm10Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "wm01Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  /* SVF smoothers */
  FILL_DESC(i++, "cut0Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "rq0Slew",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "low0Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "high0Slew",   eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "band0Slew",   eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "notch0Slew",  eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "cut1Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "rq1Slew",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "low1Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "high1Slew",   eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "band1Slew",   eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "notch1Slew",  eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "dry0Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "wet0Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "dry1Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "wet1Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "mixSlew",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  /* Osc-to-DAC output mix */
  FILL_DESC(i++, "osc0_dac0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "osc0_dac1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "osc0_dac2",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "osc0_dac3",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "osc1_dac0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "osc1_dac1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "osc1_dac2",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "osc1_dac3",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  /* ADC-to-DAC mix */
  FILL_DESC(i++, "adc0_dac0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc0_dac1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc0_dac2",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc0_dac3",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc1_dac0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc1_dac1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc1_dac2",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc1_dac3",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc2_dac0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc2_dac1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc2_dac2",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc2_dac3",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc3_dac0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc3_dac1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc3_dac2",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc3_dac3",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  /* CV */
  FILL_DESC(i++, "cvSlew3",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "cvSlew2",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "cvSlew1",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "cvSlew0",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "cvVal3",      eParamTypeFix, 0, FRACT32_MAX, 16);
  FILL_DESC(i++, "cvVal2",      eParamTypeFix, 0, FRACT32_MAX, 16);
  FILL_DESC(i++, "cvVal1",      eParamTypeFix, 0, FRACT32_MAX, 16);
  FILL_DESC(i++, "cvVal0",      eParamTypeFix, 0, FRACT32_MAX, 16);
  /* SVF filter 1 */
  FILL_DESC(i++, "cut1",        eParamTypeSvfFreq, 0, CUT_MAX, 15);
  FILL_DESC(i++, "rq1",         eParamTypeFix, 0, RQ_MAX, 1);
  FILL_DESC(i++, "low1",        eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "high1",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "band1",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "notch1",      eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "fwet1",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "fdry1",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  /* SVF filter 0 */
  FILL_DESC(i++, "cut0",        eParamTypeSvfFreq, 0, CUT_MAX, 15);
  FILL_DESC(i++, "rq0",         eParamTypeFix, 0, RQ_MAX, 1);
  FILL_DESC(i++, "low0",        eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "high0",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "band0",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "notch0",      eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "fwet0",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "fdry0",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  /* Oscillator parameters */
  FILL_DESC(i++, "fmDel0",      eParamTypeFix, 0, 0x10000, 1);
  FILL_DESC(i++, "fmDel1",      eParamTypeFix, 0, 0x10000, 1);
  FILL_DESC(i++, "bl1",         eParamTypeFix, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "bl0",         eParamTypeFix, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "wm10",        eParamTypeFix, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "wm01",        eParamTypeFix, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "pm10",        eParamTypeFix, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "pm01",        eParamTypeFix, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "wave1",       eParamTypeFix, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "wave0",       eParamTypeFix, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "amp1",        eParamTypeAmp, 0, AMP_MAX, 1);
  FILL_DESC(i++, "amp0",        eParamTypeAmp, 0, AMP_MAX, 1);
  FILL_DESC(i++, "tune1",       eParamTypeFix, RATIO_MIN, RATIO_MAX, RATIO_RADIX);
  FILL_DESC(i++, "tune0",       eParamTypeFix, RATIO_MIN, RATIO_MAX, RATIO_RADIX);
  FILL_DESC(i++, "hz1",         eParamTypeNote, OSC_HZ_MIN, OSC_HZ_MAX, OSC_HZ_RADIX);
  FILL_DESC(i++, "hz0",         eParamTypeNote, OSC_HZ_MIN, OSC_HZ_MAX, OSC_HZ_RADIX);

  mockNumParams = i;
}

/* -------------------------------------------------------------------------
   Module: "lines" (dual delay + SVF + mix matrix)
   Matches modules/lines/params.h ordering.
-------------------------------------------------------------------------- */
static void install_lines(void) {
  u32 i = 0;

  /* Crossfade time */
  FILL_DESC(i++, "fade0",       eParamTypeFix, 0x400, 0x80000, 3);
  FILL_DESC(i++, "fade1",       eParamTypeFix, 0x400, 0x80000, 3);
  /* SVF smoothers */
  FILL_DESC(i++, "cut0Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "rq0Slew",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "low0Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "high0Slew",   eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "band0Slew",   eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "notch0Slew",  eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "cut1Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "rq1Slew",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "low1Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "high1Slew",   eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "band1Slew",   eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "notch1Slew",  eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "dry0Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "wet0Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "dry1Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "wet1Slew",    eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "mixSlew",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  /* ADC-to-delay mix */
  FILL_DESC(i++, "adc0_del0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc0_del1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc1_del0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc1_del1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc2_del0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc2_del1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc3_del0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc3_del1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  /* Delay-to-DAC mix */
  FILL_DESC(i++, "del0_dac0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "del0_dac1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "del0_dac2",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "del0_dac3",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "del1_dac0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "del1_dac1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "del1_dac2",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "del1_dac3",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  /* Feedback mix */
  FILL_DESC(i++, "del0_del0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "del0_del1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "del1_del0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "del1_del1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  /* Dry ADC-to-DAC */
  FILL_DESC(i++, "adc0_dac0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc0_dac1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc0_dac2",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc0_dac3",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc1_dac0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc1_dac1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc1_dac2",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc1_dac3",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc2_dac0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc2_dac1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc2_dac2",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc2_dac3",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc3_dac0",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc3_dac1",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc3_dac2",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "adc3_dac3",   eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  /* CV */
  FILL_DESC(i++, "cvSlew3",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "cvSlew2",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "cvSlew1",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "cvSlew0",     eParamTypeIntegrator, SLEW_MIN, SLEW_MAX, SLEW_RADIX);
  FILL_DESC(i++, "cvVal3",      eParamTypeFix, 0, FRACT32_MAX, 16);
  FILL_DESC(i++, "cvVal2",      eParamTypeFix, 0, FRACT32_MAX, 16);
  FILL_DESC(i++, "cvVal1",      eParamTypeFix, 0, FRACT32_MAX, 16);
  FILL_DESC(i++, "cvVal0",      eParamTypeFix, 0, FRACT32_MAX, 16);
  /* Line 1 (delay + SVF) */
  FILL_DESC(i++, "freq1",       eParamTypeSvfFreq, 0, CUT_MAX, 15);
  FILL_DESC(i++, "rq1",         eParamTypeFix, 0, RQ_MAX, 1);
  FILL_DESC(i++, "low1",        eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "high1",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "band1",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "notch1",      eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "fwet1",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "fdry1",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "pos_read1",   eParamTypeFix, 0, DELAY_MAX, DELAY_RADIX);
  FILL_DESC(i++, "pos_write1",  eParamTypeFix, 0, DELAY_MAX, DELAY_RADIX);
  FILL_DESC(i++, "run_read1",   eParamTypeBool, 0, 1, 1);
  FILL_DESC(i++, "run_write1",  eParamTypeBool, 0, 1, 1);
  FILL_DESC(i++, "loop1",       eParamTypeFix, 0, DELAY_MAX, DELAY_RADIX);
  FILL_DESC(i++, "rMul1",       eParamTypeFix, 0x2000, 0x80000, 3);
  FILL_DESC(i++, "rDiv1",       eParamTypeFix, 0x2000, 0x80000, 3);
  FILL_DESC(i++, "pre1",        eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "write1",      eParamTypeBool, 0, 1, 1);
  FILL_DESC(i++, "delay1",      eParamTypeFix, 0, DELAY_MAX, DELAY_RADIX);
  /* Line 0 */
  FILL_DESC(i++, "freq0",       eParamTypeSvfFreq, 0, CUT_MAX, 15);
  FILL_DESC(i++, "rq0",         eParamTypeFix, 0, RQ_MAX, 1);
  FILL_DESC(i++, "low0",        eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "high0",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "band0",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "notch0",      eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "fwet0",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "fdry0",       eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "pos_read0",   eParamTypeFix, 0, DELAY_MAX, DELAY_RADIX);
  FILL_DESC(i++, "pos_write0",  eParamTypeFix, 0, DELAY_MAX, DELAY_RADIX);
  FILL_DESC(i++, "run_read0",   eParamTypeBool, 0, 1, 1);
  FILL_DESC(i++, "run_write0",  eParamTypeBool, 0, 1, 1);
  FILL_DESC(i++, "loop0",       eParamTypeFix, 0, DELAY_MAX, DELAY_RADIX);
  FILL_DESC(i++, "rMul0",       eParamTypeFix, 0x2000, 0x80000, 3);
  FILL_DESC(i++, "rDiv0",       eParamTypeFix, 0x2000, 0x80000, 3);
  FILL_DESC(i++, "pre0",        eParamTypeAmp, MIX_MIN, MIX_MAX, MIX_RADIX);
  FILL_DESC(i++, "write0",      eParamTypeBool, 0, 1, 1);
  FILL_DESC(i++, "delay0",      eParamTypeFix, 0, DELAY_MAX, DELAY_RADIX);

  mockNumParams = i;
}

/* -------------------------------------------------------------------------
   Helper: extract the short module name from a path like
   "/mod/aleph-waves.ldr" or "aleph-lines-1.2.3.ldr".
   Result: "waves", "lines", etc.  Falls back to "unknown".
-------------------------------------------------------------------------- */
static void derive_module_name(const char* path) {
  const char* p;
  const char* start;
  char buf[MODULE_NAME_LEN];
  u8 i;

  /* Find last '/' */
  start = path;
  for (p = path; *p; p++) {
    if (*p == '/' || *p == '\\') {
      start = p + 1;
    }
  }

  /* Copy basename into buf */
  strncpy(buf, start, MODULE_NAME_LEN - 1);
  buf[MODULE_NAME_LEN - 1] = '\0';

  /* Strip "aleph-" prefix if present */
  if (strncmp(buf, "aleph-", 6) == 0) {
    memmove(buf, buf + 6, strlen(buf) - 5); /* include NUL */
  }

  /* Strip extension and any version suffix: stop at '-', '.', or NUL */
  for (i = 0; buf[i]; i++) {
    if (buf[i] == '.' || buf[i] == '-') {
      buf[i] = '\0';
      break;
    }
  }

  strncpy(mockModuleName, buf, MODULE_NAME_LEN - 1);
  mockModuleName[MODULE_NAME_LEN - 1] = '\0';
}

/* -------------------------------------------------------------------------
   Public API
-------------------------------------------------------------------------- */

int bfin_mock_load(const char* name) {
  u32 i;

  /* Clear state */
  memset(mockParamDesc,  0, sizeof(mockParamDesc));
  memset(mockParamValue, 0, sizeof(mockParamValue));
  mockNumParams = 0;

  /* Derive short module name from file path */
  if (name && *name) {
    derive_module_name(name);
  } else {
    strncpy(mockModuleName, "none", MODULE_NAME_LEN);
  }

  print_dbg("\r\n [bfin_mock] loading module: ");
  print_dbg(mockModuleName);

  /* Install descriptor table for known modules */
  if (strncmp(mockModuleName, "waves", MODULE_NAME_LEN) == 0) {
    install_waves();
  } else if (strncmp(mockModuleName, "lines", MODULE_NAME_LEN) == 0) {
    install_lines();
  } else {
    /* Unknown module: 0 params, BEES will show empty DSP page. */
    print_dbg("\r\n [bfin_mock] WARNING: unknown module, installing 0 params");
    mockNumParams = 0;
  }

  /* Sanity clamp */
  if (mockNumParams > BFIN_MOCK_MAX_PARAMS) {
    print_dbg("\r\n [bfin_mock] ERROR: param count exceeds BFIN_MOCK_MAX_PARAMS, clamping");
    mockNumParams = BFIN_MOCK_MAX_PARAMS;
  }

  print_dbg("\r\n [bfin_mock] installed ");
  print_dbg_ulong((unsigned long)mockNumParams);
  print_dbg(" params");

  /* Initialise values to 0 */
  for (i = 0; i < mockNumParams; i++) {
    mockParamValue[i] = 0;
  }

  return 1;
}

u32 bfin_mock_get_num_params(void) {
  return mockNumParams;
}

void bfin_mock_get_param_desc(u16 paramIdx, volatile ParamDesc* pDesc) {
  if (paramIdx >= mockNumParams) {
    print_dbg("\r\n [bfin_mock] get_param_desc: idx out of range: ");
    print_dbg_ulong((unsigned long)paramIdx);
    return;
  }
  /* Copy field by field to satisfy volatile qualifier */
  {
    u8 i;
    for (i = 0; i < PARAM_LABEL_LEN; i++) {
      pDesc->label[i] = mockParamDesc[paramIdx].label[i];
    }
  }
  pDesc->type  = mockParamDesc[paramIdx].type;
  pDesc->min   = mockParamDesc[paramIdx].min;
  pDesc->max   = mockParamDesc[paramIdx].max;
  pDesc->radix = mockParamDesc[paramIdx].radix;
}

void bfin_mock_get_module_name(volatile char* buf) {
  u8 i;
  for (i = 0; i < MODULE_NAME_LEN; i++) {
    buf[i] = mockModuleName[i];
    if (mockModuleName[i] == '\0') { break; }
  }
}

void bfin_mock_set_param(u8 idx, s32 val) {
  if (idx >= mockNumParams) {
    print_dbg("\r\n [bfin_mock] set_param: idx out of range: ");
    print_dbg_ulong((unsigned long)idx);
    return;
  }
  mockParamValue[idx] = val;
}

s32 bfin_mock_get_param(u8 idx) {
  if (idx >= mockNumParams) {
    return 0;
  }
  return mockParamValue[idx];
}

void bfin_mock_dump_params(void) {
  u32 i;
  print_dbg("\r\n [bfin_mock] param dump: module=");
  print_dbg(mockModuleName);
  print_dbg(" numParams=");
  print_dbg_ulong((unsigned long)mockNumParams);
  for (i = 0; i < mockNumParams; i++) {
    print_dbg("\r\n  [");
    print_dbg_ulong((unsigned long)i);
    print_dbg("] ");
    print_dbg(mockParamDesc[i].label);
    print_dbg(" = 0x");
    print_dbg_hex((unsigned long)(u32)mockParamValue[i]);
  }
}

#endif /* MOCK_BFIN */
