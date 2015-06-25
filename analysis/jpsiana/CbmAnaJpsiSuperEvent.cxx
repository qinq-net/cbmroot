/*
 * CbmAnaJpsiSuperEvent.cxx
 *
 *  Created on: Jun 25, 2015
 *      Author: slebedev
 */

#include "CbmAnaJpsiSuperEvent.h"
#include "TTree.h"
#include "TFolder.h"
#include "TFile.h"
#include "TH1.h"
#include <iostream>
#include "TClonesArray.h"
#include "CbmAnaJpsiKinematicParams.h"
#include "CbmAnaJpsiCuts.h"
#include "CbmAnaJpsiUtils.h"
#include "TCanvas.h"
#include "CbmDrawHist.h"
#include "CbmHistManager.h"
//#include "CbmSimulationReport.h"


using namespace std;

CbmAnaJpsiSuperEvent::CbmAnaJpsiSuperEvent() {
	// TODO Auto-generated constructor stub

}

CbmAnaJpsiSuperEvent::~CbmAnaJpsiSuperEvent() {
	// TODO Auto-generated destructor stub
}


void CbmAnaJpsiSuperEvent::Run()
{
	cout << "-I- Run" << endl;
	InitHist();

	ReadCandidates();

	DoSuperEvent();
}

void CbmAnaJpsiSuperEvent::InitHist()
{
	fMinv = new TH1D("fh_signal_minv", "fh_signal_minv;M_{ee} [GeV/c^{2}];Yield", 4000, 0.0 , 4.0);
	fh_SuperEvent_signal_minv_Reco = new TH1D("fh_SuperEvent_signal_minv_Reco", "fh_SuperEvent_signal_minv_Reco;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	fh_SuperEvent_signal_minv_Chi2Primary = new TH1D("fh_SuperEvent_signal_minv_Chi2Primary", "fh_SuperEvent_signal_minv_Chi2Primary;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	fh_SuperEvent_signal_minv_ElID = new TH1D("fh_SuperEvent_signal_minv_ElID", "fh_SuperEvent_signal_minv_ElID;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	fh_SuperEvent_signal_minv_Pt = new TH1D("fh_SuperEvent_signal_minv_Pt", "fh_SuperEvent_signal_minv_Pt;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	//CreateAnalysisStepsH1("fh_SuperEvent_signal_minv", "M_{ee} [GeV/c^{2}]", "Yield", 4000, 0 , 4.);
}

void CbmAnaJpsiSuperEvent::CreateAnalysisStepsH1(
      const string& name,
      const string& axisX,
      const string& axisY,
      double nBins,
      double min,
      double max
      )
{
   for (Int_t i = 2; i < CbmAnaJpsiHist::fNofAnaSteps; i++)
   {
      string hname = name + "_"+ CbmAnaJpsiHist::fAnaSteps[i];
      fHM->Create1<TH1D>(hname, hname+";"+axisX+";"+axisY, nBins, min, max);
   }
}


void CbmAnaJpsiSuperEvent::ReadCandidates()
{
	fMinusCandidates.clear();
	fPlusCandidates.clear();
	cout << "-I- ReadCandidates" << endl;
	cout << "-I- fFileNames.size:" << fFileNames.size() << endl;
	for (Int_t iFile = 0; iFile < fFileNames.size(); iFile++) {
		cout << "-I- Reading file No " << iFile <<  " path:" << fFileNames[iFile] << endl;
		TFile *f = new TFile(fFileNames[iFile].c_str(), "R");
		TTree* t = (TTree*)f->Get("cbmsim");
		TFolder *fd = (TFolder*)f->Get("cbmout");
		TClonesArray* candidates = (TClonesArray*) fd->FindObjectAny("JpsiCandidates");
		t->SetBranchAddress(candidates->GetName(), &candidates);

		Int_t nofEvents = t->GetEntriesFast();
		cout << "-I- Number of events in file: " << nofEvents << endl;
		for (Int_t iEv = 0; iEv < nofEvents; iEv++) {
			//cout << "-I- Analysing event No " << iEv << endl;
			t->GetEntry(iEv);
			Int_t nofCandidates = candidates->GetEntriesFast();
			//cout << "-I- nofCandidates:" << nofCandidates << endl;
			for (Int_t iCand = 0; iCand < nofCandidates; iCand++) {
				CbmAnaJpsiCandidate* cand = (CbmAnaJpsiCandidate*) candidates->At(iCand);
				if (cand->fCharge < 0) {
					CbmAnaJpsiCandidate candM = CbmAnaJpsiCandidate(*cand);
					fMinusCandidates.push_back(candM);
				} else {
					CbmAnaJpsiCandidate candP = CbmAnaJpsiCandidate(*cand);
					fPlusCandidates.push_back(candP);
				}
			}
		}
		f->Close();
		delete f;
	}
	cout << "-I- fMinusCandidates.size:" << fMinusCandidates.size() << endl;
	cout << "-I- fPlusCandidates.size:" << fPlusCandidates.size() << endl;
}

void CbmAnaJpsiSuperEvent::DoSuperEvent()
{
	cout << "-I- DoSuperEvent" << endl;

	Int_t nofMinus = fMinusCandidates.size();
	Int_t nofPlus = fPlusCandidates.size();
	for (Int_t iM = 0; iM < nofMinus; iM++) {
		if (iM % 1000 == 0) cout << 100.*iM/nofMinus << "% done" << endl;
		CbmAnaJpsiCandidate* candM = &fMinusCandidates[iM];
		for (Int_t iP = 0; iP < nofPlus; iP++) {
			CbmAnaJpsiCandidate* candP = &fPlusCandidates[iP];
			CbmAnaJpsiKinematicParams pRec = CbmAnaJpsiKinematicParams::KinematicParamsWithCandidates(candM, candP);
		    Bool_t isChi2Primary = (candM->fChi2Prim < fCuts.fChiPrimCut && candP->fChi2Prim < fCuts.fChiPrimCut);
		    Bool_t isEl = (candM->fIsElectron && candP->fIsElectron);
		    Bool_t isPtCut = (candM->fMomentum.Perp() > fCuts.fPtCut && candP->fMomentum.Perp() > fCuts.fPtCut);
		    fh_SuperEvent_signal_minv_Reco->Fill(pRec.fMinv);

		    //fHM->H1("fh_SuperEvent_signal_minv_"+CbmAnaJpsiHist::fAnaSteps[kJpsiReco])->Fill(pRec.fMinv);

			if (isChi2Primary) fh_SuperEvent_signal_minv_Chi2Primary->Fill(pRec.fMinv);
			//if (isChi2Primary) fHM->H1("fh_SuperEvent_signal_minv_"+CbmAnaJpsiHist::fAnaSteps[kJpsiChi2Prim])->Fill(pRec.fMinv);
			if (isChi2Primary && isEl) fh_SuperEvent_signal_minv_ElID->Fill(pRec.fMinv);
			//if (isChi2Primary && isEl) fHM->H1("fh_SuperEvent_signal_minv_"+CbmAnaJpsiHist::fAnaSteps[kJpsiElId])->Fill(pRec.fMinv);
			if (isChi2Primary && isEl && isPtCut) fh_SuperEvent_signal_minv_Pt->Fill(pRec.fMinv);
			//if (isChi2Primary && isEl && isPtCut) fHM->H1("fh_SuperEvent_signal_minv_"+CbmAnaJpsiHist::fAnaSteps[kJpsiPtCut])->Fill(pRec.fMinv);
		}
	}

	TCanvas* c = new TCanvas("jpsi_fh_SuperEvent_signal_minv", "jpsi_fh_SuperEvent_signal_minv", 1200, 1200);

	c->Divide(2,2);
	c->cd(1);
	gPad->SetLogy(true);
	DrawH1(fh_SuperEvent_signal_minv_Reco);
	c->cd(2);
	gPad->SetLogy(true);
	DrawH1(fh_SuperEvent_signal_minv_Chi2Primary);
	c->cd(3);
	gPad->SetLogy(true);
	DrawH1(fh_SuperEvent_signal_minv_ElID);
	c->cd(4);
	gPad->SetLogy(true);
	DrawH1(fh_SuperEvent_signal_minv_Pt);

	/*
	for (int i=2;i<CbmAnaJpsiHist::fNofAnaSteps;i++)
	{
		c->cd(i-1);
		DrawH1(H1("fh_SuperEvent_signal_minv_"+CbmAnaJpsiHist::fAnaSteps[i]));
	}*/
}


ClassImp(CbmAnaJpsiSuperEvent)
