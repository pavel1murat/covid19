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

  fTransferInfection   = 1;

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
// health status evolves only for infected people, to speed up the execution,
// the check of current health status needs to be done 
//-----------------------------------------------------------------------------
void TCovid19Sim::UpdateHealthStatus(TPerson* P, int Time) {

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
// person with symptoms: either free-to-move, or hospitalized, or quarantined
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

  if (dist2 < fDrMin*fDrMin) {
    float r            = fRn3.Rndm(Time);
    if (r < fInfectionProb*P1->fInfectivePower) {
      P2->fHealthStatus    = TPerson::kIncubating;
      P2->fTimeOfInfection = Time;

      P1->fNInfected      += 1;
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

  // remove person from his zone

  TObjArray* oldzone = oldloc->fZone[P->fZoneIX][P->fZoneIY];
  int npz = oldzone->GetEntries();

  int loc = P->fZoneIndex;
  oldzone->RemoveAt(loc);

  if (loc < npz) {
    TPerson* last = (TPerson*) oldzone->RemoveAt(npz-1);
    oldzone->AddAt(last,loc);
    last->fZoneIndex = loc;
  }

  newloc->fListOfPeople->Add(P);
 
  P->fCurrentLocation   = newloc;
  
  int newindex          = newloc->fListOfPeople->GetEntries()-1;
  P->fIndex             = newindex;

  P->fTimeOfTravelStart = Time;
  P->fTravelStatus      = TPerson::kTraveling;
  
  float  rn[2];
  fRn3.RndmArray(2,rn);
  
  P->fDx = newloc->XMax()*(2*rn[0]-1);
  P->fDy = newloc->YMax()*(2*rn[1]-1);

  // 'install' person in a new zone

  int nz   = newloc->NZones();
  float sx = 2*newloc->fXMax/nz;
  float sy = 2*newloc->fYMax/nz;

  int   ix = (P->fDx+newloc->fXMax)/sx;
  int   iy = (P->fDy+newloc->fYMax)/sy;

  TObjArray* newzone = newloc->fZone[ix][iy];
  newzone->Add(P);
  P->fZoneIndex      = newzone->GetEntries()-1;

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
	//	printf(" --- i = %5i, skip nullptr\n",i);
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
void TCovid19Sim::ProcessZone(TLocation* Loc, int Ix, int Iy, int TimeStep) {

  TObjArray* zone = Loc->Zone(Ix,Iy);
  int np1         = zone->GetEntries();

  int nz          = Loc->NZones();

  for (int i1=0; i1<np1; i1++) {
    TPerson* p1 = (TPerson*) zone->At(i1); 
    if (p1->IsInfected()) {
//-----------------------------------------------------------------------------
// p1 is infected
//-----------------------------------------------------------------------------
      for (int ix2=Ix-1; ix2<Ix+2; ix2++) {
	if ((ix2 < 0) or (ix2 >=nz)) continue;

	for (int iy2=Iy-1; iy2<Iy+2; iy2++) {
	  if ((iy2 < 0) or (iy2 >=nz)) continue;

	  TObjArray* zone2 = Loc->fZone[ix2][iy2];
	  // loop only over people int this zone
	  int np2 = zone2->GetEntries();

	  int i2min = 1;
	  if ((ix2 == Ix) && (iy2 == Iy)) i2min = i1+1;
	  
	  for (int i2=i2min+1; i2<np2; i2++) {
	    TPerson* p2 = (TPerson*) Loc->Person(i2); 
	    if  (p2->IsSusceptible()) ModelInfectionTransfer(p1,p2,TimeStep);
	  }
	}
      }
    }
    else if (p1->IsSusceptible()) {
//-----------------------------------------------------------------------------
// p1 is not infected, but can be
//-----------------------------------------------------------------------------
      for (int ix2=Ix-1; ix2<Ix+2; ix2++) {
	if ((ix2 < 0) or (ix2 >=nz)) continue;

	for (int iy2=Iy-1; iy2<Iy+2; iy2++) {
	  if ((iy2 < 0) or (iy2 >=nz)) continue;

	  TObjArray* zone2 = Loc->fZone[ix2][iy2];
	  // loop only over people int this zone
	  int np2 = zone2->GetEntries();

	  int i2min = 1;
	  if ((ix2 == Ix) && (iy2 == Iy)) i2min = i1+1;
	  
	  for (int i2=i2min+1; i2<np2; i2++) {
	    TPerson* p2 = (TPerson*) Loc->Person(i2); 
	    if  (p2->IsInfected()) ModelInfectionTransfer(p2,p1,TimeStep);
	  }
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
	int nz         = loc->NZones();
	int np         = loc->NPeople();
//-----------------------------------------------------------------------------
// step 1: evolve individual health status
//-----------------------------------------------------------------------------
	for (int i=0; i<np; i++) {
	  TPerson* p = (TPerson*) loc->Person(i);
	  if (p->IsInfected()) UpdateHealthStatus(p,time_step);
	}
//-----------------------------------------------------------------------------
// step 2: model infection transfer between the two people currently at that location
//         loop over zones
//-----------------------------------------------------------------------------
	if (fTransferInfection != 0) {
	  
	  for (int ix=0; ix<nz; ix++) {
	    for (int iy=0; iy<nz; iy++) {
	      ProcessZone(loc,ix,iy,time_step);
	    }
	  }
	}
      }
//-----------------------------------------------------------------------------
// step 3: let people move , at this point they can move between the locations as well
//-----------------------------------------------------------------------------
//      printf(" before ModelMovement, step = %5i\n",time_step);
      ModelMovement(time_step);
//-----------------------------------------------------------------------------
// step 4: end of step (hour), visualize
//-----------------------------------------------------------------------------
      // gSystem->Sleep(fSleepTime);
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

    Draw();
    printf(" >>> end of day %3i\n",day);
  }
//-----------------------------------------------------------------------------
// this is the end, all data are collected, ready for histogramming
//-----------------------------------------------------------------------------
}
