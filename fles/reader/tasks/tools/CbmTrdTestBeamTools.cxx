/*First Version of TestBeam tools, developed for the SPS2016 beamtime and the legacy DAQ-chain.
 *Intended for the F/MS Prototypes.
 *For usage with Bucharest, modifications of the functions GetChannelMap and GetRowID are neccessary.
*/

#include "CbmTrdTestBeamTools.h"
#include "FairLogger.h"
#include "CbmTrdAddress.h"
#include <algorithm>
#include "TMath.h"

ClassImp(CbmTrdTestBeamTools)

CbmTrdTestBeamTools::CbmTrdTestBeamTools () : TObject()
{
  LOG(INFO) << TString("Default Constructor of ")+TString(this->GetName())
		<< FairLogger::endl;
  Instance(this);
}

CbmTrdTestBeamTools* CbmTrdTestBeamTools::Instance(CbmTrdTestBeamTools* ptr=nullptr){
  /*Get the current instance of TestBeam tools in use, or set it.
   *Only permits one instance of TestBeam tools.
   */
  static CbmTrdTestBeamTools* CurrentInstance=nullptr;
  if (!ptr&&!CurrentInstance)
    return new CbmTrdTestBeamTools();
  if (!ptr)
    return CurrentInstance;
  if (!CurrentInstance)
    CurrentInstance=ptr;
  if (ptr!=CurrentInstance)
    LOG(FATAL)<<"Preexisting Instance of "<< CurrentInstance->GetName()<<FairLogger::endl;
  return CurrentInstance;
};

UInt_t CbmTrdTestBeamTools::GetAddress(CbmSpadicRawMessage* raw){
  /*Get a unique address for the pad of the Message.
   *Last 3 decimal digits comprise of the ColumnID. Range:0-255;
   *Next 2 decimal digits display the Row inside the module. Range: 0-10;
   *Next 2 decimal digits are the moduleID in the respective Layer. Range:0-49;
   *Digit %1E5 gives the Layer. Range:0-3;
   */
  Int_t ColID=GetColumnID(raw);
  Int_t SectID=0;//GetSectorID(raw);
  Int_t RowID=GetRowID(raw);
  Int_t ModID=GetModuleID(raw);
  Int_t LayerID=GetLayerID(raw);
  return CbmTrdAddress::GetAddress(LayerID,ModID,SectID,RowID,ColID);
}

Int_t CbmTrdTestBeamTools::GetModuleID(CbmSpadicRawMessage* raw)
{
  /*
   * Get the numeric module ID of origin for the given message.
   * The ModuleID is unique within a layer.
   * SPS2016 Case: identical to RobID and LayerID
   */
  return GetRobID(raw);
}

// ----              -------------------------------------------------------
Int_t CbmTrdTestBeamTools::GetSpadicID(CbmSpadicRawMessage* raw)
{
  /*
   * Get the numerical SpadicID of origin for the given Message.
   * The SpadicID is unique on the Rob.
   */
  //TString spadic="";
  Int_t sourceA = raw->GetSourceAddress();
  Int_t SpaId = sourceA-SpadicBaseAddress;
  if (SpaId<0||SpaId>5){
    LOG(ERROR) << "Source Address " << sourceA << " not known." << FairLogger::endl;
    SpaId = -1;
  }
  return SpaId;
}

Int_t CbmTrdTestBeamTools::GetRobID(CbmSpadicRawMessage* raw)
{
  /*
   * Get the numerical RobId of origin for the given Message.
   * SPS2016 Case: Numerical id of the syscore on which the spadic was connected.
   */
  Int_t eqID=raw->GetEquipmentID();
  Int_t SyscoreID=eqID-BaseEquipmentID;
  if((SyscoreID<0||SyscoreID>NrOfActiveSyscores)){
    LOG(ERROR) << "EqID " << eqID << " not known." << FairLogger::endl;
    SyscoreID=-1;
  }
  return SyscoreID;
}

Int_t CbmTrdTestBeamTools::GetLayerID(CbmSpadicRawMessage* raw)
{
  /*
   * Get the layer from whic the given message originated. Defined in downstream direction.
   * Layer 0 is closest to the target.
   */
  return GetModuleID(raw);
};

Int_t CbmTrdTestBeamTools::GetRowID(CbmSpadicRawMessage* raw){
  /*Get row of the active channel.
   *Upper row is row 1.
   */
  return 1-GetChannelOnPadPlane(raw)/16;
};

Int_t CbmTrdTestBeamTools::GetColumnID(CbmSpadicRawMessage* raw){
  /*Get column of the active channel.
   *With view towards the target, column 0 is on the left side.
   *Lower numbered colums are towards the left side of the module.
   */
  Int_t ColID=(GetSpadicID(raw)/2)*16+(GetChannelOnPadPlane(raw))%16;
  return ColID;
};

TString CbmTrdTestBeamTools::GetSpadicName(Int_t RobID,Int_t SpadicID,TString RobName="SysCore",Bool_t FullSpadic=true)
{
	/*	Get a String of the Form "Syscore_0_Spadic_0" describing the specific SPADIC corresponding to the input parameters.
	 *  The Parameter InputType allows either the Equipment ID/Source Address or the final Syscore/Spadic ID to be used.
	 *  	kRawData (default) is the parameter that allows the raw EqID/Source Address to be used, kProcessedData takes Syscore/SpadicID.
	 *  The Parameter OutputType allows adressing either the corresponding FullSpadic, via kFullSpadic (default), or the original HalfSpadic,
	 *  	via kHalfSpadic.
	 * */
  TString spadicName="";
  spadicName=RobName+"_"+std::to_string(RobID)+"_";
  if(FullSpadic){
	  spadicName += "Spadic_";
  }else{
	  spadicName += "Half_Spadic_";

  }
  spadicName+=std::to_string(SpadicID);

  return spadicName;
}

//  virtual Int_t GetCombiID(CbmSpadicRawMessage* raw);
Int_t CbmTrdTestBeamTools::GetChannelOnPadPlane(CbmSpadicRawMessage* raw){
  /*
   * The Spadic channels have an indirection layer to the pad on the padplane.
   * This takes care of the mapping and returns the pad number.
   */
  std::vector<Int_t>Map=GetChannelMap(raw);
  Int_t chID=(GetSpadicID(raw)%2)*16+raw->GetChannelID();
  return Map.at(chID);
};

std::vector<Int_t> CbmTrdTestBeamTools::GetChannelMap(CbmSpadicRawMessage* raw){
  /*
   * Get the channel mapping that is valid for the spadic where raw originated.
   */
  std::vector<Int_t> map = {{31,15,30,14,29,13,28,12,27,11,26,10,25, 9,24, 8,
			     23, 7,22, 6,21, 5,20, 4,19, 3,18, 2,17, 1,16, 0}};
  return map;
};

Int_t CbmTrdTestBeamTools::GetBaseline(CbmSpadicRawMessage* raw)
{
  /*
   * Estimate the signal baseline. Returns the lowest ADC value in the first two samples.
   */
  Int_t Base =*std::min_element(raw->GetSamples(),raw->GetSamples()+1);
  //Int_t Basecand=*std::min_element(raw->GetSamples()+raw->GetNrSamples()-3,raw->GetSamples()+raw->GetNrSamples());
  return Base;//(Base<Basecand)?Base:Basecand;
};
Int_t CbmTrdTestBeamTools::GetMaximumAdc(CbmSpadicRawMessage* raw,Double_t Base)
{
  /*
   * Calculate message charge using the Maximum ADC method.
   * This is zero suppressed data, if a baseline is given.
   */
  Int_t MaxADC=-255;
  for (int i=0;i<raw->GetNrSamples();i++){
    if (raw->GetSamples()[i]>MaxADC)
      MaxADC=raw->GetSamples()[i];
  }
  return MaxADC;
};
Float_t CbmTrdTestBeamTools::GetIntegratedCharge(CbmSpadicRawMessage* raw,Double_t Base)
{
  /*
   * Get the integrated charge of raw, with Base subtracted.
   * Includes correction for unequal message lengths.
   */
  Double_t Integral=0;
  Int_t Baseline=GetBaseline(raw);
  if(Base!=0.0)
    Baseline = Base;
  Int_t NrSamples=raw->GetNrSamples();
  if(NrSamples<4)
    return 0;
  for (int i=0;i<NrSamples;i++){
    Integral+=raw->GetSamples()[i]-Baseline;
  }
  Double_t CorrectionFactor;
  Double_t Shape=GetShapingTime(),Sample=GetSamplingTime();
  for (Int_t i=0;i<raw->GetNrSamples()-1;i++)
  {
      //Calculate integral of the impulse response of the spadic.
      CorrectionFactor+=(TMath::Exp(-(NrSamples+0.25)*Sample/Shape)*((NrSamples+0.25)*Sample)/(Shape*Shape));
  }
  if (CorrectionFactor<=0.0)
    return 0;
  return static_cast<Float_t>(Integral/CorrectionFactor);
};

Float_t CbmTrdTestBeamTools::GetIntegratedCharge(Float_t* Samples,Int_t NrSamples)
{
  /*
   * Get the integrated charge of raw, with Base subtracted.
   * Includes correction for unequal message lengths.
   */
  Double_t Integral=0;
  if(NrSamples<3)
    return 0;
  for (int i=1;i<NrSamples;i++){
    Integral+=Samples[i];
  }
  Double_t CorrectionFactor;
  Double_t Shape=GetShapingTime(),Sample=GetSamplingTime();
  for (Int_t i=0;i<NrSamples-1;i++)
  {
      //Calculate integral of the impulse response of the spadic.
      CorrectionFactor+=(TMath::Exp(-(NrSamples+0.25)*Sample/Shape)*((NrSamples+0.25)*Sample)/(Shape*Shape));
  }
  if (CorrectionFactor<=0.0)
    return 0;
  return static_cast<Float_t>(Integral/CorrectionFactor);
};
