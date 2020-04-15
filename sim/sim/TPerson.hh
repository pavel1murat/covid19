//-----------------------------------------------------------------------------

#ifndef __covid19_sim_TPerson_hh__
#define __covid19_sim_TPerson_hh__

#include "TObject.h"
#include "TMarker.h"

class TLocation;
class TRandom3;

class TPerson: public TObject {
public:
					// health status
  enum HealthStatus_t {
    kSusceptible  = 0,
    kIncubating   = 1,
    kSymptomatic  = 2,
    kImmune       = 3,
    kDead         = 4
  };
					// travel status
  enum {
    kHome      = 0,
    kTraveling = 1
  };
					// movement status
  enum LocationStatus_t {
    kFreeToMove   = 0,
    kHospitalized = 1,
    kQuarantined  = 2
  };


public:

  static     float fgStep;        // step size

  TLocation*       fCurrentLocation;    // locations not owned
  TLocation*       fWorkLocation;
  TLocation*       fHomeLocation;

  int              fIndex;              // index at a current location

  int              fZoneIX;		// indices of the zone
  int              fZoneIY;
  int              fZoneIndex;

  float            fDx;                 // distance from the Location center 
  float            fDy;
  float            fStep;

  HealthStatus_t   fHealthStatus; // healthy:asymptomatic:symptomatic:hospitalized:recovered=immuned;
  int              fTravelStatus;
  int              fMovementStatus;

  TMarker          fMarker;
  int              fColor;

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
  TPerson(int Index, float Dx, float Dy, TLocation* Location, int ZoneIX, int ZoneIY, int Color);
  ~TPerson();

  int  IsSusceptible () { return (fHealthStatus == kSusceptible); }
  int  IsIncubating  () { return (fHealthStatus == kIncubating); }
  int  IsSymptomatic () { return (fHealthStatus == kSymptomatic); }
  int  IsImmune      () { return (fHealthStatus == kImmune); }
  int  IsDead        () { return (fHealthStatus == kDead); }

  int  IsHealthy     () { return ((fHealthStatus == kSusceptible) || (fHealthStatus == kImmune     )); }
  int  IsInfected    () { return ((fHealthStatus == kIncubating ) || (fHealthStatus == kSymptomatic)); }

  int  IsFreeToMove  () { return (fMovementStatus == kFreeToMove);   }
  int  IsHospitalized() { return (fMovementStatus == kHospitalized); }
  int  IsQuarantined () { return (fMovementStatus == kQuarantined);  }

  int  IsRestricted  () { return (fMovementStatus != kFreeToMove); }

  int  IncubationPeriod() { return fIncubationPeriod; }
  int  RecoveryPeriod  () { return fRecoveryPeriod;   }

  TLocation*  CurrentLocation() { return fCurrentLocation; }
  TLocation*  HomeLocation   () { return fHomeLocation   ; }
  TLocation*  WorkLocation   () { return fWorkLocation   ; }

  static void SetStep(float Step) { TPerson::fgStep = Step; }

  void        TakeOneStep(TRandom3* RnGen);
  void        ReturnHome (int Time, TRandom3* RnGen);

  void Draw(Option_t* Opt = "");

};

#endif
