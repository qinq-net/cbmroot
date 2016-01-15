#include <iostream>
#include <ctime>
#include <cstdlib>
#include <TF1>
#include <TRandom>

int rand_gaus()
{
  static const int SIZE = 20000;		// Generated numbers

  TRandom3 rand_Gaus;
  TH1F *h_rand = new TH1F("h_rand", "h_rand title", 50, -1., 1.);
  h_rand->FillRandom("gaus", 20000);
  TH1F *h1 = new TH1F("h_0.5","histograms, with different #sigma's", 300, -1., 1.);
  TH1F *h2 = new TH1F("h_1","#sigma_0.75 title", 300, -1., 1.);
  TH1F *h3 = new TH1F("h_3","#sigma_1. title", 300, -1., 1.);
  TH1F *h4 = new TH1F("h_4","#sigma_1.25 title", 300, -1., 1.);
  TH1F *h5 = new TH1F("h_5","#sigma_1.5 title", 300, -1., 1.);

  cout << endl;

  for (int i=0; i<SIZE; i++) {
    h1->Fill(rand_Gaus.Gaus(0,0.0573));
    h2->Fill(rand_Gaus.Gaus(0,0.1146));
    h3->Fill(rand_Gaus.Gaus(0,0.1719));
    h4->Fill(rand_Gaus.Gaus(0,0.2292));
    h5->Fill(rand_Gaus.Gaus(0,0.2865));
  }

  TCanvas *Can_dR = new TCanvas("Can_dR","dR", 10, 10, 510, 510);
    TPad *cpad = new TPad("cpad","Pad",0.1,0.1,0.9,0.9);
    gPad->SetLogy(0);
    gPad->SetGridx(1);
    //gPad->SetBorderMode(1); // Change the border color
    gPad->SetBorderSize(2);
    gPad->SetGridy(1);
    gPad->SetRightMargin(0.07);

  TH1F* frame = cpad->DrawFrame(-1.2,0.,1.2,5700.); // Set x and y scales
    frame->SetLabelSize(0.03,"X");
    frame->SetTitleSize(0.04,"X");
    frame->SetTitleOffset(0.8,"X");
    frame->SetXTitle("Misalignment angles");
    frame->SetLabelSize(0.03,"Y");
    frame->SetTitleSize(0.04,"Y");
    frame->SetTitleOffset(1.2,"Y");
    frame->SetYTitle("Nb of events");
    frame->GetXaxis()->SetNdivisions(505, kTRUE);
    frame->GetYaxis()->SetNdivisions(505, kTRUE);
    frame->Draw("mMrRuo");

  TLegend* LEG= new TLegend(0.7,0.7,0.91,0.88); // Set legend position
    LEG->SetBorderSize(1);
    LEG->SetFillColor(0);
    LEG->SetMargin(0.2);
    LEG->SetTextSize(0.03);
    LEG->SetHeader("#sigma's");

  h1->SetLineColor(1);
  h1->SetLineWidth(2);
  h1->Rebin(6);
  h1->Draw("same");
  h2->SetLineColor(2);
  h2->SetLineWidth(2);
  h2->Rebin(6);
  h2->Draw("same");
  h3->SetLineColor(3);
  h3->SetLineWidth(2);
  h3->Rebin(6);
  h3->Draw("same");
  h4->SetLineColor(4);
  h4->SetLineWidth(2);
  h4->Rebin(6);
  h4->Draw("same");
  h5->SetLineColor(5);
  h5->SetLineWidth(2);
  h5->Rebin(6);
  h5->Draw("same");

  char leg[128];
  sprintf(leg, "#sigma = 1. mrad");
  LEG->AddEntry(h1, leg, "l");
  sprintf(leg, "#sigma = 2. mrad");
  LEG->AddEntry(h2, leg, "l");
  sprintf(leg, "#sigma = 3. mrad");
  LEG->AddEntry(h3, leg, "l");
  sprintf(leg, "#sigma = 4. mrad");
  LEG->AddEntry(h4, leg, "l");
  sprintf(leg, "#sigma = 5. mrad");
  LEG->AddEntry(h5, leg, "l");
  LEG->Draw();

  /*TCanvas *c1 = new TCanvas("histos", "histos", 10, 10, 1010, 810);
c1->Divide(3,2);
c1->cd(1);
  h_rand->SetLineColor(1);
  h_rand->SetLineWidth(2);
  h_rand->Draw();
c1->cd(2);
  h1->SetLineColor(2);
  h1->SetLineWidth(2);
  h1->Draw("same");
c1->cd(3);
  h2->SetLineColor(3);
  h2->SetLineWidth(2);
  h2->Draw();
c1->cd(4);
  h3->SetLineColor(4);
  h3->SetLineWidth(2);
  h3->Draw();
c1->cd(5);
  h4->SetLineColor(6);
  h4->SetLineWidth(2);
  h4->Draw();
c1->cd(6);
  h5->SetLineColor(7);
  h5->SetLineWidth(2);
  h5->Draw();
c1->cd();*/

  return;
}
