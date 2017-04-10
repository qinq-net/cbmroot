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
  virtual UInt_t GetAddress(CbmSpadicRawMessage* raw);
  virtual Int_t GetModuleID(CbmSpadicRawMessage* raw);
  virtual Int_t GetLayerID(CbmSpadicRawMessage* raw);
  virtual Int_t GetRowID(CbmSpadicRawMessage* raw);
  virtual Int_t GetRobID(CbmSpadicRawMessage* raw);
  virtual Int_t GetSpadicID(CbmSpadicRawMessage* raw);
  virtual Int_t GetColumnID(CbmSpadicRawMessage* raw);
  //  virtual Int_t GetCombiID(CbmSpadicRawMessage* raw);
  virtual Int_t GetChannelOnPadPlane(CbmSpadicRawMessage* raw);
  virtual Float_t GetSamplingTime(){
    /*
     * Get inverse sampling frequency in nano seconds.
     */
    return 1E3/15;
  };
  virtual Int_t GetShapingTime(){
    /*
     * Get shaping time in nano seconds.
     */
    return 90;
  };

  virtual Int_t GetNrRobs(){
    return NrOfActiveSyscores;
  };
  virtual Int_t GetNrSpadics(){
    return NrOfActiveSpadics;
  }
  virtual std::vector<Int_t> GetChannelMap(CbmSpadicRawMessage* raw);
  TString GetSpadicName(Int_t,Int_t,TString,Bool_t);
  Int_t GetBaseline(CbmSpadicRawMessage* raw);
  virtual Int_t GetMaximumAdc(CbmSpadicRawMessage* raw);
  Float_t GetIntegratedCharge(CbmSpadicRawMessage*,Double_t Base=0.0);
  
  CbmTrdTestBeamTools(const CbmTrdTestBeamTools&);
  CbmTrdTestBeamTools operator=(const CbmTrdTestBeamTools&);
 public:
  
  ClassDef(CbmTrdTestBeamTools,1);
};

#endif
