#ifndef CBMTRDTESTBEAMTOOLS_H
#define CBMTRDTESTBEAMTOOLS_H

#include "CbmSpadicRawMessage.h"
#include "CbmBeamDefaults.h"
#include <vector>
#include "TString.h"
#include "TObject.h"

class CbmTrdTestBeamTools: public TObject
{
  public:
 CbmTrdTestBeamTools();
  // CbmTrdTestBeamTools():CbmTrdTestBeamTools("CbmTrdBeamtimeTools"){};
 ~CbmTrdTestBeamTools(){};

  static CbmTrdTestBeamTools* Instance(CbmTrdTestBeamTools*);
  virtual Int_t GetAddress(CbmSpadicRawMessage* raw);
  virtual Int_t GetModuleID(CbmSpadicRawMessage* raw);
  virtual Int_t GetLayerID(CbmSpadicRawMessage* raw);
  virtual Int_t GetRowID(CbmSpadicRawMessage* raw);
  virtual Int_t GetRobID(CbmSpadicRawMessage* raw);
  virtual Int_t GetSpadicID(CbmSpadicRawMessage* raw);
  virtual Int_t GetColumnID(CbmSpadicRawMessage* raw);
  //  virtual Int_t GetCombiID(CbmSpadicRawMessage* raw);
  virtual Int_t GetChannelOnPadPlane(CbmSpadicRawMessage* raw);
  virtual std::vector<Int_t> GetChannelMap(CbmSpadicRawMessage* raw);
  TString GetSpadicName(Int_t,Int_t,TString,Bool_t);
  Int_t GetBaseline(CbmSpadicRawMessage* raw){
    return raw->GetSamples()[0];
  };
  virtual Int_t GetMaximumAdc(CbmSpadicRawMessage* raw){
    Int_t MaxADC=-255;
    for (int i=0;i<raw->GetNrSamples();i++){
      if (raw->GetSamples()[i]>MaxADC)
	MaxADC=raw->GetSamples()[i];
    }
    return MaxADC;
  };
  //  virtual Float_t GetIntegratedCharge(CbmSpadicRawMessage* raw);
  
  CbmTrdTestBeamTools(const CbmTrdTestBeamTools&);
  CbmTrdTestBeamTools operator=(const CbmTrdTestBeamTools&);
 public:
  
  ClassDef(CbmTrdTestBeamTools,1);
};

#endif
