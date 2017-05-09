#ifndef CBMTRDLABTOOLSBUCH2016_H
#define CBMTRDLABTOOLSBUCH2016_H

#include "CbmTrdTestBeamTools.h"

class CbmTrdTestBeamToolsBuch2016: public CbmTrdTestBeamTools
{
  public:
	CbmTrdTestBeamToolsBuch2016();
  // CbmTrdLabTools():CbmTrdLabTools("CbmTrdBeamtimeTools"){};
 ~CbmTrdTestBeamToolsBuch2016(){};


  virtual Int_t GetNrRobs(){
    return 1;
  };
  virtual Int_t GetNrSpadics(){
    return 1;
  }
  virtual Int_t GetLayerID(CbmSpadicRawMessage* raw)
  {
    return 0;
  };
  
  CbmTrdTestBeamToolsBuch2016(const CbmTrdTestBeamToolsBuch2016&);
  CbmTrdTestBeamToolsBuch2016 operator=(const CbmTrdTestBeamToolsBuch2016&);
 public:
  
  ClassDef(CbmTrdTestBeamToolsBuch2016,1);
};

#endif
