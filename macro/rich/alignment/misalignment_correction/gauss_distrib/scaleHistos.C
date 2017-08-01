#include "TH3D.h"
#include <string>

int Counter=0;
void DrawDivide(TH3D* h1, TH3D* h2);
TH3D* LoadHisto(TString rootPath, TString histoName);

void scaleHistos()
{
	TH3D *h1, *h2;
	TString histoName = "fhRingTrackDistVsXYTruematchPrimel";
	TString setupName1 = "setup_misalign_gauss_sigma_1_reco.root";
	TString rootPath1 = "/data/Sim_Outputs/Gauss_sigma_1/" + setupName1;
//	TFile* Histograms1 = new TFile(rootPath1, "READ");
//	h1 = (TH3D*) gDirectory->Get(histoName);

	string setupName2 = "setup_align_reco.root";
	string rootPath2 = "/data/Sim_Outputs/Align/" + setupName2;
//	TFile* Histograms = new TFile(rootPath2.c_str(), "READ");
//	h2 = (TH3D*) gDirectory->Get(histoName);

	h1->Draw();
/*	h1 = LoadHisto(rootPath1, histoName);
	h2 = LoadHisto(rootPath2, histoName);

	TH3D* h3 = (TH3D*) h1->Clone();
	TH3D* h4 = (TH3D*) h2->Clone("h4");
	TH3D* h5 = (TH3D*) h1->Clone("h5");
	TH3D* h6 = (TH3D*) h2->Clone("h6");
	h3->GetYaxis()->SetRangeUser(110., 200.);
	h4->GetYaxis()->SetRangeUser(110., 200.);
	h5->GetYaxis()->SetRangeUser(-200., -110.);
	h6->GetYaxis()->SetRangeUser(-200., -110.);
*/
//	DrawDivide(h3, h4);
//	DrawDivide(h5, h6);
}

TH3D* LoadHisto(TString rootPath, TString histoName)
{
	TH3D* h1;
	TFile* Histograms = new TFile(rootPath, "READ");
	h1 = (TH3D*) gDirectory->Get(histoName);
	return(h1);
}

void DrawDivide(TH3D* h1, TH3D* h2)
{
	stringstream ssName, ssTitle;
        ssName << "c" << Counter << "_name";
        ssTitle << "c" << Counter << "_title";
	TH3D* h3 = (TH3D*) h2->Clone();

	TCanvas* c = new TCanvas(ssName.str().c_str(), ssTitle.str().c_str(), 1500, 500);
	c->Divide(3,1);
	c->cd(1);
	h1->Draw("colz");
	c->cd(2);
	h2->Draw("colz");
	c->cd(3);
	h3->Divide(h1);
	h3->Draw("colz");
	Counter++;
}
