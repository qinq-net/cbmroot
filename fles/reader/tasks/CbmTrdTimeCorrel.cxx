#include "CbmTrdTimeCorrel.h"
#include "CbmSpadicRawMessage.h"
#include "CbmHistManager.h"
#include "CbmBeamDefaults.h"

#include "FairLogger.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TNtuple.h"
#include "TString.h"
#include "TMath.h"
#include <cmath>
#include <map>
#include <vector>
#include <iostream>

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
    if(fHM->G1("TsCounter")->GetN()!=0){
      LOG(INFO ) << "Expected empty TsCounter before first TimeSlice, but found " << fHM->G1("TsCounter")->GetN() << " entries." << FairLogger::endl;
    }
  }
  
  std::map<TString, std::map<ULong_t, std::vector<CbmSpadicRawMessage*> > > timeBuffer;
  LOG(INFO) << "CbmTrdTimeCorrel: Number of current TimeSlice: " << fNrTimeSlices << FairLogger::endl;
  Int_t nSpadicMessages = fRawSpadic->GetEntriesFast();//SPADIC messages per TimeSlice
  Int_t nSpadicMessages0(0),nSpadicMessages1(0); //SPADIC messages per TimeSlice for single SPADICS
  Bool_t isHit = false;
  Bool_t isInfo = false;
  Bool_t isEpoch = false;
  LOG(INFO) << "nSpadicMessages: " << nSpadicMessages << FairLogger::endl;
  for (Int_t iSpadicMessage=0; iSpadicMessage < nSpadicMessages; ++iSpadicMessage) {
    CbmSpadicRawMessage* raw = static_cast<CbmSpadicRawMessage*>(fRawSpadic->At(iSpadicMessage));
    // if: set only variables which exist for current message type / initialize before to "unique" value
    isHit = raw->GetHit();
    isInfo = raw->GetInfo();
    isEpoch = raw->GetEpoch();

    if(Int_t(isHit+isInfo+isEpoch)>1) LOG(ERROR) << "SpadicMessage " << iSpadicMessage << " is classified from CbmSpadicRawMessage to be more than one message type: HIT " << Int_t(isHit) << " / INFO " << (Int_t)isInfo << " / EPOCH " << (Int_t)isEpoch << FairLogger::endl;
    if(Int_t(isHit+isInfo+isEpoch)<1) LOG(ERROR) << "SpadicMessage " << iSpadicMessage << " is classified from CbmSpadicRawMessage to be none of the defined types HIT 0 / INFO 0 / EPOCH 0" << FairLogger::endl;
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
    Int_t timeStamp = raw->GetTime();
    Int_t epoch = raw->GetEpochMarker();
    Int_t superEpoch = raw->GetSuperEpoch();
    
    // get syscore, spadic and channel
    TString syscore = GetSysCore(eqID);
    Int_t sysID     = GetSysCoreID(eqID);
    TString spadic  = GetSpadic(sourceA);
    Int_t spaID     = GetSpadicID(sourceA);
    if(spaID%2) chID+=16;
    TString channelId=Form("_Ch%02d", chID);
    TString stopName = GetStopName(stopType);
    // add raw message to map sorted by timestamps, syscore and spadic
    timeBuffer[TString(syscore+spadic)][time].push_back(raw);

    // print single spadic message coordinates .. hey, use this for a fancy fast-running output
    //    if(stopType == 0) LOG(INFO) << "SpadicMessage: " << iSpadicMessage << " sourceA: " << sourceA << " chID: " << chID << " groupID: " << groupId << " spaID: " << spaID << " stopType: " << stopType << " infoType: " << infoType << " triggerType: " << triggerType << FairLogger::endl;

    if(spadic=="Spadic0") nSpadicMessages0++;
    if(spadic=="Spadic1") nSpadicMessages1++;

    //Fill trigger-type histogram
    fHM->H1("Trigger")->Fill(TString(syscore+spadic),1);
    fHM->H1("MessageCount")->Fill(TString(spadic+"_"+stopName),1);
    if(stopType==-1) fHM->H1("MessageCount")->Fill(TString(spadic+"_"+stopName+" n-fold"),1); // replace weight 1 with number of lost messages
  }
 // complicated loop over sorted map of timestamps
  for(std::map<TString, std::map<ULong_t, std::vector<CbmSpadicRawMessage*> > >::iterator it = timeBuffer.begin() ; it != timeBuffer.end(); it++){
    // complicated loop over sorted map of raw messages
    for (std::map<ULong_t, std::vector<CbmSpadicRawMessage*> > ::iterator it2 = it->second.begin() ; it2 != it->second.end(); it2++) {
      //      LOG(INFO) <<  "ClusterSize:" << Int_t(it2->second.size()) << FairLogger::endl;
      //for (std::vector<CbmSpadicRawMessage*>::iterator it3 = it2->second.begin(); it3 != it2->second.end(); it3++) {
      for(Int_t i = 0; i < Int_t(it2->second.size()); i++){
	//delete it2->second[i];//it3->second;
	// here: looping through the vector
      }
      it2->second.clear();
    }
    it->second.clear();
  }
  timeBuffer.clear();

  //fill number of spadic-messages in tscounter-graph
  //length of one timeslice: m * n * 8 ns, with e.g. n=1250 length of microslice and m=100 microslices in one timeslice at SPS2015
    fHM->G1("TsCounter")->SetPoint(fHM->G1("TsCounter")->GetN(),fNrTimeSlices+1,nSpadicMessages);
    fHM->G1("TsCounter0")->SetPoint(fHM->G1("TsCounter0")->GetN(),fNrTimeSlices+1,nSpadicMessages0);
    fHM->G1("TsCounter1")->SetPoint(fHM->G1("TsCounter1")->GetN(),fNrTimeSlices+1,nSpadicMessages1);

  if(fNrTimeSlices==0){
    if(fHM->G1("TsCounter")->GetN()==0){
      LOG(INFO ) << "Expected entries in TsCounter after finishinig first TimeSlice, but found none." << FairLogger::endl;
    }
  }
  fNrTimeSlices++;
}
void CbmTrdTimeCorrel::Finish()
{
  TCanvas *c1 = new TCanvas("c1","c1",4*400,4*300);
  c1->Divide(2,2);
  c1->cd(1);
  fHM->G1("TsCounter")->Draw("AL");
  fHM->G1("TsCounter")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounter")->GetYaxis()->SetTitle("total SPADIC messages");
  c1->cd(3);
  fHM->G1("TsCounter0")->Draw("AL");
  fHM->G1("TsCounter0")->SetLineColor(kRed);
  fHM->G1("TsCounter0")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounter0")->GetYaxis()->SetTitle("SPADIC0 messages");
  c1->cd(4);
  fHM->G1("TsCounter1")->Draw("AL");
  fHM->G1("TsCounter1")->SetLineColor(kBlue);
  fHM->G1("TsCounter1")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounter1")->GetYaxis()->SetTitle("SPADIC1 messages");
  c1->SaveAs("pics/TsCounter.png");
  //Buffer (map) or multi SPADIC data streams based analyis have to be done here!!
  LOG(DEBUG) << "Finish of CbmTrdTimeCorrel" << FairLogger::endl;
  LOG(INFO) << "Write histo list to " << FairRootManager::Instance()->GetOutFile()->GetName() << FairLogger::endl;
  FairRootManager::Instance()->GetOutFile()->cd();
  fHM->WriteToFile();
  //delete c1;
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
  fHM->Add("MessageCount", new TH1F("MessageCount","MessageCount",16,0,16));
  for(Int_t spadic = 0; spadic < 2; ++spadic) {
    for(Int_t stopType = 0; stopType < 6; ++stopType) {
      fHM->H1("MessageCount")->GetXaxis()->SetBinLabel(8*spadic+stopType+1,TString(spadicName[spadic]+"_"+stopTypes[stopType]));
    }
    fHM->H1("MessageCount")->GetXaxis()->SetBinLabel(8*spadic+6+1,TString(spadicName[spadic]+"_Info mess"));
    fHM->H1("MessageCount")->GetXaxis()->SetBinLabel(8*spadic+7+1,TString(spadicName[spadic]+"_Info mess n-fold"));
  }
  fHM->Add("TsCounter", new TGraph());
  fHM->Add("TsCounter0", new TGraph());
  fHM->Add("TsCounter1", new TGraph());
  
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

TString CbmTrdTimeCorrel::GetStopName(Int_t stopType)
{
  TString stopName="";
  switch (stopType) {
  case (-1):
    stopName="Info mess";
    break;
  case (0):
    stopName="Normal end of message";
    break;
  case (1):
    stopName="Channel buffer full";
    break;
  case (2):
    stopName="Ordering FIFO full";
    break;
  case (3):
    stopName="Multi hit";
    break;
  case (4):
    stopName="Multi hit and channel buffer full";
    break;
  case (5):
    stopName="Multi hit and ordering FIFO full";
    break;
  default:
    LOG(ERROR) << "stopType " << stopType << "not known." << FairLogger::endl;
    break;
  }
  return stopName;
}
