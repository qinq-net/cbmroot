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
#include "CbmRichHitInfo.h"
#include "TClonesArray.h"
#include "CbmDrawHist.h"
#include "TFolder.h"

#include "TH1D.h"
#include "TCanvas.h"

#include "FairLogger.h"

CbmRichTrbUnpack::CbmRichTrbUnpack(TString hldFileName) :
	fHldFileName(hldFileName),
	fEventNum(0),
	fNofDoubleHits(0),
	fAnaType(kCbmRichBeamEvent)
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

	fRichHitInfos = new TClonesArray("CbmRichHitInfo");
	fManager->Register("RichHitInfo","RICH", fRichHitInfos, kTRUE);

	ReadEvents();

	fhNofRichHitsVsTrbNum = new TH2D("fhNofRichHitsVsTrbNum", "fhNofRichHitsVsTrbNum;TRB number;Nof hits in event",
			TRB_TDC3_NUMBOARDS, .5, TRB_TDC3_NUMBOARDS + .5, 65, -0.5, 64.5);
	fhDiffHitTimeEventTime = new TH1D("fhDiffHitTimeEventTime", "fhDiffHitTimeEventTime;dT [ns];Entries", 200, -50., 50.);

	return kTRUE;
}

Int_t CbmRichTrbUnpack::ReadEvent()
{
	LOG(DEBUG) << "Event #" << fEventNum << FairLogger::endl;

	fRichHits->Clear();
	fRichHitInfos->Clear();

	if (fOutputEventTimeHits.size() == 0) {
		LOG(ERROR) << "No reference time hits." << FairLogger::endl;
		return 1;
	}

	BuildEvent(fEventNum);

	fEventNum++;

	if (fEventNum < fOutputEventTimeHits.size()){
		return 0; // still some data
	} else {
		return 1; // no more events
	}
}

void CbmRichTrbUnpack::Close()
{
	if (fDrawHisto) {
		CreateAndDrawQa();
		CreateAndDrawEventBuildDisplay();
	}
	ClearAllBuffers();
}

void CbmRichTrbUnpack::Reset()
{

}

void CbmRichTrbUnpack::ReadInputFileToMemory()
{
	streampos size;
	ifstream file (fHldFileName.Data(), ios::in|ios::binary|ios::ate);
	if (file.is_open()) {
		size = file.tellg();
		fDataPointer = new Char_t[size];
		file.seekg (0, ios::beg);
		file.read (fDataPointer, size);
		file.close();
		LOG(INFO) << "The entire file content is in memory (" << size/(1024*1024) << " MB)" << FairLogger::endl;
	} else {
		LOG(FATAL) << "Unable to open file " << FairLogger::endl;
	}
	fDataSize = (UInt_t) size;
}

void CbmRichTrbUnpack::ReadEvents()
{
	CbmTrbIterator* trbIter = new CbmTrbIterator((void*)fDataPointer, fDataSize);
	Int_t nofRawEvents = 0;
	Int_t maxNofRawEvents = 2000000000;
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
		if (nofRawEvents >= maxNofRawEvents){
			break;
		}

		nofEventsInBuffer++;
		nofRawEvents++;
	}

	CreateOutputHits();
	delete[] fDataPointer;
}

void CbmRichTrbUnpack::ProcessTdc(CbmRawSubEvent* rawSubEvent)
{
	UInt_t tdcDataIndex = 0;
	UInt_t tdcData = 0x0;
	UInt_t trbId = rawSubEvent->SubId();

	while (true) {
		tdcData = rawSubEvent->SubDataValue(tdcDataIndex);
		UInt_t tdcNofWords = (tdcData >> 16) & 0xffff;
		UInt_t tdcId = tdcData & 0xffff;
		//printf("TDC DATA tdcNofWords = %i, ID = 0x%04x\n", tdcNofWords, tdcId);
		if (tdcId == 0x5555) break;
		if (tdcId == 0x7000 || tdcId == 0x7001 || tdcId == 0x7002 || tdcId == 0x7003){
			tdcDataIndex++;
			continue;
		}

		//read TDC words to array
		UInt_t dataArray[tdcNofWords];
		for (UInt_t i = 0; i < tdcNofWords; i++) {
			tdcDataIndex++;
			tdcData = rawSubEvent->SubDataValue(tdcDataIndex);
			dataArray[i] = tdcData;
		} // for tdcNofWords
		DecodeTdcData(dataArray, tdcNofWords, trbId, tdcId);
		tdcDataIndex++;
	}
}

void CbmRichTrbUnpack::DecodeTdcData(
		UInt_t* data,
		UInt_t size,
		UInt_t trbId,
		UInt_t tdcId)
{
	Bool_t isPmtTrb = CbmRichTrbParam::Instance()->IsPmtTrb(trbId);
	UInt_t curEpochCounter = 0;
	UInt_t prevChNum[5] = {0, 0, 0, 0, 0};
	UInt_t prevEpochCounter[5] = {0, 0, 0, 0, 0};
	UInt_t prevCoarseTime[5] = {0, 0, 0, 0, 0};
	UInt_t prevFineTime[5] = {0, 0, 0, 0, 0};
	UInt_t prevCounter = 0;
	UInt_t prevNof = 0;
	for (UInt_t i = 0; i < size; i++){
		UInt_t tdcData = data[i];

		UInt_t tdcTimeDataMarker = (tdcData >> 31) & 0x1; //1 bit
		if (tdcTimeDataMarker == 0x1) { //TIME DATA

			UInt_t chNum = (tdcData >> 22) & 0x7f; // 7bits
			UInt_t fineTime = (tdcData >> 12) & 0x3ff; // 10 bits
			UInt_t edge = (tdcData >> 11) & 0x1; // 1bit
			UInt_t coarseTime = (tdcData) & 0x7ff; // 1bits

			// Give the calibrator the read fine time so that it was taken into account
			if ((trbId != 0x7005)) CbmTrbCalibrator::Instance()->AddFineTime(trbId, tdcId, chNum, fineTime);

			if (chNum == 0) {
				// std::cout << "CHANNEL0: " << std::hex << "     TRB " << trbId << "    TDC " << tdcId << std::dec << std::endl;
			} else {
				if (tdcId == 0x7005) { //CTS

				} else if (tdcId == 0x0110) { // reference time TDC for event building
               //if ( (fAnaPulserEvents && chNum == 15) || (!fAnaPulserEvents && chNum == 5) ) {         // shit shit shit
					if  ( (fAnaType == kCbmRichBeamEvent && chNum == 5) ||           // hodoscope (beam trigger)
						  (fAnaType == kCbmRichLaserPulserEvent && chNum == 7) ||    // UV LED
						  (fAnaType == kCbmRichLedPulserEvent && chNum == 15) ){     // Laser

                 // if ( (fAnaPulserEvents && chNum == 5) || (!fAnaPulserEvents && chNum == 6) ) {         // Hodoscopes (beam trigger)
					//if ( (fAnaPulserEvents && chNum == 15) || (!fAnaPulserEvents && chNum == 16) ) {       // Laser
				   //if ( (fAnaPulserEvents && chNum == 7) || (!fAnaPulserEvents && chNum == 8) ) {         // UV LED
						CbmTrbRawHit* rawHitRef = new CbmTrbRawHit(trbId, tdcId, chNum, curEpochCounter, coarseTime, fineTime, 0, 0, 0, 0);
						fRawEventTimeHits.push_back(rawHitRef);
					}
				} else if ( isPmtTrb ) {
					if (chNum == prevChNum[prevCounter]) {
						LOG(DEBUG) << " DOUBLE HIT DETECTED TIMEDATA chNum:" << chNum << ", fineTime:" << fineTime << ", edge:" << edge << ", coarseTime:" << coarseTime
												<< ", fullTime:" << fixed << GetFullTime(trbId, tdcId, chNum, curEpochCounter, coarseTime, fineTime) << FairLogger::endl;
						fNofDoubleHits++;
						continue;
					}
					if (chNum%2 == 1) { // leading edge
						if (chNum == prevChNum[prevCounter]) {
							prevCounter++;
						}
						prevChNum[prevCounter] = chNum;
						prevEpochCounter[prevCounter] = curEpochCounter;
						prevCoarseTime[prevCounter] = coarseTime;
						prevFineTime[prevCounter] = fineTime;
					} else { // trailing edge
						//prevNof = prevCounter;
						prevCounter = 0;
						if (chNum == prevChNum[prevCounter]) {
							prevCounter++;
						}
						if (chNum - prevChNum[prevCounter] == 1) {
							CbmTrbRawHit* rawHit = new CbmTrbRawHit(trbId, tdcId, prevChNum[prevCounter],
																   prevEpochCounter[prevCounter], prevCoarseTime[prevCounter],
																   prevFineTime[prevCounter], chNum, curEpochCounter,
																   coarseTime, fineTime);
							fRawRichHits.push_back(rawHit);

							prevChNum[prevCounter] = chNum;
							prevEpochCounter[prevCounter] = 0;
							prevCoarseTime[prevCounter] = 0;
							prevFineTime[prevCounter] = 0;
						} else {
							LOG(DEBUG) << "Leading edge channel number - trailing edge channel number != 1. tdcId=" << hex << tdcId << dec <<
									", chNum=" << chNum <<  ", prevChNum=" << prevChNum[prevCounter] << FairLogger::endl;
						}
					}
				} //isPmtTrb
			}// if chNum!=0
			LOG(DEBUG) << "TIMEDATA chNum:" << chNum << ", fineTime:" << fineTime << ", edge:" << edge << ", coarseTime:" << coarseTime
					<< ", fullTime:" << fixed << GetFullTime(trbId, tdcId, chNum, curEpochCounter, coarseTime, fineTime) << FairLogger::endl;
			if (fineTime == 0x3ff) LOG(DEBUG) << "-ERROR- Dummy fine time registered: " << fineTime << FairLogger::endl;
		}//if TIME DATA

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
	}// for loop
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

	Int_t nofRefHits = fRawEventTimeHits.size();
	fOutputEventTimeHits.resize(nofRefHits);
	for (Int_t i = 0; i < nofRefHits; i++) {
		fOutputEventTimeHits[i] = CreateOutputHit(fRawEventTimeHits[i]);
	}

	std::sort(fOutputRichHits.begin(), fOutputRichHits.end(), CbmTrbOutputHitLeadingFullTimeComparatorLess());

	/*for (Int_t i = 0; i < fOutputRichHits.size(); i++){
		cout << fixed << fOutputRichHits[i]->GetLFullTime() << endl;
	}
	cout << "reference" << endl;
	for (Int_t i = 0; i < fOutputReferenceHits.size(); i++){
		cout << fixed << fOutputReferenceHits[i]->GetLFullTime() << endl;
	}*/
}

void CbmRichTrbUnpack::BuildEvent(Int_t refHitIndex)
{
	//Int_t nofRichHits = fOutputRichHits.size();
	//Int_t nofRefHits = fOutputReferenceHits.size();

	CbmRichTrbParam* param = CbmRichTrbParam::Instance();
	Int_t indmin, indmax;
	Double_t eventTime = fOutputEventTimeHits[refHitIndex]->GetLFullTime();
	FindMinMaxIndex(eventTime, EVENT_TIME_WINDOW, &indmin, &indmax);
	Int_t size = indmax - indmin + 1;
	UInt_t nofHitsTrb[TRB_TDC3_NUMBOARDS];
	for (UInt_t i = 0; i < TRB_TDC3_NUMBOARDS; i++) {
		nofHitsTrb[i] = 0;
	}
	for (Int_t iH = indmin; iH <= indmax; iH++) {
		CbmTrbOutputHit* h = fOutputRichHits[iH];
		CbmRichHitInfo* data = param->GetRichHitInfo(h->GetTdc(), h->GetLChannel());

		UShort_t trbInd = ( (h->GetTrb() >> 4) & 0x00FF ) - 1;
		nofHitsTrb[trbInd]++;

		AddRichHitToOutputArray(trbInd, data);

		fhDiffHitTimeEventTime->Fill(eventTime - h->GetLFullTime());

		LOG(DEBUG2) << data->GetX() << " " << data->GetY() << FairLogger::endl;
		LOG(DEBUG2) <<fixed << iH << " " << hex << h->GetTdc() << dec << " " << h->GetLChannel() << " " << h->GetLFullTime() << FairLogger::endl;
	}

	for (UInt_t i = 0; i < TRB_TDC3_NUMBOARDS - 1; i++) {
		fhNofRichHitsVsTrbNum->Fill(i+1, nofHitsTrb[i]);
	}
}

void CbmRichTrbUnpack::AddRichHitToOutputArray(UShort_t trbId, CbmRichHitInfo* hitInfo)
{
	UInt_t counter1 = fRichHits->GetEntries();
	new((*fRichHits)[counter1]) CbmRichHit();
	CbmRichHit* hit = static_cast<CbmRichHit*>(fRichHits->At(counter1));
	hit->SetX(hitInfo->GetX());
	hit->SetY(hitInfo->GetY());

	UInt_t counter2 = fRichHitInfos->GetEntries();
	new((*fRichHitInfos)[counter2]) CbmRichHitInfo();
	CbmRichHitInfo* newHitInfo = static_cast<CbmRichHitInfo*>(fRichHitInfos->At(counter2));
	newHitInfo->Copy(hitInfo);

	if (fRichHits->GetEntries() != fRichHitInfos->GetEntries()){
		LOG(ERROR) << "CbmRichTrbUnpack::AddRichHitToOutputArray: fRichHits->Entries() =" <<
				fRichHits->GetEntries() << "NOT EQUAL fRichHitInfos->Entries() = " << fRichHitInfos->GetEntries() << FairLogger:: endl;
	}
}

void CbmRichTrbUnpack::FindMinMaxIndex(
		Double_t refTime,
		Double_t windowT,
		Int_t *indmin,
		Int_t *indmax)
{
	Double_t windowTime = 0.5 * windowT;
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

	for (Int_t i = 0; i < fRawEventTimeHits.size(); i++) {
		delete fRawEventTimeHits[i];
	}
	fRawEventTimeHits.clear();

	for (Int_t i = 0; i < fOutputRichHits.size(); i++) {
		delete fOutputRichHits[i];
	}
	fOutputRichHits.clear();

	for (Int_t i = 0; i < fOutputEventTimeHits.size(); i++) {
		delete fOutputEventTimeHits[i];
	}
	fOutputEventTimeHits.clear();
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
	LOG(INFO) << "Number of raw RICH hits = " << fRawRichHits.size() << FairLogger::endl;
	LOG(INFO) << "Number of output RICH hits = " << fOutputRichHits.size() << FairLogger::endl;
	LOG(INFO) << "Number of raw reference time hits = " << fRawEventTimeHits.size() << FairLogger::endl;
	LOG(INFO) << "Number of output reference time hits = " << fOutputEventTimeHits.size() << FairLogger::endl;
	LOG(INFO) << "Number of double hits = " << fNofDoubleHits / 2. <<
			", " << 100. * (fNofDoubleHits / 2.) / fRawRichHits.size() <<  "%"<< FairLogger::endl;

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
			fhDeltaT[iTrb][iTdc] = new TH1D(hname, htitle, 200, -20, 40);
		}
	}

	for (int i = 0; i < fRawRichHits.size(); i++) {
		FillRawHitHist(fRawRichHits[i]);

	}

	for (int i = 0; i < fRawEventTimeHits.size(); i++) {
		//FillRawHitHist(fRawReferenceHits[i]);
	}

	for (int i = 0; i < fOutputRichHits.size(); i++) {
		FillOutputHitHist(fOutputRichHits[i]);
		CbmTrbOutputHit* rh = fOutputRichHits[i];
	}

	for (int i = 0; i < fOutputEventTimeHits.size(); i++) {
		//FillOutputHitHist(fOutputReferenceHits[i]);
	}

	TFolder* rootHistFolder = gROOT->GetRootFolder()->AddFolder("rich_trb_unpack_debug", "rich_trb_unpack_debug");
	TString cname;
	for (Int_t iTrb = 0; iTrb < TRB_TDC3_NUMBOARDS; iTrb++){
		cname.Form("rich_trb_unpack_debug_trb%d", iTrb+1);
		TCanvas* c = new TCanvas(cname.Data(), cname.Data(), 1250, 1000);
		c->Divide(5, TRB_TDC3_NUMTDC);
		for (Int_t iTdc = 0; iTdc < TRB_TDC3_NUMTDC; iTdc++)
      {
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

			//rootHistFolder->Add(c);

         //rootHistFolder->Add(fhChannelEntries[iTrb][iTdc]);
         //rootHistFolder->Add(fhEpoch[iTrb][iTdc]);
         //rootHistFolder->Add(fhCoarseTime[iTrb][iTdc]);
         //rootHistFolder->Add(fhFineTime[iTrb][iTdc]);
         rootHistFolder->Add(fhDeltaT[iTrb][iTdc]);

		}
	}

	TCanvas* c1 = new TCanvas("rich_trb_unpack_debug_rich_hits_vs_trb_num", "rich_trb_unpack_debug_rich_hits_vs_trb_num", 1200, 800);
	DrawH2(fhNofRichHitsVsTrbNum);

	TCanvas* c2 = new TCanvas("rich_trb_unpack_debug_diff_hittime_eventtime", "rich_trb_unpack_debug_diff_hittime_eventtime", 800, 800);
	DrawH1(fhDiffHitTimeEventTime);

	rootHistFolder->Add(fhNofRichHitsVsTrbNum);
	rootHistFolder->Add(fhDiffHitTimeEventTime);
	rootHistFolder->Write();
}

void CbmRichTrbUnpack::CreateAndDrawEventBuildDisplay()
{
	Double_t minT = fOutputEventTimeHits[0]->GetLFullTime();
	Double_t maxT = fOutputEventTimeHits[ fOutputEventTimeHits.size() - 1 ]->GetLFullTime();
	TH1D* hRefTime = new TH1D("hRefTime", "hRefTime;Time [ns];Entries", 10000, 0., maxT - minT);
	TH1D* hHitTime = new TH1D("hHitTime", "hHitTime;Time [ns];Entries", 10000, 0., maxT - minT);
	TH1D* hDiffRefTime = new TH1D("hDiffRefTime", "hDiffRefTime;dT [ns];Entries", 2000, -10., 200000);
	std::sort(fOutputEventTimeHits.begin(), fOutputEventTimeHits.end(), CbmTrbOutputHitLeadingFullTimeComparatorLess());
	for (int i = 0; i < fOutputEventTimeHits.size(); i++) {
		hRefTime->Fill(fOutputEventTimeHits[i]->GetLFullTime() - minT);

		if (i != 0) hDiffRefTime->Fill( fOutputEventTimeHits[i]->GetLFullTime() - fOutputEventTimeHits[i-1]->GetLFullTime() );
	}
	for (int i = 0; i < fOutputRichHits.size(); i++) {
		hHitTime->Fill(fOutputRichHits[i]->GetLFullTime() - minT);
	}

	TCanvas* c = new TCanvas("rich_trb_unpack_debug_ref_time", "rich_trb_unpack_debug_ref_time", 1600, 600);
	DrawH1(hRefTime);
	TCanvas* c2 = new TCanvas("rich_trb_unpack_debug_rich_hit_time", "rich_trb_unpack_debug_rich_hit_time", 1600, 600);
	DrawH1(hHitTime);
	TCanvas* c3 = new TCanvas("rich_trb_unpack_debug_delta_ref_time", "rich_trb_unpack_debug_delta_ref_time", 800, 800);
	DrawH1(hDiffRefTime);

	/*TH1D* div = (TH1D*)histhits->Clone();
	div->Divide(histRef);
	DrawH1(div);**/

}

ClassImp(CbmRichTrbUnpack)
