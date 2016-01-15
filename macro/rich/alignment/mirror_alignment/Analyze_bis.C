#include <Analyze.h>
#include <iomanip>
#include <vector>

/* Draw histograms of radius, dR, a, b and b over a, on a same canvass. */

void Analyze_bis(int geom_nb = 0)
{
  LoadLibs();

  LoadSimFiles(geom_nb);
  PrepareHistos(geom_nb);

  cbmrec = (TTree*) RecFile->Get("cbmsim");

  hits = new TClonesArray("CbmRichHit");
  cbmrec->SetBranchAddress("RichHit", &hits);
  
  rings = new TClonesArray("CbmRichRing");
  cbmrec->SetBranchAddress("RichRing", &rings);

  ringmatch = new TClonesArray("CbmRichRingMatch");
  cbmrec->SetBranchAddress("RichRingMatch", &ringmatch);

  int nEv = cbmrec->GetEntries();
  cout << "We have " << nEv << " events" << endl;

  // ------------------------------

  // Define timer

  /*TStopwatch timer;		// At the beginning
  timer.Start();*/

  /*timer.Stop();		// At the end
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime(); 
  cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl << endl;*/

  // ------------------------------

  for(Int_t iev = 0; iev < nEv; iev++) {

    cbmrec->GetEntry(iev);
    int nHits = hits->GetEntriesFast();
    int nRings = rings->GetEntriesFast();
    //int nRingMatch = ringmatch->GetEntriesFast();
    cout << endl;
    cout << "Event ID : " << iev+1 << "; nb of total Hits = " << nHits << endl;

    for (int ir = 0; ir < nRings; ir++) {
      CbmRichRing* ring = (CbmRichRing*) rings->At(ir);
      float radius = ring->GetRadius();
      if(radius<=0.){continue;}		// With ideal finder --> many rings with radius -1
      if(!(radius>0)) {continue;}	// Test if radius is NAN - if(!(radius<=1. || radius>1.))
      // cout << "For ring number: " << ir << ", radius = " << radius << endl;

      float aA = ring->GetAaxis();
      float bA = ring->GetBaxis();
      H_Radius->Fill(radius); H_aAxis->Fill(aA); H_bAxis->Fill(bA); H_boa->Fill(bA/aA);

      float CentX = ring->GetCenterX();
      float CentY = ring->GetCenterY();
      int nAllHitsInR = ring->GetNofHits();
      cout << "ir = " << ir << " ; Number of hits = " << nAllHitsInR << endl;
      H_nbHits->Fill(nAllHitsInR);

      for(int iH=0;iH<nAllHitsInR;iH++) {
        CbmRichHit* hit = (CbmRichHit*) hits->At(ring->GetHit(iH));
        double xH=hit->GetX();
        double yH=hit->GetY();
        double dR=aA-TMath::Sqrt( (CentX-xH)*(CentX-xH) + (CentY-yH)*(CentY-yH) );
        H_dR->Fill(dR);

        }
      }
    cout << "---------------------------------------------------" << endl;
    }

  H_nbHits->Draw();

  WriteHistos(geom_nb);

}

// Functions
// ------------------------------------------------------------------- //

void LoadLibs()
{
  gROOT->LoadMacro("$VMCWORKDIR/gconfig/basiclibs.C");
  basiclibs();
  gSystem->Load("libGeoBase");
  gSystem->Load("libParBase");
  gSystem->Load("libBase");
  gSystem->Load("libCbmBase");
  gSystem->Load("libCbmData");
  gSystem->Load("libField");
  gSystem->Load("libGen");
  gSystem->Load("libPassive");
  gSystem->Load("libMvd");
  gSystem->Load("libSts");
  gSystem->Load("libEcal");
  gSystem->Load("libKF");
  gSystem->Load("libRich");
  gSystem->Load("libTrd");
  gSystem->Load("libTof");
  gSystem->Load("libEve");
  gSystem->Load("libEventDisplay");
  gSystem->Load("libMinuit2"); // Nedded for rich ellipse fitter
}

// ------------------------------------------------------------------- //

void LoadSimFiles(int geom_nb)
{
  char PATH1[256];
  char PATH2[256];
  char PATH3[256];

  TString dir = "/home/jordan/Documents/CbmRoot/Mirr_Align_Sim/mirror_alignment/";
  sprintf(PATH1, "Parameters_RichGeo_jordan_%d.root", geom_nb);
  sprintf(PATH2, "Sim_RichGeo_jordan_%d.root", geom_nb);
  sprintf(PATH3, "Rec_RichGeo_jordan_%d.root", geom_nb);

  TString ParFileTxt = dir + PATH1; // Load Parameter File
  TString SimFileTxt = dir + PATH2; // Load Simulation File
  TString RecFileTxt = dir + PATH3; // Set Output File

  ParFile = new TFile(ParFileTxt);
  SimFile = new TFile(SimFileTxt);
  RecFile = new TFile(RecFileTxt);

  cout << ParFileTxt << endl;
  cout << SimFileTxt << endl;
  cout << RecFileTxt << endl;
}

// ------------------------------------------------------------------- //

void PrepareHistos(int geom_nb)
{
  sprintf(HistText, "Radius_%d", geom_nb);
  H_Radius = new TH1F(HistText, HistText, 2001, 1., 10.);

  sprintf(HistText, "dR_%d", geom_nb);
  H_dR = new TH1F(HistText, HistText, 2001, -2., 4.);

  sprintf(HistText, "a_Axis_%d", geom_nb);
  H_aAxis = new TH1F(HistText, HistText, 2001, 1., 10.);

  sprintf(HistText, "b_Axis_%d", geom_nb);
  H_bAxis = new TH1F(HistText, HistText, 2001, 0., 10.);

  sprintf(HistText, "b_over_a_%d", geom_nb);
  H_boa = new TH1F(HistText, HistText, 2001, -0.5, 1.5);

  sprintf(HistText, "nb_Hits%d", geom_nb);
  H_nbHits = new TH1F(HistText, HistText, 50, 0, 50);

  // TH2F *H_Points = new TH2F("rings","rings",2001, -100., 100.,2001, 50.,250.);
}

// ------------------------------------------------------------------- //

void WriteHistos(int geom_nb)
{
  char HistosFile[256];
  sprintf(HistosFile, "/home/jordan/Documents/CbmRoot/Mirr_Align_Sim/mirror_alignment/Histograms_%d.root", geom_nb);
  TFile *Histos = new TFile(HistosFile, "RECREATE");

  cout << endl << "Writing Histograms in the following file: " << HistosFile << endl;

  H_Radius->Write();
  H_dR->Write();
  H_aAxis->Write();
  H_bAxis->Write();
  H_boa->Write();
  H_nbHits->Write();
  
  Histos->Close();
}

