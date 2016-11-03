// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSUnpackTest                            -----
// -----               Created 07.11.2014 by F. Uhlig                      -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTSUnpackFHodo.h"
#include "CbmFHodoUnpackPar.h"

#include "CbmTbDaqBuffer.h"

#include "CbmFiberHodoAddress.h"
#include "CbmHistManager.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

#include "TClonesArray.h"
#include "TString.h"

#include <iostream>
#include <stdint.h>
#include <iomanip>

using std::hex;
using std::dec;

CbmTSUnpackFHodo::CbmTSUnpackFHodo()
  : CbmTSUnpack(),
    fMsgCounter(11,0), // length of enum MessageTypes initialized with 0
    fHodoStationMap(),
    fHodoFiber(),
    fHodoPlane(),
    fHodoPixel(),
    fHM(new CbmHistManager()),
    fCurrentEpoch(),
    fNofEpochs(0),
    fCurrentEpochTime(0.),
    fEquipmentId(0),
    fFiberHodoRaw(new TClonesArray("CbmNxyterRawMessage", 10)),
    fFiberHodoDigi(new TClonesArray("CbmFiberHodoDigi", 10)),
    fRawMessage(NULL),  
    fDigi(NULL),
    fBuffer(CbmTbDaqBuffer::Instance()),
	fCreateRawMessage(kFALSE),
	fUnpackPar(NULL)
{
}

CbmTSUnpackFHodo::~CbmTSUnpackFHodo()
{
}

Bool_t CbmTSUnpackFHodo::Init()
{
  LOG(INFO) << "Initializing flib nxyter unpacker" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  if (fCreateRawMessage) {
    ioman->Register("FiberHodoRawMessage", "fiberhodo raw data", fFiberHodoRaw, kTRUE);
  }
  ioman->Register("FiberHodoDigi", "fiber hodo digi", fFiberHodoDigi, kTRUE);

  CreateHistograms();
  return kTRUE;
}

void CbmTSUnpackFHodo::SetParContainers()
{
	LOG(INFO) << "Setting parameter containers for " << GetName()
			<< FairLogger::endl;
    fUnpackPar = (CbmFHodoUnpackPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmFHodoUnpackPar"));
}


Bool_t CbmTSUnpackFHodo::InitContainers()
{
	LOG(INFO) << "Init parameter containers for " << GetName()
			<< FairLogger::endl;
    return ReInitContainers();

}

Bool_t CbmTSUnpackFHodo::ReInitContainers()
{
	LOG(INFO) << "ReInit parameter containers for " << GetName()
			<< FairLogger::endl;

	Int_t nrOfRocs = fUnpackPar->GetNrOfRocs();

	LOG(INFO) << "Nr. of Rocs: " << nrOfRocs
    		<< FairLogger::endl;

	fHodoStationMap.clear();
	for (Int_t i = 0; i< nrOfRocs; ++i) {
	  fHodoStationMap[fUnpackPar->GetRocId(i)] = i;
	  LOG(INFO) << "Roc Id of fiber hodo station " << i
			  << " : " << fUnpackPar->GetRocId(i)
			  << FairLogger::endl;
	}

	Int_t nrOfChannels = fUnpackPar->GetNumberOfChannels();

	for (Int_t i = 0; i< nrOfRocs; ++i) {
		  fHodoFiber[i] = fUnpackPar->GetChannelToFiberMap(i);
		  fHodoPixel[i] = fUnpackPar->GetChannelToPixelMap(i);
		  fHodoPlane[i] = fUnpackPar->GetChannelToPlaneMap(i);
	}

	LOG(INFO) << "Nr. of Channels: " << nrOfChannels
    		<< FairLogger::endl;

	return kTRUE;
}

void CbmTSUnpackFHodo::CreateHistograms()
{
  fHM->Add("Raw_ADC_FrontHodo", 
           new TH2F("Raw_ADC_FrontHodo", 
                    "Raw_ADC_FrontHodo;channel;ADC value", 128, 0, 127, 4096, 0, 4095));   
  fHM->Add("Raw_ADC_RearHodo", 
           new TH2F("Raw_ADC_RearHodo", 
                    "Raw_ADC_RearHodo;channel;ADC value", 128, 0, 127, 4096, 0, 4095));   
}

Bool_t CbmTSUnpackFHodo::DoUnpack(const fles::Timeslice& ts, size_t component)
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

void CbmTSUnpackFHodo::FillHitInfo(ngdpb::Message mess)
{
  // --- Get absolute time, NXYTER and channel number
  Int_t rocId      = mess.getRocNumber();
  Int_t nxyterId   = mess.getNxNumber();
  Int_t nxChannel  = mess.getNxChNum(); 
  Int_t charge     = mess.getNxAdcValue();
  ULong_t hitTime  = mess.getMsgFullTime(fCurrentEpoch[rocId]);

  if (fCreateRawMessage) {
    fRawMessage = new( (*fFiberHodoRaw)[fFiberHodoRaw->GetEntriesFast()] )
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

  if (fHodoStationMap.find(rocId) != fHodoStationMap.end()) {
    Int_t station = fHodoStationMap[rocId];
    Int_t plane = fHodoPlane[nxChannel];
    Int_t fiber = fHodoFiber[nxChannel];

    Int_t address = CbmFiberHodoAddress::GetAddress(station, plane, fiber);

  
    LOG(DEBUG) << "Create digi with time " << hitTime
               << " at epoch " << fCurrentEpoch[rocId] << FairLogger::endl;
 
    fDigi = new CbmFiberHodoDigi(address, charge, hitTime);

    fBuffer->InsertData(fDigi);

    if ( 0 == station ) {
    	fHM->H2("Raw_ADC_FrontHodo")->Fill(nxChannel,charge);
    } else {
    	fHM->H2("Raw_ADC_RearHodo")->Fill(nxChannel,charge);
    }
  } else {
	LOG(ERROR) << "Unknown Roc Id " << rocId << FairLogger::endl;
  }

}

void CbmTSUnpackFHodo::FillEpochInfo(ngdpb::Message mess)
{
  Int_t rocId          = mess.getRocNumber();
  fCurrentEpoch[rocId] = mess.getEpochNumber();

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

void CbmTSUnpackFHodo::Reset()
{
  if (fCreateRawMessage) {
    fFiberHodoRaw->Clear();
  }
  fFiberHodoDigi->Clear();
}

void CbmTSUnpackFHodo::Finish()
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

  gDirectory->mkdir("Hodo_Raw");
  gDirectory->cd("Hodo_Raw");
  fHM->H2("Raw_ADC_FrontHodo")->Write();        
  fHM->H2("Raw_ADC_RearHodo")->Write();        
  gDirectory->cd("..");

}


void CbmTSUnpackFHodo::FillOutput(CbmDigi* digi)
{

  new( (*fFiberHodoDigi)[fFiberHodoDigi->GetEntriesFast()] )
    CbmFiberHodoDigi(*(dynamic_cast<CbmFiberHodoDigi*>(digi)));

}

ClassImp(CbmTSUnpackFHodo)
