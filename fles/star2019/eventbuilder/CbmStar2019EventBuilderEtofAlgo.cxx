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


Bool_t CbmTofStarEventBuilderAlgo2019::ProcessTs( const fles::Timeslice& ts )
{
   fulCurrentTsIndex = ts.index();
   fdTsStartTime = static_cast< Double_t >( ts.descriptor( 0, 0 ).idx );
   Double_t dTsStopTimeCore = fdTsStartTime + fdMsDuration * fuNbCoreMsPerTs;

   /// Compute the limits for accepting hits and trigger in thie TS, for eac gDPB/sector
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      if( fdStarTriggerDelay[ uGdpb ] < 0.0 )
      {
         /// Event window for this gDPB starts before the trigger
         if( fdStarTriggerDelay[ uGdpb ] + fdStarTriggerWinSize[ uGdpb ] < 0.0 )
         {
            /// Event window for this gDPB is fully before the trigger
            fdMessCandidateTimeStart[ uGdpb ] = fdTsStartTime;
            // << Accept more than needed as this should be safer and small amounts >>
            fdMessCandidateTimeStop[ uGdpb ]  = dTsStopTimeCore + fdAllowedTriggersSpread + 2.0 * fdStarTriggerWinSize[ uGdpb ]; // + fdStarTriggerWinSize[ uGdpb ];
            fdTrigCandidateTimeStart[ uGdpb ] = fdTsStartTime   + fdAllowedTriggersSpread - fdStarTriggerDelay[ uGdpb ];
            fdTrigCandidateTimeStop[ uGdpb ]  = dTsStopTimeCore + fdAllowedTriggersSpread - fdStarTriggerDelay[ uGdpb ]

         } // if( fdStarTriggerDelay[ uGdpb ] + fdStarTriggerWinSize[ uGdpb ] < 0.0 )
            else
            {
               /// Event window for this gDPB is on both sides of the trigger
               fdMessCandidateTimeStart[ uGdpb ] = fdTsStartTime;
               // << Accept more than needed as this should be safer and small amounts >>
               fdMessCandidateTimeStop[ uGdpb ]  = dTsStopTimeCore + fdAllowedTriggersSpread + fdStarTriggerWinSize[ uGdpb ]; // + fdStarTriggerDelay[ uGdpb ];
               fdTrigCandidateTimeStart[ uGdpb ] = fdTsStartTime   + fdAllowedTriggersSpread - fdStarTriggerDelay[ uGdpb ];
               fdTrigCandidateTimeStop[ uGdpb ]  = dTsStopTimeCore + fdAllowedTriggersSpread - fdStarTriggerDelay[ uGdpb ]
            } // else of if( fdStarTriggerDelay[ uGdpb ] + fdStarTriggerWinSize[ uGdpb ] < 0.0 )
      } // if( fdStarTriggerDeadtime[ uGdpb ] < 0.0 )
         else
         {
            /// Event window for this gDPB starts after the trigger => fully after
             // << Accept more than needed as this should be safer and small amounts >>
            fdMessCandidateTimeStart[ uGdpb ] = fdTsStartTime; // - fdAllowedTriggersSpread + fdStarTriggerDelay[ uGdpb ];
            fdMessCandidateTimeStop[ uGdpb ]  = dTsStopTimeCore + fdAllowedTriggersSpread + fdStarTriggerDelay[ uGdpb ] + fdStarTriggerWinSize[ uGdpb ];
            fdTrigCandidateTimeStart[ uGdpb ] = fdTsStartTime   + fdAllowedTriggersSpread ;
            fdTrigCandidateTimeStop[ uGdpb ]  = dTsStopTimeCore + fdAllowedTriggersSpread;
         } // else of if( fdStarTriggerDelay[ uGdpb ] < 0.0 )
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

   /// Loop over registered components
   for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsList.size(); ++uMsCompIdx )
   {
      UInt_t uMsComp = fvMsComponentsList[ uMsCompIdx ];

      /// Loop over core microslices (and overlap ones if chosen)
      for( UInt_t uMsIdx = 0; uMsIdx < uNbMsLoop; uMsIdx ++ )
      {
         if( kFALSE == ProcessMs( ts, uMsComp, uMsIdx ) )
         {
            LOG(ERROR) << "Failed to process ts " << fulCurrentTsIndex << " MS " << uMsIdx
                       << " for component " << uMsComp
                       << FairLogger::endl;
            return kFALSE;
         } // if( kFALSE == ProcessMs( ts, uMsCompIdx, uMsIdx ) )
      } // for( UInt_t uMsIdx = 0; uMsIdx < uNbMsLoop; uMsIdx ++ )
   } // for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsList.size(); ++uMsCompIdx )

   return kTRUE;
}

Bool_t CbmTofStarEventBuilderAlgo2019::ProcessMs( const fles::Timeslice& ts, size_t uMsComp, UInt_t uMsIdx )
{
   UInt_t uMsComp = fvMsComponentsList[ uMsCompIdx ];
   auto msDescriptor = ts.descriptor( uMsComp, uMsIdx );

   /// Store MS/TS information
   fuCurrentMs       = uMsIdx;
   fiEquipmentId     = msDescriptor.eq_id;
   fdMsIndex         = static_cast<double>(msDescriptor.idx);
   uint32_t size     = msDescriptor.size;
//    fulLastMsIdx = msDescriptor.idx;
   if (size > 0)
      LOG(DEBUG) << "Microslice: " << msDescriptor.idx << " has size: " << size
                 << FairLogger::endl;

   /// Get pointer on MS data
   const uint8_t* msContent = reinterpret_cast<const uint8_t*>( ts.content( uMsComp, uMsIdx ) );

   if( 0 == uMsIdx && 0 == uMsCompIdx )
      dTsStartTime = (1e-9) * fdMsIndex;

   if( fdStartTimeMsSz < 0 )
      fdStartTimeMsSz = (1e-9) * fdMsIndex;
/*
   fvhMsSzPerLink[ uMsComp ]->Fill(size);
   if( 2 * fuHistoryHistoSize < (1e-9) * fdMsIndex - fdStartTimeMsSz )
   {
      // Reset the evolution Histogram and the start time when we reach the end of the range
      fvhMsSzTimePerLink[ uMsComp ]->Reset();
      fdStartTimeMsSz = (1e-9) * fdMsIndex;
   } // if( 2 * fuHistoryHistoSize < (1e-9) * fdMsIndex - fdStartTimeMsSz )
   fvhMsSzTimePerLink[ uMsComp ]->Fill((1e-9) * fdMsIndex - fdStartTimeMsSz, size);
*/

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

   /// Store the last STAR trigger values for the core MS when reaching the first overlap MS, needed to check for clones
   if( fuNbCoreMsPerTs == fuCurrentMs )
   {
      fulGdpbTsFullLastCore[ fuGdpbNr ] = fulGdpbTsFullLast[ fuGdpbNr ];
      fulStarTsFullLastCore[ fuGdpbNr ] = fulStarTsFullLast[ fuGdpbNr ];
      fuStarTokenLastCore[ fuGdpbNr ]   = fuStarTokenLast[ fuGdpbNr ];
      fuStarDaqCmdLastCore[ fuGdpbNr ]  = fuStarDaqCmdLast[ fuGdpbNr ];
      fuStarTrigCmdLastCore[ fuGdpbNr ] = fuStarTrigCmdLast[ fuGdpbNr ];
   } // if( fuNbCoreMsPerTs == fuCurrentMs )

   /// Restore the last STAR trigger values for the core MS when reaching the first core MS, needed to check for clones
   if( 0 == fuCurrentMs )
   {
      fulGdpbTsFullLast[ fuGdpbNr ] = fulGdpbTsFullLastCore[ fuGdpbNr ];
      fulStarTsFullLast[ fuGdpbNr ] = fulStarTsFullLastCore[ fuGdpbNr ];
      fuStarTokenLast[ fuGdpbNr ]   = fuStarTokenLastCore[ fuGdpbNr ];
      fuStarDaqCmdLast[ fuGdpbNr ]  = fuStarDaqCmdLastCore[ fuGdpbNr ];
      fuStarTrigCmdLast[ fuGdpbNr ] = fuStarTrigCmdLastCore[ fuGdpbNr ];
   } // if( 0 == fuCurrentMs )

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

// -------------------------------------------------------------------------
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

void CbmTofStarEventBuilderAlgo2019::ProcessStarTrigger(gdpb::Message mess)
{
   Int_t iMsgIndex = mess.getStarTrigMsgIndex();

   switch( iMsgIndex )
   {
      case 0:
         fulGdpbTsMsb[fuGdpbNr] = mess.getGdpbTsMsbStarA();
         break;
      case 1:
         fulGdpbTsLsb[fuGdpbNr] = mess.getGdpbTsLsbStarB();
         fulStarTsMsb[fuGdpbNr] = mess.getStarTsMsbStarB();
         break;
      case 2:
         fulStarTsMid[fuGdpbNr] = mess.getStarTsMidStarC();
         break;
      case 3:
      {
         ULong64_t ulNewGdpbTsFull = ( fulGdpbTsMsb[fuGdpbNr] << 24 )
                                   + ( fulGdpbTsLsb[fuGdpbNr]       );
         ULong64_t ulNewStarTsFull = ( fulStarTsMsb[fuGdpbNr] << 48 )
                                   + ( fulStarTsMid[fuGdpbNr] <<  8 )
                                   + mess.getStarTsLsbStarD();
         UInt_t uNewToken  = mess.getStarTokenStarD();
         UInt_t uNewDaqCmd  = mess.getStarDaqCmdStarD();
         UInt_t uNewTrigCmd = mess.getStarTrigCmdStarD();

/*
         UInt_t uNewTrigWord =  ( (uNewTrigCmd & 0x00F) << 16 )
                  + ( (uNewDaqCmd   & 0x00F) << 12 )
                  + ( (uNewToken    & 0xFFF)       );
         LOG(INFO) << "New STAR trigger "
                   << " TS " << fulCurrentTsIndex
                   << " gDBB #" << fuGdpbNr << " "
                   << Form("token = %5u ", uNewToken )
                   << Form("gDPB ts  = %12llu ", ulNewGdpbTsFull )
                   << Form("STAR ts = %12llu ", ulNewStarTsFull )
                   << Form("DAQ cmd = %2u ", uNewDaqCmd )
                   << Form("TRG cmd = %2u ", uNewTrigCmd )
                   << Form("TRG Wrd = %5x ", uNewTrigWord )
                   << FairLogger::endl;
*/

         if( ( uNewToken == fuStarTokenLast[fuGdpbNr] ) && ( ulNewGdpbTsFull == fulGdpbTsFullLast[fuGdpbNr] ) &&
             ( ulNewStarTsFull == fulStarTsFullLast[fuGdpbNr] ) && ( uNewDaqCmd == fuStarDaqCmdLast[fuGdpbNr] ) &&
             ( uNewTrigCmd == fuStarTrigCmdLast[fuGdpbNr] ) )
         {
            UInt_t uTrigWord =  ( (fuStarTrigCmdLast[fuGdpbNr] & 0x00F) << 16 )
                     + ( (fuStarDaqCmdLast[fuGdpbNr]   & 0x00F) << 12 )
                     + ( (fuStarTokenLast[fuGdpbNr]    & 0xFFF)       );
            LOG(WARNING) << "Possible error: identical STAR tokens found twice in a row => ignore 2nd! "
                         << " TS " << fulCurrentTsIndex
                         << " gDBB #" << fuGdpbNr << " "
                         << Form("token = %5u ", fuStarTokenLast[fuGdpbNr] )
                         << Form("gDPB ts  = %12llu ", fulGdpbTsFullLast[fuGdpbNr] )
                         << Form("STAR ts = %12llu ", fulStarTsFullLast[fuGdpbNr] )
                         << Form("DAQ cmd = %2u ", fuStarDaqCmdLast[fuGdpbNr] )
                         << Form("TRG cmd = %2u ", fuStarTrigCmdLast[fuGdpbNr] )
                         << Form("TRG Wrd = %5x ", uTrigWord )
                         << FairLogger::endl;
            return;
         } // if exactly same message repeated

         // GDPB TS counter reset detection
         if( ulNewGdpbTsFull < fulGdpbTsFullLast[fuGdpbNr] )
            LOG(DEBUG) << "Probable reset of the GDPB TS: old = " << Form("%16llu", fulGdpbTsFullLast[fuGdpbNr])
                       << " new = " << Form("%16llu", ulNewGdpbTsFull)
                       << " Diff = -" << Form("%8llu", fulGdpbTsFullLast[fuGdpbNr] - ulNewGdpbTsFull)
                       << " GDPB #" << Form( "%2u", fuGdpbNr)
                       << FairLogger::endl;

         // STAR TS counter reset detection
         if( ulNewStarTsFull < fulStarTsFullLast[fuGdpbNr] )
            LOG(DEBUG) << "Probable reset of the STAR TS: old = " << Form("%16llu", fulStarTsFullLast[fuGdpbNr])
                       << " new = " << Form("%16llu", ulNewStarTsFull)
                       << " Diff = -" << Form("%8llu", fulStarTsFullLast[fuGdpbNr] - ulNewStarTsFull)
                       << " GDPB #" << Form( "%2u", fuGdpbNr)
                       << FairLogger::endl;

/*
         LOG(INFO) << "Updating  trigger token for " << fuGdpbNr
                   << " " << fuStarTokenLast[fuGdpbNr] << " " << uNewToken
                   << FairLogger::endl;
*/

         /// Generate Trigger object and store it for event building ///
         CbmTofStarTrigger2019 newTrig( fulGdpbTsFullLast[fuGdpbNr], fulStarTsFullLast[fuGdpbNr], fuStarTokenLast[fuGdpbNr],
                                        fuStarDaqCmdLast[fuGdpbNr], fuStarTrigCmdLast[fuGdpbNr] );
         if( fuCurrentMs < fuNbCoreMsPerTs )
            fvBufferTriggers[fuGdpbNr].push_back( newTrig );
            else fvBufferTriggersOverlap[fuGdpbNr].push_back( newTrig );

         if( fuCurrentMs < fuNbCoreMsPerTs )
         {
            ULong64_t ulGdpbTsDiff = ulNewGdpbTsFull - fulGdpbTsFullLast[fuGdpbNr];
            fulGdpbTsFullLast[fuGdpbNr] = ulNewGdpbTsFull;
            fulStarTsFullLast[fuGdpbNr] = ulNewStarTsFull;
            fuStarTokenLast[fuGdpbNr]   = uNewToken;
            fuStarDaqCmdLast[fuGdpbNr]  = uNewDaqCmd;
            fuStarTrigCmdLast[fuGdpbNr] = uNewTrigCmd;
         }
         ///---------------------------------------------------------///
         break;
      } // case 3
      default:
         LOG(ERROR) << "Unknown Star Trigger messageindex: " << iMsgIndex << FairLogger::endl;
   } // switch( iMsgIndex )
}

// -------------------------------------------------------------------------
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

   /// Compute original epoch index before epoch suppression
   ULong64_t ulCurEpochGdpbGet4 = fvvulCurrentEpochFull[ fuGdpbNr ][ fuGet4Id ];

   /// Ignore the first epoch as it should never appear (start delay!!)
   if( 0 == ulCurEpochGdpbGet4 )
      return;

   /// In Ep. Suppr. Mode, receive following epoch instead of previous
   ulCurEpochGdpbGet4 --;

   for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
   {

      messageType = fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ].getMessageType();

      fuGet4Id = ConvertElinkToGet4( fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ].getGdpbGenChipId() );
      fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + fuGet4Id;

      /// Store the full message in the proper buffer
      gdpb::FullMessage fullMess( mess, ulCurEpochGdpbGet4 );
/* ==> Try to shift the conditions to buffer filling!
      if( fuCurrentMs < fuNbCoreMsPerTs )
         fvBufferMessages[fuGdpbNr].push_back( fullMess );
         else fvBufferMessagesOverlap[fuGdpbNr].push_back( fullMess );
*/

      if(

      /// Do other actions on it if needed
      switch (messageType)
      {
         case gdpbv100::MSG_HIT:
         {
            ProcessHit( fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ], ulCurEpochGdpbGet4 );
            break;
         } // case gdpbv100::MSG_HIT:
         case gdpbv100::MSG_SLOWC:
         {
            ProcessSlCtrl( fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ], ulCurEpochGdpbGet4 );
            break;
         } // case gdpbv100::MSG_SLOWC:
         case gdpbv100::MSG_SYST:
         {
            if( gdpbv100::SYS_PATTERN == fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ].getGdpbSysSubType() )
            {
               ProcessPattern( fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ], ulCurEpochGdpbGet4 );
            } // if( gdpbv100::SYS_PATTERN == fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ].getGdpbSysSubType() )
               else ProcessSysMess( fvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ], ulCurEpochGdpbGet4 );
            break;
         } // case gdpbv100::MSG_SYST:
         case gdpbv100::MSG_EPOCH:
         case gdpbv100::MSG_STAR_TRI_A:
         case gdpbv100::MSG_STAR_TRI_B:
         case gdpbv100::MSG_STAR_TRI_C:
         case gdpbv100::MSG_STAR_TRI_D:
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

// -------------------------------------------------------------------------
void CbmTofStarEventBuilderAlgo2019::ProcessHit( gdpbv100::Message mess, uint64_t ulCurEpochGdpbGet4 )
{
   UInt_t uChannel = mess.getGdpbHitChanId();
   UInt_t uTot     = mess.getGdpbHit32Tot();

   // In 32b mode the coarse counter is already computed back to 112 FTS bins
   // => need to hide its contribution from the Finetime
   // => FTS = Fullt TS modulo 112
   UInt_t uFts     = mess.getGdpbHitFullTs() % 112;

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

}

void CbmTofStarEventBuilderAlgo2019::ProcessSlCtrl( gdpbv100::Message mess, uint64_t ulCurEpochGdpbGet4 )
{
}

void CbmTofStarEventBuilderAlgo2019::ProcessSysMess( gdpbv100::Message mess, uint64_t ulCurEpochGdpbGet4 )
{
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

void CbmTofStarEventBuilderAlgo2019::ProcessPattern( gdpbv100::Message mess, uint64_t ulCurEpochGdpbGet4 )
{
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
// -------------------------------------------------------------------------
