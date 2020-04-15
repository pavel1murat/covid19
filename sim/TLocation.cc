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
  fXMax         = fRadius;
  fYMax         = fRadius;

  fListOfPeople = new TObjArray(20);
  fDataRecord   = nullptr;

  //  fEllipse      = new TEllipse(fX0,fY0,fRadius,fRadius);
  
  fPave         = new TPave(fX0-fXMax,fY0-fYMax,fX0+fXMax,fY0+fYMax,1);
  fPave->SetLineWidth(1);
  fPave->SetFillStyle(0);

  for (int ix=0; ix<kNZones; ix++) {
    for (int iy=0; iy<kNZones; iy++) {
      fZone[ix][iy] = new TObjArray(20);
    }
  }
}

//-----------------------------------------------------------------------------
void TLocation::InitDataRecord(int NDays) {
  fDataRecord = new DataRecord_t[NDays];
}


//-----------------------------------------------------------------------------
TLocation::~TLocation() {
  //  delete fEllipse;
  delete fPave;
  delete fListOfPeople;
  delete fDataRecord;
}

//-----------------------------------------------------------------------------
void TLocation::Init(int NTotal, int NInfected, int Color, int NDays, TRandom3* RnGen) {

//-----------------------------------------------------------------------------
// model uniform population density
//-----------------------------------------------------------------------------
  float rn[2];

  float sx = 2*fXMax/kNZones;
  float sy = 2*fYMax/kNZones;

  for (int i=0; i<NTotal; i++) {
    RnGen->RndmArray(2,rn);

    float dx    = (2*rn[0]-1)*fXMax;
    float dy    = (2*rn[1]-1)*fYMax;

    int ix = (dx+fXMax)/sx;
    int iy = (dy+fYMax)/sy;

    // initial location index is the same as the loop index
    TPerson* p  = new TPerson (i,dx,dy,this,ix,iy,Color);

    fListOfPeople->Add(p);

    int n = fZone[ix][iy]->GetEntries();

    p->fZoneIndex = n;
    fZone[ix][iy]->Add(p);
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
void TLocation::UpdateZone(TPerson* P) {

  float sx = 2*fXMax/kNZones;
  float sy = 2*fYMax/kNZones;

  int ix = (P->fDx+fXMax)/sx;
  int iy = (P->fDy+fYMax)/sy;

  if ((ix != P->fZoneIX) || (iy != P->fZoneIY)) {
    // having taken a step, a person ended up in a new zone 
    
    TObjArray* oldzone = fZone[P->fZoneIX][P->fZoneIY];
    int np  = oldzone->GetEntries();

    TObjArray* newzone = fZone[ix][iy];

    int loc = P->fZoneIndex;
    oldzone->RemoveAt(loc);

    if (loc < np) {
      TPerson* last = (TPerson*) oldzone->RemoveAt(np-1);
      oldzone->AddAt(last,loc);
      last->fZoneIndex = loc;
    }

    newzone->Add(P);
    P->fZoneIndex = newzone->GetEntries()-1;
  }
}


//-----------------------------------------------------------------------------
void TLocation::Draw(Option_t* Opt) {
  //  fEllipse->Draw();
  fPave->Draw();

  int np = fListOfPeople->GetEntries();

  for (int i=0; i<np; i++) {
    TPerson* p = (TPerson*) fListOfPeople->At(i);
    p->Draw();
  }

}
