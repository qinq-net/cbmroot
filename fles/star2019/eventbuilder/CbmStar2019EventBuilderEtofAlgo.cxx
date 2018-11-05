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
               LOG(FATAL) << "This event builder does not support 24b hit message!!!."
                          << FairLogger::endl;
            } // if( getGdpbHitIs24b() )
               else
               {
                  fvmEpSupprBuffer[fuGdpbNr].push_back( mess );
               } // else of if( getGdpbHitIs24b() )
            break;
         } // case gdpbv100::MSG_HIT:
         case gdpbv100::MSG_EPOCH:
         {
            if( gdpbv100::kuChipIdMergedEpoch == fuGet4Id )
            {
               ProcessEpoch(mess);

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
                  LOG(FATAL) << "This event builder does not support unmerged epoch messages!!!."
                             << FairLogger::endl;
               } // if single chip epoch message
            break;
         } // case gdpbv100::MSG_EPOCH:
         case gdpbv100::MSG_SLOWC:
         {
            fvmEpSupprBuffer[fuGdpbNr].push_back( mess );
            break;
         } // case gdpbv100::MSG_SLOWC:
         case gdpbv100::MSG_SYST:
         {
            fvmEpSupprBuffer[fuGdpbNr].push_back( mess );
            break;
         } // case gdpbv100::MSG_SYST:
         case gdpbv100::MSG_STAR_TRI_A:
         case gdpbv100::MSG_STAR_TRI_B:
         case gdpbv100::MSG_STAR_TRI_C:
         case gdpbv100::MSG_STAR_TRI_D:
            ProcessStarTrigger( mess );
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

/*
      if( !( ulEpochCycleVal == fvulCurrentEpochCycle[fuGet4Nr] ||
             ulEpochCycleVal == fvulCurrentEpochCycle[fuGet4Nr] + 1 ) )
         LOG(ERROR) << "CbmMcbm2018MonitorTof::ProcessEpochCyle => "
                    << " Missmatch in epoch cycles detected, probably fake cycles due to epoch index corruption! "
                    << Form( " Current cycle 0x%09X New cycle 0x%09X", fvulCurrentEpochCycle[fuGet4Nr], ulEpochCycleVal )
                    << FairLogger::endl;
*/
   fvulCurrentEpochCycle[fuGdpbNr] = ulEpochCycleVal;
   
   return;
}

void CbmTofStarEventBuilderAlgo2019::ProcessEpoch( gdpbv100::Message mess )
{
   ULong64_t ulEpochNr = mess.getGdpbEpEpochNb();

   if( fvulCurrentEpoch[ fuGdpbNr ] < ulEpochNr )
      fvulCurrentEpochCycle[ fuGdpbNr ]++;

   fvulCurrentEpoch[ fuGdpbNr ] = ulEpochNr;
   fvulCurrentEpochFull[ fuGdpbNr ] = ulEpochNr + gdpbv100::kulEpochCycleBins * fvulCurrentEpochCycle[ fuGdpbNr ];

   fulCurrentEpochTime = mess.getMsgFullTime(ulEpochNr);

   /// Re-align the epoch number of the message in case it will be used later:
   /// We received the epoch after the data instead of the one before!
   if( 0 < ulEpochNr )
      mess.setGdpbEpEpochNb( ulEpochNr - 1 );
      else mess.setGdpbEpEpochNb( gdpbv100::kuEpochCounterSz );

   /// Process the corresponding messages buffer
   ProcessEpSupprBuffer( fuGdpbNr );
}

void CbmTofStarEventBuilderAlgo2019::ProcessEpSupprBuffer( uint32_t uGdpbNr )
{
   Int_t iBufferSize = fvmEpSupprBuffer[ fuGdpbNr ].size();

   if( 0 == iBufferSize )
      return;

   LOG(DEBUG) << "Now processing stored messages for for gDPB " << fuGdpbNr << " with epoch number "
              << (fvulCurrentEpoch[fuGdpbNr] - 1) << FairLogger::endl;

   /// Data are sorted between epochs, not inside => Epoch level ordering
   /// Sorting at lower bin precision level
   std::stable_sort( fvmEpSupprBuffer[ fuGdpbNr ].begin(), fvmEpSupprBuffer[ fuGdpbNr ].begin() );
   
   for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
   {
      
      messageType = fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ].getMessageType();

      fuGet4Id = ConvertElinkToGet4( fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ].getGdpbGenChipId() );
      fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + fuGet4Id;
      
      switch (messageType)
      {
         case gdpbv100::MSG_HIT:
         {
            if( fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ].getGdpbHitIs24b() )
            {
               LOG(FATAL) << "This event builder does not support 24b hit message!!!."
                          << FairLogger::endl;
            } // if( getGdpbHitIs24b() )
               else
               {
                  ProcessHit( fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ] );
               } // else of if( getGdpbHitIs24b() )
            break;
         } // case gdpbv100::MSG_HIT:
         case gdpbv100::MSG_SLOWC:
         {
            ProcessSlCtrl( fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ] );
            break;
         } // case gdpbv100::MSG_SLOWC:
         case gdpbv100::MSG_SYST:
         {
            if( gdpbv100::SYS_PATTERN == fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ].getGdpbSysSubType() )
            {
               ProcessPattern( fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ] );
            } // if( gdpbv100::SYS_PATTERN == fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ].getGdpbSysSubType() )
               else ProcessSysMess( fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ] );
            break;
         } // case gdpbv100::MSG_SYST:
         case gdpbv100::MSG_STAR_TRI_A:
         case gdpbv100::MSG_STAR_TRI_B:
         case gdpbv100::MSG_STAR_TRI_C:
         case gdpbv100::MSG_STAR_TRI_D:
            FillStarTrigInfo( fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ] );
            break;
         default:
            LOG(ERROR) << "Message type " << std::hex
                       << std::setw(2) << static_cast<uint16_t>(messageType)
                       << " not included in Get4 unpacker."
                       << FairLogger::endl;
      } // switch( mess.getMessageType() )
   } // for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
   
   fvmEpSupprBuffer[ fuGdpbNr ].clear();
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
}

void CbmTofStarEventBuilderAlgo2019::ProcessSlCtrl( gdpbv100::Message mess )
{
   /// Store the full message in the proper buffer
   gdpb::FullMessage fullMess( mess, ulCurEpochGdpbGet4 );
   if( fuCurrentMs < fuCoreMs )
      fvBufferMessages[fuGdpbNr].push_back( fullMess );
      else fvBufferMessagesOverlap[fuGdpbNr].push_back( fullMess );
}

void CbmTofStarEventBuilderAlgo2019::ProcessSysMess( gdpbv100::Message mess )
{
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

   switch( usType )
   {
      case gdpbv100::PATT_MISSMATCH:
      {
         LOG(INFO) << Form( "Missmatch pattern message => Type %d, Index %2d, Pattern 0x%08X", usType, usIndex, uPattern )
                   << FairLogger::endl;

         break;
      } // case gdpbv100::PATT_MISSMATCH:
      case gdpbv100::PATT_ENABLE:
      {
         LOG(DEBUG2) << Form( "Enable pattern message => Type %d, Index %2d, Pattern 0x%08X", usType, usIndex, uPattern )
                   << FairLogger::endl;
/*
         for( UInt_t uBit = 0; uBit < 32; ++uBit )
            if( ( uPattern >> uBit ) & 0x1 )
            {
               fhPatternEnable->Fill( 32 * usIndex + uBit, fuGdpbNr );
               fvhGdpbPatternEnableEvo[ fuGdpbNr ]->Fill( fulCurrentTsIndex, 32 * usIndex + uBit );
            } // if( ( uPattern >> uBit ) & 0x1 )
*/
         break;
      } // case gdpbv100::PATT_ENABLE:
      case gdpbv100::PATT_RESYNC:
      {
         LOG(INFO) << Form( "RESYNC pattern message => Type %d, Index %2d, Pattern 0x%08X", usType, usIndex, uPattern )
                   << FairLogger::endl;

         break;
      } // case gdpbv100::PATT_RESYNC:
      default:
      {
         LOG(DEBUG) << "Crazy pattern message, subtype " << usType << FairLogger::endl;
         break;
      } // default
   } // switch( usType )

   return;
}
