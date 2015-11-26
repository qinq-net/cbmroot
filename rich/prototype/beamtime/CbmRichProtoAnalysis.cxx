#include "CbmRichProtoAnalysis.h"
#include "FairRootManager.h"
#include "FairLogger.h"

#include "CbmRichTrbDefines.h"
#include "CbmRichTrbDigi.h"
#include "CbmRichTrbParam.h"

#include "CbmRichHit.h"
#include "CbmRichHitInfo.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TClonesArray.h"

#include "CbmUtils.h"

// Uncomment if you want to have excessive printout (do not execute on many events, may produce Gb's of output)
#define DEBUGPRINT

#define TESTEDTDC 0x0010

CbmRichProtoAnalysis::CbmRichProtoAnalysis()
  : FairTask(),
	fRichTrbDigi(NULL),
	fRichHits(NULL),
	fRichHitInfos(NULL),
	fRunTitle(""),
	fOutputDir(""),
        fOutHistoFile(""),
	fHM(NULL),
	numOfPairs(0),
	numOfSingleEdges(0),
	numOfSingleEdgesSync(0)
{
}

CbmRichProtoAnalysis::~CbmRichProtoAnalysis()
{
}

InitStatus CbmRichProtoAnalysis::Init()
{
	FairRootManager* manager = FairRootManager::Instance();
	fRichTrbDigi = (TClonesArray*)manager->GetObject("CbmRichTrbDigi");
	if (NULL == fRichTrbDigi) { Fatal("CbmRichProtoAnalysis::Init","No CbmRichTrbDigi array!"); }

	fRichHits = new TClonesArray("CbmRichHit", 10);
	manager->Register("RichHit", "RICH", fRichHits, kTRUE);

	fRichHitInfos = new TClonesArray("CbmRichHitInfo", 10);
	manager->Register("RichHitInfo", "RICH", fRichHitInfos, kTRUE);

	InitHist();

	return kSUCCESS;
}

void CbmRichProtoAnalysis::InitHist()
{
	fHM = new CbmHistManager();
	fHM->Create1<TH1D>("fNumOfDigis", "fNumOfDigis", 200, 0, 200);
	fHM->Create1<TH1D>("fNumOfDigisSent", "fNumOfDigis", 200, 0, 200);
}

void CbmRichProtoAnalysis::DrawHist()
{
}

void CbmRichProtoAnalysis::Exec(Option_t* option)
{
	// Get the parameters object
	CbmRichTrbParam* param = CbmRichTrbParam::Instance();

	// Empty output TClonesArray
	fRichHits->Clear();
	fRichHitInfos->Clear();

	// Save the number of messages coming into this task in this event
	Int_t nofDigisInEvent = fRichTrbDigi->GetEntries();

	fHM->H1("fNumOfDigis")->Fill(nofDigisInEvent);

	// Find the trigger (hodo)
	Bool_t hodoFound = kFALSE;
	for (Int_t i=0; i<nofDigisInEvent; i++) {
		CbmRichTrbDigi* theDigit = static_cast<CbmRichTrbDigi*>(fRichTrbDigi->At(i));

		if (theDigit->GetTDCid() == 0x0110 && theDigit->GetLeadingEdgeChannel() == 5) {
			hodoFound = kTRUE;
			//LOG(INFO) << "hodoFound = kTRUE" << FairLogger::endl;
			break;
		}

	}

	if (!hodoFound) return;

	fHM->H1("fNumOfDigisSent")->Fill(nofDigisInEvent);

	for (Int_t i=0; i<nofDigisInEvent; i++) {
		CbmRichTrbDigi* theDigit = static_cast<CbmRichTrbDigi*>(fRichTrbDigi->At(i));
		if (theDigit->GetHasLeadingEdge() && theDigit->GetHasTrailingEdge()) {
			numOfPairs++;			// Global - per run

			UInt_t curLch = theDigit->GetLeadingEdgeChannel();

			CbmRichHitInfo* hitData = param->GetRichHitInfo(theDigit->GetTDCid(), curLch);

			// This happens if the beam detector signal appears (from tdc 0x0110, 0x0111, 0x0113);
			// 0x0112 is skipped during unpacking
			if (hitData == NULL) {
				//printf ("Asking for param %04x ch %d\n", theDigit->GetTDCid(), curLch);
				//printf ("hitData == NULL\n");
				continue;
			}

			//printf (" ... %.8f\t%.8f\t%.8f\t%.8f\t", hitData->GetX(), hitData->GetY(),
			//	theDigit->GetLeadingEdgeTimeStamp(), theDigit->GetTOT());

			new( (*fRichHits)[fRichHits->GetEntriesFast()] )
				CbmRichHit(hitData->GetX(), hitData->GetY(),
				theDigit->GetLeadingEdgeTimeStamp(), theDigit->GetTOT());

			//printf (" ... ");

			UInt_t counter2 = fRichHitInfos->GetEntries();
			new((*fRichHitInfos)[counter2]) CbmRichHitInfo();
			CbmRichHitInfo* newHitInfo = static_cast<CbmRichHitInfo*>(fRichHitInfos->At(counter2));

			//printf (" ... ");

			newHitInfo->Copy(hitData);

			//printf (" ...\n");

		} else {
			if (theDigit->GetHasLeadingEdge() && !(theDigit->GetHasTrailingEdge()) && param->IsSyncChannel(theDigit->GetLeadingEdgeChannel())) {
				numOfSingleEdgesSync++;
			} else {
				numOfSingleEdges++;
			}
		}
	}

}

void CbmRichProtoAnalysis::Finish()
{
	TFile* curHistoFile = new TFile(fOutHistoFile, "UPDATE");
	LOG(INFO) << "Opening file " << fOutHistoFile << " for histograms" << FairLogger::endl;
	fHM->WriteToFile();
	curHistoFile->Close();

	UInt_t totalMessages = numOfPairs*2 + numOfSingleEdges + numOfSingleEdgesSync;

	LOG(INFO) << "[CbmRichProtoAnalysis] " << numOfPairs << " pairs; " << numOfSingleEdges << " single edges (not incl.) "
		<< numOfSingleEdgesSync  << " sync messages; Total " << totalMessages << " messages" << FairLogger::endl;

	LOG(INFO) << 100.*(Double_t)numOfPairs*2. / (Double_t)totalMessages << "%, "
		  << 100.*(Double_t)numOfSingleEdges / (Double_t)totalMessages << "%, "
		  << 100.*(Double_t)numOfSingleEdgesSync / (Double_t)totalMessages << "%" << FairLogger::endl;
}

ClassImp(CbmRichProtoAnalysis)
