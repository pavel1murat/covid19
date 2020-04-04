///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __covid19_sim_local_types__
#define __covid19_sim_local_types__

struct DataRecord_t {
  int          fNSusceptible;
  int          fNIncubating;
  int          fNSymptomatic;
  int          fNHospitalized;
  int          fNQuarantined;
  int          fNDead;
  int          fNImmune;
};

#endif
