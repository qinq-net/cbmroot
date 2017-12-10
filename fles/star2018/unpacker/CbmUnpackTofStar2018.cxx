// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmUnpackTofStar2018                       -----
// -----               Created 08.12.2017 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmUnpackTofStar2018.h"
#include "CbmTofUnpackPar.h"
#include "CbmTofDigiExp.h"
#include "CbmTbEvent.h"

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

static Int_t iMess=0;
const  Int_t DetMask = 0x0001FFFF;


CbmUnpackTofStar2018::CbmUnpackTofStar2018( UInt_t uNbGdpb )
  : CbmTSUnpack(),
    fuMsAcceptsPercent(100),
    fuOverlapMsNb(0),
    fuMinNbGdpb( uNbGdpb ),
    fuCurrNbGdpb( 0 ),
    fuNrOfGdpbs(0),
    fuNrOfFebsPerGdpb(0),
    fuNrOfGet4PerFeb(0),
    fuNrOfChannelsPerGet4(0),
    fuNrOfChannelsPerFeet(0),
    fuNrOfGet4(0),
    fuNrOfGet4PerGdpb(0),
    fuNrOfChannelsPerGdpb(0),
    fMsgCounter(11,0), // length of enum MessageTypes initialized with 0
    fGdpbIdIndexMap(),
    fuGdpbId(0),
    fuGdpbNr(0),
    fuGet4Id(0),
    fuGet4Nr(0),
    fHM(new CbmHistManager()),
    fvulCurrentEpoch(),
    fvbFirstEpochSeen(),
    fNofEpochs(0),
    fulCurrentEpochTime(0.),
    fEquipmentId(0),
    fdMsIndex(0.),
    fdTShiftRef(0.),
    fTofDigi(),
    fDigi(NULL),
    fBuffer(CbmTbDaqBuffer::Instance()),
    fUnpackPar(NULL),
    fdRefTime(0.),
    fdLastDigiTime(0.),
    fdFirstDigiTimeDif(0.),
    fdEvTime0(0.),
    fvmEpSupprBuffer()
{
}

CbmUnpackTofStar2018::~CbmUnpackTofStar2018()
{
}

Bool_t CbmUnpackTofStar2018::Init()
{
   LOG(INFO) << "Initializing flib Get4 unpacker" << FairLogger::endl;

   FairRootManager* ioman = FairRootManager::Instance();
   if( NULL == ioman )
   {
      LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
   }

   fTofDigi= new TClonesArray("CbmTofDigiExp", 10);
   if( NULL == fTofDigi )
   {
      LOG(FATAL) << "No Digi TClonesarray " << FairLogger::endl;
   }
   ioman->Register("CbmTofDigi", "Tof raw Digi", fTofDigi, kTRUE);

/*
   CbmTbEvent * fEventHeader = (CbmTbEvent *)ioman->GetObject("EventHeader.");
   if( NULL == fEventHeade r)
   {
      LOG(FATAL) << "No EventHeader TClonesarray " << FairLogger::endl;
   }
*/

   fUnpackPar = (CbmTofUnpackPar*)(FairRun::Instance());

   CreateHistograms();

   fvulCurrentEpoch.resize( fuNrOfGdpbs * fuNrOfGet4PerGdpb );
   fvbFirstEpochSeen.resize( fuNrOfGdpbs * fuNrOfGet4PerGdpb );
   for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )
   {
      for( UInt_t j = 0; j < fuNrOfGet4PerGdpb; ++j )
      {
         fvulCurrentEpoch[GetArrayIndex(i, j)] = 0;
         fvbFirstEpochSeen[GetArrayIndex(i, j)] = kFALSE;
      } // for( UInt_t j = 0; j < fuNrOfGet4PerGdpb; ++j )
   } // for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )

   return kTRUE;
}

void CbmUnpackTofStar2018::SetParContainers()
{
  LOG(INFO) << "Setting parameter containers for " << GetName()
	    << FairLogger::endl;
  fUnpackPar = (CbmTofUnpackPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmTofUnpackPar"));

}

Bool_t CbmUnpackTofStar2018::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
               << FairLogger::endl;
   Bool_t initOK = ReInitContainers();

   return initOK;
}

Bool_t CbmUnpackTofStar2018::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
             << FairLogger::endl;

   fuNrOfGdpbs = fUnpackPar->GetNrOfRocs();
   LOG(INFO) << "Nr. of Tof GDPBs: " << fuNrOfGdpbs << FairLogger::endl;
   fuMinNbGdpb = fuNrOfGdpbs;

   fuNrOfFebsPerGdpb = fUnpackPar->GetNrOfFebsPerGdpb();
   LOG(INFO) << "Nr. of FEBS per Tof GDPB: " << fuNrOfFebsPerGdpb
               << FairLogger::endl;

   fuNrOfGet4PerFeb = fUnpackPar->GetNrOfGet4PerFeb();
   LOG(INFO) << "Nr. of GET4 per Tof FEB: " << fuNrOfGet4PerFeb
               << FairLogger::endl;

   fuNrOfChannelsPerGet4 = fUnpackPar->GetNrOfChannelsPerGet4();
   LOG(INFO) << "Nr. of channels per GET4: " << fuNrOfChannelsPerGet4
               << FairLogger::endl;

   fuNrOfChannelsPerFeet = fuNrOfGet4PerFeb * fuNrOfChannelsPerGet4;
   LOG(INFO) << "Nr. of channels per FEET: " << fuNrOfChannelsPerFeet
               << FairLogger::endl;

   fuNrOfGet4 = fuNrOfGdpbs * fuNrOfFebsPerGdpb * fuNrOfGet4PerFeb;
   LOG(INFO) << "Nr. of GET4s: " << fuNrOfGet4 << FairLogger::endl;

   fuNrOfGet4PerGdpb = fuNrOfFebsPerGdpb * fuNrOfGet4PerFeb;
   LOG(INFO) << "Nr. of GET4s per GDPB: " << fuNrOfGet4PerGdpb
               << FairLogger::endl;

   fGdpbIdIndexMap.clear();
   for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )
   {
      fGdpbIdIndexMap[fUnpackPar->GetRocId(i)] = i;
      LOG(INFO) << "GDPB Id of TOF  " << i << " : " << std::hex << fUnpackPar->GetRocId(i)
                 << std::dec << FairLogger::endl;
   } // for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )
   UInt_t uNrOfChannels = fUnpackPar->GetNumberOfChannels();
   LOG(INFO) << "Nr. of mapped Tof channels: " << uNrOfChannels;
   for( UInt_t i = 0; i < uNrOfChannels; ++i)
   {
      if (i % 8 == 0)
         LOG(INFO) << FairLogger::endl;
      LOG(INFO) << Form(" 0x%08x", fUnpackPar->GetChannelToDetUIdMap(i) );
   } // for( UInt_t i = 0; i < uNrOfChannels; ++i)
   LOG(INFO) << FairLogger::endl;

   LOG(INFO) << "Plot Channel Rate => "
             << (fUnpackPar->IsChannelRateEnabled() ? "ON" : "OFF")
             << FairLogger::endl;

   fvmEpSupprBuffer.resize( fuNrOfGet4 );

	return kTRUE;
}

void CbmUnpackTofStar2018::CreateHistograms()
{
   LOG(INFO) << "create Histos for " << fuMinNbGdpb <<" Rocs "
	    << FairLogger::endl;

     fHM->Add( Form("Raw_TDig-EvT0"),
           new TH1F( Form("Raw_TDig-EvT0"),
                     Form("Raw digi time difference to 1st digi ; time [ns]; cts"),
                     100, 0, 50.) );

     fHM->Add( Form("Raw_TDig-Ref0"),
           new TH1F( Form("Raw_TDig-Ref0"),
                     Form("Raw digi time difference to Ref ; time [ns]; cts"),
                     5000, 0, 500000) );

     fHM->Add( Form("Raw_TDig-Ref"),
           new TH1F( Form("Raw_TDig-Ref"),
                     Form("Raw digi time difference to Ref ; time [ns]; cts"),
                     5000, 0, 50000) );

     fHM->Add( Form("Raw_TRef-Dig0"),
           new TH1F( Form("Raw_TRef-Dig0"),
                     Form("Raw Ref time difference to last digi  ; time [ns]; cts"),
                     9999, -500000000, 500000000) );
     fHM->Add( Form("Raw_TRef-Dig1"),
           new TH1F( Form("Raw_TRef-Dig1"),
                     Form("Raw Ref time difference to last digi  ; time [ns]; cts"),
                     9999, -5000000, 5000000) );
     fHM->Add( Form("Raw_Digi-LastDigi"),
           new TH1F( Form("Raw_Digi-LastDigi"),
                     Form("Raw Digi time difference to last digi  ; time [ns]; cts"),
                     9999, -5000000, 5000000) );

   for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   {
      fHM->Add( Form("Raw_Tot_gDPB_%02u", uGdpb),
           new TH2F( Form("Raw_Tot_gDPB_%02u", uGdpb),
                     Form("Raw TOT gDPB %02u; channel; TOT [bin]", uGdpb),
                     fuNrOfChannelsPerGdpb, 0, fuNrOfChannelsPerGdpb, 256, 0, 255) );
      fHM->Add( Form("ChCount_gDPB_%02u", uGdpb),
           new TH1I( Form("ChCount_gDPB_%02u", uGdpb),
                     Form("Channel counts gDPB %02u; channel; Hits", uGdpb),
                     fuNrOfChannelsPerGdpb, 0, fuNrOfChannelsPerGdpb ) );
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
}

Bool_t CbmUnpackTofStar2018::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   LOG(DEBUG) << "Timeslice contains " << ts.num_microslices(component)
              << " microslices of component " << component << FairLogger::endl;

   // Loop over microslices
   Int_t iMessageType = -111;
   size_t numCompMsInTs = ts.num_microslices(component);
   for (size_t m = 0; m < numCompMsInTs; ++m)
   {
      // Jump some microslices if needed
//      if( fuMsAcceptsPercent < m)
//         continue;

      // Ignore overlap ms if number defined by user
      if( numCompMsInTs - fuOverlapMsNb <= m )
         continue;

      constexpr uint32_t kuBytesPerMessage = 8;

      auto msDescriptor = ts.descriptor(component, m);
      fEquipmentId = msDescriptor.eq_id;
      fdMsIndex = static_cast<double>(msDescriptor.idx);
      const uint8_t* msContent = reinterpret_cast< const uint8_t* >( ts.content(component, m) );

      uint32_t size = msDescriptor.size;
      if( 0 < size )
         LOG(DEBUG1) << "Microslice "<< m <<": " << fdMsIndex
                     << " has size: " << size << FairLogger::endl;

      // If not integer number of message in input buffer, print warning/error
      if( 0 != (size % kuBytesPerMessage) )
         LOG(ERROR) << "The input microslice buffer does NOT "
                    << "contain only complete nDPB messages!"
                    << FairLogger::endl;

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

         if( gLogger->IsLogNeeded(DEBUG2) )
         {
            mess.printDataCout();
         } // if( gLogger->IsLogNeeded(DEBUG2) )

         // Increment counter for different message types
         iMessageType = mess.getMessageType();
         fMsgCounter[ iMessageType ]++;

         fuGdpbId = mess.getRocNumber();
         fuGdpbNr = fGdpbIdIndexMap[ fuGdpbId ];

         fuGet4Id = mess.getGdpbGenChipId();
         fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + fuGet4Id;

         if( fuNrOfGet4PerGdpb <= fuGet4Id &&
             ngdpb::MSG_STAR_TRI != iMessageType &&
             get4v2x::kuChipIdMergedEpoch != fuGet4Id )
            LOG(WARNING) << "Message with Get4 ID too high: " << fuGet4Id
                         << " VS " << fuNrOfGet4PerGdpb << " set in parameters." << FairLogger::endl;

         switch( mess.getMessageType() )
         {
            case ngdpb::MSG_HIT:
            case ngdpb::MSG_EPOCH:
            case ngdpb::MSG_GET4:
            {
               //           FillEpochInfo(mess);
               LOG(ERROR) << "Message type " << mess.getMessageType()
                          << " not included in unpacker."
                          << FairLogger::endl;
               break;
            } // case old non tof messages
            case ngdpb::MSG_EPOCH2:
            {
               if( get4v2x::kuChipIdMergedEpoch == fuGet4Id )
               {
                  for( uint32_t uGet4Index = 0; uGet4Index < fuNrOfGet4PerGdpb; uGet4Index ++ )
                  {
                     ngdpb::Message tmpMess(mess);
                     tmpMess.setGdpbGenChipId( uGet4Index );

//                     fHistGet4MessType->Fill(uGet4Index, ngdpb::GET4_32B_EPOCH);
                     FillEpochInfo(tmpMess);
                  } // for( uint32_t uGet4Index = 0; uGet4Index < fuNrOfGet4PerGdpb; uGetIndex ++ )
               } // if this epoch message is a merged one valiud for all chips
               else
               {
//                  fHistGet4MessType->Fill( fuGet4Nr, ngdpb::GET4_32B_EPOCH );
                  FillEpochInfo(mess);
               } // if single chip epoch message
               break;
            } // case ngdpb::MSG_EPOCH2:
            case ngdpb::MSG_GET4_32B:
            {
               fvmEpSupprBuffer[ fuGet4Nr ].push_back( mess );
               break;
            } // case ngdpb::MSG_GET4_32B:
            case ngdpb::MSG_GET4_SLC:
            {
               PrintSlcInfo(mess);
               break;
            } // case ngdpb::MSG_GET4_SLC:
            case ngdpb::MSG_GET4_SYS:
            {
               if(100 > iMess++)
                  PrintSysInfo(mess);
               break;
            } // case ngdpb::MSG_GET4_SYS:
            case ngdpb::MSG_STAR_TRI:
            {
               FillStarTrigInfo(mess);
               break;
            } // case ngdpb::MSG_STAR_TRI:
            default:
            {
               if(100 > iMess++)
                  LOG(ERROR) << "Message ("<<iMess<<") type " << std::hex << std::setw(2)
                             << static_cast< uint16_t >( mess.getMessageType() )
                             << " not included in Get4 unpacker."
                             << FairLogger::endl;
               if(100 == iMess)
                  LOG(ERROR) << "Stop reporting MSG errors... "
                             << FairLogger::endl;
            } // default:
         } // switch( mess.getMessageType() )

      } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)

   } // for (size_t m = 0; m < numCompMsInTs; ++m)


  return kTRUE;
}

void CbmUnpackTofStar2018::FillHitInfo( ngdpb::Message mess )
{
   // --- Get absolute time, NXYTER and channel number
   UInt_t uChannel    = mess.getGdpbHitChanId();
   UInt_t uTot        = mess.getGdpbHit32Tot();

   // In 32b mode the coarse counter is already computed back to 112 FTS bins
   // => need to hide its contribution from the Finetime
   // => FTS = Fullt TS modulo 112
   UInt_t uFts        = mess.getGdpbHitFullTs() % 112;

   ULong_t  ulCurEpochGdpbGet4 = fvulCurrentEpoch[ fuGet4Nr ];

   if( kTRUE == fvbFirstEpochSeen[ fuGet4Nr ] )
   {
      // In Ep. Suppr. Mode, receive following epoch instead of previous
      if( 0 < ulCurEpochGdpbGet4 )
         ulCurEpochGdpbGet4 --;
         else ulCurEpochGdpbGet4 = get4v1x::kuEpochCounterSz; // Catch epoch cycle!

      ULong_t  ulhitTime = mess.getMsgG4v2FullTime(  ulCurEpochGdpbGet4 );
      Double_t dHitTime  = mess.getMsgG4v2FullTimeD( ulCurEpochGdpbGet4 );
      Double_t dHitTot   = uTot;     // in bins


      UInt_t uFebIdx     = (fuGet4Id / fuNrOfGet4PerFeb);
      UInt_t uFullFebIdx = (fuGdpbNr * fuNrOfFebsPerGdpb) + uFebIdx;

      UInt_t uChanInGdpb = fuGet4Id * fuNrOfChannelsPerGet4 + uChannel;
      UInt_t uChanInSyst = fuGdpbNr * fuNrOfChannelsPerGdpb + uChanInGdpb;

      UInt_t uChanUId = fUnpackPar->GetChannelToDetUIdMap( uChanInSyst );
      if( fUnpackPar->GetNumberOfChannels() < uChanUId )
      {
         LOG(ERROR) << "Invalid mapping index " << uChanUId
                    <<", from " << fuGdpbNr
                    <<", " << fuGet4Id
                    <<", " << uChannel
                    << FairLogger::endl;
         return;
      } // if( fUnpackPar->GetNumberOfChannels() < uChanUId )
      if( 0 == uChanUId )
         return;   // Hit not mapped to digi

      if( (uChanUId & DetMask) == 0x00005006 )
         dHitTime += fdTShiftRef;

      LOG(DEBUG) << Form("Insert 0x%08x digi with time ", uChanUId ) << dHitTime << Form(", Tot %4.0f",dHitTot)
                 << " into buffer with " << fBuffer->GetSize() << " data from "
                 << Form("%11.1f to %11.1f ", fBuffer->GetTimeFirst(), fBuffer->GetTimeLast())
                 << " at epoch " << ulCurEpochGdpbGet4
                 << FairLogger::endl;
      fDigi = new CbmTofDigiExp(uChanUId, dHitTime, dHitTot);

      fBuffer->InsertData(fDigi);
   } // if( kTRUE == fvbFirstEpochSeen[ fuGet4Nr ] )
}

void CbmUnpackTofStar2018::FillEpochInfo( ngdpb::Message mess )
{
   ULong64_t ulEpochNr = mess.getGdpbEpEpochNb();

   fvulCurrentEpoch[ fuGet4Nr ] = ulEpochNr;

   if( kFALSE == fvbFirstEpochSeen[ fuGet4Nr ] )
      fvbFirstEpochSeen[ fuGet4Nr ] = kTRUE;

   fulCurrentEpochTime = mess.getMsgFullTime(ulEpochNr);
   fNofEpochs++;

   /// In Ep. Suppr. Mode, receive following epoch instead of previous
   /// Re-align the epoch number of the message in case it will be used later:
   /// We received the epoch after the data instead of the one before!
   if( 0 < ulEpochNr )
      mess.setEpoch2Number( ulEpochNr - 1 );
   else mess.setEpoch2Number( get4v1x::kuEpochCounterSz );

   Int_t iBufferSize = fvmEpSupprBuffer[ fuGet4Nr ].size();
   if( 0 < iBufferSize )
   {
      LOG(DEBUG) << "Now processing stored messages for for get4 " << fuGet4Nr << " with epoch number "
                 << (fvulCurrentEpoch[ fuGet4Nr ] - 1) << FairLogger::endl;

      for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
      {
         FillHitInfo( fvmEpSupprBuffer[ fuGet4Nr ][ iMsgIdx ] );
      } // for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )

      fvmEpSupprBuffer[fuGet4Nr].clear();
   } // if( 0 < fvmEpSupprBuffer[fGet4Nr] )
}

void CbmUnpackTofStar2018::PrintSlcInfo(ngdpb::Message mess)
{
   /// Nothing to do, maybe later use it to trakc parameter changes like treshold?
/*
  if( fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find( rocId ) )
     LOG(INFO) << "GET4 Slow Control message, epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][get4Id])
                << ", time " << std::setprecision(9) << std::fixed
                << Double_t(fulCurrentEpochTime) * 1.e-9 << " s "
                << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
                << FairLogger::endl
                << " +++++++ > Chip = " << std::setw(2) << mess.getGdpbGenChipId()
                << ", Chan = " << std::setw(1) << mess.getGdpbSlcChan()
                << ", Edge = " << std::setw(1) << mess.getGdpbSlcEdge()
                << ", Type = " << std::setw(1) << mess.getGdpbSlcType()
                << ", Data = " << std::hex << std::setw(6) << mess.getGdpbSlcData() << std::dec
                << ", Type = " << mess.getGdpbSlcCrc()
                << FairLogger::endl;
*/
}

void CbmUnpackTofStar2018::PrintGenInfo(ngdpb::Message mess)
{
   Int_t mType        = mess.getMessageType();
   Int_t rocId          = mess.getRocNumber();
   Int_t get4Id     = mess.getGdpbGenChipId();
   Int_t channel    = mess.getGdpbHitChanId();
   uint64_t            uData = mess.getData();
   if(100 > iMess++)
      LOG(INFO) << "Get4 MSG type "<<mType<<" from rocId "<<rocId<<", getId "<<get4Id
                << ", (hit channel) "<<channel<<Form(" hex data %0lx ",uData)
                << FairLogger::endl;
}

void CbmUnpackTofStar2018::PrintSysInfo(ngdpb::Message mess)
{
   LOG(INFO) << "GET4 System message,       epoch " << (fvulCurrentEpoch[ fuGet4Nr])
                << ", time " << std::setprecision(9) << std::fixed
                << Double_t(fulCurrentEpochTime) * 1.e-9 << " s "
                << " for board ID " << std::hex << std::setw(4) << fuGdpbId << std::dec
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

void CbmUnpackTofStar2018::Reset()
{
  //  fFiberHodoRaw->Clear();
  fTofDigi->Clear();
}

void CbmUnpackTofStar2018::Finish()
{
   TString message_type;

   for( UInt_t uType = 0; uType < fMsgCounter.size(); ++uType)
   {
      switch(uType)
      {
         case 0:   message_type ="NOP"; break;
         case 1:   message_type ="HIT"; break;
         case 2:   message_type ="EPOCH"; break;
         case 3:   message_type ="SYNC"; break;
         case 4:   message_type ="AUX"; break;
         case 5:   message_type ="EPOCH2"; break;
         case 6:   message_type ="GET4"; break;
         case 7:   message_type ="SYS"; break;
         case 8:   message_type ="GET4_SLC"; break;
         case 9:   message_type ="GET4_32B"; break;
         case 10:  message_type ="GET4_SYS"; break;
         default:  message_type ="UNKNOWN"; break;
      } // switch(uType)
      LOG(INFO) << message_type << " messages: "
                << fMsgCounter[uType] << FairLogger::endl;
   } // for( UInt_t uType = 0; uType < fMsgCounter.size(); ++uType)

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      for( UInt_t uGet4 = 0; uGet4 < fuNrOfGet4PerGdpb; ++uGet4 )
         LOG(INFO) << "Last epoch for gDPB: "<< std::setw(4) << uGdpb
                   << " , GET4  " << std::setw(4) << uGet4
                   << " => " << fvulCurrentEpoch[GetArrayIndex(uGdpb, uGet4)]
                   << FairLogger::endl;
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;


   gDirectory->mkdir("Tof_Raw_gDPB");
   gDirectory->cd("Tof_Raw_gDPB");
   fHM->H1( Form("Raw_TDig-Ref0") )->Write();
   fHM->H1( Form("Raw_TDig-Ref") )->Write();
   fHM->H1( Form("Raw_TRef-Dig0") )->Write();
   fHM->H1( Form("Raw_TRef-Dig1") )->Write();
   fHM->H1( Form("Raw_Digi-LastDigi") )->Write();
   for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   {
      fHM->H2( Form("Raw_Tot_gDPB_%02u", uGdpb) )->Write();
      fHM->H1( Form("ChCount_gDPB_%02u", uGdpb) )->Write();
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   gDirectory->cd("..");

}


void CbmUnpackTofStar2018::FillOutput(CbmDigi* digi)
{
   if( 100 > iMess++ )
      LOG(DEBUG) << "Fill digi TClonesarray with "
                 << Form("0x%08x", digi->GetAddress())
                 << " at " << static_cast<Int_t>( fTofDigi->GetEntriesFast() )
                 << FairLogger::endl;


   new( (*fTofDigi)[ fTofDigi->GetEntriesFast() ] )
      CbmTofDigiExp( *( dynamic_cast<CbmTofDigiExp*>(digi) ) );
   //CbmTofDigiExp((CbmTofDigiExp *)digi);

   if( 0 == fTofDigi->GetEntriesFast())
      fdEvTime0=digi->GetTime();
      else fHM->H1( Form("Raw_TDig-EvT0") )->Fill( digi->GetTime() - fdEvTime0 ); // TODO: replace with pointer!!!

   if( (digi->GetAddress() & DetMask) != 0x00005006 )
   {
      fHM->H1( Form("Raw_TDig-Ref0") )->Fill( digi->GetTime() - fdRefTime); // TODO: replace with pointer!!!
      fHM->H1( Form("Raw_TDig-Ref") )->Fill( digi->GetTime() - fdRefTime); // TODO: replace with pointer!!!
   } // if( (digi->GetAddress() & DetMask) != 0x00005006 )
      else  fdRefTime = digi->GetTime();

   digi->Delete();
}

static    ULong64_t fulGdpbTsMsb;
static    ULong64_t fulGdpbTsLsb;
static    ULong64_t fulStarTsMsb;
static    ULong64_t fulStarTsMid;
static    ULong64_t fulGdpbTsFullLast;
static    ULong64_t fulStarTsFullLast;
static    UInt_t    fuStarTokenLast;
static    UInt_t    fuStarDaqCmdLast;
static    UInt_t    fuStarTrigCmdLast;

void CbmUnpackTofStar2018::FillStarTrigInfo(ngdpb::Message mess)
{
  Int_t iMsgIndex = mess.getStarTrigMsgIndex();

  switch( iMsgIndex )
  {
      case 0:
         fulGdpbTsMsb = mess.getGdpbTsMsbStarA();
         break;
      case 1:
         fulGdpbTsLsb = mess.getGdpbTsLsbStarB();
         fulStarTsMsb = mess.getStarTsMsbStarB();
         break;
      case 2:
         fulStarTsMid = mess.getStarTsMidStarC();
         break;
      case 3:
      {
         ULong64_t ulNewGdpbTsFull = ( fulGdpbTsMsb << 24 )
                           + ( fulGdpbTsLsb       );
         ULong64_t ulNewStarTsFull = ( fulStarTsMsb << 48 )
                           + ( fulStarTsMid <<  8 )
                           + mess.getStarTsLsbStarD();
         UInt_t uNewToken  = mess.getStarTokenStarD();
         UInt_t uNewDaqCmd  = mess.getStarDaqCmdStarD();
         UInt_t uNewTrigCmd = mess.getStarTrigCmdStarD();
         if( ( uNewToken == fuStarTokenLast ) && ( ulNewGdpbTsFull == fulGdpbTsFullLast ) &&
             ( ulNewStarTsFull == fulStarTsFullLast ) && ( uNewDaqCmd == fuStarDaqCmdLast ) &&
             ( uNewTrigCmd == fuStarTrigCmdLast ) )
         {
            LOG(DEBUG) << "Possible error: identical STAR tokens found twice in a row => ignore 2nd! "
                         << Form("token = %5u ", fuStarTokenLast )
                         << Form("gDPB ts  = %12llu ", fulGdpbTsFullLast )
                         << Form("STAR ts = %12llu ", fulStarTsFullLast )
                         << Form("DAQ cmd = %2u ", fuStarDaqCmdLast )
                         << Form("TRG cmd = %2u ", fuStarTrigCmdLast )
                         << FairLogger::endl;
            return;
         } // if exactly same message repeated

         if( (uNewToken != fuStarTokenLast + 1) &&
             0 < fulGdpbTsFullLast && 0 < fulStarTsFullLast &&
             ( 4095 != fuStarTokenLast || 1 != uNewToken)  )
            LOG(WARNING) << "Possible error: STAR token did not increase by exactly 1! "
                         << Form("old = %5u vs new = %5u ", fuStarTokenLast,   uNewToken)
                         << Form("old = %12llu vs new = %12llu ", fulGdpbTsFullLast, ulNewGdpbTsFull)
                         << Form("old = %12llu vs new = %12llu ", fulStarTsFullLast, ulNewStarTsFull)
                         << Form("old = %2u vs new = %2u ", fuStarDaqCmdLast,  uNewDaqCmd)
                         << Form("old = %2u vs new = %2u ", fuStarTrigCmdLast, uNewTrigCmd)
                         << FairLogger::endl;

         fulGdpbTsFullLast = ulNewGdpbTsFull;
         fulStarTsFullLast = ulNewStarTsFull;
         fuStarTokenLast   = uNewToken;
         fuStarDaqCmdLast  = uNewDaqCmd;
         fuStarTrigCmdLast = uNewTrigCmd;

         Double_t dTot = 1.;
         Double_t dTime = fulGdpbTsFullLast * 6.25;
         if( 0. == fdFirstDigiTimeDif && 0. != fdLastDigiTime )
         {
            fdFirstDigiTimeDif = dTime - fdLastDigiTime;
            LOG(INFO) << "Default fake digi time shift initialized to " << fdFirstDigiTimeDif
                      <<FairLogger::endl;
         } // if( 0. == fdFirstDigiTimeDif && 0. != fdLastDigiTime )

         dTime -= fdFirstDigiTimeDif;
         dTime += fdTShiftRef;

         LOG(DEBUG) << "Insert fake digi with time " << dTime << ", Tot " << dTot
                    << FairLogger::endl;
         fHM->H1( Form("Raw_TRef-Dig0") ) ->Fill( dTime - fdLastDigiTime); // TODO: replace with pointer!!!
         fHM->H1( Form("Raw_TRef-Dig1") ) ->Fill( dTime - fdLastDigiTime); // TODO: replace with pointer!!!

         fDigi = new CbmTofDigiExp(0x00005006, dTime, dTot); // fake start counter signal
         fBuffer->InsertData(fDigi);
         break;
	  } // case 3
      default:
         LOG(FATAL) << "Unknown Star Trigger messageindex: " << iMsgIndex << FairLogger::endl;
  } // switch( iMsgIndex )
}

ClassImp(CbmUnpackTofStar2018)
