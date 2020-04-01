//-----------------------------------------------------------------------------

#ifndef __covid19_sim_TPerson_hh__
#define __covid19_sim_TPerson_hh__

#include "TObject.h"
#include "TMarker.h"

class TLocation;

class TPerson: public TObject {
public:
  enum {
    kSusceptible  = 0,
    kIncubating   = 1,
    kSymptomatic  = 2,
    kHospitalized = 3,
    kSickHome     = 4,
    kImmune       = 5,
    kDead         = 6
  };

  enum {
    kHome      = 0,
    kTraveling = 1
  };

public:

  TLocation* fCurrentLocation;    // locations not owned
  TLocation* fWorkLocation;
  TLocation* fHomeLocation;

  float            fDx;                 // distance from the Location center 
  float            fDy;

  int              fHealthStatus; // healthy:asymptomatic:symptomatic:hospitalized:recovered=immuned;
  int              fTravelStatus;

  TMarker          fMarker;

  float            fIncubationPeriod;     // could be a constant, during this time the person 
                                          // is asymptomatic
  int              fTravelPeriod;
  int              fTimeOfTravelStart;

  int              fInfectivePower;
  float            fSymptomaticPeriod;
  float            fRecoveryPeriod;

  int              fTime;
  int              fTimeOfInfection;       // time of infection
  int              fOnsetTime;             // time when a person started showing the symptoms
  int              fTimeOfHospitalization; // time when a person has been hospitalized
  int              fTimeOfRecovery;        // time 
  int              fTimeOfDeath;           // 
  int              fNInfected;             // number of other people infected by this person

  int              fRecoveryCode;          // 0:recovery, 1:death

  TPerson();
  TPerson(float Dx, float Dy, TLocation* Location);
  ~TPerson();

  int  IsSusceptible () { return (fHealthStatus == kSusceptible); }
  int  IsHealthy     () { return ((fHealthStatus == kSusceptible) || (fHealthStatus == kImmune     )); }
  int  IsInfected    () { return ((fHealthStatus == kIncubating ) || (fHealthStatus == kSymptomatic)); }

  int  IncubationPeriod() { return fIncubationPeriod; }
  int  RecoveryPeriod  () { return fRecoveryPeriod;   }

  TLocation*  CurrentLocation() { return fCurrentLocation; }
  TLocation*  HomeLocation   () { return fHomeLocation   ; }
  TLocation*  WorkLocation   () { return fWorkLocation   ; }

  void Draw(Option_t* Opt = "");

};

#endif
