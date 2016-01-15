#include <iomanip>
#include <vector>

/* Draw rings on the PMT planes, using a 2D-histogram. */

void DrawPoints(int geom_nb = 0)
{
  LoadLibs();

  char PATH[256];
  sprintf(PATH, "/home/jordan/Desktop/Sim_RichGeo_jordan_%d.root", geom_nb); //Documents/CbmRoot/Simulations/mirror_alignment/Sim_RichGeo_jordan_%d.root", geom_nb);

  //TH1F *H_Points= new TH1F("points","points",2001, -100., 100.);
  TH2F *H_Points= new TH2F("points","points",2001, -100., 100.,2001, 60.,210.);

  TFile *SimFile= new TFile(PATH);
  TTree *cbmsim = (TTree*) SimFile->Get("cbmsim");

  TClonesArray *tracks = new TClonesArray("CbmMCTrack");
  cbmsim->SetBranchAddress("MCTrack", &tracks);
  
  TClonesArray *points = new TClonesArray("CbmRichPoint");
  cbmsim->SetBranchAddress("RichPoint", &points);

  TClonesArray *ReflPoints = new TClonesArray("CbmRichPoint");
  cbmsim->SetBranchAddress("RefPlanePoint", &ReflPoints);

  int nEv = cbmsim->GetEntries();
  cout << "We have " << nEv << " events" << endl;

  for(Int_t iev = 0; iev < nEv; iev++) {
    cbmsim->GetEntry(iev);
    int nPoints = points->GetEntriesFast();
    int nReflPoints = ReflPoints->GetEntriesFast();
    cout << "Event ID " << iev << ": nPoints = " << nPoints << ", nReflPoints = " << nReflPoints << endl;

    for(Int_t ip = 0; ip < nPoints; ip++){
      CbmRichPoint* point = (CbmRichPoint*) points->At(ip);
      if(NULL == point) continue;
      H_Points->Fill(point->GetX(), point->GetY());
     }

   }

/*  TCanvas *Can_dR = new TCanvas("Can_dR","dR", 10, 10, 610, 610);
    TPad *cpad = new TPad("cpad","Pad",0.1,0.1,0.9,0.9);
    gPad->SetLogy(0);
//    gPad->SetGridx(1);
//    gPad->SetBorderMode(1); // Change the border color
    gPad->SetBorderSize(2);
//    gPad->SetGridy(1);
    gPad->SetRightMargin(0.07);//

  TH1F* frame = cpad->DrawFrame(-80.,60,80,240); // Set x and y scales
    frame->SetLabelSize(0.03,"X");
    frame->SetTitleSize(0.04,"X");
    frame->SetTitleOffset(0.8,"X");
    frame->SetXTitle("dR");
    frame->SetLabelSize(0.03,"Y");
    frame->SetTitleSize(0.04,"Y");
    frame->SetTitleOffset(1.2,"Y");
    frame->SetYTitle("number of events");
    frame->GetXaxis()->SetNdivisions(505, kTRUE);
    frame->GetYaxis()->SetNdivisions(505, kTRUE);
    frame->Draw("mMrRuo");*/

  H_Points->Draw();

}

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
