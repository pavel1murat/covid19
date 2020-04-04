//

//-----------------------------------------------------------------------------
void init(TCovid19Sim* Sim) {
  Sim->Init(1000,1,2000,0);
  TPerson::SetStep(0.002);
  Sim->fDrMin = 0.002;
  Sim->fInfectionProb = 0.1;
}
