#ifndef CBMRICHTRBPARAM_H
#define CBMRICHTRBPARAM_H
#include <iostream>
#include <fstream>
#include <string>
#include <map>

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

private:

	map<UInt_t, map<UInt_t, CbmRichTrbMapData*> > fMap;

	void ReadMap()
	{
		string line;
		ifstream myfile ("/Users/slebedev/Development/cbm/trunk/cbmroot/macro/fles/pixel-coord_channel-register.ascii");
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
				cout << pmtNum <<" " <<  pixelNum <<" " << xmm <<" " << ymm <<" " << simpleX <<" " <<
						simpleY <<" " << tdcId<<" " <<chLeadingEdge<<" " <<chTrailingEdge<<" " <<padiwaNum<<" " <<trbNum<<" " <<pmtType << endl;

				fMap[tdcId][chLeadingEdge] = new CbmRichTrbMapData(pmtNum, pixelNum, xmm, ymm, simpleX, simpleY, tdcId, chLeadingEdge, chTrailingEdge, padiwaNum, trbNum, pmtType);
			}
			myfile.close();
		}
	}
};

#endif
