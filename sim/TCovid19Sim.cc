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
void TCovid19Sim::Init(int N1, int NInf1, int N2, int NInf2) {
  fDrMin         = 0.01;
  fInfectionProb = 0.1;
  fDeathProb     = 0.01;  // out of infected
  fTravelProb    = 1.e-4;
  fTravelPeriod  = 3. ;   // in days


  float x2(0.5), y2(0.6), r2(0.15);
  float x1(0.3), y1(0.2), r1(0.1);
//-----------------------------------------------------------------------------
// define initial configuration - first location
//-----------------------------------------------------------------------------
  TLocation* location = new TLocation(0,x1,y1,r1);

  fListOfLocations->Add(location);

  for (int i=0; i<N1; i++) {
    double phi = fRn3.Rndm(i)*2*M_PI;
    double r   = fRn3.Rndm(i)*r1;

    float dx    = r*cos(phi);
    float dy    = r*sin(phi);

    TPerson* p = new TPerson (i,dx,dy,location);

    fListOfPeople->Add(p);
    location->AddPerson(p);
  }
//-----------------------------------------------------------------------------
// infect some 
//-----------------------------------------------------------------------------
  for (int i=0; i<NInf1; i++) {
    int ip = N1*fRn3.Rndm();
    TPerson* p = location->Person(ip);
    p->fHealthStatus = TPerson::kIncubating;
    p->fTimeOfInfection = -1;
  }
//-----------------------------------------------------------------------------
// second location
//-----------------------------------------------------------------------------
  location = new TLocation(1,x2,y2,r2);
  fListOfLocations->Add(location);

  for (int i=0; i<N2; i++) {
    double phi = fRn3.Rndm(i)*2*M_PI;
    double r   = fRn3.Rndm(i)*r2;

    float dx    = r*cos(phi);
    float dy    = r*sin(phi);

    TPerson* p = new TPerson (i,dx,dy,location);

    fListOfPeople->Add(p);
    location->AddPerson(p);
  }
//-----------------------------------------------------------------------------
// infect some 
//-----------------------------------------------------------------------------
  for (int i=0; i<NInf2; i++) {
    int ip = N2*fRn3.Rndm();
    TPerson* p = location->Person(ip);
    p->fHealthStatus    = TPerson::kIncubating;
    p->fTimeOfInfection = -1;
  }

  fCanvas = new TCanvas("nut", "FirstSession",900,900);
//-----------------------------------------------------------------------------
// draw initial configuration
//-----------------------------------------------------------------------------
  Draw();
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
	if (P->IsFreeToMove()) {
					// shows symptoms but not hospitalized/quarantined yet
	  float dt = TimeInDays(Time-P->fOnsetTime);
	  if (dt > P->fSymptomaticPeriod) {
//-----------------------------------------------------------------------------
// gets hospitalized , if there is enough beds
//-----------------------------------------------------------------------------
	    if (this->fNHospitalized < this->fMaxNHospitalized) {
	      P->fHealthStatus          = TPerson::kHospitalized;
	      P->fTimeOfHospitalization = Time;
	      P->fMovementStatus        = TPerson::kHospitalized;
	      
	      this->fNHospitalized     += 1;
	    }
	    else {
//-----------------------------------------------------------------------------
// could not get hospitalized, not moving, sick at home, the clock is ticking
//-----------------------------------------------------------------------------
	      P->fMovementStatus      = TPerson::kQuarantined;
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
	else if (P->fHealthStatus == TPerson::kQuarantined) {
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
					// recovered
	    P->fHealthStatus   = TPerson::kImmune;
	    P->fMovementStatus = TPerson::kFreeToMove;
	    P->fTimeOfRecovery = Time;
	  }
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

  TLocation* oldloc = P->CurrentLocation();
  TLocation* loc0 = Location(0);

  TLocation* newloc;

  if (oldloc == loc0) newloc = Location(1);
  else                newloc = Location(0);

  int index = P->fIndex;

  printf(" Time= %5i %p index %5i went on travel from loc %2i to loc %2i\n",
	 Time,P,P->fIndex,oldloc->fIndex,newloc->fIndex);

  TPerson* last = (TPerson*) oldloc->fListOfPeople->Last();
  oldloc->fListOfPeople->RemoveLast();

  oldloc->fListOfPeople->RemoveAt(index);

  oldloc->fListOfPeople->AddAt(last,index);
  last->fIndex = index;
  oldloc->fNPeople -= 1;

  newloc->fListOfPeople->Add(P);
  newloc->fNPeople += 1;
 
  P->fCurrentLocation   = newloc;
  P->fIndex             = newloc->fListOfPeople->GetEntries();

  P->fTimeOfTravelStart = Time;
  P->fTravelStatus      = TPerson::kTraveling;

  double phi  = fRn3.Rndm(Time)*2*M_PI;
		
  P->fDx = newloc->fRadius*cos(phi);
  P->fDy = newloc->fRadius*sin(phi);
}

//-----------------------------------------------------------------------------
void TCovid19Sim::ModelMovement(int Time) {

  int nl = NLocations();

  for (int il=0; il<nl; il++) {
    TLocation* l = Location(il);
    int np      = l->fNPeople;

    for (int i=0; i<np; i++) {
      TPerson* p = l->Person(i);
//-----------------------------------------------------------------------------
// due to travel, there coul be zeroes in the end...
// skip them
//-----------------------------------------------------------------------------
      if (p == nullptr) {
	printf(" --- i = %5i, skip nullptr\n",i);
	continue;
      }
      if ( ! p->IsFreeToMove()) continue;

      if (! p->IsHealthy()) { 
//-----------------------------------------------------------------------------
// infected person free to move, assume infected people do not travel
// will need to revisit this assumption
//-----------------------------------------------------------------------------
	if (p->fTravelStatus == TPerson::kTraveling) {
					// infected person on travel
	  p->ReturnHome(&fRn3);
	}
	else {                          //  infected but not yet movement-restricted person at home location
	  p->TakeOneStep(&fRn3);
	}
      }
      else {
//-----------------------------------------------------------------------------
// person is healthy
//-----------------------------------------------------------------------------
	if (p->fTravelStatus == TPerson::kHome) {
	  double x  = fRn3.Rndm(i);
	  if (x < fTravelProb) {
	    StartTravel(p,Time);
	  }
	  else                 p->TakeOneStep(&fRn3);
	}
	else {
//-----------------------------------------------------------------------------
// healthy person is traveling
//-----------------------------------------------------------------------------
	  float dt = TimeInDays(Time-p->fTimeOfTravelStart);

	  if (dt > fTravelPeriod) {
	    p->ReturnHome(&fRn3);
	  }
	  else {
	    p->TakeOneStep(&fRn3);
	  }
	}
      }
    }
  }
}

//-----------------------------------------------------------------------------
void TCovid19Sim::Run(int NTimeSteps) {
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
