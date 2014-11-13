#ifndef CBMRICHTRBPARAM_H
#define CBMRICHTRBPARAM_H

#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include "FairLogger.h"

#include "CbmRichTrbMapData.h"

using namespace std;

class CbmRichTrbParam {
private:
	CbmRichTrbParam() {
		ReadMap();
	}

public:

	static CbmRichTrbParam* Instance()
	{
		static CbmRichTrbParam fInstance;
		return &fInstance;
	}

	~CbmRichTrbParam() {

	}

	CbmRichTrbMapData* GetRichTrbMapData(UInt_t tdcId, UInt_t leadingChannel)
	{
		return fMap[tdcId][leadingChannel];
	}

	Bool_t IsPmtTrb(UInt_t trbId) const
	{
		return  (trbId==0x8015 || trbId==0x8025 || trbId==0x8035 || trbId==0x8045 ||
			                   trbId==0x8055 || trbId==0x8065 || trbId==0x8075 || trbId==0x8085 ||
					             trbId==0x8095 || trbId==0x80a5 || trbId==0x80b5 || trbId==0x80c5 ||
					             trbId==0x80d5 || trbId==0x80e5 || trbId==0x80f5 || trbId==0x8105); // || trbId == 0x8115);

	}

	Bool_t IsReferenceTimeTdc(UInt_t tdcId)
	{
		return (tdcId == 0x0110);
	}

private:

	map<UInt_t, map<UInt_t, CbmRichTrbMapData*> > fMap;

	void ReadMap()
	{
		string line;
		ifstream myfile ("pixel-coord_channel-register.ascii");
		if (myfile.is_open()) {
			getline (myfile,line);
			getline (myfile,line);

			while ( !myfile.eof() ){
				UInt_t pmtNum, pixelNum;
				Double_t xmm, ymm;
				UInt_t simpleX, simpleY, tdcId, chLeadingEdge, chTrailingEdge, padiwaNum, trbNum;
				string pmtType;
				myfile >> pmtNum >> pixelNum >> xmm >> ymm >> simpleX >> simpleY;
				myfile >> std::hex >> tdcId;
				myfile >> std::dec >> chLeadingEdge >> chTrailingEdge >> padiwaNum >> trbNum >> pmtType;
				LOG(DEBUG) << pmtNum <<" " <<  pixelNum <<" " << xmm <<" " << ymm <<" " << simpleX <<" " <<
						simpleY <<" " << tdcId<<" " <<chLeadingEdge<<" " <<chTrailingEdge<<" " <<padiwaNum<<" " <<trbNum<<" " <<pmtType << FairLogger::endl;

				fMap[tdcId][chLeadingEdge] = new CbmRichTrbMapData(pmtNum, pixelNum, xmm, ymm, simpleX, simpleY, tdcId, chLeadingEdge, chTrailingEdge, padiwaNum, trbNum, pmtType);
			}
			myfile.close();
		} else {
		   LOG(FATAL) << "[CbmRichTrbParam::ReadMap] Failed to open ASCII map file." << FairLogger::endl;
		}
	}

};

#endif
