#include "CbmTrdTimeCorrel.h"
#include "CbmSpadicRawMessage.h"
#include "CbmHistManager.h"
#include "CbmBeamDefaults.h"
#include "CbmTrdCluster.h"
#include "CbmTrdDigi.h"

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
// ----              -------------------------------------------------------
CbmTrdTimeCorrel::CbmTrdTimeCorrel()
  : FairTask("CbmTrdTimeCorrel"),
    fRawSpadic(NULL),
    fHM(new CbmHistManager()),
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
    timestampOffsets()
{
 LOG(DEBUG) << "Default constructor of CbmTrdTimeCorrel" << FairLogger::endl;
 for (Int_t SysID =0; SysID<3;++SysID)
  for (Int_t SpaID =0; SpaID<3;++SpaID)
   for (Int_t ChID =0; ChID<32;++ChID)
	fLastMessageTime[SysID][SpaID][ChID]=0;
}
// ----              -------------------------------------------------------
CbmTrdTimeCorrel::~CbmTrdTimeCorrel()
{
  LOG(DEBUG) << "Destructor of CbmTrdTimeCorrel" << FairLogger::endl;
  fRawMessages->Clear("C");
  fDigis->Clear("C");
  fClusters->Clear("C");
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

  fRawMessages = new TClonesArray("CbmSpadicRawMessage", 100);
  ioman->Register("TrdRawMessage", "TRD Raw Messages", fRawMessages, kTRUE);

  fDigis = new TClonesArray("CbmTrdDigi", 100);
  ioman->Register("TrdDigi", "TRD Digis", fDigis, kTRUE);

  fClusters = new TClonesArray("CbmTrdCluster",100);
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
  Int_t nSpadicMessages = fRawSpadic->GetEntriesFast(); //SPADIC messages per TimeSlice
  Int_t nSpadicMessages0(0),nSpadicMessages1(0); //SPADIC messages per TimeSlice for single SPADICS
  Int_t nSpadicMessagesHit0(0), nSpadicMessagesHtimeIt(0), nSpadicMessagesHitAborted0(0), nSpadicMessagesHitAborted1(0), nSpadicMessagesOverflow0(0), nSpadicMessagesOverflow1(0), nSpadicMessagesInfo0(0), nSpadicMessagesEpoch0(0), nSpadicMessagesEpoch1(0), nSpadicMessagesInfo1(0), nSpadicMessagesLost0(0), nSpadicMessagesLost1(0), nSpadicMessagesStrange0(0), nSpadicMessagesStrange1(0); //SPADIC message types per TimeSlice for single SPADICS
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
  LOG(INFO) <<"Begin Buffering Epoch Messages" << FairLogger::endl;
  {//Context to limit epochBuffers scope
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
      time = raw->GetSuperEpoch();
	//buffer all Epoch Messages
	epochBuffer[spaID][time] = raw;
	Int_t tempSize= fHM->G1(("Timestamps_Spadic"+std::to_string(spaID)))->GetN();
	fHM->G1(("Timestamps_Spadic"+std::to_string(spaID)))->SetPoint(tempSize,fNrTimeSlices,time);
      }
    }
    if(fNrTimeSlices!=0) timestampOffsets = CalculateTimestampOffsets(epochBuffer);
  }
  LOG(INFO) <<"Finish Buffering Epoch Messages" << FairLogger::endl;

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
		    fHM->G1(("Time_Offset_between_Spadic_"+std::to_string(baseSpaID)+"_and_Spadic_"+std::to_string(compSpaID)))->SetPoint(tGraphSize,fNrTimeSlices,FullTimeIt->second);
		  }
	      }
  }
  catch(std::out_of_range)
    {
      LOG(ERROR)<< "map::at() has thrown an exception " << FairLogger::endl;
    }
#endif //__CINT__


  // Starting to loop over all Spadic messages in unpacked TimeSlice
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
	    if(fActivateClusterizer)
	      if (isHit || isOverflow)
	        if(fNrTimeSlices!=0){
	          CbmSpadicRawMessage* tempPtr = new CbmSpadicRawMessage;
	          *tempPtr = *raw;
	          fLinearHitBuffer.push_back(tempPtr);
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
      if (0 <= chID && chID < 32 && isEpoch){
	if(static_cast<Long_t>(time)-static_cast<Long_t>(fLastMessageTime[0][spaID/2][chID])<-1000)  LOG(INFO) << "SpadicMessage (isEpoch): " << iSpadicMessage << " has negative Delta Fulltime. sourceA: " << sourceA << " chID: " << raw->GetChannelID() << " groupID: " << groupId << " spaID: " << spaID << " stopType: " << stopType << " infoType: " << infoType << " triggerType: " << triggerType << " isHit: " << isHit << " isInfo: " << isInfo << " isEpoch: " << isEpoch << " Lost Messages: " << lostMessages << FairLogger::endl;
	//Compute Time Deltas, write them into a histogram and store timestamps in fLastMessageTime.
	// WORKAROUND: at Present SyscoreID is not extracted, therefore all Messages are stored as if coming from SysCore 0.
	// Epoch messages are sent with chID 0 or 16, i.e. rawChID 0 from the first or the second half-Spadic. Thus, the half-chip results as 0 or 1 by dividing the chID/16 and casting this to Int_t. Bit bloody but fast.
	if(spaID != -1)fHM->H1("Delta_t_hist_for_Syscore_"+std::to_string(0)+"_Spadic_"+std::to_string(spaID/2)+"_Half_"+std::to_string((Int_t)(chID/16)))->Fill(static_cast<Long_t>(time)-static_cast<Long_t>(fLastMessageTime[0][spaID/2][chID]));
	//Write delta_t into a TGraph
	if(spaID!=-1){
	  Int_t tGraphSize = fHM->G1("Delta_t_for_Syscore_"+ std::to_string(0) +"_Spadic_"+std::to_string(spaID/2)+"_Channel_"+std::to_string(chID))->GetN();
	  fHM->G1("Delta_t_for_Syscore_"+ std::to_string(0) +"_Spadic_"+std::to_string(spaID/2)+"_Channel_"+std::to_string(chID))->SetPoint(tGraphSize,time,(static_cast<Long_t>(time)-static_cast<Long_t>(fLastMessageTime[0][spaID/2][chID])));
	}
	fLastMessageTime[0][spaID/2][chID] = time;
      }
      //if(spadicName!="") {

      //
      //  DEBUG PLOTS
      //
      
      if(isHit && chID >= 0 && chID < 32) {
        histName = spadicName + "_Half_" + std::to_string((Int_t)(chID/16)) + "_Time_vs_TimeSlice";
        fHM->H2(histName.Data())->Fill(fNrTimeSlices,time); //timeStamp(0), epoch(0), superEpoch(0);
      }

      if(isHit && chID >= 0 && chID < 32) {
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
        if(isHit) nSpadicMessagesHtimeIt++;
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
        LOG(INFO) << "SapdicMessage " << iSpadicMessage << " claims to be from " << spadicName << " with spadicID " << spaID << FairLogger::endl;
      }

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
  fHM->G1("TsCounter")->SetPoint(fHM->G1("TsCounter")->GetN(),fNrTimeSlices+1,nSpadicMessages);
  fHM->G1("TsCounterHit0")->SetPoint(fHM->G1("TsCounterHit0")->GetN(),fNrTimeSlices+1,nSpadicMessagesHit0);
  fHM->G1("TsCounterHtimeIt")->SetPoint(fHM->G1("TsCounterHtimeIt")->GetN(),fNrTimeSlices+1,nSpadicMessagesHtimeIt);
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
  
  // Catch empty TimeSlices.
  if(fNrTimeSlices==0){
    if(fHM->G1("TsCounter")->GetN()==0){
      LOG(INFO) << "Expected entries in TsCounter after finishinig first TimeSlice, but found none." << FairLogger::endl;
    }
  }
  fNrTimeSlices++;

  if(fNrTimeSlices % 10 ==0)
  {
    if (fActivateClusterizer){
      ClusterizerTime();
      ClusterizerSpace();
    }
    CleanUpBuffers();
  }
}
// ---- Finish  -------------------------------------------------------
void CbmTrdTimeCorrel::Finish()
{
  //ClusterizerSpace();
  fRawMessages->Clear("C");
  fDigis->Clear("C");
  fClusters->Clear("C");

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
  fHM->G1("TsCounterHtimeIt")->Draw("AL");
  fHM->G1("TsCounterHtimeIt")->SetLineColor(kBlue);
  fHM->G1("TsCounterHtimeIt")->GetXaxis()->SetTitle("TS number");
  fHM->G1("TsCounterHtimeIt")->GetYaxis()->SetTitle("SPADIC1 hit messages");
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

  if (true) {
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
  }
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

  //Perform uniform relabeling of Axis
  ReLabelAxis(fHM->H1("InfoType_vs_Channel")->GetYaxis(),"infoType",true,true);

  // use this to produce nice single plots
  /*
    TCanvas *cnice = new TCanvas("cnice","cnice",800,400); 
    cnice->cd();
    fHM->G1("TsCounterHtimeIt")->Draw("AL");
    fHM->G1("TsCounterHtimeIt")->SetLineColor(kBlack);
    fHM->G1("TsCounterHtimeIt")->GetXaxis()->SetTitle("timeslice");
    fHM->G1("TsCounterHtimeIt")->GetXaxis()->SetRangeUser(0,2166);
    fHM->G1("TsCounterHtimeIt")->GetYaxis()->SetTitle("SPADIC1 hit messages");
  */
  //Buffer (map) or multi SPADIC data streams based analyis have to be done here!!
  LOG(DEBUG) << "Finish of CbmTrdTimeCorrel" << FairLogger::endl;
  LOG(INFO) << "Write histo list to " << FairRootManager::Instance()->GetOutFile()->GetName() << FairLogger::endl;
  FairRootManager::Instance()->GetOutFile()->cd();
  fHM->WriteToFile();
  //delete c1;
}
// ---- FinishEvent  -------------------------------------------------------
void CbmTrdTimeCorrel::FinishEvent()
{
  LOG(DEBUG) << "FinishEvent of CbmTrdTimeCorrel" << FairLogger::endl;
}
// -------------------------------------------------------------------------
void CbmTrdTimeCorrel::ClusterizerTime()
{
  auto CompareSpadicMessages=
      [](CbmSpadicRawMessage* a,CbmSpadicRawMessage* b)
      {
      if(a->GetFullTime() == b->GetFullTime())
        if (a->GetSourceAddress() == b->GetSourceAddress())
          if(a->GetChannelID() == b->GetChannelID()){
            delete b;
            return true;
          }
      return false;
      };
  auto CompareSpadicMessagesSmaller=
        [](CbmSpadicRawMessage* a,CbmSpadicRawMessage* b)
        {
        if(a->GetFullTime() < b->GetFullTime())
          if (a->GetSourceAddress() < b->GetSourceAddress())
            if(a->GetChannelID() < b->GetChannelID())
              return true;
        return false;
        };
  const Int_t clusterWindow = 100;
  std::sort(fLinearHitBuffer.begin(),fLinearHitBuffer.end(),CompareSpadicMessagesSmaller);
  std::unique(fLinearHitBuffer.begin(),fLinearHitBuffer.end(),CompareSpadicMessages);
  std::multimap<ULong_t, CbmSpadicRawMessage*> tempmap;
  for (auto x : fLinearHitBuffer){
    long unsigned int temp = static_cast <long unsigned int>(x->GetFullTime());
    tempmap.insert(std::make_pair(temp,x));
  }
  for (auto it=tempmap.begin(); it != tempmap.end(); ++it){
    auto range = std::make_pair(tempmap.lower_bound(it->first - clusterWindow), tempmap.upper_bound(it->first + clusterWindow));
    if(tempmap.size()==0 && (range.first == tempmap.end())) continue;
    for (;range.first != range.second; ++(range.first)){
      if(it->second != nullptr && range.first->second!= nullptr)
        if(range.first->second->GetTriggerType()>= 1 || range.first->second->GetTriggerType() ==3)
        {
          Int_t ChID1 = it->second->GetChannelID();
          Int_t ChID2 = range.first->second->GetChannelID();
          Int_t SpaID1 = GetSpadicID(it->second->GetSourceAddress());
          Int_t SpaID2 = GetSpadicID(range.first->second->GetSourceAddress());
          ChID1 += (SpaID1 %2 == 1)? 16 : 0;
          ChID2 += (SpaID2 %2 == 1)? 16 : 0;
          Int_t SpaPad1 = SpaID1/2 * 32 + GetChannelOnPadPlane(ChID1);
          Int_t SpaPad2 = SpaID2/2 * 32 + GetChannelOnPadPlane(ChID2);
          if (it!=range.first) fHM->H2("Hit_Coincidences")->Fill(SpaPad1,SpaPad2);
        }
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
	/*
	  new ((*fDigis)[fiDigi]) CbmTrdDigi(address,
	  raw->GetFullTime(),//57,14 ns per timestamp
	  raw->GetTriggerType(), raw->GetInfoType(), raw->GetStopType(),
	  raw->GetNrSamples(), Samples);
	*/
	delete[] Samples;
	if (combiIt->first - lastCombiID != 1 && digiIndices.size() > 0){
	  address = CbmTrdAddress::GetAddress(layerId,moduleId,sectorId,rowId,columnId);
	  
	  //new ((*fClusters)[fiCluster]) CbmTrdCluster(digiIndices,address);
	  
	  digiIndices.clear();
	  fiCluster++;
	}
	digiIndices.push_back(fiDigi);
	lastCombiID = combiIt->first;
	fiDigi++;
      }
      if (digiIndices.size() > 0){
	address = CbmTrdAddress::GetAddress(layerId,moduleId,sectorId,rowId,columnId);
	
	//new ((*fClusters)[fiCluster]) CbmTrdCluster(digiIndices,address);
		
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
  
  for (Int_t syscore=0; syscore<1;++syscore) {
    for (Int_t spadic=0; spadic<3;++spadic) {
      for (Int_t halfchip=0; halfchip<2;++halfchip) {
	fHM->Add("Delta_t_hist_for_Syscore_"+std::to_string(syscore)+"_Spadic_"+std::to_string(spadic)+"_Half_"+std::to_string(halfchip), new TH1I(("Delta_t_hist_for_Syscore_"+std::to_string(syscore)+"_Spadic_"+std::to_string(spadic)+"_Half_"+std::to_string(halfchip)).c_str(), "Timestamp differences", 4096,-70000,70000));
      }
    }
  }

  fHM->Add("TsCounter", new TGraph());
  fHM->Add("TsCounterHit0", new TGraph());
  fHM->Add("TsCounterHtimeIt", new TGraph());
  fHM->Add("TsCounterHitAborted0", new TGraph());
  fHM->Add("TsCounterHitAborted1", new TGraph());
  fHM->Add("TsCounterOverflow0", new TGraph());
  fHM->Add("TsCounterOverflow1", new TGraph());
  fHM->Add("TsLost0", new TGraph());
  fHM->Add("TsLost1", new TGraph());
  fHM->Add("TsCounterInfo0", new TGraph());
  fHM->Add("TsCounterInfo1", new TGraph());
  fHM->Add("TsCounterEpoch0", new TGraph());
  fHM->Add("TsCounterEpoch1", new TGraph());
  fHM->Add("TsCounterStrange0", new TGraph());
  fHM->Add("TsCounterStrange1", new TGraph());
  fHM->Add("TsStrangeness0", new TGraph()); // ratio of strange messages over all messages
  fHM->Add("TsStrangeness1", new TGraph());
  for (Int_t SysID=0; SysID<1;++SysID){
    for (Int_t SpaID=0; SpaID<3;++SpaID){
      for (Int_t ChID=0; ChID<32;++ChID){
	fHM->Add(("Delta_t_for_Syscore_"+std::to_string(SysID)+"_Spadic_"+std::to_string(SpaID)+"_Channel_"+std::to_string(ChID)), new TGraph());
	fHM->G1(("Delta_t_for_Syscore_"+std::to_string(SysID)+"_Spadic_"+std::to_string(SpaID)+"_Channel_"+std::to_string(ChID)))->SetNameTitle(("Delta_t_for_Syscore_"+std::to_string(SysID)+"_Spadic_"+std::to_string(SpaID)).c_str(),("Delta_t_for_Syscore_"+std::to_string(SysID)+"_Spadic_"+std::to_string(SpaID)).c_str());
      }
    }
  }
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
  fHM->H2("Hit_Coincidences")->GetYaxis()->SetTitle("SPadicChannel");


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

