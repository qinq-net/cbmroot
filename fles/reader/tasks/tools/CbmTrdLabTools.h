#ifndef CBMTRDLABTOOLS_H
#define CBMTRDLABTOOLS_H

#include "CbmTrdTestBeamTools.h"

class CbmTrdLabTools: public CbmTrdTestBeamTools
{
  public:
 CbmTrdLabTools();
  // CbmTrdLabTools():CbmTrdLabTools("CbmTrdBeamtimeTools"){};
 ~CbmTrdLabTools(){};


  virtual Int_t GetNrRobs(){
    return 1;
  };
  virtual Int_t GetNrSpadics(){
    return 1;
  }
  
  CbmTrdLabTools(const CbmTrdLabTools&);
  CbmTrdLabTools operator=(const CbmTrdLabTools&);
 public:
  
  ClassDef(CbmTrdLabTools,1);
};

#endif
