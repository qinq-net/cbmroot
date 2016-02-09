// -------------------------------------------------------------------------
// -----                    CbmTrackParam header file                 -----
// -----                  Created 05/02/16  by T. Ablyazimov          -----
// -------------------------------------------------------------------------

/**  CbmTrackParam.h
 *@author T.Ablyazimov <t.ablyazimov@gsi.de>
 **
 ** Data class for Global CBM track parameters. Data level RECO.
 ** It is derived from the FairTrackParam class and extends its data interfaces.
 **
 **/

#ifndef CBMLTRACKPARAM_H_
#define CBMLTRACKPARAM_H_ 1

#include "FairTrackParam.h"

class CbmTrackParam : public FairTrackParam
{
public:
    CbmTrackParam() : fPx(0), fPy(0), fPz(0), fDpx(0), fDpy(0), fDpz(0) {}
    void Set(const FairTrackParam& ftp);
    Double_t GetPx() const { return fPx; }
    Double_t GetPy() const { return fPy; }
    Double_t GetPz() const { return fPz; }
    Double_t GetDpx() const { return fDpx; }
    Double_t GetDpy() const { return fDpy; }
    Double_t GetDpz() const { return fDpz; }
  
private:
    Double_t fPx;
    Double_t fPy;
    Double_t fPz;
    Double_t fDpx;
    Double_t fDpy;
    Double_t fDpz;
  ClassDef(CbmTrackParam, 1);  
};


#endif
