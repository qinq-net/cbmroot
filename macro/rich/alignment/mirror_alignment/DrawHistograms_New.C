#include <iomanip>
#include <vector>

void DrawHistograms_New(int geom_nb = 4)
{
  const int nb_of_geom_files = geom_nb;	// Specify the number of geometry files here !!

  char HistosFile[256];
  char HistosName[256];

  TH1F *R_s[nb_of_geom_files];
  TH1F *dR_s[nb_of_geom_files];
  TH1F *aAxis_s[nb_of_geom_files];
  TH1F *bAxis_s[nb_of_geom_files];
  TH1F *boa_s[nb_of_geom_files];
  TH1F *nbHits_s[nb_of_geom_files];

  double Max_R = -1.;

  for(int i=0; i<nb_of_geom_files; i++)
  {
    sprintf(HistosFile, "/home/jordan/Documents/CbmRoot/Mirr_Align_Sim/mirror_alignment/Histograms_%d.root", i);
    TFile* Histograms = new TFile(HistosFile, "READ");
    // cout << HistosFile << endl;
    sprintf(HistosName, "Radius_%d", i);
    R_s[i] = (TH1F*) gDirectory->Get(HistosName);
    sprintf(HistosName, "dR_%d", i);
    dR_s[i] = (TH1F*) gDirectory->Get(HistosName);
    sprintf(HistosName, "a_Axis_%d", i);
    aAxis_s[i] = (TH1F*) gDirectory->Get(HistosName);
    sprintf(HistosName, "b_Axis_%d", i);
    bAxis_s[i] = (TH1F*) gDirectory->Get(HistosName);
    sprintf(HistosName, "b_over_a_%d", i);
    boa_s[i] = (TH1F*) gDirectory->Get(HistosName);
    sprintf(HistosName, "nb_Hits%d", i);
    nbHits_s[i] = (TH1F*) gDirectory->Get(HistosName);

    double max1 = R_s[i]->GetMaximum();
    if(Max_R < max1) {Max_R = max1;}
    else;
  }

  cout << endl << "Maximum Radius = " << Max_R << endl;

  /*gStyle->SetOptDate(0);
  gStyle->SetOptTitle(0);
  gStyle->SetOptStat(111111);

  gStyle->SetPadBorderMode(0);
  gStyle->SetCanvasColor(10);*/

  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadBottomMargin(0.15);

  //gStyle->SetPalette(1);

// --------------------------------------------------------------------------- //
// --------------------------------------------------------------------------- //

  cout << "Drawing Histograms..." << endl;

  /*TCanvas *c1 = new TCanvas("test", "test title", 10, 10, 510, 510);
  aAxis_s[4]->SetLineColor(colorind);
  aAxis_s[4]->SetLineWidth(2);
  aAxis_s[4]->Rebin(6);
  aAxis_s[4]->Draw();*/

// dR histograms

  TCanvas *Can_dR = new TCanvas("Can_dR","dR", 10, 10, 510, 510);
    TPad *cpad = new TPad("cpad","Pad",0.1,0.1,0.9,0.9);
    gPad->SetLogy(0);
    gPad->SetGridx(1);
    //gPad->SetBorderMode(1); // Change the border color
    gPad->SetBorderSize(2);
    gPad->SetGridy(1);
    gPad->SetRightMargin(0.07);//

  TH1F* frame = cpad->DrawFrame(-2.5,0.,5.,500.); // Set x and y scales
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
    frame->Draw("mMrRuo");

  TLegend* LEG= new TLegend(0.6,0.6,0.91,0.88); // Set legend position
    LEG->SetBorderSize(1);
    LEG->SetFillColor(0);
    LEG->SetMargin(0.2);
    LEG->SetTextSize(0.03);
    LEG->SetHeader("dRs histograms");

  int colorind = 0;

  for(int j2=0; j2<nb_of_geom_files; j2++)
  {
    colorind++;
    dR_s[j2]->SetLineColor(colorind);
    dR_s[j2]->SetLineWidth(2);
    dR_s[j2]->Rebin(6);
    dR_s[j2]->Draw("same");

    char leg[128];
    sprintf(leg, "#sigma_%d", j2);
    LEG->AddEntry(dR_s[j2], leg, "l");
  }

  LEG->Draw();

// a Axis histograms

  TCanvas *Can_aAxis = new TCanvas("Can_aAxis","a_Axis", 10, 10, 510, 510);
    TPad *cpad = new TPad("cpad","Pad",0.1,0.1,0.9,0.9);
    gPad->SetLogy(0);
    gPad->SetGridx(1);
    //gPad->SetBorderMode(1); // Change the border color
    gPad->SetBorderSize(2);
    gPad->SetGridy(1);
    gPad->SetRightMargin(0.07);//

  TH1F* frame = cpad->DrawFrame(2.,0.,9.,50.); // Set x and y scales
    frame->SetLabelSize(0.03,"X");
    frame->SetTitleSize(0.04,"X");
    frame->SetTitleOffset(0.8,"X");
    frame->SetXTitle("a Axis");
    frame->SetLabelSize(0.03,"Y");
    frame->SetTitleSize(0.04,"Y");
    frame->SetTitleOffset(1.2,"Y");
    frame->SetYTitle("number of events");
    frame->GetXaxis()->SetNdivisions(505, kTRUE);
    frame->GetYaxis()->SetNdivisions(505, kTRUE);
    frame->Draw("mMrRuo");

  TLegend* LEG= new TLegend(0.58,0.6,0.91,0.88); // Set legend position
    LEG->SetBorderSize(1);
    LEG->SetFillColor(0);
    LEG->SetMargin(0.2);
    LEG->SetTextSize(0.03);
    LEG->SetHeader("a Axes histograms");

  int colorind = 0;

  for(int j3=0; j3<nb_of_geom_files; j3++)
  {
    colorind++;
    aAxis_s[j3]->SetLineColor(colorind);
    aAxis_s[j3]->SetLineWidth(2);
    aAxis_s[j3]->Rebin(6);
    aAxis_s[j3]->Draw("same");

    char leg[128];
    sprintf(leg, "#sigma_%d", j3);
    LEG->AddEntry(aAxis_s[j3], leg, "l");
  }

  LEG->Draw();

}
