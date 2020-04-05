///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////
#include "covid19/sim/TCovid19Sim.hh"

ClassImp(TCovid19Sim)

//-----------------------------------------------------------------------------
TCovid19Sim::TCovid19Sim(int NDays) {

  fNDays               = NDays;

  fDrMin               = 0.01;
  fInfectionProb       = 0.05;

  fHospitalizationProb = 0.2;    // claim that 20% need to be hospitalized
  fDeathProb           = 0.02;   // out of infected and accepted to the hospital
  fDeathAtHomeProb     = 0.06;   // out of infected and quarantined at home
  fTravelProb          = 1.e-4;
  fTravelPeriod        = 3. ;    // 3. // in days

  fListOfLocations = new TObjArray(20);

  fDataRecord      = new DataRecord_t[fNDays];


  fSleepTime       = 200;

  fCanvas          = new TCanvas("nut", "FirstSession",900,900);
}

//-----------------------------------------------------------------------------
TCovid19Sim::~TCovid19Sim() {
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
    else if (P->fHealthStatus == TPerson::kSymptomatic) {
//-----------------------------------------------------------------------------
// person with symptoms: either free-to-move, or hospitalized or quarantined
//-----------------------------------------------------------------------------
      if (P->IsFreeToMove()) {
				// shows symptoms but not hospitalized/quarantined yet
	float dt = TimeInDays(Time-P->fOnsetTime);
	if (dt > P->fSymptomaticPeriod) {
//-----------------------------------------------------------------------------
// 20% gets hospitalized , if there is enough beds
//-----------------------------------------------------------------------------
	  double p_hosp = fRn3.Rndm(-1);
	  if (p_hosp < fHospitalizationProb) {
	    if (this->fNHospitalized < this->fMaxNHospitalized) {
	      P->fTimeOfHospitalization = Time;
	      P->fMovementStatus        = TPerson::kHospitalized;
	    
	      this->fNHospitalized     += 1;
	    }
	  }
	  else {
//-----------------------------------------------------------------------------
// could not get hospitalized, not moving, sick at home, the clock is ticking
//-----------------------------------------------------------------------------
	    P->fMovementStatus        = TPerson::kQuarantined;
	    P->fTimeOfHospitalization = Time;
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
      else if (P->fMovementStatus == TPerson::kQuarantined) {
//-----------------------------------------------------------------------------
// person is not free to move - quarantined at home
//-----------------------------------------------------------------------------
	float dt = TimeInDays(Time - P->fTimeOfInfection);
	if (dt > P->RecoveryPeriod()) {
	  double x = fRn3.Rndm(-1);
	  if (x < DeathAtHomeProb(P)) {               // a small number
	    P->fHealthStatus = TPerson::kDead;
	    P->fTimeOfDeath  = Time;
	  }
	  else {
					// person recovers
	    P->fHealthStatus   = TPerson::kImmune;
	    P->fTimeOfRecovery = Time;
	    P->fMovementStatus = TPerson::kFreeToMove;
	  }
	}
      }
      else if (P->fMovementStatus == TPerson::kHospitalized) {
//-----------------------------------------------------------------------------
// person is hospitalized, not free to move
//-----------------------------------------------------------------------------
	float dt = TimeInDays(Time - P->fTimeOfInfection);
	if (dt > P->RecoveryPeriod()) {

	  double x = fRn3.Rndm(-1);

	  if (x < DeathInTheHospitalProb(P)) {                       // a small number
	    P->fHealthStatus   = TPerson::kDead;
	    P->fTimeOfDeath    = Time;
	  }
	  else {                                              // recovered
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
  float dx    = P1->fDx-P2->fDx;
  float dy    = P1->fDy-P2->fDy;
  float dist2 = dx*dx+dy*dy;

  if (dist < fDrMin*fDrMin2) {
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
  int np    = oldloc->fListOfPeople->GetEntries();

  oldloc->fListOfPeople->RemoveAt(index);

  if(index < np-1) {
    TPerson* last = (TPerson*) oldloc->fListOfPeople->RemoveAt(np-1);
    oldloc->fListOfPeople->AddAt(last,index);
    last->fIndex = index;
  }

  newloc->fListOfPeople->Add(P);
 
  P->fCurrentLocation   = newloc;
  
  int newindex          = newloc->fListOfPeople->GetEntries()-1;
  P->fIndex             = newindex;

  P->fTimeOfTravelStart = Time;
  P->fTravelStatus      = TPerson::kTraveling;

  double phi  = fRn3.Rndm(Time)*2*M_PI;
		
  P->fDx = newloc->fRadius*cos(phi);
  P->fDy = newloc->fRadius*sin(phi);

  // printf(" Time= %5i %p index %5i went on travel from loc %2i to loc %2i;",
  // 	 Time,P,index,oldloc->fIndex,newloc->fIndex);
  // printf(" new_index %5i",newindex);
  // printf(" np(oldloc),np(newloc):  %5i .. %5i\n",
  // 	 oldloc->fListOfPeople->GetEntries(),
  // 	 newloc->fListOfPeople->GetEntries());

}

//-----------------------------------------------------------------------------
void TCovid19Sim::ModelMovement(int Time) {

  int nl = NLocations();

  for (int il=0; il<nl; il++) {
    TLocation* l = Location(il);
    int np      = l->NPeople();

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

      if (p->IsIncubating()) { 
//-----------------------------------------------------------------------------
// incubating person is free to move, assume infected people do not travel
// will need to revisit this assumption
//-----------------------------------------------------------------------------
	if (p->fTravelStatus == TPerson::kHome) {
	  double x  = fRn3.Rndm(i);
	  if (x < fTravelProb)      StartTravel(p,Time);
	  else                      p->TakeOneStep(&fRn3);
	}
	else {
//-----------------------------------------------------------------------------
// incubating person on travel
//-----------------------------------------------------------------------------
	  float dt = TimeInDays(Time-p->fTimeOfTravelStart);

	  if (dt > fTravelPeriod) p->ReturnHome(Time,&fRn3);
	  else                    p->TakeOneStep(&fRn3);
	}
      }
      else if (p->IsSymptomatic()) {
//-----------------------------------------------------------------------------
// person with symptoms: do not start new travel, but do not immediately returneither
//-----------------------------------------------------------------------------
	if (p->fTravelStatus == TPerson::kHome) p->TakeOneStep(&fRn3);
	else {
	  float dt = TimeInDays(Time-p->fTimeOfTravelStart);
	  if (dt > fTravelPeriod) p->ReturnHome(Time,&fRn3);
	  else                    p->TakeOneStep(&fRn3);
	}
      }
//-----------------------------------------------------------------------------
// don't need to consider hospitalized case - not moving
//-----------------------------------------------------------------------------
      else {
//-----------------------------------------------------------------------------
// person is healthy
//-----------------------------------------------------------------------------
	if (p->fTravelStatus == TPerson::kHome) {
	  double x  = fRn3.Rndm(i);
	  if (x < fTravelProb)    StartTravel(p,Time);
	  else                    p->TakeOneStep(&fRn3);
	}
	else {
//-----------------------------------------------------------------------------
// healthy person is traveling
//-----------------------------------------------------------------------------
	  float dt = TimeInDays(Time-p->fTimeOfTravelStart);

	  if (dt > fTravelPeriod) p->ReturnHome(Time,&fRn3);
	  else                    p->TakeOneStep(&fRn3);
	}
      }
    }
  }
}

//-----------------------------------------------------------------------------
void TCovid19Sim::Run() {
//-----------------------------------------------------------------------------
// time_step - by one hour
//-----------------------------------------------------------------------------
  int   time_step;

  for (int day=0; day<fNDays; day++) {
    for (int hour=0; hour<24; hour++) {
      time_step = 24*day+hour;
      fTimeStep = time_step;
      fHour     = time_step % 24;
      fDay      = time_step / 24;
//-----------------------------------------------------------------------------
// 1. evolve the system : propagate infection at at his location
//-----------------------------------------------------------------------------
      int nl = NLocations();
      for (int iloc=0; iloc<nl; iloc++) {
	TLocation* loc = Location(iloc);
	int np = loc->NPeople();
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
//      printf(" before ModelMovement, step = %5i\n",time_step);
      ModelMovement(time_step);
//-----------------------------------------------------------------------------
// step 4: visualize end of the step
//-----------------------------------------------------------------------------
      Draw();
      gSystem->Sleep(fSleepTime);
    }
//-----------------------------------------------------------------------------
// in the end of the day, update status and recalculate all important parameters
//-----------------------------------------------------------------------------
    DataRecord_t* dr    = fDataRecord+day;
    dr->fNSusceptible   = 0;
    dr->fNIncubating    = 0;
    dr->fNSymptomatic   = 0;
    dr->fNDead          = 0;
    dr->fNImmune        = 0;

    dr->fNFreeToMove    = 0;
    dr->fNHospitalized  = 0;
    dr->fNQuarantined   = 0;

    int nloc = NLocations();
    for (int iloc=0; iloc<nloc; iloc++) {
      TLocation* loc = Location(iloc);
      DataRecord_t* ldr = loc->fDataRecord+day;
      ldr->fNSusceptible   = 0;
      ldr->fNIncubating    = 0;
      ldr->fNSymptomatic   = 0;
      ldr->fNDead          = 0;
      ldr->fNImmune        = 0;

      ldr->fNFreeToMove    = 0;
      ldr->fNHospitalized  = 0;
      ldr->fNQuarantined   = 0;

      int np = loc->NPeople();
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
      for (int i=0; i<np; i++) {
	TPerson* p = (TPerson*) loc->Person(i);
	if (p->IsSusceptible ()) ldr->fNSusceptible  += 1;
	if (p->IsIncubating  ()) ldr->fNIncubating   += 1;
	if (p->IsSymptomatic ()) ldr->fNSymptomatic  += 1;
	if (p->IsDead        ()) ldr->fNDead         += 1;
	if (p->IsImmune      ()) ldr->fNImmune       += 1;

	if (p->IsFreeToMove  ()) ldr->fNFreeToMove   += 1;
	if (p->IsHospitalized()) ldr->fNHospitalized += 1;
	if (p->IsQuarantined ()) ldr->fNQuarantined  += 1;
      }

      dr->fNSusceptible   += ldr->fNSusceptible;
      dr->fNIncubating    += ldr->fNIncubating;
      dr->fNSymptomatic   += ldr->fNSymptomatic;
      dr->fNImmune        += ldr->fNImmune;
      dr->fNDead          += ldr->fNDead;

      dr->fNFreeToMove    += ldr->fNFreeToMove;
      dr->fNHospitalized  += ldr->fNHospitalized;
      dr->fNQuarantined   += ldr->fNQuarantined;
    }
    printf(" >>> end of day %3i\n",day);
  }
//-----------------------------------------------------------------------------
// this is the end, all data are collected, ready for histogramming
//-----------------------------------------------------------------------------
}
