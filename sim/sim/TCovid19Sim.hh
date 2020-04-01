//
#ifndef __covid19_sim_TCovid19Sim_hh__
#define __covid19_sim_TCovid19Sim_hh__

#include "math.h"

#include "TEllipse.h"
#include "TCanvas.h"
#include "TObjArray.h"
#include "TMarker.h"
#include "TRandom3.h"
#include "TSystem.h"

#include "covid19/sim/TPerson.hh"
#include "covid19/sim/TLocation.hh"

//
class TCovid19Sim {
public:
  TObjArray* fListOfLocations;
  TObjArray* fListOfPeople;

  TCanvas*   fCanvas;
  TRandom3   fRn3;

  float      fDrMin;
  float      fInfectionProb;
  float      fTravelProb;
  int        fTravelPeriod;
  float      fDeathProb;
  float      fDeathAtHomeProb;
  float      fStep;

  int        fTimeStep;
  int        fHour;
  int        fDay;
  
  int        fNHospitalized;
  int        fMaxNHospitalized;		// limit on the number of hospitalized people

  TCovid19Sim();
  virtual ~TCovid19Sim();

  void Init(int N1 = 100, int N2 = 100);
  void Run(int NTimeSteps = 100);

  void ModelInfectionTransfer(TPerson* P1, TPerson* P2, int Time);
  void ModelMovement         (int Time);
  void UpdateHealthStatus    (TPerson* Person, int Time);

  void ReturnHome (TPerson* P);
  void StartTravel(TPerson* P, int Time);

  void Draw(Option_t* Opt = "");


  TLocation* Location(int I) { return (TLocation*) fListOfLocations->UncheckedAt(I); }
  TPerson*   Person  (int I) { return (TPerson*  ) fListOfPeople->UncheckedAt(I);    }

  int        NLocations() { return fListOfLocations->GetEntriesFast(); }

					// imagine time step = 1 hour
  float      TimeInDays(int time_step) { return time_step/24.;}

					// death at a hospital probability
  float      DeathProb      (TPerson* P) { return fDeathProb; }

					// death at a home probability
  float      DeathAtHomeProb(TPerson* P) { return fDeathAtHomeProb; }

  ClassDef(TCovid19Sim,0)
};

#endif
