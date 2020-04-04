//

TCovid19Sim* sim;
//-----------------------------------------------------------------------------
void run(int NDays) {
  sim = new TCovid19Sim(NDays);
  sim->Init(1000,1,2000,0);
  TPerson::SetStep(0.002);
  sim->fDrMin = 0.003;
  sim->fInfectionProb = 0.1;
  sim->SetSleepTime(1);
  sim->Run();
}
