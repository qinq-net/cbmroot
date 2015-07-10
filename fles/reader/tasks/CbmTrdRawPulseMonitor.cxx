#include "CbmTrdRawPulseMonitor.h"
#include "CbmTrdRawBeamProfile.h"

#include "CbmSpadicRawMessage.h"
#include "CbmTrdDigi.h"
#include "CbmTrdCluster.h"
#include "CbmHistManager.h"
#include "CbmBeamDefaults.h"
#include "CbmTrdAddress.h"
#include "CbmTrdDaqBuffer.h"

#include "FairLogger.h"

#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"

#include "TString.h"
#include "TStyle.h"

#include <cmath>
#include <map>
#include <vector>
// ---- Default constructor -------------------------------------------
CbmTrdRawPulseMonitor::CbmTrdRawPulseMonitor()
  : FairTask("CbmTrdRawPulseMonitor"),
    fRawSpadic(NULL),
    fMonitor(NULL)
{
  LOG(DEBUG) << "Default Constructor of CbmTrdRawPulseMonitor" << FairLogger::endl;
}

// ---- Destructor ----------------------------------------------------
CbmTrdRawPulseMonitor::~CbmTrdRawPulseMonitor()
{
  LOG(DEBUG) << "Destructor of CbmTrdRawPulseMonitor" << FairLogger::endl;
}

// ----  Initialisation  ----------------------------------------------
void CbmTrdRawPulseMonitor::SetParContainers()
{
  LOG(DEBUG) << "SetParContainers of CbmTrdRawPulseMonitor" << FairLogger::endl;
  // Load all necessary parameter containers from the runtime data base
  /*
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

  <CbmTrdRawPulseMonitorDataMember> = (<ClassPointer>*)
    (rtdb->getContainer("<ContainerName>"));
  */
}

// ---- Init ----------------------------------------------------------
InitStatus CbmTrdRawPulseMonitor::Init()
{
  LOG(DEBUG) << "Initilization of CbmTrdRawPulseMonitor" << FairLogger::endl;

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance();

  // Get a pointer to the previous already existing data level
  fRawSpadic = static_cast<TClonesArray*>(ioman->GetObject("SpadicRawMessage"));
  if ( ! fRawSpadic ) {
    LOG(FATAL) << "No InputDataLevelName array!\n CbmTrdRawPulseMonitor will be inactive" << FairLogger::endl;
    return kERROR;
  }
  fMonitor = new TCanvas("PulseMonitor","PulseMonitor", 0, 0, 1700, 1000);
  fMonitor->Divide(4,4);
  rawpulse = new TH1I("rawPulse","rawPulse",32,-0.5,31.5);
  rawpulse->GetYaxis()->SetRangeUser(-255,256);
  return kSUCCESS;
}

// ---- ReInit  -------------------------------------------------------
InitStatus CbmTrdRawPulseMonitor::ReInit()
{
  LOG(DEBUG) << "Initilization of CbmTrdRawPulseMonitor" << FairLogger::endl;
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void CbmTrdRawPulseMonitor::Exec(Option_t*)
{
  CbmTrdRawBeamProfile* dummy = new CbmTrdRawBeamProfile();
  const Int_t maxNrColumns = 16;
  //TH1I* rawpulse = new TH1I("rawPulse","rawPulse",32,-0.5,31.5);
 
  /*
    This function is reinitialized for each new TimeSliceContainer
  */
  fContainerCounter++;
  /*
    Int_t channelMapping[32] = {31,15,30,14,29,13,28,12,27,11,26,10,25, 9,24, 8,
    23, 7,22, 6,21, 5,20, 4,19, 3,18, 2,17, 1,16, 0};
  */
  TString timebinName[32] = { "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", 
			      "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", 
			      "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", 
			      "30", "31"};

  //std::map<TString, std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> > > timeBuffer;// <ASIC ID "Syscore%d_Spadic%d"<Time, <CombiId, SpadicMessage> >

  Int_t entriesInMessage = fRawSpadic->GetEntriesFast();


  LOG(DEBUG) << "Container:                                       " << fContainerCounter << FairLogger::endl;
  LOG(DEBUG) << "Entries in Message:                              " << entriesInMessage << FairLogger::endl;
  LOG(DEBUG) << "Entries in total:                                " << fMessageCounter << FairLogger::endl;
  // Find info about hitType, stopType and infoType in cbmroot/fles/spadic/message/constants/..
  /*
    TString triggerTypes[4] = {"Global trigger",
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
    "infoType out of array"};
  */
  //if (entriesInMessage > 1) entriesInMessage = 1; // for fast data visualization
 
  Int_t rowId(0), columnId(0), combiId(0);
  Int_t eventCounter = 0;
  for (Int_t i=0; i < entriesInMessage; ++i) {
    fMessageCounter++;
    CbmSpadicRawMessage* raw = /*static_cast<*/(CbmSpadicRawMessage*)/*>*/(fRawSpadic->At(i));
    Int_t eqID = raw->GetEquipmentID();
    Int_t sourceA = raw->GetSourceAddress();
    //printf("EI%i SA%i ->",eqID,sourceA);
    Int_t chID = raw->GetChannelID();
    Int_t SysId(-1), SpaId(-1);
    Int_t nrSamples=raw->GetNrSamples();
    Int_t triggerType=raw->GetTriggerType();
    Int_t stopType=raw->GetStopType();
    Int_t infoType=raw->GetInfoType();
    if (stopType != 0 && stopType != 3)
      if (infoType > 6) {
	LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter <<  " eqId " << eqID << " sourceA " << sourceA <<  " InfoType " << infoType << " out of range. Set to 7. StopType:" << stopType << " TriggerType:" << triggerType << FairLogger::endl;
	infoType = 7;
      }
    Int_t groupId=raw->GetGroupId();
    ULong_t time = raw->GetFullTime();
 
   
    TString syscore="";
    switch (eqID) {
    case kMuenster:  // Muenster
      syscore="SysCore0_";
      SysId = 0;
      break;
    case kFrankfurt: // Frankfurt
      syscore="SysCore1_";
      SysId = 1;
      break;
    case kBucarest: // Bucarest
      syscore="SysCore2_";
      SysId = 2;
      break;
    default:
      LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter <<  " EquipmentID " << eqID << "not known." << FairLogger::endl;
      break;
    }     
      
    TString spadic="";
    switch (sourceA) {
    case (SpadicBaseAddress+0):  // first spadic
      spadic="Spadic0";
      SpaId = 0;
      break;
    case (SpadicBaseAddress+1):  // first spadic
      spadic="Spadic0";
      SpaId = 1;
      chID += 16;
      break;
    case (SpadicBaseAddress+2):  // second spadic
      spadic="Spadic1";
      SpaId = 2;
      break;
    case (SpadicBaseAddress+3):  // second spadic
      spadic="Spadic1";
      SpaId = 3;
      chID += 16;
      break;
    case (SpadicBaseAddress+4):  // third spadic
      spadic="Spadic2";
      SpaId = 4;
      break;
    case (SpadicBaseAddress+5):  // third spadic
      spadic="Spadic2";
      SpaId = 5;
      chID += 16;
      break;
    default:
      LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter << " Source Address " << sourceA << "not known." << FairLogger::endl;
      break;
    }   

    if (SysId > 0 || SpaId > 1) continue;
    eventCounter++;


    chID = dummy->GetChannelOnPadPlane(chID);//channelMapping[chID];// Remapping from ASIC to pad-plane

    columnId = dummy->GetColumnID(raw);
    rowId = dummy->GetRowID(raw);
    combiId = rowId * (maxNrColumns + 1) + columnId;

    rawpulse->Reset();
    /*
      if (eventCounter % 10 == 0){
      for (Int_t pad = 1; pad <=32; pad++) {
      fMonitor->cd(pad);
      rawpulse->DrawCopy();
      }
      }
    */
    for (Int_t iBin = 0; iBin < raw->GetNrSamples(); iBin++){
      rawpulse->SetBinContent(iBin+1,raw->GetSamples()[iBin]);
    }

    fMonitor->cd(chID+1);   
    rawpulse->DrawCopy("same");
    if (eventCounter % 10 == 0)
      fMonitor->Update();
  } //entriesInMessage
  fMonitor->Update();
  rawpulse->Reset();
  for (Int_t iBin = 0; iBin < 32; iBin++){
    rawpulse->SetBinContent(iBin+1,-256);
  }
  for (Int_t pad = 1; pad <=32; pad++) {
    fMonitor->cd(pad);
    rawpulse->DrawCopy();
  }  
}

  // ---- Finish --------------------------------------------------------
  void CbmTrdRawPulseMonitor::Finish()
  {
    LOG(DEBUG) << "Finish of CbmTrdRawPulseMonitor" << FairLogger::endl;
    // Write to file
    fHM->WriteToFile();
    // Update Histos and Canvases
  
    LOG(INFO) << "CbmTrdRawPulseMonitor::Finish Container:            " << fContainerCounter << FairLogger::endl;
    LOG(INFO) << "CbmTrdRawPulseMonitor::Finish Messages:             " << fMessageCounter << FairLogger::endl;
  }

  ClassImp(CbmTrdRawPulseMonitor)
