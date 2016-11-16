// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSUnpackMuch                            -----
// -----               Created 11.11.2016 by V. Singhal and A. Kumar                      -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTSUnpackMuch.h"
#include "CbmMuchUnpackPar.h"

#include "CbmTbDaqBuffer.h"

//#include "CbmMuchAddress.h"
#include "CbmHistManager.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

#include "TClonesArray.h"
#include "TString.h"
#include "TRandom.h"

#include <iostream>
#include <stdint.h>
#include <iomanip>

class CbmMuchAddress;
using std::hex;
using std::dec;
using namespace std;

CbmTSUnpackMuch::CbmTSUnpackMuch()
  : CbmTSUnpack(),
    fMsgCounter(11,0), // length of enum MessageTypes initialized with 0
    fMuchStationMapX(),
    fMuchStationMapY(),
    fHM(new CbmHistManager()),
    fCurrentEpoch(),
    fNofEpochs(0),
    fCurrentEpochTime(0.),
    fEquipmentId(0),
    fMuchRaw(new TClonesArray("CbmNxyterRawMessageForMUCH", 10)),
    fMuchDigi(new TClonesArray("CbmSpsMuchDigi", 10)),
    fRawMessage(NULL),  
    fDigi(NULL),
    fBuffer(CbmTbDaqBuffer::Instance()),
	fCreateRawMessage(kFALSE),
	fUnpackPar(NULL)
{
}

CbmTSUnpackMuch::~CbmTSUnpackMuch()
{
}

Bool_t CbmTSUnpackMuch::Init()
{
  LOG(INFO) << "Initializing flib nxyter unpacker for MUCH" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  if (fCreateRawMessage) {
    ioman->Register("MuchRawMessage", "MUCH raw data", fMuchRaw, kTRUE);
  }
  ioman->Register("MuchDigi", "Sps Much digi", fMuchDigi, kTRUE);

  CreateHistograms();
  return kTRUE;
}

void CbmTSUnpackMuch::SetParContainers()
{
	LOG(INFO) << "Setting parameter containers for " << GetName()
			<< FairLogger::endl;
	fUnpackPar = (CbmMuchUnpackPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmMuchUnpackPar"));
}


Bool_t CbmTSUnpackMuch::InitContainers()
{
	LOG(INFO) << "Init parameter containers for " << GetName()
			<< FairLogger::endl;
	return ReInitContainers();

}

Bool_t CbmTSUnpackMuch::ReInitContainers()
{
	LOG(INFO) << "ReInit parameter containers for " << GetName()
			<< FairLogger::endl;

	Int_t NrOfnDpbsModA = fUnpackPar->GetNrOfnDpbsModA();

	LOG(INFO) << "Nr. of nDPBs : " << NrOfnDpbsModA
    		<< FairLogger::endl;

	Int_t NrOfFebs = fUnpackPar->GetNrOfFebs();

	LOG(INFO) << "Nr. of FEBs : " << NrOfFebs
    		<< FairLogger::endl;


	Int_t NrOfChannels = fUnpackPar->GetNrOfChannels();
	LOG(INFO) << "Nr. of Channels : " << NrOfChannels
    		<< FairLogger::endl;

	//fHodoStationMap.clear();
	// Need to clear the 2 Arrays fMuchStationMapX and MapY


/*	for (Int_t i = 0; i< nrOfRocs; ++i) {
	  fHodoStationMap[fUnpackPar->GetRocId(i)] = i;
	  LOG(INFO) << "Roc Id of fiber hodo station " << i
			  << " : " << fUnpackPar->GetRocId(i)
			  << FairLogger::endl;
	}
*/
	// log all the DPBs number

	// Filling all the Mapping values in the 2 X and Y arrays
	//	Int_t nrOfChannels = fUnpackPar->GetNumberOfChannels();
	
	for (Int_t febId = 0 ; febId<NrOfFebs; febId++){// looping on all the FEB IDs
		for (Int_t channelId=0; channelId<NrOfChannels; channelId++){

			//Have to check GetPadX and GetPadY values.
			//LOG(INFO) << "Value of GetPadX " << 	fUnpackPar->GetPadX(febId,channelId) << FairLogger::endl;
			fMuchStationMapX[febId][channelId] = fUnpackPar->GetPadX(febId,channelId);
			//LOG(INFO) << "Value of GetPadX " << 	fUnpackPar->GetPadX(febId,channelId) <<FairLogger::endl;		
			fMuchStationMapY[febId][channelId] = fUnpackPar->GetPadY(febId,channelId);
		}
	}

	return kTRUE;
}

void CbmTSUnpackMuch::CreateHistograms()
{
  
	fHM->Add("Raw_ADC_Much", new TH2F("Raw_ADC_Much", 
                    "Raw_ADC_Much;channel;ADC value", 128, 0, 127, 4096, 0, 4095));   
	fHM->Add("Pad_Distribution", 
           new TH2F("Pad_Distribution", 
                    "Pad_Distribution; Sectors in Horizontal Direction; Channels in Vertical Direction", 79, 0, 78, 23, 0, 22));   


}

Bool_t CbmTSUnpackMuch::DoUnpack(const fles::Timeslice& ts, size_t component)
{

  LOG(DEBUG) << "Timeslice contains " << ts.num_microslices(component)
             << "microslices." << FairLogger::endl;
  
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
          fMsgCounter[mess.getMessageType()]++;
          
          switch(mess.getMessageType()) {
          case ngdpb::MSG_HIT: 
            FillHitInfo(mess);
            break;
          case ngdpb::MSG_EPOCH:
            FillEpochInfo(mess);
            break;
          case ngdpb::MSG_SYNC:
            // Do nothing, this message is just there to make sure we get all Epochs
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

void CbmTSUnpackMuch::FillHitInfo(ngdpb::Message mess)
{
  // --- Get absolute time, NXYTER and channel number
  Int_t rocId      = mess.getRocNumber();
  Int_t nxyterId   = mess.getNxNumber();
  Int_t nxChannel  = mess.getNxChNum(); 
  Int_t charge     = mess.getNxAdcValue();
 
  ULong_t hitTime  = mess.getMsgFullTime(fCurrentEpoch[rocId]);

  if (fCreateRawMessage) {
    fRawMessage = new( (*fMuchRaw)[fMuchRaw->GetEntriesFast()] )
      CbmNxyterRawMessage(fEquipmentId,
                          rocId*4 + nxyterId,   //TODO check
                          nxChannel,
                          fCurrentEpoch[rocId],             // note the trick
                          mess.getNxTs(),
                          charge,
                          mess.getNxLastEpoch(),
                          mess.getNxPileup(),
                          mess.getNxOverflow());
  }
 
  LOG(DEBUG) << "Hit: " << rocId << ", " << nxyterId 
             << ", " << nxChannel << ", " << charge << FairLogger::endl;


  //here converting channel number into the MUCH Digi.

	Int_t address = CreateAddress(rocId,nxyterId,0, 0, 0, 0, nxChannel);
	if (address){	
		LOG(DEBUG) << "Create digi with time " << hitTime
               << " at epoch " << fCurrentEpoch[rocId] << FairLogger::endl;
 
		fDigi = new CbmSpsMuchDigi(address, charge, hitTime);
		fBuffer->InsertData(fDigi);
		LOG(DEBUG) << "Number of Digis created till  now " << fMuchRaw->GetEntriesFast() << endl;
	}
	else {
		LOG(ERROR) << "Unknown Roc Id " << rocId << " or nxyterId "<< nxyterId << " or channelId "<< nxChannel << FairLogger::endl;
	}
	fHM->H2("Raw_ADC_Much")->Fill(nxChannel,charge);
//	fHM->H2("Pad_Distribution")->Fill(nxChannel,charge);

  }





Int_t CbmTSUnpackMuch::CreateAddress(Int_t rocId, Int_t febId, Int_t stationId, Int_t layerId, Int_t sideId, Int_t moduleId, Int_t channelId)
{
	CbmMuchUnpackPar * UnpackerHandle = new CbmMuchUnpackPar();        
	Int_t sector = UnpackerHandle->GetPadX(febId, channelId);
        Int_t channel = UnpackerHandle->GetPadY(febId, channelId);
	Int_t address = CbmMuchAddress::GetAddress(stationId, layerId, sideId, moduleId, sector, channel);
	fHM->H2("Pad_Distribution")->Fill(sector,channel);
	return(address);

}

void CbmTSUnpackMuch::FillEpochInfo(ngdpb::Message mess)
{
  Int_t rocId          = mess.getRocNumber();
  fCurrentEpoch[rocId] = mess.getEpochNumber();
/*	// For Testing generating some Charge value on some channels 
	Int_t nxChannel = gRandom->Gaus(0, 128);
	Int_t charge = gRandom->Gaus(0, 4096); 
	fHM->H2("Raw_ADC_Much")->Fill(nxChannel,charge);
	Int_t channel = gRandom->Gaus(0,22);
	Int_t sector = gRandom->Gaus(0,78);
	fHM->H2("Pad_Distribution")->Fill(sector,channel);
*/
  //  LOG(INFO) << "Epoch message for ROC " << rocId << " with epoch number "
  //            << fCurrentEpoch[rocId] << FairLogger::endl;
  fCurrentEpochTime = mess.getMsgFullTime(fCurrentEpoch[rocId]);
  fNofEpochs++;
  LOG(DEBUG) << "Epoch message "
             << fNofEpochs << ", epoch " << static_cast<Int_t>(fCurrentEpoch[rocId])
             << ", time " << std::setprecision(9) << std::fixed
             << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
             << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
             << FairLogger::endl;

}

void CbmTSUnpackMuch::Reset()
{
  if (fCreateRawMessage) {
    fMuchRaw->Clear();
  }
  fMuchDigi->Clear();
}

void CbmTSUnpackMuch::Finish()
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
      LOG(INFO) << "Last epoch for nDPB: " 
                << std::hex << std::setw(4) << it->first 
                << " => " << it->second 
                << FairLogger::endl;
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;

  gDirectory->mkdir("Much_Raw");
  gDirectory->cd("Much_Raw");
  fHM->H2("Raw_ADC_Much")->Write();        
  fHM->H2("Pad_Distribution")->Write();        
  gDirectory->cd("..");

}


void CbmTSUnpackMuch::FillOutput(CbmDigi* digi)
{

  new( (*fMuchDigi)[fMuchDigi->GetEntriesFast()] )
    CbmSpsMuchDigi(*(dynamic_cast<CbmSpsMuchDigi*>(digi)));

}

ClassImp(CbmTSUnpackMuch)
