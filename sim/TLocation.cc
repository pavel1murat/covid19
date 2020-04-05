//-----------------------------------------------------------------------------
#include "covid19/sim/local_types.hh"
#include "covid19/sim/TLocation.hh"
#include "covid19/sim/TPerson.hh"

#include "TSystem.h"
#include "TPad.h"
#include "TRandom3.h"

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
void TLocation::Init(int NTotal, int NInfected, int Color, int NDays, TRandom3* RnGen) {

//-----------------------------------------------------------------------------
// model uniform population density
//-----------------------------------------------------------------------------
  for (int i=0; i<NTotal; i++) {
    double phi = RnGen->Rndm(i)*2*M_PI;

    double r   = sqrt(RnGen->Rndm(i))*fRadius;

    float dx    = r*cos(phi);
    float dy    = r*sin(phi);

    TPerson* p  = new TPerson (i,dx,dy,this,Color);

    fListOfPeople->Add(p);
  }
//-----------------------------------------------------------------------------
// some are infected at T=0
//-----------------------------------------------------------------------------
  for (int i=0; i<NInfected; i++) {
					// choose infected ones randomly
    int ip = NTotal*RnGen->Rndm();
    TPerson* p = Person(ip);
    p->fHealthStatus    = TPerson::kIncubating;
    p->fTimeOfInfection = -1;
  }
					// init data summary to store the results
  InitDataRecord(NDays);
}

//-----------------------------------------------------------------------------
void TLocation::Draw(Option_t* Opt) {
  fEllipse->Draw();

  int np = fListOfPeople->GetEntries();

  for (int i=0; i<np; i++) {
    TPerson* p = (TPerson*) fListOfPeople->At(i);
    p->Draw();
  }

}
