/*
 * CbmAnaJpsiSuperEvent.cxx
 *
 *  Created on: Jun 25, 2015
 *      Author: slebedev
 */
#include "CbmReportElement.h"
#include "CbmAnaJpsiSuperEvent.h"
#include "TTree.h"
#include "TFolder.h"
#include "TFile.h"
#include "TH1.h"
#include "TPad.h"
#include "TF1.h"
#include "TMarker.h"
#include <boost/assign/list_of.hpp>
#include <map>
#include <iostream>
#include "TClonesArray.h"
#include "CbmAnaJpsiKinematicParams.h"
#include "CbmAnaJpsiCuts.h"
#include "CbmAnaJpsiUtils.h"
#include "CbmUtils.h"
#include "TCanvas.h"
#include "CbmDrawHist.h"
#include "CbmHistManager.h"
#include "TLatex.h"
#include "TDirectory.h"
using namespace std;

using boost::assign::list_of;
using Cbm::NumberToString;
using Cbm::Split;
using std::map;
using std::endl;
using std::cout;

CbmAnaJpsiSuperEvent::CbmAnaJpsiSuperEvent()
		: TObject(),
		fFileNames(),
		fMinusCandidates(),
		fPlusCandidates(),
		fOutputFile(""),
                fHM(NULL),
		fCuts(),
		fRunAfterPtCut(kTRUE)
{

}

CbmAnaJpsiSuperEvent::~CbmAnaJpsiSuperEvent()
{

}

void CbmAnaJpsiSuperEvent::Run()
{
	cout << "-I- Run" << endl;

	InitHist();

	ReadCandidates();

	DoSuperEvent();

	Draw();

	TFile* file = new TFile(fOutputFile.c_str(), "RECREATE");
	fHM->WriteToFile();
	file->Close();

	cout << "-I- Output file:" << fOutputFile << endl;
}

void CbmAnaJpsiSuperEvent::InitHist()
{
	fHM = new CbmHistManager();
	fHM->Create1<TH1D>("fh_se_bg_minv_reco", "fh_se_bg_minv_reco;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	fHM->Create1<TH1D>("fh_se_bg_minv_chi2prim", "fh_se_bg_minv_chi2prim;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	fHM->Create1<TH1D>("fh_se_bg_minv_elid", "fh_se_bg_minv_elid;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	fHM->Create1<TH1D>("fh_se_bg_minv_ptcut", "fh_se_bg_minv_ptcut;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);

	fHM->Create1<TH1D>("fh_se_event_number", "fh_se_event_number;a.u.;Number of events", 1, 0, 1.);

	fHM->Create1<TH1D>("fh_se_bg_participants_minv_gg", "fh_se_bg_participants_minv_gg;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	fHM->Create1<TH1D>("fh_se_bg_participants_minv_gp", "fh_se_bg_participants_minv_gg;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	fHM->Create1<TH1D>("fh_se_bg_participants_minv_go", "fh_se_bg_participants_minv_gg;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	fHM->Create1<TH1D>("fh_se_bg_participants_minv_pg", "fh_se_bg_participants_minv_gg;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	fHM->Create1<TH1D>("fh_se_bg_participants_minv_pp", "fh_se_bg_participants_minv_gg;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	fHM->Create1<TH1D>("fh_se_bg_participants_minv_po", "fh_se_bg_participants_minv_gg;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	fHM->Create1<TH1D>("fh_se_bg_participants_minv_og", "fh_se_bg_participants_minv_gg;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	fHM->Create1<TH1D>("fh_se_bg_participants_minv_op", "fh_se_bg_participants_minv_gg;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
	fHM->Create1<TH1D>("fh_se_bg_participants_minv_oo", "fh_se_bg_participants_minv_gg;M_{ee} [GeV/c^{2}];Yield", 4000, 0 , 4.);
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
		if (fd == NULL) continue;
		TClonesArray* candidates = (TClonesArray*) fd->FindObjectAny("JpsiCandidates");
		t->SetBranchAddress(candidates->GetName(), &candidates);
		Int_t nofEvents = t->GetEntriesFast();
		cout << "-I- Number of events in file: " << nofEvents << endl;
		for (Int_t iEv = 0; iEv < nofEvents; iEv++) {
			fHM->H1("fh_se_event_number")->Fill(0.5);
			t->GetEntry(iEv);
			Int_t nofCandidates = candidates->GetEntriesFast();
			//cout << "-I- nofCandidates:" << nofCandidates << endl;
			for (Int_t iCand = 0; iCand < nofCandidates; iCand++) {
				CbmAnaJpsiCandidate* cand = (CbmAnaJpsiCandidate*) candidates->At(iCand);
				if (cand->fIsMcSignalElectron) continue;

				Bool_t isPtCut = (cand->fChi2Prim < fCuts.fChiPrimCut && cand->fIsElectron && cand->fMomentum.Perp() > fCuts.fPtCut);
				Bool_t isGood = fRunAfterPtCut?(isPtCut):true;
				if (cand->fCharge < 0) {
					CbmAnaJpsiCandidate candM = CbmAnaJpsiCandidate(*cand);
					if (isGood) fMinusCandidates.push_back(candM);
				} else {
					CbmAnaJpsiCandidate candP = CbmAnaJpsiCandidate(*cand);
					if (isGood) fPlusCandidates.push_back(candP);
				}
			}
		}
		f->Close();
		delete f;
	}
	cout << "-I- fMinusCandidates.size:" << fMinusCandidates.size() << endl;
	cout << "-I- fPlusCandidates.size:" << fPlusCandidates.size() << endl;
	cout << "-I- number of events:"<< fHM->H1("fh_se_event_number")->GetEntries() << endl;
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

		    Bool_t isSignal = candP->fIsMcSignalElectron && candM->fIsMcSignalElectron;
			//Bool_t isPi0 = (candP->fIsMcPi0Electron && candM->fIsMcPi0Electron && candP->fStsMcMotherId == candM->fStsMcMotherId);
			//Bool_t isGamma = (candP->fIsMcGammaElectron && candM->fIsMcGammaElectron && candP->fStsMcMotherId == candM->fStsMcMotherId);
		    Bool_t isBG = !isSignal;// (!isGamma) && (!isPi0);// && (!(candP->fIsMcSignalElectron || candM->fIsMcSignalElectron));

		    if (!isBG) continue;
		    fHM->H1("fh_se_bg_minv_reco")->Fill(pRec.fMinv);
			if (isChi2Primary) fHM->H1("fh_se_bg_minv_chi2prim")->Fill(pRec.fMinv);
			if (isChi2Primary && isEl) fHM->H1("fh_se_bg_minv_elid")->Fill(pRec.fMinv);
			if (isChi2Primary && isEl && isPtCut){
					fHM->H1("fh_se_bg_minv_ptcut")->Fill(pRec.fMinv);

			    	if (candM->fIsMcGammaElectron) {
			    		if (candP->fIsMcGammaElectron && candP->fStsMcMotherId!=candM->fStsMcMotherId){
			    			fHM->H1("fh_se_bg_participants_minv_gg")->Fill(pRec.fMinv); //gamma + gamma
			    		} else if (candP->fIsMcPi0Electron) {
			    			fHM->H1("fh_se_bg_participants_minv_gp")->Fill(pRec.fMinv); //gamma + Pi0
			    		} else {
			    			fHM->H1("fh_se_bg_participants_minv_go")->Fill(pRec.fMinv);	//gamma + other
			    		}
			    	} else if (candM->fIsMcPi0Electron) {
			    		if (candP->fIsMcGammaElectron) {
			    			fHM->H1("fh_se_bg_participants_minv_pg")->Fill(pRec.fMinv); //pi0 + gamma
			    		} else if (candP->fIsMcPi0Electron && candP->fStsMcMotherId!=candM->fStsMcMotherId) {
			    			fHM->H1("fh_se_bg_participants_minv_pp")->Fill(pRec.fMinv); //pi0 + Pi0
			    		} else {
			    			fHM->H1("fh_se_bg_participants_minv_po")->Fill(pRec.fMinv);	//pi0 + other
			    		}
			    	} else {
			    		if (candP->fIsMcGammaElectron) {
			    			fHM->H1("fh_se_bg_participants_minv_og")->Fill(pRec.fMinv);	//other + gamma
			    		} else if (candP->fIsMcPi0Electron) {
			    		    fHM->H1("fh_se_bg_participants_minv_op")->Fill(pRec.fMinv); //other + Pi0
			    		} else {
			    		 	fHM->H1("fh_se_bg_participants_minv_oo")->Fill(pRec.fMinv);	//other + other
			    		}
			    	}
			}
		}
	}
}

void CbmAnaJpsiSuperEvent::Draw()
{

	TCanvas* c = new TCanvas("jpsi_se_bg_minv","jpsi_se_bg_minv",1200, 1200);

	fHM->RebinByPattern("fh_se_bg_minv.*", 20);
	fHM->ScaleByPattern("fh_se_bg_minv.*", 20);

	c->Divide(2,2);
	c->cd(1);
	DrawH1(fHM->H1("fh_se_bg_minv_reco"));
	DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[kJpsiReco], 0.6, 0.89, 0.7, 0.99);
	c->cd(2);
	DrawH1(fHM->H1("fh_se_bg_minv_chi2prim"));
	DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[kJpsiChi2Prim], 0.6, 0.89, 0.7, 0.99);
	c->cd(3);
	DrawH1(fHM->H1("fh_se_bg_minv_elid"));
	DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[kJpsiElId], 0.6, 0.89, 0.7, 0.99);
	c->cd(4);
	fHM->H1("fh_se_bg_minv_ptcut")->SetMinimum(1e-11);
	gPad->SetLogy();
	DrawH1(fHM->H1("fh_se_bg_minv_ptcut"));
	DrawTextOnPad(CbmAnaJpsiHist::fAnaStepsLatex[kJpsiPtCut], 0.6, 0.89, 0.7, 0.99);

}



ClassImp(CbmAnaJpsiSuperEvent)
