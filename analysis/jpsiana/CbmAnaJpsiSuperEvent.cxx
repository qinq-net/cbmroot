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
#include <iostream>

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
	ReadCandidates();

	DoSuperEvent();
}

void CbmAnaJpsiSuperEvent::ReadCandidates()
{
	fMinusCandidates.clear();
	fPlusCandidates.clear();
	cout << "-I- ReadCandidates" << endl;
	for (Int_t iFile; iFile < fFileNames.size(); iFile++) {
		cout << "-I- Reading file No " << iFile <<  " path:" << fFileNames[iFile] << endl;

		TFile *f = new TFile(fFileNames[iFile].c_str(), "R");
		TTree* t = (TTree*)f->Get("cbmsim");
		TFolder *fd = (TFolder*)f->Get("cbmout");
		fCandidates = (TClonesArray*) fd->FindObjectAny("JpsiCandidates");
		t->SetBranchAddress(fCandidates->GetName(), &fCandidates);

		Int_t nofEvents = t->GetEntriesFast();
		cout << "-I- Number of events in file: " << nofEvents << endl;
		for (Int_t iEv = 0; iEv < nofEvents; iEv++) {
			//cout << "-I- Analysing event No " << iEv << endl;
			t->GetEntry(iEv);
			Int_t nofCandidates = fCandidates->GetEntriesFast();
			//cout << "-I- nofCandidates:" << nofCandidates << endl;
			for (Int_t iCand = 0; iCand < nofCandidates; iCand++) {
				CbmAnaJpsiCandidate* cand = (CbmAnaJpsiCandidate*) fCandidates->At(iCand);

				if (cand->fCharge < 0) {
					CbmAnaJpsiCandidate candM = CbmAnaJpsiCandidate(*cand);
					fMinusCandidates.push_back(candM);
				} else {
					CbmAnaJpsiCandidate candP = CbmAnaJpsiCandidate(*cand);
					fMinusCandidates.push_back(candP);
				}
			}
		}
	}
	cout << "-I- fMinusCandidates.size:" << fMinusCandidates.size() << endl;
	cout << "-I- fPlusCandidates.size:" << fPlusCandidates.size() << endl;
}

void CbmAnaJpsiSuperEvent::DoSuperEvent()
{
	cout << "-I- DoSuperEvent" << endl;
}


ClassImp(CbmAnaJpsiSuperEvent)
