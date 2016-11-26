// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSMonitorMuch                            -----
// -----               Created 11.11.2016 by V. Singhal and A. Kumar                      -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTSMonitorMuch.h"
#include "CbmMuchUnpackPar.h"

#include "CbmMuchAddress.h"
#include "CbmHistManager.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRunOnline.h"

#include "TClonesArray.h"
#include "TString.h"
#include "TRandom.h"
#include "THttpServer.h"

#include <iostream>
#include <stdint.h>
#include <iomanip>

class CbmMuchAddress;
using std::hex;
using std::dec;
using namespace std;

CbmTSMonitorMuch::CbmTSMonitorMuch()
  : CbmTSUnpack(),
    fMsgCounter(11,0), // length of enum MessageTypes initialized with 0
    fNdpbIdIndexMap(),
    fMuchStationMapX(),
    fMuchStationMapY(),
    fHM(new CbmHistManager()),
    fCurrentEpoch(),
    fNofEpochs(0),
    fCurrentEpochTime(0.),
    fdStartTime( -1 ),
    fEquipmentId(0),
	fUnpackPar(NULL)
{
}

CbmTSMonitorMuch::~CbmTSMonitorMuch()
{
}

Bool_t CbmTSMonitorMuch::Init()
{
  LOG(INFO) << "Initializing flib nxyter unpacker for MUCH" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  return kTRUE;
}

void CbmTSMonitorMuch::SetParContainers()
{
	LOG(INFO) << "Setting parameter containers for " << GetName()
			<< FairLogger::endl;
	fUnpackPar = (CbmMuchUnpackPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmMuchUnpackPar"));
}


Bool_t CbmTSMonitorMuch::InitContainers()
{
	LOG(INFO) << "Init parameter containers for " << GetName()
			<< FairLogger::endl;
         
   Bool_t bReInit = ReInitContainers();
   CreateHistograms();
   
   return bReInit;
}

Bool_t CbmTSMonitorMuch::ReInitContainers()
{
	LOG(INFO) << "ReInit parameter containers for " << GetName()
			<< FairLogger::endl;

	Int_t NrOfnDpbsModA = fUnpackPar->GetNrOfnDpbsModA();

	LOG(INFO) << "Nr. of nDPBs Mod. A: " << NrOfnDpbsModA
    		<< FairLogger::endl;
      
   fNdpbIdIndexMap.clear();
   for (Int_t i = 0; i< NrOfnDpbsModA; ++i) 
   {
     fNdpbIdIndexMap[fUnpackPar->GetNdpbIdA(i)] = i;
     LOG(INFO) << "nDPB Id of MUCH  " << i
               << " : 0x" << std::hex << fUnpackPar->GetNdpbIdA(i)
               << std::dec
               << FairLogger::endl;
   } // for (Int_t i = 0; i< NrOfnDpbsModA; ++i) 

	Int_t NrOfFebs = fUnpackPar->GetNrOfFebs();

	LOG(INFO) << "Nr. of FEBs : " << NrOfFebs
    		<< FairLogger::endl;


	Int_t NrOfChannels = fUnpackPar->GetNrOfChannels();
	LOG(INFO) << "Nr. of Channels : " << NrOfChannels
    		<< FairLogger::endl;

	// Need to clear the 2 Arrays fMuchStationMapX and MapY

	// log all the DPBs number

	// Filling all the Mapping values in the 2 X and Y arrays
	//	Int_t nrOfChannels = fUnpackPar->GetNumberOfChannels();
	
	for (Int_t febId = 0 ; febId<NrOfFebs; febId++){// looping on all the FEB IDs
		for (Int_t channelId=0; channelId<NrOfChannels; channelId++){

			//Have to check GetPadX and GetPadY values.
//			LOG(INFO) << "Value of GetPadX " << 	fUnpackPar->GetPadX(febId,channelId) << FairLogger::endl;
			fMuchStationMapX[febId][channelId] = fUnpackPar->GetPadX(febId,channelId);
//			LOG(INFO) << "Value of GetPadY " << 	fUnpackPar->GetPadY(febId,channelId) <<FairLogger::endl;		
			fMuchStationMapY[febId][channelId] = fUnpackPar->GetPadY(febId,channelId);
		}
	}

	return kTRUE;
}

void CbmTSMonitorMuch::CreateHistograms()
{
#ifdef USE_HTTP_SERVER
    THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
#endif

	TString sHistName{""};
    	TString title{""};

  sHistName = "hMessageTypeMuch";
  title = "Nb of message for each type; Type";
  TH1I* hMessageType = new TH1I(sHistName, title, 16, 0., 16.);
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_NOP,      "NOP");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_HIT,      "HIT");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_EPOCH,    "EPOCH");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_SYNC,     "SYNC");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_AUX,      "AUX");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_EPOCH2,   "EPOCH2");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4,     "GET4");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_SYS,      "SYS");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4_SLC, "MSG_GET4_SLC");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4_32B, "MSG_GET4_32B");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4_SYS, "MSG_GET4_SYS");
  hMessageType->GetXaxis()->SetBinLabel(1 + 15, "GET4 Hack 32B");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_NOP,      "NOP");
  fHM->Add(sHistName.Data(), hMessageType);
#ifdef USE_HTTP_SERVER
      if (server) server->Register("/MuchRaw", fHM->H1(sHistName.Data()));
#endif

  sHistName = "hSysMessTypeMuch";
  title = "Nb of system message for each type; System Type";
  TH1I* hSysMessType = new TH1I(sHistName, title, 17, 0., 17.);
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_NX_PARITY,       "NX PARITY");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_SYNC_PARITY,     "SYNC PARITY");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_RESUME,      "DAQ RESUME");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_FIFO_RESET,      "FIFO RESET");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_USER,            "USER");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_PCTIME,          "PCTIME");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_ADC,             "ADC");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_PACKETLOST,      "PACKET LOST");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_GET4_EVENT,      "GET4 ERROR");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_CLOSYSYNC_ERROR, "CLOSYSYNC ERROR");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_TS156_SYNC,        "TS156 SYNC");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_GDPB_UNKWN,        "UNKW GET4 MSG");
  hSysMessType->GetXaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
  fHM->Add(sHistName.Data(), hSysMessType);
#ifdef USE_HTTP_SERVER
  if (server) server->Register("/MuchRaw", fHM->H1(sHistName.Data()));
#endif

	for ( Int_t febId = 0 ; 
         febId < fUnpackPar->GetNrOfnDpbsModA()*fUnpackPar->GetNrOfFebsPerNdpb(); 
         febId++){// looping on all the FEB IDs
		sHistName = Form("Chan_Counts_Much_%02u", febId);
		title = Form("Channel counts Much FEB %02u; channel; Counts", febId);
		fHM->Add( sHistName.Data(), new TH1F( sHistName.Data(), title.Data(), 128, 0, 127) );
#ifdef USE_HTTP_SERVER
    if (server) server->Register("/MuchRaw", fHM->H1(sHistName.Data()));
#endif
      
		sHistName = Form("Raw_ADC_Much_%02u", febId);
		title = Form("Raw ADC Much FEB %02u; channel; ADC value", febId);
		fHM->Add( sHistName.Data(), new TH2F( sHistName.Data(), title.Data(), 128, 0, 127, 4096, 0, 4095) );
#ifdef USE_HTTP_SERVER
    if (server) server->Register("/MuchRaw", fHM->H2(sHistName.Data()));
#endif

		sHistName = Form("FebRate_%02u", febId);
		title = Form("Counts per second in FEB%02u; Time[s] ; Counts", febId);
		fHM->Add( sHistName.Data(), new TH1F( sHistName.Data(), title.Data(), 1800, 0, 1800 ) );
#ifdef USE_HTTP_SERVER
    if (server) server->Register("/MuchRaw", fHM->H1(sHistName.Data()));
#endif
  }

	sHistName = "Pad_Distribution";
	title = "Pad_Distribution; Sectors in Horizontal Direction; Channels in Vertical Direction";
	fHM->Add( sHistName.Data(), new TH2F(sHistName.Data(), title.Data(), 79, -0.5, 78.5, 23, -0.5, 22.5) );

/*	TH2* histPadDistr = fHM->H2("Pad_Distribution");
	for( Int_t iFeb = 0; iFeb < 15; iFeb ++)
		for( Int_t iChan = 0; iChan <  128; iChan ++)
		{
			Int_t sector  = fUnpackPar->GetPadX(iFeb, iChan);
			Int_t channel = fUnpackPar->GetPadY(iFeb, iChan);
			if(!(sector<0||channel<0)){
	
			histPadDistr->Fill((78-sector),(22-channel), (iFeb + 1));
			if (sector==25 && channel==0) cout <<	"Pad X value" << fUnpackPar->GetPadX(iFeb, iChan) << "Pad Y value" << fUnpackPar->GetPadY(iFeb, iChan) << endl;
		}
	}
//	Int_t iFeb = 3, iChan =32;	

*/	
#ifdef USE_HTTP_SERVER
    if (server) server->Register("/MuchRaw", fHM->H2(sHistName.Data()));
#endif
}

Bool_t CbmTSMonitorMuch::DoUnpack(const fles::Timeslice& ts, size_t component)
{

  LOG(DEBUG) << "Timeslice contains " << ts.num_microslices(component)
             << "microslices." << FairLogger::endl;
  
   TH1* histMessType = fHM->H1("hMessageTypeMuch");
   TH1* histSysMessType = fHM->H1("hSysMessTypeMuch");
   
  std::vector<TH1*> Chan_Counts_Much;
  std::vector<TH2*> Raw_ADC_Much;
  std::vector<TH1*> FebRate;

  TString sHistName{""};
  TString title{""};

  for ( Int_t febId = 0 ;
		  febId < fUnpackPar->GetNrOfnDpbsModA()*fUnpackPar->GetNrOfFebsPerNdpb();
		  febId++){// looping on all the FEB IDs
      sHistName = Form("Chan_Counts_Much_%02u", febId);
      Chan_Counts_Much.push_back(fHM->H1(sHistName.Data()));
      sHistName = Form("Raw_ADC_Much_%02u", febId);
      Raw_ADC_Much.push_back(fHM->H2(sHistName.Data()));
      sHistName = Form("FebRate_%02u", febId);
      FebRate.push_back(fHM->H1(sHistName.Data()));
  }
  TH2* histPadDistr = fHM->H2("Pad_Distribution");

   Int_t messageType = -111;
  // Loop over microslices
  for (size_t m = 0; m < ts.num_microslices(component); ++m)
    {

      constexpr uint32_t kuBytesPerMessage = 8;

      auto msDescriptor = ts.descriptor(component, m);
      fEquipmentId = msDescriptor.eq_id;
      const uint8_t* msContent = reinterpret_cast<const uint8_t*>(ts.content(component, m));

      uint32_t size = msDescriptor.size;
      LOG(DEBUG) << "Microslice: " << msDescriptor.idx 
                << " has size: " << size << FairLogger::endl; 

      // If not integer number of message in input buffer, print warning/error
      if( 0 != (size % kuBytesPerMessage) )
        LOG(ERROR) << "The input microslice buffer does NOT " 
                   << "contain only complete nDPB messages!" 
                   << FairLogger::endl;

      // Compute the number of complete messages in the input microslice buffer
      uint32_t uNbMessages = (size - (size % kuBytesPerMessage) )
                              / kuBytesPerMessage;
      
      // Prepare variables for the loop on contents
      const uint64_t* pInBuff = reinterpret_cast<const uint64_t*>( msContent );
      for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)
        {
          // Fill message
          uint64_t ulData = static_cast<uint64_t>( pInBuff[uIdx] );
          ngdpb::Message mess( ulData );

          if(gLogger->IsLogNeeded(DEBUG)) {
            mess.printDataCout();
          }

          // Increment counter for different message types 
          // and fill the corresponding histogram
          messageType = mess.getMessageType();
          fMsgCounter[messageType]++;
          histMessType->Fill(messageType);
          
          switch( messageType ) {
          case ngdpb::MSG_HIT: 
            FillHitInfo(mess, Chan_Counts_Much, Raw_ADC_Much, FebRate, histPadDistr);
            break;
          case ngdpb::MSG_EPOCH:
            FillEpochInfo(mess);
            break;
          case ngdpb::MSG_SYNC:
            // Do nothing, this message is just there to make sure we get all Epochs
            break;
          case ngdpb::MSG_SYS:
            // Just keep track of which type of System message we receive
            histSysMessType->Fill(mess.getSysMesType());
            break;
          default: 
            LOG(ERROR) << "Message type " << std::hex << std::setw(2) 
                       << static_cast< uint16_t >( mess.getMessageType() )
                       << " not yet include in nXYTER unpacker."
                       << FairLogger::endl;
          }
          


        } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)
      
    }


  return kTRUE;
}

void CbmTSMonitorMuch::FillHitInfo(ngdpb::Message mess, std::vector<TH1*> Chan_Counts_Much,
                                   std::vector<TH2*> Raw_ADC_Much,
                                   std::vector<TH1*> FebRate,
                                   TH2* histPadDistr)
{
  // --- Get absolute time, NXYTER and channel number
  Int_t rocId      = mess.getRocNumber();
  Int_t nxyterId   = mess.getNxNumber();
  Int_t nxChannel  = mess.getNxChNum(); 
  Int_t charge     = mess.getNxAdcValue();

   // First check if nDPB is mapped
   if( fNdpbIdIndexMap.end() == fNdpbIdIndexMap.find( rocId ) )
   {
		LOG(FATAL) << "Unmapped nDPB Id " << std::hex << rocId << std::dec << FairLogger::endl;
   } // if( fNdpbIdIndexMap.end() == fNdpbIdIndexMap.find( rocId ) )
 
  ULong_t hitTime  = mess.getMsgFullTime(fCurrentEpoch[rocId][nxyterId]);
 
  LOG(DEBUG) << "Hit: " << rocId << ", " << nxyterId 
             << ", " << nxChannel << ", " << charge << FairLogger::endl;

  //here converting channel number into the MUCH Digi.

	Int_t address = CreateAddress(rocId,nxyterId,0, 0, 0, 0, nxChannel, histPadDistr);
	if (address){	
		LOG(DEBUG) << "Create digi with time " << hitTime
               << " at epoch " << fCurrentEpoch[rocId][nxyterId] << FairLogger::endl;
	}
	else {
		LOG(ERROR) << "Unknown Roc Id " << rocId << " or nxyterId "<< nxyterId << " or channelId "
                 << nxChannel << FairLogger::endl;
	}
	Int_t channelNr = fNdpbIdIndexMap[rocId]*fUnpackPar->GetNrOfFebsPerNdpb() + nxyterId;
	Chan_Counts_Much[channelNr]->Fill(nxChannel);
	Raw_ADC_Much[channelNr]->Fill(nxChannel, charge);
	histPadDistr->Fill(nxChannel,charge);

   if( fCurrentEpoch.end() != fCurrentEpoch.find( rocId ) ) {
      if( fCurrentEpoch[rocId].end() != fCurrentEpoch[rocId].find( nxyterId ) ) {
         if( fdStartTime <= 0 )
         {
            fdStartTime = mess.getMsgFullTimeD( fCurrentEpoch[rocId][nxyterId] );
            
           LOG(INFO) << "Start time set to " << (fdStartTime/1e9) 
                     << " s using first hit on channel " << nxChannel 
                     << " of FEB " << nxyterId 
                     << " on nDPB " << fNdpbIdIndexMap[rocId] 
                     << " in epoch " << fCurrentEpoch[rocId][nxyterId] << FairLogger::endl;
         }
            
         if( 0 < fdStartTime )
        	FebRate[channelNr]->Fill( 1e-9*( mess.getMsgFullTimeD( fCurrentEpoch[rocId][nxyterId] )
                                      - fdStartTime)  );
     }
   }

}

Int_t CbmTSMonitorMuch::CreateAddress(Int_t rocId, Int_t febId, Int_t stationId,
		Int_t layerId, Int_t sideId, Int_t moduleId, Int_t channelId,
		TH2* histPadDistr)
{
	Int_t febNr = fNdpbIdIndexMap[rocId]*fUnpackPar->GetNrOfFebsPerNdpb() + febId;
	Int_t sector  = fUnpackPar->GetPadX(febNr, channelId);
	Int_t channel = fUnpackPar->GetPadY(febNr, channelId);
   
	Int_t address = CbmMuchAddress::GetAddress(stationId, layerId, sideId, moduleId, sector, channel);
	if(!(sector<0||channel<0)){
		
		histPadDistr->Fill(78-sector,22-channel);
		
	}
//	fHM->H2("Pad_Distribution")->Fill(sector,channel);
	
	return address;
}

void CbmTSMonitorMuch::FillEpochInfo(ngdpb::Message mess)
{
  Int_t rocId          = mess.getRocNumber();
  Int_t nxyterId       = mess.getEpochNxNum();

   // First check if nDPB is mapped
   if( fNdpbIdIndexMap.end() == fNdpbIdIndexMap.find( rocId ) )
   {
		LOG(FATAL) << "Unmapped nDPB Id " << std::hex << rocId << std::dec << FairLogger::endl;
   } // if( fNdpbIdIndexMap.end() == fNdpbIdIndexMap.find( rocId ) )
   
  fCurrentEpoch[rocId][nxyterId] = mess.getEpochNumber();

  //  LOG(INFO) << "Epoch message for ROC " << rocId << " with epoch number "
  //            << fCurrentEpoch[rocId] << FairLogger::endl;
  fCurrentEpochTime = mess.getMsgFullTime(fCurrentEpoch[rocId][nxyterId]);
  fNofEpochs++;
  LOG(DEBUG) << "Epoch message "
             << fNofEpochs << ", epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][nxyterId])
             << ", time " << std::setprecision(9) << std::fixed
             << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
             << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
             << FairLogger::endl;
}

void CbmTSMonitorMuch::Reset()
{
}

void CbmTSMonitorMuch::Finish()
{
  TString message_type;

  for (unsigned int i=0; i< fMsgCounter.size(); ++i) {
    switch(i) {
    case 0: message_type ="NOP"; break;
    case 1: message_type ="HIT"; break;
    case 2: message_type ="EPOCH"; break;
    case 3: message_type ="SYNC"; break;
    case 4: message_type ="AUX"; break;
    case 5: message_type ="EPOCH2"; break;
    case 6: message_type ="GET4"; break;
    case 7: message_type ="SYS"; break;
    case 8: message_type ="GET4_SLC"; break;
    case 9: message_type ="GET4_32B"; break;
    case 10: message_type ="GET4_SYS"; break;
    }
    LOG(INFO) << message_type << " messages: " 
              << fMsgCounter[i] << FairLogger::endl;
  }

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;                
   for( auto it = fCurrentEpoch.begin(); it != fCurrentEpoch.end(); ++it)
      for( auto itN = (it->second).begin(); itN != (it->second).end(); ++itN)
      LOG(INFO) << "Last epoch for nDPB: " 
                << std::hex << std::setw(4) << it->first << std::dec
                << " , FEB  " << std::setw(4) << itN->first 
                << " => " << itN->second 
                << FairLogger::endl;
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;

   gDirectory->mkdir("Much_Raw");
   gDirectory->cd("Much_Raw");
	for ( Int_t febId = 0 ; 
         febId < fUnpackPar->GetNrOfnDpbsModA()*fUnpackPar->GetNrOfFebsPerNdpb(); 
         febId++){// looping on all the FEB IDs
     fHM->H1( Form("Chan_Counts_Much_%02u", febId) )->Write();
     fHM->H2( Form("Raw_ADC_Much_%02u", febId) )->Write();  
   }      
   fHM->H2("Pad_Distribution")->Write();        
   gDirectory->cd("..");

}


void CbmTSMonitorMuch::FillOutput(CbmDigi* digi)
{
}

ClassImp(CbmTSMonitorMuch)