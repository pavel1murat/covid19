//-----------------------------------------------------------------------------
#ifndef __covid19_sim_TLocation_hh__
#define __covid19_sim_TLocation_hh__

#include "TObject.h"
#include "TObjArray.h"
#include "TEllipse.h"

#include "covid19/sim/local_types.hh"

class TPerson;

class TLocation: public TObject  {
public:
  int           fIndex;
  float         fX0;
  float         fY0;
  float         fRadius;

  int           fNPeople;
  TObjArray*    fListOfPeople;
  TEllipse*     fEllipse;

  DataRecord_t* fDataRecord;
  
  TLocation();
  TLocation(int Index, float X0, float Y0, float Radius);
  ~TLocation();

  void AddPerson(TPerson* p) {
    fListOfPeople->Add((TObject*) p);
    fNPeople++;
  }

  TPerson*    Person(int I) { return (TPerson*) fListOfPeople->UncheckedAt(I); }

  float R2() { return fRadius*fRadius; }

  void InitDataRecord(int NDays);
  void Draw(Option_t* Opt = "");
};

#endif
