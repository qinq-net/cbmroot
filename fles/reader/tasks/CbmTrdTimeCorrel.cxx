#include "CbmTrdTimeCorrel.h"
#include "CbmSpadicRawMessage.h"
#include "CbmHistManager.h"
#include "CbmBeamDefaults.h"

#include "FairLogger.h"

#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TNtuple.h"
#include "TString.h"
#include "TMath.h"
#include <cmath>
#include <map>
#include <vector>

CbmTrdTimeCorrel::CbmTrdTimeCorrel()
  : FairTask("CbmTrdTimeCorrel"),
    fRawSpadic(NULL),
    fHM(new CbmHistManager()),
    fNrTimeSlices(0)  
{
 LOG(DEBUG) << "Default constructor of CbmTrdTimeCorrel" << FairLogger::endl;
}

CbmTrdTimeCorrel::~CbmTrdTimeCorrel()
{
 LOG(DEBUG) << "Destructor of CbmTrdTimeCorrel" << FairLogger::endl;
}
void CbmTrdTimeCorrel::SetParContainers()
{
 LOG(DEBUG) << "SetParContainers of CbmTrdTimeCorrel" << FairLogger::endl;
}
InitStatus CbmTrdTimeCorrel::Init()
{
 LOG(DEBUG) << "Initilization of CbmTrdTimeCorrel" << FairLogger::endl;
 FairRootManager* ioman = FairRootManager::Instance();
 fRawSpadic = static_cast<TClonesArray*>(ioman->GetObject("SpadicRawMessage"));
 if ( !fRawSpadic ) {
    LOG(FATAL) << "No InputDataLevelName array!\n CbmTrdTimeCorrel will be inactive" << FairLogger::endl;
    return kERROR;
  }
  CreateHistograms();
  return kSUCCESS;
}
// ---- ReInit  -------------------------------------------------------
InitStatus CbmTrdTimeCorrel::ReInit()
{
  LOG(DEBUG) << "Initilization of CbmTrdTimeCorrel" << FairLogger::endl;
  return kSUCCESS;
}
void CbmTrdTimeCorrel::Exec(Option_t* option)
{
  // Analysis based on single SPADIC data streams can be done here!!!

  if(fNrTimeSlices==0){
    if(TsaContainerCounter->GetN()!=0){
      LOG(INFO ) << "Expected empty TsaContainerCounter before first TimeSlice, but found " << " entries."  << TsaContainerCounter->GetN() << FairLogger::endl;
    }
  }
  
  std::map<TString, std::map<ULong_t, std::vector<CbmSpadicRawMessage*> > > timeBuffer;
  LOG(INFO) << "CbmTrdTimeCorrel: Number of found TimeSlices: " << fNrTimeSlices << FairLogger::endl;
  Int_t nSpadicMessages = fRawSpadic->GetEntriesFast();//SPADIC messages per TimeSlice
  LOG(INFO) << "nSpadicMessages: " << nSpadicMessages << FairLogger::endl;
  for (Int_t iSpadicMessages=0; iSpadicMessages < nSpadicMessages; ++iSpadicMessages) {
    CbmSpadicRawMessage* raw = static_cast<CbmSpadicRawMessage*>(fRawSpadic->At(iSpadicMessages));
    Int_t eqID = raw->GetEquipmentID();
    Int_t sourceA = raw->GetSourceAddress();
    Int_t chID = raw->GetChannelID();
    //    Int_t nrSamples=raw->GetNrSamples();
    Int_t triggerType=raw->GetTriggerType();
    Int_t stopType=raw->GetStopType();
    Int_t infoType=raw->GetInfoType();
    if (infoType > 6) infoType = 7;
    Int_t groupId=raw->GetGroupId();
    ULong_t time = raw->GetFullTime();

    // get syscore, spadic and channel
    TString syscore = GetSysCore(eqID);
    Int_t sysID     = GetSysCoreID(eqID);
    TString spadic  = GetSpadic(sourceA);
    Int_t spaID     = GetSpadicID(sourceA);
    if(spaID%2) chID+=16;
    TString channelId=Form("_Ch%02d", chID);
    // add raw message to map sorted by timestamps, syscore and spadic
    timeBuffer[TString(syscore+spadic)][time].push_back(raw);
  }
 // complicated loop over sorted map of timestamps
  for(std::map<TString, std::map<ULong_t, std::vector<CbmSpadicRawMessage*> > >::iterator it = timeBuffer.begin() ; it != timeBuffer.end(); it++){
    // complicated loop over sorted map of raw messages
    for (std::map<ULong_t, std::vector<CbmSpadicRawMessage*> > ::iterator it2 = it->second.begin() ; it2 != it->second.end(); it2++) {
      LOG(INFO) <<  "ClusterSize:" << Int_t(it2->second.size()) << FairLogger::endl;
      //for (std::vector<CbmSpadicRawMessage*>::iterator it3 = it2->second.begin(); it3 != it2->second.end(); it3++) {
      for(Int_t i = 0; i < Int_t(it2->second.size()); i++){
	delete it2->second[i];//it3->second;
      }
    }
  }
  fNrTimeSlices++;
}
void CbmTrdTimeCorrel::Finish()
{
  if(fNrTimeSlices==0){
    if(TsaContainerCounter->GetN()==0){
      LOG(INFO ) << "Expected entries TsaContainerCounter after first TimeSlice, but found none." << FairLogger::endl;
    }
  }
  //Buffer (map) or multi SPADIC data streams based analyis have to be done here!!
  LOG(DEBUG) << "Finish of CbmTrdTimeCorrel" << FairLogger::endl;
  LOG(INFO) << "Write histo list to " << FairRootManager::Instance()->GetOutFile()->GetName() << FairLogger::endl;
  FairRootManager::Instance()->GetOutFile()->cd();
  fHM->WriteToFile();
}
void CbmTrdTimeCorrel::FinishEvent()
{
  LOG(DEBUG) << "FinishEvent of CbmTrdTimeCorrel" << FairLogger::endl;
}
void CbmTrdTimeCorrel::CreateHistograms()
{    
  TString syscoreName[3] = { "SysCore0", "SysCore1", "SysCore2" };
  TString spadicName[3]  = { "Spadic0",  "Spadic1",  "Spadic2" };
  TString channelName[32] = { "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", 
			      "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", 
			      "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", 
			      "30", "31"};
  TString triggerTypes[4] = { "Global trigger",
			      "Self triggered",
			      "Neighbor triggered",
			      "Self and neighbor triggered"};
  TString stopTypes[6] = {"Normal end of message", 
			  "Channel buffer full", 
			  "Ordering FIFO full", 
			  "Multi hit", 
			  "Multi hit and channel buffer full", 
			  "Multi hit and ordering FIFO full"};
  TString infoTypes[8] = {"Channel disabled during message building", 
			  "Next grant timeout", 
			  "Next request timeout", 
			  "New grant but channel empty", 
			  "Corruption in message builder", 
			  "Empty word", 
			  "Epoch out of sync", 
			  "infoType out of array"}; //not official type, just to monitor overflows

  fHM->Add("Trigger", new TH1F("Trigger", "Trigger", 9,0,9));
  for(Int_t syscore = 0; syscore < 3; ++syscore) {
    for(Int_t spadic = 0; spadic < 3; ++spadic) {
      fHM->H1("Trigger")->GetXaxis()->SetBinLabel(3*syscore+spadic+1,TString(syscoreName[syscore]+"_"+spadicName[spadic]));
    }
  }
  fHM->Add("TsaContainerCounter", new TGraph(500));
  fHM->G1("TsaContainerCounter")->GetXaxis()->SetTitle("TSA number")
}
TString CbmTrdTimeCorrel::GetSysCore(Int_t eqID)
{
  TString syscore="";
  //  Int_t SysId=-1;
  switch (eqID) {
  case kFlesMuenster:  // Muenster
    syscore="SysCore0_";
    //SysId = 0;
    break;
  case kFlesFrankfurt: // Frankfurt
    syscore="SysCore1_";
    //SysId = 1;
    break;
  case kFlesBucarest: // Bucarest
    syscore="SysCore2_";
    //SysId = 2;
    break;
  default:
    LOG(ERROR) << "EquipmentID " << eqID << "not known." << FairLogger::endl;
    break;
  }
  return syscore;
}

Int_t CbmTrdTimeCorrel::GetSysCoreID(Int_t eqID)
{
  //TString syscore="";
  Int_t SysId=-1;
  switch (eqID) {
  case kFlesMuenster:  // Muenster
    //syscore="SysCore0_";
    SysId = 0;
    break;
  case kFlesFrankfurt: // Frankfurt
    //syscore="SysCore1_";
    SysId = 1;
    break;
  case kFlesBucarest: // Bucarest
    //syscore="SysCore2_";
    SysId = 2;
    break;
  default:
    LOG(ERROR) << "EquipmentID " << eqID << "not known." << FairLogger::endl;
    break;
  }
  return SysId;
}

TString CbmTrdTimeCorrel::GetSpadic(Int_t sourceA)
{
  TString spadic="";
  //Int_t SpaId = -1;
  switch (sourceA) {
  case (SpadicBaseAddress+0):  // first spadic
    spadic="Spadic0";
    //SpaId = 0;
    break;
  case (SpadicBaseAddress+1):  // first spadic
    spadic="Spadic0";
    //SpaId = 1;
    break;
  case (SpadicBaseAddress+2):  // second spadic
    spadic="Spadic1";
    //SpaId = 2;
    break;
  case (SpadicBaseAddress+3):  // second spadic
    spadic="Spadic1";
    //SpaId = 3;
    break;
  case (SpadicBaseAddress+4):  // third spadic
    spadic="Spadic2";
    //SpaId = 4;
    break;
  case (SpadicBaseAddress+5):  // third spadic
    spadic="Spadic2";
    //SpaId = 5;
    break;
  default:
    LOG(ERROR) << "Source Address " << sourceA << "not known." << FairLogger::endl;
    break;
  }
  return spadic;
}

Int_t CbmTrdTimeCorrel::GetSpadicID(Int_t sourceA)
{
  //TString spadic="";
  Int_t SpaId = -1;
  switch (sourceA) {
  case (SpadicBaseAddress+0):  // first spadic
    //spadic="Spadic0";
    SpaId = 0;
    break;
  case (SpadicBaseAddress+1):  // first spadic
    //spadic="Spadic0";
    SpaId = 1;
    break;
  case (SpadicBaseAddress+2):  // second spadic
    //spadic="Spadic1";
    SpaId = 2;
    break;
  case (SpadicBaseAddress+3):  // second spadic
    //spadic="Spadic1";
    SpaId = 3;
    break;
  case (SpadicBaseAddress+4):  // third spadic
    //spadic="Spadic2";
    SpaId = 4;
    break;
  case (SpadicBaseAddress+5):  // third spadic
    //spadic="Spadic2";
    SpaId = 5;
    break;
  default:
    LOG(ERROR) << "Source Address " << sourceA << "not known." << FairLogger::endl;
    break;
  }
  return SpaId;
}
