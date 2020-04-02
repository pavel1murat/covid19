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
TPerson::TPerson(int Index, float Dx, float Dy, TLocation* Location) {
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
  fIncubationPeriod =  5.;		// days , on average
  fRecoveryPeriod   = 14.;	        // days , on average .. close to that

  fMarker.SetMarkerStyle(7);
}


//-----------------------------------------------------------------------------
void TPerson::Draw(Option_t* Opt) {

  float x = fCurrentLocation->fX0+fDx;
  float y = fCurrentLocation->fY0+fDy;
  
  fMarker.SetX(x);
  fMarker.SetY(y);
  
  int col(1);  // black: recovered

  if      (fHealthStatus == kSusceptible) col = kBlue;
  else if (fHealthStatus == IsInfected()) col = kRed;
  
  fMarker.SetMarkerColor(col);

  fMarker.Draw();
}


//-----------------------------------------------------------------------------
void TPerson::ReturnHome(TRandom3* RnGen) {

  TLocation* oldloc  = fCurrentLocation;
  fCurrentLocation   = fHomeLocation;

  // 
  int index = fIndex;
  oldloc->fListOfPeople->RemoveAt(index);

  TPerson* last = (TPerson*) oldloc->fListOfPeople->Last();
  oldloc->fListOfPeople->RemoveLast();

  oldloc->fListOfPeople->AddAt(last,index);
  last->fIndex = index;
  oldloc->fNPeople -= 1;
  
  fCurrentLocation->fListOfPeople->Add(this);
  fCurrentLocation->fNPeople += 1;

  fTimeOfTravelStart = -1;
  fTravelStatus      = TPerson::kHome;
  fMovementStatus    = TPerson::kFreeToMove;

					// use fTimeOfInfection as just an integer

  double phi = RnGen->Rndm(fTimeOfInfection)*2*M_PI;
  float r    = fHomeLocation->fRadius;

  fDx = r*cos(phi);
  fDy = r*sin(phi);
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
