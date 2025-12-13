// Auto-generated descriptor lookup table
// DO NOT EDIT - regenerate with convert_descriptors.py

#include "types.h"
#include "embedded_descriptors.h"
#include <string.h>


struct _DescEntry {
  const char* name;
  const u8* data;
  u32 size;
};

static const struct _DescEntry _desc_entries[] = {
  {"aleph-analyser", desc_aleph_analyser, 292},
  {"aleph-dacs", desc_aleph_dacs, 260},
  {"aleph-dsyn", desc_aleph_dsyn, 3716},
  {"aleph-fmsynth", desc_aleph_fmsynth, 2212},
  {"aleph-grains", desc_aleph_grains, 2692},
  {"aleph-lines", desc_aleph_lines, 3172},
  {"aleph-mix", desc_aleph_mix, 516},
  {"aleph-monosynth", desc_aleph_monosynth, 388},
  {"aleph-tape", desc_aleph_tape, 260},
  {"aleph-varilines", desc_aleph_varilines, 3172},
  {"aleph-voder", desc_aleph_voder, 708},
  {"aleph-waves", desc_aleph_waves, 2372},
  {NULL, NULL, 0}
};

const u8* embedded_desc_find(const char* module_name, u32* out_size) {
  if (module_name == NULL) {
    return NULL;
  }
  
  for (int i = 0; _desc_entries[i].name != NULL; ++i) {
    if (strcmp(_desc_entries[i].name, module_name) == 0) {
      if (out_size != NULL) {
        *out_size = _desc_entries[i].size;
      }
      return _desc_entries[i].data;
    }
  }
  
  return NULL;
}
