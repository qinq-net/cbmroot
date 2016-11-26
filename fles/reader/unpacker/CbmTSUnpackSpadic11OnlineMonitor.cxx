// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmTSUnpackSpadic11OnlineMonitor                       -----
// -----                    Created 07.11.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------
#include "CbmTSUnpackSpadic11OnlineMonitor.h"

#include "CbmSpadicRawMessage.h"

#include "TimesliceReader.hpp"

#include "FairLogger.h"
#include "FairRootManager.h"

#include "TClonesArray.h"

#include "CbmHistManager.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TMultiGraph.h"

#include "FairRun.h"
#include "FairRunOnline.h"
#include "THttpServer.h"

#include <iostream>
#include <map>
#include <vector>

CbmTSUnpackSpadic11OnlineMonitor::CbmTSUnpackSpadic11OnlineMonitor()
  : CbmTSUnpack(),
    fSpadicRaw(new TClonesArray("CbmSpadicRawMessage", 10)),
    fEpochMarkerArray(),
    fPreviousEpochMarkerArray(),
    fSuperEpochArray(),
    fEpochMarker(0),
    fSuperEpoch(0),
    fcB(NULL),
    fcM(NULL),
    fcH(NULL),
    fcL(NULL),
    fcE(NULL),
    fcO(NULL),
    fcS(NULL),
    fcI(NULL),
    fBaseline({NULL}),
    fmaxADCmaxTimeBin({NULL}),
    fHit({NULL}),
    fLost({NULL}),
    fEpoch({NULL}),
    fOutOfSync({NULL}),
    fStrange({NULL}),
    fInfo({NULL}),
    fHM(new CbmHistManager()),
    fNrExtraneousSamples{0}
{
  for (Int_t i=0; i < NrOfSyscores; ++i) { 
    for (Int_t j=0; j < NrOfHalfSpadics; ++j) { 
      fEpochMarkerArray[i][j] = 0;
	  fPreviousEpochMarkerArray[i][j] =0;
      fSuperEpochArray[i][j] = 0;
    }
  }
  InitHistos();
  InitCanvas();
}

CbmTSUnpackSpadic11OnlineMonitor::~CbmTSUnpackSpadic11OnlineMonitor()
{
  LOG(INFO) << "Number of extraneous Samples "<< fNrExtraneousSamples << FairLogger::endl;
}

Bool_t CbmTSUnpackSpadic11OnlineMonitor::Init()
{
  LOG(INFO) << "Initializing" << FairLogger::endl; 

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }
  ioman->Register("SpadicRawMessage", "spadic raw data", fSpadicRaw, kTRUE);

  return kTRUE;
}

Bool_t CbmTSUnpackSpadic11OnlineMonitor::DoUnpack(const fles::Timeslice& ts, size_t component)
{
  // compare to: https://github.com/spadic/spadic10-software/blob/master/lib/message/message.h
  // or fles/spadic/message/wrap/cpp/message.cpp
  LOG(DEBUG) << "Unpacking Spadic Data" << FairLogger::endl; 
  TH2I* h = NULL;
  TString cName;
  spadic::TimesliceReader r;
  Int_t counter=0;

  r.add_component(ts, component);

  for (auto addr : r.sources()) {
    LOG(DEBUG) << "---- reader " << addr << " ----" << FairLogger::endl;
    while (auto mp = r.get_message(addr)) {
      if(gLogger->IsLogNeeded(DEBUG)) {
	print_message(*mp);
      }
      counter++;
      Int_t link = ts.descriptor(component, 0).eq_id;
      Int_t address = addr;
      /*
	if ( mp->is_valid() ){
	LOG(DEBUG) << counter << " This message is valid" << FairLogger::endl; 
	} else {
	LOG(ERROR) << counter <<  " This message is not valid" << FairLogger::endl; 
	continue;
	}
      */
      Bool_t isInfo(false), isHit(false), isEpoch(false), isEpochOutOfSync(false), isOverflow(false), isHitAborted(false), isStrange(false);
      if ( mp->is_epoch_out_of_sync() ){
	isEpochOutOfSync = true;
        FillEpochInfo(link, addr, mp->epoch_count());
	GetEpochInfo(link, addr);
	Int_t triggerType = -1;
        Int_t infoType = -1;
        Int_t stopType = -1;
	Int_t groupId = mp->group_id();
	Int_t channel = mp->channel_id();
	Int_t time = -1;
	Int_t bufferOverflowCounter = 0;
	Int_t samples = 1;
	Int_t* sample_values = new Int_t[samples];
	sample_values[0] = -256;
	LOG(DEBUG) <<  counter << " This is an out of sync Epoch Marker" << FairLogger::endl; 
	LOG(DEBUG) << "   TimeStamp: " << mp->timestamp()<< FairLogger::endl; 
	LOG(DEBUG) << "   Channel:   " << mp->channel_id()<< FairLogger::endl; 
	LOG(DEBUG) << "   Group:     " << mp->group_id()<< FairLogger::endl; 
	new( (*fSpadicRaw)[fSpadicRaw->GetEntriesFast()] )
	  CbmSpadicRawMessage(link, address, channel, fEpochMarker, time, 
			      fSuperEpoch, triggerType, infoType, stopType, groupId,
			      bufferOverflowCounter, samples, sample_values,
			      isHit, isInfo, isEpoch, isEpochOutOfSync, isHitAborted, isOverflow, isStrange);
	delete[] sample_values;
	fOutOfSync[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(channel,groupId,1);
      }
      else if ( mp->is_epoch_marker() ) { 
	LOG(DEBUG) <<  counter << " This is an Epoch Marker" << FairLogger::endl; 
	isEpoch = true;
        FillEpochInfo(link, addr, mp->epoch_count());
	GetEpochInfo(link, addr);
	Int_t triggerType = -1;
        Int_t infoType = -1;
        Int_t stopType = -1;
	Int_t groupId = mp->group_id();
	Int_t channel = mp->channel_id();
	Int_t time = -1;
	Int_t bufferOverflowCounter = 0;
	Int_t samples = 1;
	Int_t* sample_values = new Int_t[samples];
	sample_values[0] = -256;
	new( (*fSpadicRaw)[fSpadicRaw->GetEntriesFast()] )
	  CbmSpadicRawMessage(link, address, channel, fEpochMarker, time, 
			      fSuperEpoch, triggerType, infoType, stopType, groupId,
			      bufferOverflowCounter, samples, sample_values,
			      isHit, isInfo, isEpoch, isEpochOutOfSync, isHitAborted, isOverflow, isStrange);
	delete[] sample_values;
	fEpoch[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(channel,groupId,1);
      } 
      else if ( mp->is_buffer_overflow() ){
	LOG(DEBUG) <<  counter << " This is a buffer overflow message" << FairLogger::endl; 
	isOverflow = true;
	GetEpochInfo(link, addr);
	Int_t triggerType = -1;
        Int_t infoType = -1;
        Int_t stopType = -1;
	Int_t groupId = mp->group_id();
	Int_t channel = mp->channel_id();
	Int_t time = mp->timestamp();
        Int_t bufferOverflowCounter = static_cast<Int_t>(mp->buffer_overflow_count());
	Int_t samples = 1;
	Int_t* sample_values = new Int_t[samples];
	sample_values[0] = -256;
	new( (*fSpadicRaw)[fSpadicRaw->GetEntriesFast()] )
	  CbmSpadicRawMessage(link, address, channel, fEpochMarker, time, 
			      fSuperEpoch, triggerType, infoType, stopType, groupId,
			      bufferOverflowCounter, samples, sample_values,
			      isHit, isInfo, isEpoch, isEpochOutOfSync, isHitAborted, isOverflow, isStrange);
	delete[] sample_values;
	channel = GetChannelOnPadPlane(channel,groupId);
	if (channel > 15){
	  channel-= 16;
	  groupId = 1;
	} else {
	  groupId = 0;
	}
	fLost[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(channel,groupId,bufferOverflowCounter);
      }
      else if ( mp->is_info() ){
	LOG(DEBUG) <<  counter << " This is a info message" << FairLogger::endl; 
	isInfo = true;
	GetEpochInfo(link, addr);

        Int_t triggerType = -1;
        Int_t infoType = static_cast<Int_t>(mp->info_type());
        Int_t stopType = -1;
	Int_t groupId = mp->group_id();
	Int_t channel = mp->channel_id();
	Int_t time = mp->timestamp();
        Int_t bufferOverflowCounter = 0;//mp->buffer_overflow_count();// should be now obsolete
	Int_t samples = 1;
	Int_t* sample_values = new Int_t[samples];
	sample_values[0] = -256;
	new( (*fSpadicRaw)[fSpadicRaw->GetEntriesFast()] )
	  CbmSpadicRawMessage(link, address, channel, fEpochMarker, time, 
			      fSuperEpoch, triggerType, infoType, stopType, groupId,
			      bufferOverflowCounter, samples, sample_values,
			      isHit, isInfo, isEpoch, isEpochOutOfSync, isHitAborted, isOverflow, isStrange);
	delete[] sample_values;
	
	channel = GetChannelOnPadPlane(channel,groupId);
	if (channel > 15){
	  channel-= 16;
	  groupId = 1;
	} else {
	  groupId = 0;
	}
	fInfo[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(channel,groupId,1);
      }
      else if ( mp->is_hit() ) { 
	LOG(DEBUG) <<  counter << " This is a hit message" << FairLogger::endl; 
	isHit = true;
	GetEpochInfo(link, addr);
	Int_t triggerType =  static_cast<Int_t>(mp->hit_type());
	Int_t stopType = static_cast<Int_t>(mp->stop_type());
	Int_t time = mp->timestamp();
        Int_t infoType = -1;
	Int_t groupId = mp->group_id();
	Int_t bufferOverflowCounter = 0;
	Int_t samples = mp->samples().size();
	if(samples>32) {
	  fNrExtraneousSamples++;
	  samples=32; //Suppress extraneous Samples, which cannot (!) occur in Raw Data Stream.
	}
	Int_t* sample_values =  new Int_t[samples];
	Int_t channel = mp->channel_id();
	Int_t counter1=0;
	Int_t maxADC(-256), maxTB(-1);
	for (auto x : mp->samples()) {
	  sample_values[counter1] = x;
	  if (x > maxADC){
	    maxADC = x;
	    maxTB = counter1;
	  }
	  ++counter1;
	}
	if (GetSpadicID(address) > 2 || GetSyscoreID(link) > 3){
	  printf("link:%i SysCoreID:%i address:%i SpadicID:%i array:%i\n",link,GetSyscoreID(link),address,GetSpadicID(address),GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address));
	} else {
	  cName.Form("SysCore_%i_Spadic_%i",GetSyscoreID(link),GetSpadicID(address));     
	  //h = (TH2I*)fHM->H2(TString("Baseline_"+cName).Data());
	  //h->Fill(sample_values[0],groupId*16+channel);
	  fBaseline[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(sample_values[0],groupId*16+channel);	  
	  fmaxADCmaxTimeBin[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(maxTB,maxADC);
	}
	//}
	new( (*fSpadicRaw)[fSpadicRaw->GetEntriesFast()] )
	  CbmSpadicRawMessage(link, address, channel, fEpochMarker, time, 
			      fSuperEpoch, triggerType, infoType, stopType, groupId,
			      bufferOverflowCounter, samples, sample_values,
			      isHit, isInfo, isEpoch, isEpochOutOfSync, isHitAborted, isOverflow, isStrange);
	//++counter;
	delete[] sample_values;	
	channel = GetChannelOnPadPlane(channel,groupId);
	if (channel > 15){
	  channel-= 16;
	  groupId = 1;
	} else {
	  groupId = 0;
	}
	fHit[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(channel,groupId,1);
      } 
      else if ( mp->is_hit_aborted()) {
	LOG(DEBUG) <<  counter << " This is a hit message was aborted" << FairLogger::endl; 
	isHitAborted = true;
	GetEpochInfo(link, addr);
	Int_t triggerType = -1;
	Int_t stopType = -1;
	Int_t time = mp->timestamp();
	Int_t infoType = -1;
	Int_t groupId = -1;//mp->group_id();//???
	Int_t bufferOverflowCounter = 0;
	Int_t samples = 1;
	Int_t* sample_values = NULL;
	Int_t channel = mp->channel_id();
	infoType = static_cast<Int_t>(mp->info_type());// should here be stoptype instead???
	sample_values = new Int_t[samples];
	sample_values[0] = -256;
	new( (*fSpadicRaw)[fSpadicRaw->GetEntriesFast()] )
	  CbmSpadicRawMessage(link, address, channel, fEpochMarker, time, 
			      fSuperEpoch, triggerType, infoType, stopType, groupId,
			      bufferOverflowCounter, samples, sample_values,
			      isHit, isInfo, isEpoch, isEpochOutOfSync, isHitAborted, isOverflow, isStrange);
	//++counter;
	delete[] sample_values;

      }
      else {
	isStrange = true;
	//GetEpochInfo(link, addr);
	Int_t triggerType = -1;
	Int_t stopType = -1;
	Int_t time = -1;//mp->timestamp();
	Int_t infoType = -1;
	Int_t groupId = -1;//mp->group_id();
	Int_t bufferOverflowCounter = 0;
	Int_t samples = 1;
	Int_t* sample_values = NULL;
	Int_t channel = -1;//mp->channel_id();
	sample_values = new Int_t[samples];
	sample_values[0] = -256;
	new( (*fSpadicRaw)[fSpadicRaw->GetEntriesFast()] )
	  CbmSpadicRawMessage(link, address, channel, fEpochMarker, time, 
			      fSuperEpoch, triggerType, infoType, stopType, groupId,
			      bufferOverflowCounter, samples, sample_values,
			      isHit, isInfo, isEpoch, isEpochOutOfSync, isHitAborted, isOverflow, isStrange);
	//++counter;
	delete[] sample_values;
	//fStrange[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(channel,groupId);
	LOG(DEBUG) <<  counter << " This message type is not hit, info, epoch or overflow and will not be stored in the TClonesArray" << FairLogger::endl; 
	LOG(DEBUG) << " valide:" << mp->is_valid() << " epoch marker:" << fEpochMarker << " super epoch marker:" << fSuperEpoch << " time:" << time << " link:" << link << " address:" << address << FairLogger::endl;
	LOG(DEBUG) << "Channel ID:" << mp->channel_id() << FairLogger::endl;
	//if ( mp->is_hit_aborted() )
	//LOG(INFO) << "hit is aborted" << FairLogger::endl;
	//if ( mp->is_hit() )
	LOG(DEBUG) << "GroupID:" << mp->group_id() << "hit: triggerType:" << static_cast<Int_t>(mp->hit_type()) << " stopType:" << static_cast<Int_t>(mp->stop_type()) << " Nr.of samples:" << mp->samples().size() << FairLogger::endl;

	//if ()
	for (auto x : mp->samples()) {
	  LOG(DEBUG) << " " << x;
	}
	LOG(DEBUG) << FairLogger::endl;
	//if (mp->is_info())
	LOG(DEBUG) << "InfoType:" << static_cast<Int_t>(mp->info_type()) << FairLogger::endl;
	//if ()
	LOG(DEBUG) << "Nr. of overflows:" << static_cast<Int_t>(mp->buffer_overflow_count()) << FairLogger::endl;
	//print_message(mp);
      }
    }
  }
  if (fSuperEpoch%1000 == 0)
    UpdateCanvas();
  return kTRUE;
}

  void CbmTSUnpackSpadic11OnlineMonitor::print_message(const spadic::Message& m)
  {
    LOG(INFO) << "v: " << (m.is_valid() ? "o" : "x");
    LOG(DEBUG) << " / gid: " << static_cast<int>(m.group_id());
    LOG(DEBUG) << " / chid: " << static_cast<int>(m.channel_id());
    if ( m.is_hit() ) { 
      LOG(DEBUG) << " / ts: " << m.timestamp();
      LOG(DEBUG) << " / samples (" << m.samples().size() << "):";
      for (auto x : m.samples()) {
	LOG(DEBUG) << " " << x;
      }
      LOG(DEBUG) << FairLogger::endl;
    } else {
      if ( m.is_epoch_marker() ) { 
	LOG(DEBUG) << " This is an Epoch Marker" << FairLogger::endl; 
      } else if ( m.is_epoch_out_of_sync() ) { 
	LOG(INFO) << " This is an out of sync Epoch Marker" << FairLogger::endl; 
      } else {
	LOG(INFO) << " This is not known" << FairLogger::endl;
      }
    }
  }

void CbmTSUnpackSpadic11OnlineMonitor::FillEpochInfo(Int_t link, Int_t addr, Int_t epoch_count) 
{
  auto it=groupToExpMap.find(link);
  if (it == groupToExpMap.end()) {
    LOG(FATAL) << "Could not find an entry for equipment ID" << 
      std::hex << link << std::dec << FairLogger::endl;
  } else {
    /* Check for repeated Epoch Messages, as the repeated Microslices
       are not captured by the CbmTsUnpacker. This is to ensure the 
       linearity of the GetFullTime() method.
     */
    // dirty workaround in the following line: only sufficiently big backsteps of Epoch will trigger an upcount of SuperEpoch. this avoids a high sensitivity of the SuperEpoch counting method to overlaps of some epochs, before the overlaps are understood better
    if ( epoch_count < (fEpochMarkerArray[it->second][addr] - 3000) )
    {
      if(SuppressMultipliedEpochMessages)
      if ( epoch_count != fPreviousEpochMarkerArray[it->second][addr] ){
        fSuperEpochArray[it->second][addr]++;
      } else {
        LOG(ERROR)<< "Multiply repeated Epoch Messages at Super Epoch "
            << fSuperEpoch << " Epoch "
            << epoch_count << " for Syscore"
            << it->second << "_Spadic"
            << addr << FairLogger::endl;
      }
      else fSuperEpochArray[it->second][addr]++;

      LOG(DEBUG) << "Overflow of EpochCounter for Syscore" 
		 << it->second << "_Spadic"  
		 << addr << FairLogger::endl;
    } else if ((epoch_count - fEpochMarkerArray[it->second][addr]) !=1 ) {
      LOG(INFO) << "Missed epoch counter for Syscore" 
		<< it->second << "_Spadic"  
		<< addr << FairLogger::endl;
    } else if (epoch_count == fEpochMarkerArray[it->second][addr]){
      LOG(ERROR) << "Identical Epoch Counters for Syscore" 
		 << it->second << "_Spadic"  
		 << addr << FairLogger::endl;
    }
    fPreviousEpochMarkerArray[it->second][addr] = fEpochMarkerArray[it->second][addr];
    fEpochMarkerArray[it->second][addr] = epoch_count; 
  }

}

  void CbmTSUnpackSpadic11OnlineMonitor::GetEpochInfo(Int_t link, Int_t addr) 
  {
    auto it=groupToExpMap.find(link);
    if (it == groupToExpMap.end()) {
      LOG(FATAL) << "Could not find an entry for equipment ID" << 
	std::hex << link << std::dec << FairLogger::endl;
    } else {
      fEpochMarker = fEpochMarkerArray[it->second][addr]; 
      fSuperEpoch = fSuperEpochArray[it->second][addr]; 
    }
  
  }
Int_t CbmTSUnpackSpadic11OnlineMonitor::GetChannelOnPadPlane(Int_t SpadicChannel, Int_t groupId)
{
  SpadicChannel = groupId * 16 + SpadicChannel;
  Int_t channelMapping[32] = {31,15,30,14,29,13,28,12,27,11,26,10,25, 9,24, 8,
			      23, 7,22, 6,21, 5,20, 4,19, 3,18, 2,17, 1,16, 0};
  if (SpadicChannel < 0 || SpadicChannel > 31){
    if (SpadicChannel !=-1) LOG(ERROR) << "CbmTrdTimeCorrel::GetChannelOnPadPlane ChId " << SpadicChannel << FairLogger::endl;
    return -1;
  } else {
    return channelMapping[SpadicChannel];
  }
}

Int_t CbmTSUnpackSpadic11OnlineMonitor::GetSpadicID(Int_t address)
{
  //TString spadic="";
  Int_t SpaId = -1;
  switch (address) {
  case (SpadicBaseAddress+0):  // first spadic
    //spadic="Spadic0";
    SpaId = 0;
    break;
  case (SpadicBaseAddress+1):  // first spadic
    //spadic="Spadic0";
    SpaId = 0;
    break;
  case (SpadicBaseAddress+2):  // second spadic
    //spadic="Spadic1";
    SpaId = 1;
    break;
  case (SpadicBaseAddress+3):  // second spadic
    //spadic="Spadic1";
    SpaId = 1;
    break;
  case (SpadicBaseAddress+4):  // third spadic
    //spadic="Spadic2";
    SpaId = 2;
    break;
  case (SpadicBaseAddress+5):  // third spadic
    //spadic="Spadic2";
    SpaId = 2;
    break;
  default:
    LOG(ERROR) << "Source Address " << address << " not known." << FairLogger::endl;
    break;
  }
  return SpaId;
}
Int_t CbmTSUnpackSpadic11OnlineMonitor::GetSyscoreID(Int_t link)
{
  Int_t SyscoreID=link-BaseEquipmentID;
  if((SyscoreID<0||SyscoreID>3)){
    LOG(DEBUG) << "EqID " << link << " not known." << FairLogger::endl;
    SyscoreID=-1;
  }
  return SyscoreID;
}
// ----              -------------------------------------------------------

inline TString CbmTSUnpackSpadic11OnlineMonitor::GetSpadicName(Int_t link,Int_t address){

  TString spadicName="";
  Int_t SpadicID=0;
  spadicName="SysCore_"+std::to_string(GetSyscoreID(link))+"_";
  SpadicID=GetSpadicID(address);
  
  //SpadicID/=2;  

  spadicName += "Spadic_";

  spadicName+=std::to_string(SpadicID);

  return spadicName;
}
void CbmTSUnpackSpadic11OnlineMonitor::InitHistos()
{
#ifdef USE_HTTP_SERVER
  THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
#endif
  //cout << "InitHistos" << endl;
  TString histName;
  for (Int_t iLink = 0; iLink < NrOfSyscores; iLink++){
    for (Int_t iAddress = 0; iAddress < NrOfSpadics; iAddress++){
      histName.Form("SysCore_%i_Spadic_%i",iLink,iAddress);
      fHM->Add(TString("Baseline_"+histName).Data(),new TH2I (TString("Baseline_"+histName).Data(),TString("Baseline_"+histName).Data(), 512,-256.5,255.5,33,-0.5,32.5));
      fBaseline[(iLink)*(NrOfSpadics)+iAddress]=(TH2I*)fHM->H2(TString("Baseline_"+histName).Data());
      fHM->Add(TString("maxADC_vs_maxTimeBin_"+histName).Data(),new TH2I (TString("maxADC_vs_maxTimeBin_"+histName).Data(),TString("maxADC_vs_maxTimeBin_"+histName).Data(),33,-0.5,32.5, 512,-256.5,255.5));
      fmaxADCmaxTimeBin[(iLink)*(NrOfSpadics)+iAddress]=(TH2I*)fHM->H2(TString("maxADC_vs_maxTimeBin_"+histName).Data());
      fHM->Add(TString("Hit_"+histName).Data(),new TH2I (TString("Hit_"+histName).Data(),TString("Hit_"+histName).Data(),16,-0.5,15.5,2,-0.5,1.5));
      fHit[(iLink)*(NrOfSpadics)+iAddress]=(TH2I*)fHM->H2(TString("Hit_"+histName).Data());
      fHM->Add(TString("Lost_"+histName).Data(),new TH2I (TString("Lost_"+histName).Data(),TString("Lost_"+histName).Data(),16,-0.5,15.5,2,-0.5,1.5));
      fLost[(iLink)*(NrOfSpadics)+iAddress]=(TH2I*)fHM->H2(TString("Lost_"+histName).Data());
      fHM->Add(TString("Epoch_"+histName).Data(),new TH2I (TString("Epoch_"+histName).Data(),TString("Epoch_"+histName).Data(),16,-0.5,15.5,2,-0.5,1.5));
      fEpoch[(iLink)*(NrOfSpadics)+iAddress]=(TH2I*)fHM->H2(TString("Epoch_"+histName).Data());
      fHM->Add(TString("OutOfSync_"+histName).Data(),new TH2I (TString("OutOfSync_"+histName).Data(),TString("OutOfSync_"+histName).Data(),16,-0.5,15.5,2,-0.5,1.5));
      fOutOfSync[(iLink)*(NrOfSpadics)+iAddress]=(TH2I*)fHM->H2(TString("OutOfSync_"+histName).Data());
      fHM->Add(TString("Strange_"+histName).Data(),new TH2I (TString("Strange_"+histName).Data(),TString("Strange_"+histName).Data(),16,-0.5,15.5,2,-0.5,1.5));
      fStrange[(iLink)*(NrOfSpadics)+iAddress]=(TH2I*)fHM->H2(TString("Strange_"+histName).Data());
      fHM->Add(TString("Info_"+histName).Data(),new TH2I (TString("Info_"+histName).Data(),TString("Info_"+histName).Data(),16,-0.5,15.5,2,-0.5,1.5));
      fInfo[(iLink)*(NrOfSpadics)+iAddress]=(TH2I*)fHM->H2(TString("Info_"+histName).Data());
    }
  }
}
void CbmTSUnpackSpadic11OnlineMonitor::InitCanvas()
{
  //cout << "InitCanvas" << endl;
  //TCanvas* c[2];
  //TH2I* h = NULL;
  TString cName;
  fcB = new TCanvas(TString("Baseline").Data(),TString("Baseline").Data(),1600,1200);
  fcB->Divide(3,4);
  fcM/*[(iLink)*(NrOfSpadics)+iAddress]*/ = new TCanvas(TString("maxADC_vs_maxTimeBin").Data(),TString("maxADC_vs_maxTimeBin").Data(),1600,1200);
  fcM->Divide(3,4);
  fcH= new TCanvas(TString("HitMap").Data(),TString("HitMap").Data(),1600,1200);
  fcH->Divide(3,4);
  fcL= new TCanvas(TString("LostMap").Data(),TString("LostMap").Data(),1600,1200);
  fcL->Divide(3,4);
  fcE= new TCanvas(TString("EpochMap").Data(),TString("EpochMap").Data(),1600,1200);
  fcE->Divide(3,4);
  fcO= new TCanvas(TString("OutOfSyncMap").Data(),TString("OutOfSyncMap").Data(),1600,1200);
  fcO->Divide(3,4);
  fcS= new TCanvas(TString("StrangeMap").Data(),TString("StrangeMap").Data(),1600,1200);
  fcS->Divide(3,4);
  fcI= new TCanvas(TString("InfoMap").Data(),TString("InfoMap").Data(),1600,1200);
  fcI->Divide(3,4);
  for (Int_t iLink = 0; iLink < NrOfSyscores; iLink++){
    for (Int_t iAddress = 0; iAddress < NrOfSpadics; iAddress++){
      cName.Form("SysCore_%i_Spadic_%i",iLink,iAddress);      
      fcB->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz();
      fBaseline[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      //h = (TH2I*)fHM->H2(TString("Baseline_"+cName).Data());
      //h->Draw("colz");
      //fcB->cd((iLink+1)*(NrOfSpadics)+iAddress)->Update();
      
      cName.Form("SysCore_%i_Spadic_%i",iLink,iAddress);
      fcM->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz();
      fmaxADCmaxTimeBin[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      //h = (TH2I*)fHM->H2(TString("maxADC_vs_maxTimeBin_"+cName).Data());
      //h->Draw("colz");
      //fcM->cd((iLink)*(NrOfSpadics)+iAddress+1)->Update();
      fcH->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz(0);
      fHit[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      fcL->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz(0);
      fLost[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      fcE->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz(0);
      fEpoch[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      fcO->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz(0);
      fOutOfSync[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      fcS->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz(0);
      fStrange[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      fcI->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz(0);
      fInfo[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
    }
  }
}
void CbmTSUnpackSpadic11OnlineMonitor::UpdateCanvas()
{
  /*
  //cout << "UpdateCanvas" << endl;
  TH2I* h = NULL;
  TString cName;
  for (Int_t iLink = 0; iLink < NrOfSyscores; iLink++){
    for (Int_t iAddress = 0; iAddress < NrOfSpadics; iAddress++){
      cName.Form("SysCore_%i_Spadic_%i",iLink,iAddress);
      fcB->cd((iLink)*(NrOfSpadics)+iAddress+1);
      fBaseline[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      //h = (TH2I*)fHM->H2(TString("Baseline_"+cName).Data());
      //h->Draw("colz");
      //fcB->cd((iLink)*(NrOfSpadics)+iAddress+1)->Update();
      cName.Form("SysCore_%i_Spadic_%i",iLink,iAddress);
      fcM->cd((iLink)*(NrOfSpadics)+iAddress+1);
      fmaxADCmaxTimeBin[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      //h = (TH2I*)fHM->H2(TString("maxADC_vs_maxTimeBin_"+cName).Data());
      //h->Draw("colz");
      //fcM->cd((iLink)*(NrOfSpadics)+iAddress+1)->Update();
    }
  }
  */
}
  void CbmTSUnpackSpadic11OnlineMonitor::Reset()
  {
    fSpadicRaw->Clear();
  }
void CbmTSUnpackSpadic11OnlineMonitor::FinishEvent()
{
  //UpdateCanvas();
}
void CbmTSUnpackSpadic11OnlineMonitor::Finish()
{
  for (Int_t i=0; i < NrOfSyscores; ++i) { 
    for (Int_t j=0; j < NrOfHalfSpadics; ++j) { 
      LOG(DEBUG) << "There have been " << fSuperEpochArray[i][j] 
		 << " SuperEpochs for Syscore" << i << "_Spadic" 
		 << j << " in this file" << FairLogger::endl;
    }
  }
  UpdateCanvas();
}


  /*
    void CbmTSUnpackSpadic11OnlineMonitor::Register()
    {
    }
  */


  ClassImp(CbmTSUnpackSpadic11OnlineMonitor)

