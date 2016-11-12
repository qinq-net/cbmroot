// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSUnpackTof                             -----
// -----               Created 27.10.2016 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTSUnpackTof.h"
#include "CbmTofUnpackPar.h"
#include "CbmTofDigiExp.h"

#include "CbmTbDaqBuffer.h"

//#include "CbmFiberHodoAddress.h"
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

const UInt_t kuNbChanGet4 =  4;
const UInt_t kuNbChanAfck = 96;  // FIXME - should be read from parameter file 
static Int_t iMess=0;
const Double_t FineTimeConvFactor=0.048828;

CbmTSUnpackTof::CbmTSUnpackTof( UInt_t uNbGdpb )
  : CbmTSUnpack(),
    fuMsAcceptsPercent(100),
    fuMinNbGdpb( uNbGdpb ),
    fuCurrNbGdpb( 0 ),
    fMsgCounter(11,0), // length of enum MessageTypes initialized with 0
    fGdpbIdIndexMap(),
    fHM(new CbmHistManager()),
    fCurrentEpoch(),
    fNofEpochs(0),
    fCurrentEpochTime(0.),
    fEquipmentId(0),
//    fFiberHodoRaw(new TClonesArray("CbmNxyterRawMessage", 10)),
    fTofDigi(),
//    fRawMessage(NULL),  
    fDigi(NULL),
    fBuffer(CbmTbDaqBuffer::Instance()),
    fUnpackPar(NULL)
{
}

CbmTSUnpackTof::~CbmTSUnpackTof()
{
}

Bool_t CbmTSUnpackTof::Init()
{
  LOG(INFO) << "Initializing flib Get4 unpacker" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  fTofDigi= new TClonesArray("CbmTofDigiExp", 10);
  if (fTofDigi == NULL) {
    LOG(FATAL) << "No Digi TClonesarray " << FairLogger::endl;
  }
  //  ioman->Register("FiberHodoRawMessage", "fiberhodo raw data", fFiberHodoRaw, kTRUE);
  ioman->Register("CbmTofDigi", "Tof raw Digi", fTofDigi, kTRUE);

  CreateHistograms();

  return kTRUE;
}

void CbmTSUnpackTof::SetParContainers()
{
  LOG(INFO) << "Setting parameter containers for " << GetName()
	    << FairLogger::endl;
  fUnpackPar = (CbmTofUnpackPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmTofUnpackPar"));

}

Bool_t CbmTSUnpackTof::InitContainers()
{
	LOG(INFO) << "Init parameter containers for " << GetName()
			<< FairLogger::endl;
	return ReInitContainers();
}

Bool_t CbmTSUnpackTof::ReInitContainers()
{
	LOG(INFO) << "ReInit parameter containers for " << GetName()
			<< FairLogger::endl;
	Int_t nrOfRocs = fUnpackPar->GetNrOfRocs();

	LOG(INFO) << "Nr. of Tof Rocs: " << nrOfRocs
    		  << FairLogger::endl;
	
	fGdpbIdIndexMap.clear();
	for (Int_t i = 0; i< nrOfRocs; ++i) {
	  fGdpbIdIndexMap[fUnpackPar->GetRocId(i)] = i;
	  LOG(INFO) << "Roc Id of TOF  " << i
			  << " : " << fUnpackPar->GetRocId(i)
			  << FairLogger::endl;
	}
	Int_t NrOfChannels = fUnpackPar->GetNumberOfChannels();
	LOG(INFO) << "Nr. of mapped Tof channels: " << NrOfChannels;
	for (Int_t i = 0; i< NrOfChannels; ++i) {
	  if(i%8 == 0)  LOG(INFO) << FairLogger::endl;
	  LOG(INFO) << Form(" 0x%08x",fUnpackPar->GetChannelToDetUIdMap(i));
	}
	LOG(INFO)  << FairLogger::endl;
	
	return kTRUE;
}

void CbmTSUnpackTof::CreateHistograms()
{
   for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   {
      fHM->Add( Form("Raw_Tot_gDPB_%02u", uGdpb),
           new TH2F( Form("Raw_Tot_gDPB_%02u", uGdpb),
                     Form("Raw TOT gDPB %02u; channel; TOT [bin]", uGdpb),
                     96, 0, 95, 256, 0, 255) );
      fHM->Add( Form("ChCount_gDPB_%02u", uGdpb),
           new TH1I( Form("ChCount_gDPB_%02u", uGdpb),
                     Form("Channel counts gDPB %02u; channel; Hits", uGdpb),
                     96, 0, 95 ) );
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
}

Bool_t CbmTSUnpackTof::DoUnpack(const fles::Timeslice& ts, size_t component)
{

  LOG(DEBUG1) << "Timeslice contains " << ts.num_microslices(component)
             << "microslices." << FairLogger::endl;
  
  // Loop over microslices
  for (size_t m = 0; m < ts.num_microslices(component); ++m)
    {
       if( fuMsAcceptsPercent < m )
         continue;

      constexpr uint32_t kuBytesPerMessage = 8;

      auto msDescriptor = ts.descriptor(component, m);
      fEquipmentId = msDescriptor.eq_id;
      const uint8_t* msContent = reinterpret_cast<const uint8_t*>(ts.content(component, m));

      uint32_t size = msDescriptor.size;
      if(size>0)
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
 //           FillHitInfo(mess);
            LOG(ERROR) << "Message type " << mess.getMessageType() 
                       << " not yet included in unpacker."
                       << FairLogger::endl;
            break;
          case ngdpb::MSG_EPOCH:
 //           FillEpochInfo(mess);
            LOG(ERROR) << "Message type " << mess.getMessageType() 
                       << " not yet included in unpacker."
                       << FairLogger::endl;
            break;
          case ngdpb::MSG_EPOCH2:
            FillEpochInfo(mess);
            break;
          case ngdpb::MSG_GET4:
            PrintGenInfo(mess);
            break;
	  case ngdpb::MSG_GET4_32B:
            FillHitInfo(mess);
            break;
          case ngdpb::MSG_GET4_SLC:
            PrintSlcInfo(mess);
            break;
          case ngdpb::MSG_GET4_SYS:
	    if(100 > iMess++)
            PrintSysInfo(mess);
            break;
          default:
	    if(100 > iMess++)
	      LOG(ERROR) << "Message ("<<iMess<<") type " << std::hex << std::setw(2) 
                       << static_cast< uint16_t >( mess.getMessageType() ) 
                       << " not yet included in Get4 unpacker."
                       << FairLogger::endl;
	    if(100 == iMess)
	      LOG(ERROR) << "Stop reporting MSG errors... "
                         << FairLogger::endl;
          }
          
        } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)
      
    }


  return kTRUE;
}

void CbmTSUnpackTof::FillHitInfo(ngdpb::Message mess)
{
  // --- Get absolute time, NXYTER and channel number
  Int_t rocId      = mess.getRocNumber();
  Int_t get4Id     = mess.getGdpbGenChipId();
  Int_t channel    = mess.getGdpbHitChanId(); 
  Int_t tot        = mess.getGdpbHit32Tot();
  ULong_t hitTime  = mess.getMsgFullTime( 0 );
      
  if( fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find( rocId ) )
  {
    fHM->H2( Form("Raw_Tot_gDPB_%02u", fGdpbIdIndexMap[ rocId ]) )
       ->Fill( get4Id*kuNbChanGet4 + channel, tot);
    fHM->H1( Form("ChCount_gDPB_%02u", fGdpbIdIndexMap[ rocId ]) )
       ->Fill( get4Id*kuNbChanGet4 + channel );
    
    hitTime  = mess.getMsgFullTime(fCurrentEpoch[rocId][get4Id]);
    Int_t Ft = mess.getGdpbHitFineTs();
    
    if(100 > iMess++)
    LOG(DEBUG) << "Hit: " << Form("0x%08x ",rocId) << ", " << get4Id 
            << ", " << channel << ", " << tot
	    << ", epoch " << fCurrentEpoch[rocId][get4Id]
	    << ", FullTime " << hitTime 
	    << ", FineTime " << Ft
            << FairLogger::endl;
  
    Int_t iChan = fGdpbIdIndexMap[rocId]*kuNbChanAfck + get4Id*kuNbChanGet4 + channel;
    if (iChan >  fUnpackPar->GetNumberOfChannels()){
      LOG(ERROR) << "Invalid mapping index "<<iChan
		 <<", from " << fGdpbIdIndexMap[rocId]
		 <<", " << get4Id
		 <<", " << channel
		 << FairLogger::endl;
      return;
    }
    Int_t iChanUId = fUnpackPar->GetChannelToDetUIdMap( iChan );
    if(0==iChanUId) return;   // Hit not mapped to digi
    Double_t dTime = hitTime + Ft*FineTimeConvFactor; // in ns, FIXME conversion factor, -> missing calibration
    Double_t dTot  = tot;     // in ps ?
    if(1){
    LOG(DEBUG) << "Create digi with time " << dTime<<", Tot "<<dTot
      //<< " at epoch " << fCurrentEpoch[rocId]
	       << FairLogger::endl;
    fDigi = new CbmTofDigiExp(iChanUId, dTime, dTot);

    fBuffer->InsertData(fDigi);
    }
  }else
    LOG(WARNING) << "found rocId: " << Form("0x%08x ",rocId) << FairLogger::endl;
      
}

void CbmTSUnpackTof::FillEpochInfo(ngdpb::Message mess)
{
  Int_t rocId          = mess.getRocNumber();
  Int_t get4Id     = mess.getGdpbGenChipId();
  fCurrentEpoch[rocId][get4Id] = mess.getEpoch2Number();
  
  if( fGdpbIdIndexMap.end() == fGdpbIdIndexMap.find( rocId ) )
  {
     fGdpbIdIndexMap[ rocId ] = fuCurrNbGdpb;
     fuCurrNbGdpb ++;
     
     if( fuMinNbGdpb < fuCurrNbGdpb )
     {
        // Add new histo
        fHM->Add( Form("Raw_Tot_gDPB_%02u", fuMinNbGdpb),
           new TH2F( Form("Raw_Tot_gDPB_%02u", fuMinNbGdpb),
                     Form("Raw TOT gDPB %02u; channel; TOT [bin]", fuMinNbGdpb),
                     96, 0, 95, 256, 0, 255) );
                     
         fHM->Add( Form("ChCount_gDPB_%02u", fuMinNbGdpb),
              new TH1I( Form("ChCount_gDPB_%02u", fuMinNbGdpb),
                        Form("Channel counts gDPB %02u; channel; Hits", fuMinNbGdpb),
                        96, 0, 95 ) );
	 LOG(INFO)<<" Add histos for gDPB "<<fuMinNbGdpb<<", rocID "<<rocId<<FairLogger::endl;
        // increase fuMinNbGdpb
        fuMinNbGdpb++;
     } // if( fuMinNbGdpb < fuCurrNbGdpb )
  } // if( std::map::end == fGdpbIdIndexMap.find( rocId ) )

  //  LOG(INFO) << "Epoch message for ROC " << rocId << " with epoch number "
  //            << fCurrentEpoch[rocId] << FairLogger::endl;

  fCurrentEpochTime = mess.getMsgFullTime(fCurrentEpoch[rocId][get4Id]);
  fNofEpochs++;
  LOG(DEBUG) << "Epoch message "
             << fNofEpochs << ", epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][get4Id])
             << ", time " << std::setprecision(9) << std::fixed
             << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
             << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
             << " and chip " << mess.getEpoch2ChipNumber()
             << FairLogger::endl;

}

void CbmTSUnpackTof::PrintSlcInfo(ngdpb::Message mess)
{
  Int_t rocId          = mess.getRocNumber();
  Int_t get4Id     = mess.getGdpbGenChipId();

  if( fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find( rocId ) )
     LOG(INFO) << "GET4 Slow Control message, epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][get4Id])
                << ", time " << std::setprecision(9) << std::fixed
                << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
                << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
                << FairLogger::endl
                << " +++++++ > Chip = " << std::setw(2) << mess.getGdpbGenChipId()
                << ", Chan = " << std::setw(1) << mess.getGdpbSlcChan()
                << ", Edge = " << std::setw(1) << mess.getGdpbSlcEdge()
                << ", Type = " << std::setw(1) << mess.getGdpbSlcType()
                << ", Data = " << std::hex << std::setw(6) << mess.getGdpbSlcData() << std::dec
                << ", Type = " << mess.getGdpbSlcCrc()
                << FairLogger::endl;

}

void CbmTSUnpackTof::PrintGenInfo(ngdpb::Message mess)
{
  Int_t mType        = mess.getMessageType();
  Int_t rocId          = mess.getRocNumber();
  Int_t get4Id     = mess.getGdpbGenChipId();
  Int_t channel    = mess.getGdpbHitChanId();
  uint64_t            uData = mess.getData(); 
  if(100 > iMess++)
  LOG(INFO) << "Get4 MSG type "<<mType<<" from rocId "<<rocId<<", getId "<<get4Id
	    << ", (hit channel) "<<channel<<Form(" data 0x%01Fx ",uData)
            << FairLogger::endl;  
}

void CbmTSUnpackTof::PrintSysInfo(ngdpb::Message mess)
{
  Int_t rocId          = mess.getRocNumber();
  Int_t get4Id     = mess.getGdpbGenChipId();

  if( fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find( rocId ) )
     LOG(INFO) << "GET4 System message,       epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][get4Id])
                << ", time " << std::setprecision(9) << std::fixed
                << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
                << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
                << FairLogger::endl;
             
   switch( mess.getGdpbSysSubType() )
   {
      case ngdpb::SYSMSG_GET4_EVENT:
      {
         LOG(INFO) << " +++++++ > Chip = " << std::setw(2) << mess.getGdpbGenChipId()
                   << ", Chan = " << std::setw(1) << mess.getGdpbSysErrChanId()
                   << ", Edge = " << std::setw(1) << mess.getGdpbSysErrEdge()
                   << ", Empt = " << std::setw(1) << mess.getGdpbSysErrUnused()
                   << ", Data = " << std::hex << std::setw(2) << mess.getGdpbSysErrData() << std::dec
                   << " -- GET4 V1 Error Event"
                   << FairLogger::endl;
         break;
      } //
      case ngdpb::SYSMSG_CLOSYSYNC_ERROR:
         LOG(INFO) << "Closy synchronization error" << FairLogger::endl;
         break;
      case ngdpb::SYSMSG_TS156_SYNC:
         LOG(INFO) << "156.25MHz timestamp reset" << FairLogger::endl;
         break;
      case ngdpb::SYSMSG_GDPB_UNKWN:
         LOG(INFO) << "Unknown GET4 message, data: " << std::hex << std::setw(8)
                   << mess.getGdpbSysUnkwData() << std::dec
                   << FairLogger::endl;
         break;
   } // switch( getGdpbSysSubType() )
}

void CbmTSUnpackTof::Reset()
{
  //  fFiberHodoRaw->Clear();
  fTofDigi->Clear();
}

void CbmTSUnpackTof::Finish()
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
    default:  message_type ="UNKNOWN"; break;
    }
    LOG(INFO) << message_type << " messages: " 
              << fMsgCounter[i] << FairLogger::endl;
  }
  
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   for( auto it = fCurrentEpoch.begin(); it != fCurrentEpoch.end(); ++it)
      for( auto itG = (it->second).begin(); itG != (it->second).end(); ++itG)
      LOG(INFO) << "Last epoch for gDPB: " 
                << std::hex << std::setw(4) << it->first 
                << " , GET4  " << std::setw(4) << itG->first 
                << " => " << itG->second 
                << FairLogger::endl;
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   
   
   gDirectory->mkdir("Tof_Raw_gDPB");
   gDirectory->cd("Tof_Raw_gDPB");
   for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   {
      fHM->H2( Form("Raw_Tot_gDPB_%02u", uGdpb) )->Write();
      fHM->H1( Form("ChCount_gDPB_%02u", uGdpb) )->Write();
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   gDirectory->cd("..");

}


void CbmTSUnpackTof::FillOutput(CbmDigi* digi)
{
 if(100 > iMess++)
 LOG(DEBUG) << "Fill digi TClonesarray at " << (Int_t)fTofDigi->GetEntriesFast()
           << FairLogger::endl;

 new( (*fTofDigi)[fTofDigi->GetEntriesFast()] )
    CbmTofDigiExp(*(dynamic_cast<CbmTofDigiExp*>(digi)));
    //CbmTofDigiExp((CbmTofDigiExp *)digi);
    
 digi->Delete();

}


ClassImp(CbmTSUnpackTof)
