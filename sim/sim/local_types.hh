///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __covid19_sim_local_types__
#define __covid19_sim_local_types__

struct DataRecord_t {
  int          fNSusceptible;
  int          fNIncubating;
  int          fNSymptomatic;
  int          fNDead;
  int          fNImmune;

  int          fNFreeToMove;
  int          fNHospitalized;
  int          fNQuarantined;
};

#endif
