
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
#include "TFile.h"
#include "TMarker.h"
#include <boost/assign/list_of.hpp>
#include <map>
#include <iostream>
#include "CbmAnaJpsiSuperEventReport.h"
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

CbmAnaJpsiSuperEventReport::CbmAnaJpsiSuperEventReport(): 
		CbmSimulationReport(),
		fHMSuperEvent(),
		fHMEventByEvent()
{
	SetReportName("jpsi_superevent_qa");
}

CbmAnaJpsiSuperEventReport::~CbmAnaJpsiSuperEventReport()
{
}

void CbmAnaJpsiSuperEventReport::Create(
		const string& fileEventByEvent,
		const string& fileSuperEvent,
		const string& outputDir)
{
	fHMSuperEvent = new CbmHistManager();
	TFile* fileSE = new TFile(fileSuperEvent.c_str());
	fHMSuperEvent->ReadFromFile(fileSE);

	fHMEventByEvent = new CbmHistManager();
	TFile* fileEbE = new TFile(fileEventByEvent.c_str());
	fHMEventByEvent->ReadFromFile(fileEbE);

	SetOutputDir(outputDir);
	CreateReports();
}
void CbmAnaJpsiSuperEventReport::Create()
{
   //Out().precision(3);
   Out() << R()->DocumentBegin();
   Out() << R()->Title(0, GetTitle());

   PrintCanvases();

   Out() << R()->DocumentEnd();
}

void CbmAnaJpsiSuperEventReport::Draw()
{
	SetDefaultDrawStyle();

	int nofSEEvents = fHMSuperEvent->H1("fh_event_number")->GetEntries();
	cout << "Number of SE events = " << nofSEEvents * nofSEEvents << endl;
	fHMSuperEvent->ScaleByPattern(".*", 1./(nofSEEvents * nofSEEvents));

	int nofEEEvents = fHMEventByEvent->H1("fh_event_number")->GetEntries();
	cout << "Number of EE events = " << nofEEEvents << endl;
	fHMEventByEvent->ScaleByPattern(".*", 1./nofEEEvents);


	Int_t nRebins = 20;
	fHMEventByEvent->RebinByPattern("fh_signal_minv.+", nRebins);
	fHMEventByEvent->RebinByPattern("fh_bg_minv.+", nRebins);
	fHMSuperEvent->RebinByPattern("fh_se_bg_minv.+", nRebins);


	fHMEventByEvent->ScaleByPattern("fh_signal_minv.+", nRebins);
	fHMEventByEvent->ScaleByPattern("fh_bg_minv.+", nRebins);
	fHMSuperEvent->ScaleByPattern("fh_se_bg_minv.+", nRebins);

	DrawComparison();

	DrawMinvSignalBg();

	SignalOverBgAllSteps();
}

void CbmAnaJpsiSuperEventReport::DrawComparison()
{
	TCanvas* c1 = CreateCanvas("jpsi_se_fh_se_ee_bg_minv_reco", "jpsi_fh_se_ee_bg_minv_reco", 900, 900);
	DrawH1(list_of((TH1*) fHMEventByEvent->H1("fh_bg_minv_reco")->Clone())((TH1*) fHMSuperEvent->H1("fh_se_bg_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiReco])->Clone()), list_of("Event-by-Event")("SuperEvent"), kLinear, kLinear, true, 0.6, 0.75, 0.99, 0.99);
	DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[kJpsiReco], 0.4, 0.89, 0.5, 0.99);


	TCanvas* c2 = CreateCanvas("jpsi_se_fh_se_ee_bg_minv_chi2prim", "jpsi_fh_se_ee_bg_minv_chi2prim", 900, 900);
	DrawH1(list_of((TH1*) fHMEventByEvent->H1("fh_bg_minv_chi2prim")->Clone())((TH1*) fHMSuperEvent->H1("fh_se_bg_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiChi2Prim])->Clone()), list_of("Event-by-Event")("SuperEvent"), kLinear, kLinear, true, 0.6, 0.75, 0.99, 0.99);
	DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[kJpsiChi2Prim], 0.4, 0.89, 0.5, 0.99);


	TCanvas* c3 = CreateCanvas("jpsi_se_fh_se_ee_bg_minv_elid", "jpsi_fh_se_ee_bg_minv_elid", 900, 900);
	DrawH1(list_of((TH1*) fHMEventByEvent->H1("fh_bg_minv_elid")->Clone())((TH1*) fHMSuperEvent->H1("fh_se_bg_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiElId])->Clone()), list_of("Event-by-Event")("SuperEvent"), kLinear, kLinear, true, 0.6, 0.75, 0.99, 0.99);
	DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[kJpsiElId], 0.4, 0.89, 0.5, 0.99);


	TCanvas* c4 = CreateCanvas("jpsi_se_fh_se_ee_bg_minv_ptcut", "jpsi_fh_se_ee_bg_minv_ptcut", 900, 900);
	DrawH1(list_of((TH1*) fHMEventByEvent->H1("fh_bg_minv_ptcut")->Clone())((TH1*) fHMSuperEvent->H1("fh_se_bg_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiPtCut])->Clone()), list_of("Event-by-Event")("SuperEvent"), kLinear, kLinear, true, 0.6, 0.75, 0.99, 0.99);
	DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[kJpsiPtCut], 0.4, 0.89, 0.5, 0.99);
}

void CbmAnaJpsiSuperEventReport::DrawMinvSignalBg()
{
  TCanvas* c1 = CreateCanvas("jpsi_se_minv_reco","jpsi_se_minv_reco",900,900);
  fHMSuperEvent->H1("fh_se_bg_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiReco])->SetMinimum(1e-9);
  DrawH1(list_of((TH1*) fHMSuperEvent->H1("fh_se_bg_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiReco])->Clone())((TH1*) fHMEventByEvent->H1("fh_signal_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiReco])->Clone()),list_of("Background SuperEvent")("Signal Event-By-Event"), kLinear, kLog, true, 0.6, 0.75, 0.99, 0.99);
  DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[kJpsiReco], 0.4, 0.89, 0.5, 0.99);

  TCanvas* c2 = CreateCanvas("jpsi_se_minv_chi2prim","jpsi_se_minv_chi2prim",900,900);
  fHMSuperEvent->H1("fh_se_bg_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiChi2Prim])->SetMinimum(1e-9);
  DrawH1(list_of((TH1*) fHMSuperEvent->H1("fh_se_bg_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiChi2Prim])->Clone())((TH1*) fHMEventByEvent->H1("fh_signal_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiChi2Prim])->Clone()),list_of("Background SuperEvent")("Signal Event-By-Event"), kLinear, kLog, true, 0.6, 0.75, 0.99, 0.99);
  DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[kJpsiChi2Prim], 0.4, 0.89, 0.5, 0.99);

  TCanvas* c3 = CreateCanvas("jpsi_se_minv_elid","jpsi_se_minv_elid",900,900);
  fHMSuperEvent->H1("fh_se_bg_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiElId])->SetMinimum(1e-9);
  DrawH1(list_of((TH1*) fHMSuperEvent->H1("fh_se_bg_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiElId])->Clone())((TH1*) fHMEventByEvent->H1("fh_signal_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiElId])->Clone()),list_of("Background SuperEvent")("Signal Event-By-Event"), kLinear, kLog, true, 0.6, 0.75, 0.99, 0.99);
  DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[kJpsiElId], 0.4, 0.89, 0.5, 0.99);

  TCanvas* c4 = CreateCanvas("jpsi_se_minv_ptcut","jpsi_se_minv_ptcut",900,900);
  fHMSuperEvent->H1("fh_se_bg_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiPtCut])->SetMinimum(1e-9);
  DrawH1(list_of((TH1*) fHMSuperEvent->H1("fh_se_bg_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiPtCut])->Clone())((TH1*) fHMEventByEvent->H1("fh_signal_minv_" + CbmAnaJpsiHist::fAnaSteps[kJpsiPtCut])->Clone()),list_of("Background SuperEvent")("Signal Event-By-Event"), kLinear, kLog, true, 0.6, 0.75, 0.99, 0.99);
  DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[kJpsiPtCut], 0.4, 0.89, 0.5, 0.99);

}

double CbmAnaJpsiSuperEventReport::SignalOverBg(
		int step)
{
  TH1D* signal_ee = (TH1D*) fHMEventByEvent->H1("fh_signal_minv_"+CbmAnaJpsiHist::fAnaSteps[step])->Clone();
  TH1D* bg_se = (TH1D*) fHMSuperEvent->H1("fh_se_bg_minv_" + CbmAnaJpsiHist::fAnaSteps[step])->Clone();

  //Create Histogram for the Gaus-Fit
  TH1D* signalFit = (TH1D*)signal_ee->Clone();
  signalFit->Fit("gaus","Q");

  //Calculate sigma and Mean
  Double_t sigmaSignal = signalFit->GetFunction("gaus")->GetParameter("Sigma");
  Double_t meanSignal = signalFit->GetFunction("gaus")->GetParameter("Mean");

  //Get the number of the Bins of Min and Max
  int signalMin = signal_ee->FindBin(meanSignal - 2.*sigmaSignal);
  int signalMax = signal_ee->FindBin(meanSignal + 2.*sigmaSignal);

  double NOfSignalEntries = 0.;
  double NOfBgEntries = 0.;

  //sum up all the bins
  for (int i=signalMin; i<=signalMax; i++)
  {
	  NOfSignalEntries += signal_ee->GetBinContent(i);
	  NOfBgEntries += bg_se->GetBinContent(i);
  }

  //Calculate Signal/Background
  if (NOfBgEntries <= 0.)	  {
	  return 0.;
  } else
  {
  double sOverBg = NOfSignalEntries / NOfBgEntries;
  return sOverBg;
  }

}

void CbmAnaJpsiSuperEventReport::SignalOverBgAllSteps()
{
	TCanvas *c1 = CreateCanvas("jpsi_fh_se_SignalOverBg_allAnaSteps","jpsi_fh_se_SignalOverBg_allAnaSteps",600,600);
	fHMSuperEvent->Create1<TH1D>("fh_se_SignalOverBg_allAnaSteps","fh_se_SignalOverBg_allAnaSteps;AnaSteps;S/Bg",4,0,4);


	for (int i=0; i<CbmAnaJpsiHist::fNofAnaSteps-2;i++)
	{
		double nstep = i + 0.5;
		double SOverBg = SignalOverBg(i+2);
		fHMSuperEvent->H1("fh_se_SignalOverBg_allAnaSteps")->Fill(nstep,SOverBg);
		gPad->SetLogy();
	}

	fHMSuperEvent->H1("fh_se_SignalOverBg_allAnaSteps")->GetXaxis()->SetLabelSize(0.06);
	   int x = 1;
	   for (Int_t step = 2; step < CbmAnaJpsiHist::fNofAnaSteps; step++){
		   fHMSuperEvent->H1("fh_se_SignalOverBg_allAnaSteps")->GetXaxis()->SetBinLabel(x, CbmAnaJpsiHist::fAnaStepsLatex[step].c_str());
	      x++;
	   }

	DrawH1(fHMSuperEvent->H1("fh_se_SignalOverBg_allAnaSteps"));
}


ClassImp(CbmAnaJpsiSuperEventReport)

