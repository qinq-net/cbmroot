/*First Version of TestBeam tools, developed for the SPS2016 beamtime and the legacy DAQ-chain.
 *Intended for the F/MS Prototypes.
 *For usage with Bucharest, modifications of the functions GetChannelMap and GetRowID are neccessary.
*/

#include "CbmTrdTestBeamTools.h"
#include "FairLogger.h"

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

Int_t CbmTrdTestBeamTools::GetAddress(CbmSpadicRawMessage* raw){
  /*Get a unique address for the pad of the Message.
   *Last 3 decimal digits comprise of the ColumnID. Range:0-255;
   *Next 2 decimal digits display the Row inside the module. Range: 0-10;
   *Next 2 decimal digits are the moduleID in the respective Layer. Range:0-49;
   *Digit %1E5 gives the Layer. Range:0-3;
   */
  Int_t Address=GetColumnID(raw);
  Address+=GetRowID(raw)*1000;
  Address+=GetModuleID(raw)*10*1000;
  Address+=GetLayerID(raw)*10*10*1000;
  return Address;
}

Int_t CbmTrdTestBeamTools::GetModuleID(CbmSpadicRawMessage* raw){
  return GetRobID(raw);
}

// ----              -------------------------------------------------------
Int_t CbmTrdTestBeamTools::GetSpadicID(CbmSpadicRawMessage* raw)
{
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
  return GetModuleID(raw);
};

Int_t CbmTrdTestBeamTools::GetRowID(CbmSpadicRawMessage* raw){
  /*Get row of the active channel.
   *Upper row is row 0.
   */
  return GetChannelOnPadPlane(raw)%2;
};

Int_t CbmTrdTestBeamTools::GetColumnID(CbmSpadicRawMessage* raw){
  /*Get column of the active channel.
   *With view towards the target, column 0 is on the left side.
   *Lower numbered colums are towards the left side of the module.
   */
  Int_t ColID=(GetSpadicID(raw)/2)*32+GetChannelOnPadPlane(raw)/2;
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
  std::vector<Int_t>Map=GetChannelMap(raw);
  Int_t chID=(GetSpadicID(raw)%2)*16+raw->GetChannelID();
  return Map.at(chID);
};

std::vector<Int_t> CbmTrdTestBeamTools::GetChannelMap(CbmSpadicRawMessage* raw){
  std::vector<Int_t> map = {{31,15,30,14,29,13,28,12,27,11,26,10,25, 9,24, 8,
			     23, 7,22, 6,21, 5,20, 4,19, 3,18, 2,17, 1,16, 0}};
  return map;
};


