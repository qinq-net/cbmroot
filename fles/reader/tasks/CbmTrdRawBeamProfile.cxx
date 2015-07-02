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

#include "TString.h"
#include "TStyle.h"

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
  CbmTrdRawBeamProfile::CreateHistograms();

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
  //if (entriesInMessage > 1) entriesInMessage = 1; // for fast data visualization
  ULong_t lastSpadicTime[3][6] = {{0}}; //[sys][spa]
  ULong_t lastSpadicTimeCh[3][6][32] = {{{0}}}; //[sys][spa][ch]
  Int_t lastTriggerType[32] = {-1};
  Int_t clusterSize[3][6] = {{1}};
  Int_t rowId(0), columnId(0), combiId(0);
  Int_t wrongTimeOrder[3][6] = {{0}};
 
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
    if(chID % 2 == 0)
      rowId = 0;
    else
      rowId = 1;
    //printf("EI%i SA%i ->|\n",eqID,sourceA);
    chID = channelMapping[chID];// Remapping from ASIC to pad-plane
    // Compare to channel mapping from pad-plane to ASIC
    columnId = chID;
    if (columnId > 16) 
      columnId -= 16;

    combiId = rowId * (maxNrColumns + 1) + columnId;

    //if (stopType == 0 && triggerType > 0){

    std::map<Int_t, CbmSpadicRawMessage*>::iterator timeBufferIt = fTimeBuffer[TString(syscore+spadic)][time].find(combiId);
    if (timeBufferIt == fTimeBuffer[TString(syscore+spadic)][time].end()){
      //printf("%p  time %12lu %12lu %s CI%i EI%i SA%i TT%i\n",std::addressof(raw),time,raw->GetFullTime(),(syscore+spadic).Data(),combiId,Int_t(raw->GetEquipmentID()),Int_t(raw->GetSourceAddress()),raw->GetTriggerType());
     
      fTimeBuffer[TString(syscore+spadic)][time][combiId]= new CbmSpadicRawMessage((Int_t)raw->GetEquipmentID(), (Int_t)raw->GetSourceAddress(), raw->GetChannelID(),
										  raw->GetEpochMarker(), raw->GetTime(), raw->GetSuperEpoch(), raw->GetTriggerType(),
										   raw->GetInfoType(), raw->GetStopType(), raw->GetGroupId(), raw->GetBufferOverflowCount(), raw->GetNrSamples(), raw->GetSamples());
    } else {
      fTimeBuffer[TString(syscore+spadic)][time][-1]=raw;
      LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter << " SysId " << SysId << " SpaId " << SpaId << " at time " << time << " with combiId " << combiId << " already in buffer" << FairLogger::endl;
      LOG(ERROR) << "                        " << TString(syscore+spadic) << " at time " << time << " with combiId " << combiId << " row " << rowId << " column " << columnId << FairLogger::endl;
    }
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
      for (Int_t bin = 1; bin < 4; bin++){
	Baseline += raw->GetSamples()[nrSamples-bin];
	fHM->H1(histName.Data())->Fill(raw->GetSamples()[nrSamples-bin]);
      }
      Baseline /= 3.;
   
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
  Int_t mapDigiCounter = 0;
  CbmSpadicRawMessage* raw = NULL;
  Int_t  layerId(0), moduleId(0), sectorId(0), rowId(0), columnId(0), clusterSize(0);
  ULong_t lastClusterTime = 0;
  ULong_t time = 0;
  TString SysSpaID = "";
  std::vector<Int_t> digiIndices;
  for (std::map<TString, std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> > >::iterator SpaSysIt = fTimeBuffer.begin() ; SpaSysIt != fTimeBuffer.end(); SpaSysIt++){
    SysSpaID = SpaSysIt->first;
    for (std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> > ::iterator timeIt = (SpaSysIt->second).begin() ; timeIt != (SpaSysIt->second).end(); timeIt++){
      LOG(DEBUG) <<  "ClusterSize:" << Int_t((timeIt->second).size()) << FairLogger::endl;
      clusterSize = Int_t((timeIt->second).size());
      time = timeIt->first;
      fHM->H1(TString("ClusterSize_" + SysSpaID).Data())->Fill(clusterSize);

      //Delta time between time clusters
      fHM->H1(TString("DeltaTime_Cluster_" + SysSpaID).Data())->Fill(time - lastClusterTime);
      lastClusterTime = time;

      Int_t lastCombiID = -1;
      //digiIndices.clear();
      //printf("\nTime: %lu\n",time);
      for (std::map<Int_t, CbmSpadicRawMessage*> ::iterator combiIt = (timeIt->second).begin(); combiIt != (timeIt->second).end(); combiIt++){
	mapDigiCounter++;
	raw = combiIt->second;
	fHM->H2(TString("StopType_ClusterSize_" + SysSpaID).Data())->Fill(clusterSize,Int_t(raw->GetStopType()));

	if (Int_t(raw->GetStopType()) != 0 && Int_t(raw->GetStopType() != 3))
	  fHM->H2(TString("InfoType_ClusterSize_" + SysSpaID).Data())->Fill(clusterSize,Int_t(raw->GetInfoType()));
	fHM->H2(TString("TriggerType_ClusterSize_" + SysSpaID).Data())->Fill(clusterSize,Int_t(raw->GetTriggerType()));

	fHM->H2(TString("ClusterSize_Message_Length_" + SysSpaID).Data())->Fill(clusterSize,Int_t(raw->GetNrSamples()));
	layerId = GetLayerID(raw);
	moduleId = GetModuleID(raw);
	//printf("%p  time %12lu %12lu %s CI%2i layer %i EI%i SA%i TT%i\n",std::addressof(raw),time,raw->GetFullTime(),(SysSpaID).Data(),combiIt->first,layerId,Int_t(raw->GetEquipmentID()),Int_t(raw->GetSourceAddress()),raw->GetTriggerType());
	fHM->H2(TString("LayerId").Data())->Fill(layerId,SysSpaID,1);
	fHM->H2(TString("ModuleId").Data())->Fill(moduleId,SysSpaID,1);

	sectorId = GetSectorID(raw);
	rowId = GetRowID(raw);
	columnId = GetColumnID(raw);
	// BaseLineCorrection==
	Float_t Baseline = 0.;
	if (raw->GetStopType() == 0){
	  for (Int_t bin = 1; bin < 4; bin++){
	    Baseline += raw->GetSamples()[raw->GetNrSamples()-bin];
	  }
	  Baseline /= 3.;
	} else {
	  // Use average baseline estimated for full message length
	}
	const Int_t nSamples = 32;//raw->GetNrSamples();
	Float_t Samples[nSamples] = {0.};
	for (Int_t iBin = 0; iBin < raw->GetNrSamples(); iBin++){
	  Samples[iBin] = raw->GetSamples()[iBin] - Baseline;
	}
	//=====================
	//printf("la%i mo%i se%i ro%i co%i\n",layerId,moduleId,sectorId,rowId,columnId);
	new ((*fDigis)[fiDigi]) CbmTrdDigi(CbmTrdAddress::GetAddress(layerId,moduleId,sectorId,rowId,columnId),
					   raw->GetFullTime()*57.14,//57,14 ns per timestamp
					   raw->GetTriggerType(), raw->GetInfoType(), raw->GetStopType(),
					   raw->GetNrSamples(), Samples/*&Samples[32]*/);
	digiIndices.push_back(fiDigi);

	if (combiIt != timeIt->second.begin()){
	  fHM->H1(TString("DeltaCh_Cluster_" + SpaSysIt->first).Data())->Fill(combiIt->first - lastCombiID);
	}

	if (combiIt->first - lastCombiID > 1){
	  //printf("\n");
	  CbmTrdCluster* cluster = new ((*fClusters)[fiCluster]) CbmTrdCluster();
	  cluster->SetAddress(CbmTrdAddress::GetAddress(layerId,moduleId,sectorId,rowId,columnId));
	  cluster->SetDigis(digiIndices);
	  digiIndices.clear();
	  fiCluster++;
	}
	
	//printf("%i ",combiIt->first);
	//delete combiIt->second;
	lastCombiID = combiIt->first;
	fiDigi++;
      }
      if (digiIndices.size() > 0){
	//printf("digiIndices %i > 0\n",(Int_t)digiIndices.size());
	CbmTrdCluster* cluster = new ((*fClusters)[fiCluster]) CbmTrdCluster();
	cluster->SetAddress(CbmTrdAddress::GetAddress(layerId,moduleId,sectorId,rowId,columnId));
	cluster->SetDigis(digiIndices);
	digiIndices.clear();
	fiCluster++;
      }
      //timeIt->second.clear();   
    }
    //SpaSysIt->second.clear();
  }
  printf("[INFO   ] CbmTrdRawBeamProfile::Clusterizer Digis:           %i\n",mapDigiCounter);
}

Int_t CbmTrdRawBeamProfile::GetSysCoreID(CbmSpadicRawMessage* raw)
{
  Int_t eqID = raw->GetEquipmentID();
  if (eqID == (Int_t)kMuenster) {  
    return 0;
  }  else if (eqID == (Int_t)kFrankfurt){
    return 1;
  }  else if (eqID == (Int_t)kBucarest){  
    return 2;
  } else
    LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter <<  " EquipmentID " << eqID << "not known." << FairLogger::endl;   
  return -1;
}
Int_t CbmTrdRawBeamProfile::GetSpadicID(CbmSpadicRawMessage* raw)
{
  Int_t sourceA = raw->GetSourceAddress() - SpadicBaseAddress;
  if (sourceA == 0 || sourceA == 1)
    return 0;
  else if (sourceA == 2 || sourceA == 3)
    return 1;
  else  if (sourceA == 4 || sourceA == 5)
    return 2;
  else 
    LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter <<  " Source Address " << sourceA << "not known." << FairLogger::endl;  
  return -1; 
}
Int_t CbmTrdRawBeamProfile::GetModuleID(CbmSpadicRawMessage* raw)
{
  if (raw == NULL){
    LOG(ERROR) << "CbmTrdRawBeamProfile::GetModuleID: CbmSpadicRawMessage == NULL" << FairLogger::endl;
    return -1;
  } else {
    Int_t sys = GetSysCoreID(raw);
    Int_t spa = GetSpadicID(raw);

    return sys * 3 + spa;  
  }
}
  Int_t CbmTrdRawBeamProfile::GetLayerID(CbmSpadicRawMessage* raw)
  {
    if (raw == NULL){
      LOG(ERROR) << "CbmTrdRawBeamProfile::GetModuleID: CbmSpadicRawMessage == NULL" << FairLogger::endl;
      return -1;
    } else {
    Int_t sys = GetSysCoreID(raw);
    Int_t spa = GetSpadicID(raw);

      return sys * 3 + spa; 
    }
  }
  Int_t CbmTrdRawBeamProfile::GetSectorID(CbmSpadicRawMessage* raw)
  {
    if (raw == NULL)
      LOG(ERROR) << "CbmTrdRawBeamProfile::GetSectorID: CbmSpadicRawMessage == NULL" << FairLogger::endl;
    return 0;
  }
  Int_t CbmTrdRawBeamProfile::GetRowID(CbmSpadicRawMessage* raw)
  {
    if (raw == NULL)
      LOG(ERROR) << "CbmTrdRawBeamProfile::GetRowID: CbmSpadicRawMessage == NULL" << FairLogger::endl;
    Int_t chID = raw->GetChannelID();
    Int_t sourceA = raw->GetSourceAddress();
    switch (sourceA) {
    case (SpadicBaseAddress+0):  // first spadic
      break;
    case (SpadicBaseAddress+1):  // first spadic
      chID += 16;
      break;
    case (SpadicBaseAddress+2):  // second spadic
      break;
    case (SpadicBaseAddress+3):  // second spadic
      chID += 16;
      break;
    case (SpadicBaseAddress+4):  // third spadic
      break;
    case (SpadicBaseAddress+5):  // third spadic
      chID += 16;
      break;
    default:
      LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter << " Source Address " << sourceA << "not known." << FairLogger::endl;
      break;
    }   
    Int_t columnId = GetChannelOnPadPlane(chID);
    if (columnId > 16) 
      return 1;
    else
      return 0;
  }
  Int_t CbmTrdRawBeamProfile::GetColumnID(CbmSpadicRawMessage* raw)
  {
    if (raw == NULL)
      LOG(ERROR) << "CbmTrdRawBeamProfile::GetColumnID: CbmSpadicRawMessage == NULL" << FairLogger::endl;
    Int_t chID = raw->GetChannelID();
    Int_t sourceA = raw->GetSourceAddress();
    switch (sourceA) {
    case (SpadicBaseAddress+0):  // first spadic
      break;
    case (SpadicBaseAddress+1):  // first spadic
      chID += 16;
      break;
    case (SpadicBaseAddress+2):  // second spadic
      break;
    case (SpadicBaseAddress+3):  // second spadic
      chID += 16;
      break;
    case (SpadicBaseAddress+4):  // third spadic
      break;
    case (SpadicBaseAddress+5):  // third spadic
      chID += 16;
      break;
    default:
      LOG(ERROR) << "Container " << fContainerCounter << " Message " << fMessageCounter << " Source Address " << sourceA << "not known." << FairLogger::endl;
      break;
    }   
    Int_t columnId = GetChannelOnPadPlane(chID);
    if (columnId >= 16) 
      columnId -= 16;
    return columnId;
  }
  Int_t CbmTrdRawBeamProfile::GetChannelOnPadPlane(Int_t SpadicChannel)
  {
    Int_t channelMapping[32] = {31,15,30,14,29,13,28,12,27,11,26,10,25, 9,24, 8,
				23, 7,22, 6,21, 5,20, 4,19, 3,18, 2,17, 1,16, 0};
    if (SpadicChannel < 0 || SpadicChannel > 31){
      LOG(ERROR) << "CbmTrdRawBeamProfile::GetChannelOnPadPlane ChId " << SpadicChannel << FairLogger::endl;
      return -1;
    } else {
      return channelMapping[SpadicChannel];
    }
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
    gStyle->SetNumberContours(99);
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);
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
    fHM->Add("LayerId", new TH2I("LayerId", "LayerId", 9, -0.5, 8.5, 9, -0.5, 8.5));
    for(Int_t syscore = 0; syscore < 3; ++syscore) {
      for(Int_t spadic = 0; spadic < 3; ++spadic) {
	fHM->H1("LayerId")->GetYaxis()->SetBinLabel(syscore*3+spadic+1,syscoreName[syscore]+"_"+spadicName[spadic]); 
      }
    }
    fHM->Add("ModuleId", new TH2I("ModuleId", "ModuleId", 9, -0.5, 8.5, 9, -0.5, 8.5));
    for(Int_t syscore = 0; syscore < 3; ++syscore) {
      for(Int_t spadic = 0; spadic < 3; ++spadic) {
	fHM->H1("ModuleId")->GetYaxis()->SetBinLabel(syscore*3+spadic+1,syscoreName[syscore]+"_"+spadicName[spadic]); 
      }
    }
    for(Int_t syscore = 0; syscore < 3; ++syscore) {
      for(Int_t spadic = 0; spadic < 3; ++spadic) {
	fHM->H1("TriggerSum")->GetXaxis()->SetBinLabel(3*syscore+spadic+1,TString(syscoreName[syscore]+"_"+spadicName[spadic]));

	TString histName = "CountRate_" + syscoreName[syscore] + "_" + spadicName[spadic];
	TString title = histName + ";Channel;Counts";
	fHM->Add(histName.Data(), new TH1I(histName, title, 32, -0.5, 31.5));

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
	title = histName + ";Message Length (Time-Bins) ;";
	fHM->Add(histName.Data(), new TH2I(histName, title, 33, -0.5, 32.5,6,-0.5,5.5));
	for (Int_t sType=0; sType < 6; sType++)
	  fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(sType+1,stopTypes[sType]); 

	histName = "InfoType_Message_Length_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Message Length (Time-Bins) ;";
	fHM->Add(histName.Data(), new TH2I(histName, title, 33, -0.5, 32.5, 8, -0.5, 7.5));
	for (Int_t iType=0; iType < 8; iType++)
	  fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(iType+1,infoTypes[iType]);

	histName = "TriggerType_Message_Length_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Message Length (Time-Bins) ;";
	fHM->Add(histName.Data(), new TH2I(histName, title, 33, -0.5, 32.5,4,-0.5,3.5));
	for (Int_t tType=0; tType < 4; tType++)
	  fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(tType+1,triggerTypes[tType]);

	histName = "Message_Length_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Message Length (Time-Bins) ;Channel";
	fHM->Add(histName.Data(), new TH2I(histName, title, 33, -0.5, 32.5, 32, -0.5, 31.5));
    
	histName = "TriggerTypes_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "; ;Channel";
	fHM->Add(histName.Data(), new TH2I(histName, title, 4, -0.5, 3.5,32,-0.5,31.5));
	for (Int_t tType=0; tType < 4; tType++)
	  fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(tType+1,triggerTypes[tType]);
   
	histName = "StopTypes_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "; ;Channel";
	fHM->Add(histName.Data(), new TH2I(histName, title, 6, -0.5, 5.5,32,-0.5,31.5));
	for (Int_t sType=0; sType < 6; sType++)
	  fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(sType+1,stopTypes[sType]);    
  
	histName = "InfoTypes_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "; ;Channel";
	fHM->Add(histName.Data(), new TH2I(histName, title, 8, -0.5, 7.5,32,-0.5,31.5));
	for (Int_t iType=0; iType < 8; iType++)
	  fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(iType+1,infoTypes[iType]);

	histName = "Trigger_Stop_Types_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "; ;";
	fHM->Add(histName.Data(), new TH2I(histName, title, 4, -0.5, 3.5, 6, -0.5, 5.5));
	for (Int_t tType=0; tType < 4; tType++)
	  fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(tType+1,triggerTypes[tType]);
	for (Int_t sType=0; sType < 6; sType++)
	  fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(sType+1,stopTypes[sType]);   

	histName = "Trigger_Info_Types_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "; ;";
	fHM->Add(histName.Data(), new TH2I(histName, title, 4, -0.5, 3.5, 8, -0.5, 7.5));
	for (Int_t tType=0; tType < 4; tType++)
	  fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(tType+1,triggerTypes[tType]);
	for (Int_t iType=0; iType < 8; iType++)
	  fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(iType+1,infoTypes[iType]);


	histName = "Stop_Info_Types_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "; ;";
	fHM->Add(histName.Data(), new TH2I(histName, title, 6, -0.5, 5.5, 8, -0.5, 7.5));
	for (Int_t sType=0; sType < 6; sType++)
	  fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(sType+1,stopTypes[sType]);    
	for (Int_t iType=0; iType < 8; iType++)
	  fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(iType+1,infoTypes[iType]);


	histName = "GroupId_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "; ;Counts";
	fHM->Add(histName.Data(), new TH1I(histName, title, 2, -0.5, 1.5));


	histName = "ClusterSize_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Cluster Size [Channel] ;Counts";
	fHM->Add(histName.Data(), new TH1I(histName, title, 10, -0.5, 9.5));

	histName = "ClusterSize_Message_Length_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Cluster Size [Channel] ;Message Length (Time-Bins)";
	fHM->Add(histName.Data(), new TH2I(histName, title, 10, -0.5, 9.5, 33, -0.5, 32.5));


	histName = "StopType_ClusterSize_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Cluster Size [Channel] ; ";
	fHM->Add(histName.Data(), new TH2I(histName, title, 10, -0.5, 9.5, 6, -0.5, 5.5));
	for (Int_t sType=0; sType < 6; sType++)
	  fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(sType+1,stopTypes[sType]);    

	histName = "TriggerType_ClusterSize_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Cluster Size [Channel] ; ";
	fHM->Add(histName.Data(), new TH2I(histName, title, 10, -0.5, 9.5, 4, -0.5, 3.5));
	for (Int_t tType=0; tType < 4; tType++)
	  fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(tType+1,triggerTypes[tType]);

	histName = "InfoType_ClusterSize_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Cluster Size [Channel] ; ";
	fHM->Add(histName.Data(), new TH2I(histName, title, 10, -0.5, 9.5, 8, -0.5, 7.5));
	for (Int_t iType=0; iType < 8; iType++)
	  fHM->H1(histName.Data())->GetYaxis()->SetBinLabel(iType+1,infoTypes[iType]);


	histName = "TriggerCounter_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";TimeSlice;Trigger / TimeSlice";
	fHM->Add(histName.Data(), new TH1I(histName, title, 500, 0, 500));

	histName = "ErrorCounter_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Channel;ADC value in Bin 0";
	fHM->Add(histName.Data(), new TH1I(histName, title, 32, -0.5, 31.5));

	histName = "BaseLine_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Channel;ADC value";
	fHM->Add(histName.Data(), new TH2F(histName, title, 32, -0.5, 31.5, 511, -256, 255));

	histName = "Integrated_ADC_Spectrum_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Channel;Integr. ADC values in Bin [0,31]";
	fHM->Add(histName.Data(), new TH2F(histName, title, 32, -0.5, 31.5, 2*33*256, -33*256, 33*256));

	histName = "Trigger_Heatmap_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Column;Row";
	fHM->Add(histName.Data(), new TH2I(histName, title, 16, -0.5, 15.5, 2, -0.5, 1.5));
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
	  fHM->Add(histName.Data(), new TH2I(histName, title, 32, -0.5, 31.5, 767, -256, 511));
	}
	for(Int_t  channel = 0; channel < 32; channel++) {
	  histName = "Pulse_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_Ch" + channelName[channel];
	  title = histName + ";Time Bin;ADC value";
	  fHM->Add(histName.Data(), new TH2I(histName, title, 32, -0.5, 31.5, 767, -256, 511));
	}
	for(Int_t  channel = 0; channel < 32; channel++) {
	  histName = "maxTimeBin_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_Ch" + channelName[channel];
	  title = histName + ";max Time Bin;count";
	  fHM->Add(histName.Data(), new TH1I(histName, title, 32, -0.5, 31.5));
	}
	for(Int_t  channel = 0; channel < 32; channel++) {
	  histName = "maxADC_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_Ch" + channelName[channel];
	  title = histName + ";max ADC value;count";
	  fHM->Add(histName.Data(), new TH1I(histName, title, 511, -256, 255));
	}
	for(Int_t  channel = 0; channel < 32; channel++) {
	  histName = "maxADC_maxTimeBin_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_Ch" + channelName[channel];
	  title = histName + ";max Time Bin;max ADC value";
	  fHM->Add(histName.Data(), new TH2I(histName, title, 32, -0.5, 31.5, 511, -256, 255));
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
