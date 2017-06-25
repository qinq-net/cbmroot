#ifndef CBMTRDTESTBEAMTOOLS_H
#define CBMTRDTESTBEAMTOOLS_H

#include "CbmTrdCluster.h"
#include "CbmTrdDigi.h"
#include "TClonesArray.h"
#include "CbmSpadicRawMessage.h"
#include <vector>
#include "TString.h"
#include "TObject.h"
#include "TMath.h"

class CbmTrdTestBeamTools: public TObject
{
  protected:
  TClonesArray* fDigis;

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
  /**
   * GetSpadicResponse: Calculate detector response of the Spadic for given Parameters.
   * The response is modeled by calculating two delta Pulses and overlaying them.
   *
   * @return The return value is a std::vector of 32 Samples for the given Parameters.
   * @param Max Maximum ADC-value of the Signal. This is not Baseline corrected.
   * @param Base Baseline value for the calculation of the response
   * @param FirstTime Timing information for the first hit. The Value is the interpolated index where the signal crosses the half Maximum point.
   *        It is given in units of the sampling time.
   * @param FWHM Timing information for the second hit. This is the interpolated index where the signal crosses the half Maximum point on the downward slope.
   */
  std::vector<Float_t> GetSpadicResponse(Float_t Max,Float_t Base, Float_t FirstHalfMax, Float_t SecondHalfMax){
    return std::vector<Float_t>({0,1});
  }

/**
 * GetDeltaResponse: Get the spadic Response for a Delta pulse occurring at Index 0.
 *              The measured pulses have this occurring at Index ~1, so adjust appropriately.
 *
 * @param Index This is a fractional index for the Sample occurring at this point relative to the original Delta pulse.
 *        Index 0.0 might be offset relative to the sample index in a real Spadic Response.
 */
  Float_t GetDeltaResponse(Float_t Index){
    static std::vector<Float_t> ResponseArray;
    if(ResponseArray.empty()){
        Float_t Sample=GetSamplingTime();
        Float_t Shape=GetShapingTime();
        for (Int_t i=0;i<320;i++){
            ResponseArray.push_back((TMath::Exp(-(i/10.0)*Sample/Shape)*((i/10.0)*Sample)/(Shape*Shape)));
        }
    }
    Int_t intIndex=static_cast<Int_t>(round(Index*10));
    return ResponseArray.at(intIndex);
  }

  Float_t FindIntercept(Float_t FirstSample, Float_t SecondSample, Float_t Max, Float_t Base){
    FirstSample-=Base;
    SecondSample-=Base;
    Max-=Base;
    Max/=2.0;
    Float_t Slope=SecondSample-FirstSample;
    return (Max-FirstSample)/Slope;

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
  Float_t GetIntegratedCharge(const Float_t* Samples,Int_t NrSamples=32);

  CbmTrdTestBeamTools(const CbmTrdTestBeamTools&);
  CbmTrdTestBeamTools operator=(const CbmTrdTestBeamTools&);

  /**
   * Now define all necessary functions for the clusters.
   */
  void
  SetDigisArray (TClonesArray*Digis)
  {
    if (Digis)
      this->fDigis = Digis;
  }
  ;

  virtual Int_t
  GetModuleID (CbmTrdCluster*);

  virtual Int_t
  GetLayerID (CbmTrdCluster*);

  virtual Int_t
  GetSectorID (CbmTrdCluster* clust)
  {
    //returns sectorID of the Cluster. Only defined for compatibility.
    auto Digi = static_cast<CbmTrdDigi*> (fDigis->At (clust->GetDigi (0)));
    return CbmTrdAddress::GetSectorId (Digi->GetAddress ());
  }
  ;
  virtual Int_t
  GetCentralRowID (CbmTrdCluster*);
  virtual Int_t
  GetCentralColumnID (CbmTrdCluster*);
  virtual Int_t
  GetRowWidth (CbmTrdCluster*);
  virtual Int_t
  GetColumnWidth (CbmTrdCluster*);
  Float_t
  GetCharge (CbmTrdCluster*);
  virtual Float_t
  GetRowDisplacement (CbmTrdCluster*);
  virtual Float_t
  GetColumnDisplacement (CbmTrdCluster*);
  enum CbmTrdClusterClassification
    : char
      { kNormal = 0,
    kMissingSTR,
    kMissingFNR,
    kInvalidCharge,
    kFragmented,
    kEmpty
  };
  CbmTrdClusterClassification
  ClassifyCluster (CbmTrdCluster*);
    TString
    GetClassName (CbmTrdClusterClassification Class)
    {
      switch (Class)
        {
        case kNormal:
          return "kNormal";
        case kMissingSTR:
          return "kMissingSTR";
        case kMissingFNR:
          return "kMissingFNR";
        case kInvalidCharge:
          return "kInvalidCharge";
        case kFragmented:
          return "kFragmented";
        case kEmpty:
          return "kFragmented";
        }
  }

protected:

  Float_t
  CalculateSECHS (Float_t, Float_t, Float_t);
 public:
  
  ClassDef(CbmTrdTestBeamTools,1);
};

#endif
