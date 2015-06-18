
#include "CbmReportElement.h"
#include "CbmHistManager.h"
#include "CbmDrawHist.h"
#include "CbmUtils.h"
#include "TH1.h"
#include "TF1.h"
#include "TPad.h"
#include "TDirectory.h"
#include "TLatex.h"
#include "TCanvas.h"
#include "TMarker.h"
#include <boost/assign/list_of.hpp>
#include <map>
#include <iostream>
#include "CbmAnaJpsiReport.h"
#include "CbmAnaJpsiHist.h"
#include "CbmAnaJpsiCuts.h"
#include "TLine.h"
#include "TStyle.h"

using boost::assign::list_of;
using Cbm::NumberToString;
using Cbm::Split;
using std::map;
using std::endl;
using std::cout;

CbmAnaJpsiReport::CbmAnaJpsiReport():
		CbmSimulationReport()
{
	SetReportName("jpsi_qa");
}

CbmAnaJpsiReport::~CbmAnaJpsiReport()
{
}

void CbmAnaJpsiReport::Create()
{
   //Out().precision(3);
   Out() << R()->DocumentBegin();
   Out() << R()->Title(0, GetTitle());

   PrintCanvases();

   Out() << R()->DocumentEnd();
}

void CbmAnaJpsiReport::Draw()
{
	  SetDefaultDrawStyle();

	  int nofEvents = H1("fh_event_number")->GetEntries();
	  cout << "Number of events = " << nofEvents << endl;
	  HM()->ScaleByPattern(".*", 1./nofEvents);

	  //Rebin minv histograms
	  Int_t nRebins = 20;
	  HM()->RebinByPattern("fh_signal_minv.+", nRebins);
	  HM()->RebinByPattern("fh_bg_minv.+", nRebins);
	  HM()->RebinByPattern("fh_pi0_minv.+", nRebins);
	  HM()->RebinByPattern("fh_gamma_minv.+", nRebins);
	  HM()->RebinByPattern("fh_bg_truematch_minv.+", nRebins);
	  HM()->RebinByPattern("fh_bg_truematch_el_minv.+", nRebins);
	  HM()->RebinByPattern("fh_bg_truematch_notel_minv.+", nRebins);
	  HM()->RebinByPattern("fh_bg_mismatch_minv.+", nRebins);


	  Draw2DCut("fh_rich_pmt_xy");
	  DrawCutDistributions();

	  DrawMinvMismatchesAll();

	  DrawAnalysisStepsH1("fh_signal_minv",false, 5e-5, 0.1);
	  DrawAnalysisStepsH1("fh_bg_minv",false, 5e-5, 0.1);
	  DrawAnalysisStepsH1("fh_pi0_minv",false);
	  DrawAnalysisStepsH1("fh_gamma_minv",false);

	  DrawAnalysisStepsH2("fh_signal_minv_pt",false);
	  DrawAnalysisStepsH1("fh_signal_mom",false);
	  DrawAnalysisStepsH2("fh_signal_pty",true);

	  {
		  TCanvas* c = CreateCanvas("jpsi_fh_vertex_el_gamma","jpsi_fh_vertex_el_gamma",1000,1000);
		  c->Divide(2,2);
		  c->cd(1);
	  	  DrawH2(H2("fh_vertex_el_gamma_xz"));
	  	  c->cd(2);
		  DrawH2(H2("fh_vertex_el_gamma_yz"));
		  c->cd(3);
		  DrawH2(H2("fh_vertex_el_gamma_xy"));
	      c->cd(4);
		  DrawH2(H2("fh_vertex_el_gamma_rz"));
	  }

	  { // Number of BG and signal tracks after each cut
	   	  TCanvas* c = CreateCanvas("jpsi_fh_nof_tracks","jpsi_fh_nof_tracks",1000, 500);
	   	  c->Divide(2, 1);
	   	  c->cd(1);
	      DrawH1(H1("fh_nof_bg_tracks"));
	      H1("fh_nof_bg_tracks")->SetMinimum(0.0);
	      SetAnalysisStepLabels(H1("fh_nof_bg_tracks"));
	      c->cd(2);
	  	  DrawH1(H1("fh_nof_el_tracks"));
	  	  H1("fh_nof_el_tracks")->SetMinimum(0.0);
	  	  SetAnalysisStepLabels(H1("fh_nof_el_tracks"));
	  }

	  { //number of mismatches
	  	  TCanvas* c = CreateCanvas("jpsi_fh_nof_mismatches","jpsi_fh_nof_mismatches",1000,1000);
	  	  c->Divide(2,2);
	  	  c->cd(1);
	  	  DrawH1(H1("fh_nof_mismatches"));
	  	  DrawTextOnPad("All mismatches", 0.15, 0.9, 0.85, 0.99);
	  	  SetAnalysisStepLabels(H1("fh_nof_mismatches"));
	  	  c->cd(2);
	  	  DrawH1(H1("fh_nof_mismatches_rich"));
	  	  DrawTextOnPad("Mismatches in RICH", 0.15, 0.9, 0.85, 0.99);
	  	  SetAnalysisStepLabels(H1("fh_nof_mismatches_rich"));
	  	  c->cd(3);
	  	  DrawH1(H1("fh_nof_mismatches_trd"));
	  	  DrawTextOnPad("mismatches in TRD", 0.15, 0.9, 0.85, 0.99);
	  	  SetAnalysisStepLabels(H1("fh_nof_mismatches_trd"));
	  	  c->cd(4);
	  	  DrawH1(H1("fh_nof_mismatches_tof"));
	  	  DrawTextOnPad("mismatches in TOF", 0.15, 0.9, 0.85, 0.99);
	  	  SetAnalysisStepLabels(H1("fh_nof_mismatches_tof"));
	  }

	   DrawBgSource2D("fh_source_tracks", list_of("#gamma")("#pi^{0}")("#pi^{#pm}")("p")("K")("e^{#pm}_{sec}")("oth."), "Tracks per event");

		  DrawPtYEfficiencyAll();
}

void CbmAnaJpsiReport::DrawAnalysisStepsH2(
      const string& hName, bool DoDrawEfficiency)
{
	TCanvas* c = CreateCanvas(("jpsi_"+hName).c_str(), ("jpsi_"+hName).c_str(), 1200, 800);
	c->Divide(3,2);
	for (int i = 0;i< CbmAnaJpsiHist::fNofAnaSteps; i++){
		c->cd(i+1);
		string h = hName+"_"+CbmAnaJpsiHist::fAnaSteps[i];

		DrawH2(H2(h));
		DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[i], 0.6, 0.89, 0.7, 0.99);

		if (DoDrawEfficiency) DrawEfficiency(h, hName+"_"+CbmAnaJpsiHist::fAnaSteps[kJpsiMc]);
	}
}

void CbmAnaJpsiReport::DrawAnalysisStepsH1(
      const string& hName,
      bool doScale,
	  double min,
	  double max)
{
	TCanvas* c = CreateCanvas( ("jpsi_" + hName).c_str(), ("jpsi_" + hName).c_str(), 600, 600);
	vector<TH1*> h;
	vector<string> hLegend;
	for (int i = 0; i < CbmAnaJpsiHist::fNofAnaSteps; i++){
		string fullName = hName+"_"+CbmAnaJpsiHist::fAnaSteps[i];
		h.push_back( H1(fullName) );
		h[i]->SetLineWidth(2);
		h[i]->SetLineColor(CbmAnaJpsiHist::fAnaStepsColor[i]);
		if (doScale) h[i]->Scale(1. / h[i]->Integral());
		hLegend.push_back( CbmAnaJpsiHist::fAnaStepsLatex[i] );
		if (min != max) {
			h[i]->SetMinimum(min);
			h[i]->SetMaximum(max);
		}
	}
	DrawH1(h, hLegend, kLinear, kLog, true, 0.90, 0.7, 0.99, 0.99);


}

void CbmAnaJpsiReport::DrawSourceTypesH1(
      const string& hName,
      bool doScale)
{
   vector<TH1*> h;
   vector<string> hLegend;
   for (int i = 0; i < CbmAnaJpsiHist::fNofSourceTypes; i++){
      string fullName = hName+"_"+CbmAnaJpsiHist::fSourceTypes[i];
      h.push_back( H1(fullName) );
      h[i]->SetLineWidth(2);
      h[i]->SetLineColor(CbmAnaJpsiHist::fSourceTypesColor[i]);
      if (doScale){
    	  Double_t integral = h[i]->Integral();
    	  if (integral != 0.0) {
    		  h[i]->Scale(1. / h[i]->Integral());
    	  }
      }
      hLegend.push_back( CbmAnaJpsiHist::fSourceTypesLatex[i] );
   }
   DrawH1(h, hLegend, kLinear, kLog, true, 0.90, 0.7, 0.99, 0.99);
}

void CbmAnaJpsiReport::DrawCutH1(
      const string& hName,
      double cutValue,
	  bool doScale)
{
   TCanvas *c = CreateCanvas( ("jpsi_" + hName).c_str(), ("jpsi_" + hName).c_str(), 600, 600);
   DrawSourceTypesH1(hName, doScale);
   if (cutValue != -999999.){
      TLine* cutLine = new TLine(cutValue, 0.0, cutValue, 1.);
      cutLine->SetLineWidth(2);
      cutLine->Draw();
   }
}

void CbmAnaJpsiReport::Draw2DCut(
      const string& hist)
{
   TCanvas *c = CreateCanvas(("jpsi_" + hist).c_str(), ("jpsi_" + hist).c_str(), 900, 900);
   c->Divide(2,2);
   for (int i = 0; i < CbmAnaJpsiHist::fNofSourceTypes; i++){
      c->cd(i+1);
      DrawH2(H2( hist + "_"+ CbmAnaJpsiHist::fSourceTypes[i]));
      DrawTextOnPad(CbmAnaJpsiHist::fSourceTypesLatex[i], 0.6, 0.89, 0.7, 0.99);
   }
}

void CbmAnaJpsiReport::DrawCutDistributions()
{
	CbmAnaJpsiCuts cuts;
	cuts.SetDefaultCuts();
	DrawCutH1("fh_track_chi2prim", cuts.fChiPrimCut, true);
	DrawCutH1("fh_track_mom", -999999., true);
	DrawCutH1("fh_track_chi2sts", -999999.,true);
	DrawCutH1("fh_track_rapidity", -999999., true);
	DrawCutH1("fh_track_pt", cuts.fPtCut, true);
	DrawCutH1("fh_track_rich_ann", cuts.fRichAnnCut, true);
	DrawCutH1("fh_track_trd_ann", cuts.fTrdAnnCut, true);
	Draw2DCut("fh_track_tof_m2");
}

void CbmAnaJpsiReport::DrawMinvMismatches(
		int step)
{
	double trueMatch = H1("fh_bg_truematch_minv_" + CbmAnaJpsiHist::fAnaSteps[step])->GetEntries();
	double trueMatchEl = H1("fh_bg_truematch_el_minv_" + CbmAnaJpsiHist::fAnaSteps[step])->GetEntries();
	double trueMatchNotEl = H1("fh_bg_truematch_notel_minv_" + CbmAnaJpsiHist::fAnaSteps[step])->GetEntries();
	double misMatch = H1("fh_bg_mismatch_minv_" + CbmAnaJpsiHist::fAnaSteps[step])->GetEntries();
	double nofBg = H1("fh_bg_minv_" + CbmAnaJpsiHist::fAnaSteps[step])->GetEntries();

	DrawH1(list_of( H1("fh_bg_truematch_minv_" + CbmAnaJpsiHist::fAnaSteps[step]) )
		( H1("fh_bg_truematch_el_minv_" + CbmAnaJpsiHist::fAnaSteps[step]) )
		( H1("fh_bg_truematch_notel_minv_" + CbmAnaJpsiHist::fAnaSteps[step]) )
		( H1("fh_bg_mismatch_minv_" + CbmAnaJpsiHist::fAnaSteps[step]) ),
		list_of("true match (" + Cbm::NumberToString(100. * trueMatch / nofBg, 1) + "%)")
		("true match (e^{#pm}) (" + Cbm::NumberToString(100. * trueMatchEl / nofBg, 1)+ "%)")
		("true match (not e^{#pm}) (" + Cbm::NumberToString(100. * trueMatchNotEl / nofBg, 1)+ "%)")
		("mismatch (" + Cbm::NumberToString(100. * misMatch / nofBg)+ "%)"),
		kLinear, kLinear, true, 0.4, 0.7, 0.99, 0.99);

    DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[step], 0.15, 0.9, 0.35, 0.99);
}

void CbmAnaJpsiReport::DrawMinvMismatchesAll()
{
   Int_t hi = 1;
   TCanvas *c = CreateCanvas("jpsi_fh_minv_mismatches", "jpsi_fh_minv_mismatches", 1000, 1000);
   c->Divide(2,2);
   for (int step = kJpsiReco; step < CbmAnaJpsiHist::fNofAnaSteps; step++){
      c->cd(hi++);
      DrawMinvMismatches(step);
   }

   TCanvas *cPtCut = CreateCanvas("jpsi_fh_minv_mismatches_ptcut", "jpsi_fh_minv_mismatches_ptcut", 600, 600);
   DrawMinvMismatches(kJpsiPtCut);
}


void CbmAnaJpsiReport::DrawEfficiency(
		const string& histName,
		const string& McHistName)
{
	Double_t nofMCEntries =H2(McHistName)->GetEntries();
	if (nofMCEntries!=0){
 	DrawTextOnPad(Cbm::NumberToString((Double_t) H2(histName)->GetEntries() / nofMCEntries*100.)+"%",0.2,0.9,0.35,0.99);
	}
}

void CbmAnaJpsiReport::DrawPtYEfficiency(
		int step)
{
	   TH2D* Efficiency = Cbm::DivideH2(H2("fh_signal_pty_" + CbmAnaJpsiHist::fAnaSteps[step]), H2("fh_signal_pty_" + CbmAnaJpsiHist::fAnaSteps[kJpsiMc]));
	   DrawH2(Efficiency);
	   DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[step], 0.6, 0.89, 0.7, 0.99);
	   DrawEfficiency("fh_signal_pty_" + CbmAnaJpsiHist::fAnaSteps[step], "fh_signal_pty_" + CbmAnaJpsiHist::fAnaSteps[kJpsiMc]);
}

void CbmAnaJpsiReport::DrawPtYEfficiencyAll()
{

	TCanvas *c = CreateCanvas("jpsi_fh_pty_efficiency", "jpsi_fh_pty_efficiency", 1200, 800);
	   c->Divide(3,2);
	   for (int i = 1; i < CbmAnaJpsiHist::fNofAnaSteps; i++){
	      c->cd(i);
	      DrawPtYEfficiency(i);
	   }
	}

void CbmAnaJpsiReport::SetAnalysisStepLabels(
      TH1* h)
{
   h->GetXaxis()->SetLabelSize(0.06);
   int x = 1;
   for (Int_t step = 0; step < CbmAnaJpsiHist::fNofAnaSteps; step++){
      h->GetXaxis()->SetBinLabel(x, CbmAnaJpsiHist::fAnaStepsLatex[step].c_str());
      x++;
   }
}

void CbmAnaJpsiReport::DrawBgSource2D(
      const string& histName,
      const vector<string>& yLabels,
      const string& zTitle)
{
	gStyle->SetPaintTextFormat("4.2f");
	TCanvas *c1 = CreateCanvas(string("jpsi_" + histName+"_abs").c_str(), string("jpsi_"+histName+"_abs").c_str(), 900, 600);
	TH2D* habs = (TH2D*)H2(histName)->Clone();
	habs->SetStats(false);
	habs->GetZaxis()->SetTitle(zTitle.c_str());
	habs->GetXaxis()->SetRange(kJpsiReco + 1, CbmAnaJpsiHist::fNofAnaSteps);
	habs->SetMarkerSize(1.8);
	DrawH2(habs, kLinear, kLinear, kLog, "text COLZ");

	TCanvas *c2 = CreateCanvas(string("jpsi_" + histName+"_percent").c_str(), string("jpsi_" + histName+"_percent").c_str(), 900, 600);
	TH2D* hperc = (TH2D*)H2(histName)->Clone();
	hperc->SetStats(false);
	Int_t nBinsX = hperc->GetNbinsX();
	Int_t nBinsY = hperc->GetNbinsY();
	for (Int_t x = 1; x <= nBinsX; x++){
		// calculate total number of BG tracks (pairs) for a current step
		double nbg = 0.;
		for (Int_t y = 1; y <= nBinsY; y++){
			nbg += habs->GetBinContent(x, y);
		}
		Double_t sc =100. / (nbg);
		for (Int_t y = 1; y <= nBinsY; y++){
			Double_t val = sc * hperc->GetBinContent(x, y);
			hperc->SetBinContent(x, y, val);
		}
	}
	hperc->GetZaxis()->SetTitle("[%]");
	hperc->GetXaxis()->SetLabelSize(0.06);
	hperc->GetYaxis()->SetLabelSize(0.06);
	hperc->SetMarkerColor(kBlack);
	hperc->SetMarkerSize(1.8);
	hperc->GetXaxis()->SetRange(kJpsiReco + 1, CbmAnaJpsiHist::fNofAnaSteps);
	DrawH2(hperc, kLinear, kLinear, kLinear, "text COLZ");

	for (Int_t y = 1; y <= yLabels.size(); y++){
		hperc->GetYaxis()->SetBinLabel(y, yLabels[y-1].c_str());
		habs->GetYaxis()->SetBinLabel(y, yLabels[y-1].c_str());
	}

	SetAnalysisStepLabels(hperc);
	SetAnalysisStepLabels(habs);
}

ClassImp(CbmAnaJpsiReport)

