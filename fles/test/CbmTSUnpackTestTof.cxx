// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSUnpackTestTof                         -----
// -----               Created 27.10.2016 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTSUnpackTestTof.h"

#include "CbmTbDaqBuffer.h"

//#include "CbmFiberHodoAddress.h"
#include "CbmHistManager.h"

#include "FairLogger.h"
#include "FairRootManager.h"

#include "TClonesArray.h"
#include "TString.h"

#include <iostream>
#include <stdint.h>
#include <iomanip>

const UInt_t kuNbChanGet4 = 4;

CbmTSUnpackTestTof::CbmTSUnpackTestTof( UInt_t uNbGdpb )
  : CbmTSUnpack(),
    fuMinNbGdpb( uNbGdpb ),
    fuCurrNbGdpb( 0 ),
    fMsgCounter(11,0), // length of enum MessageTypes initialized with 0
    fGdpbIdIndexMap(),
//    fHodoStationMap(),
//    fHodoFiber(),
//    fHodoPlane(),
//    fHodoPixel(),
    fHM(new CbmHistManager()),
    fCurrentEpoch(),
    fNofEpochs(0),
    fCurrentEpochTime(0.),
    fEquipmentId(0),
//    fFiberHodoRaw(new TClonesArray("CbmNxyterRawMessage", 10)),
//    fFiberHodoDigi(new TClonesArray("CbmFiberHodoDigi", 10)),
//    fRawMessage(NULL),  
//    fDigi(NULL),
    fBuffer(CbmTbDaqBuffer::Instance())
{
//  InitializeFiberHodoMapping();
}

CbmTSUnpackTestTof::~CbmTSUnpackTestTof()
{
}

Bool_t CbmTSUnpackTestTof::Init()
{
  LOG(INFO) << "Initializing flib nxyter unpacker" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  //  ioman->Register("FiberHodoRawMessage", "fiberhodo raw data", fFiberHodoRaw, kTRUE);
//  ioman->Register("FiberHodoDigi", "fiber hodo digi", fFiberHodoDigi, kTRUE);

  CreateHistograms();

  return kTRUE;
}

void CbmTSUnpackTestTof::CreateHistograms()
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

Bool_t CbmTSUnpackTestTof::DoUnpack(const fles::Timeslice& ts, size_t component)
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
 //           FillHitInfo(mess);
            LOG(ERROR) << "Message type " << mess.getMessageType() 
                       << " not yet include in unpacker."
                       << FairLogger::endl;
            break;
          case ngdpb::MSG_EPOCH:
 //           FillEpochInfo(mess);
            LOG(ERROR) << "Message type " << mess.getMessageType() 
                       << " not yet include in unpacker."
                       << FairLogger::endl;
            break;
          case ngdpb::MSG_EPOCH2:
            FillEpochInfo(mess);
            break;
          case ngdpb::MSG_GET4_32B:
            FillHitInfo(mess);
            break;
          case ngdpb::MSG_GET4_SLC:
            PrintSlcInfo(mess);
            break;
          case ngdpb::MSG_GET4_SYS:
            PrintSysInfo(mess);
            break;
          default: 
            LOG(ERROR) << "Message type " << std::hex << std::setw(2) 
                       << static_cast< uint16_t >( mess.getMessageType() ) 
                       << " not yet include in Get4 unpacker."
                       << FairLogger::endl;
          }
          


        } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)
      
    }


  return kTRUE;
}

void CbmTSUnpackTestTof::FillHitInfo(ngdpb::Message mess)
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
   }
 
  LOG(DEBUG) << "Hit: " << rocId << ", " << get4Id 
             << ", " << channel << ", " << tot << ", " << hitTime 
             << FairLogger::endl;
/*
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
*/
}

void CbmTSUnpackTestTof::FillEpochInfo(ngdpb::Message mess)
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

void CbmTSUnpackTestTof::PrintSlcInfo(ngdpb::Message mess)
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

void CbmTSUnpackTestTof::PrintSysInfo(ngdpb::Message mess)
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

void CbmTSUnpackTestTof::Reset()
{
  //  fFiberHodoRaw->Clear();
  //fFiberHodoDigi->Clear();
}

void CbmTSUnpackTestTof::Finish()
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
/*
  gDirectory->mkdir("Hodo_Raw_gDPB");
  gDirectory->cd("Hodo_Raw_gDPB");
  fHM->H2("Raw_ADC_FrontHodo_gDPB")->Write();        
  fHM->H2("Raw_ADC_RearHodo_gDPB")->Write();        
  gDirectory->cd("..");
*/
}


void CbmTSUnpackTestTof::FillOutput(CbmDigi* digi)
{

//  new( (*fFiberHodoDigi)[fFiberHodoDigi->GetEntriesFast()] )
//    CbmFiberHodoDigi(*(dynamic_cast<CbmFiberHodoDigi*>(digi)));

}

// ---------------------------------------------------------------------------
/*
void CbmTSUnpackTestTof::InitializeFiberHodoMapping()
{
  // This code was copied from the Go4 analysis used for previous beamtimes
  for (Int_t i=0; i<128; i++) {
    fHodoFiber[i] = -1;
    fHodoPlane[i] = -1;
    fHodoPixel[i] = -1;
  }
  
  for (Int_t ifiber=1; ifiber<=64; ifiber++) {
    // Calculate fiber number [1..64] from feb channel
    // lcn: linearconnectornumber, is the wire number on one of the
    // flat cables. [1..16]
    // each 16 fibers go to one connector.
    // fibersubnr[0..15] linear fiber counter in groups of 16

    Int_t fibersubnr=(ifiber-1)%16;

    Int_t lcn=15-fibersubnr*2;
    if (fibersubnr>=8) lcn=(fibersubnr-7)*2;
    
    Int_t channel=-1;
    Int_t cable=(ifiber-1)/16+1;
    Int_t pixel= ((lcn-1)/2)*8 +((lcn-1)%2);
    if (cable==1) {
      channel=(lcn-1)*4+0;
      pixel=pixel+1;
    }
    if (cable==2) {
      channel=(lcn-1)*4+2;
      pixel=pixel+3;
    }
    if (cable==3) {
      channel=(lcn-1)*4+1;
      pixel=pixel+5;
    }
    if (cable==4) {
      channel=(lcn-1)*4+3;
      pixel=pixel+7;
    }
    
    // new code to resolve cabling problem during cern-oct12
    int ifiber_bis = ifiber;
    if (ifiber <= 8 )  ifiber_bis = ifiber + 56; else
      if (ifiber <= 16 ) ifiber_bis = ifiber + 40; else
        if (ifiber <= 24 ) ifiber_bis = ifiber + 24; else
          if (ifiber <= 32 ) ifiber_bis = ifiber + 8; else 
            if (ifiber <= 40 ) ifiber_bis = ifiber - 8; else 
              if (ifiber <= 48 ) ifiber_bis = ifiber - 24; else
                if (ifiber <= 56 ) ifiber_bis = ifiber - 40; else
                  if (ifiber <= 64 ) ifiber_bis = ifiber - 56;
    
    // and swap at the end
    ifiber_bis = 65 - ifiber_bis;

    fHodoFiber[channel] = ifiber_bis - 1;
    fHodoPlane[channel] = 0;
    fHodoPixel[channel] = pixel;

    fHodoFiber[channel+64] = ifiber_bis - 1;
    fHodoPlane[channel+64] = 1;
    fHodoPixel[channel+64] = pixel;

  }

  for (Int_t i=0; i<128; i++) {
    LOG(DEBUG) << "Channel[" << i << "]: " << fHodoFiber[i] << ", " 
              << fHodoPlane[i] << ", " << fHodoPixel[i] 
              << FairLogger::endl;
  }

}
*/

ClassImp(CbmTSUnpackTestTof)
