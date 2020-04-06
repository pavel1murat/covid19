//
#include "covid19/sim/TCovid19Sim.cc"

TCovid19Sim* sim;


//-----------------------------------------------------------------------------
void run(int NDays, int TransferInfection = 1) {
//-----------------------------------------------------------------------------
// initialization
//-----------------------------------------------------------------------------
  int const nloc = 2;

  float x0   [nloc] = { 0.25, 0.65 } ;
  float y0   [nloc] = { 0.25, 0.65 } ;
  float r    [nloc] = { 0.20, 0.20 } ;
  int   np   [nloc] = { 1000, 4000 } ;
  int   ninf [nloc] = {    1,    0 } ;
  int   color[nloc] = { kBlue+1, kGreen };


  sim = new TCovid19Sim(NDays);

  for (int i=0; i<nloc; i++) {
    TLocation* loc = new TLocation(i,x0[i],y0[i],r[i]);
    loc->Init(np[i], ninf[i], color[i], NDays, sim->RnGen());
    sim->AddLocation(loc);
  }

  TPerson::SetStep(0.002);

  sim->fDrMin         = 0.003;
  sim->fInfectionProb = 0.1;
  sim->SetSleepTime(1);

  sim->fTransferInfection = TransferInfection;

  sim->Draw();

  sim->Run();
}

//-----------------------------------------------------------------------------
void run2(int NDays) {
//-----------------------------------------------------------------------------
// initialization
//-----------------------------------------------------------------------------
  int const nloc = 2;

  float x0   [nloc] = { 0.25, 0.65 } ;
  float y0   [nloc] = { 0.25, 0.65 } ;
  float r    [nloc] = { 0.10, 0.10 } ;
  int   np   [nloc] = { 1000, 400 } ;
  int   ninf [nloc] = {    1,    0 } ;
  int   color[nloc] = { kBlue+1, kGreen };


  sim = new TCovid19Sim(NDays);

  for (int i=0; i<nloc; i++) {
    TLocation* loc = new TLocation(i,x0[i],y0[i],r[i]);
    loc->Init(np[i], ninf[i], color[i], NDays, sim->RnGen());
    sim->AddLocation(loc);
  }

  TPerson::SetStep(0.002);

  sim->fDrMin         = 0.003;
  sim->fInfectionProb = 0.1;
  sim->SetSleepTime(1);

  sim->Draw();

  sim->Run();
}

//-----------------------------------------------------------------------------
void covid19_sim() {
  run(3);
}
