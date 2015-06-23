#include "CbmTrdRawBeamProfile.h"

#include "CbmSpadicRawMessage.h"
#include "CbmTrdDigi.h"
#include "CbmTrdCluster.h"
#include "CbmHistManager.h"
#include "CbmBeamDefaults.h"

#include "CbmTrdDaqBuffer.h"

#include "FairLogger.h"

#include "TH1.h"
#include "TH2.h"

#include "TString.h"

#include <cmath>
#include <map>
#include <vector>
// ---- Default constructor -------------------------------------------
CbmTrdRawBeamProfile::CbmTrdRawBeamProfile()
  : FairTask("CbmTrdRawBeamProfile"),
    fRawSpadic(NULL),
    fDigis(NULL),
    fClusters(NULL),
    fiDigi(0),
    fiCluster(0),
    fHM(new CbmHistManager()),
    fMessageCounter(0),
    fContainerCounter(0),
    fTimeBuffer()
{
  LOG(DEBUG) << "Default Constructor of CbmTrdRawBeamProfile" << FairLogger::endl;
}

// ---- Destructor ----------------------------------------------------
CbmTrdRawBeamProfile::~CbmTrdRawBeamProfile()
{
  fDigis->Delete();
  delete fDigis;
  fClusters->Delete();
  delete fClusters;
  for (std::map<TString, std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> > >::iterator SpaSysIt = fTimeBuffer.begin() ; SpaSysIt != fTimeBuffer.end(); SpaSysIt++){
    for (std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> > ::iterator timeIt = SpaSysIt->second.begin() ; timeIt != SpaSysIt->second.end(); timeIt++){
      for (std::map<Int_t, CbmSpadicRawMessage*> ::iterator combiIt = timeIt->second.begin(); combiIt != timeIt->second.end(); combiIt++){
	if(combiIt->second != NULL)
	  delete combiIt->second;
      }
      timeIt->second.clear();   
    }
    SpaSysIt->second.clear();
  }
  LOG(DEBUG) << "Destructor of CbmTrdRawBeamProfile" << FairLogger::endl;
}

// ----  Initialisation  ----------------------------------------------
void CbmTrdRawBeamProfile::SetParContainers()
{
  LOG(DEBUG) << "SetParContainers of CbmTrdRawBeamProfile" << FairLogger::endl;
  // Load all necessary parameter containers from the runtime data base
  /*
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

  <CbmTrdRawBeamProfileDataMember> = (<ClassPointer>*)
    (rtdb->getContainer("<ContainerName>"));
  */
}

// ---- Init ----------------------------------------------------------
InitStatus CbmTrdRawBeamProfile::Init()
{
  LOG(DEBUG) << "Initilization of CbmTrdRawBeamProfile" << FairLogger::endl;

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance();

  // Get a pointer to the previous already existing data level
  fRawSpadic = static_cast<TClonesArray*>(ioman->GetObject("SpadicRawMessage"));
  if ( ! fRawSpadic ) {
    LOG(FATAL) << "No InputDataLevelName array!\n CbmTrdRawBeamProfile will be inactive" << FairLogger::endl;
    return kERROR;
  }

  fDigis = new TClonesArray("CbmTrdDigi", 100);
  ioman->Register("TrdDigi", "TRD Digis", fDigis, kTRUE);

  fClusters = new TClonesArray("CbmTrdCluster",100);
  ioman->Register("TrdCluster", "TRD Clusters", fClusters, kTRUE);

  // Do whatever else is needed at the initilization stage
  // Create histograms to be filled
  // initialize variables
  CreateHistograms();

  return kSUCCESS;

}

// ---- ReInit  -------------------------------------------------------
InitStatus CbmTrdRawBeamProfile::ReInit()
{
  LOG(DEBUG) << "Initilization of CbmTrdRawBeamProfile" << FairLogger::endl;
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void CbmTrdRawBeamProfile::Exec(Option_t*)
{
  const Int_t maxNrColumns = 16;
  /*
    This function is reinitialized for each new TimeSliceContainer
  */
  fContainerCounter++;
  Int_t channelMapping[32] = {31,15,30,14,29,13,28,12,27,11,26,10,25, 9,24, 8,
			      23, 7,22, 6,21, 5,20, 4,19, 3,18, 2,17, 1,16, 0};

  TString timebinName[32] = { "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", 
			      "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", 
			      "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", 
			      "30", "31"};

  //std::map<TString, std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> > > timeBuffer;// <ASIC ID "Syscore%d_Spadic%d"<Time, <CombiId, SpadicMessage> >

  Int_t entriesInMessage = fRawSpadic->GetEntriesFast();
  Int_t entries = fDigis->GetEntriesFast();
  Int_t sumTrigger[3][6] = {{0}};

  LOG(DEBUG) << "Container:                                       " << fContainerCounter << FairLogger::endl;
  LOG(DEBUG) << "Entries in Message:                              " << entriesInMessage << FairLogger::endl;
  LOG(DEBUG) << "Entries in total:                                " << fMessageCounter << FairLogger::endl;
  LOG(DEBUG) << "Digis in TClonesArray:                           " << fDigis->GetEntriesFast() << FairLogger::endl;
  LOG(DEBUG) << "Clusters in TClonesArray:                        " << fClusters->GetEntriesFast() << FairLogger::endl;
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
  //if (entriesInMessage > 1000) entriesInMessage = 1000; // for fast data visualization
  ULong_t lastSpadicTime[3][6] = {{0}}; //[sys][spa]
  ULong_t lastSpadicTimeCh[3][6][32] = {{0}}; //[sys][spa][ch]
  Int_t lastTriggerType[32] = {-1};
  Int_t clusterSize[3][6] = {{1}};
  Int_t bufferOverflow(0), layerId(0), moduleId(0), sectorId(0), rowId(0), columnId(0), combiId(0);
  Int_t wrongTimeOrder[3][6] = {{0}};

  for (Int_t i=0; i < entriesInMessage; ++i) {
    fMessageCounter++;
    CbmSpadicRawMessage* raw = static_cast<CbmSpadicRawMessage*>(fRawSpadic->At(i));
    Int_t eqID = raw->GetEquipmentID();
    Int_t sourceA = raw->GetSourceAddress();
    Int_t chID = raw->GetChannelID();
    Int_t SysId(-1), SpaId(-1);
    Int_t nrSamples=raw->GetNrSamples();
    Int_t triggerType=raw->GetTriggerType();
    Int_t stopType=raw->GetStopType();
    Int_t infoType=raw->GetInfoType();
    if (stopType > 0)
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
    if(chID % 2 == 0)
      rowId = 0;
    else
      rowId = 1;

    chID = channelMapping[chID];// Remapping from ASIC to pad-plane
    // Compare to channel mapping from pad-plane to ASIC
    columnId = chID;
    if (columnId > 16) 
      columnId -= 16;

    combiId = rowId * (maxNrColumns + 1) + columnId;


    if (SysId == 0){
      if (SpaId == 0 || SpaId == 1){
	layerId = 0;
	moduleId = 0;
      } else if (SpaId == 2 || SpaId == 3){
	layerId = 1;
	moduleId = 1;
      } else if (SpaId == 4 || SpaId == 5){
	layerId = 2;
	moduleId = 2;
      } else { LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter <<  " SpaId " << SpaId << "not known." << FairLogger::endl;}
    } else  if (SysId == 1){
      if (SpaId == 0 || SpaId == 1){
	layerId = 3;
	moduleId = 3;
      } else if (SpaId == 2 || SpaId == 3){
	layerId = 4;
	moduleId = 4;
      } else if (SpaId == 4 || SpaId == 5){
	layerId = 5;
	moduleId = 5;
      } else { LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter << " SpaId " << SpaId << "not known." << FairLogger::endl;}
    } else  if (SysId == 2){
      if (SpaId == 0 || SpaId == 1){
	layerId = 6;
	moduleId = 6;
      } else if (SpaId == 2 || SpaId == 3){
	layerId = 7;
	moduleId = 7;
      } else if (SpaId == 4 || SpaId == 5){
	layerId = 8;
	moduleId = 8;
      } else { LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter << " SpaId " << SpaId << "not known." << FairLogger::endl;}
    } else { LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter << " SysId " << SysId << "not known." << FairLogger::endl;}

    //if (stopType == 0 && triggerType > 0){

    std::map<Int_t, CbmSpadicRawMessage*>::iterator timeBufferIt = fTimeBuffer[TString(syscore+spadic)][time].find(combiId);
    if (timeBufferIt == fTimeBuffer[TString(syscore+spadic)][time].end())
      fTimeBuffer[TString(syscore+spadic)][time][combiId]=raw;
    else
      LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter << " SysId " << SysId << " SpaId " << SpaId << " at time " << time << " with combiId " << combiId << " already in buffer" << FairLogger::endl;

    //}
    if (time > lastSpadicTime[SysId][SpaId]){
      // ok, next hit
    
      clusterSize[SysId][SpaId] = 1;
    } else if (time == lastSpadicTime[SysId][SpaId]) { // Clusterizer
      // possible FNR trigger
      if (stopType == 0){ // normal ending
	//clusterSize = 1;
	if (triggerType == 0) { // gobal dlm trigger

	} else if (triggerType == 1) { //Self triggered
	  // central pad candidate
	  clusterSize[SysId][SpaId]+=1;
	} else if (triggerType == 2) { //Neighbor triggered
	  // outer pad candidate
	  clusterSize[SysId][SpaId]+=1;
	} else { //Self and neighbor triggered
	  // central pad candidate
	  clusterSize[SysId][SpaId]+=1;
	}
      } else {// multihit or currupted
      }
    } else {
      wrongTimeOrder[SysId][SpaId]++;
      LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter << " SPADIC " << SysId << SpaId << " event time " << time << " < last time " << lastSpadicTime[SysId][SpaId] << " entry " << i << " of " << entriesInMessage << FairLogger::endl;
    }
    TString channelId;
    channelId.Form("_Ch%02d", chID);

    fHM->H1(TString("Message_Length_" + syscore + spadic).Data())->Fill(nrSamples,chID);
    fHM->H1(TString("StopType_Message_Length_" + syscore + spadic).Data())->Fill(nrSamples,stopType);
    fHM->H1(TString("InfoType_Message_Length_" + syscore + spadic).Data())->Fill(nrSamples,infoType);
    fHM->H1(TString("TriggerType_Message_Length_" + syscore + spadic).Data())->Fill(nrSamples,triggerType);
    fHM->H2(TString("TriggerTypes_" + syscore + spadic).Data())->Fill(triggerType,chID);
    fHM->H2(TString("StopTypes_" + syscore + spadic).Data())->Fill(stopType,chID);
    fHM->H2(TString("Trigger_Stop_Types_" + syscore + spadic).Data())->Fill(triggerType,stopType);
    fHM->H2(TString("Trigger_Info_Types_" + syscore + spadic).Data())->Fill(triggerType,infoType);
    fHM->H2(TString("Stop_Info_Types_" + syscore + spadic).Data())->Fill(stopType,infoType);
    fHM->H2(TString("InfoTypes_" + syscore + spadic).Data())->Fill(infoType,chID);
   
    fHM->H1(TString("GroupId_" + syscore + spadic).Data())->Fill(groupId);

    if (stopType > 0){ //corrupt or multi message
      TString histName = "ErrorCounter_" + syscore + spadic;
      fHM->H1(histName.Data())->Fill(chID);   
    } else  {  //only normal message end
      sumTrigger[SysId][SpaId]++;

      TString histName = "CountRate_" + syscore + spadic;
      fHM->H1(histName.Data())->Fill(chID);

      histName = "Noise1_" + syscore + spadic + channelId;
      Float_t Baseline = 0;
      for (Int_t bin = 1; bin < 7; bin++){
	Baseline += raw->GetSamples()[nrSamples-bin];
	fHM->H1(histName.Data())->Fill(raw->GetSamples()[nrSamples-bin]);
      }
      Baseline /= 6.;

      histName = "BaseLine_" + syscore + spadic;
      fHM->H2(histName.Data())->Fill(chID, Baseline);

      Float_t AdcIntegral = 0;
      Float_t maxADC(-300);
      Int_t maxTimeBin(-1);

      for (Int_t bin = 0; bin < nrSamples; bin++) {
	if (raw->GetSamples()[bin] > maxADC){
	  maxADC = raw->GetSamples()[bin];
	  maxTimeBin = bin;
	}
	AdcIntegral += raw->GetSamples()[bin] - Baseline;

	histName = "Signal_Shape_" + syscore + spadic + channelId;
	fHM->H2(histName.Data())->Fill(bin,raw->GetSamples()[bin] - Baseline);
	histName = "Pulse_" + syscore + spadic + channelId;
	fHM->H2(histName.Data())->Fill(bin,raw->GetSamples()[bin]);
      }
      
      for (Int_t bin = 0; bin < nrSamples; bin++) {
	histName = "maxADC_TimeBinCorr_" + syscore + spadic + "_TB" + timebinName[bin];
	fHM->H2(histName.Data())->Fill(raw->GetSamples()[maxTimeBin],raw->GetSamples()[bin]);
      }

      histName = "maxTimeBin_" + syscore + spadic + channelId;
      fHM->H1(histName.Data())->Fill(maxTimeBin);

      histName = "maxADC_" + syscore + spadic + channelId;
      fHM->H1(histName.Data())->Fill(maxADC);

      histName = "maxADC_maxTimeBin_" + syscore + spadic + channelId;
      fHM->H2(histName.Data())->Fill(maxTimeBin,maxADC);

      histName = "Integrated_ADC_Spectrum_" + syscore + spadic;
      if (maxADC > -200) // minimum amplitude threshold to reduce noise
	fHM->H2(histName.Data())->Fill(chID, AdcIntegral);

      histName = "Trigger_Heatmap_" + syscore + spadic;
      if (maxADC > -200){ // minimum amplitude threshold to reduce noise
	fHM->H2(histName.Data())->Fill(columnId  ,rowId);
      }
 
      histName = "DeltaTime_" + syscore + spadic;
      fHM->H1(histName.Data())->Fill(time-lastSpadicTime[SysId][SpaId]);

      histName = "DeltaTime_Left_Right_" + syscore + spadic;
      if (chID < 31)
	fHM->H1(histName.Data())->Fill(time-lastSpadicTimeCh[SysId][SpaId][chID+1]);
      if (chID > 0)
	fHM->H1(histName.Data())->Fill(time-lastSpadicTimeCh[SysId][SpaId][chID-1]);

      histName = "DeltaTime_Left_Right_Trigger_" + syscore + spadic;
      if (triggerType == 1 || triggerType == 3){
	if (chID < 31)
	  if (lastTriggerType[chID+1] == 2 || lastTriggerType[chID+1] == 3)
	    fHM->H1(histName.Data())->Fill(time-lastSpadicTimeCh[SysId][SpaId][chID+1]);
	if (chID > 0)
	  if (lastTriggerType[chID-1] == 2 || lastTriggerType[chID-1] == 3)
	    fHM->H1(histName.Data())->Fill(time-lastSpadicTimeCh[SysId][SpaId][chID-1]);
      }
    }


    lastSpadicTime[SysId][SpaId] = time;
    lastSpadicTimeCh[SysId][SpaId][chID] = time;
    lastTriggerType[chID] = triggerType;

    entries++;
  } //entriesInMessage
  for (Int_t sy = 0; sy < 2; sy++){
    for (Int_t sp = 0; sp < 2; sp++){
      TString histName = "TriggerCounter_SysCore" + std::to_string(sy) + "_Spadic" + std::to_string(sp);
      for (Int_t timeSlice = 1; timeSlice <= fHM->H1(histName.Data())->GetNbinsX(); timeSlice++){
	fHM->H1(histName.Data())->SetBinContent(timeSlice,fHM->H1(histName.Data())->GetBinContent(timeSlice+1)); // shift all bin one to the left
      }
      fHM->H1(histName.Data())->SetBinContent(fHM->H1(histName.Data())->GetNbinsX(),sumTrigger[sy][2*sp] + sumTrigger[sy][2*sp+1]);// set only the spa sys combi to new value
      fHM->H1("TriggerSum")->Fill(TString("SysCore" + std::to_string(sy) + "_Spadic" + std::to_string(sp)),sumTrigger[sy][2*sp] + sumTrigger[sy][2*sp+1]);
    }
  }
  for(Int_t syscore = 0; syscore < 3; ++syscore) {
    for(Int_t spadic = 0; spadic < 3; ++spadic) {
      if(wrongTimeOrder[syscore][spadic] > 0)
	LOG(ERROR) << "SPADIC " << syscore << spadic << " Wrong time stamp order in " << Float_t(wrongTimeOrder[syscore][spadic]*100./entriesInMessage) <<"% of all events " <<  wrongTimeOrder[syscore][spadic] << FairLogger::endl;
    }
  }
}


void CbmTrdRawBeamProfile::Clusterizer()
{
  Int_t bufferOverflow(0), layerId(0), moduleId(0), sectorId(0), rowId(0), columnId(0);
  ULong_t lastClusterTime = 0;
  std::vector<Int_t> digiIndices;
  for (std::map<TString, std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> > >::iterator SpaSysIt = fTimeBuffer.begin() ; SpaSysIt != fTimeBuffer.end(); SpaSysIt++){
    for (std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> > ::iterator timeIt = SpaSysIt->second.begin() ; timeIt != SpaSysIt->second.end(); timeIt++){
      LOG(DEBUG) <<  "ClusterSize:" << Int_t(timeIt->second.size()) << FairLogger::endl;
      fHM->H1(TString("ClusterSize_" + SpaSysIt->first).Data())->Fill(Int_t(timeIt->second.size()));

      //Delta time between time clusters
      fHM->H1(TString("DeltaTime_Cluster_" + SpaSysIt->first).Data())->Fill(timeIt->first - lastClusterTime);
      lastClusterTime = timeIt->first;

      Int_t lastCombiID = -1;
      digiIndices.clear();
      for (std::map<Int_t, CbmSpadicRawMessage*> ::iterator combiIt = timeIt->second.begin(); combiIt != timeIt->second.end(); combiIt++){
	layerId = GetLayerID(combiIt->second);
	moduleId = GetModuleID(combiIt->second);
	sectorId = GetSectorID(combiIt->second);
	rowId = GetRowID(combiIt->second);
	columnId = GetColumnID(combiIt->second);
	//printf("la%i mo%i se%i ro%i co%i\n",layerId,moduleId,sectorId,rowId,columnId);
	new ((*fDigis)[fiDigi]) CbmTrdDigi(layerId,moduleId,sectorId,rowId,columnId,
					   combiIt->second->GetFullTime()*57.1,//57,1ns per timestamp
					   combiIt->second->GetTriggerType(), combiIt->second->GetInfoType(), combiIt->second->GetStopType(),  bufferOverflow, 
					   combiIt->second->GetNrSamples(), combiIt->second->GetSamples());
	if (combiIt != timeIt->second.begin())
	  fHM->H1(TString("DeltaCh_Cluster_" + SpaSysIt->first).Data())->Fill(combiIt->first - lastCombiID);

	if (combiIt->first - lastCombiID == 1){
	  digiIndices.push_back(fiDigi);
	  //} else if (){ // Merge clusters across adjacent rows

	} else {
	  CbmTrdCluster* cluster = new ((*fClusters)[fiCluster]) CbmTrdCluster();
	  cluster->SetAddress(CbmTrdAddress::GetAddress(layerId,moduleId,sectorId,rowId,columnId));
	  cluster->SetDigis(digiIndices);
	  digiIndices.clear();
	  fiCluster++;
	}
	//delete combiIt->second;
	lastCombiID = combiIt->first;
	fiDigi++;
      }
      //timeIt->second.clear();   
    }
    //SpaSysIt->second.clear();
  }
}



Int_t CbmTrdRawBeamProfile::GetModuleID(CbmSpadicRawMessage* raw)
{
  Int_t eqID = raw->GetEquipmentID();
  Int_t sourceA = raw->GetSourceAddress();
  Int_t moduleId = 0;
  switch (eqID) {
  case kMuenster:  // Muenster
    moduleId = 0 * 3;
    break;
  case kFrankfurt: // Frankfurt
    moduleId = 1 * 3;
    break;
  case kBucarest: // Bucarest     
    moduleId = 2 * 3;
    break;
  default:
    LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter <<  " EquipmentID " << eqID << "not known." << FairLogger::endl;
    break;
  }     
  switch (sourceA) {
  case (SpadicBaseAddress+0):  // first spadic
    moduleId += 0;
    break;
  case (SpadicBaseAddress+1):  // first spadic
    moduleId += 0;
    break;
  case (SpadicBaseAddress+2):  // second spadic
    moduleId += 1;
    break;
  case (SpadicBaseAddress+3):  // second spadic
    moduleId += 1;
    break;
  case (SpadicBaseAddress+4):  // third spadic
    moduleId += 2;
    break;
  case (SpadicBaseAddress+5):  // third spadic
    moduleId += 2;
    break;
  default:
    LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter << " Source Address " << sourceA << "not known." << FairLogger::endl;
    break;
  }   
  return moduleId; // SysCoreId [0,2] * SpaId [0,2] same as layer
}
Int_t CbmTrdRawBeamProfile::GetLayerID(CbmSpadicRawMessage* raw)
{
  return GetModuleID(raw);
}
Int_t CbmTrdRawBeamProfile::GetSectorID(CbmSpadicRawMessage* raw)
{
  return 0;
}
Int_t CbmTrdRawBeamProfile::GetRowID(CbmSpadicRawMessage* raw)
{
  Int_t chID = raw->GetChannelID() + raw->GetSourceAddress() * 16;
  Int_t columnId = GetChannelOnPadPlane(chID);
  if (columnId > 16) 
    return 1;
  else
    return 0;
}
Int_t CbmTrdRawBeamProfile::GetColumnID(CbmSpadicRawMessage* raw)
{
  Int_t chID = raw->GetChannelID() + raw->GetSourceAddress() * 16;
  Int_t columnId = GetChannelOnPadPlane(chID);
  if (columnId >= 16) 
    columnId -= 16;
  return columnId;
}
Int_t CbmTrdRawBeamProfile::GetChannelOnPadPlane(Int_t SpadicChannel)
{
  Int_t channelMapping[32] = {31,15,30,14,29,13,28,12,27,11,26,10,25, 9,24, 8,
			      23, 7,22, 6,21, 5,20, 4,19, 3,18, 2,17, 1,16, 0};
  return channelMapping[SpadicChannel];
}

  // ---- Finish --------------------------------------------------------
  void CbmTrdRawBeamProfile::Finish()
  {
    Clusterizer();

    LOG(DEBUG) << "Finish of CbmTrdRawBeamProfile" << FairLogger::endl;
    // Write to file
    fHM->WriteToFile();
    // Update Histos and Canvases
    LOG(INFO) << "CbmTrdRawBeamProfile::Finish Container:            " << fContainerCounter << FairLogger::endl;
    LOG(INFO) << "CbmTrdRawBeamProfile::Finish Messages:             " << fMessageCounter << FairLogger::endl;
    LOG(INFO) << "CbmTrdRawBeamProfile::Finish Digis:                " << fDigis->GetEntriesFast() << FairLogger::endl;
    LOG(INFO) << "CbmTrdRawBeamProfile::Finish Clusters:             " << fClusters->GetEntriesFast() << FairLogger::endl;
  }

void CbmTrdRawBeamProfile::CreateHistograms()
{

  // Create histograms for 3 Syscores with maximum 3 Spadics

  TString syscoreName[3] = { "SysCore0", "SysCore1", "SysCore2" };
  TString spadicName[3]  = { "Spadic0",  "Spadic1",  "Spadic2" };
  TString channelName[32] = { "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", 
			      "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", 
			      "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", 
			      "30", "31"};
  TString timebinName[32] = { "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", 
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

  fHM->Add("TriggerSum", new TH1I("TriggerSum", "TriggerSum", 9,0,9));
  for(Int_t syscore = 0; syscore < 3; ++syscore) {
    for(Int_t spadic = 0; spadic < 3; ++spadic) {
      fHM->H1("TriggerSum")->GetXaxis()->SetBinLabel(3*syscore+spadic+1,TString(syscoreName[syscore]+"_"+spadicName[spadic]));

      TString histName = "CountRate_" + syscoreName[syscore] + "_" + spadicName[spadic];
      TString title = histName + ";Channel;Counts";
      fHM->Add(histName.Data(), new TH1I(histName, title, 32, 0, 32));

      histName = "DeltaTime_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Delta t ;Counts";
      fHM->Add(histName.Data(), new TH1I(histName, title, 5101, -100.5, 5000.5));
   
      histName = "DeltaTime_Left_Right_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Delta t ;Counts";
      fHM->Add(histName.Data(), new TH1I(histName, title, 5101, -100.5, 5000.5));

      histName = "DeltaTime_Left_Right_Trigger_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Delta t ;Counts";
      fHM->Add(histName.Data(), new TH1I(histName, title, 5101, -100.5, 5000.5));

      histName = "DeltaTime_Cluster_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Delta t ;Counts";
      fHM->Add(histName.Data(), new TH1I(histName, title, 5101, -100.5, 5000.5));

      histName = "DeltaCh_Cluster_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Delta t ;Counts";
      fHM->Add(histName.Data(), new TH1I(histName, title, 65, -32.5, 32.5));

      histName = "StopType_Message_Length_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Time-Bins ;";
      fHM->Add(histName.Data(), new TH2I(histName, title, 33, -0.5, 32.5,6,0,6));
      for (Int_t sType=0; sType < 6; sType++)
	fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(sType+1,stopTypes[sType]); 

      histName = "InfoType_Message_Length_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Time-Bins ;";
      fHM->Add(histName.Data(), new TH2I(histName, title, 33, -0.5, 32.5, 8, 0, 8));
      for (Int_t iType=0; iType < 8; iType++)
	fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(iType+1,infoTypes[iType]);

      histName = "TriggerType_Message_Length_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Time-Bins ;";
      fHM->Add(histName.Data(), new TH2I(histName, title, 33, -0.5, 32.5,4,0,4));
      for (Int_t tType=0; tType < 4; tType++)
	fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(tType+1,triggerTypes[tType]);

      histName = "Message_Length_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Time-Bins ;Channel";
      fHM->Add(histName.Data(), new TH2I(histName, title, 33, -0.5, 32.5,32, 0, 32));
      /*
	histName = "TriggerTypes_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "; ;Counts";
	fHM->Add(histName.Data(), new TH1I(histName, title, 4, 0, 4));
	for (Int_t tType=0; tType < 4; tType++)
	fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(tType+1,triggerTypes[tType]);
      
	histName = "StopTypes_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "; ;Counts";
	fHM->Add(histName.Data(), new TH1I(histName, title, 6, 0, 6));
	for (Int_t sType=0; sType < 6; sType++)
	fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(sType+1,stopTypes[sType]);

	histName = "InfoTypes_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "; ;Counts";
	fHM->Add(histName.Data(), new TH1I(histName, title, 8, 0, 8));
	for (Int_t iType=0; iType < 8; iType++)
	fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(iType+1,infoTypes[iType]);
      */
      histName = "TriggerTypes_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + "; ;Channel";
      fHM->Add(histName.Data(), new TH2I(histName, title, 4, 0, 4,32,0,32));
      for (Int_t tType=0; tType < 4; tType++)
	fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(tType+1,triggerTypes[tType]);
   
      histName = "StopTypes_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + "; ;Channel";
      fHM->Add(histName.Data(), new TH2I(histName, title, 6, 0, 6,32,0,32));
      for (Int_t sType=0; sType < 6; sType++)
	fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(sType+1,stopTypes[sType]);    
  
      histName = "InfoTypes_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + "; ;Channel";
      fHM->Add(histName.Data(), new TH2I(histName, title, 8, 0, 8,32,0,32));
      for (Int_t iType=0; iType < 8; iType++)
	fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(iType+1,infoTypes[iType]);

      histName = "Trigger_Stop_Types_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + "; ;";
      fHM->Add(histName.Data(), new TH2I(histName, title, 4, 0, 4, 6, 0, 6));
      for (Int_t tType=0; tType < 4; tType++)
	fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(tType+1,triggerTypes[tType]);
      for (Int_t sType=0; sType < 6; sType++)
	fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(sType+1,stopTypes[sType]);   

      histName = "Trigger_Info_Types_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + "; ;";
      fHM->Add(histName.Data(), new TH2I(histName, title, 4, 0, 4, 8, 0, 8));
      for (Int_t tType=0; tType < 4; tType++)
	fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(tType+1,triggerTypes[tType]);
      for (Int_t iType=0; iType < 8; iType++)
	fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(iType+1,infoTypes[iType]);


      histName = "Stop_Info_Types_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + "; ;";
      fHM->Add(histName.Data(), new TH2I(histName, title, 6, 0, 6, 8, 0, 8));
      for (Int_t sType=0; sType < 6; sType++)
	fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(sType+1,stopTypes[sType]);    
      for (Int_t iType=0; iType < 8; iType++)
	fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(iType+1,infoTypes[iType]);


      histName = "GroupId_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + "; ;Counts";
      fHM->Add(histName.Data(), new TH1I(histName, title, 2, 0, 2));


      histName = "ClusterSize_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Cluster Size [Channel] ;Counts";
      fHM->Add(histName.Data(), new TH1I(histName, title, 10, -0.5, 9.5));


      histName = "TriggerCounter_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";TimeSlice;Trigger / TimeSlice";
      fHM->Add(histName.Data(), new TH1I(histName, title, 500, 0, 500));

      histName = "ErrorCounter_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Channel;ADC value in Bin 0";
      fHM->Add(histName.Data(), new TH1I(histName, title, 32, 0, 32));

      histName = "BaseLine_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Channel;ADC value";
      fHM->Add(histName.Data(), new TH2F(histName, title, 32, 0, 32, 511, -256, 255));

      histName = "Integrated_ADC_Spectrum_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Channel;Integr. ADC values in Bin [0,31]";
      fHM->Add(histName.Data(), new TH2F(histName, title, 32, 0, 32, 2*33*256, -33*256, 33*256));

      histName = "Trigger_Heatmap_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Channel;Trigger Counter";
      fHM->Add(histName.Data(), new TH2I(histName, title, 16, 0, 16, 2, 0, 2));
      /*
	histName = "maxADC_TimeBinCorr_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "max ADC value; value in time bin";
	fHM->Add(histName.Data(), new TH2I(histName, title, 511, -256, 255, 511, -256, 255));
      */



      for(Int_t  channel = 0; channel < 32; channel++) {
	histName = "Noise1_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_Ch" + channelName[channel];
	title = histName + ";ADC value;Count";
	fHM->Add(histName.Data(), new TH1F(histName, title, 511, -256, 255));
      }

      for(Int_t  channel = 0; channel < 32; channel++) {
	histName = "Signal_Shape_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_Ch" + channelName[channel];
	title = histName + ";Time Bin;ADC value";
	fHM->Add(histName.Data(), new TH2I(histName, title, 32, 0, 32, 767, -256, 511));
      }
      for(Int_t  channel = 0; channel < 32; channel++) {
	histName = "Pulse_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_Ch" + channelName[channel];
	title = histName + ";Time Bin;ADC value";
	fHM->Add(histName.Data(), new TH2I(histName, title, 32, 0, 32, 767, -256, 511));
      }
      for(Int_t  channel = 0; channel < 32; channel++) {
	histName = "maxTimeBin_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_Ch" + channelName[channel];
	title = histName + ";max Time Bin;count";
	fHM->Add(histName.Data(), new TH1I(histName, title, 33, 0, 33));
      }
      for(Int_t  channel = 0; channel < 32; channel++) {
	histName = "maxADC_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_Ch" + channelName[channel];
	title = histName + ";max ADC value;count";
	fHM->Add(histName.Data(), new TH1I(histName, title, 511, -256, 255));
      }
      for(Int_t  channel = 0; channel < 32; channel++) {
	histName = "maxADC_maxTimeBin_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_Ch" + channelName[channel];
	title = histName + ";max Time Bin;max ADC value";
	fHM->Add(histName.Data(), new TH2I(histName, title, 33, 0, 33, 511, -256, 255));
      }
      
      //for(Int_t  channel = 0; channel < 32; channel++) {
      for(Int_t  bin = 0; bin < 32; bin++) {
	histName = "maxADC_TimeBinCorr_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_TB" + timebinName[bin];
	title = histName + "max ADC value; value in time bin";
	fHM->Add(histName.Data(), new TH2I(histName, title, 511, -256, 255, 511, -256, 255));
      }
      //	}
      
    }
  } 
}
  ClassImp(CbmTrdRawBeamProfile)
