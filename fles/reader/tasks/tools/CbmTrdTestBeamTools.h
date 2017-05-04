#ifndef CBMTRDTESTBEAMTOOLS_H
#define CBMTRDTESTBEAMTOOLS_H

#include "CbmSpadicRawMessage.h"
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
  virtual Int_t GetRobID(CbmSpadicRawMessage* raw);    //blubb
  virtual Int_t GetSpadicID(CbmSpadicRawMessage* raw); //blubb
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

  virtual Int_t GetNrLayers(){
    return GetNrRobs();
  };
  virtual Int_t GetNrRows(Int_t Layer=0){
    //Get the Number of active Rows in the corresponding Layer
    return 2;
  };
  virtual Int_t GetNrColumns(Int_t Layer=0){
    //Get the Number of active Columns in the corresponding Layer
    return 16*GetNrSpadics();
  }
  virtual Int_t GetNrRobs(){
    //Get the Number of active ROBS in the Setup;
    return 4;
  };
  virtual Int_t GetNrSpadics(Int_t RobID=0){
    //Get the Number of active Spadics on a ROB
    return 3;
  }
  virtual Int_t GetSpadicBaseAddress(){
	  /** Base address which will be added to each half spadic number
	      the first halfspadic which is connected to the Syscore
	      get SpadicBaseAddress+0, the second SpadicBaseAddress +1 ... */
    return 0;
  }
  virtual Int_t GetBaseEquipmentID(){
    //Get the BaseEquipmentID
    return 0xE000;
  }


  virtual std::vector<Int_t> GetChannelMap(CbmSpadicRawMessage* raw);
  TString GetSpadicName(Int_t,Int_t,TString,Bool_t);
  Int_t GetBaseline(CbmSpadicRawMessage* raw);
  virtual Int_t GetMaximumAdc(CbmSpadicRawMessage*,Double_t Base=0.0);
  Float_t GetIntegratedCharge(CbmSpadicRawMessage*,Double_t Base=0.0);
  Float_t GetIntegratedCharge(Float_t* Samples,Int_t NrSamples=32);

  CbmTrdTestBeamTools(const CbmTrdTestBeamTools&);
  CbmTrdTestBeamTools operator=(const CbmTrdTestBeamTools&);
 public:
  
  ClassDef(CbmTrdTestBeamTools,1);
};

#endif
