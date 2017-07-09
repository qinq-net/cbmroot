// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmTSUnpackSpadic20OnlineMonitor                       -----
// -----                    Created 07.11.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------
#include "CbmTSUnpackSpadic20OnlineMonitor.h"

#include "CbmSpadicRawMessage.h"

#include "TimesliceReader.hpp"
#include "Timeslice.hpp"

#include "Message.hpp"

#include "FairLogger.h"
#include "FairRootManager.h"

#include "TClonesArray.h"

#include "CbmHistManager.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TMultiGraph.h"
#include "TMath.h"
#include "FairRun.h"
#include "FairRunOnline.h"
#include "THttpServer.h"

#include <iostream>
#include <map>
#include <vector>



CbmTSUnpackSpadic20OnlineMonitor::CbmTSUnpackSpadic20OnlineMonitor(Bool_t highPerformance)
  : CbmTSUnpack(),
    fSpadicRaw(new TClonesArray("CbmSpadicRawMessage", 10)),
    fHighPerformance(highPerformance),
    fEpochMarkerArray(),
    fPreviousEpochMarkerArray(),
    fSuperEpochArray(),
    fEpochMarker(0),
    fSuperEpoch(0),
    fLastSuperEpochA{0},
    fLastSuperEpochB{0},
    fLastFullTime{{{0}}},
    fcB(NULL),
    fcM(NULL),
    fcH(NULL),
    fcL(NULL),
    fcE(NULL),
    fcO(NULL),
    fcS(NULL),
    fcI(NULL),
    fcTS(NULL),
    fcF(NULL),
    fcSp(NULL),
    fcPS{NULL},
    fcMS{NULL},
    fBaseline{NULL},
    fmaxADCmaxTimeBin{NULL},
    fHit{NULL},
    fLost{NULL},
    fEpoch{NULL},
    fOutOfSync{NULL},
    fStrange{NULL},
    fInfo{NULL},
    fTSGraph{NULL},
    fHitTimeA{NULL},
    fHitTimeB{NULL},
    fHitFrequency{NULL},
    fSpectrum{NULL},
    fPulseShape{NULL},
    fMaxADC{NULL},
    fMessageStatistic{NULL},
    fMessageTypes{"Epoch",
	  "Epoch out of synch",
	  "Hit",
	  "Hit aborted",
	  "Info",
	  "Overflow",
	  "Strange"},
    fTriggerTypes{ "Global trigger",
	  "Self triggered",
	  "Neighbor triggered",
	  "Self and neighbor triggered"},
    fStopTypes{"Normal end of message",
	  "Channel buffer full",
	  "Ordering FIFO full",
	  "Multi hit",
	  "Multi hit and channel buffer full",
	  "Multi hit and ordering FIFO full"},
    fInfoTypes{"Channel disabled during message building",
	  "Next grant timeout",
	  "Next request timeout",
	  "New grant but channel empty",
	  "Corruption in message builder",
	  "Empty word",
	  "Epoch out of sync"},
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

CbmTSUnpackSpadic20OnlineMonitor::~CbmTSUnpackSpadic20OnlineMonitor()
{
	LOG(INFO) << "Number of extraneous Samples "<< fNrExtraneousSamples << FairLogger::endl;
}

Bool_t CbmTSUnpackSpadic20OnlineMonitor::Init()
{
	LOG(INFO) << "Initializing" << FairLogger::endl;

	FairRootManager* ioman = FairRootManager::Instance();
	if (ioman == NULL) {
	  LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
	}
	ioman->Register("SpadicRawMessage", "spadic raw data", fSpadicRaw, kTRUE);
	return kTRUE;
}

Bool_t CbmTSUnpackSpadic20OnlineMonitor::DoUnpack(const fles::Timeslice& ts, size_t component)
{
  // compare to: https://github.com/spadic/spadic10-software/blob/master/lib/message/message.h
  // or fles/spadic/message/wrap/cpp/message.cpp
  LOG(DEBUG) << "Unpacking Spadic Data" << FairLogger::endl;

  TH2I* h = NULL;
  TString cName;

  LOG(DEBUG) << "Timeslice contains " << ts.num_microslices(component)
             << "microslices." << FairLogger::endl;

  std::vector<uint16_t> buf0;
  std::vector<uint16_t> buf1;

  buf0.clear();
  buf1.clear();

   // Loop over microslices
  for (size_t m = 0; m < ts.num_microslices(component); ++m){

      auto msDescriptor = ts.descriptor(component, m);
      Int_t fEquipmentId = msDescriptor.eq_id;
      const uint8_t* msContent = reinterpret_cast<const uint8_t*>(ts.content(component, m));
      uint32_t size = msDescriptor.size;

      if(size>0)
        LOG(DEBUG) << "Microslice: " << msDescriptor.idx
                   << " has size: " << size << FairLogger::endl;

      const uint64_t* pInBuff = reinterpret_cast<const uint64_t*>( msContent );
      for (auto uIdx = 0; uIdx < size; uIdx ++){
          // Fill message
          uint64_t ulData = static_cast<uint64_t>( pInBuff[uIdx] );
          uint16_t afck_id = (ulData & 0xffff000000000000) >> 48;

          //printf("Frame: 0x%X\n", ulData);
          //printf("AFCK ID: 0x%04X\n", afck_id);

	        // Last 2bytes of mac addr of the afck, hard coded now for sps2016
          if(afck_id != 0x5b9d)
	         continue;

	        //std::cout << "AFCK ID: " << std::hex << afck_id << std::endl;
	        //std::cout << "0x" << std::hex << ulData << std::endl;
	        uint8_t  downlink_header = (ulData & 0xf00000) >> 20;
	        uint8_t  downlink_nr     = (ulData & 0xf0000)  >> 16;
	        uint16_t payload         =  ulData & 0xffff;

	        //std::cout << "Header: " << std::hex << downlink_header << " Downlink number: " << downlink_nr << std::endl;
	        //printf("Header 0x%02x Link: 0x%02x \n", downlink_header, downlink_nr);

      	  if(downlink_header != 0xa)
      	    std::cout << "Corruption in downlink header found" << std::endl;

      	  // Put the data in a buffer
      	  if(downlink_nr == 0){
      	    buf0.push_back(payload);
      	  }
      	  //else if(downlink_nr == 1)
      	  //  buf1.push_back(payload);

      	}
    }

  // This is not nice... quick n' dirty
  //std::move(buf1.begin(), buf1.end(), std::back_inserter(buf0));

  /*
  if(buf0.size() > 0){
    printf("Buffer:");
    for(auto i=0; i<buf0.size();i++)
      printf(" %04x", buf0[i]);
    printf("\n");
  }
  */

  // Unpack the timeslice
  spadic::MessageReader r;
  size_t buf_size = buf0.size();
  std::vector<uint16_t>::pointer p = &buf0[0];
  uint16_t* buf_array = p;
  r.add_buffer( buf_array , buf0.size());
  Int_t address = 0; // Fake the addr for now
  Int_t addr = 0; // Fake!
  Int_t counter=0;
  Int_t link = ts.descriptor(component, 0).eq_id;
  Bool_t isInfo(false), isHit(false), isEpoch(false), isEpochOutOfSync(false), isOverflow(false), isHitAborted(false), isStrange(false);

  while(auto m = r.get_message()) {

    if(m->is_hit()){
      auto& s = m->samples();
      //printf("Group: %i Ch: %i Ts: %i Samples: %i\n", m->group_id(), m->channel_id(), m->timestamp(),  s.size());

      /*
      printf("Ch: %i Ts: %i Samples: %i Trace: ", m->channel_id(), m->timestamp(),  s.size());
      for(int i = 0; i < s.size(); i++)
	     printf(" %02i,", s[i]);
      printf("\n");
      */

      // Fill some nice histos
      Int_t triggerType =  static_cast<Int_t>(m->hit_type());
      Int_t stopType = static_cast<Int_t>(m->stop_type());
      Int_t time = m->timestamp();
      Int_t infoType = -1;
      Int_t channel = m->channel_id();
      Int_t groupId = m->group_id();
      Int_t bufferOverflowCounter = 0;
      Int_t samples = m->samples().size();
      Int_t padRow(-1);
      Int_t padChannel = GetChannelOnPadPlane(channel,groupId);
      Int_t counter1=0;
      Int_t maxADC(-256), maxTB(-1);
      Int_t* sample_values = new Int_t[samples];
      for (auto x : m->samples()) {
	       sample_values[counter1] = x;
	       //if (sample_values[2]>-150) {
	        //if (!fHighPerformance)
	         //fPulseShape[ ( GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address))*32+channel+16*(groupId%2) ]->Fill(counter1,x);
			   fPulseShape[channel+16*(groupId%2)]->Fill(counter1,x);
			   //  fMaxADC[channel]->Fill(counter1,x);
	       //}
  	    if (x > maxADC){
  	      maxADC = x;
  	      maxTB = counter1;
  	    }
	      ++counter1;
      }

      fMaxADC[channel]->Fill(maxADC);
      
      cName.Form("SysCore_%i_Spadic_%i",0,0);
  	  //h = (TH2I*)fHM->H2(TString("Baseline_"+cName).Data());
  	  //h->Fill(sample_values[0],groupId*16+channel);
  	  fBaseline[0]->Fill(sample_values[0],groupId*16+channel);
    }

    else if ( m->is_buffer_overflow() ){
    	LOG(DEBUG) <<  counter << " This is a buffer overflow message" << FairLogger::endl;
    	isOverflow = true;
    	//GetEpochInfo(link, addr);
    	Int_t triggerType = -1;
    	Int_t infoType = -1;
    	Int_t stopType = -1;
    	Int_t groupId = m->group_id();
    	Int_t channel = m->channel_id();
    	Int_t time = m->timestamp();
    	Int_t bufferOverflowCounter = static_cast<Int_t>(m->buffer_overflow_count());
    	Int_t samples = 1;
    	Int_t* sample_values = new Int_t[samples];
    	sample_values[0] = -256;
      /*
    	new( (*fSpadicRaw)[fSpadicRaw->GetEntriesFast()] )
    	  CbmSpadicRawMessage(link, address, channel, fEpochMarker, time,
    			      fSuperEpoch, triggerType, infoType, stopType, groupId,
    			      bufferOverflowCounter, samples, sample_values,
    			      isHit, isInfo, isEpoch, isEpochOutOfSync, isHitAborted, isOverflow, isStrange);
                */
    	delete[] sample_values;
    	fMessageStatistic[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(fMessageTypes[5].Data(),bufferOverflowCounter);
    	channel = GetChannelOnPadPlane(channel,groupId);
    	if (channel > 15){
    	  channel-= 16;
    	  groupId = 1;
    	} else {
    	  groupId = 0;
    	}
    	fLost[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(channel,groupId,bufferOverflowCounter);
    }

    else if ( m->is_info() ){
    	LOG(DEBUG) <<  counter << " This is a info message" << FairLogger::endl;
    	isInfo = true;
    	//GetEpochInfo(link, addr);

    	Int_t triggerType = -1;
    	Int_t infoType = static_cast<Int_t>(m->info_type());
    	Int_t stopType = -1;
    	Int_t groupId = m->group_id();
    	Int_t channel = m->channel_id();
    	Int_t time = m->timestamp();
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
    	fMessageStatistic[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(fInfoTypes[infoType].Data(),1);
    	channel = GetChannelOnPadPlane(channel,groupId);
    	if (channel > 15){
    	  channel-= 16;
    	  groupId = 1;
    	} else {
    	  groupId = 0;
    	}
    	if (!fHighPerformance)fInfo[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(channel,groupId,1);
    }


    else if ( m->is_hit_aborted()) {
    	LOG(DEBUG) <<  counter << " This is a hit message was aborted" << FairLogger::endl;
    	isHitAborted = true;
    	GetEpochInfo(link, addr);
    	Int_t triggerType = -1;
    	Int_t stopType = -1;
    	Int_t time = m->timestamp();
    	Int_t infoType = -1;
    	Int_t groupId = -1;//mp->group_id();//???
    	Int_t bufferOverflowCounter = 0;
    	Int_t samples = 1;
    	Int_t* sample_values = NULL;
    	Int_t channel = m->channel_id();
    	infoType = static_cast<Int_t>(m->info_type());// should here be stoptype instead???
    	sample_values = new Int_t[samples];
    	sample_values[0] = -256;
    	new( (*fSpadicRaw)[fSpadicRaw->GetEntriesFast()] )
    	  CbmSpadicRawMessage(link, address, channel, fEpochMarker, time,
    			      fSuperEpoch, triggerType, infoType, stopType, groupId,
    			      bufferOverflowCounter, samples, sample_values,
    			      isHit, isInfo, isEpoch, isEpochOutOfSync, isHitAborted, isOverflow, isStrange);
    	//++counter;
    	delete[] sample_values;
    	fMessageStatistic[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(fMessageTypes[3].Data(),1);
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
    	fMessageStatistic[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(fMessageTypes[6].Data(),1);
    	//fStrange[GetSyscoreID(link) * NrOfSpadics + GetSpadicID(address)]->Fill(channel,groupId);
    	LOG(DEBUG) <<  counter << " This message type is not hit, info, epoch or overflow and will not be stored in the TClonesArray" << FairLogger::endl;
    	LOG(DEBUG) << " valide:" << m->is_valid() << " epoch marker:" << fEpochMarker << " super epoch marker:" << fSuperEpoch << " time:" << time << " link:" << link << " address:" << address << FairLogger::endl;
    	LOG(DEBUG) << "Channel ID:" << m->channel_id() << FairLogger::endl;
    	//if ( mp->is_hit_aborted() )
    	//LOG(INFO) << "hit is aborted" << FairLogger::endl;
    	//if ( mp->is_hit() )
    	LOG(DEBUG) << "GroupID:" << m->group_id() << "hit: triggerType:" << static_cast<Int_t>(m->hit_type()) << " stopType:" << static_cast<Int_t>(m->stop_type()) << " Nr.of samples:" << m->samples().size() << FairLogger::endl;

    	//if ()
    	for (auto x : m->samples()) {
    	  LOG(DEBUG) << " " << x;
    	}
    	LOG(DEBUG) << FairLogger::endl;
    	//if (mp->is_info())
    	LOG(DEBUG) << "InfoType:" << static_cast<Int_t>(m->info_type()) << FairLogger::endl;
    	//if ()
    	LOG(DEBUG) << "Nr. of overflows:" << static_cast<Int_t>(m->buffer_overflow_count()) << FairLogger::endl;
    	//print_message(mp);
    }
  }
  return kTRUE;
}

void CbmTSUnpackSpadic20OnlineMonitor::print_message(const spadic::Message& m)
{
  printf("PrintMsg\n");
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


void CbmTSUnpackSpadic20OnlineMonitor::FillEpochInfo(Int_t link, Int_t addr, Int_t epoch_count)
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

void CbmTSUnpackSpadic20OnlineMonitor::GetEpochInfo(Int_t link, Int_t addr)
{
  printf("Get epoch info\n");
  auto it=groupToExpMap.find(link);
  if (it == groupToExpMap.end()) {
    LOG(FATAL) << "Could not find an entry for equipment ID" <<
      std::hex << link << std::dec << FairLogger::endl;
  } else {
    fEpochMarker = fEpochMarkerArray[it->second][addr];
    fSuperEpoch = fSuperEpochArray[it->second][addr];
  }

}

Int_t CbmTSUnpackSpadic20OnlineMonitor::GetChannelOnPadPlane(Int_t SpadicChannel, Int_t groupId)
{
  if(SpadicChannel > 31 || groupId > 1) LOG(DEBUG) << "CbmTSUnpackSpadic20OnlineMonitor::                     ChId " << SpadicChannel << "  GroupId: " << groupId << FairLogger::endl;
  SpadicChannel = groupId * 16 + SpadicChannel;
  Int_t channelMapping[32] = {31,15,30,14,29,13,28,12,27,11,26,10,25, 9,24, 8,
			      23, 7,22, 6,21, 5,20, 4,19, 3,18, 2,17, 1,16, 0};
  if (SpadicChannel < 0 || SpadicChannel > 31){
    if (SpadicChannel !=-1) LOG(DEBUG) << "CbmTSUnpackSpadic20OnlineMonitor::GetChannelOnPadPlane ChId " << SpadicChannel << "  GroupId: " << groupId << FairLogger::endl;
    return -1;
  } else {
    return channelMapping[SpadicChannel];
  }
}

Int_t CbmTSUnpackSpadic20OnlineMonitor::GetSpadicID(Int_t address)
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

Int_t CbmTSUnpackSpadic20OnlineMonitor::GetSyscoreID(Int_t link)
{
  /*
  Int_t SyscoreID=link-BaseEquipmentID;
  if((SyscoreID<0||SyscoreID>3)){
    LOG(DEBUG) << "EqID " << link << " not known." << FairLogger::endl;
    SyscoreID=-1;
  }
  return SyscoreID;
  */
  return 0;   // FIXME!!!!!! Defaults to AFCK 0!!
}
// ----              -------------------------------------------------------


inline TString CbmTSUnpackSpadic20OnlineMonitor::GetSpadicName(Int_t link,Int_t address){

  TString spadicName="";
  Int_t SpadicID=0;
  spadicName="SysCore_"+std::to_string(GetSyscoreID(link))+"_";
  SpadicID=GetSpadicID(address);

  //SpadicID/=2;

  spadicName += "Spadic_";

  spadicName+=std::to_string(SpadicID);

  return spadicName;
}

void CbmTSUnpackSpadic20OnlineMonitor::InitHistos()
{

  #ifdef USE_HTTP_SERVER
  THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
  #endif

  //cout << "InitHistos" << endl;
  TString histName;


  for (Int_t iLink = 0; iLink < NrOfSyscores; iLink++){
    for (Int_t iAddress = 0; iAddress < NrOfSpadics; iAddress++){

    //histName.Form("SysCore_%i_Spadic_%i",iLink,iAddress);
    histName.Form("AFCK_0_Spadic_0");

    fHM->Add(TString("Baseline_"+histName).Data(),new TH2I (TString("Baseline_"+histName).Data(),TString("Baseline_"+histName).Data(), 128,-256.5,255.5,32,-0.5,31.5));
    fBaseline[(iLink)*(NrOfSpadics)+iAddress]=(TH2I*)fHM->H2(TString("Baseline_"+histName).Data());
    fBaseline[(iLink)*(NrOfSpadics)+iAddress]->GetXaxis()->SetTitle("ADC");
    fBaseline[(iLink)*(NrOfSpadics)+iAddress]->GetYaxis()->SetTitle("Channel ID");
    fHM->Add(TString("maxADC_vs_maxTimeBin_"+histName).Data(),new TH2I (TString("maxADC_vs_maxTimeBin_"+histName).Data(),TString("maxADC_vs_maxTimeBin_"+histName).Data(),32,-0.5,31.5, 128,-256.5,255.5));

    fmaxADCmaxTimeBin[(iLink)*(NrOfSpadics)+iAddress]=(TH2I*)fHM->H2(TString("maxADC_vs_maxTimeBin_"+histName).Data());
    fmaxADCmaxTimeBin[(iLink)*(NrOfSpadics)+iAddress]->GetXaxis()->SetTitle("max. Time Bin");
    fmaxADCmaxTimeBin[(iLink)*(NrOfSpadics)+iAddress]->GetYaxis()->SetTitle("max ADC");
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
    fTSGraph[(iLink)*(NrOfSpadics)+iAddress] = new TGraph();

    fHM->Add(TString("HitTimeA_"+histName).Data(),new TH1I (TString("HitTimeA_"+histName).Data(),TString("HitTimeA_"+histName).Data(),300,-299.5,0.5));
    fHitTimeA[(iLink)*(NrOfSpadics)+iAddress]=(TH1I*)fHM->H1(TString("HitTimeA_"+histName).Data());
    fHitTimeA[(iLink)*(NrOfSpadics)+iAddress]->GetXaxis()->SetTitle("SuperEpoch count");
    fHM->Add(TString("HitTimeB_"+histName).Data(),new TH1I (TString("HitTimeB_"+histName).Data(),TString("HitTimeB_"+histName).Data(),300,-299.5,0.5));
    fHitTimeB[(iLink)*(NrOfSpadics)+iAddress]=(TH1I*)fHM->H1(TString("HitTimeB_"+histName).Data());
    fHitTimeB[(iLink)*(NrOfSpadics)+iAddress]->GetXaxis()->SetTitle("SuperEpoch count");
    fHitTimeB[(iLink)*(NrOfSpadics)+iAddress]->SetLineColor(2);
    fHM->Add(TString("MessageStatistic_"+histName).Data(),new TH1I (TString("MessageStatistic_"+histName).Data(),TString("MessageStatistic_"+histName).Data(),24,-0.5,23.5));
    fMessageStatistic[(iLink)*(NrOfSpadics)+iAddress]=(TH1I*)fHM->H1(TString("MessageStatistic_"+histName).Data());
    for (Int_t iM = 0; iM < 7; iM++){
    	fMessageStatistic[(iLink)*(NrOfSpadics)+iAddress]->GetXaxis()->SetBinLabel(iM+1,fMessageTypes[iM]);
    }
    for (Int_t iT = 0; iT < 4; iT++){
    	fMessageStatistic[(iLink)*(NrOfSpadics)+iAddress]->GetXaxis()->SetBinLabel(7+1+iT,fTriggerTypes[iT]);
    }
    for (Int_t iS = 0; iS < 6; iS++){
    	fMessageStatistic[(iLink)*(NrOfSpadics)+iAddress]->GetXaxis()->SetBinLabel(7+4+1+iS,fStopTypes[iS]);
    }
    for (Int_t iI = 0; iI < 7; iI++){
    	fMessageStatistic[(iLink)*(NrOfSpadics)+iAddress]->GetXaxis()->SetBinLabel(7+4+6+1+iI,fInfoTypes[iI]);
    }

      // const Int_t nBins=70;
      // auto BinBoarders = [&nBins] (Int_t reBinHigh=32){
      // //auto BinBoarders = [&nBins] (Int_t reBinLow=32){
      // const Double_t Threshold = 100.;
      // const Double_t MaxFreq = (17500000.0);
      // const Double_t slope=MaxFreq/nBins;
      // const Int_t iThreshhold=std::ceil((MaxFreq/Threshold-0.5)/static_cast<Double_t>(reBinHigh));
      // LOG(FATAL) << "iThresshold = " << iThreshhold << FairLogger::endl;
      // Double_t* Result=new Double_t[nBins+1];
      //
      // for(Int_t i=0;i<nBins;i++){
      //  size_t j=nBins-i;
      //  //Result[j]=TMath::Power(TMath::E(),i*slope+TMath::Log(2));
      //  //printf("%d\n",Result[j]);
      //  Double_t LowEdge = MaxFreq/(reBinHigh*(i*i)+0.5);
      //  Result[j]=LowEdge;
      // }/*
      //   for(Int_t i=0;i<nBins-iThreshhold;i++) {
      //   size_t j=nBins-iThreshhold-i;
      //   Double_t LowEdge = MaxFreq/(reBinHigh*(iThreshhold)+0.5+RebinLow*(i));
      //   Result[j]=LowEdge;
      //   }*/
      // Result[0]=0.0;
      // std::sort(Result,&Result[nBins]);
      // return Result;
      // };
      // Double_t* freqbins = (BinBoarders());

      const Int_t nBins = 234;
      const Double_t maxFreq = (175000000.0);
      auto BinBoarders = [&nBins] (Int_t reBinHigh=32)
      	{
      	  const Double_t MaxFreq = (175000000.0);
      	  const Double_t slope=10;//MaxFreq/nBins;
      	  Double_t* Result=new Double_t[nBins+1];
      	  for(Int_t i=0;i<nBins;i++)
      	    {
      	      size_t j=nBins-i;
      	      Double_t LowEdge = MaxFreq/(TMath::Power(TMath::E(),(0.1*Int_t(i))));
      	      //printf("LowEdge: %e\n",LowEdge);
      	      Result[j]=LowEdge;
      	      //Result[j]=TMath::Power(TMath::E(),i*slope+TMath::Log(2));
      	    }
      	  Result[0]=0.1;
      	  std::sort(Result,&Result[nBins]);
      	  return Result;
      	};
      Double_t* freqbins = (BinBoarders());
      fHM->Add(TString("HitFrequency_"+histName).Data(),new TH1I (TString("HitFrequency_"+histName).Data(),TString("HitFrequency_"+histName).Data(),nBins,freqbins/*100000,1,10000000*/));
      fHitFrequency[(iLink)*(NrOfSpadics)+iAddress]=(TH1I*)fHM->H1(TString("HitFrequency_"+histName).Data());
      fHitFrequency[(iLink)*(NrOfSpadics)+iAddress]->GetXaxis()->SetTitle("Trigger frequency (Hz)");
      fHitFrequency[(iLink)*(NrOfSpadics)+iAddress]->GetYaxis()->SetTitle("Channel ID");
      fHM->Add(TString("Spectrum_"+histName).Data(),new TH1I (TString("Spectrum_"+histName).Data(),TString("Spectrum_"+histName).Data(),513,-256.5,256.5));
      fSpectrum[(iLink)*(NrOfSpadics)+iAddress]=(TH1I*)fHM->H1(TString("Spectrum_"+histName).Data());
      fSpectrum[(iLink)*(NrOfSpadics)+iAddress]->GetXaxis()->SetTitle("max. ADC (ADC)");



      for (Int_t iCh = 0; iCh < 32; iCh++){
      	TString chName;
      	chName.Form("Ch%02i_",iCh);
        // FIXME!!!
      	fHM->Add(TString("PulseShape_"+chName+histName).Data(),new TH2I (TString("PulseShape_"+chName+histName).Data(),TString("PulseShape_"+chName+histName).Data(),32,-0.5,31.5, 128,-256.5,255.5));
      	fPulseShape[((iLink)*(NrOfSpadics)+iAddress)*32+iCh]=(TH2I*)fHM->H2(TString("PulseShape_"+chName+histName).Data());
      	fPulseShape[((iLink)*(NrOfSpadics)+iAddress)*32+iCh]->GetXaxis()->SetTitle("Time Bin");
      	fPulseShape[((iLink)*(NrOfSpadics)+iAddress)*32+iCh]->GetYaxis()->SetTitle("ADC");
      	fPulseShape[((iLink)*(NrOfSpadics)+iAddress)*32+iCh]->GetZaxis()->SetTitle("counts");
      }

      for (Int_t iCh = 0; iCh < 32; iCh++){
      	TString chName;
      	chName.Form("Ch%02i_",iCh);
        // FIXME!!!
      	fHM->Add(TString("MaxADC_"+chName+histName).Data(),new TH1D (TString("MaxADC_"+chName+histName).Data(),TString("MaxADC_"+chName+histName).Data(),256,-256.5,255.5));
      	fMaxADC[iCh]=(TH1D*)fHM->H1(TString("MaxADC_"+chName+histName).Data());
      	fMaxADC[iCh]->GetXaxis()->SetTitle("ADC");
      	fMaxADC[iCh]->GetYaxis()->SetTitle("counts");
      }

    }
  }
}




void CbmTSUnpackSpadic20OnlineMonitor::InitCanvas()
{
  TString cName;
  if (!fHighPerformance){
    fcE= new TCanvas(TString("EpochMap").Data(),TString("EpochMap").Data(),1600,1200);
    fcE->Divide(3,4);
    fcO= new TCanvas(TString("OutOfSyncMap").Data(),TString("OutOfSyncMap").Data(),1600,1200);
    fcO->Divide(3,4);
    fcS= new TCanvas(TString("StrangeMap").Data(),TString("StrangeMap").Data(),1600,1200);
    fcS->Divide(3,4);
    fcI= new TCanvas(TString("InfoMap").Data(),TString("InfoMap").Data(),1600,1200);
    fcI->Divide(3,4);
    fcF= new TCanvas(TString("HitFrequency").Data(),TString("HitFrequency").Data(),1600,1200);
    fcF->Divide(3,4);
    fcSp= new TCanvas(TString("HitSelfTriggersSpectrum").Data(),TString("HitSelfTriggersSpectrum").Data(),1600,1200);
    fcSp->Divide(3,4);
  }
  fcB = new TCanvas(TString("Baseline").Data(),TString("Baseline").Data(),1600,1200);
  fcB->Divide(3,4);
  fcM/*[(iLink)*(NrOfSpadics)+iAddress]*/ = new TCanvas(TString("maxADC_vs_maxTimeBin").Data(),TString("maxADC_vs_maxTimeBin").Data(),1600,1200);
  fcM->Divide(3,4);
  fcH= new TCanvas(TString("HitMap").Data(),TString("HitMap").Data(),1600,1200);
  fcH->Divide(3,4);
  fcL= new TCanvas(TString("LostMap").Data(),TString("LostMap").Data(),1600,1200);
  fcL->Divide(3,4);
  fcTS= new TCanvas(TString("TSGraph").Data(),TString("TSGraph").Data(),1600,1200);
  fcTS->Divide(3,4);
  fcMS= new TCanvas(TString("MessageStatistic").Data(),TString("MessageStatistic").Data(),1600,1200);
  fcMS->Divide(3,4);

  for (Int_t iLink = 0; iLink < NrOfSyscores; iLink++){
    for (Int_t iAddress = 0; iAddress < NrOfSpadics; iAddress++){
      cName.Form("SysCore_%i_Spadic_%i",iLink,iAddress);
      if (!fHighPerformance){
      	fcPS[(iLink)*(NrOfSpadics)+iAddress] = new TCanvas(TString("PulseShapes_"+cName).Data(),TString("PulseShapes_"+cName).Data(),1600,1200);
      	fcPS[(iLink)*(NrOfSpadics)+iAddress]->Divide(4,8);
	      for(Int_t iCh = 0; iCh < 32; iCh++){
      	  fcPS[(iLink)*(NrOfSpadics)+iAddress]->cd(iCh+1);
      	  //fPulseShape[((iLink)*(NrOfSpadics)+iAddress)*32+iCh]->Draw("colz");
          fPulseShape[((iLink)*(NrOfSpadics)+iAddress)*32+iCh]->Draw("col");
      	}
        for(Int_t iCh = 0; iCh < 32; iCh++){
      	  fcPS[(iLink)*(NrOfSpadics)+iAddress]->cd(iCh+1);
          fMaxADC[((iLink)*(NrOfSpadics)+iAddress)*32+iCh]->Draw("");
      	}
      	fcE->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz(0);
      	fEpoch[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      	fcO->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz(0);
      	fOutOfSync[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      	fcS->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz(0);
      	fStrange[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      	fcI->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogx(0);
      	fInfo[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      	fcF->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogx(1);
      	fcF->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogy(1);
      	fcF->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz(0);
      	fHitFrequency[(iLink)*(NrOfSpadics)+iAddress]->Draw(/*"colz"*/);
      	fcSp->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogy(1);
      	fSpectrum[(iLink)*(NrOfSpadics)+iAddress]->Draw();
      }
      fcB->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz();
      fBaseline[(iLink)*(NrOfSpadics)+iAddress]->Draw("col");
      cName.Form("AFCK_%i_Spadic_%i",iLink,iAddress);
      fcM->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz();
      fmaxADCmaxTimeBin[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      fcH->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz(0);
      fHit[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      fcL->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogz(0);
      fLost[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
      fcMS->cd((iLink)*(NrOfSpadics)+iAddress+1)->SetLogy(1);
      fMessageStatistic[(iLink)*(NrOfSpadics)+iAddress]->Draw("");
      fcTS->cd((iLink)*(NrOfSpadics)+iAddress+1);
      fHitTimeA[(iLink)*(NrOfSpadics)+iAddress]->GetYaxis()->SetRangeUser(0,1000);
      fHitTimeA[(iLink)*(NrOfSpadics)+iAddress]->Draw("");
      fHitTimeB[(iLink)*(NrOfSpadics)+iAddress]->Draw("same");
      //fTSGraph[(iLink)*(NrOfSpadics)+iAddress]->Draw("ALP");


    }
  }

}



void CbmTSUnpackSpadic20OnlineMonitor::UpdateCanvas()
{
  /*
    //cout << "UpdateCanvas" << endl;
    TH2I* h = NULL;
    TString cName;
    for (Int_t iLink = 0; iLink < NrOfSyscores; iLink++){
      for (Int_t iAddress = 0; iAddress < NrOfSpadics; iAddress++){
        cName.Form("AFCK_%i_Spadic_%i",iLink,iAddress);
        fcB->cd((iLink)*(NrOfSpadics)+iAddress+1);
        fBaseline[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
        h = (TH2I*)fHM->H2(TString("Baseline_"+cName).Data());
        h->Draw("colz");
        fcB->cd((iLink)*(NrOfSpadics)+iAddress+1)->Update();
        cName.Form("AFCK_%i_Spadic_%i",iLink,iAddress);
        fcM->cd((iLink)*(NrOfSpadics)+iAddress+1);
        fmaxADCmaxTimeBin[(iLink)*(NrOfSpadics)+iAddress]->Draw("colz");
        h = (TH2I*)fHM->H2(TString("maxADC_vs_maxTimeBin_"+cName).Data());
        h->Draw("colz");
        fcM->cd((iLink)*(NrOfSpadics)+iAddress+1)->Update();
      }
    }
    */
}

void CbmTSUnpackSpadic20OnlineMonitor::Reset()
{
  fSpadicRaw->Clear();
}
void CbmTSUnpackSpadic20OnlineMonitor::FinishEvent()
{
  //UpdateCanvas();
}

void CbmTSUnpackSpadic20OnlineMonitor::Finish()
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
    void CbmTSUnpackSpadic20OnlineMonitor::Register()
    {
    }
  */


  ClassImp(CbmTSUnpackSpadic20OnlineMonitor)
