#include "CbmTSUnpackTestMiniRich.h"

// ROOT
#include "TClonesArray.h"

// FairRoot
#include "FairLogger.h"
#include "FairRootManager.h"

// Temporary
#include "Support.hpp"

// Project
#include "CbmTrbRawMessage.h"

CbmTSUnpackTestMiniRich::CbmTSUnpackTestMiniRich() :
	CbmTSUnpack(),
	fCurEpochCounter(0),
	fCurMSid(0),
	fInSubSubEvent(kFALSE),
	fEventsCounter(0)
{
}

CbmTSUnpackTestMiniRich::~CbmTSUnpackTestMiniRich()
{
}

Bool_t CbmTSUnpackTestMiniRich::Init()
{
	LOG(INFO) << "Initializing test mRICH unpacker" << FairLogger::endl;

	FairRootManager* ioman = FairRootManager::Instance();
	if (ioman == NULL) {
		LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
	}

	fTrbRawMessages = new TClonesArray("CbmTrbRawMessage");
	ioman->Register("fTrbRawMessages", "Raw TRB messages", fTrbRawMessages, kTRUE);

	return kTRUE;
}

Bool_t CbmTSUnpackTestMiniRich::DoUnpack(const fles::Timeslice& ts, size_t component)
{
	/*if (fEventsCounter%100 == 0)*/
	{
		LOG(INFO) << "Processing timeslice " << fEventsCounter << FairLogger::endl;
	}
	fEventsCounter++;

	LOG(DEBUG) << "Components:  " << ts.num_components() << FairLogger::endl;
	LOG(DEBUG) << "Microslices: " << ts.num_microslices(component) << FairLogger::endl;

	/*const uint64_t compSize = ts.size_component(component);
	LOG(DEBUG) << "Component " << component << " has size " << compSize << FairLogger::endl;*/

	for (size_t iMS = 0; iMS < ts.num_microslices(component); ++iMS)
	{
		fCurMSid = iMS;
		LOG(DEBUG) << "=======================================================" << FairLogger::endl;
		const fles::MicrosliceView mv = ts.get_microslice(component, iMS);
		const fles::MicrosliceDescriptor& msDesc = mv.desc();
		const uint8_t* msContent = mv.content();
		LOG(DEBUG) << "msDesc.size=" << msDesc.size << FairLogger::endl;
		LOG(DEBUG) << "msDesc.idx=" << msDesc.idx << FairLogger::endl;
		//PrintRaw(msDesc.size, msContent);
		//LOG(DEBUG) << "=======================================================" << FairLogger::endl;
		ProcessMicroslice(msDesc.size, msContent);
		//LOG(DEBUG) << "=======================================================" << FairLogger::endl;
	}
}

void CbmTSUnpackTestMiniRich::Reset()
{
	fTrbRawMessages->Clear();
}

void CbmTSUnpackTestMiniRich::Finish()
{
}

void CbmTSUnpackTestMiniRich::ProcessMicroslice(size_t const size, uint8_t const * const ptr)
{
	if (size == 0) return;

	fGwordCnt = 0; //TODO testing

	Int_t offset; // offset in bytes
	Int_t* dataPtr;
	
	offset = 0; SwapBytes(4, ptr+offset);
	LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "ok" << "\t"
	            << "Reserved 0000 0000"
	            << FairLogger::endl;
	
	offset = 4; SwapBytes(4, ptr+offset); dataPtr = (Int_t*)(ptr+offset);
	Int_t mbsNumber = (Int_t)(dataPtr[0] & 0xffffff);
	LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "ok" << "\t"
	            << "mbsNumber = " << mbsNumber
	            << FairLogger::endl;


	// We suppose that the first word is
	// "HadesTransportUnitQueue - Length"
	offset = 0+8; SwapBytes(4, ptr+offset); dataPtr = (Int_t*)(ptr+offset);
	fTRBeventSize1 = (Int_t)(dataPtr[0]);
	LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "ok" << "\t"
	            << "HadesTransportUnitQueue - Length = " << fTRBeventSize1
	            << FairLogger::endl;

	if (*dataPtr == 0x80030000) {
		LOG(INFO) << "dataPtr == 0x80030000" << FairLogger::endl;
		exit(EXIT_FAILURE);
	}


	// We suppose that the second word is
	// "HadesTransportUnitQueue - Decoder  (Seems to be allways the same)"
	offset = 4+8; SwapBytes(4, ptr+offset); dataPtr = (Int_t*)(ptr+offset);
	Int_t dcdr = (Int_t)(dataPtr[0]);
	if (dcdr == 0x00030062) {
		LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "ok" << "\t"
		            << "HadesTransportUnitQueue - Decoder = " << dcdr
		            << FairLogger::endl;
	} else {
		LOG(WARNING) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "er" << "\t"
		             << "HadesTransportUnitQueue - Decoder = " << dcdr
		             << " is not 0x00030062 (196706)"
		             << FairLogger::endl;
	}

	// We suppose that the third word is
	// TRB event length (in bytes)
	// It should be 8 less than the size specified two words ago
	offset = 8+8; SwapBytes(4, ptr+offset); dataPtr = (Int_t*)(ptr+offset);
	fTRBeventSize2 = (Int_t)(dataPtr[0]);
	if (fTRBeventSize2 == fTRBeventSize1-8) {
		LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "ok" << "\t"
		            << "TRB event - Length = " << fTRBeventSize2
		            << " == " << fTRBeventSize1 << "-8"
		            << FairLogger::endl;
	} else {
		LOG(DEBUG) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "er" << "\t"
		           << "TRB event - Length = " << fTRBeventSize2
		           << " != " << fTRBeventSize1 << "-8=" << fTRBeventSize1-8
		           << FairLogger::endl;
	}

	ProcessTRBevent(fTRBeventSize2, ptr+offset);
}

Int_t CbmTSUnpackTestMiniRich::ProcessTRBevent(size_t const size, uint8_t const * const ptr)
{
	Int_t offset; // offset in bytes
	Int_t* dataPtr;

	// Skip first word (already processed outside)

	// We suppose that the second word consists of
	// 0002 - number of following word till the Event Data Starts (should be the same)
	// 00<TriggerType>1 - value in [7:4] defines TriggerType
	offset = 4; SwapBytes(4, ptr+offset); dataPtr = (Int_t*)(ptr+offset);
	Int_t checkSize = (Int_t)((dataPtr[0] >> 16) & 0xffff);
	fTriggerType = (Int_t)((dataPtr[0] >> 4) & 0xf);
	if (checkSize == 2) {
		LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "ok" << "\t"
		            << "checkSize == 2" << "\t"
		            << "trigger type = " << fTriggerType
		            << FairLogger::endl;
	} else {
		LOG(WARNING) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "er" << "\t"
		             << "checkSize != 2" << "\t"
		             << "trigger type = " << fTriggerType
		             << FairLogger::endl;
	}

	/*for (size_t iWord=2; iWord<size; iWord++) {
		offset = iWord*4;
		LOG(DEBUG4) << "\t" << GetWordHexRepr(ptr+offset) << FairLogger::endl;
	}*/

	// We suppose that the third word consists of
	// 0000 <SubEventId>
	offset = 8; SwapBytes(4, ptr+offset); dataPtr = (Int_t*)(ptr+offset);
	Int_t checkBytes = (Int_t)((dataPtr[0] >> 16) & 0xffff);
	fSubEvId = (Int_t)((dataPtr[0]) & 0xffff);
	if (checkBytes == 0) {
		LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "ok" << "\t"
		            << "checkBytes == 0" << "\t"
		            << "subevent ID = 0x" << GetHexRepresentation(2, ptr+offset)
		            << FairLogger::endl;
	} else {
		LOG(WARNING) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "er" << "\t"
		             << "checkBytes != 0" << "\t"
		             << "subevent ID = 0x" << GetHexRepresentation(2, ptr+offset)
		             << FairLogger::endl;
	}

	// We suppose that the fourth word is the trigger number
	offset = 12; SwapBytes(4, ptr+offset); dataPtr = (Int_t*)(ptr+offset);
	fTriggerNum = (Int_t)(dataPtr[0]);
	LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "ok" << "\t"
				<< "trigger num = " << fTriggerNum
				<< FairLogger::endl;

	offset = 16;

	Int_t iIter = 0;

	while (offset < size) {

		//std::cout << "SE iteration " << iIter++ << "\toffset=" << offset << "\tsize=" << size << std::endl;

		// We suppose that the fifth word is the header of the subevent
		// <Length> <HubId>
		SwapBytes(4, ptr+offset); dataPtr = (Int_t*)(ptr+offset);
		fSubEvSize = (Int_t)((dataPtr[0] >> 16) & 0xffff);
		fHubId = (Int_t)((dataPtr[0]) & 0xffff);
		LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "ok" << "\t"
					<< "hub ID = 0x" << GetHexRepresentation(2, ptr+offset) << "\t"
					<< "subevent size = " << fSubEvSize
					<< FairLogger::endl;

		//FIXME change from 0xc001 to 0xc000 at some point
		if ((fHubId == 0xc001) || (fHubId == 0xc000)) {
			fSubSubEvId = fHubId;
			offset += (4 + ProcessCTSsubevent(fSubEvSize*4, ptr+offset));
			// In principle, should be reset here for safety
			fSubSubEvId = 0;
		} else if (fHubId == 0x5555) {
			//TODO one could implement additional checks here about the
			// words coming after the "event end" but we skip everything by now.
			offset += (4 + ProcessSKIPsubevent(fSubEvSize*4, ptr+offset));
		} else {
			offset += (4 + ProcessTRBsubevent(fSubEvSize*4, ptr+offset));
		}
	}

	//TODO implement checks
	//std::cout << "Done processing TRB event. offset=" << offset << "\tsize=" << size << std::endl;

	return size; //TODO check

}

Int_t CbmTSUnpackTestMiniRich::ProcessTRBsubevent(size_t const size, uint8_t const * const ptr)
{
	//std::cout << "ProcessTRBsubevent size=" << size << " bytes" << std::endl;

	Int_t offset; // offset in bytes
	Int_t* dataPtr;

	// Skip first word (already processed outside)

	Int_t iIter = 0;
	offset = 4;

	while (offset < size) {

		//std::cout << "SSE iteration " << iIter++ << "\toffset=" << offset << "\tsize=" << size << std::endl;

		// We suppose that the second word is the header of the subsubevent
		// <Length> <SubSubEv.Id>
		SwapBytes(4, ptr+offset); dataPtr = (Int_t*)(ptr+offset);
		fSubSubEvSize = (Int_t)((dataPtr[0] >> 16) & 0xffff);
		fSubSubEvId = (Int_t)((dataPtr[0]) & 0xffff);
		LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "ok" << "\t"
					<< "subsubevent ID (FPGA ID) = 0x" << GetHexRepresentation(2, ptr+offset) << "\t"
					<< "subsubevent size = " << fSubSubEvSize
					<< FairLogger::endl;

		// Add 4 bytes which correspond to the header word
		offset += (4 + ProcessTRBsubsubevent(fSubSubEvSize*4, ptr+offset+4)); // MAIN CALL HERE

		// In principle, should be reset here for safety
		fSubSubEvId = 0;

	}


	//TODO implement checks
	//std::cout << "Done processing TRB subevent. offset=" << offset << "\tsize=" << size << std::endl;

	return size; //TODO check
}

Int_t CbmTSUnpackTestMiniRich::ProcessCTSsubevent(size_t const size, uint8_t const * const ptr)
{
	//std::cout << "ProcessCTSsubevent size=" << size << " bytes" << std::endl;

	Int_t offset; // offset in bytes
	Int_t* dataPtr;

	// Skip first word (already processed outside)

	// We suppose that the second word is the header of the subsubevent
	offset = 4; SwapBytes(4, ptr+offset); dataPtr = (Int_t*)(ptr+offset);
	LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << "ok" << "\t"
	            << "CTS header"
				<< FairLogger::endl;

	Short_t trigState[16];
	for (Int_t i=0; i<16; i++) {
		trigState[i] = ((*dataPtr >> i) & 0x1); // 16 x 1 bit
	}
	Short_t nInp = ((*dataPtr >> 16) & 0xf); // 4 bits
	Short_t nTrigCh = ((*dataPtr >> 20) & 0x1f); // 5 bits
	Short_t inclLastIdle = ((*dataPtr >> 25) & 0x1); // 1 bit
	Short_t inclTrigInfo = ((*dataPtr >> 26) & 0x1); // 1 bit
	Short_t inclTS = ((*dataPtr >> 27) & 0x1); // 1 bit
	Short_t ETM = ((*dataPtr >> 28) & 0x3); // 2 bits
	
	// in words (not bytes)
	Short_t CTSinfo_size = nInp*2 + nTrigCh*2 + inclLastIdle*2 + inclTrigInfo*3 + inclTS;
	switch (ETM) {
		case 0:
			break;
		case 1:
			CTSinfo_size += 1;
			break;
		case 2:
			CTSinfo_size += 4;
			break;
		case 3:
			LOG(DEBUG) << "ETM == 3" << FairLogger::endl;
			//TODO implement
			break;
	}

	LOG(DEBUG) << "CTS information size (extracted from the CTS header): " << CTSinfo_size << FairLogger::endl;

	offset = 8;

	while (offset-8 < CTSinfo_size*4) {

		SwapBytes(4, ptr+offset); dataPtr = (Int_t*)(ptr+offset);
		ULong_t MSidx = 102400UL * ((ULong_t)(*dataPtr) - 1);
		LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset) << "\t" << MSidx << "\t" << "ok" << "\t"
		            << "CTS information"
					<< FairLogger::endl;

		offset += 4;
	}

	// size - full size including CTS header word, CTS informations words (CTSinfo_size) and TCD data
	// Thus TDC data size = full size - 1 word (header) - CTSinfo_size words (CTS informations)
	offset += (ProcessTRBsubsubevent((size-(1+CTSinfo_size)*4), ptr+offset)); // MAIN CALL HERE

	//TODO implement checks
	//std::cout << "Done processing CTS subevent. offset=" << offset << "\tsize=" << size << std::endl;

	return size; //TODO check
}

Int_t CbmTSUnpackTestMiniRich::ProcessSKIPsubevent(size_t const size, uint8_t const * const ptr)
{
	//std::cout << "ProcessSKIPsubevent size=" << size << " bytes" << std::endl;

	Int_t offset; // offset in bytes
	Int_t* dataPtr;

	// Skip first word (already processed outside)

	Int_t iIter = 0;
	offset = 4;

	while (offset < size+4) {

		SwapBytes(4, ptr+offset); dataPtr = (Int_t*)(ptr+offset);
		LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr+offset)
					<< FairLogger::endl;

		offset += 4;
	}

	//TODO implement checks
	//std::cout << "Done processing SKIP subevent. offset=" << offset << "\tsize=" << size << std::endl;

	return size; //TODO check
}

Int_t CbmTSUnpackTestMiniRich::ProcessTRBsubsubevent(size_t const size, uint8_t const * const ptr)
{
	//std::cout << "ProcessTRBsubsubevent size=" << size  << " bytes" << std::endl;

	fCurEpochCounter = 0; //TODO check
	fInSubSubEvent = kFALSE; //TODO check

	for (size_t iWord=0; iWord<size/4; iWord++) {
		SwapBytes(4, ptr+iWord*4);
		ProcessTDCword(ptr+iWord*4); // MAIN CALL HERE
	}

	//TODO Implement checks that the first word was the header and the last word was the trailer

	return size; //TODO check
}

void CbmTSUnpackTestMiniRich::ProcessTDCword(uint8_t const * const ptr)
{
	Int_t* tdcDataPtr = (Int_t*)ptr;
	Int_t tdcData = tdcDataPtr[0];
	Int_t tdcTimeDataMarker = (tdcData >> 31) & 0x1; // 1 bit

	if (tdcTimeDataMarker == 0x1) {
		
		Int_t channel = (tdcData >> 22) & 0x7f; // 7 bits
		Int_t fine = (tdcData >> 12) & 0x3ff; // 10 bits
		Int_t edge = (tdcData >> 11) & 0x1; // 1 bit
		Int_t coarse = (tdcData) & 0x7ff; // 11 bits
		Int_t epoch = fCurEpochCounter;

		LOG(DEBUG4) << "[" << fGwordCnt << "]\t" << GetWordHexRepr(ptr) << "\t" << "ok" << "\t" << "TIMESTAMP" << "\t"
		          << "ch=" << channel << "\t"
		          << "edge=" << edge << "\t"
		          << "epoch=" << epoch << "\t"
		          << "coarse=" << coarse << "\t"
		          << "fine=" << fine << "\t"
		          << "full=" << (Double_t)(coarse)*5. - (Double_t)(fine)*0.005
		          << FairLogger::endl;

		fGwordCnt++;

		WriteEdge(fSubSubEvId, channel, edge, epoch, coarse, fine);

	} else {

		UInt_t tdcMarker = (tdcData >> 29) & 0x7; // 3 bits

		if (tdcMarker == 0x0) { // TDC trailer
			if (fInSubSubEvent) {
				LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr) << "\t" << "ok" << "\t" << "TDC TRAILER" << FairLogger::endl;
				fInSubSubEvent = kFALSE;
			} else {
				LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr) << "\t" << "er" << "\t" << "UNKNOWN (TDC TRAILER not after header)" << FairLogger::endl;
				exit(EXIT_FAILURE);
			}
		} else if (tdcMarker == 0x1) { // TDC header
		//	UInt_t randomCode = (tdcData >> 16) & 0xff; // 8 bits
		//	UInt_t errorBits = (tdcData) & 0xffff; //16 bits
			if (!fInSubSubEvent) {
				LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr) << "\t" << "ok" << "\t" << "TDC HEADER" << FairLogger::endl;
				fInSubSubEvent = kTRUE;
			} else {
				LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr) << "\t" << "er" << "\t" << "UNKNOWN (TDC HEADER not after trailer)" << FairLogger::endl;
			}
		} else if (tdcMarker == 0x2) { // DEBUG
		//	UInt_t debugMode = (tdcData >> 24) & 0x1f; // 5 bits
		//	UInt_t debugBits = (tdcData) & 0xffffff; // 24 bits
			LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr) << "\t" << "ok" << "\t" << "DEBUG" << FairLogger::endl;
		} else if (tdcMarker == 0x3) { // EPOCH counter
			fCurEpochCounter = (tdcData) & 0xfffffff; // 28 bits
			LOG(DEBUG4) << "[" << fGwordCnt << "]\t" << GetWordHexRepr(ptr) << "\t" << "ok" << "\t" << "EPOCH\t" << fCurEpochCounter << FairLogger::endl;
			fGwordCnt++;
		} else {
			if (tdcTimeDataMarker != 0x1) {
				LOG(DEBUG4) << "[" << fGwordCnt++ << "]\t" << GetWordHexRepr(ptr) << "\t" << "er" << "\t" << "UNKNOWN" << FairLogger::endl;
			}
		}
	}
}

void CbmTSUnpackTestMiniRich::WriteEdge(CbmTrbRawMessage* p_obj)
{
	Int_t nEdges = fTrbRawMessages->GetEntriesFast();
	new ((*fTrbRawMessages)[nEdges]) CbmTrbRawMessage(*p_obj);
}

void CbmTSUnpackTestMiniRich::WriteEdge(Int_t febID, Int_t ch, Int_t edge, Int_t epoch, Int_t coarse, Int_t fine)
{
	Int_t nEdges = fTrbRawMessages->GetEntriesFast();
	new ((*fTrbRawMessages)[nEdges]) CbmTrbRawMessage(0x30, febID, ch, epoch, coarse, fine, edge, 0., fCurMSid);
}

ClassImp(CbmTSUnpackTestMiniRich)
