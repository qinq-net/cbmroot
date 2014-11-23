// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                              CbmTSUnpackTrb                       -----
// -----                    Created 07.11.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------
#include "CbmTSUnpackTrb.h"

#include "CbmTrbIterator.h"
#include "CbmRawEvent.h"
#include "CbmRawSubEvent.h"
#include "CbmRichTrbParam.h"
#include "CbmTrbCalibrator.h"
#include "CbmTrbRawHit.h"

#include "TimesliceReader.hpp"

#include "FairLogger.h"
#include "FairRootManager.h"

#include "TClonesArray.h"

#include <iostream>

CbmTSUnpackTrb::CbmTSUnpackTrb()
  : CbmTSUnpack(),
    fTrbRaw(new TClonesArray("CbmTrbRawHit", 10)),
    fTrbBridge(new TrbBridge()),
    fTrbEventList(),
    fData(),
    fDataSize(0),
    fSynchRefTime(0.),
    fSynchOffsetTimeMap()
{
}

CbmTSUnpackTrb::~CbmTSUnpackTrb()
{
}

Bool_t CbmTSUnpackTrb::Init()
{
  LOG(INFO) << "Initializing" << FairLogger::endl; 

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }
  ioman->Register("TrbRawMessage", "trb raw data", fTrbRaw, kTRUE);

  return kTRUE;
}

Bool_t CbmTSUnpackTrb::DoUnpack(const fles::Timeslice& ts, size_t component)
{
  LOG(DEBUG) << "Unpacking Trb Data" << FairLogger::endl; 

  Int_t skipMS = 1;
  fTrbEventList = fTrbBridge->extractEventsFromTimeslice(ts, skipMS);

  LOG(INFO) << "Found " << fTrbEventList.size() << " TRB events in time slice"
	    << FairLogger::endl;

  while ( !fTrbEventList.empty() ) {  
    fData = fTrbEventList.front();
    fDataSize = fData.size();
    UnpackTrbEvents();
    fTrbEventList.pop_front();
  }

  return kTRUE;
}


void CbmTSUnpackTrb::Reset()
{
  fTrbEventList.clear();
  fTrbRaw->Clear();
}

void CbmTSUnpackTrb::Finish()
{
}

void CbmTSUnpackTrb::UnpackTrbEvents()
{
  CbmTrbIterator* trbIter = new CbmTrbIterator(static_cast<void*>(fData.data()), fDataSize);

  // Loop over TRB events 
  while (true) {
    
    // Try to extract next event from the Iterator. If no events left - go out of the loop
    CbmRawEvent* rawEvent = trbIter->NextEvent();
    if (rawEvent == NULL) break;
    
    fSynchRefTime = -1.;

    Int_t subEvtCounter = 0;
    // Loop over TRB subevents
    while (true){
      CbmRawSubEvent* rawSubEvent = trbIter->NextSubEvent();
      if (rawSubEvent == NULL) break;
      //      ProcessTdc(rawSubEvent);
    }
    
  }
  
}

void CbmTSUnpackTrb::ProcessTdc(CbmRawSubEvent* rawSubEvent)
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

void CbmTSUnpackTrb::DecodeTdcData(UInt_t* data, UInt_t size, UInt_t trbId, UInt_t tdcId)
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
      UInt_t fineTime = (tdcData >> 12) & 0x3ff; // 10 bits //0x3ff
      UInt_t edge = (tdcData >> 11) & 0x1; // 1bit
      UInt_t coarseTime = (tdcData) & 0x7ff; // 1bits
      
      // Give the calibrator the read fine time so that it was taken into account
      if ((trbId != 0x7005)) CbmTrbCalibrator::Instance()->AddFineTime(trbId, tdcId, chNum, fineTime);
      
      if (chNum == 0 ) {
	Double_t time = GetFullTime(trbId, tdcId, chNum, curEpochCounter, coarseTime, fineTime);
	//cout << "CHANNEL0: " << fixed << fSynchRefTime << " " << time << "  "<<fSynchRefTime - time << " " <<hex << "     TRB " << trbId << "    TDC " << tdcId << dec << endl;
	if (fSynchRefTime == -1. ) {//&& (isPmtTrb || tdcId == 0x0110)
	  fSynchRefTime = time;
	}
	if (fSynchRefTime != -1){
	  fSynchOffsetTimeMap[tdcId] = fSynchRefTime - time;
	} else{
	  fSynchOffsetTimeMap[tdcId] = 0.;
	}
      } else {
	if (tdcId == 0x7005) { //CTS
	  
	} else if (tdcId == 0x0110) { // reference time TDC for event building
	  //cout << fSynchOffsetTimeMap[0x0110] << endl;
	  if  (chNum == 5) { // ||           // hodoscope (beam trigger)
	      //		(fAnaType == kCbmRichLaserPulserEvent && chNum == 7) ||    // UV LED
	      //		(fAnaType == kCbmRichLedPulserEvent && chNum == 15) ){     // Laser


	new( (*fTrbRaw)[fTrbRaw->GetEntriesFast()] )      	    
	    CbmTrbRawHit(trbId, tdcId, chNum, curEpochCounter, coarseTime, fineTime, 0, 0, 0, 0);
	    //	    fRawEventTimeHits.push_back(rawHitRef);
	  }
	} else if ( isPmtTrb ) {
	  if (chNum == prevChNum[prevCounter]) {
	    LOG(DEBUG) << " DOUBLE HIT DETECTED TIMEDATA chNum:" << chNum << ", fineTime:" << fineTime << ", edge:" << edge << ", coarseTime:" << coarseTime
		       << ", fullTime:" << fixed << GetFullTime(trbId, tdcId, chNum, curEpochCounter, coarseTime, fineTime) << FairLogger::endl;
	    //fNofDoubleHits++;
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
	      new( (*fTrbRaw)[fTrbRaw->GetEntriesFast()] )      	    
		CbmTrbRawHit(trbId, tdcId, prevChNum[prevCounter],
			     prevEpochCounter[prevCounter], prevCoarseTime[prevCounter],
			     prevFineTime[prevCounter], chNum, curEpochCounter,
			     coarseTime, fineTime);

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

Double_t CbmTSUnpackTrb::GetFullTime(UShort_t TRB, UShort_t TDC, UShort_t CH, UInt_t epoch, UShort_t coarseTime, UShort_t fineTime)
{
  Double_t coarseUnit = 5.;
  Double_t epochUnit = coarseUnit * 0x800;
  
  uint32_t trb_index = (TRB >> 4) & 0x00FF - 1;
  uint32_t tdc_index = (TDC & 0x000F);
  
  Double_t time = epoch * epochUnit + coarseTime * coarseUnit -
    CbmTrbCalibrator::Instance()->GetFineTimeCalibrated(trb_index, tdc_index, CH, fineTime);
  if (CH != 0){
    if (fSynchOffsetTimeMap[TDC] > 150) {
      LOG(ERROR) << "CbmRichTrbUnpack::GetFullTime Synch Offset > 150 ns for TDC" << TDC << FairLogger::endl;
    } else {
      time = time + fSynchOffsetTimeMap[TDC];
    }
  }
  return time;
}

/*
void CbmTSUnpackTrb::Register()
{
}
*/


ClassImp(CbmTSUnpackTrb)
