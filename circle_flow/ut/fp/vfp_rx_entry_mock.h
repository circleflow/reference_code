
#include "fp/vfp_entry.h"

void VFP_ENTRY_construct(UINT8 _unit, UINT8 port,
                              VFP_CLASS_ID class_id,
                              const QUAL::PDF_RULES &rules,
                              UINT8 pri);
void VFP_ENTRY_destruct(VFP_ENTRY *instance);
