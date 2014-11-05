#ifndef CBMRICHTDCTOPMTMAPPING_H
#define CBMRICHTDCTOPMTMAPPING_H
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class CbmTdcToPmtMapping {
public:
	CbmTdcToPmtMapping() {
		readMap();
	}

	~CbmTdcToPmtMapping() {

	}

private:

	void readMap() {
		string line;
		ifstream myfile ("/Users/slebedev/Development/cbm/trunk/cbmroot/macro/fles/pixel-coord_channel-register.ascii");
		if (myfile.is_open()) {
			getline (myfile,line);
			getline (myfile,line);

			while ( !myfile.eof() ){
				unsigned int pmtNum, pixelNum;
				double xmm, ymm;
				unsigned int simpleX, simpleY, tdcId, chLeadingEdge, chTrailingEdge, padiwaNum, trbNum;
				string pmtType;
				myfile >> pmtNum >> pixelNum >> xmm >> ymm >> simpleX >> simpleY;
				myfile >> std::hex >> tdcId;
				myfile >> std::dec >> chLeadingEdge >> chTrailingEdge >> padiwaNum >> trbNum >> pmtType;
				cout << pmtNum <<" " <<  pixelNum <<" " << xmm <<" " << ymm <<" " << simpleX <<" " <<
						simpleY <<" " << tdcId<<" " <<chLeadingEdge<<" " <<chTrailingEdge<<" " <<padiwaNum<<" " <<trbNum<<" " <<pmtType << endl;
			}
			myfile.close();
		}
	}
};

#endif
