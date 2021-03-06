// Do not edit!!!!!!!
// This file was generated by MeP-Integrator

#include "mep.h"

#include "mep-core1-defs.h"
#include "mep-core1-decode.h"
#include "mep-core1-model.h"
#include "mep-cop1-16-defs.h"
#include "mep-cop1-16-decode.h"
#include "mep-cop1-16-model.h"
#include "mep-cop1-32-defs.h"
#include "mep-cop1-32-decode.h"
#include "mep-cop1-32-model.h"
#include "mep-cop1-48-defs.h"
#include "mep-cop1-48-decode.h"
#include "mep-cop1-48-model.h"
#include "mep-cop1-64-defs.h"
#include "mep-cop1-64-decode.h"
#include "mep-cop1-64-model.h"

#include "common_model.h"

using namespace mep;

namespace mep_ext1 {
  class mep_ext1_cpu: public mep_cpu_vliw<mep_ext1_cpu,mepcore1_scache,mepcop1_16_scache,mepcop1_32_scache,mepcop1_48_scache,mepcop1_64_scache>
  {
  public:
    mep_ext1_cpu() :
      mep_cpu_vliw<mep_ext1_cpu,mepcore1_scache,mepcop1_16_scache,mepcop1_32_scache,mepcop1_48_scache,mepcop1_64_scache>
      (this,ISA_EXT_CORE1,ISA_EXT_COP1_16,ISA_EXT_COP1_32,ISA_EXT_COP1_48,ISA_EXT_COP1_64),
      my_core_model (this),
      my_cop16_model (this),
      my_cop32_model (this),
      my_cop48_model (this),
      my_cop64_model (this)
    {}
    ~mep_ext1_cpu() throw() {}

    void core_model_insn_before (bool first_p = true) { my_core_model.model_insn_before (first_p); }
    void core_model_step_latency (sid::host_int_4 cycles = 0) { my_core_model.step_latency (cycles); }

    UINT model_before (mepcore1_scache* sem)
      {
        UINT stall = my_core_model.model_before (this, sem);
        if (stall) update_total_latency (stall);
        return stall;
      }

    UINT model_after (mepcore1_scache* sem) { return my_core_model.model_after (this, sem); }
    UINT model_after (mepcop1_16_scache* sem) { return my_cop16_model.model_after (this, sem); }
    UINT model_after (mepcop1_32_scache* sem) { return my_cop32_model.model_after (this, sem); }
    UINT model_after (mepcop1_48_scache* sem) { return my_cop48_model.model_after (this, sem); }
    UINT model_after (mepcop1_64_scache* sem) { return my_cop64_model.model_after (this, sem); }

    void core_model_insn_after (bool last_p = true, UINT cycles = 1) { my_core_model.model_insn_after (last_p, cycles); }

  public:
#include "ivc2.h"

  private:
    mep_common_model<mepcore1_mep_model, mep_ext1_cpu, mepcore1_idesc, mepcore1_scache> my_core_model;
    mep_common_model<mepcop1_16_mep_model, mep_ext1_cpu, mepcop1_16_idesc, mepcop1_16_scache> my_cop16_model;
    mep_common_model<mepcop1_32_mep_model, mep_ext1_cpu, mepcop1_32_idesc, mepcop1_32_scache> my_cop32_model;
    mep_common_model<mepcop1_48_mep_model, mep_ext1_cpu, mepcop1_48_idesc, mepcop1_48_scache> my_cop48_model;
    mep_common_model<mepcop1_64_mep_model, mep_ext1_cpu, mepcop1_64_idesc, mepcop1_64_scache> my_cop64_model;
  };

#define mep_ext1_hardware hardware
#define CGEN_CPU_FPU(cpu) (&cpu->fpu)
}
