#include "CbmRichTrbUnpack.h"
#include "CbmTrbIterator.h"
#include "CbmTrbRawHit.h"

#include <iostream>
#include <fstream>
#include <vector>

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
		cout << "the entire file content is in memory" << endl;
		cout << "file size=" << size << endl;

	} else {
		cout << "Unable to open file" << endl;
	}

	ReadEvents((void*)memblock, size);

	delete[] memblock;
}

void CbmRichTrbUnpack::ReadEvents(void* data, int size)
{
	CbmTrbIterator* trbIter = new CbmTrbIterator(data, size);
	Int_t nofRawEvents = 0;
	Int_t maxNofRawEvents = 10;
	while (true) {
		CbmRawEvent* rawEvent = trbIter->NextEvent();
		if (rawEvent == NULL) break;
		rawEvent->Print();
		while (true){
			CbmRawSubEvent* rawSubEvent = trbIter->NextSubEvent();
			if (rawSubEvent == NULL) break;
			rawSubEvent->Print();
			ProcessTdc(rawSubEvent);
		}
		if (nofRawEvents >= maxNofRawEvents) break;
		nofRawEvents++;
	}

	cout << "nofRawTrbHits = " << fRawTrbHits.size() << endl;
}

void CbmRichTrbUnpack::ProcessTdc(CbmRawSubEvent* rawSubEvent)
{
	uint32_t tdcDataIndex = 0;
	uint32_t tdcData = 0x0;
	uint32_t trbId = rawSubEvent->SubId();
	bool isPmtTrb = (trbId==0x8015 || trbId==0x8025 || trbId==0x8035 || trbId==0x8045 || trbId==0x8055 || trbId==0x8065 || trbId==0x8075 || trbId==0x8085 ||
			trbId==0x8095 || trbId==0x80a5 || trbId==0x80b5 || trbId==0x80c5 || trbId==0x80d5 || trbId==0x80e5 || trbId==0x8015 || trbId==0x80f5 || trbId == 0x8105);


	while (true) {
		tdcData = rawSubEvent->SubDataValue(tdcDataIndex);
		uint32_t tdcNofWords = (tdcData >> 16) & 0xffff;
		uint32_t tdcId = tdcData & 0xffff;
		printf("\nTDC DATA tdcNofWords = %i, tdcId = 0x%04x\n", tdcNofWords, tdcId);
		if (tdcId == 0x5555) break;

		uint32_t curEpochCounter = 0x0;
		for (uint32_t i = 0; i < tdcNofWords; i++) {
			tdcDataIndex++;
			tdcData = rawSubEvent->SubDataValue(tdcDataIndex);

			uint32_t tdcTimeDataMarker = (tdcData >> 31) & 0x1; //1 bit
			if (tdcTimeDataMarker == 0x1) { //TIME DATA
				uint32_t chNum = (tdcData >> 22) & 0x7f; // 7bits
				uint32_t fineTime = (tdcData >> 12) & 0x3ff; // 10 bits
				uint32_t edge = (tdcData >> 11) & 0x1; // 1bit
				uint32_t coarseTime = (tdcData) & 0x7ff; // 1bits

				// create rawHits only for PMT, skip channel0
				if (chNum == 0) {

				} else {
					if ( isPmtTrb ) {
						CbmTrbRawHit* rawHit = new  CbmTrbRawHit(tdcId, chNum, edge, curEpochCounter, coarseTime, fineTime);
						fRawTrbHits.push_back(rawHit);
					}
				}
				cout << "TIMEDATA chNum:" << chNum << ", fineTime:" << fineTime << ", edge:" << edge << ", coarseTime:" << coarseTime << endl;
				if (fineTime == 0x3ff) printf("-ERROR- Dummy fine time registered.");
			}

			uint32_t tdcMarker = (tdcData >> 29) & 0x7; //3 bits
			if (tdcMarker == 0x1) {// TDC header
				uint32_t randomCode = (tdcData >> 16) & 0xff; // 8bits
				uint32_t errorBits = (tdcData) & 0xffff; //16 bits
				printf("TDC HEADER randomCode:0x%02x, errorBits:0x%04x\n", randomCode, errorBits);
			} else if (tdcMarker == 0x2) {// DEBUG
				uint32_t debugMode = (tdcData >> 24) & 0x1f; //5
				uint32_t debugBits = (tdcData) & 0xffffff;//24 bits
				printf("DEBUG debugMode:%i, debugBits:0x%06x\n", debugMode, debugBits);
			} else if (tdcMarker == 0x3){ // EPOCH counter
				curEpochCounter = (tdcData) & 0xfffffff; //28 bits
				printf("EPOCHE COUNTER epochCounetr:0x%07x\n", curEpochCounter);
			}
		} // for tdcNofWords
		tdcDataIndex++;
	}
}

ClassImp(CbmRichTrbUnpack)
