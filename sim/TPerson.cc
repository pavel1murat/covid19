//-----------------------------------------------------------------------------
#include "covid19/sim/TPerson.hh"
#include "covid19/sim/TLocation.hh"


//-----------------------------------------------------------------------------
TPerson::TPerson() {
}

//-----------------------------------------------------------------------------
TPerson::~TPerson() {
}


//-----------------------------------------------------------------------------
TPerson::TPerson(float Dx, float Dy, TLocation* Location) {
  fDx               = Dx;
  fDy               = Dy;
  fCurrentLocation  = Location;
  fHomeLocation     = Location;
  fWorkLocation     = nullptr;
  fHealthStatus     = kSusceptible;
  fTime             = 0;
  fTimeOfInfection  = -1;
  fInfectivePower   = 1;
  fNInfected        = 0;
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


