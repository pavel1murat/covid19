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
TPerson::TPerson(float Dx, float Dy, const TLocation* Location) {
  fDx            = Dx;
  fDy            = Dy;
  fLocation      = Location;
  fHealthStatus  = kHealthy;
  fTime          = 0;
  fInfectionTime = -1;
  fMarker.SetMarkerStyle(7);
}


//-----------------------------------------------------------------------------
void TPerson::Draw(Option_t* Opt) {

  float x = fLocation->fX0+fDx;
  float y = fLocation->fY0+fDy;
  
  fMarker.SetX(x);
  fMarker.SetY(y);
  
  int col(1);  // black: recovered

  if      (fHealthStatus == kHealthy ) col = kBlue;
  else if (fHealthStatus == kInfected) col = kRed;
  
  fMarker.SetMarkerColor(col);

  fMarker.Draw();
}


