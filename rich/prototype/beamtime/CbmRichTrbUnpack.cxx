#include "CbmRichTrbUnpack.h"

#include <fstream>
#include <vector>
#include <algorithm>

#include "CbmTrbIterator.h"
#include "CbmTrbRawHit.h"
#include "CbmTrbOutputHit.h"
#include "CbmRichTrbParam.h"
#include "CbmTrbCalibrator.h"
#include "FairRootManager.h"
#include "CbmRichHit.h"
#include "TClonesArray.h"
#include "CbmDrawHist.h"
#include "TFolder.h"

#include "TH1D.h"
#include "TCanvas.h"

#include "FairLogger.h"

CbmRichTrbUnpack::CbmRichTrbUnpack(const string& hldFileName) :
	fHldFileName(hldFileName)
{
   ;
}

CbmRichTrbUnpack::~CbmRichTrbUnpack()
{
	;
}

Bool_t CbmRichTrbUnpack::Init()
{
	LOG(INFO) << "CbmRichTrbUnpack::Init()" << FairLogger::endl;
	LOG(INFO) << "Input file name " << fHldFileName << endl;

	ReadInputFileToMemory();

	FairRootManager* fManager = FairRootManager::Instance();

	fRichHits = new TClonesArray("CbmRichHit");
	fManager->Register("RichHit","RICH", fRichHits, kTRUE);

	ReadEvents();

	return kTRUE;
}

Int_t CbmRichTrbUnpack::ReadEvent()
{
	LOG(DEBUG) << "Event #" << fEventNum << FairLogger::endl;
	fRichHits->Clear();


	BuildEvent(fEventNum);

	LOG(DEBUG) << "CbmRichTrbUnpack::ReadEvent : # hits in event " << fRichHits->GetEntries() << FairLogger::endl;
	for (int i = 0; i < fRichHits->GetEntries(); i++) {
	CbmRichHit* hit = (CbmRichHit*)fRichHits->At(i);
		//LOG(DEBUG) << hit->GetX() << " " << hit->GetY() << FairLogger::endl;
	}
	fEventNum++;

	if (fEventNum < fOutputReferenceHits.size()){
		return 0; // still some data
	} else {
		return 1; // no more events
	}
}

void CbmRichTrbUnpack::Close()
{
   CbmTrbCalibrator::Instance()->Save("calibration.root");
	CreateAndDrawQa();
	ClearAllBuffers();
	delete[] fDataPointer;
}

void CbmRichTrbUnpack::Reset()
{

}

void CbmRichTrbUnpack::ReadInputFileToMemory()
{
	streampos size;
	char* memblock;

	ifstream file (fHldFileName.c_str(), ios::in|ios::binary|ios::ate);
	if (file.is_open()) {
		size = file.tellg();
		memblock = new char [size];
		file.seekg (0, ios::beg);
		file.read (memblock, size);
		file.close();
		LOG(INFO) << "The entire file content is in memory (" << size/(1024*1024) << " MB)" << FairLogger::endl;
	} else {
		LOG(FATAL) << "Unable to open file " << FairLogger::endl;
	}
	fDataPointer = (void*) memblock;
	fDataSize = (UInt_t) size;
}

void CbmRichTrbUnpack::ReadEvents()
{
	CbmTrbIterator* trbIter = new CbmTrbIterator(fDataPointer, fDataSize);
	Int_t nofRawEvents = 0;
	//Int_t maxNofRawEvents = 20;
	Int_t nofEventsInBuffer = 0;
	
	// Loop through events
	while (true) {
	
	   // Try to extract next event from the Iterator. If no events left - go out of the loop
		CbmRawEvent* rawEvent = trbIter->NextEvent();
		if (rawEvent == NULL) break;

		if (nofRawEvents % 10000 == 0) LOG(INFO) << "Raw event # " << nofRawEvents << FairLogger::endl;
		//rawEvent->Print();

		// Loop over subevents
		while (true){
			CbmRawSubEvent* rawSubEvent = trbIter->NextSubEvent();
			if (rawSubEvent == NULL) break;
			//rawSubEvent->Print();
			ProcessTdc(rawSubEvent);
		}

		if (nofEventsInBuffer >= NOF_RAW_EVENTS_IN_BUFFER) {
			//CreateOutputHits();
			//BuildEvent();
			//ClearAllBuffers();
			//nofEventsInBuffer = 0;
		}

      // Go out if exceed the limit of total number of raw hits
		//if (nofRawEvents >= maxNofRawEvents){
		//	break;
		//}

		nofEventsInBuffer++;
		nofRawEvents++;
		
		//CbmTrbCalibrator::Instance()->NextRawEvent();    //TODO Quite tricky, maybe refactor this
	}

	CreateOutputHits();
	//BuildEvent();
	//ClearAllBuffers();
}

void CbmRichTrbUnpack::ProcessTdc(CbmRawSubEvent* rawSubEvent)
{
	UInt_t tdcDataIndex = 0;
	UInt_t tdcData = 0x0;
	UInt_t trbId = rawSubEvent->SubId();
	Bool_t isPmtTrb = CbmRichTrbParam::Instance()->IsPmtTrb(trbId);

	while (true) {
		tdcData = rawSubEvent->SubDataValue(tdcDataIndex);
		UInt_t tdcNofWords = (tdcData >> 16) & 0xffff;
		UInt_t tdcId = tdcData & 0xffff;
		//printf("TDC DATA tdcNofWords = %i, tdcId = 0x%04x\n", tdcNofWords, tdcId);
		if (tdcId == 0x5555) break;

		UInt_t curEpochCounter = 0;
		UInt_t prevChNum = 0;
		UInt_t prevEpochCounter = 0;
		UInt_t prevCoarseTime = 0;
		UInt_t prevFineTime = 0;
		for (UInt_t i = 0; i < tdcNofWords; i++) {
			tdcDataIndex++;
			tdcData = rawSubEvent->SubDataValue(tdcDataIndex);

			UInt_t tdcTimeDataMarker = (tdcData >> 31) & 0x1; //1 bit
			if (tdcTimeDataMarker == 0x1) { //TIME DATA
				UInt_t chNum = (tdcData >> 22) & 0x7f; // 7bits
				UInt_t fineTime = (tdcData >> 12) & 0x3ff; // 10 bits
				UInt_t edge = (tdcData >> 11) & 0x1; // 1bit
				UInt_t coarseTime = (tdcData) & 0x7ff; // 1bits

            // Give the calibrator the read fine time so that it was taken into account
            CbmTrbCalibrator::Instance()->AddFineTime(trbId, tdcId, chNum, fineTime);


				if (chNum == 0) {
					// TODO: do smth with ch0
				} else {

					if ( isPmtTrb ) {
						if (chNum%2 == 1) { // leading edge
							prevChNum = chNum;
							prevEpochCounter = curEpochCounter;
							prevCoarseTime = coarseTime;
							prevFineTime = fineTime;
						} else { // trailing edge
							if (chNum - prevChNum == 1) {
								CbmTrbRawHit* rawHit = new CbmTrbRawHit(trbId, tdcId, prevChNum,
								                                       prevEpochCounter, prevCoarseTime,
								                                       prevFineTime, chNum, curEpochCounter,
								                                       coarseTime, fineTime);
								fRawRichHits.push_back(rawHit);
								
								prevChNum = 0;
								prevEpochCounter = 0;
								prevCoarseTime = 0;
								prevFineTime = 0;
							} else {
								LOG(ERROR) << "Leading edge channel number - trailing edge channel number != 1" << FairLogger::endl;
							}
						}
					} //isPmtTrb

					if (tdcId == 0x0110) { // reference time TDC
						if (chNum == 1) {
							CbmTrbRawHit* rawHitRef = new CbmTrbRawHit(trbId, tdcId, chNum, curEpochCounter, coarseTime, fineTime, 0, 0, 0, 0);
							fRawReferenceHits.push_back(rawHitRef);
						}
					}

				}
      		LOG(DEBUG2) << "TIMEDATA chNum:" << chNum << ", fineTime:" << fineTime << ", edge:" << edge << ", coarseTime:" << coarseTime
						<< ", fullTime:" << fixed << GetFullTime(trbId, tdcId, chNum, curEpochCounter, coarseTime, fineTime) << FairLogger::endl;
				if (fineTime == 0x3ff) LOG(DEBUG) << "-ERROR- Dummy fine time registered: " << fineTime << FairLogger::endl;
			}

			UInt_t tdcMarker = (tdcData >> 29) & 0x7; //3 bits
			if (tdcMarker == 0x1) {// TDC header
				UInt_t randomCode = (tdcData >> 16) & 0xff; // 8bits
				UInt_t errorBits = (tdcData) & 0xffff; //16 bits
				//printf("TDC HEADER randomCode:0x%02x, errorBits:0x%04x\n", randomCode, errorBits);
			} else if (tdcMarker == 0x2) {// DEBUG
				UInt_t debugMode = (tdcData >> 24) & 0x1f; //5
				UInt_t debugBits = (tdcData) & 0xffffff;//24 bits
				//printf("DEBUG debugMode:%i, debugBits:0x%06x\n", debugMode, debugBits);
			} else if (tdcMarker == 0x3){ // EPOCH counter
				curEpochCounter = (tdcData) & 0xfffffff; //28 bits
				//printf("EPOCH COUNTER epochCounter:0x%07x\n", curEpochCounter);
			}
		} // for tdcNofWords
		tdcDataIndex++;
	}
}

CbmTrbOutputHit* CbmRichTrbUnpack::CreateOutputHit(CbmTrbRawHit* h)
{
	Double_t lFullTime = this->GetFullTime(h->GetTrb(), h->GetTdc(), h->GetLChannel(),
	                                       h->GetLEpoch(), h->GetLCTime(), h->GetLFTime());
	Double_t tFullTime = this->GetFullTime(h->GetTrb(), h->GetTdc(), h->GetTChannel(),
	                                       h->GetTEpoch(), h->GetTCTime(), h->GetTFTime());
	return new CbmTrbOutputHit(h->GetTrb(), h->GetTdc(), h->GetLChannel(), lFullTime, h->GetTChannel(), tFullTime);
}

Double_t CbmRichTrbUnpack::GetFullTime(UShort_t TRB, UShort_t TDC, UShort_t CH, UInt_t epoch, UShort_t coarseTime, UShort_t fineTime)
{
	Double_t coarseUnit = 5.;
	Double_t epochUnit = coarseUnit * 0x800;
	
   uint32_t trb_index = (TRB >> 4) & 0x00FF - 1;
   uint32_t tdc_index = (TDC & 0x000F);
	
	Double_t time = epoch * epochUnit + coarseTime * coarseUnit -
	               CbmTrbCalibrator::Instance()->GetFineTimeCalibrated(trb_index, tdc_index, CH, fineTime);

	return time;
}

void CbmRichTrbUnpack::CreateOutputHits()
{
	Int_t nofRichHits = fRawRichHits.size();
	fOutputRichHits.resize(nofRichHits);
	for (int i = 0; i < nofRichHits; i++){
		fOutputRichHits[i] = CreateOutputHit(fRawRichHits[i]);
	}

	Int_t nofRefHits = fRawReferenceHits.size();
	fOutputReferenceHits.resize(nofRefHits);
	for (Int_t i = 0; i < nofRefHits; i++) {
		fOutputReferenceHits[i] = CreateOutputHit(fRawReferenceHits[i]);
	}

	std::sort(fOutputRichHits.begin(), fOutputRichHits.end(), CbmTrbOutputHitLeadingFullTimeComparatorLess());
}

void CbmRichTrbUnpack::BuildEvent(Int_t refHitIndex)
{
	//Int_t nofRichHits = fOutputRichHits.size();
	//Int_t nofRefHits = fOutputReferenceHits.size();

	CbmRichTrbParam* param = CbmRichTrbParam::Instance();
	Int_t indmin, indmax;
	FindMinMaxIndex(fOutputReferenceHits[refHitIndex]->GetLFullTime(), &indmin, &indmax);
	Int_t size = indmax - indmin + 1;
	LOG(DEBUG) << "NEW EVENT, size " << size << FairLogger::endl;
	for (Int_t iH = indmin; iH <= indmax; iH++) {
		CbmTrbOutputHit* h = fOutputRichHits[iH];
		CbmRichTrbMapData* data = param->GetRichTrbMapData(h->GetTdc(), h->GetLChannel());

		AddRichHitToOutputArray(data->GetX(), data->GetY());

		LOG(DEBUG2) << data->GetX() << " " << data->GetY() << FairLogger::endl;
		LOG(DEBUG2) <<fixed << iH << " " << hex << h->GetTdc() << dec << " " << h->GetLChannel() << " " << h->GetLFullTime() << FairLogger::endl;
	}
}

void CbmRichTrbUnpack::AddRichHitToOutputArray(Double_t x, Double_t y)
{
	UInt_t counter = fRichHits->GetEntries();
	new((*fRichHits)[counter]) CbmRichHit();
	CbmRichHit* hit = (CbmRichHit*)fRichHits->At(counter);
	hit->SetX(x);
	hit->SetY(y);
}

void CbmRichTrbUnpack::FindMinMaxIndex(Double_t refTime, Int_t *indmin, Int_t *indmax)
{
	Double_t windowTime = 0.5 * EVENT_TIME_WINDOW;
	CbmTrbOutputHit* mpnt = new CbmTrbOutputHit();
	vector<CbmTrbOutputHit*>::iterator itmin, itmax;

	mpnt->SetLeadingFullTime(refTime - windowTime);
	itmin = std::lower_bound(fOutputRichHits.begin(), fOutputRichHits.end(), mpnt, CbmTrbOutputHitLeadingFullTimeComparatorLess());

	mpnt->SetLeadingFullTime(refTime + windowTime);
	itmax = std::lower_bound(fOutputRichHits.begin(), fOutputRichHits.end(), mpnt, CbmTrbOutputHitLeadingFullTimeComparatorLess()) - 1;

	*indmin = itmin - fOutputRichHits.begin();
	*indmax = itmax - fOutputRichHits.begin();
}

void CbmRichTrbUnpack::ClearAllBuffers()
{
	for (Int_t i = 0; i < fRawRichHits.size(); i++) {
		delete fRawRichHits[i];
	}
	fRawRichHits.clear();

	for (Int_t i = 0; i < fRawReferenceHits.size(); i++) {
		delete fRawReferenceHits[i];
	}
	fRawReferenceHits.clear();

	for (Int_t i = 0; i < fOutputRichHits.size(); i++) {
		delete fOutputRichHits[i];
	}
	fOutputRichHits.clear();

	for (Int_t i = 0; i < fOutputReferenceHits.size(); i++) {
		delete fOutputReferenceHits[i];
	}
	fOutputReferenceHits.clear();
}

void CbmRichTrbUnpack::FillRawHitHist(CbmTrbRawHit* rh)
{
	UShort_t trbInd = ( (rh->GetTrb() >> 4) & 0x00FF ) - 1;
	UShort_t tdcInd = (rh->GetTdc() & 0x000F);

	fhChannelEntries[trbInd][tdcInd]->Fill(rh->GetLChannel());
	fhChannelEntries[trbInd][tdcInd]->Fill(rh->GetTChannel());
	fhEpoch[trbInd][tdcInd]->Fill(rh->GetLEpoch());
	fhEpoch[trbInd][tdcInd]->Fill(rh->GetTEpoch());
	fhCoarseTime[trbInd][tdcInd]->Fill(rh->GetLCTime());
	fhCoarseTime[trbInd][tdcInd]->Fill(rh->GetTCTime());
	fhFineTime[trbInd][tdcInd]->Fill(rh->GetLFTime());
	fhFineTime[trbInd][tdcInd]->Fill(rh->GetTFTime());
}

void CbmRichTrbUnpack::FillOutputHitHist(CbmTrbOutputHit* outHit)
{
	UShort_t trbInd = ( (outHit->GetTrb() >> 4) & 0x00FF ) - 1;
	UShort_t tdcInd = (outHit->GetTdc() & 0x000F);

	fhDeltaT[trbInd][tdcInd]->Fill(outHit->GetDeltaT());
}

void CbmRichTrbUnpack::CreateAndDrawQa()
{

	TString hname, htitle;
	for (Int_t iTrb = 0; iTrb < TRB_TDC3_NUMBOARDS; iTrb++){
		for (Int_t iTdc = 0; iTdc < TRB_TDC3_NUMTDC; iTdc++){
			hname.Form("hChannelEntries_%d_%d", iTrb+1, iTdc);
			htitle.Form("hChannelEntries_%d_%d;Channel number;Entries", iTrb+1, iTdc);
			fhChannelEntries[iTrb][iTdc] = new TH1D( hname.Data(), htitle.Data(), 33, -0.5, 32.5);

			hname.Form("hEpoch_%d_%d", iTrb+1, iTdc);
			htitle.Form("hEpoch_%d_%d;Epoch counter;Entries", iTrb+1, iTdc);
			fhEpoch[iTrb][iTdc] = new TH1D(hname, htitle, 1000, 0, 0);

			hname.Form("hCoraseTime_%d_%d", iTrb+1, iTdc);
			htitle.Form("hCoraseTime_%d_%d;Coarse time;Entries", iTrb+1, iTdc);
			fhCoarseTime[iTrb][iTdc] = new TH1D(hname, htitle, 2048, -0.5, 2047.5);

			hname.Form("hFineTime_%d_%d", iTrb+1, iTdc);
			htitle.Form("hFineTime_%d_%d;Fine time;Entries", iTrb+1, iTdc);
			fhFineTime[iTrb][iTdc] = new TH1D(hname, htitle, 1024, -0.5, 1023.5);

			hname.Form("hDeltaT_%d_%d", iTrb+1, iTdc);
			htitle.Form("hDeltaT_%d_%d;DeltaT [ns];Entries", iTrb+1, iTdc);
			fhDeltaT[iTrb][iTdc] = new TH1D(hname, htitle, 200, 0, 0);
		}
	}

	for (int i = 0; i < fRawRichHits.size(); i++) {
		FillRawHitHist(fRawRichHits[i]);
	}

	for (int i = 0; i < fRawReferenceHits.size(); i++) {
		FillRawHitHist(fRawReferenceHits[i]);
	}

	for (int i = 0; i < fOutputRichHits.size(); i++) {
		FillOutputHitHist(fOutputRichHits[i]);
	}

	for (int i = 0; i < fOutputReferenceHits.size(); i++) {
		FillOutputHitHist(fOutputReferenceHits[i]);
	}

	TFolder* rootHistFolder = gROOT->GetRootFolder()->AddFolder("rich_trb_unpack_debug", "rich_trb_unpack_debug");
	TString cname;
	for (Int_t iTrb = 0; iTrb < TRB_TDC3_NUMBOARDS; iTrb++){
		cname.Form("rich_trb_unpack_debug_trb%d", iTrb+1);
		TCanvas* c = new TCanvas(cname.Data(), cname.Data(), 1250, 1000);
		c->Divide(5, TRB_TDC3_NUMTDC);
		for (Int_t iTdc = 0; iTdc < TRB_TDC3_NUMTDC; iTdc++){
			c->cd(5*iTdc + 1);
			DrawH1(fhChannelEntries[iTrb][iTdc]);
			c->cd(5*iTdc + 2);
			DrawH1(fhEpoch[iTrb][iTdc]);
			c->cd(5*iTdc + 3);
			DrawH1(fhCoarseTime[iTrb][iTdc]);
			c->cd(5*iTdc + 4);
			DrawH1(fhFineTime[iTrb][iTdc]);
			c->cd(5*iTdc + 5);
			DrawH1(fhDeltaT[iTrb][iTdc]);

			rootHistFolder->Add(c);
		}
	}
	rootHistFolder->Write();
}

ClassImp(CbmRichTrbUnpack)
