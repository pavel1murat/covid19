///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////
#include "math.h"

#include "TEllipse.h"
#include "TCanvas.h"
#include "TObjArray.h"
#include "TMarker.h"
#include "TRandom3.h"
#include "TSystem.h"

#include "covid19/sim/TPerson.hh"
#include "covid19/sim/TLocation.hh"

//
class TCovid19Sim {
  TObjArray* fListOfLocations;
  TObjArray* fListOfPeople;

  TCanvas*   fCanvas;
  TRandom3   fRn3;

  float      fDrMin;
  float      fInfectionProb;

  TCovid19Sim();
  ~TCovid19Sim();

  void Init();
  void Run();

  void Draw(Option_t* Opt = "");

  TLocation* Location(int I) { return (TLocation*) fListOfLocations->UncheckedAt(I); }
  TPerson*   Person  (int I) { return (TPerson*) fListOfPeople->UncheckedAt(I); }

  int NLocations() { return fListOfLocations->GetEntriesFast(); }
};


//-----------------------------------------------------------------------------
TCovid19Sim::TCovid19Sim() {
}

//-----------------------------------------------------------------------------
TCovid19Sim::~TCovid19Sim() {
}

//-----------------------------------------------------------------------------
void TCovid19Sim::Init() {
  fDrMin         = 0.01;
  fInfectionProb = 0.9;

  //  int      nlocations = 2;

  float x1(0.3), y1(0.2), r1(0.1);
  int   n1(20);

  float x2(0.5), y2(0.6), r2(0.15);
  int   n2(100);

//-----------------------------------------------------------------------------
// define initial configuration
//-----------------------------------------------------------------------------
  TLocation* place = new TLocation(x1,y1,r1);

  fListOfLocations->Add(place);

  for (int i=0; i<n1; i++) {
    double phi = fRn3.Rndm(i)*2*M_PI;
    double r   = fRn3.Rndm(i)*r1;

    float dx    = r*cos(phi);
    float dy    = r*sin(phi);

    TPerson* p = new TPerson (dx,dy,place);

    fListOfPeople->Add(p);
    place->AddPerson(p);

    if (i == 0) p->fHealthStatus = TPerson::kInfected;
  }

  place = new TLocation(x2,y2,r2);
  fListOfLocations->Add(place);

  for (int i=0; i<n2; i++) {
    double phi = fRn3.Rndm(i)*2*M_PI;
    double r   = fRn3.Rndm(i)*r2;

    float dx    = r*cos(phi);
    float dy    = r*sin(phi);

    TPerson* p = new TPerson (dx,dy,place);

    fListOfPeople->Add(p);
    place->AddPerson(p);
  }

  fCanvas = new TCanvas("nut", "FirstSession",900,900);
}

//-----------------------------------------------------------------------------
void TCovid19Sim::Draw(Option_t* Opt) {
  fCanvas->Clear();

  int nl = fListOfLocations->GetEntriesFast();

  for (int i=0; i<nl; i++) {
    TLocation* loc = Location(i);
    loc->Draw();
  }

  fCanvas->Modified();
  fCanvas->Update();
}

//-----------------------------------------------------------------------------
void TCovid19Sim::Run() {
//-----------------------------------------------------------------------------
// draw initial configuration
//-----------------------------------------------------------------------------

  Draw();

  for (int itime=0; itime<100; itime++) {
//-----------------------------------------------------------------------------
// 1. evolve the system : propagate infection
//-----------------------------------------------------------------------------
    int nl = NLocations();
    for (int iloc=0; iloc<nl; iloc++) {
      TLocation* loc = Location(iloc);
      int np = loc->fNPeople;
      // check people at this location
      for (int i1=0; i1<np; i1++) {
	TPerson* p1 = (TPerson*) loc->Person(i1);

	for (int i2=i1+1; i2<np; i2++) {
	  TPerson* p2 = (TPerson*) loc->Person(i2);

	  if ((p1->IsInfected() && p2->IsHealthy()) || 
	      (p1->IsHealthy() && p2->IsInfected())    ) {
	    // one is healthy, another one is infected -
	    // calculate distance between the two persons - this could lead to an infection
	    // start from the simplest model
	    float dx = p1->fDx-p2->fDx;
	    float dy = p1->fDy-p2->fDy;
	    float dist = sqrt(dx*dx+dy*dy);
	    if (dist < fDrMin) {
	      float r = fRn3.Rndm(i2);
	      if (r  < fInfectionProb) {
		if (p1->IsInfected()) p2->fHealthStatus = TPerson::kInfected;
		else                  p1->fHealthStatus = TPerson::kInfected;
	      }
	    }
	  }
	}
      }
    }

//-----------------------------------------------------------------------------
// step 2: let people move to new positions
//-----------------------------------------------------------------------------
    float kStep = 0.01;

    for (int il=0; il<nl; il++) {
      TLocation* l = Location(il);
      float R2    = l->fRadius*l->fRadius;
      int np      = l->fNPeople;

      for (int i=0; i<np; i++) {
	TPerson* p = l->Person(i);

	// take a step in random direction
	float r2      = R2+1;

	float dx(0), dy(0);

	while (r2 > R2) {
	  double phi  = fRn3.Rndm(i)*2*M_PI;
	  double step = fRn3.Rndm(i)*kStep;

	  dx = p->fDx+step*cos(phi);
	  dy = p->fDy+step*sin(phi);
	  
	  r2 = dx*dx+dy*dy;
	}
	
	p->fDx = dx;
	p->fDy = dy;
      };
    }
//-----------------------------------------------------------------------------
// step 3: visualize
//-----------------------------------------------------------------------------
    Draw();
    gSystem->Sleep(200);
  }
}
