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

};




void TCovid19Sim::Init() {
}

//-----------------------------------------------------------------------------
void test() {

  TRandom3 rn3;

  TLocation* place[100];
  int      nlocations = 2;

  float x1(0.3), y1(0.2), r1(0.1);
  int   n1(20);

  float x2(0.5), y2(0.6), r2(0.15);
  int   n2(100);

  float   kDrMin        (0.01);
  float   kInfectionProb(0.9);
//-----------------------------------------------------------------------------
// define initial configuration
//-----------------------------------------------------------------------------
  place[0] = new TLocation(x1,y1,r1);

  for (int i=0; i<n1; i++) {
    double phi = rn3.Rndm(i)*2*M_PI;
    double r   = rn3.Rndm(i)*r1;

    float dx    = r*cos(phi);
    float dy    = r*sin(phi);

    TPerson* p = new TPerson (dx,dy,place[0]);

    place[0]->AddPerson(p);

    if (i == 0) p->fHealthStatus = TPerson::kInfected;
  }

  place[1] = new Location(x2,y2,r2);
  for (int i=0; i<n2; i++) {
    double phi = rn3.Rndm(i)*2*M_PI;
    double r   = rn3.Rndm(i)*r2;

    float dx    = r*cos(phi);
    float dy    = r*sin(phi);

    TPerson* p = new TPerson (dx,dy,place[1]);

    place[1]->AddPerson(p);
  }

  TCanvas* canvas = new TCanvas("nut", "FirstSession",800,800);
//-----------------------------------------------------------------------------
// draw initial configuration
//-----------------------------------------------------------------------------
  canvas->Clear();
  for (int i=0; i<nlocations; i++) {
    place[i]->Draw();
  }

  canvas->Modified();
  canvas->Update();

  for (int itime=0; itime<100; itime++) {
//-----------------------------------------------------------------------------
// 1. evolve the system : propagate infection
//-----------------------------------------------------------------------------
    for (int il=0; il<nlocations; il++) {
      Location* loc = place[il];
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
	    if (dist < kDrMin) {
	      float r = rn3.Rndm(i2);
	      if (r  < kInfectionProb) {
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

    for (int il=0; il<nlocations; il++) {
      Location* l = place[il];
      float R2    = l->fRadius*l->fRadius;
      int np      = l->fNPeople;

      for (int i=0; i<np; i++) {
	TPerson* p = (TPerson*) l->fListOfPeople->UncheckedAt(i);

	// take a step in random direction
	float r2      = R2+1;

	float dx(0), dy(0);

	while (r2 > R2) {
	  double phi  = rn3.Rndm(i)*2*M_PI;
	  double step = rn3.Rndm(i)*kStep;

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
    canvas->Clear();
    for (int i=0; i<nlocations; i++) {
      place[i]->Draw();
    }

    canvas->Modified();
    canvas->Update();

    gSystem->Sleep(200);
  }
}
