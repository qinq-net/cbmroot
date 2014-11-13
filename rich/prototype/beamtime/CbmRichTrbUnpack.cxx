#include "CbmRichTrbUnpack.h"

#include <fstream>
#include <vector>
#include <algorithm>

#include "CbmTrbIterator.h"
#include "CbmTrbRawHit.h"
#include "CbmTrbOutputHit.h"
#include "CbmRichTrbParam.h"
#include "CbmTrbCalibrator.h"

#include "TH1D.h"
#include "TCanvas.h"

#include "FairLogger.h"

CbmRichTrbUnpack::CbmRichTrbUnpack()
{
   ;
}

CbmRichTrbUnpack::~CbmRichTrbUnpack()
{
	;
}

void CbmRichTrbUnpack::Run(const string& hldFileName)
{
	streampos size;
	char * memblock;

	ifstream file (hldFileName.c_str(), ios::in|ios::binary|ios::ate);
	if (file.is_open()) {
		size = file.tellg();
		memblock = new char [size];
		file.seekg (0, ios::beg);
		file.read (memblock, size);
		file.close();
		LOG(INFO) << "The entire file content is in memory (" << size/1024 << " kB)" << FairLogger::endl;
	} else {
		LOG(FATAL) << "Unable to open file" << FairLogger::endl;
	}

	ReadEvents((void*)memblock, (int)size);

	//this->DrawQa();

	delete[] memblock;
}

void CbmRichTrbUnpack::ReadEvents(void* data, int size)
{
	CbmTrbIterator* trbIter = new CbmTrbIterator(data, size);
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

		// Loop throught subevents
		while (true){
			CbmRawSubEvent* rawSubEvent = trbIter->NextSubEvent();
			if (rawSubEvent == NULL) break;
			//rawSubEvent->Print();
			ProcessTdc(rawSubEvent);
		}

		if (nofEventsInBuffer >= NOF_RAW_EVENTS_IN_BUFFER) {
			//LOG(INFO) << "Event building, nof events in buffer:" << nofEventsInBuffer << FairLogger::endl;
			CreateOutputHits();
			BuildEvent();
			ClearAllBuffers();
			nofEventsInBuffer = 0;
		}

      // Go out if exceed the limit of total number of raw hits
		if (nofRawEvents >= maxNofRawEvents){
			break;
		}

		nofEventsInBuffer++;
		nofRawEvents++;
		
		CbmTrbCalibrator::Instance()->NextRawEvent();    //TODO Quite tricky, maybe refactor this
	}
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

				// create rawHits only for PMT, skip channel0
				if (chNum == 0) {

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


                        // Give the calibrator the read fine time so that it was taken into account
                        CbmTrbCalibrator::Instance()->AddFineTime(trbId, tdcId, chNum, prevFineTime, fineTime);

								
								prevChNum = 0;
								prevEpochCounter = 0;
								prevCoarseTime = 0;
								prevFineTime = 0;
							} else {
								//LOG(INFO) << "-ERROR- leading edge channel number - trailing edge channel number != 1" << FairLogger::endl;
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
	return new CbmTrbOutputHit(h->GetTdc(), h->GetLChannel(), lFullTime, h->GetTChannel(), tFullTime);
}

Double_t CbmRichTrbUnpack::GetFullTime(UShort_t TRB, UShort_t TDC, UShort_t CH, UInt_t epoch, UShort_t coarseTime, UShort_t fineTime)
{
	Double_t coarseUnit = 5.;
	Double_t epochUnit = coarseUnit * 0x800;
	
   uint32_t trb_index = (TRB >> 4) & 0x00FF;
   uint32_t tdc_index = (TDC & 0x000F);
	
	Double_t time = epoch * epochUnit + coarseTime * coarseUnit + 
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
}

void CbmRichTrbUnpack::BuildEvent()
{
	Int_t nofRichHits = fOutputRichHits.size();
	Int_t nofRefHits = fOutputReferenceHits.size();
	// sort array of RICH hits
	std::sort(fOutputRichHits.begin(), fOutputRichHits.end(), CbmTrbOutputHitLeadingFullTimeComparatorLess());

	CbmRichTrbParam* param = CbmRichTrbParam::Instance();
	for (Int_t iRef = 0; iRef < nofRefHits; iRef++) {
		Int_t indmin, indmax;
		FindMinMaxIndex(fOutputReferenceHits[iRef]->GetLFullTime(), &indmin, &indmax);
		Int_t size = indmax - indmin + 1;
	   LOG(DEBUG) << "NEW EVENT, size " << size << FairLogger::endl;
		for (Int_t iH = indmin; iH <= indmax; iH++) {
			CbmTrbOutputHit* h = fOutputRichHits[iH];
			CbmRichTrbMapData* data = param->GetRichTrbMapData(h->GetTdc(), h->GetLChannel());

			LOG(DEBUG2) << data->GetX() << " " << data->GetY() << FairLogger::endl;
        	LOG(DEBUG2) <<fixed << iH << " " << hex << h->GetTdc() << dec << " " << h->GetLChannel() << " " << h->GetLFullTime() << FairLogger::endl;
		}
	}
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

void CbmRichTrbUnpack::DrawQa()
{
	TH1D* hLEpoch = new TH1D("hLEpoch", "hLEpoch", 100, 0, 0);
	TH1D* hTEpoch = new TH1D("hTEpoch", "hTEpoch", 100, 0, 0);
	TH1D* hLCTime = new TH1D("hLCTime", "hLCTime", 100, 0, 0);
	TH1D* hLFTime = new TH1D("hLFTime", "hLFTime", 100, 0, 0);
	TH1D* hTCTime = new TH1D("hTCTime", "hTCTime", 100, 0, 0);
	TH1D* hTFTime = new TH1D("hTFTime", "hTFTime", 100, 0, 0);
	TH1D* hFullTime = new TH1D("hFullTime", "hFullTime", 10000, 0, 0);
	TH1D* hFullTimeRef = new TH1D("hFullTimeRef", "hFullTimeRef", 10000, 0, 0);

	for (int i = 0; i < fRawRichHits.size(); i++) {
		CbmTrbRawHit* rh = fRawRichHits[i];
		hLEpoch->Fill( rh->GetLEpoch() );
		hTEpoch->Fill( rh->GetTEpoch() );
		hLCTime->Fill( rh->GetLCTime() );
		hLFTime->Fill( rh->GetLFTime() );
		hTCTime->Fill( rh->GetTCTime() );
		hTFTime->Fill( rh->GetTFTime() );
		Double_t fullTime = this->GetFullTime(rh->GetTrb(), rh->GetTdc(), rh->GetLChannel(), rh->GetLEpoch(), rh->GetLCTime(), rh->GetLFTime());
		hFullTime->Fill( fullTime );
	}

	for (int i = 0; i < fRawReferenceHits.size(); i++) {
		CbmTrbRawHit* rh = fRawReferenceHits[i];
		Double_t fullTime = this->GetFullTime(rh->GetTrb(), rh->GetTdc(), rh->GetLChannel(), rh->GetLEpoch(), rh->GetLCTime(), rh->GetLFTime());
		hFullTimeRef->Fill( fullTime );
	}

	TCanvas* c2 = new TCanvas();
	hLCTime->Draw();
	TCanvas* c3 = new TCanvas();
	hLFTime->Draw();
	TCanvas* c4 = new TCanvas();
	hTCTime->Draw();
	TCanvas* c5 = new TCanvas();
	hTFTime->Draw();
	TCanvas* c6 = new TCanvas();
	hFullTime->Draw();
	hFullTimeRef->SetLineColor(kRed);
	hFullTimeRef->Draw("same");
	TCanvas* c7 = new TCanvas();
	hLEpoch->Draw();
	TCanvas* c8 = new TCanvas();
	hTEpoch->Draw();

}

ClassImp(CbmRichTrbUnpack)
