#include <iomanip>
#include <vector>

void DrawHistograms(int geom_nb = 4)
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

// --------------------------------------------------------------------------- //
// --------------------------------------------------------------------------- //

// Statistical study

  /*cout << endl << "------------------------------------" << endl;
  cout << "Radius measurements:" << endl << endl;
  for(int j=0; j<nb_of_geom_files; j++)
  {
    cout << "Mean value for radius " << j << " = " << R_s[j]->GetMean() << endl;
    cout << "Standard deviation for radius " << j << " = " << R_s[j]->GetStdDev() << endl;
    cout << "RMS for radius " << j << " = " << R_s[j]->GetRMS() << endl;
    cout << "Kurtosis for radius " << j << " = " << R_s[j]->GetKurtosis() << endl;
    cout << "Skewness for radius " << j << " = " << R_s[j]->GetSkewness() << endl << endl;
  }

  cout << endl << "------------------------------------" << endl;
  cout << "dR measurements:" << endl << endl;
  for(int j=0; j<nb_of_geom_files; j++)
  {
    cout << "Mean value for dR " << j << " = " << dR_s[j]->GetMean() << endl;
    cout << "Standard deviation for dR " << j << " = " << dR_s[j]->GetStdDev() << endl;
    cout << "RMS for dR " << j << " = " << dR_s[j]->GetRMS() << endl;
    cout << "Kurtosis for dR " << j << " = " << dR_s[j]->GetKurtosis() << endl;
    cout << "Skewness for dR " << j << " = " << dR_s[j]->GetSkewness() << endl << endl;
  }

  cout << endl << "------------------------------------" << endl;
  cout << "a Axis measurements:" << endl << endl;
  for(int j=0; j<nb_of_geom_files; j++)
  {
    cout << "Mean value for a axis " << j << " = " << aAxis_s[j]->GetMean() << endl;
    cout << "Standard deviation for a axis " << j << " = " << aAxis_s[j]->GetStdDev() << endl;
    cout << "RMS for a axis " << j << " = " << aAxis_s[j]->GetRMS() << endl;
    cout << "Kurtosis for a axis " << j << " = " << aAxis_s[j]->GetKurtosis() << endl;
    cout << "Skewness for a axis " << j << " = " << aAxis_s[j]->GetSkewness() << endl << endl;
  }

  cout << endl << "------------------------------------" << endl;
  cout << "b Axis measurements:" << endl << endl;
  for(int j=0; j<nb_of_geom_files; j++)
  {
    cout << "Mean value for b axis " << j << " = " << bAxis_s[j]->GetMean() << endl;
    cout << "Standard deviation for b axis " << j << " = " << bAxis_s[j]->GetStdDev() << endl;
    cout << "RMS for b axis " << j << " = " << bAxis_s[j]->GetRMS() << endl;
    cout << "Kurtosis for b axis " << j << " = " << bAxis_s[j]->GetKurtosis() << endl;
    cout << "Skewness for b axis " << j << " = " << bAxis_s[j]->GetSkewness() << endl << endl;
  }

  cout << endl << "------------------------------------" << endl;
  cout << "b over a measurements:" << endl << endl;
  for(int j=0; j<nb_of_geom_files; j++)
  {
    cout << "Mean value for boa " << j << " = " << boa_s[j]->GetMean() << endl;
    cout << "Standard deviation for boa " << j << " = " << boa_s[j]->GetStdDev() << endl;
    cout << "RMS for boa " << j << " = " << boa_s[j]->GetRMS() << endl;
    cout << "Kurtosis for boa " << j << " = " << boa_s[j]->GetKurtosis() << endl;
    cout << "Skewness for boa " << j << " = " << boa_s[j]->GetSkewness() << endl << endl;
  }

  cout << endl << "------------------------------------" << endl;
  cout << "nb of Hits measurements:" << endl << endl;
  for(int j=0; j<nb_of_geom_files; j++)
  {
    cout << "Mean value for nbHits " << j << " = " << nbHits_s[j]->GetMean() << endl;
    cout << "Standard deviation for nbHits " << j << " = " << nbHits_s[j]->GetStdDev() << endl;
    cout << "RMS for nbHits " << j << " = " << nbHits_s[j]->GetRMS() << endl;
    cout << "Kurtosis for nbHits " << j << " = " << nbHits_s[j]->GetKurtosis() << endl;
    cout << "Skewness for nbHits " << j << " = " << nbHits_s[j]->GetSkewness() << endl << endl;
  }

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

// Radius histograms

  TCanvas *Can_R = new TCanvas("Can_Rad","Radius", 10, 10, 510, 510);
    TPad *cpad = new TPad("cpad","Pad", 0.1, 0.1, 0.9, 0.9);
    gPad->SetLogy(0);
    gPad->SetGridx(1);
//  gPad->SetBorderMode(1); // Change the border color
//    gPad->SetBorderSize(2);
    gPad->SetGridy(1);
    gPad->SetLeftMargin(0.1);
    gPad->SetRightMargin(0.05);
//    gPad->SetBottomMargin(0.08);
//    gPad->SetTopMargin(0.1);

  TH1F* frame = cpad->DrawFrame(2.,0.,9.,50); // Set x and y scales 270.
    frame->SetLabelSize(0.03,"X");
    frame->SetTitleSize(0.04,"X");
    frame->SetTitleOffset(0.8,"X");
    frame->SetXTitle("Radius");
    frame->SetLabelSize(0.03,"Y");
    frame->SetTitleSize(0.04,"Y");
    frame->SetTitleOffset(1.2,"Y");
    frame->SetYTitle("number of events");
//  frame->GetXaxis()->SetNdivisions(505, kTRUE);
//  frame->GetYaxis()->SetNdivisions(505, kTRUE);
    frame->Draw("mMrRuo");

  TLegend* LEG= new TLegend(0.7,0.64,0.93,0.88); // Set legend position
    LEG->SetBorderSize(1);
    LEG->SetFillColor(0);
    LEG->SetMargin(0.2);
    LEG->SetTextSize(0.03);
    LEG->SetHeader("Radii histograms");
//    LEG->SetTextAlign(13);
//    LEG->SetTextColor(kBlue);

  int colorind = 0;
  TLegendEntry *e;

  for(int j1=0; j1<nb_of_geom_files; j1++)
  {
    colorind++;
    R_s[j1]->SetTitle("Radius Title");
    R_s[j1]->SetLineColor(colorind);
    R_s[j1]->SetLineWidth(2);
    R_s[j1]->Rebin(6);
    R_s[j1]->Draw("same");

    char leg[128];
    sprintf(leg, "#sigma_%d", j1);
    e = LEG->AddEntry(R_s[j1], leg, "l");
    e->SetTextColor(colorind);
  }

  LEG->Draw();

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

// b Axis histograms

  TCanvas *Can_bAxis = new TCanvas("Can_bAxis","b_Axis", 10, 10, 510, 510);
    TPad *cpad = new TPad("cpad","Pad",0.1,0.1,0.9,0.9);
    gPad->SetLogy(0);
    gPad->SetGridx(1);
    //gPad->SetBorderMode(1); // Change the border color
    gPad->SetBorderSize(2);
    gPad->SetGridy(1);
    gPad->SetRightMargin(0.07);

  TH1F* frame = cpad->DrawFrame(2.5,0.,6.,70.); // Set x and y scales
    frame->SetLabelSize(0.03,"X");
    frame->SetTitleSize(0.04,"X");
    frame->SetTitleOffset(0.8,"X");
    frame->SetXTitle("b Axis");
    frame->SetLabelSize(0.03,"Y");
    frame->SetTitleSize(0.04,"Y");
    frame->SetTitleOffset(1.2,"Y");
    frame->SetYTitle("number of events");
    frame->GetXaxis()->SetNdivisions(505, kTRUE);
    frame->GetYaxis()->SetNdivisions(505, kTRUE);
    frame->Draw("mMrRuo");

  TLegend* LEG= new TLegend(0.2,0.6,0.5,0.88); // Set legend position
    LEG->SetBorderSize(1);
    LEG->SetFillColor(0);
    LEG->SetMargin(0.2);
    LEG->SetTextSize(0.03);
    LEG->SetHeader("b Axes histograms");

  int colorind = 0;

  for(int j4=0; j4<nb_of_geom_files; j4++)
  {
    colorind++;
    bAxis_s[j4]->SetLineColor(colorind);
    bAxis_s[j4]->SetLineWidth(2);
    bAxis_s[j4]->Rebin(6);
    bAxis_s[j4]->Draw("same");

    char leg[128];
    sprintf(leg, "#sigma_%d", j4);
    LEG->AddEntry(bAxis_s[j4], leg, "l");
  }

  LEG->Draw();

// b over a histograms

  TCanvas *Can_boa = new TCanvas("Can_boa","boa", 10, 10, 510, 510);
    TPad *cpad = new TPad("cpad","Pad",0.1,0.1,0.9,0.9);
    gPad->SetLogy(0);
    gPad->SetGridx(1);
    //gPad->SetBorderMode(1); // Change the border color
    gPad->SetBorderSize(2);
    gPad->SetGridy(1);
    gPad->SetRightMargin(0.07);//

  TH1F* frame = cpad->DrawFrame(-0.1,0.,1.1,65.); // Set x and y scales
    frame->SetLabelSize(0.03,"X");
    frame->SetTitleSize(0.04,"X");
    frame->SetTitleOffset(0.8,"X");
    frame->SetXTitle("b over a");
    frame->SetLabelSize(0.03,"Y");
    frame->SetTitleSize(0.04,"Y");
    frame->SetTitleOffset(1.2,"Y");
    frame->SetYTitle("number of events");
    frame->GetXaxis()->SetNdivisions(505, kTRUE);
    frame->GetYaxis()->SetNdivisions(505, kTRUE);
    frame->Draw("mMrRuo");

  TLegend* LEG= new TLegend(0.2,0.6,0.45,0.88); // Set legend position
    LEG->SetBorderSize(1);
    LEG->SetFillColor(0);
    LEG->SetMargin(0.2);
    LEG->SetTextSize(0.03);
    LEG->SetHeader("b over a histograms");

  int colorind = 0;

  for(int j5=0; j5<nb_of_geom_files; j5++)
  {
    colorind++;
    boa_s[j5]->SetLineColor(colorind);
    boa_s[j5]->SetLineWidth(2);
    boa_s[j5]->Rebin(6);
    boa_s[j5]->Draw("same");

    char leg[128];
    sprintf(leg, "#sigma_%d", j5);
    LEG->AddEntry(boa_s[j5], leg, "l");
  }

  LEG->Draw();

// nb of Hits histograms

  TCanvas *Can_nbHits = new TCanvas("Can_nbHits","nbHits", 10, 10, 510, 510);
    TPad *cpad = new TPad("cpad","Pad",0.1,0.1,0.9,0.9);
    gPad->SetLogy(0);
    gPad->SetGridx(1);
    //gPad->SetBorderMode(1); // Change the border color
    gPad->SetBorderSize(2);
    gPad->SetGridy(1);
    gPad->SetRightMargin(0.07);//

  TH1F* frame = cpad->DrawFrame(5.,0.,45.,100.); // Set x and y scales
    frame->SetLabelSize(0.03,"X");
    frame->SetTitleSize(0.04,"X");
    frame->SetTitleOffset(0.8,"X");
    frame->SetXTitle("nb of Hits");
    frame->SetLabelSize(0.03,"Y");
    frame->SetTitleSize(0.04,"Y");
    frame->SetTitleOffset(1.2,"Y");
    frame->SetYTitle("number of events");
    frame->GetXaxis()->SetNdivisions(505, kTRUE);
    frame->GetYaxis()->SetNdivisions(505, kTRUE);
    frame->Draw("mMrRuo");

  TLegend* LEG= new TLegend(0.18,0.6,0.42,0.88); // Set legend position
    LEG->SetBorderSize(1);
    LEG->SetFillColor(0);
    LEG->SetMargin(0.2);
    LEG->SetTextSize(0.03);
    LEG->SetHeader("nbHits histograms");

  int colorind = 0;

  for(int j5=0; j5<nb_of_geom_files; j5++)
  {
    colorind++;
    nbHits_s[j5]->SetLineColor(colorind);
    nbHits_s[j5]->SetLineWidth(2);
    //nbHits_s[j5]->Rebin(6);
    nbHits_s[j5]->Draw("same");

    char leg[128];
    sprintf(leg, "#sigma_%d", j5);
    LEG->AddEntry(nbHits_s[j5], leg, "l");
  }

  LEG->Draw();

}
