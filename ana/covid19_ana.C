//

#include "covid19/sim/TCovid19Sim.hh"
#include "covid19/sim/TLocation.hh"

//-----------------------------------------------------------------------------
class TCovid19Ana {

  enum {
    kNValHistSets    = 1000,
    kNHealthHistSets = 1000
  };

public: 

  struct ValHist_t {
    TH1F* fSumHealth;
    TH1F* fSumMove;
  };

  struct HealthHist_t {
    TH1F* fIncub;         // incubating
    TH1F* fInfected;       // incubating+symptomatic+immune+dead
    TH1F* fPositive;       // symptomatic+immune+dead
    TH1F* fImmune;         //
    TH1F* fDead;           // 
    TH1F* fHealthy;        // incubating+immmune

    TH1F* fHosp;           // hospitalized
  };

  struct Hist_t {
    ValHist_t*     fVal   [kNValHistSets   ];
    HealthHist_t*  fHealth[kNHealthHistSets];
  } fHist;
  
  TCovid19Sim*  fSim;

  TCovid19Ana(TCovid19Sim* Sim);
  ~TCovid19Ana();

  void BookValHistograms   (ValHist_t*    Hist, int Loc);
  void BookHealthHistograms(HealthHist_t* Hist, int Loc);

  void BookHistograms();

  void FillValHistograms   (ValHist_t*    Hist, TLocation* Loc);
  void FillHealthHistograms(HealthHist_t* Hist, TLocation* Loc);
  void FillHistograms();
};





//-----------------------------------------------------------------------------
TCovid19Ana::TCovid19Ana(TCovid19Sim* Sim) {
  fSim = Sim;
  BookHistograms();
}


//-----------------------------------------------------------------------------
void TCovid19Ana::BookValHistograms(ValHist_t* Hist, int Set) {
  int ndays = fSim->fNDays;

  Hist->fSumHealth = new TH1F(Form("sum_health_%i",Set),Form("sum health states_%i",Set),ndays,0,ndays);
  Hist->fSumMove   = new TH1F(Form("sum_move_%i"  ,Set),Form("sum move   states_%i",Set),ndays,0,ndays);
}

//-----------------------------------------------------------------------------
void TCovid19Ana::BookHealthHistograms(HealthHist_t* Hist, int Set) {
  int ndays = fSim->fNDays;

  Hist->fIncub      = new TH1F(Form("incub_%i"   ,Set),Form("incub %i"   ,Set),ndays,0,ndays);
  Hist->fInfected   = new TH1F(Form("infected_%i",Set),Form("infected %i",Set),ndays,0,ndays);
  Hist->fPositive   = new TH1F(Form("positive_%i",Set),Form("positive %i",Set),ndays,0,ndays);
  Hist->fImmune     = new TH1F(Form("immune_%i"  ,Set),Form("immune %i"  ,Set),ndays,0,ndays);
  Hist->fDead       = new TH1F(Form("dead_%i"    ,Set),Form("dead %i"    ,Set),ndays,0,ndays);
  Hist->fHealthy    = new TH1F(Form("healthy_%i" ,Set),Form("healthy %i" ,Set),ndays,0,ndays);

  Hist->fHosp       = new TH1F(Form("hosp_%i"    ,Set),Form("hosp %i"    ,Set),ndays,0,ndays);
}

//-----------------------------------------------------------------------------
void TCovid19Ana::BookHistograms() {

  int book_val_hist_set[kNValHistSets];
  for (int i = 0; i<kNValHistSets; i++) book_val_hist_set[i] = 0;

  book_val_hist_set[  0] = 1;
  book_val_hist_set[  1] = 1;
  book_val_hist_set[100] = 1;

  for (int i=0; i<kNValHistSets; i++) {
    if (book_val_hist_set[i] != 0) {
      fHist.fVal[i] = new ValHist_t();
      BookValHistograms(fHist.fVal[i],  i);
    }
  }

  int book_health_hist_set[kNHealthHistSets];
  for (int i = 0; i<kNHealthHistSets; i++) book_health_hist_set[i] = 0;

  book_health_hist_set[  0] = 1;
  book_health_hist_set[  1] = 1;
  book_health_hist_set[100] = 1;

  for (int i=0; i<kNHealthHistSets; i++) {
    if (book_health_hist_set[i] != 0) {
      fHist.fHealth[i] = new HealthHist_t();
      BookHealthHistograms(fHist.fHealth[i],  i);
    }
  }
}

//-----------------------------------------------------------------------------
// 'Loc' < 0 : all
//-----------------------------------------------------------------------------
void TCovid19Ana::FillValHistograms(ValHist_t* Hist, TLocation* Loc) {

  int ndays = fSim->fNDays;			// assume < 200

  Hist->fSumHealth->Reset();
  Hist->fSumMove->Reset();

  DataRecord_t* r;
    
  for (int day=0; day<ndays; day++) {
    
    if   (Loc != nullptr) r = Loc->fDataRecord+day;
    else                  r = fSim->fDataRecord+day;

    float sum_health = r->fNSusceptible+r->fNIncubating+r->fNSymptomatic+r->fNImmune+r->fNDead;
    float sum_move   = r->fNFreeToMove+r->fNHospitalized+r->fNQuarantined;

    int bin     = day+1;
    Hist->fSumHealth->SetBinContent(bin,sum_health);
    Hist->fSumHealth->SetBinError  (bin,sqrt(sum_health));

    Hist->fSumMove->SetBinContent(bin,sum_move);
    Hist->fSumMove->SetBinError  (bin,sqrt(sum_move));
  }

}

//-----------------------------------------------------------------------------
// 'Loc' < 0 : all
//-----------------------------------------------------------------------------
void TCovid19Ana::FillHealthHistograms(HealthHist_t* Hist, TLocation* Loc) {

  int ndays = fSim->fNDays;			// assume < 200

  Hist->fIncub->Reset();    
  Hist->fInfected->Reset(); 
  Hist->fPositive->Reset(); 
  Hist->fImmune->Reset();  
  Hist->fDead->Reset();    
  Hist->fHealthy->Reset();  
		  
  DataRecord_t* r;
    
  for (int day=0; day<ndays; day++) {
    
    if   (Loc == nullptr) r = fSim->fDataRecord+day;
    else                  r = Loc->fDataRecord+day;

    float sum_health = r->fNSusceptible+r->fNIncubating+r->fNSymptomatic+r->fNImmune+r->fNDead;
    float sum_move   = r->fNFreeToMove+r->fNHospitalized+r->fNQuarantined;

    int bin     = day+1;

    float y;

    y = r->fNIncubating;
    Hist->fIncub->SetBinContent(bin,y);
    Hist->fIncub->SetBinError  (bin,sqrt(y));

    y     = r->fNIncubating+r->fNSymptomatic+r->fNImmune+r->fNDead;
    Hist->fInfected->SetBinContent(bin,y);
    Hist->fInfected->SetBinError  (bin,sqrt(y));

    y     = r->fNSymptomatic+r->fNImmune+r->fNDead;
    Hist->fPositive->SetBinContent(bin,y);
    Hist->fPositive->SetBinError  (bin,sqrt(y));

    y     = r->fNImmune;
    Hist->fImmune->SetBinContent(bin,y);
    Hist->fImmune->SetBinError  (bin,sqrt(y));

    y     = r->fNDead;
    Hist->fDead->SetBinContent(bin,y);
    Hist->fDead->SetBinError  (bin,sqrt(y));

    y     = r->fNSusceptible+r->fNImmune;
    Hist->fHealthy->SetBinContent(bin,y);
    Hist->fHealthy->SetBinError  (bin,sqrt(y));

    y     = r->fNHospitalized;
    Hist->fHosp->SetBinContent(bin,y);
    Hist->fHosp->SetBinError  (bin,sqrt(y));
  }
}

//-----------------------------------------------------------------------------
void TCovid19Ana::FillHistograms() {

  FillValHistograms(fHist.fVal[  0],fSim->Location(0));
  FillValHistograms(fHist.fVal[  1],fSim->Location(1));
  FillValHistograms(fHist.fVal[100],nullptr    );

  FillHealthHistograms(fHist.fHealth[  0],fSim->Location(0));
  FillHealthHistograms(fHist.fHealth[  1],fSim->Location(1));
  FillHealthHistograms(fHist.fHealth[100],nullptr    );

}

