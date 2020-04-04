//-----------------------------------------------------------------------------
#include "covid19/sim/local_types.hh"
#include "covid19/sim/TLocation.hh"
#include "covid19/sim/TPerson.hh"

#include "TSystem.h"
#include "TPad.h"

//-----------------------------------------------------------------------------
TLocation::TLocation() {
  fDataRecord   = nullptr;
}

//-----------------------------------------------------------------------------
TLocation::TLocation(int Index, float X0, float Y0, float Radius) {
  fIndex        = Index;
  fX0           = X0;
  fY0           = Y0;
  fRadius       = Radius;
  fNPeople      = 0;
  fListOfPeople = new TObjArray(20);
  fDataRecord   = nullptr;
  fEllipse      = new TEllipse(fX0,fY0,fRadius,fRadius);
}

//-----------------------------------------------------------------------------
void TLocation::InitDataRecord(int NDays) {
  fDataRecord = new DataRecord_t[NDays];
}
//-----------------------------------------------------------------------------
TLocation::~TLocation() {
  delete fEllipse;
  delete fListOfPeople;
  delete fDataRecord;
}

//-----------------------------------------------------------------------------
void TLocation::Draw(Option_t* Opt) {
  fEllipse->Draw();

  for (int i=0; i<fNPeople; i++) {
    TPerson* p = (TPerson*) fListOfPeople->At(i);
    p->Draw();
  }

}
