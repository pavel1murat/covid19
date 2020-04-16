//-----------------------------------------------------------------------------
#include "TRandom3.h"
#include "covid19/sim/TPerson.hh"
#include "covid19/sim/TLocation.hh"

float TPerson::fgStep = 0.01;

//-----------------------------------------------------------------------------
TPerson::TPerson() {
}

//-----------------------------------------------------------------------------
TPerson::~TPerson() {
}


//-----------------------------------------------------------------------------
TPerson::TPerson(int Index, float Dx, float Dy, TLocation* Location, int ZoneIX, int ZoneIY, int Color) {
  fIndex            = Index;
  fDx               = Dx;
  fDy               = Dy;
  fCurrentLocation  = Location;
  fZoneIX           = ZoneIX;
  fZoneIY           = ZoneIY;

  fHomeLocation     = Location;
  fWorkLocation     = nullptr;

  fHealthStatus     = kSusceptible;
  fTravelStatus     = kHome;
  fMovementStatus   = kFreeToMove;

  fStep              = fgStep;

  fTime              = 0;
  fTimeOfInfection   = -1;
  fTimeOfTravelStart = -1;

  fInfectivePower    = 1;
  fNInfected         = 0;
					// start from making those constants
  fIncubationPeriod  =  5.;		// should be 5 days , on average
  fRecoveryPeriod    =  14.;            // days , on average .. close to that

  fColor             = Color;
  fMarker.SetMarkerStyle(20);
  fMarker.SetMarkerSize (0.7);
  fMarker.SetMarkerColor(fColor);
}


//-----------------------------------------------------------------------------
void TPerson::Draw(Option_t* Opt) {

  float x = fCurrentLocation->fX0+fDx;
  float y = fCurrentLocation->fY0+fDy;
  
  fMarker.SetX(x);
  fMarker.SetY(y);
  
  int   col(1);   // black: recovered
  float size(0.5); 

  if      (fHealthStatus == kSusceptible) {
    col = fColor;
    size = 0.5;
  }
  else if (fHealthStatus == kIncubating) {
    col  = 2;
    size = 0.5;
  }
  else if (fHealthStatus == kSymptomatic) {
    col  = 2;
    size = 0.7;
  }
  else if (fHealthStatus == kImmune) {
    col  = 1;
    size = 0.5;
  }
  else if (fHealthStatus == kDead) {
    col  = 1;
    size = 0.7;
  }
  
  fMarker.SetMarkerColor(col );
  fMarker.SetMarkerSize (size);
  fMarker.Draw();
}


//-----------------------------------------------------------------------------
void TPerson::ReturnHome(int Time, TRandom3* RnGen) {

  float   rn[2];

  TLocation* oldloc  = fCurrentLocation;
  fCurrentLocation   = fHomeLocation;

  int index = fIndex;
  int np    = oldloc->fListOfPeople->GetEntries();

  oldloc->fListOfPeople->RemoveAt(index);

  if (index < np-1) {
    TPerson* last = (TPerson*) oldloc->fListOfPeople->RemoveAt(np-1);
    oldloc->fListOfPeople->AddAt(last,index);
    last->fIndex = index;
  }

  // remove person from his old zone

  TObjArray* oldzone = oldloc->fZone[fZoneIX][fZoneIY];
  int npz = oldzone->GetEntries();
  int loc = fZoneIndex;
  oldzone->RemoveAt(loc);

  if (loc < npz) {
    TPerson* last = (TPerson*) oldzone->RemoveAt(npz-1);
    oldzone->AddAt(last,loc);
    last->fZoneIndex = loc;
  }

  TLocation* newloc = fCurrentLocation;
  
  newloc->fListOfPeople->Add(this);
  
  int newindex = newloc->fListOfPeople->GetEntries()-1;
  fIndex       = newindex;

  fTimeOfTravelStart = -1;
  fTravelStatus      = TPerson::kHome;
  fMovementStatus    = TPerson::kFreeToMove;

					// use fTimeOfInfection as just an integer
  int nz   = newloc->NZones();

  RnGen->RndmArray(2,rn);
  fDx = 2*(rn[0]-1)*newloc->fRadius;
  fDy = 2*(rn[1]-1)*newloc->fRadius;
					// 'install' person in a new zone
  float sx = 2*newloc->fXMax/nz;
  float sy = 2*newloc->fYMax/nz;

  int   ix = (fDx+newloc->fXMax)/sx;
  int   iy = (fDy+newloc->fYMax)/sy;

  TObjArray* newzone = newloc->fZone[ix][iy];
  newzone->Add(this);
  fZoneIndex = newzone->GetEntries()-1;

  // printf(" Time= %5i %p index %5i returned home from loc %2i to loc %2i;",
  //  	 Time,this,index,oldloc->fIndex,newloc->fIndex);

  // printf(" new_index %5i",newindex);
  // printf(" np(oldloc),np(newloc):  %5i .. %5i\n",
  //  	 oldloc->fListOfPeople->GetEntries(),
  //  	 newloc->fListOfPeople->GetEntries());
}

//-----------------------------------------------------------------------------
void TPerson::TakeOneStep(TRandom3* RnGen) {

  float dx(0), dy(0), rn[2];
  
  while (1) {
    RnGen->RndmArray(2,rn);

    double step = fStep; // RnGen->Rndm(fTravelStatus)*fStep;
    
    dx    = fDx+step*(2*rn[0]-1);
    if (fabs(dx) < fCurrentLocation->fXMax) {
      dy    = fDy+step*(2*rn[1]-1);
      if (fabs(dy) < fCurrentLocation->fYMax) break;
    }
  }
	    
  fDx = dx;
  fDy = dy;
					// recalculate the zone

  fCurrentLocation->UpdateZone(this);
}
