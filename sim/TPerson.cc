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
TPerson::TPerson(int Index, float Dx, float Dy, TLocation* Location, int Color) {
  fIndex            = Index;
  fDx               = Dx;
  fDy               = Dy;
  fCurrentLocation  = Location;
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

  TLocation* oldloc  = fCurrentLocation;
  fCurrentLocation   = fHomeLocation;

  // 
  int index = fIndex;
  int np    = oldloc->fListOfPeople->GetEntries();

  oldloc->fListOfPeople->RemoveAt(index);

  if (index < np-1) {
    TPerson* last = (TPerson*) oldloc->fListOfPeople->RemoveAt(np-1);
    oldloc->fListOfPeople->AddAt(last,index);
    last->fIndex = index;
  }

  TLocation* newloc = fCurrentLocation;
  
  newloc->fListOfPeople->Add(this);
  
  int newindex = newloc->fListOfPeople->GetEntries()-1;
  fIndex       = newindex;

  fTimeOfTravelStart = -1;
  fTravelStatus      = TPerson::kHome;
  fMovementStatus    = TPerson::kFreeToMove;

					// use fTimeOfInfection as just an integer

  double phi = RnGen->Rndm(fTimeOfInfection)*2*M_PI;
  float r    = fHomeLocation->fRadius;

  fDx = r*cos(phi);
  fDy = r*sin(phi);

  printf(" Time= %5i %p index %5i returned home from loc %2i to loc %2i;",
   	 Time,this,index,oldloc->fIndex,newloc->fIndex);

  printf(" new_index %5i",newindex);
  printf(" np(oldloc),np(newloc):  %5i .. %5i\n",
   	 oldloc->fListOfPeople->GetEntries(),
   	 newloc->fListOfPeople->GetEntries());
}

//-----------------------------------------------------------------------------
void TPerson::TakeOneStep(TRandom3* RnGen) {

  float R2 = fCurrentLocation->R2();

  float r2      = R2+1;
  float dx(0), dy(0);
  
  while (r2 > R2) {
    double phi  = RnGen->Rndm(fTravelStatus)*2*M_PI;
    double step = fStep; // RnGen->Rndm(fTravelStatus)*fStep;
    
    dx    = fDx+step*cos(phi);
    dy    = fDy+step*sin(phi);
    
    r2    = dx*dx+dy*dy;
  }
	    
  fDx = dx;
  fDy = dy;
}
