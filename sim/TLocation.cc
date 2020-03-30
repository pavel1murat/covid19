//-----------------------------------------------------------------------------
#include "covid19/sim/TLocation.hh"
#include "covid19/sim/TPerson.hh"
//-----------------------------------------------------------------------------
TLocation::TLocation() {
}

//-----------------------------------------------------------------------------
TLocation::TLocation(float X0, float Y0, float Radius) {
  fX0      = X0;
  fY0      = Y0;
  fRadius  = Radius;
  fNPeople = 0;
  fListOfPeople = new TObjArray(20);
  fEllipse      = new TEllipse(fX0,fY0,fRadius,fRadius);
}

//-----------------------------------------------------------------------------
TLocation::~TLocation() {
  delete fEllipse;
  delete fListOfPeople;
}

//-----------------------------------------------------------------------------
void TLocation::Draw(Option_t* Opt) {
  fEllipse->Draw();
  for (int i=0; i<fNPeople; i++) {
    TPerson* p = (TPerson*) fListOfPeople->At(i);
    p->Draw();
  }
}
