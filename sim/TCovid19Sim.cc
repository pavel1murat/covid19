///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////
#include "covid19/sim/TCovid19Sim.hh"

ClassImp(TCovid19Sim)

//-----------------------------------------------------------------------------
TCovid19Sim::TCovid19Sim() {
  fListOfLocations = new TObjArray(20);
  fListOfPeople    = new TObjArray(20);
}

//-----------------------------------------------------------------------------
TCovid19Sim::~TCovid19Sim() {
}

//-----------------------------------------------------------------------------
void TCovid19Sim::Init(int N1, int N2) {
  fDrMin         = 0.01;
  fInfectionProb = 0.9;
  fDeathProb     = 0.01;  // out of infected
  fTravelProb    = 0.01;
  fTravelPeriod  =  72 ;  // time_steps = hours


  fStep          = 0.01;

  float x2(0.5), y2(0.6), r2(0.15);
  float x1(0.3), y1(0.2), r1(0.1);
//-----------------------------------------------------------------------------
// define initial configuration
//-----------------------------------------------------------------------------
  TLocation* place = new TLocation(x1,y1,r1);

  fListOfLocations->Add(place);

  for (int i=0; i<N1; i++) {
    double phi = fRn3.Rndm(i)*2*M_PI;
    double r   = fRn3.Rndm(i)*r1;

    float dx    = r*cos(phi);
    float dy    = r*sin(phi);

    TPerson* p = new TPerson (dx,dy,place);

    fListOfPeople->Add(p);
    place->AddPerson(p);

    if (i == 0) {
      p->fHealthStatus    = TPerson::kIncubating;
      p->fTimeOfInfection = -1;
    }
  }

  place = new TLocation(x2,y2,r2);
  fListOfLocations->Add(place);

  for (int i=0; i<N2; i++) {
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
// Time - in hours from the start of the simulation
//-----------------------------------------------------------------------------
void TCovid19Sim::UpdateHealthStatus(TPerson* P, int Time) {

  if (P->IsInfected()) {
//-----------------------------------------------------------------------------
// check if the recovery time has come
//-----------------------------------------------------------------------------
    if (P->fHealthStatus == TPerson::kIncubating) {
      float dt = TimeInDays(Time - P->fTimeOfInfection);
      if (dt > P->IncubationPeriod()) {
//-----------------------------------------------------------------------------
// the person transitions from the incubating period to showing the symptoms
// he could be restricted 
//-----------------------------------------------------------------------------
	P->fHealthStatus = TPerson::kSymptomatic;
	P->fOnsetTime    = Time;
      }
    }
    else {
//-----------------------------------------------------------------------------
// either symptomatic or hospitalized 
//-----------------------------------------------------------------------------
      if (P->fHealthStatus == TPerson::kSymptomatic) {
	float dt = TimeInDays(Time-P->fOnsetTime);
	if (dt > P->fSymptomaticPeriod) {
//-----------------------------------------------------------------------------
//gets hospitalized , if there is enough beds
//-----------------------------------------------------------------------------
	  if (this->fNHospitalized < this->fMaxNHospitalized) {
	    P->fHealthStatus          = TPerson::kHospitalized;
	    P->fTimeOfHospitalization = Time;
	    
	    this->fNHospitalized += 1;
	  }
	  else {
//-----------------------------------------------------------------------------
// could not get hospitalized, not moving, sick at home, the clock is ticking
//-----------------------------------------------------------------------------
	    P->fHealthStatus        = TPerson::kSickHome;
	  }
	}
	else {
//-----------------------------------------------------------------------------
// dt < hospitalization time, sick person still moving around, 
// the person's infecting capability increases by x2 (assumption)
//-----------------------------------------------------------------------------
	  if (P->fInfectivePower == 1) P->fInfectivePower = 2;
	}
      }
      else if (P->fHealthStatus == TPerson::kSickHome) {
	float dt = TimeInDays(Time - P->fTimeOfInfection);
	if (dt > P->RecoveryPeriod()) {
	  double x = fRn3.Rndm(-1);
	  if (x < DeathAtHomeProb(P)) {               // a small number
	    P->fHealthStatus = TPerson::kDead;
	    P->fTimeOfDeath  = Time;
	  }
	}
      }
      else {
//-----------------------------------------------------------------------------
// the person is hospitalized
//-----------------------------------------------------------------------------
	float dt = TimeInDays(Time - P->fTimeOfInfection);
	if (dt > P->RecoveryPeriod()) {
	  P->fHealthStatus   = TPerson::kImmune;
	  P->fTimeOfRecovery = Time;
	}
      }
    }
  }
}


//-----------------------------------------------------------------------------
void TCovid19Sim::ModelInfectionTransfer(TPerson* P1, TPerson* P2, int Time) {
//-----------------------------------------------------------------------------
// one person is healthy, another one is infected -
// calculate distance between the two - this could lead to an infection
// start from the simplest assumption
//-----------------------------------------------------------------------------
  float dx   = P1->fDx-P2->fDx;
  float dy   = P1->fDy-P2->fDy;
  float dist = sqrt(dx*dx+dy*dy);

  if (dist < fDrMin) {
    float r            = fRn3.Rndm(Time);
    if (r < fInfectionProb*P1->fInfectivePower) {
      P2->fHealthStatus    = TPerson::kIncubating;
      P2->fTimeOfInfection = Time;

      P1->fNInfected += 1;
    }
  }
}

//-----------------------------------------------------------------------------
// send person to another location
//-----------------------------------------------------------------------------
void TCovid19Sim::StartTravel(TPerson* P, int Time) {

  // choose travel destination 
  // currently have only two locations

  TLocation* loc  = P->CurrentLocation();
  TLocation* loc0 = Location(0);

  TLocation* newloc;

  if (loc == loc0) newloc = Location(1);
  else             newloc = Location(0);

  P->fCurrentLocation   = newloc;
  P->fTimeOfTravelStart = Time;
  P->fTravelStatus      = TPerson::kTraveling;

  double phi  = fRn3.Rndm(Time)*2*M_PI;
		
  P->fDx = newloc->fRadius*cos(phi);
  P->fDy = newloc->fRadius*sin(phi);
}

//-----------------------------------------------------------------------------
void TCovid19Sim::ReturnHome(TPerson* P) {

  P->fCurrentLocation   = P->fHomeLocation;
  P->fTimeOfTravelStart = -1;
  P->fTravelStatus      = TPerson::kHome;
					// use fTimeOfInfection as just an integer
  double phi  = fRn3.Rndm(P->fTimeOfInfection)*2*M_PI;
	
  float r = P->fHomeLocation->fRadius;

  P->fDx = r*cos(phi);
  P->fDy = r*sin(phi);
}

//-----------------------------------------------------------------------------
void TCovid19Sim::ModelMovement(int Time) {

  int nl = NLocations();

  for (int il=0; il<nl; il++) {
    TLocation* l = Location(il);
    float R2    = l->fRadius*l->fRadius;
    int np      = l->fNPeople;

    for (int i=0; i<np; i++) {
      TPerson* p = l->Person(i);
      
      if (! p->IsHealthy()) { 
//-----------------------------------------------------------------------------
// hospitalized or asymptomatic person on travel
//-----------------------------------------------------------------------------
	if (p->fTravelStatus == TPerson::kTraveling) {
	  ReturnHome(p);
	}
	continue;
      }
//-----------------------------------------------------------------------------
// person is healthy
//-----------------------------------------------------------------------------
      if (p->fTravelStatus == TPerson::kHome) {
	double x  = fRn3.Rndm(i);
	if (x < fTravelProb) {
//-----------------------------------------------------------------------------
// healthy=susceptible and kIncubating people can travel
//-----------------------------------------------------------------------------
	  StartTravel(p,Time);
	}
	else {
//-----------------------------------------------------------------------------
// move locally
//-----------------------------------------------------------------------------
	  float r2      = R2+1;
	  float dx(0), dy(0);

	  while (r2 > R2) {
	    double phi  = fRn3.Rndm(i)*2*M_PI;
	    double step = fRn3.Rndm(i)*fStep;
	    
	    dx    = p->fDx+step*cos(phi);
	    dy    = p->fDy+step*sin(phi);
	    
	    r2    = dx*dx+dy*dy;
	  }
	    
	  p->fDx = dx;
	  p->fDy = dy;
	}
      }
    }
  }
}

//-----------------------------------------------------------------------------
void TCovid19Sim::Run(int NTimeSteps) {
//-----------------------------------------------------------------------------
// draw initial configuration
//-----------------------------------------------------------------------------
  Draw();

//-----------------------------------------------------------------------------
// time_step - by one hour
//-----------------------------------------------------------------------------
  for (int time_step=0; time_step<NTimeSteps; time_step++) {
    fTimeStep = time_step;
    fHour     = time_step % 24;
    fDay      = time_step / 24;
//-----------------------------------------------------------------------------
// 1. evolve the system : propagate infection at at his location
//-----------------------------------------------------------------------------
    int nl = NLocations();
    for (int iloc=0; iloc<nl; iloc++) {
      TLocation* loc = Location(iloc);
      int np = loc->fNPeople;
//-----------------------------------------------------------------------------
// step 1: evolve individual health status
//-----------------------------------------------------------------------------
      for (int i=0; i<np; i++) {
	TPerson* p = (TPerson*) loc->Person(i); 
	UpdateHealthStatus(p,time_step);
      }
//-----------------------------------------------------------------------------
// step 2: model infection transfer between the two people currently at that location
//-----------------------------------------------------------------------------
      for (int i1=0; i1<np; i1++) {
	TPerson* p1 = (TPerson*) loc->Person(i1); 
	for (int i2=i1+1; i2<np; i2++) {
	  TPerson* p2 = (TPerson*) loc->Person(i2); 
	  if      (p1->IsInfected() && p2->IsSusceptible()) ModelInfectionTransfer(p1,p2,time_step);
	  else if (p2->IsInfected() && p1->IsSusceptible()) ModelInfectionTransfer(p2,p1,time_step);
	}
      }
    }
//-----------------------------------------------------------------------------
// step 3: let people move , at this point they can move between the locations as well
//-----------------------------------------------------------------------------
    printf(" before ModelMovement, step = %5i\n",time_step);
    ModelMovement(time_step);
//-----------------------------------------------------------------------------
// step 4: visualize end of the step
//-----------------------------------------------------------------------------
    Draw();
    gSystem->Sleep(200);
  }
}
