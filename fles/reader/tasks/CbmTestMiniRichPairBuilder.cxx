#include "CbmTestMiniRichPairBuilder.h"

// ROOT
#include "TClonesArray.h"
#include "TH2D.h"

// FairRoot
#include "FairLogger.h"

CbmTestMiniRichPairBuilder::CbmTestMiniRichPairBuilder() :
	FairTask()
{
}

CbmTestMiniRichPairBuilder::~CbmTestMiniRichPairBuilder()
{
}

InitStatus CbmTestMiniRichPairBuilder::Init()
{
	FairRootManager* ioman = FairRootManager::Instance();

	fInputTrbRawMessages = static_cast<TClonesArray*> (ioman->GetObject("fTrbRawMessages"));
	if (!fInputTrbRawMessages) {
		LOG(FATAL) << FairLogger::endl;
		return kERROR;
	}

	fhToT = new TH2D("fhToT", "ToT;ch;ToT, ns", 33, 0., 33., 2500, 0., 50.);

	return kSUCCESS;
}

void CbmTestMiniRichPairBuilder::Exec(Option_t* opt)
{
	LOG(DEBUG3) << "CbmTestMiniRichPairBuilder::Exec" << FairLogger::endl;

	fLeadingEdges.clear();
	fTrailingEdges.clear();

	LOG(DEBUG3) << "########################################################" << FairLogger::endl;

	Int_t nRawMess = fInputTrbRawMessages->GetEntriesFast();
	for (Int_t iRawMess=0; iRawMess<nRawMess; iRawMess++) {
		CbmTrbRawMessage* curRawMess = static_cast<CbmTrbRawMessage*>(fInputTrbRawMessages->At(iRawMess));
		//curRawMess->Print();

		// Push all leading edges into a list
		if (curRawMess->GetEdge() == 1) {
			// LEADING EDGE
			fLeadingEdges.push_back(curRawMess);
		}

		// Find reference timestamp
		if (curRawMess->GetSourceAddress() == 0xc001) {
		}
	}

	LOG(DEBUG3) << "########################################################" << FairLogger::endl;

	nRawMess = fInputTrbRawMessages->GetEntries();
	for (Int_t iRawMess=0; iRawMess<nRawMess; iRawMess++) {
		CbmTrbRawMessage* curRawMess = static_cast<CbmTrbRawMessage*>(fInputTrbRawMessages->At(iRawMess));
		//curRawMess->Print();
		if (curRawMess->GetEdge() == 0) {
			// TRAILING EDGE

			Bool_t v_LEfound = kFALSE;
			std::list<CbmTrbRawMessage*>::iterator iter = fLeadingEdges.begin();
			for ( ; iter != fLeadingEdges.end(); ++iter) {
				if ((curRawMess->GetSourceAddress() == (*iter)->GetSourceAddress()) &&
				    (curRawMess->GetChannelID() == (*iter)->GetChannelID())) {
					Double_t tot = curRawMess->GetFullTime2() - (*iter)->GetFullTime2();
					if ((tot > -20.) && (tot < 200.)) {
						// Both leading and trailing edges are accounted
						//LOG(DEBUG4) << "pair found. ToT=" << tot << FairLogger::endl;
						fhToT->Fill(curRawMess->GetChannelID(), tot);
						fLeadingEdges.erase(iter);
						v_LEfound = kTRUE;
						break;
					} /*else {
						// Wrong ToT for the current pair
					}*/
				} /*else {
					// Leading edge is of a different channel
				}*/
			}
			if (!v_LEfound) {
				fTrailingEdges.push_back(curRawMess);
			}
		}
	}

	if (fTrailingEdges.size() != 0) {
		LOG(WARNING) << fTrailingEdges.size() << " trailing edges are left without a leading edge." << FairLogger::endl;
	}

/*
	LOG(DEBUG3) << "########################################################" << FairLogger::endl;

	std::list<CbmTrbRawMessage*>::iterator iterL = fLeadingEdges.begin();

	LOG(DEBUG4) << "Remaining leading edges:" << FairLogger::endl;
	for (; iterL != fLeadingEdges.end(); ++iterL) {
		(*iterL)->Print();
	}

	std::list<CbmTrbRawMessage*>::iterator iterT = fTrailingEdges.begin();
	LOG(DEBUG4) << "Remaining trailing edges:" << FairLogger::endl;
	for (; iterT != fTrailingEdges.end(); ++iterT) {
		(*iterT)->Print();
	}
*/
	LOG(DEBUG3) << "########################################################" << FairLogger::endl;
}

void CbmTestMiniRichPairBuilder::Finish()
{
	fhToT->Write();
}

ClassImp(CbmTestMiniRichPairBuilder)
