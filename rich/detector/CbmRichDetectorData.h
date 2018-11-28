/*detector
 * CbmRichData.h
 *
 *  Created on: Dec 17, 2015
 *      Author: slebedev
 */

#ifndef RICH_DETECTOR_CBMRICHDETECTORDATA_H_
#define RICH_DETECTOR_CBMRICHDETECTORDATA_H_

#include "TObject.h"
#include "FairLogger.h"
#include <vector>
#include <algorithm>

using namespace std;

class CbmRichPixelData {
public:
    Int_t fAddress;
	Double_t fX;
	Double_t fY;
	Double_t fZ;
	Int_t fPmtId;
};

class CbmRichPmtData {
public:
    Bool_t ContainsPixel(Int_t address) {
        return std::find(fPixelAddresses.begin(), fPixelAddresses.end(), address) != fPixelAddresses.end();
    }

    string ToString() {
        return "id:" + to_string(fId) + " nofPixels:" +  to_string(fPixelAddresses.size()) + " x:" + to_string(fX) +
               " y:" + to_string(fY) + " z:" + to_string(fZ) + " W:" + to_string(fWidth) + " H:" + to_string(fHeight);
    }
    Int_t fId;
    vector<Int_t> fPixelAddresses;
    Double_t fX;
    Double_t fY;
    Double_t fZ;
    Double_t fWidth;
    Double_t fHeight;

};

#endif /* RICH_DETECTOR_CBMRICHDETECTORDATA_H_ */
