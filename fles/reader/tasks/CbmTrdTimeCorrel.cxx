#include "CbmTrdTimeCorrel.h"
#include "CbmSpadicRawMessage.h"
#include "CbmHistManager.h"
#include "CbmBeamDefaults.h"
#include "CbmTrdCluster.h"
#include "CbmTrdDigi.h"
#include "TStyle.h"
#include "FairLogger.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TMultiGraph.h"
#include "TNtuple.h"
#include "TString.h"
#include "TMath.h"
#include <cmath>
#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
// ----              -------------------------------------------------------
CbmTrdTimeCorrel::CbmTrdTimeCorrel()
  : FairTask("CbmTrdTimeCorrel"),
    fRawSpadic(NULL),
    fHM(new CbmHistManager()),
    fBaselineHM(new CbmHistManager()),
    fNrTimeSlices(0),
    fRun(0),
    fRewriteSpadicName(true),
    fSpadics(0),
    fMessageBuffer(),
    fRawMessages(NULL),
    fDigis(NULL),
    fClusters(NULL),
    fiDigi(0),
    fiCluster(0),
    fiRawMessage(0),
    timestampOffsets(),
    fLastMessageTime{{{0}}},
    fEpochMarkerArray{{0}},
    fFirstEpochMarker{{true}},
    fGraph(true),
    EpochRegressCounter(),
    EpochRegressOffset(),
    EpochRegressTriggered(),
    fLinearHitBuffer(),
    fClusterBuffer(),
    fOutputCloneArrays(false)  
{
   for (Int_t i=0; i < 3; ++i) { 
     for (Int_t j=0; j < 6; ++j) {
        fFirstEpochMarker[i][j] = true;
     }
   }
   gStyle->SetNumberContours(99);
    {
      TString CalibrationFilename =
          FairRootManager::Instance ()->GetOutFile ()->GetName ();
      Int_t DefaultBaselineFrankfurt = -190;
      Int_t DefaultBaselineMuenster = -220;
      CalibrationFilename.ReplaceAll (".root", "_Calibration.root");
      TFile* Calibration = new TFile (CalibrationFilename.Data (), "READ");
      if (Calibration == nullptr)
        {
          LOG(INFO) << "No calibration file found, continuing with default baseline."
                       << FairLogger::endl;
          for (Int_t syscore = 0; syscore < 1; ++syscore)
            {
              for (Int_t spadic = 0; spadic < 4; ++spadic)
                {
                  for (Int_t channel = 0; channel < 16; ++channel)
                    {
                      fBaseline[syscore * 64 + spadic * 16 + channel] =
                          ((spadic / 2) == 0 ?
                              DefaultBaselineFrankfurt : DefaultBaselineMuenster);
                    }
                }
            }
        }
      else{ Calibration->cd();
          LOG(INFO) << "Calibration file found at " <<CalibrationFilename<< " reading baseline."
                       << FairLogger::endl;
          for (Int_t syscore = 0; syscore < 1; ++syscore)
            {
              for (Int_t spadic = 0; spadic < 4; ++spadic)
                {
                  for (Int_t channel = 0; channel < 16; ++channel)
                    {
                      TString fitname = "Fit_Baseline_for_Syscore_"
                          + std::to_string (syscore) + "_Spadic_"
                          + std::to_string (spadic)
                          + "_Channel_" + std::to_string (channel);
                      TFitResult* CurrentFit = dynamic_cast<TFitResult*>(Calibration->Get(fitname.Data()));
                      if(CurrentFit==nullptr){
                          LOG(INFO) << "No baseline fit found for Spadic "<<spadic << "Channel"<<channel
                                       << FairLogger::endl;
                          fBaseline[syscore * 64 + spadic * 16 + channel] =
                                                    ((spadic / 2) == 0 ?
                                                        DefaultBaselineFrankfurt : DefaultBaselineMuenster);
                      }
                      else {
                          fBaseline[syscore * 64 + spadic * 16 + channel] = CurrentFit->GetParams()[1];
                      }
                    }
                }
            }
          FairRootManager::Instance ()->GetOutFile ()->cd();
          Calibration->Close();
        }
    }
 };

// ----              -------------------------------------------------------
CbmTrdTimeCorrel::~CbmTrdTimeCorrel()
{
  LOG(DEBUG) << "Destructor of CbmTrdTimeCorrel" << FairLogger::endl;
  fRawMessages->Clear();
  fDigis->Delete();
  fClusters->Delete();
}
// ----              -------------------------------------------------------
void CbmTrdTimeCorrel::SetParContainers()
{
 LOG(DEBUG) << "SetParContainers of CbmTrdTimeCorrel" << FairLogger::endl;
}
// ----              -------------------------------------------------------
InitStatus CbmTrdTimeCorrel::Init()
{
  LOG(INFO) << "Initilization of CbmTrdTimeCorrel" << FairLogger::endl;
  FairRootManager* ioman = FairRootManager::Instance();
  fRawSpadic = static_cast<TClonesArray*>(ioman->GetObject("SpadicRawMessage"));
  if ( !fRawSpadic ) {
    LOG(FATAL) << "No InputDataLevelName array!\n CbmTrdTimeCorrel will be inactive" << FairLogger::endl;
    return kERROR;
  }

  fRawMessages = new TClonesArray("CbmSpadicRawMessage");
  ioman->Register("TrdRawMessage", "TRD Raw Messages", fRawMessages, kTRUE);

  fDigis = new TClonesArray("CbmTrdDigi");
  ioman->Register("TrdDigi", "TRD Digis", fDigis, kTRUE);

  fClusters = new TClonesArray("CbmTrdCluster");
  ioman->Register("TrdCluster", "TRD Clusters", fClusters, kTRUE);

  CreateHistograms();
  return kSUCCESS;
}
// ---- ReInit  -------------------------------------------------------
InitStatus CbmTrdTimeCorrel::ReInit()
{
  LOG(DEBUG) << "Initilization of CbmTrdTimeCorrel" << FairLogger::endl;
  return kSUCCESS;
}
// ---- Exec  -------------------------------------------------------
void CbmTrdTimeCorrel::Exec(Option_t* option)
{
  const Int_t maxNrColumns = 16; //max number of channels on a pad plane per asic and row

  // Analysis based on single SPADIC data streams can be done here!!!

  if(fNrTimeSlices==0){
    if(fHM->G1("TsCounter")->GetN()!=0){
      LOG(INFO ) << "Expected empty TsCounter before first TimeSlice, but found " << fHM->G1("TsCounter")->GetN() << " entries." << FairLogger::endl;
    }
  }
  
  TString spadicName = "";
  TString histName="";
  TString title="";
  
  std::map<TString, std::map<ULong_t, std::vector<CbmSpadicRawMessage*> > > timeBuffer;


  LOG(INFO) << "CbmTrdTimeCorrel: Number of current TimeSlice: " << fNrTimeSlices << FairLogger::endl;
  LOG(INFO) << "Digis in TClonesArray:                           " << fDigis->GetEntriesFast() << FairLogger::endl;
  LOG(INFO) << "Clusters in TClonesArray:                        " << fClusters->GetEntriesFast() << FairLogger::endl;
  LOG(INFO) << "rawMessages in TS:                               " << fRawSpadic->GetEntriesFast() << FairLogger::endl;
  fDigis->Delete();
  fDigis = new TClonesArray("CbmTrdDigi");
  fClusters->Delete();
  fClusters = new TClonesArray("CbmTrdCluster");
  Int_t nSpadicMessages = fRawSpadic->GetEntriesFast(); //SPADIC messages per TimeSlice
  Int_t nSpadicMessages0(0),nSpadicMessages1(0); //SPADIC messages per TimeSlice for single SPADICS
  Int_t nSpadicMessagesHit0(0), nSpadicMessagesHit1(0), nSpadicMessagesHitAborted0(0), nSpadicMessagesHitAborted1(0), nSpadicMessagesOverflow0(0), nSpadicMessagesOverflow1(0), nSpadicMessagesInfo0(0), nSpadicMessagesEpoch0(0), nSpadicMessagesEpoch1(0), nSpadicMessagesInfo1(0), nSpadicMessagesLost0(0), nSpadicMessagesLost1(0), nSpadicMessagesStrange0(0), nSpadicMessagesStrange1(0); //SPADIC message types per TimeSlice for single SPADICS
  Int_t lostMessages(0);// this variable should be reset to 0 for every SPADIC message
  // Getting message type bools from Spadic raw message
  Bool_t isHit = false;
  Bool_t isHitAborted = false;
  Bool_t isOverflow = false;
  Bool_t isInfo = false;
  Bool_t isEpoch = false;
  Bool_t isEpochOutOfSynch = false;
  Bool_t isStrange = false;
  
  // Initialise message coordinates to -1 to recognise unset variables
  Int_t eqID(-1), sourceA(-1), triggerType(-1), stopType(-1), infoType(-1), groupId(-1), sysID(-1), spaID(-1);
  // chID in ASIC. Take care, neighboured numbers are not neccessarily neighboured on the connected TRD cathode pad plane. Resorted lateron!
  // padID are sorted chIDs in the order as on the pad plane
  Int_t chID(-1), padID(-1), columnID(-1), rowID(-1), combiID(-1);
  ULong_t time = 0;
  // Time stamp and epoch are counted in the Spadic
  Int_t timeStamp(0), epoch(0), superEpoch(0);
  LOG(INFO) << "nSpadicMessages: " << nSpadicMessages << FairLogger::endl;

  Int_t hitCounter[3][6]={{0}};
  CbmSpadicRawMessage* raw = NULL;
  //Clear Offset Map
  LOG(DEBUG)<< "Size of timestampOffsets: "<<timestampOffsets.size() << FairLogger::endl;
  timestampOffsets.clear();

  //Calculate Timestamp Offsets
  if (fActivateOffsetAnalysis) {//Context to limit epochBuffers scope
    LOG(INFO) <<"Begin Buffering Epoch Messages" << FairLogger::endl;
    EpochMap epochBuffer;
    //Loop over all epoch messages to build Fulltime offsets.
    for (Int_t iSpadicMessage=0; iSpadicMessage < nSpadicMessages; ++iSpadicMessage){
      raw = static_cast<CbmSpadicRawMessage*>(fRawSpadic->At(iSpadicMessage));
      lostMessages = 0; // reset lost-counter for a new message
      isEpoch = raw->GetEpoch();
      isEpochOutOfSynch = raw->GetEpochOutOfSynch();
      if(isEpoch||isEpochOutOfSynch){
	sourceA = raw->GetSourceAddress();
	spaID = GetSpadicID(sourceA);
	time = raw->GetFullTime();
	//buffer all Epoch Messages
	epochBuffer[spaID][time] = raw;
	Int_t tempSize= fHM->G1(("Timestamps_Spadic"+std::to_string(spaID)))->GetN();
	if (fGraph) fHM->G1(("Timestamps_Spadic"+std::to_string(spaID)))->SetPoint(tempSize,fNrTimeSlices,time);
      }
    }
    if(fNrTimeSlices!=0) timestampOffsets = CalculateTimestampOffsets(epochBuffer);
    LOG(INFO) <<"Finish Buffering Epoch Messages" << FairLogger::endl;
  }

  //Fill Histograms with the Offsets
#ifndef __CINT__
  try{
    if(fNrTimeSlices!=0)
      if(timestampOffsets.size()!=0) //If there are no Epoch Messages, skip loop
	for (auto baseSpaIt = timestampOffsets.begin() ; baseSpaIt!= timestampOffsets.end() ; ++baseSpaIt) //Loop over all Base Spadics
	  if(baseSpaIt->second.size()!=0) //If this Base Spadic is missing, skip to next Base Spadic
	    for (auto compSpaIt = baseSpaIt->second.begin(); compSpaIt != baseSpaIt->second.end(); ++compSpaIt) //For every Base Spadic, loop over all Spadics
	      {
		int baseSpaID = baseSpaIt->first;
		int compSpaID = compSpaIt->first;
		const Int_t SysID =0;
		auto FullTimeIt = timestampOffsets.at(baseSpaID).at(compSpaID).begin();
		for (; FullTimeIt != timestampOffsets.at(baseSpaID).at(compSpaID).end(); ++FullTimeIt) //Loop over all timestamps and Fill Fulltime Offsets for Epoch Messages into Histograms
		  {
		    Int_t tGraphSize = fHM->G1(("Time_Offset_between_Spadic_"+std::to_string(baseSpaID)+"_and_Spadic_"+std::to_string(compSpaID)))->GetN();
		    	if (fGraph) fHM->G1(("Time_Offset_between_Spadic_"+std::to_string(baseSpaID)+"_and_Spadic_"+std::to_string(compSpaID)))->SetPoint(tGraphSize,fNrTimeSlices,FullTimeIt->second);
		  }
	      }
  }
  catch(std::out_of_range)
    {
      LOG(ERROR)<< "map::at() has thrown an exception " << FairLogger::endl;
    }
#endif //__CINT__

  // Do the message type counting per timeslice here to make the full numbers available early in the following analysis loop
  for (Int_t iSpadicMessage=0; iSpadicMessage < nSpadicMessages; ++iSpadicMessage){
    raw = static_cast<CbmSpadicRawMessage*>(fRawSpadic->At(iSpadicMessage));
    lostMessages = 0; // reset lost-counter for a new message
    sourceA = raw->GetSourceAddress();
    spaID = GetSpadicID(sourceA);
    eqID = raw->GetEquipmentID();
    spadicName = GetSpadicName(eqID,sourceA);
    isHit = raw->GetHit();
    isHitAborted = raw->GetHitAborted();
    isOverflow = raw->GetOverFlow();
    isInfo = raw->GetInfo();
    isEpoch = raw->GetEpoch();
    isEpochOutOfSynch = raw->GetEpochOutOfSynch();
    isStrange = raw->GetStrange();
    // Count total messages per ASIC and message-types per ASIC.
    if(spadicName == RewriteSpadicName("SysCore0_Spadic0")) {
      nSpadicMessages0++;
      if(isHit) nSpadicMessagesHit0++;
      else if(isHitAborted) nSpadicMessagesHitAborted0++;
      else if(isOverflow) {
	nSpadicMessagesOverflow0++;
	if (lostMessages > 0) nSpadicMessagesLost0 += lostMessages; //lostMessages might be -1 for hits or epochs, therefore one has to ensure that it is > 0
      }
      else if(isInfo) nSpadicMessagesInfo0++;
      else if(isEpoch) nSpadicMessagesEpoch0++;
      else if(isStrange) nSpadicMessagesStrange0++;
    }
    
    else if(spadicName == RewriteSpadicName("SysCore0_Spadic1")) {
      nSpadicMessages1++;
      if(isHit) nSpadicMessagesHit1++;
      else if(isHitAborted) nSpadicMessagesHitAborted1++;
      else if(isOverflow) {
	nSpadicMessagesOverflow1++;
	if (lostMessages > 0) nSpadicMessagesLost1 += lostMessages; //lostMessages might be -1 for hits or epochs, therefore one has to ensure that it is > 0
      }
      else if(isInfo) nSpadicMessagesInfo1++;
      else if(isEpoch) nSpadicMessagesEpoch1++;
      else if(isStrange) nSpadicMessagesStrange1++;
    }
    // Currently only expecting Spadic0 and Spadic1. Logging others, if appearing.
    else {
      LOG(INFO) << "SpadicMessage " << iSpadicMessage << " claims to be from " << spadicName << " with spadicID " << spaID << FairLogger::endl;
    }
  }

  // Starting to loop over all Spadic messages in unpacked TimeSlice, Analysis loop
  for (Int_t iSpadicMessage=0; iSpadicMessage < nSpadicMessages; ++iSpadicMessage) {
    //std::cout << "  " << iSpadicMessage << std::endl;
    raw = static_cast<CbmSpadicRawMessage*>(fRawSpadic->At(iSpadicMessage));
    lostMessages = 0; // reset lost-counter for a new message
    chID = -1; // reset to notice if set to a new ID
    padID = -1;
    isHit = raw->GetHit();
    isHitAborted = raw->GetHitAborted();
    isOverflow = raw->GetOverFlow();
    isInfo = raw->GetInfo();
    isEpoch = raw->GetEpoch();
    isStrange = raw->GetStrange();
    isEpochOutOfSynch = raw->GetEpochOutOfSynch();
    // Seriously guys, a message can only be of one type.
    if(Int_t(isHit+isInfo+isEpoch+isHitAborted+isOverflow+isStrange+isEpochOutOfSynch)!=1) LOG(ERROR) << "SpadicMessage " << iSpadicMessage << " is classified from CbmSpadicRawMessage to be: HIT " << (Int_t)isHit << " / INFO " << (Int_t)isInfo << " / EPOCH " << (Int_t)isEpoch << " / HITaborted " << (Int_t)isHitAborted << " / OVERFLOW " << (Int_t)isOverflow << " / STRANGE " << (Int_t)isStrange << FairLogger::endl;

    // Get SysCore & Spadic propertys
    eqID = raw->GetEquipmentID();
    sourceA = raw->GetSourceAddress();

    groupId=raw->GetGroupId();
    chID = raw->GetChannelID();
    spaID = GetSpadicID(sourceA);
    if(chID > -1 && chID < 16 && spaID%2==1) chID+=16; // eqID ?
    padID = GetChannelOnPadPlane(chID);// Remapping from ASIC to pad-plane channel numbers.

    columnID = GetColumnID(raw);
    rowID = GetRowID(raw);
    combiID = rowID * (maxNrColumns + 1) + columnID;// Is needed to cluster messages within one detector layer. combiID provides a linear representation of  a 2dim coordinate system (row, column) with an additional column. By using continues combiIDs one avoids to build continues clusters between to rows.


    Int_t nrSamples=raw->GetNrSamples();

    lostMessages = raw->GetBufferOverflowCount();


    if (!isOverflow && lostMessages!=0) LOG(ERROR) << "SpadicMessage " << iSpadicMessage << " is HIT " << (Int_t)isHit << " / INFO " << (Int_t)isInfo << " / EPOCH " << (Int_t)isEpoch << " / HITaborted " << (Int_t)isHitAborted << " / STRANGE " << (Int_t)isStrange << " but claims to have lost " << lostMessages << " messages" << FairLogger::endl;
    time = raw->GetFullTime();
    timeStamp = raw->GetTime();
    
    epoch = raw->GetEpochMarker();// is copied to each SpadicRawMessage by the unpacker not only epoch messages
    superEpoch = raw->GetSuperEpoch();// is copied to each SpadicRawMessage by the unpacker not only epoch message

    stopType = raw->GetStopType();
    TString stopName = GetStopName(stopType);
    infoType=raw->GetInfoType();
    triggerType = raw->GetTriggerType();

    if(isHit) hitCounter[sysID][spaID]++;

    if (raw->GetChannelID()>100) LOG(ERROR) << "SpadicMessage with strange chID: " << iSpadicMessage << " sourceA: " << sourceA << " chID: " << raw->GetChannelID() << " groupID: " << groupId << " spaID: " << spaID << " stopType: " << stopType << " infoType: " << infoType << " triggerType: " << triggerType << " isHit: " << isHit << " isInfo: " << isInfo << " isEpoch: " << isEpoch << " Lost Messages: " << lostMessages << FairLogger::endl;
    if (raw->GetChannelID()>32 && !isInfo && !isStrange) LOG(FATAL) << "SpadicMessage: " << iSpadicMessage << " sourceA: " << sourceA << " chID: " << raw->GetChannelID() << " groupID: " << groupId << " spaID: " << spaID << " stopType: " << stopType << " infoType: " << infoType << " triggerType: " << triggerType << " isHit: " << isHit << " isInfo: " << isInfo << " isEpoch: " << isEpoch << " Lost Messages: " << lostMessages << FairLogger::endl;


    if (infoType > 6) {
      LOG(ERROR) << " InfoType " << infoType << "is larger 6, set to 7!" << FairLogger::endl;
      infoType = 7;
    }
    
    spadicName = GetSpadicName(eqID,sourceA);
    //printf("spadicName:%s:\n",spadicName.Data());
    if ( isOverflow )
      if (spadicName != "")
	fHM->H2((spadicName + TString("_LostCount_vs_PadID")).Data())->Fill(lostMessages,padID);

    // add raw message to map sorted by timestamps, syscore and spadic
    if(spadicName == ""){
      LOG(ERROR) << "eqID:" << eqID << " sourceA:" << sourceA << " spadicName:" << spadicName << FairLogger::endl;
      //continue;
    } else {
      //if (false)
  
      if (!isStrange && !isEpoch && !isEpochOutOfSynch){

	timeBuffer[TString(spadicName)][time].push_back(raw);
	if (!isInfo) {
	  std::map<Int_t, CbmSpadicRawMessage*>::iterator MessageBufferIt = fMessageBuffer[TString(spadicName)][time].find(combiID);
	  if (MessageBufferIt == fMessageBuffer[TString(spadicName)][time].end()){ 
	    /*
	      if there is no message found for spadicName at this time and the same padID (which should never be the case) the 
	      processed message is added to the map. This avoids per definition the use of overlapping microslices. It is up to you to 
	      define the time for spacial and time clusterization (after each timeSliceContainer or ad the end of the file. This will 
	      be mainly a question of avainlable RAM. It might be a good idea to clusterize after each timeSliceContainer, write all 
	      rawMessages inside of the buffer to a TClonesArray or TTree as well as all found CbmTrdClusters to a separate TClonesArray 
	      or TTree. Afterwards one should erase the buffer partially (leaving a rest of messages at the end of the buffer to be able 
	      to cluster messages at the beginning of the next TimeSliceContainer.
	    */
	    //fMessageBuffer[TString(spadicName)][time][combiID] = raw;
	    /*
	      In order to not interfere with the clean up of the TClonesArray, we copy here the information and delete the stored objects in the clean up of the map
	    */
	    fMessageBuffer[TString(spadicName)][time][combiID] = new CbmSpadicRawMessage(raw->GetEquipmentID(), raw->GetSourceAddress(), raw->GetChannelID(),
											 raw->GetEpochMarker(), raw->GetTime(), 
											 raw->GetSuperEpoch(), raw->GetTriggerType(),
											 raw->GetInfoType(), raw->GetStopType(), 
											 raw->GetGroupId(), raw->GetBufferOverflowCount(), 
											 raw->GetNrSamples(), raw->GetSamples(),
											 raw->GetHit(), raw->GetInfo(), raw->GetEpoch(), raw->GetEpochOutOfSynch(), raw->GetHitAborted(), raw->GetOverFlow(), raw->GetStrange());

	    /*
	      A new TClonesArray without MS overlaps is created for offline analysis
	    *//*
		new ((*fRawMessages)[fiRawMessage])CbmSpadicRawMessage(raw->GetEquipmentID(), raw->GetSourceAddress(), raw->GetChannelID(),
		raw->GetEpochMarker(), raw->GetTime(), 
		raw->GetSuperEpoch(), raw->GetTriggerType(),
		raw->GetInfoType(), raw->GetStopType(), 
		raw->GetGroupId(), raw->GetBufferOverflowCount(), 
		raw->GetNrSamples(), raw->GetSamples(),
		raw->GetHit(), raw->GetInfo(), raw->GetEpoch(), raw->GetEpochOutOfSynch(), raw->GetHitAborted(), raw->GetOverFlow(), raw->GetStrange());
	      */
	    fiRawMessage++;
	    if (isHit&&(triggerType==1||triggerType==3))fHM->H2("Map_of_Hits_for_Syscore_"+std::to_string(0)+"_Spadic_"+std::to_string(static_cast<Int_t>(spaID/2)))->Fill(padID%16,static_cast<Int_t>(padID/16));
	    if (isHit&&(triggerType==1||triggerType==3))fHM->H2("Map_of_Charge_for_Syscore_"+std::to_string(0)+"_Spadic_"+std::to_string(static_cast<Int_t>(spaID/2)))->Fill(padID%16,static_cast<Int_t>(padID/16),GetMaxADC(*raw));
	    if (isHit) FillBaselineHistogram(raw);
	    if(fActivateClusterizer)
	      if (isHit || isHitAborted || isOverflow)
	        if(fNrTimeSlices!=0){
	          CbmSpadicRawMessage* tempPtr = new CbmSpadicRawMessage;
	          *tempPtr = *raw;
	          fLinearHitBuffer.push_back(tempPtr);
	        }
	    if(fDrawSignalDebugHistograms){
	    	if (isHit||isHitAborted){
	    		string detectorName = (GetSpadicID(raw->GetSourceAddress())/2 == 0 ? "Frankfurt" : "Muenster");
				Int_t maxADC= GetMaxADC(*raw,false);
				Int_t TriggerType = raw->GetTriggerType();
				string TriggerName;
				{
					TString triggerTypes[4] = {
							"Global trigger", "Self triggered",
							"Neighbor triggered",
							"Self and neighbor triggered" };
					TriggerName = triggerTypes[TriggerType];
				}
				Int_t StopType = raw->GetStopType();
				Int_t AvgBaseline=GetAvgBaseline(*raw);
				string Histname = "MaxADC_vs_NrSamples_for_Syscore_"
						+std::to_string(0)+"_Prototype_from_"+detectorName+TriggerName;
				string Hittype;
				fHM->H2(Histname)->Fill(raw->GetNrSamples(),maxADC);
				if (StopType==0) {
					Hittype = "_Hits";
				}else {
					Hittype = "_MultiHits";
				}
				Histname = "MaxADC_vs_avgBaseline_for_Syscore_"
						+std::to_string(0)+"_Prototype_from_"+detectorName+Hittype+TriggerName;
				fHM->H2(Histname)->Fill(AvgBaseline,maxADC);
				Histname = "avgBaseline_for_Syscore_"+std::to_string(0)
						+"_Prototype_from_"+detectorName+TriggerName;
				if(raw->GetStopType()==0)fHM->H1(Histname)->Fill(AvgBaseline);
				Histname = "avgBaseline_vs_NrSamples_for_Syscore_"
						+std::to_string(0)+"_Prototype_from_"+detectorName+TriggerName;
				fHM->H2(Histname)->Fill(raw->GetNrSamples(),AvgBaseline);
				static Int_t DetectorBeaselinesIndex[2]={0,0};
				if (raw->GetStopType() == 0) {
					if (maxADC + AvgBaseline > 50){
						DetectorBeaselinesIndex[GetSpadicID(raw->GetSourceAddress())/2]++;
						if(DetectorBeaselinesIndex[GetSpadicID(raw->GetSourceAddress())/2]%13==0 && DetectorBeaselinesIndex[GetSpadicID(raw->GetSourceAddress())/2]<1300){
							histName = "High_Baseline_SignalShape_for_Syscore_"+std::to_string(0)
									+"_Prototype_from_"+detectorName+"_Nr_"
									+std::to_string(DetectorBeaselinesIndex[GetSpadicID(raw->GetSourceAddress())/2])
									+"_Trigger_"+std::to_string(raw->GetTriggerType());
							TString title2 = histName + " ";
							fBaselineHM->Create2<TH2I>(histName.Data(), title2.Data(),33,-0.5,32.5,512,-256.5,255.5);
							fBaselineHM->H2(histName.Data())->GetXaxis()->SetNameTitle("Sample","Sample");
							fBaselineHM->H2(histName.Data())->GetYaxis()->SetNameTitle("ADC Value","ADC Value");
							FillSignalShape(*raw,histName.Data(),true);
						}
					}
				}
	    	}
	    }

	  } else {  
	    
	    LOG(DEBUG) << "Found Message already in fMessageBuffer at :" << TString(spadicName).Data() << ":, time:" << time << ", padID:" << padID << ". Potential overlapping MS container!" << FairLogger::endl;

	    if (fMessageBuffer[TString(spadicName)][time][combiID] != NULL)
	      fHM->H2("DoubleMessage_MessageType")->Fill(GetMessageType(fMessageBuffer[TString(spadicName)][time][combiID]),GetMessageType(raw));
	    else 	  
	      fHM->H2("DoubleMessage_MessageType")->Fill(-1,GetMessageType(raw));
	    /*
	      LOG(INFO) << "------------------------------------" << FairLogger::endl;
	      raw->PrintMessage();
	      LOG(INFO) << "<raw---------------------------->" << FairLogger::endl;
	      fMessageBuffer[TString(spadicName)][time][combiID]->PrintMessage();
	      LOG(INFO) << ">buffer-------------------------<" << FairLogger::endl;
	      LOG(INFO) << "------------------------------------" << FairLogger::endl;
	    */
	  }
	}
      }
      if (isInfo){
	if(chID < 32)fHM->H2("InfoType_vs_Channel")->Fill(padID,infoType);
	else fHM->H2("InfoType_vs_Channel")->Fill(33,infoType); // chIDs greater than 32 are quite strange and will be put into the last bin
      }
      if (isEpoch){   // fill epoch messages in an additional row
	if(chID < 32)fHM->H2("InfoType_vs_Channel")->Fill(padID,9);
	else fHM->H2("InfoType_vs_Channel")->Fill(33,9);
      }
      if (isOverflow){   // fill overflow messages in an additional row
	if(chID < 32)fHM->H2("InfoType_vs_Channel")->Fill(padID,10);
	else fHM->H2("InfoType_vs_Channel")->Fill(33,10);
      }
      if (isEpoch) {
	// sysID is hardcoded here to 0. To be changed as soon as sysID is set correctly.
	if (!fFirstEpochMarker[0][spaID]) {
	  fHM->H1("Delta_Epoch_hist_for_Syscore_"+std::to_string(0)+"_Spadic_"+std::to_string(spaID/2)+"_Half_"+std::to_string((Int_t)(chID/16)))->Fill(epoch - fEpochMarkerArray[0][spaID]);
	  fHM->H2("Hitrate_vs_DeltaEpoch_hist_for_Syscore_"+std::to_string(0)+"_Spadic_"+std::to_string(spaID/2)+"_Half_"+std::to_string((Int_t)(chID/16)))->Fill(epoch - fEpochMarkerArray[0][spaID], nSpadicMessagesHit0+nSpadicMessagesHit1);
	  // Book keep here the trending of Epoch Counters after a regress has been detected
	  if ( EpochRegressTriggered[0][spaID] ) { // Fill subsequent epoch messages in the histo, if the mechanism is triggered
	    EpochRegressCounter[0][spaID]++; // Count how many epochs are put in the trending histo already
	    LOG(INFO) << "EPOCH REGRESS ongoing (step " << EpochRegressCounter[0][spaID] << ") for spaID " << spaID << " from " << fEpochMarkerArray[0][spaID] << " to " << epoch << " (timeslice " << fNrTimeSlices << ", spadicmessage " << iSpadicMessage << ")" << FairLogger::endl;
	    fHM->H2("EpochTrend_postregress_hist_for_Syscore_"+std::to_string(0)+"_Spadic_"+std::to_string(spaID/2)+"_Half_"+std::to_string((Int_t)(chID/16)))->Fill(EpochRegressCounter[0][spaID],epoch - EpochRegressOffset[0][spaID]);
	    if ( EpochRegressCounter[0][spaID] >= 150 ) { // After this condition is fulfilled, enough epochs have been recorded -- reset trigger and counter
	      EpochRegressTriggered[0][spaID] = false;
	      EpochRegressCounter[0][spaID] = 0;
	    }
	  }
	  if (epoch - fEpochMarkerArray[0][spaID] <= 0  && !EpochRegressTriggered[0][spaID]) { // Trigger the recording if a regress is detected and the recording is not running up to now. The natural overflow of epoch (regress of 4095) is excluded
	    fHM->G1("DeltaEpoch_vs_timeslice_for_Syscore_"+std::to_string(0)+"_Spadic_"+std::to_string(spaID/2)+"_Half_"+std::to_string(chID/16))->SetPoint(fHM->G1("DeltaEpoch_vs_timeslice_for_Syscore_"+std::to_string(0)+"_Spadic_"+std::to_string(spaID/2)+"_Half_"+std::to_string(chID/16))->GetN(),fNrTimeSlices,epoch - fEpochMarkerArray[0][spaID]); // Fill TGraph with every regress of Epoch, also the normal overflows by 4095
	    if (epoch - fEpochMarkerArray[0][spaID] > -4000) { // but trigger the recording only for irregular regress
	      LOG(INFO) << "EPOCH REGRESS triggered for spaID " << spaID << " from " << fEpochMarkerArray[0][spaID] << " to " << epoch << " (timeslice " << fNrTimeSlices << ", spadicmessage " << iSpadicMessage << ")" << FairLogger::endl;
	      EpochRegressTriggered[0][spaID] = true;
	      EpochRegressOffset[0][spaID] = epoch;
	    }
	  }
	}
     	fEpochMarkerArray[0][spaID] = epoch;
	fFirstEpochMarker[0][spaID] = false;
      }
      if (0 <= chID && chID < 32 && isEpoch){
	if(static_cast<Long_t>(time)-static_cast<Long_t>(fLastMessageTime[0][spaID][chID])<-1000)  LOG(INFO) << "SpadicMessage (isEpoch): " << iSpadicMessage << " has negative Delta Fulltime. sourceA: " << sourceA << " chID: " << raw->GetChannelID() << " groupID: " << groupId << " spaID: " << spaID << " stopType: " << stopType << " infoType: " << infoType << " triggerType: " << triggerType << " isHit: " << isHit << " isInfo: " << isInfo << " isEpoch: " << isEpoch << " Lost Messages: " << lostMessages << FairLogger::endl;
	//Compute Time Deltas, write them into a histogram and store timestamps in fLastMessageTime.
	// WORKAROUND: at Present SyscoreID is not extracted, therefore all Messages are stored as if coming from SysCore 0.
	// Epoch messages are sent with chID 0 or 16, i.e. rawChID 0 from the first or the second half-Spadic. Thus, the half-chip results as 0 or 1 by dividing the chID/16 and casting this to Int_t. Bit bloody but fast.
	if(fActivateDeltaTAnalysis)
	if(spaID != -1)fHM->H1("Delta_t_hist_for_Syscore_"+std::to_string(0)+"_Spadic_"+std::to_string(spaID/2)+"_Half_"+std::to_string((Int_t)(chID/16)))->Fill(static_cast<Long_t>(epoch)-static_cast<Long_t>(fLastMessageTime[0][spaID][chID]));
	//Write delta_t into a TGraph
	if(fActivateDeltaTAnalysis)
	if(spaID!=-1){
	  Int_t tGraphSize = fHM->G1("Delta_t_for_Syscore_"+ std::to_string(0) +"_Spadic_"+std::to_string(spaID/2)+"_Channel_"+std::to_string((Int_t)(chID+(15*spaID%2))))->GetN();
	  	if (fGraph) fHM->G1("Delta_t_for_Syscore_"+ std::to_string(0) +"_Spadic_"+std::to_string(spaID/2)+"_Channel_"+std::to_string((Int_t)(chID/16+(15*spaID%2))))->SetPoint(tGraphSize,time,(static_cast<Long_t>(epoch)-static_cast<Long_t>(fLastMessageTime[0][spaID][chID])));
	}
	fLastMessageTime[0][spaID][chID] = epoch;
      }
      //if(spadicName!="") {

      //
      //  DEBUG PLOTS
      //
      
      if(isHit && chID >= 0 && chID < 32) {
        histName = spadicName + "_Half_" + std::to_string((Int_t)(chID/16)) + "_Time_vs_TimeSlice";
        fHM->H2(histName.Data())->Fill(fNrTimeSlices,time); //timeStamp(0), epoch(0), superEpoch(0);
      }

      if(isEpoch && chID >= 0 && chID < 32) {
        histName = spadicName + "_Half_" + std::to_string((Int_t)(chID/16)) + "_Epoch_vs_TimeSlice";
        fHM->H2(histName.Data())->Fill(fNrTimeSlices,epoch); //timeStamp(0), epoch(0), superEpoch(0);
      }
      
      // These are pure debuging histos to ensure that the unpacker is running without errors (C.B)
      fHM->H2("TriggerType_vs_InfoType")->Fill(raw->GetTriggerType(), raw->GetInfoType());
      fHM->H2("TriggerType_vs_StopType")->Fill(raw->GetTriggerType(), raw->GetStopType());
      fHM->H2("InfoType_vs_StopType")->Fill(raw->GetInfoType(), raw->GetStopType());
      fHM->H2("NrSamples_vs_StopType")->Fill(raw->GetNrSamples(), raw->GetStopType());
      fHM->H2("NrSamples_vs_InfoType")->Fill(raw->GetNrSamples(), raw->GetInfoType());
      fHM->H2("NrSamples_vs_TriggerType")->Fill(raw->GetNrSamples(), raw->GetTriggerType());
      //-------------------
      
      /*Extended Message Debugging:
	-Check for deformed Messages
	-Check for empty Messages claiming to be normally stopped
	->search for BufferOverflows
      */
      if(false){
	if(stopType == 0 && (chID <-1 || chID >32)) LOG(FATAL) << "SpadicMessage: " << iSpadicMessage << " sourceA: " << sourceA << " chID: " << chID << " groupID: " << groupId << " spaID: " << spaID << " stopType: " << stopType << " infoType: " << infoType << " triggerType: " << triggerType << " isHit: " << isHit << " is Info: " << isInfo << FairLogger::endl;
	if(stopType == 0 && raw->GetNrSamples()==0 && iSpadicMessage < nSpadicMessages){
	  for ( Int_t i=iSpadicMessage;i<nSpadicMessages;i++){
	    if ((static_cast<CbmSpadicRawMessage*>(fRawSpadic->At(i)))->GetOverFlow()==true && GetSpadicID((static_cast<CbmSpadicRawMessage*>(fRawSpadic->At(i)))->GetSourceAddress())==spaID)
	      {
		lostMessages = (static_cast<CbmSpadicRawMessage*>(fRawSpadic->At(i)))->GetBufferOverflowCount();
		break;
	      }
	    //LOG(INFO) << i << "Info not found " << FairLogger::endl;
	  }
	  LOG(INFO) << "SpadicMessages: " << nSpadicMessages << " Lost Messages " <<FairLogger::endl;
	  LOG(ERROR) << "SpadicMessage: " << iSpadicMessage << " sourceA: " << sourceA << " chID: " << raw->GetChannelID() << " groupID: " << groupId << " spaID: " << spaID << " stopType: " << stopType << " infoType: " << infoType << " triggerType: " << triggerType << " isHit: " << isHit << " is Info: " << isInfo << " Lost Messages: " << lostMessages <<FairLogger::endl;
	}
      }


      //Fill trigger-type histogram
      fHM->H1("Trigger")->Fill(spadicName,1);
      fHM->H1("MessageCount")->Fill(TString(spadicName+"_"+stopName),1);
      if(stopType==-1) fHM->H1("MessageCount")->Fill(TString(spadicName+"_"+stopName+" n-fold"),1); // replace weight 1 with number of lost messages
    }
    //std::cout << "   tuut" << std::endl;
  }
  // complicated loop over sorted map of timestamps, manually delete all elements in the nested maps
  // commented out, since obviuously the following outer clear command destructs all contained elements recursively
  // The objects correlated to the pointers stored in this map are deleted after each run of Exec. Therefore it is leading to seg.fa. if one tryies to delete the pointers here twice
  /*
    for(std::map<TString, std::map<ULong_t, std::vector<CbmSpadicRawMessage*> > >::iterator it = timeBuffer.begin() ; it != timeBuffer.end(); it++){
    // complicated loop over sorted map of raw messages
    for (std::map<ULong_t, std::vector<CbmSpadicRawMessage*> > ::iterator it2 = it->second.begin() ; it2 != it->second.end(); it2++) {
    //      LOG(INFO) <<  "ClusterSize:" << Int_t(it2->second.size()) << FairLogger::endl;
    //for (std::vector<CbmSpadicRawMessage*>::iterator it3 = it2->second.begin(); it3 != it2->second.end(); it3++) {
    for(Int_t i = 0; i < Int_t(it2->second.size()); i++){
    //delete it2->second[i];//it3->second;
    // here: looping through the vector
    }
    //      it2->second.clear();
    }
    //    it->second.clear();
    }
  */
  timeBuffer.clear();

  // Fill number of spadic-messages in tscounter-graph. Use TimeSlices (slices in processing time) here instead of physical full-time on the x-axis.
  // Length of one timeslice: m * n * 8 ns, with e.g. n=1250 length of microslice and m=100 microslices in one timeslice at SPS2015
  if (fGraph) {
  fHM->G1("TsCounter")->SetPoint(fHM->G1("TsCounter")->GetN(),fNrTimeSlices+1,nSpadicMessages);
  fHM->G1("TsCounterHit0")->SetPoint(fHM->G1("TsCounterHit0")->GetN(),fNrTimeSlices+1,nSpadicMessagesHit0);
  fHM->G1("TsCounterHit1")->SetPoint(fHM->G1("TsCounterHit1")->GetN(),fNrTimeSlices+1,nSpadicMessagesHit1);
  fHM->G1("TsCounterHitAborted0")->SetPoint(fHM->G1("TsCounterHitAborted0")->GetN(),fNrTimeSlices+1,nSpadicMessagesHitAborted0);
  fHM->G1("TsCounterHitAborted1")->SetPoint(fHM->G1("TsCounterHitAborted1")->GetN(),fNrTimeSlices+1,nSpadicMessagesHitAborted1);
  fHM->G1("TsCounterOverflow0")->SetPoint(fHM->G1("TsCounterOverflow0")->GetN(),fNrTimeSlices+1,nSpadicMessagesOverflow0);
  fHM->G1("TsCounterOverflow1")->SetPoint(fHM->G1("TsCounterOverflow1")->GetN(),fNrTimeSlices+1,nSpadicMessagesOverflow1);
  if(nSpadicMessagesOverflow0 > 0) fHM->G1("TsLost0")->SetPoint(fHM->G1("TsLost0")->GetN(),fNrTimeSlices+1,(Double_t(nSpadicMessagesLost0)/Double_t(nSpadicMessagesOverflow0)));
  else fHM->G1("TsLost0")->SetPoint(fHM->G1("TsLost0")->GetN(),fNrTimeSlices+1,0);
  if(nSpadicMessagesOverflow1 > 0) fHM->G1("TsLost1")->SetPoint(fHM->G1("TsLost1")->GetN(),fNrTimeSlices+1,(Double_t(nSpadicMessagesLost1)/Double_t(nSpadicMessagesOverflow1)));
  else fHM->G1("TsLost1")->SetPoint(fHM->G1("TsLost1")->GetN(),fNrTimeSlices+1,0);
  fHM->G1("TsCounterInfo0")->SetPoint(fHM->G1("TsCounterInfo0")->GetN(),fNrTimeSlices+1,nSpadicMessagesInfo0);
  fHM->G1("TsCounterInfo1")->SetPoint(fHM->G1("TsCounterInfo1")->GetN(),fNrTimeSlices+1,nSpadicMessagesInfo1);
  fHM->G1("TsCounterEpoch0")->SetPoint(fHM->G1("TsCounterEpoch0")->GetN(),fNrTimeSlices+1,nSpadicMessagesEpoch0);
  fHM->G1("TsCounterEpoch1")->SetPoint(fHM->G1("TsCounterEpoch1")->GetN(),fNrTimeSlices+1,nSpadicMessagesEpoch1);
  fHM->G1("TsCounterStrange0")->SetPoint(fHM->G1("TsCounterStrange0")->GetN(),fNrTimeSlices+1,nSpadicMessagesStrange0);
  fHM->G1("TsCounterStrange1")->SetPoint(fHM->G1("TsCounterStrange1")->GetN(),fNrTimeSlices+1,nSpadicMessagesStrange1);
  if(nSpadicMessages0 > 0) fHM->G1("TsStrangeness0")->SetPoint(fHM->G1("TsStrangeness0")->GetN(),fNrTimeSlices+1,(Double_t(nSpadicMessagesStrange0)/Double_t(nSpadicMessages0)));
  else fHM->G1("TsStrangeness0")->SetPoint(fHM->G1("TsStrangeness0")->GetN(),fNrTimeSlices+1,0);
  if(nSpadicMessages1 > 0) fHM->G1("TsStrangeness1")->SetPoint(fHM->G1("TsStrangeness1")->GetN(),fNrTimeSlices+1,(Double_t(nSpadicMessagesStrange1)/Double_t(nSpadicMessages1)));
  else fHM->G1("TsStrangeness1")->SetPoint(fHM->G1("TsStrangeness1")->GetN(),fNrTimeSlices+1,0);
  }
  // Catch empty TimeSlices.
  if(fNrTimeSlices==0){
    if(fHM->G1("TsCounter")->GetN()==0){
      LOG(INFO) << "Expected entries in TsCounter after finishinig first TimeSlice, but found none." << FairLogger::endl;
    }
  }
  fNrTimeSlices++;

  if(fNrTimeSlices % 1 ==0)
    {
      if (fActivateClusterizer){
	if (fNrTimeSlices!=0)ClusterizerTime();
	//ClusterizerSpace();
      }
      CleanUpBuffers();
    }
}
// ---- Finish  -------------------------------------------------------
void CbmTrdTimeCorrel::Finish()
{
  ClusterizerTime();
  fRawMessages->Clear();
  fDigis->Clear();
  fClusters->Clear();
  FitBaseline();
  FitPRF();

  TString runName="";
  if(fRun!=0) runName=Form(" (Run %d)",fRun);

  // Plot message counter histos to screen
  TCanvas *c1 = new TCanvas("c1","histograms"+runName,5*320,3*300);
  c1->Divide(5,3);
  c1->cd(1);
  fHM->G1("TsCounter")->Draw("AL");
  fHM->G1("TsCounter")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounter")->GetYaxis()->SetTitle("total SPADIC(all) messages");
  c1->cd(6);
  fHM->G1("TsCounterHit0")->Draw("AL");
  fHM->G1("TsCounterHit0")->SetLineColor(kRed);
  fHM->G1("TsCounterHit0")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounterHit0")->GetYaxis()->SetTitle("SPADIC0 hit messages");
  /*
    c1->cd(8);
    fHM->G1("TsCounterHitAborted0")->Draw("AL");
    fHM->G1("TsCounterHitAborted0")->SetLineColor(kRed);
    fHM->G1("TsCounterHitAborted0")->GetXaxis()->SetTitle("TS number");
    fHM->G1("TsCounterHitAborted0")->GetYaxis()->SetTitle("SPADIC0 hit aborted messages");
  */
  c1->cd(7);
  fHM->G1("TsCounterOverflow0")->Draw("AL");
  fHM->G1("TsCounterOverflow0")->SetLineColor(kRed);
  fHM->G1("TsCounterOverflow0")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounterOverflow0")->GetYaxis()->SetTitle("SPADIC0 overflow messages");
  c1->cd(8);
  fHM->G1("TsCounterInfo0")->Draw("AL");
  fHM->G1("TsCounterInfo0")->SetLineColor(kRed);
  fHM->G1("TsCounterInfo0")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounterInfo0")->GetYaxis()->SetTitle("SPADIC0 info messages");
  c1->cd(9);
  fHM->G1("TsCounterEpoch0")->Draw("AL");
  fHM->G1("TsCounterEpoch0")->SetLineColor(kRed);
  fHM->G1("TsCounterEpoch0")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounterEpoch0")->GetYaxis()->SetTitle("SPADIC0 epoch messages");
  c1->cd(10);
  fHM->G1("TsCounterStrange0")->Draw("AL");
  fHM->G1("TsCounterStrange0")->SetLineColor(kRed);
  fHM->G1("TsCounterStrange0")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounterStrange0")->GetYaxis()->SetTitle("SPADIC0 strange messages");
  c1->cd(11);
  fHM->G1("TsCounterHit1")->Draw("AL");
  fHM->G1("TsCounterHit1")->SetLineColor(kBlue);
  fHM->G1("TsCounterHit1")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounterHit1")->GetYaxis()->SetTitle("SPADIC1 hit messages");
  /*
    c1->cd(14);
    fHM->G1("TsCounterHitAborted1")->Draw("AL");
    fHM->G1("TsCounterHitAborted1")->SetLifNrTimeSlicesneColor(kBlue);
    fHM->G1("TsCounterHitAborted1")->GetXaxis()->SetTitle("TS number");
    fHM->G1("TsCounterHitAborted1")->GetYaxis()->SetTitle("SPADIC1 hit aborted messages");
  */
  c1->cd(12);
  fHM->G1("TsCounterOverflow1")->Draw("AL");
  fHM->G1("TsCounterOverflow1")->SetLineColor(kBlue);
  fHM->G1("TsCounterOverflow1")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounterOverflow1")->GetYaxis()->SetTitle("SPADIC1 overflow messages");
  c1->cd(13);
  fHM->G1("TsCounterInfo1")->Draw("AL");
  fHM->G1("TsCounterInfo1")->SetLineColor(kBlue);
  fHM->G1("TsCounterInfo1")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounterInfo1")->GetYaxis()->SetTitle("SPADIC1 info messages");
  c1->cd(14);
  fHM->G1("TsCounterEpoch1")->Draw("AL");
  fHM->G1("TsCounterEpoch1")->SetLineColor(kBlue);
  fHM->G1("TsCounterEpoch1")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounterEpoch1")->GetYaxis()->SetTitle("SPADIC1 epoch messages");
  c1->cd(15);
  fHM->G1("TsCounterStrange1")->Draw("AL");
  fHM->G1("TsCounterStrange1")->SetLineColor(kBlue);
  fHM->G1("TsCounterStrange1")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounterStrange1")->GetYaxis()->SetTitle("SPADIC1 strange messages");
  c1->SaveAs("pics/"+runName+"TsCounter.png");

  // Plot message counter ratios to screen
  TCanvas *c2 = new TCanvas("c2","ratios"+runName,3*320,2*300);
  c2->Divide(3,2);
  c2->cd(1);
  fHM->G1("TsLost0")->Draw("AB"); // use bar chart here, since we have a ratio
  fHM->G1("TsLost0")->SetLineColor(kRed);
  fHM->G1("TsLost0")->SetFillColor(kRed);
  fHM->G1("TsLost0")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsLost0")->GetYaxis()->SetTitle("SPADIC0 lost per overflow");
  c2->cd(4);
  fHM->G1("TsLost1")->Draw("AB"); // bar chart
  fHM->G1("TsLost1")->SetLineColor(kBlue);
  fHM->G1("TsLost1")->SetFillColor(kBlue);
  fHM->G1("TsLost1")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsLost1")->GetYaxis()->SetTitle("SPADIC1 lost per overflow");
  c2->cd(2);
  fHM->G1("TsStrangeness0")->Draw("AB"); // bar chart
  fHM->G1("TsStrangeness0")->SetLineColor(kRed);
  fHM->G1("TsStrangeness0")->SetFillColor(kRed);
  fHM->G1("TsStrangeness0")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsStrangeness0")->GetYaxis()->SetTitle("SPADIC0 strangeness");
  c2->cd(5);
  fHM->G1("TsStrangeness1")->Draw("AB"); // bar chart
  fHM->G1("TsStrangeness1")->SetLineColor(kBlue);
  fHM->G1("TsStrangeness1")->SetFillColor(kBlue);
  fHM->G1("TsStrangeness1")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsStrangeness1")->GetYaxis()->SetTitle("SPADIC1 strangeness");
  c2->SaveAs("pics/"+runName+"TsCounterRatio.png");

  if(fActivateDeltaTAnalysis)
  if (false) {
    TCanvas *c3 = nullptr;

    for (Int_t SysID=0; SysID<1;++SysID)
      for (Int_t SpaID=0; SpaID<4;++SpaID){
	if(SpaID%2 == 0){
	  c3 = new TCanvas("c3","Delta_t"+runName+"_Spadic_"+std::to_string(SpaID),5*320,3*300);
	  c3->Divide(8,4);
	}
	for (Int_t ChID=0; ChID<32;++ChID)
	  {
	    c3->cd(ChID+1);
	    fHM->G1(("Delta_t_for_Syscore_"+std::to_string(SysID)+"_Spadic_"+std::to_string(SpaID/2)+"_Channel_"+std::to_string(ChID)))->Draw("AB");
	    fHM->G1(("Delta_t_for_Syscore_"+std::to_string(SysID)+"_Spadic_"+std::to_string(SpaID/2)+"_Channel_"+std::to_string(ChID)))->SetLineColor(kRed);
	    fHM->G1(("Delta_t_for_Syscore_"+std::to_string(SysID)+"_Spadic_"+std::to_string(SpaID/2)+"_Channel_"+std::to_string(ChID)))->SetFillColor(kRed);
	    fHM->G1(("Delta_t_for_Syscore_"+std::to_string(SysID)+"_Spadic_"+std::to_string(SpaID/2)+"_Channel_"+std::to_string(ChID)))->GetXaxis()->SetTitle("Fulltime()");
  	  }
	c3->Update();
	c3->SaveAs(TString("pics/Delta_t_Graph_Spadic_"+std::to_string(SpaID/2)+".pdf"));
      };

    TCanvas *c4 = new TCanvas("c4","Time_Offsets"+runName,5*320,3*300);
    c4->Divide(4,4);
    Int_t i=1;
    for (Int_t baseSpaID=0; baseSpaID<4;++baseSpaID)
      for (Int_t compSpaID=0; compSpaID<4;++compSpaID)
	{
	  c4->cd(i++);
	  fHM->G1(("Time_Offset_between_Spadic_"+std::to_string(baseSpaID)+"_and_Spadic_"+std::to_string(compSpaID)))->Draw("AB");
	  fHM->G1(("Time_Offset_between_Spadic_"+std::to_string(baseSpaID)+"_and_Spadic_"+std::to_string(compSpaID)))->SetLineColor(kRed);
	  fHM->G1(("Time_Offset_between_Spadic_"+std::to_string(baseSpaID)+"_and_Spadic_"+std::to_string(compSpaID)))->GetXaxis()->SetTitle("Timeslice");
	}
    c4->Update();
    c4->SaveAs(TString("pics/"+runName+"TimeOffsets"+".pdf"));
  }
  
  if (fActivateOffsetAnalysis) {
    TCanvas *c5 = new TCanvas("c5","Fulltime_vs_Timeslice"+runName,5*320,3*300);
    TMultiGraph * mg = new TMultiGraph("Fulltime_vs_TimeSlice_all_Spadics","Fulltime_vs_TimeSlice_all_Spadics");
    for (Int_t baseSpaID=0; baseSpaID<4;++baseSpaID){
      fHM->G1(("Timestamps_Spadic"+std::to_string(baseSpaID)))->SetMarkerStyle(20);
      fHM->G1(("Timestamps_Spadic"+std::to_string(baseSpaID)))->SetMarkerColor(baseSpaID+2);
      mg->Add(fHM->G1(("Timestamps_Spadic"+std::to_string(baseSpaID))));
    }
    mg->Draw("AB");
    mg->GetXaxis()->SetTitle("Timeslice");
    mg->GetYaxis()->SetTitle("SuperEpoch");
    fHM->Add("Fulltime_vs_TimeSlice_all_Spadic",mg);
    c5->SaveAs(TString("pics/"+runName+"SuperEpochs"+".png"));
  }
  
  //Perform uniform relabeling of Axis
  ReLabelAxis(fHM->H1("InfoType_vs_Channel")->GetYaxis(),"infoType",true,true);

  // use this to produce nice single plots
  /*
    TCanvas *cnice = new TCanvas("cnice","cnice",800,400); 
    cnice->cd();
    fHM->G1("TsCounterHit1")->Draw("AL");
    fHM->G1("TsCounterHit1")->SetLineColor(kBlack);
    fHM->G1("TsCounterHit1")->GetXaxis()->SetTitle("timeslice");
    fHM->G1("TsCounterHit1")->GetXaxis()->SetRangeUser(0,2166);
    fHM->G1("TsCounterHit1")->GetYaxis()->SetTitle("SPADIC1 hit messages");
  */
  //Buffer (map) or multi SPADIC data streams based analyis have to be done here!!

  std::vector<TH2*> TH2vector = fHM->H2Vector(".*");
  for (std::vector<TH2*>::iterator it = TH2vector.begin() ; it != TH2vector.end(); ++it){
    LOG(INFO) << (*it)->GetTitle() << FairLogger::endl;
    (*it)->SetContour(99);
    }

  if(fBatchAssessment){
	  vector<TH2*> PadResponses = fHM->H2Vector("Pad_Response.*");
	  vector<TH2*> Heatmap = fHM->H2Vector(".*Heatmap.*");
	  vector<TH2*> Baselines = fHM->H2Vector(".*Baseline.*");
	  TCanvas *c6 = new TCanvas("BatchAssesment",""+runName,1600,900);
	  TString path = FairRootManager::Instance()->GetOutFile()->GetName();
	  TString RunNumber = path(TRegexp("[0-9]+_"));
	  path = "/ddfs/user/data/p/p_munk01/Pictures";
	  for (auto h : PadResponses){
		  TString Title = h->GetTitle();
		  if(!(Title.Contains("_3")||Title.Contains("_4"))) continue;
		  c6->Clear();
		  c6->SetLogz();
		  h->SetContour(99);
		  h->Draw("colz");
		  c6->Update();
		  c6->SaveAs(path+RunNumber+Title+".png");
	  }
	  for (auto h : Heatmap){
		  TString Title = h->GetTitle();
		  if(!(Title.Contains("Cluster(3")||Title.Contains("Cluster(4"))) continue;
		  c6->Clear();
		  c6->SetLogz();
		  h->SetContour(99);
		  h->Draw("colz");
		  c6->Update();
		  c6->SaveAs(path+RunNumber+h->GetTitle()+".png");
	  }
	  for (auto h : Baselines){
		  TString Title = h->GetTitle();
		  c6->Clear();
		  c6->SetLogz();
		  h->SetContour(99);
		  h->Draw("colz");
		  c6->Update();
		  c6->SaveAs(path+RunNumber+h->GetTitle()+".png");
	  }
  }

  LOG(DEBUG) << "Finish of CbmTrdTimeCorrel" << FairLogger::endl;
  LOG(INFO) << "Write histo list to " << FairRootManager::Instance()->GetOutFile()->GetName() << FairLogger::endl;
  FairRootManager::Instance()->GetOutFile()->cd();
  fHM->WriteToFile();
  //if(false)
  if(fDrawSignalDebugHistograms){
	  TCanvas *c6 = new TCanvas("c6","Debug_Signal_Shapes"+runName,5*320,3*300);
	  TH2vector = fBaselineHM->H2Vector(".*");
	  TH2* currentPtr=nullptr;
	  for (std::vector<TH2*>::iterator it = TH2vector.begin() ; it != TH2vector.end(); ++it){
		  if(*it!=nullptr)currentPtr =*it;
		  c6->Clear();
		  LOG(INFO) << currentPtr->GetTitle() << FairLogger::endl;
		  currentPtr->SetContour(99);
		  currentPtr->Draw("colz");
		  c6->Update();
		  c6->SaveAs("pics/"+TString(currentPtr->GetTitle())+".pdf");
		  c6->SaveAs("pics/"+TString(currentPtr->GetTitle())+".png");
	  }
		TString outfile = FairRootManager::Instance()->GetOutFile()->GetName();
		outfile.ReplaceAll(".root", "_Signal_shapes.root");
		TFile Outfile(outfile, "RECREATE");
		Outfile.cd();
		fBaselineHM->WriteToFile();
		FairRootManager::Instance()->GetOutFile()->cd();
		Outfile.Close();

  }
  //delete c1;
  TString outfile = FairRootManager::Instance()->GetOutFile()->GetName();
      outfile.ReplaceAll(".root", "_Calibration.root");
    LOG(INFO) << "Baselines extracted and stored at "<< outfile << FairLogger::endl << " If this is your first time running this analysis, please rerun this analysis." <<FairLogger::endl;
}
// ---- FinishEvent  -------------------------------------------------------
void CbmTrdTimeCorrel::FinishEvent()
{
  LOG(DEBUG) << "FinishEvent of CbmTrdTimeCorrel" << FairLogger::endl;
  
  LOG(INFO) << "Digis in TClonesArray:                           " << fDigis->GetEntriesFast() << FairLogger::endl;
  LOG(INFO) << "Clusters in TClonesArray:                        " << fClusters->GetEntriesFast() << FairLogger::endl;
  fClusters->Delete();
  fClusters = new TClonesArray("CbmTrdCluster");
  fDigis->Delete();
  fDigis = new TClonesArray("CbmTrdDigi");  
}
// -------------------------------------------------------------------------
void CbmTrdTimeCorrel::ClusterizerTime()
{
  static Int_t Clusterrun =0;
  auto CompareSpadicMessages=
      [&](CbmSpadicRawMessage* a,CbmSpadicRawMessage* b)
	  {
		  Int_t SpadicIDA = GetSpadicID(a->GetSourceAddress());
		  Int_t SpadicIDB = GetSpadicID(b->GetSourceAddress());
		  Int_t ChIDA = a->GetChannelID() + ((SpadicIDA %2 == 1)? 16 : 0);
		  Int_t ChIDB = b->GetChannelID() + ((SpadicIDB %2 == 1)? 16 : 0);
		  Int_t PadA = GetChannelOnPadPlane(ChIDA);
		  Int_t PadB = GetChannelOnPadPlane(ChIDB);
		  Int_t rowA = (SpadicIDA/2)*2+PadA/16;
		  Int_t rowB = (SpadicIDB/2)*2+PadB/16;
		  if(a->GetFullTime() == b->GetFullTime())
			  if (SpadicIDA == SpadicIDB)
				  if(rowA==rowB)
					  if(PadA==PadB)
						  return true;
		  return false;
      };
  auto CompareSpadicMessagesSmaller=
      [&](CbmSpadicRawMessage* a,CbmSpadicRawMessage* b)
      {
		Int_t SpadicIDA = GetSpadicID(a->GetSourceAddress());
		Int_t SpadicIDB = GetSpadicID(b->GetSourceAddress());
        Int_t ChIDA = a->GetChannelID() + ((SpadicIDA %2 == 1)? 16 : 0);
        Int_t ChIDB = b->GetChannelID() + ((SpadicIDB %2 == 1)? 16 : 0);
        Int_t PadA = GetChannelOnPadPlane(ChIDA);
        Int_t PadB = GetChannelOnPadPlane(ChIDB);
        Int_t rowA = (SpadicIDA/2)*4+PadA/16;
        Int_t rowB = (SpadicIDB/2)*4+PadB/16;
        if(a->GetFullTime() < b->GetFullTime())
          if(rowA<rowB)
            return true;
          else if(rowA == rowB)
            if(PadA<PadB)
              return true;
        return false;
      };
  const Int_t clusterWindow = 0; // size of time window in which two hits are called "correlated", unit is timestamps
  std::sort(fLinearHitBuffer.begin(),fLinearHitBuffer.end(),CompareSpadicMessagesSmaller);
  std::unique(fLinearHitBuffer.begin(),fLinearHitBuffer.end(),CompareSpadicMessages);
  std::multimap<ULong_t, CbmSpadicRawMessage*> TempHitMap,TempOverflowMap;
  for (auto x : fLinearHitBuffer){
      long unsigned int MessageTime = static_cast <long unsigned int>(x->GetFullTime());
      (x->GetOverFlow()) ? TempOverflowMap.insert(std::make_pair(MessageTime,x)): TempHitMap.insert(std::make_pair(MessageTime,x));
  }
  Int_t LastTimestamp = 0;
  for (auto it=TempHitMap.begin(); it != TempHitMap.end(); ++it){
      auto range = std::make_pair(TempHitMap.lower_bound(it->first), TempHitMap.upper_bound(it->first + clusterWindow));
      auto rangeCopy=range;
      if(TempHitMap.size()==0 || (range.first == TempHitMap.end())) continue;
      for (;range.first != range.second; ++(range.first)){
	  if(it->second == nullptr || range.first->second == nullptr) continue;
	    if(range.first->second->GetTriggerType() == 1 || range.first->second->GetTriggerType() ==3) { // exclude purely neighbour triggered hits
		if(it->second->GetTriggerType() == 1 || it->second->GetTriggerType() == 3) { // exclude purely neighbour triggered hits for the comparator side too
		    Int_t ChID1 = it->second->GetChannelID();
		    Int_t ChID2 = range.first->second->GetChannelID();
		    Int_t SpaID1 = GetSpadicID(it->second->GetSourceAddress());
		    Int_t SpaID2 = GetSpadicID(range.first->second->GetSourceAddress());
		    ChID1 += (SpaID1 %2 == 1)? 16 : 0; // Remap the channel IDs of each second Half-Spadic to 16...31
		    ChID2 += (SpaID2 %2 == 1)? 16 : 0;
		    // special for SPS2015 data: the chamber with SpaID 2 and 3 was turned by 180 degree with respect to the first one. thus, turn the pad plane number here
		    //Int_t SpaPad1 = (Int_t)(SpaID1/2) * 32 + ((SpaID1>1) ? 31-GetChannelOnPadPlane(ChID1) : GetChannelOnPadPlane(ChID1));
		    //Int_t SpaPad2 = (Int_t)(SpaID2/2) * 32 + ((SpaID2>1) ? 31-GetChannelOnPadPlane(ChID2) : GetChannelOnPadPlane(ChID2));
		    Int_t SpaPad1 = (Int_t)(SpaID1/2) * 32 + (GetChannelOnPadPlane(ChID1));
		    Int_t SpaPad2 = (Int_t)(SpaID2/2) * 32 + (GetChannelOnPadPlane(ChID2));
		    if (it!=range.first) { //Exlude Correlations of messages with themselves
			//TODO:Implement Clusters and fill them
			if(fDebugMode){
			    fHM->H2("Hit_Coincidences")->Fill(SpaPad1,SpaPad2);
			    // Fill for map of correlations following: require origin from variant TRD chambers to cut on physical correlations between two chambers, which is adressed for the moment just by requiring different SpaID/2
			    // Furthermore, physical correlations coming from the target require a positve direction from Chamber 1 to Chamber 2, which means ascending SpaIDs
			    fHM->H2("Correlation_Map")->Fill(SpaPad2-SpaPad1-32,((range.first->second->GetFullTime())-(it->second->GetFullTime())));
			}else{
			    if (((Int_t)(SpaID1/2) - (Int_t)(SpaID2/2)) != 0) fHM->H2("Hit_Coincidences")->Fill(SpaPad1,SpaPad2);
			    // Fill for map of correlations following: require origin from variant TRD chambers to cut on physical correlations between two chambers, which is adressed for the moment just by requiring different SpaID/2
			    // Furthermore, physical correlations coming from the target require a positve direction from Chamber 1 to Chamber 2, which means ascending SpaIDs
			    if (((Int_t)(SpaID2/2) - (Int_t)(SpaID1/2)) > 0) fHM->H2("Correlation_Map")->Fill(SpaPad2-SpaPad1-32,((range.first->second->GetFullTime())-(it->second->GetFullTime())));
			}
		    }
		}
	    }
      }
      if(LastTimestamp>=it->first-clusterWindow) continue;
      else LastTimestamp = it->first;
/*
 * Important Notice:
 * If fActivate2DClusterizer is set to true,
 * Messages are sorted byChIDs instead of PadID's,
 * also turns of row detection.
 */
      auto SortSpadicMessageRange =
    		  [&](CbmSpadicRawMessage a,CbmSpadicRawMessage b)
			  {	if (fActivate2DClusterizer) {
				  Int_t ChIDA = a.GetChannelID() + ((GetSpadicID(a.GetSourceAddress()) %2 == 1)? 16 : 0);
				  Int_t ChIDB = b.GetChannelID() + ((GetSpadicID(b.GetSourceAddress()) %2 == 1)? 16 : 0);
				  Int_t SpaIDA = (GetSpadicID(a.GetSourceAddress())/2);
				  Int_t SpaIDB = (GetSpadicID(b.GetSourceAddress())/2);
				  if(SpaIDA<SpaIDB)
					  return true;
				  else if(SpaIDA == SpaIDB)
					  if(ChIDA<ChIDB)
						  return true;
				  return false;
			  } else {
				  Int_t ChIDA = a.GetChannelID() + ((GetSpadicID(a.GetSourceAddress()) %2 == 1)? 16 : 0);
				  Int_t ChIDB = b.GetChannelID() + ((GetSpadicID(b.GetSourceAddress()) %2 == 1)? 16 : 0);
				  Int_t PadA = GetChannelOnPadPlane(ChIDA);
				  Int_t PadB = GetChannelOnPadPlane(ChIDB);
				  Int_t rowA = (GetSpadicID(a.GetSourceAddress())/2)*2+PadA/16;
				  Int_t rowB = (GetSpadicID(b.GetSourceAddress())/2)*2+PadB/16;
				  if(rowA<rowB)
					  return true;
				  else if(rowA == rowB)
					  if(PadA<PadB)
						  return true;
				  return false;
			  }
          };
      Bool_t OutsideCluster = true;
      std::vector<CbmSpadicRawMessage> linearClusterBuffer;
      std::vector<Int_t> HitChs,HitSpadic;
      for (; rangeCopy.first != rangeCopy.second; ++(rangeCopy.first)) //Copy Messages from the previously selected range for easier access
        {
          linearClusterBuffer.push_back (*rangeCopy.first->second);
          Int_t Trigger = (*rangeCopy.first->second).GetTriggerType();
          if(Trigger==1||Trigger==3) {
        	  HitChs.push_back((*rangeCopy.first->second).GetChannelID()+(GetSpadicID((*rangeCopy.first->second).GetSourceAddress())%2)*16);
        	  HitSpadic.push_back(GetSpadicID((*rangeCopy.first->second).GetSourceAddress()));
          }
		}
		for (Int_t i = 0; i < HitChs.size(); i++) {
			for (auto x : linearClusterBuffer) {
				Int_t SpadicID = GetSpadicID(x.GetSourceAddress());
				Int_t Channel = x.GetChannelID()
						+ (GetSpadicID(x.GetSourceAddress()) % 2) * 16;
				if ((HitSpadic.at(i) / 2) == (SpadicID / 2)) {
					Int_t Trigger = x.GetTriggerType();
					if (std::abs(Channel - HitChs.at(i)) < 3)
						if (Trigger == 2) {
							string histname =
									"Trigger_Bug_Investigation_Syscore_"
											+ std::to_string(0) + "_Spadic_"
											+ std::to_string(SpadicID / 2);
							fHM->H2(histname)->Fill(HitChs.at(i), Channel);
						}
				}
			}
		}
      Clusterrun++;
      std::sort (linearClusterBuffer.begin (), linearClusterBuffer.end (),SortSpadicMessageRange);//Sort the Hitmessages by their position of origin on the padplane
      Cluster BuildingCluster (fBaseline,clusterWindow,120); //Create a new Cluster
      Int_t lastRow = (GetSpadicID(linearClusterBuffer.begin()->GetSourceAddress())/2)*32+GetChannelOnPadPlane(linearClusterBuffer.begin()->GetChannelID() + ((GetSpadicID(linearClusterBuffer.begin()->GetSourceAddress()) %2 == 1)? 16 : 0))/16;
      Int_t lastPad = GetChannelOnPadPlane(linearClusterBuffer.begin()->GetChannelID() + ((GetSpadicID(linearClusterBuffer.begin()->GetSourceAddress()) %2 == 1)? 16 : 0));
      auto SatteliteDebug =
    		  [&](Cluster currentCluster){
    	  if(/*currentCluster.size() != 2&&*/!currentCluster.Get2DStatus()) return 0;
    	  return 0;
    	  Int_t Detector = currentCluster.GetSpadic()/2;
          string Detectorname = (Detector == 0 ? "Frankfurt" : "Muenster");
    		  //std::cout << "Fragmented Cluster Found on Pad: " <<currentCluster.GetHorizontalPosition() << std::endl;
    		  //std::cout << "Satelites on ChID|SpaID|Timestamp|HitType|StopType" << std::endl;
    		  CbmSpadicRawMessage MainMassage;
    		  auto Messages= currentCluster.GetEntries();
    		  for (auto tempMessage=Messages.first;tempMessage!= Messages.second;tempMessage++){
    			  if (tempMessage->GetTriggerType()!=2) {
    				  MainMassage = *tempMessage;
    				  break;
    			  }
    		  }
    		  string sidedness= (Messages.first->GetTriggerType()==1)? "Left":"Right";
              string histName = "Shape_of_"+sidedness+"sided_Clusters"+ Detectorname;
              TH2* Hist = fHM->H2(histName);
    		  for (auto tempMessage=Messages.first;tempMessage!= Messages.second;tempMessage++){
    			  Hist->Fill(GetChannelonPadPlaneMessage(*tempMessage),tempMessage->GetTriggerType());
    		  }
    		  for (auto x: linearClusterBuffer ){
    		  std::cout << x.GetChannelID() << " " << GetSpadicID(x.GetSourceAddress()) <<" " << x.GetFullTime() << " " <<x.GetTriggerType() << " " << x.GetStopType() << std::endl;

    	  }
    		  std::cout << "All Satellites printed " << std::endl;
    		  std::cout << "________________________________________________________"<< std::endl;
    	  return 0;
      };
      for (auto currentMessage = linearClusterBuffer.begin(); currentMessage != linearClusterBuffer.end(); currentMessage++) //Loop over all Messages inside the Hitwindow
        {
    	  Int_t currentPad = GetChannelOnPadPlane(currentMessage->GetChannelID() + ((GetSpadicID(currentMessage->GetSourceAddress()) %2 == 1)? 16 : 0));
    	  if(BuildingCluster.size()>16) std::cout << "Cluster of Size " << BuildingCluster.size() << " with current Pad " << currentPad << " " << lastPad <<std::endl;
          if (currentPad-lastPad >1 ){
        	  if(!fActivate2DClusterizer) OutsideCluster=true;
        	  else if(currentPad-lastPad >3)OutsideCluster=true;
        	  //std::cout << "Gap in Cluster found " << std::endl;
          }
          Int_t currentRow = (GetSpadicID(currentMessage->GetSourceAddress())/2)*32+currentPad/16;
          if((!fActivate2DClusterizer&&(BuildingCluster.size () > currentPad || lastRow != currentRow||lastPad >= currentPad)))//Check if the current message is not from the same row/spadic
        	  {
              if (BuildingCluster.size () != 0)//Store away nonempty Cluster
                {
            	  SatteliteDebug(BuildingCluster);
                  fClusterBuffer.push_back (BuildingCluster);
                  Cluster tempCluster (fBaseline,clusterWindow,120);
                  BuildingCluster = tempCluster;
                }
              //std::cout << " Rowchange " << lastRow << " " << currentRow << " " << std::endl;
              OutsideCluster = true; //New Row, so we start outside any Cluster
              lastRow = currentRow;
            } else if(fActivate2DClusterizer){
            	if(currentRow%32==lastRow%32){
            		lastRow=currentRow;
            	}
            	else {
            		if (BuildingCluster.size () != 0)//Store away nonempty Cluster
            				{
            			SatteliteDebug(BuildingCluster);
            			fClusterBuffer.push_back (BuildingCluster);
            			Cluster tempCluster (fBaseline,clusterWindow,120);
            			BuildingCluster = tempCluster;
            				}
            		//std::cout << " Rowchange " << lastRow << " " << currentRow << " " << std::endl;
            		OutsideCluster = true; //New Row, so we start outside any Cluster
            		lastRow = currentRow;
            	}
            }

          if (currentMessage->GetTriggerType () == 2)//Check for neighbour triggered Messages
            {
              if (OutsideCluster)
                {//Beginning of a Cluster
                  if (BuildingCluster.size () != 0) //Store away the previous Cluster, if not already done
                    {
                	  SatteliteDebug(BuildingCluster);
                      fClusterBuffer.push_back (BuildingCluster);
                      Cluster tempCluster (fBaseline,clusterWindow,120);
                      BuildingCluster = tempCluster;
                    }
                  BuildingCluster.AddEntry (*currentMessage);
                  OutsideCluster = false;
                }
              else
                {//Test if currentMessage is Part of next Cluster
            	  /*
                  auto nextMessage = currentMessage;
                  if(currentMessage!=linearClusterBuffer.end()) nextMessage++;
                  if (currentMessage != linearClusterBuffer.begin())
                    if(nextMessage != linearClusterBuffer.end())
                      if(nextMessage->GetTriggerType () == 1||nextMessage->GetTriggerType () == 3)
                        {
                          if(GetChannelOnPadPlane(nextMessage->GetChannelID() + ((GetSpadicID(nextMessage->GetSourceAddress()) %2 == 1)? 16 : 0))-
                              currentPad >1)
                            {
                        	  std::cout << " Black Magic Code > 2 has triggered "<< std::endl;

                              BuildingCluster.AddEntry (*currentMessage);
                              if (BuildingCluster.size () != 0) //Store away the completed Cluster
                                {
                            	  SatteliteDebug(BuildingCluster);
                                  fClusterBuffer.push_back (BuildingCluster);
                                  Cluster tempCluster (fBaseline,clusterWindow,120);
                                  BuildingCluster = tempCluster;
                                }
                            }
                          else if(GetChannelOnPadPlane(nextMessage->GetChannelID() + ((GetSpadicID(nextMessage->GetSourceAddress()) %2 == 1)? 16 : 0))-
                              currentPad <=1)
                            {
                        	  std::cout << " Black Magic Code <=1 has triggered "<< std::endl;
                              if (BuildingCluster.size () != 0) //Store away the completed Cluster
                                {
                            	  SatteliteDebug(BuildingCluster);
                                  fClusterBuffer.push_back (BuildingCluster);
                                  Cluster tempCluster (fBaseline,clusterWindow,120);
                                  BuildingCluster = tempCluster;
                                }
                              currentMessage--;
                            }
                          OutsideCluster = true;
                          std::cout << "EVIL GOTO" << std::endl;
                          goto SchleifenendeClusterizer;
                        }*/
                  //End of a Cluster, marked by second neighbour triggered message
                  BuildingCluster.AddEntry (*currentMessage);
                  OutsideCluster = true;
                  if (BuildingCluster.size () != 0) //Store away the completed Cluster
                    {
                	  SatteliteDebug(BuildingCluster);
                      fClusterBuffer.push_back (BuildingCluster);
                      Cluster tempCluster (fBaseline,clusterWindow,120);
                      BuildingCluster = tempCluster;
                    }
                }
            }
          else if ((currentMessage->GetTriggerType () == 3 || currentMessage->GetTriggerType () == 1)){
              if (OutsideCluster)
                {//incomplete Cluster without a starting neighbour triggered Hitmessage
                  if (BuildingCluster.size () != 0)//Should not occur, but stores away previous Cluster
                    {
                	  SatteliteDebug(BuildingCluster);
                      fClusterBuffer.push_back (BuildingCluster);
                      Cluster tempCluster (fBaseline,clusterWindow,120);
                      BuildingCluster = tempCluster;
                    }//Start the Cluster
                  BuildingCluster.AddEntry (*currentMessage);
                  OutsideCluster = false;//we are now inside a cluster
                }
              else //Fill the Cluster
                {
                  BuildingCluster.AddEntry (*currentMessage);
                }
          }
          SchleifenendeClusterizer:
          lastPad = currentPad;
        }
      //std::cout << " CLusterizer loop finished" << std::endl;
      if (BuildingCluster.size () != 0) //Store away the last Cluster
        {
    	  SatteliteDebug(BuildingCluster);
          fClusterBuffer.push_back (BuildingCluster);
          Cluster tempCluster (fBaseline,clusterWindow,120);
          BuildingCluster = tempCluster;
        }

      //std::sort(rangeCopy.first,rangeCopy.second,CompareSpadicMessagesSmaller);
  }
  if(fClusterBuffer.size()>0)
  {
	  TH2* CoincidenceHistogram = fHM->H2("Cluster_Coincidences");
	  for (int i =0; i< fClusterBuffer.size()-1;i++){
		  Float_t CurrentPosition=fClusterBuffer.at(i).GetHorizontalPosition();
		  if (fClusterBuffer.at(i).size()<3 ||fClusterBuffer.at(i).size()>4) continue;
		  Int_t CurrentDetector = fClusterBuffer.at(i).GetSpadic()/2;
		  if(CurrentPosition>16.0) CurrentPosition -= 16.0;
		  Long_t CurrentTime = fClusterBuffer.at(i).GetFulltime();
		  for(int j=1;j<10&&i+j<fClusterBuffer.size();j++){
			  if(fClusterBuffer.at(i+j).GetSpadic()/2==CurrentDetector) continue;
			  if (fClusterBuffer.at(i+j).size()<3 ||fClusterBuffer.at(i+j).size()>4) continue;
			  Float_t NextPosition=fClusterBuffer.at(i+j).GetHorizontalPosition();
			  if(NextPosition>16.0) NextPosition -= 16.0;
			  Long_t NextTime = fClusterBuffer.at(i+j).GetFulltime();
			  CoincidenceHistogram->Fill(NextPosition - CurrentPosition,NextTime -CurrentTime);
		  }
	  }
  }

  for (auto x: fClusterBuffer){
      fHM->H1("Clustersize_for_Syscore_"+std::to_string (0)+"_Spadic_"+std::to_string (static_cast<Int_t>(x.GetSpadic ()/2)))->Fill(static_cast<Int_t>(x.size ()));
      fHM->H2("Cluster("+std::to_string(static_cast<Int_t>(x.size()))+")_Heatmap_for_Syscore_"+std::to_string (0) +"_Spadic_"+std::to_string(static_cast<Int_t>(x.GetSpadic ()/2)))->Fill(x.GetHorizontalPosition(),0/*<16.0 ? x.GetHorizontalPosition() : x.GetHorizontalPosition()-16.0),static_cast<Int_t>(x.GetRow())*/);
      string detectorName = (x.GetSpadic()/2 == 0 ? "Frankfurt" : "Muenster");
      if (fDrawClustertypes){
    	  string histname = "Clustertypes_for_Syscore_"+std::to_string(0)+"_Prototype_from_"+detectorName;
    	  fHM->H2(histname)->Fill(x.size(),x.Type());
      }
      if(x.Type() != 0 && x.size()>=3) continue;
      //if(x.size()==3&&(x.GetTotalCharge()> 250)) continue;
      if(fDrawPadResponse){
    	  string histname = "Pad_Response_"+ detectorName + "_for_Clusters_of_Size_"+std::to_string(static_cast<Int_t>(x.size()));
    	  x.FillChargeDistribution(fHM->H2(histname));
      }

  }
}
// -------------------------------------------------------------------------
void CbmTrdTimeCorrel::ClusterizerSpace()
{
  LOG(INFO) <<  "ClusterizerSpace"<< FairLogger::endl;
  Int_t mapDigiCounter = 0;
  CbmSpadicRawMessage* raw = NULL;
  //CbmTrdCluster* cluster = NULL;
  Int_t  layerId(0), moduleId(0), sectorId(0), rowId(0), columnId(0), clusterSize(0), address(0);
  ULong_t lastClusterTime = 0;
  ULong_t fullTime = 0;
  TString SysSpaID = "";
  std::vector<Int_t> digiIndices;
  for (std::map<TString, std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> > >::iterator SpaSysIt = fMessageBuffer.begin() ; SpaSysIt != fMessageBuffer.end(); SpaSysIt++){
    SysSpaID = SpaSysIt->first;
    for (std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> > ::iterator timeIt = (SpaSysIt->second).begin() ; timeIt != (SpaSysIt->second).end(); timeIt++){
      LOG(DEBUG) <<  "ClusterSize:" << Int_t((timeIt->second).size()) << FairLogger::endl;
      clusterSize = Int_t((timeIt->second).size());
      if ( SysSpaID != "")
	fHM->H1((SysSpaID + TString("_CluterSize")).Data())->Fill(clusterSize);

      fullTime = timeIt->first;
      lastClusterTime = fullTime;

      Int_t lastCombiID = -1;
      for (std::map<Int_t, CbmSpadicRawMessage*> ::iterator combiIt = (timeIt->second).begin(); combiIt != (timeIt->second).end(); combiIt++){
	mapDigiCounter++;
	if (combiIt->second == NULL) {
	  LOG(DEBUG) <<  "Entrie in fMessageBuffer == NULL. SysSpaID:" << SysSpaID << " fullTime:" << fullTime << " combiID:" << combiIt->first << FairLogger::endl;
	  continue;
	}
	raw = combiIt->second;
	
	layerId = GetLayerID(raw);
	moduleId = GetModuleID(raw);
	sectorId = GetSectorID(raw);
	rowId = GetRowID(raw);
	columnId = GetColumnID(raw);
	LOG(DEBUG) <<  " layerId:" << layerId  << " moduleId:" << moduleId << " sectorId:" << sectorId << " rowId:" << rowId << " columnId:" << columnId << FairLogger::endl;
	// BaseLineCorrection==
	Float_t Baseline = 0.;
	
	if (raw->GetStopType() == 0){
	  /*
	    for (Int_t bin = 1; bin < 4; bin++){
	    Baseline += raw->GetSamples()[raw->GetNrSamples()-bin];
	    }
	    Baseline /= 3.;
	  */
	} else {
	  // Use average baseline estimated for full message length
	}
	const Int_t nSamples = raw->GetNrSamples();
	//Float_t Samples[nSamples] = {0.};
	Float_t* Samples = new Float_t[nSamples];
	for (Int_t iBin = 0; iBin < raw->GetNrSamples(); iBin++){
	  Samples[iBin] = raw->GetSamples()[iBin] - Baseline;
	}
	address = CbmTrdAddress::GetAddress(layerId,moduleId,sectorId,rowId,columnId);
	if (fOutputCloneArrays){
	  new ((*fDigis)[fiDigi]) CbmTrdDigi(address,
					     raw->GetFullTime(),//57,14 ns per timestamp
					     raw->GetTriggerType(), raw->GetInfoType(), raw->GetStopType(),
					     raw->GetNrSamples(), Samples);
	}
	delete[] Samples;
	if (combiIt->first - lastCombiID != 1 && digiIndices.size() > 0){
	  address = CbmTrdAddress::GetAddress(layerId,moduleId,sectorId,rowId,columnId);
	  if (fOutputCloneArrays){
	    //new ((*fClusters)[fiCluster]) CbmTrdCluster(digiIndices,address);
	  }
	  digiIndices.clear();
	  fiCluster++;
	}
	digiIndices.push_back(fiDigi);
	lastCombiID = combiIt->first;
	fiDigi++;
      }
      if (digiIndices.size() > 0){
	address = CbmTrdAddress::GetAddress(layerId,moduleId,sectorId,rowId,columnId);
	if (fOutputCloneArrays){
	  //new ((*fClusters)[fiCluster]) CbmTrdCluster(digiIndices,address);
	}	
	digiIndices.clear();      
	fiCluster++;
      }
    }
  }
  //LOG(INFO) << "CbmTrdTimeCorrel::ClusterizerSpace Digis:" << mapDigiCounter << FairLogger::endl;
  //CleanUpBuffers();
}
// -------------------------------------------------------------------------
void CbmTrdTimeCorrel::CleanUpBuffers()
{  
  LOG(INFO) <<  "CleanUpBuffers" << FairLogger::endl;
  //LOG(INFO) <<  "fMessageBuffer.size() " << fMessageBuffer.size() << FairLogger::endl;
  for (std::map<TString, std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> > >::iterator SpaSysIt = fMessageBuffer.begin(); SpaSysIt != fMessageBuffer.end(); SpaSysIt++){
    //LOG(INFO) <<  "  (SpaSysIt->second).size() " << (SpaSysIt->second).size() << FairLogger::endl;
    for (std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> >::iterator TimeIt = SpaSysIt->second.begin(); TimeIt != SpaSysIt->second.end(); TimeIt++){
      //LOG(INFO) <<  "    (TimeIt->second).size() " << (TimeIt->second).size() << FairLogger::endl;
      for (std::map<Int_t, CbmSpadicRawMessage*> ::iterator CombiIt = TimeIt->second.begin(); CombiIt != TimeIt->second.end(); CombiIt++){
        //LOG(INFO) <<  "      (CombiIt->second).size() " << (CombiIt->second).size() << FairLogger::endl;
        if(CombiIt->second != NULL){
          delete CombiIt->second;
        }
      }
      TimeIt->second.clear();
    }
    SpaSysIt->second.clear();
  }
  fMessageBuffer.clear();  
  for (auto ptr :fLinearHitBuffer)
    delete ptr;
  fLinearHitBuffer.clear();
  fClusterBuffer.clear ();
  //fDigis->Clear("C");
  //fClusters->Clear("C");
}
// -------------------------------------------------------------------------
Int_t CbmTrdTimeCorrel::GetMessageType(CbmSpadicRawMessage* raw)
{
  Int_t messageType = -1;

  if (raw->GetEpoch())
    messageType = 0;
  else if (raw->GetEpochOutOfSynch())
    messageType = 1;
  else if (raw->GetHit())
    messageType = 2;
  else if (raw->GetHitAborted())
    messageType = 3;
  else if (raw->GetOverFlow())
    messageType = 5;
  else if (raw->GetInfo())
    messageType = 4;
  else if (raw->GetStrange())
    messageType = 6;

  return messageType;
}
// ----              -------------------------------------------------------
void CbmTrdTimeCorrel::ReLabelAxis(TAxis* axis, TString type, Bool_t underflow, Bool_t overflow)
{
  Int_t startBin(1), stopBin(axis->GetNbins());
  if (underflow){
    axis->SetBinLabel(1,"underflow -1");
    startBin++;
  }
  if (overflow){
    axis->SetBinLabel(axis->GetNbins(),"Type out of array");
    stopBin--;
  }
  TString messageTypes[7] = {"Epoch",
			     "Epoch out of synch",
			     "Hit",
			     "Hit aborted",
			     "Info",
			     "Overflow",
			     "Strange"};
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
  TString infoTypes[7] = {"Channel disabled during message building", 
			  "Next grant timeout", 
			  "Next request timeout", 
			  "New grant but channel empty", 
			  "Corruption in message builder", 
			  "Empty word", 
			  "Epoch out of sync"};
  if (type == "infoType") {
    if (stopBin - startBin == 7){
      for (Int_t iBin = startBin; iBin < stopBin; iBin++)
	axis->SetBinLabel(iBin,infoTypes[iBin-startBin]);
    } else {
      LOG(ERROR) << "  axis could not be labeled with " << type << ". Wrong number of bins in histogram." << FairLogger::endl; 
    }
  } else if (type == "triggerType") {
    if (stopBin - startBin == 4){
      for (Int_t iBin = startBin; iBin < stopBin; iBin++)
	axis->SetBinLabel(iBin,triggerTypes[iBin-startBin]); 
    } else {
      LOG(ERROR) << "  axis could not be labeled with " << type << ". Wrong number of bins in histogram." << FairLogger::endl; 
    }
  } else if (type == "stopType") {
    if (stopBin - startBin == 6){
      for (Int_t iBin = startBin; iBin < stopBin; iBin++)
	axis->SetBinLabel(iBin,stopTypes[iBin-startBin]);
    } else {
      LOG(ERROR) << "  axis could not be labeled with " << type << ". Wrong number of bins in histogram." << FairLogger::endl; 
    }
  } else if (type == "messageType") {
    if (stopBin - startBin == 7){
      for (Int_t iBin = startBin; iBin < stopBin; iBin++)
	axis->SetBinLabel(iBin,messageTypes[iBin-startBin]);
    } else {
      LOG(ERROR) << "  axis could not be labeled with " << type << ". Wrong number of bins in histogram." << FairLogger::endl; 
    }
  } else {
    LOG(ERROR) << type << " not known " << FairLogger::endl;
  }

}
// ----              -------------------------------------------------------
void CbmTrdTimeCorrel::CreateHistograms()
{    
  
  TString spadicName = "";
  TString runName="";
  if(fRun!=0) runName=Form(" (Run %d)",fRun);
  TString histName="";
  TString title="";

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
  
  fSpadics = 0;
  
  for(Int_t syscore = 0; syscore < 3; ++syscore) {
    for(Int_t spadic = 0; spadic < 3; ++spadic) {
      spadicName = RewriteSpadicName(Form("SysCore%01d_Spadic%01d", syscore, spadic));
      if(spadicName!="") fSpadics++;
    }   
  }
  
  
  Int_t n = 0;
  
  fHM->Add("Trigger", new TH1F("Trigger", "Trigger", 9,0,9));
  for(Int_t syscore = 0; syscore < 3; ++syscore) {
    for(Int_t spadic = 0; spadic < 3; ++spadic) {
      spadicName = RewriteSpadicName(Form("SysCore%01d_Spadic%01d", syscore, spadic));
      if(spadicName != "") {
        n++;
        fHM->H1("Trigger")->GetXaxis()->SetBinLabel(n,spadicName);
      }
    }
  }

  fHM->Add("MessageCount", new TH1D("MessageCount","MessageCount",fSpadics*8,0,fSpadics*8));
  n = 0;
  for(Int_t syscore = 0; syscore < 3; ++syscore) {
    for(Int_t spadic = 0; spadic < 3; ++spadic) {
      spadicName = RewriteSpadicName(Form("SysCore%01d_Spadic%01d", syscore, spadic));
      if(spadicName != "") {
	fHM->Add((spadicName + TString("_LostCount_vs_PadID")).Data(), new TH2D((spadicName + TString("_LostCount_vs_PadID")).Data(),(spadicName + TString("_LostCount_vs_PadID")).Data(),257,-0.5,256.5,32,-0.5,31.5));
	fHM->H2((spadicName + TString("_LostCount_vs_PadID")).Data())->GetXaxis()->SetTitle("LostMessages");
	fHM->H2((spadicName + TString("_LostCount_vs_PadID")).Data())->GetYaxis()->SetTitle("PadID");
	fHM->Add((spadicName + TString("_CluterSize")).Data(), new TH1D((spadicName + TString("_ClusterSize")).Data(),(spadicName + TString("_ClusterSize")).Data(),32,-0.5,31.5));
        for(Int_t stopType = 0; stopType < 6; ++stopType) {
          n++;
          fHM->H1("MessageCount")->GetXaxis()->SetBinLabel(n,TString(spadicName+"_"+stopTypes[stopType]));
        }
        n++;
        fHM->H1("MessageCount")->GetXaxis()->SetBinLabel(n,TString(spadicName+"_Info or epoch mess"));
        n++;
        fHM->H1("MessageCount")->GetXaxis()->SetBinLabel(n,TString(spadicName+"_Info or epoch mess n-fold"));
      }
    }
  }

  for(Int_t syscore = 0; syscore < 3; ++syscore) {
    for(Int_t spadic = 0; spadic < 3; ++spadic) {
      spadicName = RewriteSpadicName(Form("SysCore%01d_Spadic%01d", syscore, spadic));
      if(spadicName != "") {
	for(Int_t halfchip = 0; halfchip < 2; ++halfchip) {
	  histName = spadicName + "_Half_" + std::to_string(halfchip) + "_Time_vs_TimeSlice";
	  title = histName + runName + ";TimeSlice;Time";
	  fHM->Add(histName.Data(), new TH2F(histName, title, 2001, -0.5, 2000.5, 9000, 0, 90000000000));
	}
      }
    }
  }

  for(Int_t syscore = 0; syscore < 3; ++syscore) {
    for(Int_t spadic = 0; spadic < 3; ++spadic) {
      for(Int_t halfchip = 0; halfchip < 2; ++halfchip) {
	spadicName = RewriteSpadicName(Form("SysCore%01d_Spadic%01d", syscore, spadic));
	if(spadicName != "") {
	  histName = spadicName + "_Half_" + std::to_string(halfchip) + "_Epoch_vs_TimeSlice";
	  title = histName + runName + ";TimeSlice;Epoch";
	  fHM->Add(histName.Data(), new TH2F(histName, title, 2001, -0.5, 2000.5, 4097, -0.5, 4096.5));
	}
      }
    }
  }

  for(Int_t syscore = 0; syscore < 3; ++syscore) {
    for(Int_t spadic = 0; spadic < 3; ++spadic) {
      for(Int_t halfchip = 0; halfchip < 2; ++halfchip) {
	spadicName = RewriteSpadicName(Form("SysCore%01d_Spadic%01d", syscore, spadic));
	if(spadicName != "") {
	  histName = "Hitrate_vs_DeltaEpoch_hist_for_Syscore_"+std::to_string(syscore)+"_Spadic_"+std::to_string(spadic)+"_Half_"+std::to_string(halfchip);
	  title = histName + runName + ";DeltaEpoch;HitMessages per TimeSlice";
	  fHM->Add(histName.Data(), new TH2F(histName, title, 8301, -4150.5, 4150.5, 1000, 0, 10E5));
	}
      }
    }
  }

  for (Int_t syscore=0; syscore<3;++syscore) {
    for (Int_t spadic=0; spadic<3;++spadic) {
      for (Int_t halfchip=0; halfchip<2;++halfchip) {
	spadicName = RewriteSpadicName(Form("SysCore%01d_Spadic%01d", syscore, spadic));
	if(spadicName != "") {
	  histName = "EpochTrend_postregress_hist_for_Syscore_"+std::to_string(syscore)+"_Spadic_"+std::to_string(spadic)+"_Half_"+std::to_string(halfchip);
	  title = histName + runName;
	  fHM->Add(histName.Data(), new TH2I(histName, title, 150,0.5,150.5,201,-50.5,150.5));
	}
      }
    }
  }
  if(fActivateDeltaTAnalysis)
  for (Int_t syscore=0; syscore<3;++syscore) {
    for (Int_t spadic=0; spadic<3;++spadic) {
      for (Int_t halfchip=0; halfchip<2;++halfchip) {
	spadicName = RewriteSpadicName(Form("SysCore%01d_Spadic%01d", syscore, spadic));
	if(spadicName != "") {
	  histName = "Delta_t_hist_for_Syscore_"+std::to_string(syscore)+"_Spadic_"+std::to_string(spadic)+"_Half_"+std::to_string(halfchip);
	  title = histName + runName;
	  fHM->Add(histName.Data(), new TH1I(histName, title, 4096,-70000,70000));
	}
      }
    }
  }

  if(true){
	  for (Int_t syscore=0; syscore<1;++syscore)
	  {
		  for (Int_t spadic=0; spadic<2;++spadic)
		  {
			  histName = "Trigger_Bug_Investigation_Syscore_"+std::to_string(syscore)+"_Spadic_"+std::to_string(spadic);
			  title = histName + runName;
			  fHM->Create2<TH2I>(histName.Data(), title.Data(), 32,-0.5,31.5, 32,-0.5,31.5);
			  fHM->H2(histName.Data())->GetXaxis()->SetNameTitle("Ch Self triggered","Ch Self triggered");
			  fHM->H2(histName.Data())->GetYaxis()->SetNameTitle("Ch Neighbour triggered","Ch Neighbour triggered");
		  }
	  }
  }


  for (Int_t syscore=0; syscore<3;++syscore) {
    for (Int_t spadic=0; spadic<3;++spadic) {
      for (Int_t halfchip=0; halfchip<2;++halfchip) {
	spadicName = RewriteSpadicName(Form("SysCore%01d_Spadic%01d", syscore, spadic));
	if(spadicName != "") {
	   histName = "Delta_Epoch_hist_for_Syscore_"+std::to_string(syscore)+"_Spadic_"+std::to_string(spadic)+"_Half_"+std::to_string(halfchip);
	  title = histName + runName;
	  fHM->Add(histName.Data(), new TH1I(histName, title, 8301,-4150.5,4150.5));
	}
      }
    }
  }
  for (Int_t syscore=0; syscore<3;++syscore) {
    for (Int_t spadic=0; spadic<3;++spadic) {
      for (Int_t halfchip=0; halfchip<2;++halfchip) {
	spadicName = RewriteSpadicName(Form("SysCore%01d_Spadic%01d", syscore, spadic));
	if(spadicName != "") {
	  histName = "DeltaEpoch_vs_timeslice_for_Syscore_"+std::to_string(syscore)+"_Spadic_"+std::to_string(spadic)+"_Half_"+std::to_string(halfchip);
	  title = histName + runName;
	  fHM->Add(histName.Data(), new TGraph());
	  fHM->G1(histName.Data())->SetNameTitle(title,title);
	}
      }
    }
  }
  for (Int_t syscore=0; syscore<3;++syscore) {
      for (Int_t spadic=0; spadic<3;++spadic) {
          spadicName = RewriteSpadicName(Form("SysCore%01d_Spadic%01d", syscore, spadic));
          if(spadicName != "") {
              histName = "Map_of_Hits_for_Syscore_"+std::to_string(syscore)+"_Spadic_"+std::to_string(spadic);
              title = histName + runName;
              fHM->Create2<TH2I>(histName.Data(), title.Data(), 16,-0.5,15.5,2,-0.5,1.5);
              histName = "Map_of_Charge_for_Syscore_"+std::to_string(syscore)+"_Spadic_"+std::to_string(spadic);
              title = histName + runName;
			  fHM->Create2<TH2I>(histName.Data(), title.Data(), 16,-0.5,15.5,2,-0.5,1.5);
          }
      }
  }
  if(fActivateClusterizer)
	  for (Int_t syscore=0; syscore<3;++syscore) {
		  for (Int_t spadic=0; spadic<3;++spadic) {
			  spadicName = RewriteSpadicName(Form("SysCore%01d_Spadic%01d", syscore, spadic));
			  if(spadicName != "") {
				  histName = "Clustersize_for_Syscore_"+std::to_string(syscore)+"_Spadic_"+std::to_string(spadic);
				  title = histName + runName;
				  fHM->Create1<TH1I>(histName.Data(), title.Data(), 16,-0.5,15.5);
				  for (Int_t clusterSize=1; clusterSize<=16;clusterSize++){
					  histName = "Cluster("+std::to_string(clusterSize)+")_Heatmap_for_Syscore_"+std::to_string(syscore)+"_Spadic_"+std::to_string(spadic);
					  title = histName + runName;
					  fHM->Create2<TH2I>(histName.Data(), title.Data(), 330,-0.5,32.5,2,-0.5,1.5);
				  }
			  }
		  }
	  }
  if(fActivateClusterizer)
	  if(fDrawPadResponse)
  for(Int_t Size=1 ;Size <= 16; Size++)
    {
      for(Int_t Detector =0;Detector<=1;Detector++){
          TString Detectorname = (Detector == 0 ? "Frankfurt" : "Muenster");
          histName = "Pad_Response_"+ Detectorname + "_for_Clusters_of_Size_"+std::to_string(Size);
          title = histName + runName;
          fHM->Create2<TH2I>(histName.Data(), title.Data(), 1001,-(Size+0.05)*7.125,(Size+0.05)*7.125,101,-0.005,1.005/*,100,0,1000*/);
          fHM->H2(histName.Data())->GetXaxis()->SetTitle("Displacement frac{d}{mm}");
          fHM->H2(histName.Data())->GetYaxis()->SetTitle("Chargeratio frac{Q_{i}}{#sum_{k}^{ } Q_{k}}");
          //fHM->H3(histName.Data())->GetZaxis()->SetTitle("TotalCharge");
      }
    }

  for(Int_t Detector =0;Detector<=1;Detector++){
            TString Detectorname = (Detector == 0 ? "Frankfurt" : "Muenster");
            histName = "Shape_of_Rightsided_Clusters"+ Detectorname;
            title = histName + runName;
            fHM->Create2<TH2I>(histName.Data(), title.Data(), 32,-0.5,31.5,3,0.5,3.5);
            histName = "Shape_of_Leftsided_Clusters"+ Detectorname;
            title = histName + runName;
            fHM->Create2<TH2I>(histName.Data(), title.Data(), 32,-0.5,31.5,3,0.5,3.5);
        }

  for (Int_t syscore=0; syscore<1;++syscore) {
      for(Int_t Detector =0;Detector<=1;Detector++){
		  TString Detectorname = (Detector == 0 ? "Frankfurt" : "Muenster");
		  histName = "Baseline_for_Syscore_"+std::to_string(syscore)+"_Prototype_from_"+Detectorname;
		  title = histName + runName;
		  fHM->Create2<TH2I>(histName.Data(), title.Data(),512,-256.5,255.5,33,-0.5,32.5);
	  }
  }
  //Cluster Coincidences
  {
	  histName="Cluster_Coincidences";
	  title = histName + runName;
	  fHM->Create2<TH2I>(histName.Data(), title.Data(),461,-32.5,32.5,100,0,100);
  }

  if(fDrawSignalShapes){
	  for (Int_t syscore=0; syscore<1;++syscore) {
		  for(Int_t Detector =0;Detector<=1;Detector++){
			  for(Int_t ChID = 0 ; ChID < 32 ; ChID++){
				  TString Detectorname = (Detector == 0 ? "Frankfurt" : "Muenster");
				  histName = "SignalShape_for_Syscore_"+std::to_string(0)+"_Prototype_from_"+Detectorname+"_Channel_"+ std::to_string(ChID);
				  title = histName + runName;
				  fHM->Create2<TH2I>(histName.Data(), title.Data(),33,-0.5,32.5,512,-256.5,255.5);
				  fHM->H2(histName.Data())->GetXaxis()->SetNameTitle("Sample","Sample");
				  fHM->H2(histName.Data())->GetYaxis()->SetNameTitle("ADC Value","ADC Value");
			  }
		  }
	  }
  }
  if (fDrawClustertypes){
	  for (Int_t syscore=0; syscore<1;++syscore)
	  {
		  for(Int_t Detector =0;Detector<=1;Detector++)
		  {
			  TString Detectorname = (Detector == 0 ? "Frankfurt" : "Muenster");
			  histName = "Clustertypes_for_Syscore_"+std::to_string(0)+"_Prototype_from_"+Detectorname;
			  title = histName + runName;
			  fHM->Create2<TH2I>(histName.Data(), title.Data(),17,-0.5,16.5,4,-0.5,3.5);
			  fHM->H2(histName.Data())->GetXaxis()->SetNameTitle("Clustersize in Pads","Clustersize in Pads");
			  fHM->H2(histName.Data())->GetYaxis()->SetNameTitle("Clustertype","Clustertype");
		  }
	  }
  }
	if (fDrawSignalDebugHistograms) {
		for (Int_t syscore = 0; syscore < 1; ++syscore) {
			for (Int_t Detector = 0; Detector <= 1; Detector++) {
				for(Int_t TriggerType =0;TriggerType<4;TriggerType++){
				  TString Detectorname = (Detector == 0 ? "Frankfurt" : "Muenster");
				  histName = "MaxADC_vs_NrSamples_for_Syscore_"+std::to_string(syscore)+"_Prototype_from_"+Detectorname+triggerTypes[TriggerType];
				  title = histName + runName;
				  fHM->Create2<TH2I>(histName.Data(), title.Data(),33,-0.5,32.5,512,-256.5,255.5);
				  fHM->H2(histName.Data())->GetXaxis()->SetTitle("NrSamples");
				  fHM->H2(histName.Data())->GetYaxis()->SetTitle("MaxADC");
				  histName = "avgBaseline_vs_NrSamples_for_Syscore_"+std::to_string(0)+"_Prototype_from_"+Detectorname+triggerTypes[TriggerType];
				  title = histName + runName;
				  fHM->Create2<TH2I>(histName.Data(), title.Data(),33,-0.5,32.5,512,-256.5,255.5);
				  fHM->H2(histName.Data())->GetXaxis()->SetTitle("NrSamples");
				  fHM->H2(histName.Data())->GetYaxis()->SetTitle("averagedBaseline");
				  vector<string> Hittypes = {"_Hits","_MultiHits"};
				  for (string Hittype : Hittypes){
					  histName = "MaxADC_vs_avgBaseline_for_Syscore_"+std::to_string(syscore)+"_Prototype_from_"+Detectorname+Hittype+triggerTypes[TriggerType];
					  title = histName + runName;
					  fHM->Create2<TH2I>(histName.Data(), title.Data(),512,-256.5,255.5,512,-256.5,255.5);
					  fHM->H2(histName.Data())->GetXaxis()->SetTitle("averagedBaseline");
					  fHM->H2(histName.Data())->GetYaxis()->SetTitle("MaxADC");
				  }
				  histName = "avgBaseline_for_Syscore_"+std::to_string(syscore)+"_Prototype_from_"+Detectorname+triggerTypes[TriggerType];
				  title = histName + runName;
				  fHM->Create1<TH1I>(histName.Data(), title.Data(),512,-256.5,255.5);
				  fHM->H1(histName.Data())->GetXaxis()->SetTitle("averagedBaseline");
			}
			}
		  }
	  }


  fHM->Add("TsCounter", new TGraph());
  fHM->G1("TsCounter")->SetNameTitle("TsCounter","TsCounter");
  fHM->Add("TsCounterHit0", new TGraph());
  fHM->G1("TsCounterHit0")->SetNameTitle("TsCounterHit0","TsCounterHit0");
  fHM->Add("TsCounterHit1", new TGraph());
  fHM->G1("TsCounterHit1")->SetNameTitle("TsCounterHit1","TsCounterHit1");
  fHM->Add("TsCounterHitAborted0", new TGraph());
  fHM->Add("TsCounterHitAborted1", new TGraph());
  fHM->G1("TsCounterHitAborted0")->SetNameTitle("TsCounterHitAborted0","TsCounterHitAborted0");
  fHM->G1("TsCounterHitAborted1")->SetNameTitle("TsCounterHitAborted1","TsCounterHitAborted1");
  fHM->Add("TsCounterOverflow0", new TGraph());
  fHM->Add("TsCounterOverflow1", new TGraph());
  fHM->G1("TsCounterOverflow0")->SetNameTitle("TsCounterOverflow0","TsCounterOverflow0");
  fHM->G1("TsCounterOverflow1")->SetNameTitle("TsCounterOverflow1","TsCounterOverflow1");
  fHM->Add("TsLost0", new TGraph());
  fHM->Add("TsLost1", new TGraph());
  fHM->G1("TsLost0")->SetNameTitle("TsLost0","TsLost0");
  fHM->G1("TsLost1")->SetNameTitle("TsLost1","TsLost1");
  fHM->Add("TsCounterInfo0", new TGraph());
  fHM->Add("TsCounterInfo1", new TGraph());
  fHM->G1("TsCounterInfo0")->SetNameTitle("TsCounterInfo0","TsCounterInfo0");
  fHM->G1("TsCounterInfo1")->SetNameTitle("TsCounterInfo1","TsCounterInfo1");
  fHM->Add("TsCounterEpoch0", new TGraph());
  fHM->Add("TsCounterEpoch1", new TGraph());
  fHM->G1("TsCounterEpoch0")->SetNameTitle("TsCounterEpoch0","TsCounterEpoch0");
  fHM->G1("TsCounterEpoch1")->SetNameTitle("TsCounterEpoch1","TsCounterEpoch1");
  fHM->Add("TsCounterStrange0", new TGraph());
  fHM->Add("TsCounterStrange1", new TGraph());
  fHM->G1("TsCounterStrange0")->SetNameTitle("TsCounterStrange0","TsCounterStrange0");
  fHM->G1("TsCounterStrange1")->SetNameTitle("TsCounterStrange1","TsCounterStrange1");
  fHM->Add("TsStrangeness0", new TGraph()); // ratio of strange messages over all messages
  fHM->Add("TsStrangeness1", new TGraph());
  fHM->G1("TsStrangeness0")->SetNameTitle("TsStrangeness0","TsStrangeness0");
  fHM->G1("TsStrangeness1")->SetNameTitle("TsStrangeness1","TsStrangeness1");
  if(fActivateDeltaTAnalysis)
  for (Int_t SysID=0; SysID<1;++SysID){
    for (Int_t SpaID=0; SpaID<3;++SpaID){
      for (Int_t ChID=0; ChID<32;++ChID){
	fHM->Add(("Delta_t_for_Syscore_"+std::to_string(SysID)+"_Spadic_"+std::to_string(SpaID)+"_Channel_"+std::to_string(ChID)), new TGraph());
	fHM->G1(("Delta_t_for_Syscore_"+std::to_string(SysID)+"_Spadic_"+std::to_string(SpaID)+"_Channel_"+std::to_string(ChID)))->SetNameTitle(("Delta_t_for_Syscore_"+std::to_string(SysID)+"_Spadic_"+std::to_string(SpaID)).c_str(),("Delta_t_for_Syscore_"+std::to_string(SysID)+"_Spadic_"+std::to_string(SpaID)).c_str());
      }
    }
  }
  if(fActivateOffsetAnalysis)
  for (Int_t baseSpaID=0; baseSpaID<4;++baseSpaID){
    fHM->Add(("Timestamps_Spadic"+std::to_string(baseSpaID)), new TGraph());
    fHM->G1(("Timestamps_Spadic"+std::to_string(baseSpaID)))->SetNameTitle(("Timestamps_Spadic"+std::to_string(baseSpaID)).c_str(),("Timestamps_Spadic"+std::to_string(baseSpaID)).c_str());
    for (Int_t compSpaID=0; compSpaID<4;++compSpaID) {
      fHM->Add(("Time_Offset_between_Spadic_"+std::to_string(baseSpaID)+"_and_Spadic_"+std::to_string(compSpaID)), new TGraph());
      fHM->G1(("Time_Offset_between_Spadic_"+std::to_string(baseSpaID)+"_and_Spadic_"+std::to_string(compSpaID)))->SetNameTitle(("Time_Offset_between_Spadic_"+std::to_string(baseSpaID)+"_and_Spadic_"+std::to_string(compSpaID)).c_str(),("Time_Offset_between_Spadic_"+std::to_string(baseSpaID)+"_and_Spadic_"+std::to_string(compSpaID)).c_str());
    }
  }
  fHM->Add("DoubleMessage_MessageType", new TH2I("DoubleMessage_MessageType","DoubleMessage_MessageType",8,-1.5,6.5,8,-1.5,6.5));
  fHM->H2("DoubleMessage_MessageType")->GetYaxis()->SetTitle("MessageType TClonesArray");
  fHM->H2("DoubleMessage_MessageType")->GetXaxis()->SetTitle("MessageType Buffer Map");

  fHM->Add("TriggerType_vs_InfoType", new TH2I("TriggerType_vs_InfoType","TriggerType_vs_InfoType",5,-1.5,3.5,9,-1.5,7.5));
  fHM->H2("TriggerType_vs_InfoType")->GetYaxis()->SetTitle("InfoType");
  fHM->H2("TriggerType_vs_InfoType")->GetXaxis()->SetTitle("TriggerType");

  fHM->Add("TriggerType_vs_StopType", new TH2I("TriggerType_vs_StopType","TriggerType_vs_StopType",5,-1.5,3.5,7,-1.5,5.5));
  fHM->H2("TriggerType_vs_StopType")->GetYaxis()->SetTitle("StopType");
  fHM->H2("TriggerType_vs_StopType")->GetXaxis()->SetTitle("TriggerType");

  fHM->Add("InfoType_vs_StopType", new TH2I("InfoType_vs_StopType","InfoType_vs_StopType",9,-1.5,7.5,7,-1.5,5.5));
  fHM->H2("InfoType_vs_StopType")->GetYaxis()->SetTitle("StopType");
  fHM->H2("InfoType_vs_StopType")->GetXaxis()->SetTitle("InfoType");

  fHM->Add("NrSamples_vs_StopType", new TH2I("NrSamples_vs_StopType","NrSamples_vs_StopType",34,-1.5,32.5,7,-1.5,5.5));
  fHM->H2("NrSamples_vs_StopType")->GetXaxis()->SetTitle("Nr. of Samples per Message");
  fHM->H2("NrSamples_vs_StopType")->GetYaxis()->SetTitle("StopType");

  fHM->Add("NrSamples_vs_TriggerType", new TH2I("NrSamples_vs_TriggerType","NrSamples_vs_TriggerType",34,-1.5,32.5,5,-1.5,3.5));
  fHM->H2("NrSamples_vs_TriggerType")->GetXaxis()->SetTitle("Nr. of Samples per Message");
  fHM->H2("NrSamples_vs_TriggerType")->GetYaxis()->SetTitle("TriggerType");

  fHM->Add("NrSamples_vs_InfoType", new TH2I("NrSamples_vs_InfoType","NrSamples_vs_InfoType",34,-1.5,32.5,9,-1.5,7.5));
  fHM->H2("NrSamples_vs_InfoType")->GetXaxis()->SetTitle("Nr. of Samples per Message");
  fHM->H2("NrSamples_vs_InfoType")->GetYaxis()->SetTitle("InfoType");

  fHM->Add("InfoType_vs_Channel", new TH2I("InfoType_vs_Channel","InfoType_vs_Channel",35,-1.5,33.5,10,0.5,10.5));
  fHM->H2("InfoType_vs_Channel")->GetXaxis()->SetTitle("Channel");
  fHM->H2("InfoType_vs_Channel")->GetYaxis()->SetTitle("Info or Type");

  fHM->Add("Hit_Coincidences", new TH2I("Hit_Coincidences","Hit_Coincidences",64,-0.5,63.5,64,-0.5,63.5));
  fHM->H2("Hit_Coincidences")->GetXaxis()->SetTitle("SpadicChannel");
  fHM->H2("Hit_Coincidences")->GetYaxis()->SetTitle("SpadicChannel");

  fHM->Add("Correlation_Map", new TH2I("Correlation_Map","Correlation_Map",63,-31.5,31.5,21,-0.5,20.5));
  fHM->H2("Correlation_Map")->GetXaxis()->SetTitle("#Delta Pad");
  fHM->H2("Correlation_Map")->GetYaxis()->SetTitle("#Delta Time (1 timestamp = 57 ns)");

}
// ----              -------------------------------------------------------
 Int_t CbmTrdTimeCorrel::GetModuleID(CbmSpadicRawMessage* raw)
  {
    if (raw == NULL){
      LOG(ERROR) << "CbmTrdRawBeamProfile::GetModuleID: CbmSpadicRawMessage == NULL" << FairLogger::endl;
      return -1;
    } else {
      return GetLayerID(raw);
    }
  }
 // ----              -------------------------------------------------------
void CbmTrdTimeCorrel::FitBaseline() {
	if (!fCalculateBaseline) {
		return;
	}
	TString outfile = FairRootManager::Instance()->GetOutFile()->GetName();
	outfile.ReplaceAll(".root", "_Calibration.root");
	TFile CalibrationFile(outfile, "RECREATE");
	CalibrationFile.cd();
	vector<TFitResult> FitResults;
	//TODO: Refine fits based on previous iterations
	//DONE: Check for Rootfile of fits and import them
	for (Int_t Detector = 0; Detector <= NrOfSpadics / 2; Detector++) {
		string Detectorname = (Detector == 0 ? "Frankfurt" : "Muenster");
		string histName = "Baseline_for_Syscore_" + std::to_string(0)
				+ "_Prototype_from_" + Detectorname;
		TH2* BaselineHistogram2D = fHM->H2(histName);
		for (Int_t ChannelID = 0; ChannelID < 32; ChannelID++) {
			TH1D* BaselineProjection = BaselineHistogram2D->ProjectionX("_px",
					ChannelID + 1, ChannelID + 1);
			if (BaselineProjection->GetEntries() < 50)
				continue;
			TFitResultPtr Baselinefit = BaselineProjection->Fit("gaus", "MQS", "",
					-253, -10);
			TString fitname = "Fit_Baseline_for_Syscore_"
			    + std::to_string (0) + "_Spadic_"
			    + std::to_string (Detector*2 + ChannelID / 16) + "_Channel_"
			    + std::to_string (ChannelID % 16);
			Baselinefit->SetName(fitname);
			FitResults.push_back(*Baselinefit);
			//Baselinefit->Print("V ");
			std::cout << std::endl;
		}
		std::cout << "Fits " << Detectorname << " " << FitResults.size()
				<< std::endl;
	}
	for (auto x : FitResults) {
		x.Write();
	}
	FairRootManager::Instance()->GetOutFile()->cd();
	CalibrationFile.Close();
}

// ----              -------------------------------------------------------
void CbmTrdTimeCorrel::FitPRF() {
	TString outfile = FairRootManager::Instance()->GetOutFile()->GetName();
	FairRootManager::Instance()->GetOutFile()->cd();
	vector<TFitResult> FitResults;
	auto PRFLambda =[&](Double_t Displacement, Double_t * Parameters)
        {
          Double_t d = Displacement;
          Double_t K3 = Parameters[0];
          Double_t A = Parameters[2];
          Double_t SqrtK3 = sqrt(K3);
          Double_t W=Parameters[3];
          Double_t h = Parameters[1];

          return std::fabs(
              - A / (2. * atan(SqrtK3)) * (
                  std::atan(SqrtK3 * std::tanh(TMath::Pi() * (-2. + SqrtK3 ) * (W + 2.* d) / (8.* h) )) +
                  std::atan(SqrtK3 * std::tanh(TMath::Pi() * (-2. + SqrtK3 ) * (W - 2.* d) / (8.* h) ))
              )
          );
        };
	TF1* PRF =
	    new TF1 (
	        "PRF",[&PRFLambda](double*x, double *p){ return PRFLambda(x[0],p);}
	        ,-10.0, 10.0, 4);
	//TODO: Check for Rootfile of fits
	for(Int_t size=3;size<=4;size++)
	for (Int_t Detector = 0; Detector <= NrOfSpadics / 2; Detector++) {
		string Detectorname = (Detector == 0 ? "Frankfurt" : "Muenster");
		string histName = "Pad_Response_" + Detectorname
				+ "_for_Clusters_of_Size_" + std::to_string(size);
		TH2* PRFHist = fHM->H2(histName);
		if (PRFHist->GetEntries() < 50)
			continue;
		TString ProfileName ="Profile_"+histName;
                TCanvas *c1 = new TCanvas("c1","",1600,600);
		TProfile* PRFProfile = PRFHist->ProfileX(ProfileName.Data(),1,-1);
		PRFProfile->Draw("");
		PRFProfile->Write();
	        PRF->SetParameters(0.1,3.5,1,7.125);
	        PRF->SetParName(0,"K3");
                //PRF->FixParameter(0,0.92);
	        //PRF->SetParLimits(0,0.78,0.92);
	        PRF->SetParName(1,"h");
	        PRF->FixParameter(1,3.5);
	        PRF->SetParName(2,"A");
	        PRF->FixParameter(2,1.0);
	        //PRF->SetParLimits(2,0.99999,1.00001);
	        PRF->SetParName(3,"W");
	        PRF->FixParameter(3,7.125);
		PRF->Draw();
                c1->Update();
                TString fitname = "Fit_PRF_" + histName;
                c1->SaveAs("pics/Pre_"+fitname+".pdf");
                c1->SaveAs("pics/Pre_"+fitname+".png");
                c1->Clear();
                PRFProfile->Draw("colz");
		TFitResultPtr PRFFit = PRFProfile->Fit(PRF, "S", "", -10.0, 10.0);
		PRFFit->SetName(fitname);
		FitResults.push_back(*PRFFit);
		PRFFit->Print("V");
		c1->Update();
		c1->SaveAs("pics/"+fitname+".pdf");
                c1->SaveAs("pics/"+fitname+".png");
                delete c1;
		std::cout << std::endl;
		std::cout << "Fits " << Detectorname << " " << FitResults.size()
				<< std::endl;
	}
        outfile.ReplaceAll(".root", "_Calibration.root");
        TFile CalibrationFile(outfile, "UPDATE");
        CalibrationFile.cd();
	for (auto x : FitResults) {
		x.Write();
	}
	FairRootManager::Instance()->GetOutFile()->cd();
	CalibrationFile.Close();
}
// ----              -------------------------------------------------------
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
    LOG(ERROR) << "Source Address " << sourceA << " not known." << FairLogger::endl;
    break;
  }
  return SpaId;
}
// ----              -------------------------------------------------------
TString CbmTrdTimeCorrel::GetStopName(Int_t stopType)
{
  TString stopName="";
  switch (stopType) {
  case (-1):
    stopName="Info or epoch mess";
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
    LOG(ERROR) << "stopType " << stopType << " not known." << FairLogger::endl;
    break;
  }
  return stopName;
}

TString CbmTrdTimeCorrel::GetSpadicName(Int_t eqID,Int_t sourceA)
{
  TString spadicName="";
  
  switch (eqID) {
  case kFlesMuenster:
    spadicName="SysCore0_";
    break;
  case kFlesFrankfurt:
    spadicName="SysCore1_";
    break;
  case kFlesBucarest:
    spadicName="SysCore2_";
    break;
  default:
    LOG(ERROR) << "EquipmentID " << eqID << " not known." << FairLogger::endl;
    break;
  }
  
  switch (sourceA) {
  case (SpadicBaseAddress+0):  // first spadic
  case (SpadicBaseAddress+1):  // first spadic
    spadicName+="Spadic0";
    break;
  case (SpadicBaseAddress+2):  // second spadic
  case (SpadicBaseAddress+3):  // second spadic
    spadicName+="Spadic1";
    break;
  case (SpadicBaseAddress+4):  // third spadic
  case (SpadicBaseAddress+5):  // third spadic
    spadicName+="Spadic2";
    break;
  default:
    LOG(ERROR) << "Source Address " << sourceA << " not known." << FairLogger::endl;
    break;
  }
  
  spadicName = RewriteSpadicName(spadicName);
  
  return spadicName;
}

TString CbmTrdTimeCorrel::RewriteSpadicName(TString spadicName) 
{
  if(spadicName=="SysCore0_Spadic0") {
    if(fRewriteSpadicName) spadicName="Frankfurt";
  }else if(spadicName=="SysCore0_Spadic1"){
    if(fRewriteSpadicName) spadicName="Muenster";
  }else{
    spadicName="";
  }
  
  return spadicName;
}

// ----              -------------------------------------------------------
  Int_t CbmTrdTimeCorrel::GetChannelOnPadPlane(Int_t SpadicChannel)
  {
    Int_t channelMapping[32] = {31,15,30,14,29,13,28,12,27,11,26,10,25, 9,24, 8,
				23, 7,22, 6,21, 5,20, 4,19, 3,18, 2,17, 1,16, 0};
    if (SpadicChannel < 0 || SpadicChannel > 31){
      if (SpadicChannel !=-1) LOG(ERROR) << "CbmTrdTimeCorrel::GetChannelOnPadPlane ChId " << SpadicChannel << FairLogger::endl;
      return -1;
    } else {
      return channelMapping[SpadicChannel];
    }
  }

//---------------------------------------------------------------------------
  Int_t  CbmTrdTimeCorrel::GetSectorID(CbmSpadicRawMessage* raw)// To be used to create CbmTrdDigis
  {
    return 0;
  }
//---------------------------------------------------------------------------
Int_t  CbmTrdTimeCorrel::GetRowID(CbmSpadicRawMessage* raw)// To be used to create CbmTrdDigis
{
  Int_t sourceA = raw->GetSourceAddress();
  Int_t chID = raw->GetChannelID();
  Int_t spaID = GetSpadicID(sourceA);
  if(chID > -1 && chID < 16 && spaID%2==1) chID+=16; // eqID ?
  Int_t padID = GetChannelOnPadPlane(chID);// Remapping from ASIC to pad-plane channel numbers.
  if (padID > 16) 
    return 1;
  else
    return 0;
}
//---------------------------------------------------------------------------
Int_t  CbmTrdTimeCorrel::GetLayerID(CbmSpadicRawMessage* raw)// To be used to create CbmTrdDigis
{
  TString name = GetSpadicName(raw->GetEquipmentID(), raw->GetSourceAddress());
  if (name == "Frankfurt")
    return 1;
  else if (name == "Muenster")
    return 2;
  else
    return -1;
}
//---------------------------------------------------------------------------
Int_t  CbmTrdTimeCorrel::GetColumnID(CbmSpadicRawMessage* raw)// To be used to create CbmTrdDigis
{
  Int_t sourceA = raw->GetSourceAddress();
  Int_t chID = raw->GetChannelID();
  Int_t spaID = GetSpadicID(sourceA);
  if(chID > -1 && chID < 16 && spaID%2==1) chID+=16; // eqID ?
 
  Int_t columnId = GetChannelOnPadPlane(chID);
  if (columnId >= 16) 
    columnId -= 16;
  return columnId;
}
//---------------------------------------------------------------------------
std::map<Int_t, std::map<Int_t,std::map<ULong_t, Long_t> > > CbmTrdTimeCorrel::CalculateTimestampOffsets(const EpochMap &epochBuffer)
{
  //Calculate time offsets between various Spadics
  std::map<Int_t, std::map<Int_t,std::map<ULong_t, Long_t > > > epochOffsets;
  //loop over all SpaIDs in the epochBuffer for base timestamps
  for (auto baseSpaIDIt = epochBuffer.begin() ; baseSpaIDIt != epochBuffer.end(); ++baseSpaIDIt)
    //loop over all SpaIDs in the epochBuffer for comparing timestamps
    for (auto compSpaIDIt = epochBuffer.begin() ; compSpaIDIt != epochBuffer.end(); ++compSpaIDIt){
      //Retrieve SpadicID's
      Int_t baseSource = baseSpaIDIt->second.begin()->second->GetSourceAddress();
      Int_t baseSpaID = GetSpadicID(baseSource);
      Int_t compSource = compSpaIDIt->second.begin()->second->GetSourceAddress();
      Int_t compSpaID = GetSpadicID(compSource);
      //loop over all EpochMessages in the epochBuffer for comparing timestamps
      auto baseTimestampIt = baseSpaIDIt->second.begin();
      auto compTimestampIt = compSpaIDIt->second.begin();
      for ((baseTimestampIt = baseSpaIDIt->second.begin(),compTimestampIt = compSpaIDIt->second.begin());baseTimestampIt != baseSpaIDIt->second.end(); ++baseTimestampIt, ++compTimestampIt)
	{
	  //If there are no more messages in the comparing buffer set the difference to the last known value
	  if(compTimestampIt == compSpaIDIt->second.end())
	    {
	      for (auto innerIt = baseTimestampIt; innerIt != baseSpaIDIt->second.end(); ++innerIt)
		{
		  innerIt--;
		  ULong_t previousTimestamp = (innerIt++)->second->GetFullTime();
		  ULong_t baseTimestamp = innerIt->second->GetFullTime();
		  epochOffsets[baseSpaID][compSpaID][baseTimestamp] = epochOffsets[baseSpaID][compSpaID][previousTimestamp];
		}
	      break;
	    } else{
	    ULong_t baseTimestamp = baseTimestampIt->second->GetFullTime();
	    ULong_t compTimestamp = compTimestampIt->second->GetFullTime();
	    epochOffsets[baseSpaID][compSpaID][baseTimestamp]=static_cast<Long_t>(compTimestamp-baseTimestamp);
	  }
	}
    }
  return epochOffsets;
}


CbmTrdTimeCorrel::Cluster::Cluster(Int_t * BaselineArray,Int_t initWindowsize, Int_t ChargeThreshhold = 50) :
    TObject (),
    fEntries (),
    fParametersCalculated(false),
    fSpadic (0),
    fRow(0),
    fType (0),
    fBaseline(BaselineArray),
    fTotalCharge (0),
    fHorizontalPosition (0),
    fWindowsize(initWindowsize),
	fPreCalculatedBaseline(true),
	fClusterChargeThreshhold(ChargeThreshhold),
	fFullTime()
{};

CbmTrdTimeCorrel::Cluster::~Cluster(){};

Bool_t CbmTrdTimeCorrel::Cluster::AddEntry(CbmSpadicRawMessage NewEntry){
  auto CompareSpadicMessages=
      [&](CbmSpadicRawMessage a,CbmSpadicRawMessage b)
      {
        Int_t ChIDA = a.GetChannelID() + ((GetSpadicID(a.GetSourceAddress()) %2 == 1)? 16 : 0);
        Int_t ChIDB = b.GetChannelID() + ((GetSpadicID(b.GetSourceAddress()) %2 == 1)? 16 : 0);
        Int_t PadA = GetChannelOnPadPlane(ChIDA);
        Int_t PadB = GetChannelOnPadPlane(ChIDB);
        Int_t rowA = (GetSpadicID(a.GetSourceAddress())/2)+PadA/16;
        Int_t rowB = (GetSpadicID(b.GetSourceAddress())/2)+PadB/16;
        if(a.GetFullTime() == b.GetFullTime())
          if(rowA==rowB)
            if(PadA==PadB)
              return true;
        return false;
      };
  auto CompareSpadicMessagesSmaller=
      [&](CbmSpadicRawMessage a,CbmSpadicRawMessage b)
      {
        Int_t ChIDA = a.GetChannelID() + ((GetSpadicID(a.GetSourceAddress()) %2 == 1)? 16 : 0);
        Int_t ChIDB = b.GetChannelID() + ((GetSpadicID(b.GetSourceAddress()) %2 == 1)? 16 : 0);
        Int_t PadA = GetChannelOnPadPlane(ChIDA);
        Int_t PadB = GetChannelOnPadPlane(ChIDB);
        Int_t rowA = (GetSpadicID(a.GetSourceAddress())/2)+PadA/16;
        Int_t rowB = (GetSpadicID(b.GetSourceAddress())/2)+PadB/16;
        if(rowA<rowB)
          return true;
        else if(rowA==rowB)
          if(PadA<PadB)
            return true;
        return false;
      };

  fParametersCalculated = false;
  fEntries.push_back(NewEntry);
  std::sort(fEntries.begin(),fEntries.end(),CompareSpadicMessagesSmaller);
  std::unique(fEntries.begin(),fEntries.end(),CompareSpadicMessages);
  fSpadic = GetSpadicID(fEntries.begin()->GetSourceAddress());
  fRow = GetHorizontalMessagePosition(*(fEntries.begin()))/16;
//GetChannelOnPadPlane(fEntries.begin()->GetChannelID() + ((GetSpadicID(fEntries.begin()->GetSourceAddress()) %2 == 1)? 16 : 0))/16;
  BeforeLoop:
  if (fIs2D) {
	  for (auto it = fEntries.begin(); it != fEntries.end(); it++) {
		  if (((GetSpadicID(it->GetSourceAddress()) / 2) != fSpadic / 2)) {
			  fEntries.erase(it);
			  goto BeforeLoop;
		  }
	  }
  } else {
	  for (auto it = fEntries.begin(); it != fEntries.end(); it++) {
		  if (((GetSpadicID(it->GetSourceAddress()) / 2) != fSpadic / 2)
				  || GetHorizontalMessagePosition(*it) / 16 != fRow) {
			  fEntries.erase(it);
			  goto BeforeLoop;
		  }
	  }
  }//End of goto scope
  return true;
}

Int_t CbmTrdTimeCorrel::Cluster::GetTotalCharge() {
	if (fParametersCalculated) {
		return fTotalCharge;
	} else {
		CalculateParameters();
		return fTotalCharge;
	}
}

Bool_t CbmTrdTimeCorrel::Cluster::FillChargeDistribution(TH2* ChargeMap)
{
  if (!fParametersCalculated)
    CalculateParameters();
  if(fMaxStopType>0)return false;
  for (auto currentMessage : fEntries )
    {
      Int_t Charge = GetMaxADC(currentMessage);
      Float_t ChargeRatio =  static_cast<Float_t>(Charge)/static_cast<Float_t>(GetTotalCharge());
      Float_t Displacement = static_cast<Float_t>(GetHorizontalMessagePosition(currentMessage));
      Displacement -= GetHorizontalPosition();
      Displacement *= 7.125;
      ChargeMap->Fill(Displacement,ChargeRatio/*,fTotalCharge*/);
  }
  return true;
}

Int_t CbmTrdTimeCorrel::Cluster::GetMaxADC(CbmSpadicRawMessage& message)
{
  Int_t maxADC=-255;
Bool_t validHit=false;
Int_t previousADC=-255;
Int_t NrSamples = message.GetNrSamples ();
 for (Int_t i = 0 ; i < NrSamples ; i++){
     Int_t currentADC = *(message.GetSamples() + i);
     if(currentADC > maxADC) maxADC=currentADC;
     if((currentADC > previousADC) && ((currentADC - previousADC)>10)) validHit = true;
 }
 Int_t Baseline=0;
 if(!fPreCalculatedBaseline){
 for (Int_t i = NrSamples -3 ; i < NrSamples ; i++)
   Baseline += *(message.GetSamples() + i);
 Baseline = Baseline/3;
 }
 else{
	 Baseline = fBaseline[0*64+16*GetSpadicID(message.GetSourceAddress())+message.GetChannelID()];
 }
 return (maxADC-Baseline);
};

Int_t CbmTrdTimeCorrel::GetMaxADC(CbmSpadicRawMessage& message,Bool_t SubtractBaseline)
{
  Int_t maxADC=-255;
  Int_t previousADC=-255;
  Int_t NrSamples = message.GetNrSamples ();
  Bool_t validHit=(NrSamples==32);
  Int_t Spadic = GetSpadicID(message.GetSourceAddress());
  for (Int_t i = 0 ; i < NrSamples ; i++){
      Int_t currentADC = *(message.GetSamples() + i);
      if(currentADC > maxADC) maxADC=currentADC;
      if((currentADC > previousADC) && ((currentADC - previousADC)>10)) validHit = true;
      previousADC = currentADC;
  }
  //Int_t Baseline=GetAvgBaseline(message);
  return (SubtractBaseline ?  maxADC-fBaseline[0*64+16*GetSpadicID(message.GetSourceAddress())+message.GetChannelID()] : maxADC);
};

Int_t CbmTrdTimeCorrel::GetAvgBaseline(CbmSpadicRawMessage& message,Int_t n){
	  Int_t NrSamples = message.GetNrSamples ();
	  if(NrSamples==0||n==0) return -255;
	  if(NrSamples<n) n = NrSamples;
	  Int_t Baseline=0;
	  for (Int_t i = NrSamples -n ; i < NrSamples ; i++)
	    Baseline += *(message.GetSamples() + i);
	  Baseline /= n;
	  return Baseline;
};


void CbmTrdTimeCorrel::FillBaselineHistogram(CbmSpadicRawMessage* message){
	string histName;
	Int_t Syscore = 0;
	Int_t Spadic = GetSpadicID(message->GetSourceAddress())/2;
	string Detectorname = (Spadic == 0 ? "Frankfurt" : "Muenster");
	histName = "Baseline_for_Syscore_"+std::to_string(0)+"_Prototype_from_"+Detectorname;
	Int_t StopType = message->GetStopType();
	if (StopType > 0) return;
	if (GetMaxADC(*message,false)>0) return;
	TH2* Histogram = fHM->H2(histName);
	Int_t ChID =  message->GetChannelID() + ((GetSpadicID(message->GetSourceAddress()) %2 == 1)? 16 : 0);
	Int_t NrSamples = message->GetNrSamples ();
	Int_t * Samples=message->GetSamples();
	Histogram->Fill(GetAvgBaseline(*message),ChID);

};

void CbmTrdTimeCorrel::FillSignalShape(CbmSpadicRawMessage& message,string Hist,Bool_t HighBaseline){
	Int_t SpadicID = GetSpadicID(message.GetSourceAddress());
	string Detectorname = ((SpadicID/2) == 0 ? "Frankfurt" : "Muenster");
	Int_t ChID = message.GetChannelID() + (SpadicID%2)*16;
	string histName=Hist;
	if(Hist=="") histName = "SignalShape_for_Syscore_"+std::to_string(0)+"_Prototype_from_"+Detectorname+"_Channel_"+ std::to_string(ChID);
	Int_t NrSamples = message.GetNrSamples();
	Int_t * Samples = message.GetSamples();
	TH2* Histogram = nullptr;
	if(HighBaseline){
		Histogram = fBaselineHM->H2(histName);
	}else{
		Histogram = fHM->H2(histName);
	}
	for (Int_t i = 0 ; i < NrSamples ; i++){
		Histogram->Fill(i,*(Samples+ i));
	}
}

std::pair<std::vector<CbmSpadicRawMessage>::iterator,std::vector<CbmSpadicRawMessage>::iterator> CbmTrdTimeCorrel::Cluster::GetEntries()
{
  return std::make_pair(fEntries.begin(),fEntries.end());
}

Int_t CbmTrdTimeCorrel::Cluster::GetSpadic(){
  if(fParametersCalculated) return fSpadic;
  else {
      CalculateParameters();
      return fSpadic;
  }
}

Int_t CbmTrdTimeCorrel::Cluster::GetRow(){
  if(fParametersCalculated) return (static_cast<Int_t>(fHorizontalPosition)/16);
  else {
      CalculateParameters();
      return (static_cast<Int_t>(fHorizontalPosition)/16);
  }
}

Bool_t CbmTrdTimeCorrel::Cluster::Get2DStatus(){
	if(fParametersCalculated) return fIs2D;
	else {
		CalculateParameters();
		return fIs2D;
	}
}

ULong_t CbmTrdTimeCorrel::Cluster::GetFulltime(){
	if(fParametersCalculated) return fFullTime;
	else {
		CalculateParameters();
		return fFullTime;
	}
}


Float_t CbmTrdTimeCorrel::Cluster::GetHorizontalPosition(){
  if(fParametersCalculated) return fHorizontalPosition;
  else {
      CalculateParameters();
      return fHorizontalPosition;
  }
}

Int_t CbmTrdTimeCorrel::Cluster::GetHorizontalMessagePosition(CbmSpadicRawMessage& Message){
  Int_t Position= GetChannelOnPadPlane(Message.GetChannelID()+((GetSpadicID(Message.GetSourceAddress())%2 == 1) ? 16 : 0 ));
  return Position;
}

Int_t CbmTrdTimeCorrel::Cluster::Type(){
	/* Get Information about the status of the Cluster
	 * Type 0 means that the cluster is complete (no gaps and capping TriggerType 2 messages)
	 * Type 1 means that the cluster is incomplete (either with gaps or without capping TriggerType 2 messages)
	 * Type 2 means that the cluster is hitless (no selftriggerd HitMessages)
	 * Type 3 means that the Cluster is invalid (see Veto())
	 */
  if(fParametersCalculated) return fType;
  else {
      CalculateParameters();
      return fType;
  }
}

void CbmTrdTimeCorrel::Cluster::Veto() {
	//Abort Conditions
	if (fType > 0 || size() <= 2)
		return;
	//secondly veto based on total charge
	if (fTotalCharge <= fClusterChargeThreshhold) {
		//std::cout << " Veto based on Threshold" << std::endl;
		fType = 3;
		return;
	}
	//Lastly Veto based on Charge Distribution.
	Float_t VetoThreshhold = 100.0 / (size() - 0.5);

	for (auto x : fEntries) {
		Float_t ChargeRatio = 100.0 * static_cast<Float_t>(GetMaxADC(x))
				/ fTotalCharge;
		Float_t Displacement =
				static_cast<Float_t>(GetHorizontalMessagePosition(x))
						- fHorizontalPosition;
		if(x.GetTriggerType()==1||x.GetTriggerType()==3){
			if(ChargeRatio<VetoThreshhold)
			{
				fType=3;
			}

		}
		/*
		if(x.GetTriggerType()==2)
        if(x.GetTriggerType()==1||x.GetTriggerType()==3){
			if (ChargeRatio<VetoThreshhold) fType=3;
		}
		continue;
		if (std::abs(Displacement) > (size() / 2.0)) {
			//std::cout << " Veto based on width" << std::endl;
			fType = 3;
			return;
		} else if ((x.GetTriggerType() ==1)||(x.GetTriggerType()==3))
		if (std::abs(Displacement)
				<= static_cast<Float_t>((size() - 2.0) / 2.0)) {
			if ((ChargeRatio < VetoThreshhold)) {
				//std::cout << "Veto based on Distribution " << ChargeRatio << " " << VetoThreshhold << " " <<Displacement<< " " <<std::abs(Displacement)<< " " <<  static_cast<Float_t>((size()-2.0)/2.0) <<  std::endl;
				fType = 3;
				return;
			}
		} else if (std::abs(Displacement)
				>= static_cast<Float_t>((size() - 2.0) / 2.0)) {
			if (ChargeRatio >= VetoThreshhold) {
				//std::cout << "Veto based on Distribution " << ChargeRatio << " " << VetoThreshhold << " " <<Displacement<< " " <<std::abs(Displacement)<< " " <<  static_cast<Float_t>((size()-2.0)/2.0) <<  std::endl;
				fType = 3;
				return;
			}
		}*/
	}
}

std::pair<Int_t,Float_t> CbmTrdTimeCorrel::Cluster::GetPosition(){
  Int_t Detector = GetSpadic()/2;
  Float_t HorPos = GetHorizontalPosition();
  return std::make_pair(Detector,HorPos);
}

Size_t CbmTrdTimeCorrel::Cluster::size(){
  return fEntries.size();
}

void CbmTrdTimeCorrel::Cluster::CalculateParameters(){
  fSpadic = GetSpadicID(fEntries.begin()->GetSourceAddress());//GetChannelOnPadPlane(fEntries.begin()->GetChannelID() + ((GetSpadicID(fEntries.begin()->GetSourceAddress()) %2 == 1)? 16 : 0))/16;
  fType = 0;
  fTotalCharge = 0;
  fHorizontalPosition = 0;
  fFullTime = fEntries.begin()->GetFullTime();
  fIs2D=false;
  std::vector<Int_t> unweightedPosSum;
  std::vector<Int_t> Charges;
  Int_t NumberOfTypeTwoMessages=0;
  Int_t NumberOfHits=0;
  Int_t LastPad= GetHorizontalMessagePosition(*fEntries.begin())-1;
  Int_t maxADC = -255;
  fMaxStopType=0;
  for(auto message : fEntries){
	  Int_t CurrentPad = GetHorizontalMessagePosition(message);
      if (LastPad+1 != CurrentPad&&fType!=3)
	{
    	  //std::cout << LastPad << " " << CurrentPad << std:: endl;
	  fType=1;
	}
	  LastPad = CurrentPad;
      Int_t Charge = GetMaxADC(message);
      if(maxADC < Charge) maxADC = Charge;
      if(message.GetStopType()>fMaxStopType)fMaxStopType=message.GetStopType();
      if(Charge < 0){
    	  fType = 3;
      }
      Charges.push_back(Charge);
      fTotalCharge += Charge;
      fMaxCharge=maxADC;
      unweightedPosSum.push_back(CurrentPad);
      if(message.GetTriggerType()==2) NumberOfTypeTwoMessages++;
      if(message.GetTriggerType()==1||message.GetTriggerType()==3) NumberOfHits++;
  }
  if(NumberOfTypeTwoMessages!=2) fType=1;
  if(NumberOfHits==0) fType=2;
  const Float_t PadWidth = 7.0/7.125;
  const Float_t sigma = 0.646432;
  if (size()<3||size()>4||fType == 3/*||(size()==2&&!(fType==1||fType==2))*/){
	  for (Int_t i=0;i<fEntries.size();i++){
		  Double_t Weight = static_cast<Double_t>(Charges.at(i))/static_cast<Double_t>(fTotalCharge);
		  fHorizontalPosition += static_cast<Float_t>(Weight* static_cast<Double_t>(unweightedPosSum.at(i)));
	  }
  }/*else if(size()==2){//For the special Case of size 2 Clusters reconstruction formulas for both Energy and displacement are given in [Bergmann2014]
	  if(fType==1){
		  //First identify if the cluster is left sided or right sided
		  Bool_t IsLeftsided=false; //Leftsided is shorthand for the following Trigger Type composition: 2;(1|3).
		  IsLeftsided = (fEntries.at(0).GetTriggerType()==2 &&
				  	 (fEntries.at(1).GetTriggerType()==1||fEntries.at(1).GetTriggerType()==3));
		  Bool_t IsRightsided=false; //Leftsided is shorthand for the following Trigger Type composition: 2;(1|3).
		  IsRightsided = ((fEntries.at(0).GetTriggerType()==1||fEntries.at(0).GetTriggerType()==3) &&
				  	 fEntries.at(1).GetTriggerType()==2);
		  if (IsLeftsided) {
				Float_t Displacement = (sigma * sigma) / PadWidth
						* log( static_cast<Double_t>(Charges.at(1))
										/ Charges.at(0)) - PadWidth / 2;
				fHorizontalPosition = Displacement + GetHorizontalMessagePosition(fEntries.at(1));

		  } else if(IsRightsided){
				Float_t Displacement = (sigma * sigma) / PadWidth
						* log( static_cast<Double_t>(Charges.at(1))
										/ Charges.at(0)) + PadWidth / 2;
				fHorizontalPosition = Displacement + GetHorizontalMessagePosition(fEntries.at(0));
		  }
	  }else if(fType==2){
		  Float_t a = -0.19;
			Float_t Displacement = a
					* log(static_cast<Double_t>(Charges.at(0)) / Charges.at(1))
					+ PadWidth / 2;
			fHorizontalPosition = Displacement + (GetHorizontalMessagePosition(fEntries.at(0))+ GetHorizontalMessagePosition(fEntries.at(1)))/2;
	  }
  }*/
  else if(size()==3){
		fHorizontalPosition = PadWidth / 2.0
				* log(static_cast<Double_t>(Charges.at(2)) / static_cast<Double_t>(Charges.at(0)))
				/ log(static_cast<Double_t>(Charges.at(1)*Charges.at(1))
								/ static_cast<Double_t>(Charges.at(0)
								* Charges.at(2)));
	  fHorizontalPosition += GetHorizontalMessagePosition(fEntries.at(1));
	  //Float_t NormalizationFactor = 1.0/(Charges.at(0)*Charges.at(0)+Charges.at(1)*Charges.at(1));
	  //Float_t LeftDisplacement=
  }
  else if (size()==4){
    fHorizontalPosition = PadWidth * 2.0
				* log(static_cast<Double_t>(Charges.at(2)) / static_cast<Double_t>(Charges.at(1)))
				/ (log(static_cast<Double_t>(Charges.at(1))
								/ static_cast<Double_t>(Charges.at(3)))
				+ log(static_cast<Double_t>(Charges.at(2))
								/ static_cast<Double_t>(Charges.at(0))));
	  fHorizontalPosition += static_cast<Double_t>(GetHorizontalMessagePosition(fEntries.at(1))
				+GetHorizontalMessagePosition(fEntries.at(2)))/2.0;
  }

  //fHorizontalPosition=fHorizontalPosition/(size())+Offset;
  Veto();
  fParametersCalculated =true;
}

Int_t CbmTrdTimeCorrel::Cluster::GetChannelOnPadPlane(Int_t SpadicChannel)
{
  Int_t channelMapping[32] = {31,15,30,14,29,13,28,12,27,11,26,10,25, 9,24, 8,
				23, 7,22, 6,21, 5,20, 4,19, 3,18, 2,17, 1,16, 0};
  if (SpadicChannel < 0 || SpadicChannel > 31){
    if (SpadicChannel !=-1) LOG(ERROR) << "CbmTrdTimeCorrel::GetChannelOnPadPlane ChId " << SpadicChannel << FairLogger::endl;
    return -1;
  } else {
    return channelMapping[SpadicChannel];
  }
}
Int_t CbmTrdTimeCorrel::Cluster::GetSpadicID(Int_t sourceA)
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
    LOG(ERROR) << "Source Address " << sourceA << " not known." << FairLogger::endl;
    break;
  }
  return SpaId;
}

;
