#include <Analyze.h>
#include <iomanip>
#include <vector>

/* Draw histograms of radius, dR, a, b and b over a, on a same canvass. */

void Analyze(int geom_nb = 0)
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

/*  TCanvas *Can_R = new TCanvas("Can_nbHits","nbHits", 10, 10, 510, 510);
    TPad *cpad = new TPad("cpad","Pad", 0.1, 0.1, 0.9, 0.9);
    gPad->SetLogy(0);
    gPad->SetGridx(1);
    gPad->SetGridy(1);
    gPad->SetLeftMargin(0.1);
    gPad->SetRightMargin(0.05);
    gPad->SetBottomMargin(0.08);
    gPad->SetTopMargin(0.1);

  TH1F* frame = cpad->DrawFrame(10.,0.,40.,7000.); // Set x and y scales 270.
    frame->SetLabelSize(0.03,"X");
    frame->SetTitleSize(0.04,"X");
    frame->SetTitleOffset(0.8,"X");
    frame->SetXTitle("nbHits");
    frame->SetLabelSize(0.03,"Y");
    frame->SetTitleSize(0.04,"Y");
    frame->SetTitleOffset(1.2,"Y");
    frame->SetYTitle("number of events");
    frame->Draw("mMrRuo");

  H_nbHits->Draw("same");*/

  WriteHistos(geom_nb);

/* // Plots multiple histograms on one window
  TCanvas *c1 = new TCanvas();
  c1->Divide(2,3); // First param = column; Second = row
  c1->cd(1);
  H_Radius->SetLineColor(2);
  H_Radius->Draw();
  c1->cd(2);
  H_dR->SetLineColor(3);
  H_dR->Draw();
  c1->cd(3);
  H_aAxis->Draw();
  c1->cd(4);
  H_bAxis->Draw();
  c1->cd(5);
  H_boa->Draw();

  // Draw multiple histograms on same graph
  H_Radius->SetLineColor(1);
  H_Radius->SetLineWidth(2);
  H_Radius->Draw("same");
  
  H_aAxis->SetLineColor(2);
  H_aAxis->SetLineWidth(2);
  H_aAxis->Draw("same");

  H_bAxis->SetLineColor(3);
  H_bAxis->SetLineWidth(2);
  H_bAxis->Draw("same");

  H_boa->SetLineColor(4);
  H_boa->SetLineWidth(2);
  H_boa->Draw("same");

  H_dR->SetLineColor(5);
  H_dR->SetLineWidth(2);
  H_dR->Draw("same");
*/

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
  H_nbHits = new TH1F(HistText, HistText, 50., 0., 50.);

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

