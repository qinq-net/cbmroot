// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                  CbmTofStarEventBuilderAlgo2019                   -----
// -----               Created 03.11.2018 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTofStarEventBuilderAlgo2019.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRunOnline.h"

#include "TROOT.h"
#include "TString.h"
#include "THttpServer.h"
#include "TProfile.h"

#include <iostream>
#include <stdint.h>
#include <iomanip>
#include <fstream>

Bool_t CbmTofStarEventBuilderAlgo2019::ProcessMs( const fles::Timeslice& ts, size_t uMsComp, UInt_t uMsIdx )
{
   UInt_t uMsComp = fvMsComponentsList[ uMsCompIdx ];
   auto msDescriptor = ts.descriptor( uMsComp, uMsIdx );
   fiEquipmentId = msDescriptor.eq_id;
   fdMsIndex = static_cast<double>(msDescriptor.idx);
   const uint8_t* msContent = reinterpret_cast<const uint8_t*>( ts.content( uMsComp, uMsIdx ) );

   uint32_t size = msDescriptor.size;
//    fulLastMsIdx = msDescriptor.idx;
   if (size > 0)
      LOG(DEBUG) << "Microslice: " << msDescriptor.idx << " has size: " << size
                 << FairLogger::endl;

   if( 0 == uMsIdx && 0 == uMsCompIdx )
      dTsStartTime = (1e-9) * fdMsIndex;

   if( fdStartTimeMsSz < 0 )
      fdStartTimeMsSz = (1e-9) * fdMsIndex;
   fvhMsSzPerLink[ uMsComp ]->Fill(size);
   if( 2 * fuHistoryHistoSize < (1e-9) * fdMsIndex - fdStartTimeMsSz )
   {
      // Reset the evolution Histogram and the start time when we reach the end of the range
      fvhMsSzTimePerLink[ uMsComp ]->Reset();
      fdStartTimeMsSz = (1e-9) * fdMsIndex;
   } // if( 2 * fuHistoryHistoSize < (1e-9) * fdMsIndex - fdStartTimeMsSz )
   fvhMsSzTimePerLink[ uMsComp ]->Fill((1e-9) * fdMsIndex - fdStartTimeMsSz, size);

   /// If not integer number of message in input buffer, print warning/error
   if (0 != (size % kuBytesPerMessage))
      LOG(ERROR) << "The input microslice buffer does NOT "
                 << "contain only complete nDPB messages!"
                 << FairLogger::endl;

   /// Compute the number of complete messages in the input microslice buffer
   uint32_t uNbMessages = (size - (size % kuBytesPerMessage)) / kuBytesPerMessage;

   /// Get the gDPB ID from the MS header
   fuGdpbId = fiEquipmentId;

   /// Check if this gDPB ID was declared in parameter file and stop there if not
   auto it = fGdpbIdIndexMap.find( fuGdpbId );
   if( it == fGdpbIdIndexMap.end() )
   {
      LOG(FATAL) << "Could not find the gDPB index for AFCK id 0x"
                << std::hex << fuGdpbId << std::dec
                << " in microslice " << fdMsIndex
                << FairLogger::endl
                << "If valid this index has to be added in the TOF parameter file in the RocIdArray field"
                << FairLogger::endl;
      continue;
   } // if( it == fGdpbIdIndexMap.end() )
      else fuGdpbNr = fGdpbIdIndexMap[ fuGdpbId ];

   // Prepare variables for the loop on contents
   const uint64_t* pInBuff = reinterpret_cast<const uint64_t*>(msContent);
   for( uint32_t uIdx = 0; uIdx < uNbMessages; uIdx++ )
   {
      // Fill message
      uint64_t ulData = static_cast<uint64_t>(pInBuff[uIdx]);

      /// Catch the Epoch cycle block which is always the first 64b of the MS
      if( 0 == uIdx )
      {
         ProcessEpochCyle( ulData );
         continue;
      } // if( 0 == uIdx )

      gdpbv100::Message mess(ulData);

      if( fuRawDataPrintMsgIdx < fuRawDataPrintMsgNb || gLogger->IsLogNeeded(DEBUG2) )
      {
         mess.printDataCout();
         fuRawDataPrintMsgIdx ++;
      } // if( fuRawDataPrintMsgIdx < fuRawDataPrintMsgNb || gLogger->IsLogNeeded(DEBUG2) )

      // Increment counter for different message types
      // and fill the corresponding histogram
      messageType = mess.getMessageType();
      fviMsgCounter[messageType]++;
      fhMessType->Fill(messageType);
      fhGdpbMessType->Fill(messageType, fuGdpbNr );

///         fuGet4Id = mess.getGdpbGenChipId();
      fuGet4Id = ConvertElinkToGet4( mess.getGdpbGenChipId() );
      fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + fuGet4Id;

      if( fuNrOfGet4PerGdpb <= fuGet4Id &&
          !mess.isStarTrigger()  &&
          ( gdpbv100::kuChipIdMergedEpoch != fuGet4Id ) )
         LOG(WARNING) << "Message with Get4 ID too high: " << fuGet4Id
                      << " VS " << fuNrOfGet4PerGdpb << " set in parameters." << FairLogger::endl;

      switch (messageType)
      {
         case gdpbv100::MSG_HIT:
         {
            if( mess.getGdpbHitIs24b() )
            {
/*** TODO: Histogram support
               fhGet4MessType->Fill( fuGet4Nr, 4 );
               fvhGdpbGet4MessType[ fuGdpbNr ]->Fill( fuGet4Id, 4 );
               PrintGenInfo(mess);
***/
            } // if( getGdpbHitIs24b() )
               else
               {
/*** TODO: Histogram support
                  fhGet4MessType->Fill( fuGet4Nr, 0 );
                  fvhGdpbGet4MessType[ fuGdpbNr ]->Fill( fuGet4Id, 0 );
***/
                  fvmEpSupprBuffer[fuGet4Nr].push_back( mess );
               } // else of if( getGdpbHitIs24b() )
            break;
         } // case gdpbv100::MSG_HIT:
         case gdpbv100::MSG_EPOCH:
         {
            if( gdpbv100::kuChipIdMergedEpoch == fuGet4Id )
            {
/*** TODO: Histogram support
               if (1 == mess.getGdpbEpSync())
               {
                  fhGdpbEpochFlags->Fill( fuGdpbNr, 0 );
                  fhGdpbEpochSyncEvo->Fill( (1e-9) * fdMsIndex - fdStartTimeMsSz, fuGdpbNr );
               } // if (1 == mess.getGdpbEpSync())

               if (1 == mess.getGdpbEpDataLoss())
                  fhGdpbEpochFlags->Fill( fuGdpbNr, 1 );

               if (1 == mess.getGdpbEpEpochLoss())
                  fhGdpbEpochFlags->Fill( fuGdpbNr, 2 );

               if (1 == mess.getGdpbEpMissmatch())
               {
                  fhGdpbEpochFlags->Fill( fuGdpbNr, 3 );
                  fhGdpbEpochMissEvo->Fill( (1e-9) * fdMsIndex - fdStartTimeMsSz, fuGdpbNr );
               } // if (1 == mess.getGdpbEpMissmatch())
***/

               for( uint32_t uGet4Index = 0; uGet4Index < fuNrOfGet4PerGdpb; uGet4Index ++ )
               {
                  fuGet4Id = uGet4Index;
                  fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + fuGet4Id;
                  gdpbv100::Message tmpMess(mess);
                  tmpMess.setGdpbGenChipId( uGet4Index );

/*** TODO: Histogram support
                  fhGet4MessType->Fill( fuGet4Nr, 1);
                  fvhGdpbGet4MessType[ fuGdpbNr ]->Fill( fuGet4Id, 1 );
***/
                  FillEpochInfo(tmpMess);
               } // for( uint32_t uGet4Index = 0; uGet4Index < fuNrOfGet4PerGdpb; uGetIndex ++ )

               if( kTRUE == fbPrintAllEpochsEnable )
               {
                  LOG(INFO) << "Epoch: " << Form("0x%08x ", fuGdpbId)
                            << ", Merg"
                            << ", Link " << std::setw(1) << mess.getGdpbEpLinkId()
                            << ", epoch " << std::setw(8) << mess.getGdpbEpEpochNb()
                            << ", Sync " << std::setw(1) << mess.getGdpbEpSync()
                            << ", Data loss " << std::setw(1) << mess.getGdpbEpDataLoss()
                            << ", Epoch loss " << std::setw(1) << mess.getGdpbEpEpochLoss()
                            << ", Epoch miss " << std::setw(1) << mess.getGdpbEpMissmatch()
                            << FairLogger::endl;
               } // if( kTRUE == fbPrintAllEpochsEnable )
            } // if this epoch message is a merged one valid for all chips
            else
            {
               fhGet4MessType->Fill( fuGet4Nr, 1);
               fvhGdpbGet4MessType[ fuGdpbNr ]->Fill( fuGet4Id, 1 );
***/
               FillEpochInfo(mess);

               if( kTRUE == fbPrintAllEpochsEnable )
               {
                  LOG(INFO) << "Epoch: " << Form("0x%08x ", fuGdpbId)
                            << ", " << std::setw(4) << fuGet4Nr
                            << ", Link " << std::setw(1) << mess.getGdpbEpLinkId()
                            << ", epoch " << std::setw(8) << mess.getGdpbEpEpochNb()
                            << ", Sync " << std::setw(1) << mess.getGdpbEpSync()
                            << ", Data loss " << std::setw(1) << mess.getGdpbEpDataLoss()
                            << ", Epoch loss " << std::setw(1) << mess.getGdpbEpEpochLoss()
                            << ", Epoch miss " << std::setw(1) << mess.getGdpbEpMissmatch()
                            << FairLogger::endl;
               } // if( kTRUE == fbPrintAllEpochsEnable )
            } // if single chip epoch message
            break;
         } // case gdpbv100::MSG_EPOCH:
         case gdpbv100::MSG_SLOWC:
         {
            fhGet4MessType->Fill( fuGet4Nr, 2);
            fvhGdpbGet4MessType[ fuGdpbNr ]->Fill( fuGet4Id, 2 );
***/
            PrintSlcInfo(mess);
            break;
         } // case gdpbv100::MSG_SLOWC:
         case gdpbv100::MSG_SYST:
         {
/*** TODO: Histogram support
            fhSysMessType->Fill(mess.getGdpbSysSubType());
            fhGdpbSysMessType->Fill(mess.getGdpbSysSubType(), fuGdpbNr );
            if( gdpbv100::SYS_GET4_ERROR == mess.getGdpbSysSubType() )
            {
               fhGet4MessType->Fill( fuGet4Nr, 3);
               fvhGdpbGet4MessType[ fuGdpbNr ]->Fill( fuGet4Id, 3 );

               UInt_t uFeeNr   = (fuGet4Id / fuNrOfGet4PerFee);
               if (0 <= fdStartTime)
               {
                  fvhFeeErrorRate_gDPB[(fuGdpbNr * fuNrOfFeePerGdpb) + uFeeNr]->Fill(
                     1e-9 * (mess.getMsgFullTimeD(fvulCurrentEpoch[fuGet4Nr]) - fdStartTime));
                  fvhFeeErrorRatio_gDPB[(fuGdpbNr * fuNrOfFeePerGdpb) + uFeeNr]->Fill(
                     1e-9 * (mess.getMsgFullTimeD(fvulCurrentEpoch[fuGet4Nr]) - fdStartTime), 1, 1 );
               } // if (0 <= fdStartTime)
               if (0 <= fdStartTimeLong)
               {
                  fvhFeeErrorRateLong_gDPB[(fuGdpbNr * fuNrOfFeePerGdpb) + uFeeNr]->Fill(
                     1e-9 / 60.0 * (mess.getMsgFullTimeD(fvulCurrentEpoch[fuGet4Nr]) - fdStartTimeLong), 1 / 60.0);
                  fvhFeeErrorRatioLong_gDPB[(fuGdpbNr * fuNrOfFeePerGdpb) + uFeeNr]->Fill(
                     1e-9 / 60.0 * (mess.getMsgFullTimeD(fvulCurrentEpoch[fuGet4Nr]) - fdStartTimeLong), 1, 1 / 60.0);
               } // if (0 <= fdStartTime)

               Int_t dGdpbChId =  fuGet4Id * fuNrOfChannelsPerGet4 + mess.getGdpbSysErrChanId();
               Int_t dFullChId =  fuGet4Nr * fuNrOfChannelsPerGet4 + mess.getGdpbSysErrChanId();
               switch( mess.getGdpbSysErrData() )
               {
                  case gdpbv100::GET4_V2X_ERR_READ_INIT:
                     fhGet4ChanErrors->Fill(dFullChId, 0);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 0 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_SYNC:
                     fhGet4ChanErrors->Fill(dFullChId, 1);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 1 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_EP_CNT_SYNC:
                     fhGet4ChanErrors->Fill(dFullChId, 2);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 2 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_EP:
                     fhGet4ChanErrors->Fill(dFullChId, 3);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 3 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_FIFO_WRITE:
                     fhGet4ChanErrors->Fill(dFullChId, 4);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 4 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_LOST_EVT:
                     fhGet4ChanErrors->Fill(dFullChId, 5);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 5 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_CHAN_STATE:
                     fhGet4ChanErrors->Fill(dFullChId, 6);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 6 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_TOK_RING_ST:
                     fhGet4ChanErrors->Fill(dFullChId, 7);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 7 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_TOKEN:
                     fhGet4ChanErrors->Fill(dFullChId, 8);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 8 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_READOUT_ERR:
                     fhGet4ChanErrors->Fill(dFullChId, 9);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 9 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_SPI:
                     fhGet4ChanErrors->Fill(dFullChId, 10);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 10 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_DLL_LOCK:
                     fhGet4ChanErrors->Fill(dFullChId, 11);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 11 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_DLL_RESET:
                     fhGet4ChanErrors->Fill(dFullChId, 12);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 12 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_TOT_OVERWRT:
                     fhGet4ChanErrors->Fill(dFullChId, 13);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 13 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_TOT_RANGE:
                     fhGet4ChanErrors->Fill(dFullChId, 14);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 14 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_EVT_DISCARD:
                     fhGet4ChanErrors->Fill(dFullChId, 15);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 15 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_ADD_RIS_EDG:
                     fhGet4ChanErrors->Fill(dFullChId, 16);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 16 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_UNPAIR_FALL:
                     fhGet4ChanErrors->Fill(dFullChId, 17);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 17 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_SEQUENCE_ER:
                     fhGet4ChanErrors->Fill(dFullChId, 18);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 18 );
                     break;
                  case gdpbv100::GET4_V2X_ERR_UNKNOWN:
                     fhGet4ChanErrors->Fill(dFullChId, 19);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 19 );
                     break;
                  default: // Corrupt error or not yet supported error
                     fhGet4ChanErrors->Fill(dFullChId, 20);
                     fvhGdpbGet4ChanErrors[ fuGdpbNr ]->Fill( dGdpbChId, 20 );
                     break;
               } // Switch( mess.getGdpbSysErrData() )
            } // if( gdpbv100::SYSMSG_GET4_EVENT == mess.getGdpbSysSubType() )
***/
            if( gdpbv100::SYS_PATTERN == mess.getGdpbSysSubType() )
            {
/*** TODO: Histogram support
               fhGdpbSysMessPattType->Fill(mess.getGdpbSysPattType(), fuGdpbNr );
***/
               FillPattInfo( mess );
            } // if( gdpbv100::SYS_PATTERN == mess.getGdpbSysSubType() )
            PrintSysInfo(mess);
            break;
         } // case gdpbv100::MSG_SYST:
         case gdpbv100::MSG_STAR_TRI_A:
         case gdpbv100::MSG_STAR_TRI_B:
         case gdpbv100::MSG_STAR_TRI_C:
         case gdpbv100::MSG_STAR_TRI_D:
            FillStarTrigInfo(mess);
            break;
         default:
            LOG(ERROR) << "Message type " << std::hex
                       << std::setw(2) << static_cast<uint16_t>(messageType)
                       << " not included in Get4 unpacker."
                       << FairLogger::endl;
      } // switch( mess.getMessageType() )
   } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)

   return kTRUE;
}

void CbmTofStarEventBuilderAlgo2019::ProcessEpochCycle( uint64_t ulCycleData )
{
   uint64_t ulEpochCycleVal = ulCycleData & gdpbv100::kulEpochCycleFieldSz;

   if( fuRawDataPrintMsgIdx < fuRawDataPrintMsgNb || gLogger->IsLogNeeded(DEBUG2) )
   {
      LOG(INFO) << "CbmMcbm2018MonitorTof::ProcessEpochCyle => "
                 << Form( " TS %5d MS %3d In data 0x%016X Cycle 0x%016X",
                           fulCurrentTsIndex, fuCurrentMs, ulCycleData, ulEpochCycleVal )
                 << FairLogger::endl;
      fuRawDataPrintMsgIdx ++;
   } // if( fuRawDataPrintMsgIdx < fuRawDataPrintMsgNb || gLogger->IsLogNeeded(DEBUG2) )

   for( uint32_t uGet4Index = 0; uGet4Index < fuNrOfGet4PerGdpb; uGet4Index ++ )
   {
      fuGet4Id = uGet4Index;
      fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + fuGet4Id;
/*
      if( !( ulEpochCycleVal == fvulCurrentEpochCycle[fuGet4Nr] ||
             ulEpochCycleVal == fvulCurrentEpochCycle[fuGet4Nr] + 1 ) )
         LOG(ERROR) << "CbmMcbm2018MonitorTof::ProcessEpochCyle => "
                    << " Missmatch in epoch cycles detected, probably fake cycles due to epoch index corruption! "
                    << Form( " Current cycle 0x%09X New cycle 0x%09X", fvulCurrentEpochCycle[fuGet4Nr], ulEpochCycleVal )
                    << FairLogger::endl;
*/
      fvulCurrentEpochCycle[fuGet4Nr] = ulEpochCycleVal;
   } // for( uint32_t uGet4Index = 0; uGet4Index < fuNrOfGet4PerGdpb; uGet4Index ++ )
   return;
}


void CbmTofStarEventBuilderAlgo2019::ProcessHit( gdpbv100::Message mess )
{
   UInt_t uChannel = mess.getGdpbHitChanId();
   UInt_t uTot     = mess.getGdpbHit32Tot();

   // In 32b mode the coarse counter is already computed back to 112 FTS bins
   // => need to hide its contribution from the Finetime
   // => FTS = Fullt TS modulo 112
   UInt_t uFts     = mess.getGdpbHitFullTs() % 112;

   ULong64_t ulCurEpochGdpbGet4 = fvvulCurrentEpochFull[ fuGdpbNr ][ fuGet4Id ];

   // In Ep. Suppr. Mode, receive following epoch instead of previous
   if( 0 < ulCurEpochGdpbGet4 )
      ulCurEpochGdpbGet4 --;
      else ulCurEpochGdpbGet4 = gdpbv100::kuEpochCounterSz; // Catch epoch cycle!

   UInt_t uChannelNr         = fuGet4Id * fuNrOfChannelsPerGet4 + uChannel;
   UInt_t uChannelNrInFee    = (fuGet4Id % fuNrOfGet4PerFee) * fuNrOfChannelsPerGet4 + uChannel;
   UInt_t uFeeNr             = (fuGet4Id / fuNrOfGet4PerFee);
   UInt_t uFeeNrInSys        = fuGdpbNr * fuNrOfFeePerGdpb + uFeeNr;
   UInt_t uRemappedChannelNr = uFeeNr * fuNrOfChannelsPerFee + fvuGet4ToPadi[ uChannelNrInFee ];
   UInt_t uGbtxNr            = (uFeeNr / kuNbFeePerGbtx);
   UInt_t uFeeInGbtx         = (uFeeNr % kuNbFeePerGbtx);
   UInt_t uGbtxNrInSys       = fuGdpbNr * kuNbGbtxPerGdpb + uGbtxNr;

   ULong_t  ulHitTime = mess.getMsgFullTime(ulCurEpochGdpbGet4);
   Double_t dHitTime  = mess.getMsgFullTimeD(ulCurEpochGdpbGet4);

   ULong_t  ulhitTime = mess.getMsgFullTime(  ulCurEpochGdpbGet4 );
   Double_t dHitTime  = mess.getMsgFullTimeD( ulCurEpochGdpbGet4 );
   Double_t dHitTot   = uTot;     // in bins

   UInt_t uFebIdx     = (uGet4Id / fuNrOfGet4PerFeb);
   UInt_t uFullFebIdx = (fuGdpbNr * fuNrOfFebsPerGdpb) + uFebIdx;

   UInt_t uChanInGdpb = uGet4Id * fuNrOfChannelsPerGet4 + uChannel;
   UInt_t uChanInSyst = fuGdpbNr * fuNrOfChannelsPerGdpb + uChanInGdpb;
   if( fUnpackPar->GetNumberOfChannels() < uChanInSyst )
   {
      LOG(ERROR) << "Invalid mapping index " << uChanInSyst
                 << " VS " << fUnpackPar->GetNumberOfChannels()
                 <<", from " << fuGdpbNr
                 <<", " << uGet4Id
                 <<", " << uChannel
                 << FairLogger::endl;
      return;
   } // if( fUnpackPar->GetNumberOfChannels() < uChanUId )

   /// Store the full message in the proper buffer
   gdpb::FullMessage fullMess( mess, ulCurEpochGdpbGet4 );
   if( fuCurrentMs < fuCoreMs )
      fvBufferMessages[fuGdpbNr].push_back( fullMess );
      else fvBufferMessagesOverlap[fuGdpbNr].push_back( fullMess );

/*** TODO: Histogram support
      /// Histograms filling
         /// In Run rate evolution
      if (fdStartTime < 0)
         fdStartTime = dHitTime;
      if (fdStartTimeLong < 0)
         fdStartTimeLong = dHitTime;
         /// Reset the evolution Histogram and the start time when we reach the end of the range
      if( fuHistoryHistoSize < 1e-9 * (dHitTime - fdStartTime) )
      {
         ResetEvolutionHistograms();
         fdStartTime = dHitTime;
      } // if( fuHistoryHistoSize < 1e-9 * (dHitTime - fdStartTime) )
          /// Reset the long evolution Histogram and the start time when we reach the end of the range
      if( fuHistoryHistoSizeLong < 1e-9 * (dHitTime - fdStartTimeLong) / 60.0 )
      {
         ResetLongEvolutionHistograms();
         fdStartTimeLong = dHitTime;
      } // if( fuHistoryHistoSizeLong < 1e-9 * (dHitTime - fdStartTimeLong) / 60.0 )
***/

}

void CbmTofStarEventBuilderAlgo2019::ProcessEpoch( gdpbv100::Message mess )
{
   ULong64_t ulEpochNr = mess.getGdpbEpEpochNb();

   if( fvulCurrentEpoch[ fuGdpbNr ][ fuGet4Id ] < ulEpochNr )
      fvulCurrentEpochCycle[ fuGdpbNr ][ fuGet4Id ]++;

   fvulCurrentEpoch[ fuGdpbNr ][ fuGet4Id ] = ulEpochNr;
   fvulCurrentEpochFull[ fuGdpbNr ][ fuGet4Id ] = ulEpochNr + gdpbv100::kulEpochCycleBins * fvulCurrentEpochCycle[ fuGdpbNr ][ fuGet4Id ];

   fulCurrentEpochTime = mess.getMsgFullTime(ulEpochNr);

   /// Re-align the epoch number of the message in case it will be used later:
   /// We received the epoch after the data instead of the one before!
   if( 0 < ulEpochNr )
      mess.setGdpbEpEpochNb( ulEpochNr - 1 );
      else mess.setGdpbEpEpochNb( gdpbv100::kuEpochCounterSz );

   Int_t iBufferSize = fvmEpSupprBuffer[ fuGdpbNr ][ fuGet4Id ].size();
   if( 0 < iBufferSize )
   {
      LOG(DEBUG) << "Now processing stored messages for for get4 " << fuGet4Nr << " with epoch number "
                 << (fvulCurrentEpoch[fuGet4Nr] - 1) << FairLogger::endl;

      /// Data are sorted between epochs, not inside => Epoch level ordering
      /// Sorting at lower bin precision level
      std::stable_sort( fvmEpSupprBuffer[ fuGdpbNr ][ fuGet4Id ].begin(), fvmEpSupprBuffer[ fuGdpbNr ][ fuGet4Id ].begin() );

      for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
      {
         ProcessHit( fvmEpSupprBuffer[ fuGdpbNr ][ fuGet4Id ][ iMsgIdx ] );
      } // for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )

      fvmEpSupprBuffer[ fuGdpbNr ][ fuGet4Id ].clear();
   } // if( 0 < fvmEpSupprBuffer[fuGet4Nr] )

/*** TODO: Histogram support
   if (1 == mess.getGdpbEpSync())
      fhGet4EpochFlags->Fill(fuGet4Nr, 0);
   if (1 == mess.getGdpbEpDataLoss())
      fhGet4EpochFlags->Fill(fuGet4Nr, 1);
   if (1 == mess.getGdpbEpEpochLoss())
      fhGet4EpochFlags->Fill(fuGet4Nr, 2);
   if (1 == mess.getGdpbEpMissmatch())
      fhGet4EpochFlags->Fill(fuGet4Nr, 3);
***/
}

void CbmTofStarEventBuilderAlgo2019::ProcessSlCtrl( gdpbv100::Message mess )
{
/*** TODO: Use epoch suppression buffer to go in the right epoch!!! ***/

   /// Store the full message in the proper buffer
   gdpb::FullMessage fullMess( mess, ulCurEpochGdpbGet4 );
   if( fuCurrentMs < fuCoreMs )
      fvBufferMessages[fuGdpbNr].push_back( fullMess );
      else fvBufferMessagesOverlap[fuGdpbNr].push_back( fullMess );
}

void CbmTofStarEventBuilderAlgo2019::ProcessSysMess( gdpbv100::Message mess )
{
/*** TODO: Use epoch suppression buffer to go in the right epoch!!! ***/
   /// TODO FIXME: Add the error messages to the buffer for inclusion in the STAR event
   ///             NEEDS the proper "<" operator in FullMessage or Message to allow time sorting
   gdpb::FullMessage fullMess( mess, ulCurEpochGdpbGet4 );
   if( fuCurrentMs < fuCoreMs )
      fvBufferMessages[fuGdpbNr].push_back( fullMess );
      else fvBufferMessagesOverlap[fuGdpbNr].push_back( fullMess );

   switch( mess.getGdpbSysSubType() )
   {
      case gdpbv100::SYS_GET4_ERROR:
      {
         uint32_t uData = mess.getGdpbSysErrData();
         if( gdpbv100::GET4_V2X_ERR_TOT_OVERWRT == uData
          || gdpbv100::GET4_V2X_ERR_TOT_RANGE   == uData
          || gdpbv100::GET4_V2X_ERR_EVT_DISCARD == uData
          || gdpbv100::GET4_V2X_ERR_ADD_RIS_EDG == uData
          || gdpbv100::GET4_V2X_ERR_UNPAIR_FALL == uData
          || gdpbv100::GET4_V2X_ERR_SEQUENCE_ER == uData
           )
            LOG(DEBUG) << " +++++++ > gDPB: " << std::hex << std::setw(4) << fuGdpbId
                       << std::dec << ", Chip = " << std::setw(2)
                       << mess.getGdpbGenChipId() << ", Chan = " << std::setw(1)
                       << mess.getGdpbSysErrChanId() << ", Edge = "
                       << std::setw(1) << mess.getGdpbSysErrEdge() << ", Empt = "
                       << std::setw(1) << mess.getGdpbSysErrUnused()
                       << ", Data = " << std::hex << std::setw(2) << uData
                       << std::dec << " -- GET4 V1 Error Event"
                       << FairLogger::endl;
            else LOG(DEBUG) << " +++++++ >gDPB: " << std::hex << std::setw(4) << fuGdpbId
                            << std::dec << ", Chip = " << std::setw(2)
                            << mess.getGdpbGenChipId() << ", Chan = " << std::setw(1)
                            << mess.getGdpbSysErrChanId() << ", Edge = "
                            << std::setw(1) << mess.getGdpbSysErrEdge() << ", Empt = "
                            << std::setw(1) << mess.getGdpbSysErrUnused()
                            << ", Data = " << std::hex << std::setw(2) << uData
                            << std::dec << " -- GET4 V1 Error Event "
                            << FairLogger::endl;
         break;
      } // case gdpbv100::SYSMSG_GET4_EVENT
      case gdpbv100::SYS_GDPB_UNKWN:
      {
         LOG(DEBUG) << "Unknown GET4 message, data: " << std::hex << std::setw(8)
                    << mess.getGdpbSysUnkwData() << std::dec
                    <<" Full message: " << std::hex << std::setw(16)
                    << mess.getData() << std::dec
                    << FairLogger::endl;
         break;
      } // case gdpbv100::SYS_GDPB_UNKWN:
      case gdpbv100::SYS_GET4_SYNC_MISS:
      {
         LOG(DEBUG) << "GET4 synchronization pulse missing" << FairLogger::endl;
         break;
      } // case gdpbv100::SYS_GET4_SYNC_MISS:
      case gdpbv100::SYS_PATTERN:
      {
         LOG(DEBUG) << "ASIC pattern for missmatch, disable or resync" << FairLogger::endl;
         break;
      } // case gdpbv100::SYS_PATTERN:
      default:
      {
         LOG(DEBUG) << "Crazy system message, subtype " << mess.getGdpbSysSubType() << FairLogger::endl;
         break;
      } // default

   } // switch( getGdpbSysSubType() )
}

void CbmTofStarEventBuilderAlgo2019::ProcessPattern( gdpbv100::Message mess )
{
/*** TODO: Use epoch suppression buffer to go in the right epoch!!! ***/
   /// TODO FIXME: Add the error messages to the buffer for inclusion in the STAR event
   ///             NEEDS the proper "<" operator in FullMessage or Message to allow time sorting
   gdpb::FullMessage fullMess( mess, ulCurEpochGdpbGet4 );
   if( fuCurrentMs < fuCoreMs )
      fvBufferMessages[fuGdpbNr].push_back( fullMess );
      else fvBufferMessagesOverlap[fuGdpbNr].push_back( fullMess );

   uint16_t usType   = mess.getGdpbSysPattType();
   uint16_t usIndex  = mess.getGdpbSysPattIndex();
   uint32_t uPattern = mess.getGdpbSysPattPattern();

/*** TODO: Histogram support
   switch( usType )
   {
      case gdpbv100::PATT_MISSMATCH:
      {
         LOG(INFO) << Form( "Missmatch pattern message => Type %d, Index %2d, Pattern 0x%08X", usType, usIndex, uPattern )
                   << FairLogger::endl;
         for( UInt_t uBit = 0; uBit < 32; ++uBit )
            if( ( uPattern >> uBit ) & 0x1 )
            {
               fhPatternMissmatch->Fill( 32 * usIndex + uBit, fuGdpbNr );
               fvhGdpbPatternMissmatchEvo[ fuGdpbNr ]->Fill( fulCurrentTsIndex, 32 * usIndex + uBit );
            } // if( ( uPattern >> uBit ) & 0x1 )

         break;
      } // case gdpbv100::PATT_MISSMATCH:
      case gdpbv100::PATT_ENABLE:
      {
         for( UInt_t uBit = 0; uBit < 32; ++uBit )
            if( ( uPattern >> uBit ) & 0x1 )
            {
               fhPatternEnable->Fill( 32 * usIndex + uBit, fuGdpbNr );
               fvhGdpbPatternEnableEvo[ fuGdpbNr ]->Fill( fulCurrentTsIndex, 32 * usIndex + uBit );
            } // if( ( uPattern >> uBit ) & 0x1 )

         break;
      } // case gdpbv100::PATT_ENABLE:
      case gdpbv100::PATT_RESYNC:
      {
         LOG(INFO) << Form( "RESYNC pattern message => Type %d, Index %2d, Pattern 0x%08X", usType, usIndex, uPattern )
                   << FairLogger::endl;

         for( UInt_t uBit = 0; uBit < 32; ++uBit )
            if( ( uPattern >> uBit ) & 0x1 )
            {
               fhPatternResync->Fill( 32 * usIndex + uBit, fuGdpbNr );
               fvhGdpbPatternResyncEvo[ fuGdpbNr ]->Fill( fulCurrentTsIndex, 32 * usIndex + uBit );
            } // if( ( uPattern >> uBit ) & 0x1 )

         break;
      } // case gdpbv100::PATT_RESYNC:
      default:
      {
         LOG(DEBUG) << "Crazy pattern message, subtype " << usType << FairLogger::endl;
         break;
      } // default
   } // switch( usType )
***/

   return;
}
