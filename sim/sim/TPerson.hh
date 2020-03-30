//-----------------------------------------------------------------------------

#ifndef __covid19_sim_TPerson_hh__
#define __covid19_sim_TPerson_hh__

#include "TObject.h"
#include "TMarker.h"

class TLocation;

class TPerson: public TObject {
public:
  enum {
    kHealthy   = 0,
    kInfected  = 1,
    kRecovered = 2
  };

public:
  const TLocation* fLocation;  // not owned
  float            fDx;        // distance from the Location center 
  float            fDy;

  int              fHealthStatus; // healthy:infected:recovered=immuned;

  TMarker          fMarker;

  int              fTime;
  int              fInfectionTime;

  TPerson();
  TPerson(float Dx, float Dy, const TLocation* Location);
  ~TPerson();

  int  IsInfected() { return (fHealthStatus == kInfected) ; }
  int  IsHealthy () { return (fHealthStatus == kHealthy ) ; }

  void Draw(Option_t* Opt = "");

};

#endif
