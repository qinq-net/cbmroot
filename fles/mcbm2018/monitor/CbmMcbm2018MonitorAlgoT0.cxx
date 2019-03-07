// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                  CbmMcbm2018MonitorAlgoT0                         -----
// -----               Created 10.02.2019 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018MonitorAlgoT0.h"

#include "CbmMcbm2018TofPar.h"
#include "CbmTofAddress.h"
#include "CbmTofDetectorId_v14a.h" // in cbmdata/tof

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRunOnline.h"

#include "TROOT.h"
#include "TList.h"
#include "TString.h"
#include "TH2.h"
#include "TProfile.h"
#include "TH1.h"
#include "TCanvas.h"

#include <iostream>
#include <stdint.h>
#include <iomanip>
#include <fstream>

// -------------------------------------------------------------------------
CbmMcbm2018MonitorAlgoT0::CbmMcbm2018MonitorAlgoT0() :
   CbmStar2019Algo(),
   /// From the class itself
   fbMonitorMode( kFALSE ),
   fbDebugMonitorMode( kFALSE ),
   fvbMaskedComponents(),
   fUnpackPar( nullptr ),
   fuNrOfGdpbs( 0 ),
   fGdpbIdIndexMap(),
   fuNrOfFeePerGdpb( 0 ),
   fuNrOfGet4PerFee( 0 ),
   fuNrOfChannelsPerGet4( 0 ),
   fuNrOfChannelsPerFee( 0 ),
   fuNrOfGet4( 0 ),
   fuNrOfGet4PerGdpb( 0 ),
   fuNrOfChannelsPerGdpb( 0 ),
   fulCurrentTsIdx( 0 ),
   fulCurrentMsIdx( 0 ),
   fdTsStartTime( -1.0 ),
   fdTsStopTimeCore( -1.0 ),
   fdMsTime( -1.0 ),
   fuMsIndex( 0 ),
   fmMsgCounter(),
   fuCurrentEquipmentId( 0 ),
   fuCurrDpbId( 0 ),
   fuCurrDpbIdx( 0 ),
   fiRunStartDateTimeSec( -1 ),
   fiBinSizeDatePlots( -1 ),
   fuGet4Id( 0 ),
   fuGet4Nr( 0 ),
   fvulCurrentEpoch(),
   fvulCurrentEpochCycle(),
   fvulCurrentEpochFull(),
   fdStartTime( - 1.0 ),
   fdStartTimeMsSz( 0.0 ),
   ftStartTimeUnix( std::chrono::steady_clock::now() ),
   fvvmEpSupprBuffer(),
   fvmHitsInMs(),
   fuHistoryHistoSize( 3600 ),
   fvuHitCntChanMs( kuNbChanDiamond, 0 ),
   fvuErrorCntChanMs( kuNbChanDiamond, 0  ),
   fvuEvtLostCntChanMs( kuNbChanDiamond, 0  ),
   fvhHitCntEvoChan( kuNbChanDiamond, nullptr ),
   fvhHitCntPerMsEvoChan( kuNbChanDiamond, nullptr ),
   fvhErrorCntEvoChan( kuNbChanDiamond, nullptr ),
   fvhErrorCntPerMsEvoChan( kuNbChanDiamond, nullptr ),
   fvhEvtLostCntEvoChan( kuNbChanDiamond, nullptr ),
   fvhEvtLostCntPerMsEvoChan( kuNbChanDiamond, nullptr ),
   fvhErrorFractEvoChan( kuNbChanDiamond, nullptr ),
   fvhErrorFractPerMsEvoChan( kuNbChanDiamond, nullptr ),
   fvhEvtLostFractEvoChan( kuNbChanDiamond, nullptr ),
   fvhEvtLostFractPerMsEvoChan( kuNbChanDiamond, nullptr ),
   fhChannelMap( nullptr ),
   fhHitMapEvo( nullptr ),
   fhMsgCntEvo( nullptr ),
   fhHitCntEvo( nullptr ),
   fhErrorCntEvo( nullptr ),
   fhLostEvtCntEvo( nullptr ),
   fhErrorFractEvo( nullptr ),
   fhLostEvtFractEvo( nullptr ),
   fhMsgCntPerMsEvo( nullptr ),
   fhHitCntPerMsEvo( nullptr ),
   fhErrorCntPerMsEvo( nullptr ),
   fhLostEvtCntPerMsEvo( nullptr ),
   fhErrorFractPerMsEvo( nullptr ),
   fhLostEvtFractPerMsEvo( nullptr ),
   fcSummary( nullptr ),
   fcHitMaps( nullptr ),
   fcGenCntsPerMs( nullptr )
{
}
CbmMcbm2018MonitorAlgoT0::~CbmMcbm2018MonitorAlgoT0()
{
   /// Clear buffers
   fvmHitsInMs.clear();
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
//      fvmHitsInMs[ uDpb ].clear();
      fvvmEpSupprBuffer[ uGdpb ].clear();
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
}

// -------------------------------------------------------------------------
Bool_t CbmMcbm2018MonitorAlgoT0::Init()
{
   LOG(INFO) << "Initializing mCBM T0 2019 monitor algo" << FairLogger::endl;

   return kTRUE;
}
void CbmMcbm2018MonitorAlgoT0::Reset()
{
}
void CbmMcbm2018MonitorAlgoT0::Finish()
{
   /// Printout Goodbye message and stats

   /// Write Output histos

}

// -------------------------------------------------------------------------
Bool_t CbmMcbm2018MonitorAlgoT0::InitContainers()
{
   LOG(INFO) << "Init parameter containers for CbmMcbm2018MonitorAlgoT0"
             << FairLogger::endl;
   Bool_t initOK = ReInitContainers();

   return initOK;
}
Bool_t CbmMcbm2018MonitorAlgoT0::ReInitContainers()
{
   LOG(INFO) << "**********************************************"
             << FairLogger::endl;
   LOG(INFO) << "ReInit parameter containers for CbmMcbm2018MonitorAlgoT0"
             << FairLogger::endl;

   fUnpackPar = (CbmMcbm2018TofPar*)fParCList->FindObject("CbmMcbm2018TofPar");
   if( nullptr == fUnpackPar )
      return kFALSE;

   Bool_t initOK = InitParameters();

   return initOK;
}
TList* CbmMcbm2018MonitorAlgoT0::GetParList()
{
   if( nullptr == fParCList )
      fParCList = new TList();
   fUnpackPar = new CbmMcbm2018TofPar("CbmMcbm2018TofPar");
   fParCList->Add(fUnpackPar);

   return fParCList;
}
Bool_t CbmMcbm2018MonitorAlgoT0::InitParameters()
{

   fuNrOfGdpbs = fUnpackPar->GetNrOfGdpbs();
   LOG(INFO) << "Nr. of Tof GDPBs: " << fuNrOfGdpbs << FairLogger::endl;

   fuNrOfFeePerGdpb = fUnpackPar->GetNrOfFeesPerGdpb();
   LOG(INFO) << "Nr. of FEES per Tof GDPB: " << fuNrOfFeePerGdpb
               << FairLogger::endl;

   fuNrOfGet4PerFee = fUnpackPar->GetNrOfGet4PerFee();
   LOG(INFO) << "Nr. of GET4 per Tof FEE: " << fuNrOfGet4PerFee
               << FairLogger::endl;

   fuNrOfChannelsPerGet4 = fUnpackPar->GetNrOfChannelsPerGet4();
   LOG(INFO) << "Nr. of channels per GET4: " << fuNrOfChannelsPerGet4
               << FairLogger::endl;

   fuNrOfChannelsPerFee = fuNrOfGet4PerFee * fuNrOfChannelsPerGet4;
   LOG(INFO) << "Nr. of channels per FEE: " << fuNrOfChannelsPerFee
               << FairLogger::endl;

   fuNrOfGet4 = fuNrOfGdpbs * fuNrOfFeePerGdpb * fuNrOfGet4PerFee;
   LOG(INFO) << "Nr. of GET4s: " << fuNrOfGet4 << FairLogger::endl;

   fuNrOfGet4PerGdpb = fuNrOfFeePerGdpb * fuNrOfGet4PerFee;
   LOG(INFO) << "Nr. of GET4s per GDPB: " << fuNrOfGet4PerGdpb
               << FairLogger::endl;

   fuNrOfChannelsPerGdpb = fuNrOfGet4PerGdpb * fuNrOfChannelsPerGet4;
   LOG(INFO) << "Nr. of channels per GDPB: " << fuNrOfChannelsPerGdpb
               << FairLogger::endl;

   fGdpbIdIndexMap.clear();
   for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )
   {
      fGdpbIdIndexMap[fUnpackPar->GetGdpbId(i)] = i;
      LOG(INFO) << "GDPB Id of TOF  " << i << " : " << std::hex << fUnpackPar->GetGdpbId(i)
                 << std::dec << FairLogger::endl;
   } // for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )

   /// Internal status initialization
   fvulCurrentEpoch.resize( fuNrOfGdpbs, 0 );
   fvulCurrentEpochCycle.resize( fuNrOfGdpbs, 0 );
   fvulCurrentEpochFull.resize( fuNrOfGdpbs, 0 );

   /// Buffer initialization
   fvvmEpSupprBuffer.resize( fuNrOfGdpbs );

	return kTRUE;
}
// -------------------------------------------------------------------------

void CbmMcbm2018MonitorAlgoT0::AddMsComponentToList( size_t component, UShort_t usDetectorId )
{
   /// Check for duplicates and ignore if it is the case
   for( UInt_t uCompIdx = 0; uCompIdx < fvMsComponentsList.size(); ++uCompIdx )
      if( component == fvMsComponentsList[ uCompIdx ] )
         return;

   /// Add to list
   fvMsComponentsList.push_back( component );

   LOG(INFO) << "CbmMcbm2018MonitorAlgoT0::AddMsComponentToList => Component "
             << component << " with detector ID 0x"
             << std::hex << usDetectorId << std::dec << " added to list"
             << FairLogger::endl;

}
// -------------------------------------------------------------------------

Bool_t CbmMcbm2018MonitorAlgoT0::ProcessTs( const fles::Timeslice& ts )
{
   fulCurrentTsIdx = ts.index();
   fdTsStartTime = static_cast< Double_t >( ts.descriptor( 0, 0 ).idx );

   /// On first TS, extract the TS parameters from header (by definition stable over time)
   if( -1.0 == fdTsCoreSizeInNs )
   {
      fuNbCoreMsPerTs = ts.num_core_microslices();
      fuNbOverMsPerTs = ts.num_microslices( 0 ) - ts.num_core_microslices();
      fdTsCoreSizeInNs = fdMsSizeInNs * ( fuNbCoreMsPerTs );
      fdTsFullSizeInNs = fdMsSizeInNs * ( fuNbCoreMsPerTs + fuNbOverMsPerTs );
      LOG(INFO) << "Timeslice parameters: each TS has "
                << fuNbCoreMsPerTs << " Core MS and "
                << fuNbOverMsPerTs << " Overlap MS, for a core duration of "
                << fdTsCoreSizeInNs << " ns and a full duration of "
                << fdTsFullSizeInNs << " ns"
                << FairLogger::endl;

      /// Ignore overlap ms if flag set by user
      fuNbMsLoop = fuNbCoreMsPerTs;
      if( kFALSE == fbIgnoreOverlapMs )
         fuNbMsLoop += fuNbOverMsPerTs;
      LOG(INFO) << "In each TS " << fuNbMsLoop << " MS will be looped over"
                << FairLogger::endl;
   } // if( -1.0 == fdTsCoreSizeInNs )

   /// Compute time of TS core end
   fdTsStopTimeCore = fdTsStartTime + fdTsCoreSizeInNs;
//      LOG(INFO) << Form( "TS %5d Start %12f Stop %12f", fulCurrentTsIdx, fdTsStartTime, fdTsStopTimeCore )
//                << FairLogger::endl;

   /// Loop over core microslices (and overlap ones if chosen)
   for( fuMsIndex = 0; fuMsIndex < fuNbMsLoop; fuMsIndex ++ )
   {
      /// Loop over registered components
      for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsList.size(); ++uMsCompIdx )
      {
         UInt_t uMsComp = fvMsComponentsList[ uMsCompIdx ];

         if( kFALSE == ProcessMs( ts, uMsComp, fuMsIndex ) )
         {
            LOG(ERROR) << "Failed to process ts " << fulCurrentTsIdx << " MS " << fuMsIndex
                       << " for component " << uMsComp
                       << FairLogger::endl;
            return kFALSE;
         } // if( kFALSE == ProcessMs( ts, uMsCompIdx, fuMsIndex ) )
      } // for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsList.size(); ++uMsCompIdx )
/*
      /// Sort the buffers of hits
      std::sort( fvmHitsInMs.begin(), fvmHitsInMs.end() );

      /// Add the hits to the output buffer as Digis
      for( auto itHitIn = fvmHitsInMs.begin(); itHitIn < fvmHitsInMs.end(); ++itHitIn )
      {
         UInt_t uFebIdx =  itHitIn->GetAsic() / fUnpackPar->GetNbAsicsPerFeb()
                          + ( itHitIn->GetDpb() * fUnpackPar->GetNbCrobsPerDpb() + itHitIn->GetCrob() )
                            * fUnpackPar->GetNbFebsPerCrob();
         UInt_t uChanInFeb = itHitIn->GetChan()
                            + fUnpackPar->GetNbChanPerAsic() * (itHitIn->GetAsic() % fUnpackPar->GetNbAsicsPerFeb());

         ULong64_t ulTimeInNs = static_cast< ULong64_t >( itHitIn->GetTs() * stsxyter::kdClockCycleNs - fdTimeOffsetNs );

         fDigiVect.push_back( CbmTofDigi( fviFebAddress[ uFebIdx ], uChanInFeb, ulTimeInNs, itHitIn->GetAdc() ) );
      } // for( auto itHitIn = fvmHitsInMs.begin(); itHitIn < fvmHitsInMs.end(); ++itHitIn )
*/
      /// Clear the buffer of hits
      fvmHitsInMs.clear();
   } // for( fuMsIndex = 0; fuMsIndex < uNbMsLoop; fuMsIndex ++ )

   /// Clear buffers to prepare for the next TS
   fvmHitsInMs.clear();
/*
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fvmHitsInMs[ uDpb ].clear();
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
*/
   /// Fill plots if in monitor mode
   if( fbMonitorMode )
   {
      if( kFALSE == FillHistograms() )
      {
         LOG(ERROR) << "Failed to fill histos in ts " << fulCurrentTsIdx
                    << FairLogger::endl;
         return kFALSE;
      } // if( kFALSE == FillHistograms() )
   } // if( fbMonitorMode )

   return kTRUE;
}

Bool_t CbmMcbm2018MonitorAlgoT0::ProcessMs( const fles::Timeslice& ts, size_t uMsCompIdx, size_t uMsIdx )
{
   auto msDescriptor = ts.descriptor( uMsCompIdx, uMsIdx );
   fuCurrentEquipmentId = msDescriptor.eq_id;
   const uint8_t* msContent = reinterpret_cast<const uint8_t*>( ts.content( uMsCompIdx, uMsIdx ) );

   uint32_t uSize  = msDescriptor.size;
   fulCurrentMsIdx = msDescriptor.idx;
   fdMsTime = (1e-9) * static_cast<double>(fulCurrentMsIdx);
   LOG(DEBUG) << "Microslice: " << fulCurrentMsIdx
              << " from EqId " << std::hex << fuCurrentEquipmentId << std::dec
              << " has size: " << uSize << FairLogger::endl;

   if( 0 == fvbMaskedComponents.size() )
      fvbMaskedComponents.resize( ts.num_components(), kFALSE );

   fuCurrDpbId  = static_cast< uint32_t >( fuCurrentEquipmentId & 0xFFFF );
//   fuCurrDpbIdx = fDpbIdIndexMap[ fuCurrDpbId ];

   /// Check if this sDPB ID was declared in parameter file and stop there if not
   auto it = fGdpbIdIndexMap.find( fuCurrDpbId );
   if( it == fGdpbIdIndexMap.end() )
   {
       if( kFALSE == fvbMaskedComponents[ uMsCompIdx ] )
       {
          LOG(INFO) << "---------------------------------------------------------------"
                    << FairLogger::endl;
          LOG(INFO) << "hi hv eqid flag si sv idx/start        crc      size     offset"
                    << FairLogger::endl;
          LOG(INFO) << Form( "%02x %02x %04x %04x %02x %02x %016lx %08x %08x %016lx",
                            static_cast<unsigned int>(msDescriptor.hdr_id),
                            static_cast<unsigned int>(msDescriptor.hdr_ver), msDescriptor.eq_id, msDescriptor.flags,
                            static_cast<unsigned int>(msDescriptor.sys_id),
                            static_cast<unsigned int>(msDescriptor.sys_ver), msDescriptor.idx, msDescriptor.crc,
                            msDescriptor.size, msDescriptor.offset )
                    << FairLogger::endl;
         LOG(WARNING) << "Could not find the gDPB index for AFCK id 0x"
                   << std::hex << fuCurrDpbId << std::dec
                   << " in timeslice " << fulCurrentTsIdx
                   << " in microslice " << uMsIdx
                   << " component " << uMsCompIdx
                   << "\n"
                   << "If valid this index has to be added in the TOF parameter file in the DbpIdArray field"
                   << FairLogger::endl;
         fvbMaskedComponents[ uMsCompIdx ] = kTRUE;
      } // if( kFALSE == fvbMaskedComponents[ uMsComp ] )
         else return kTRUE;

      /// Try to get it from the second message in buffer (first is epoch cycle without gDPB ID)
      /// TODO!!!!

      return kFALSE;
   } // if( it == fGdpbIdIndexMap.end() )
      else fuCurrDpbIdx = fGdpbIdIndexMap[ fuCurrDpbId ];

   /// Save start time of first valid MS )
   if( fdStartTime < 0 )
      fdStartTime = fdMsTime;

   // If not integer number of message in input buffer, print warning/error
   if( 0 != ( uSize % kuBytesPerMessage ) )
      LOG(ERROR) << "The input microslice buffer does NOT "
                 << "contain only complete nDPB messages!"
                 << FairLogger::endl;

   // Compute the number of complete messages in the input microslice buffer
   uint32_t uNbMessages = ( uSize - ( uSize % kuBytesPerMessage ) )
                          / kuBytesPerMessage;

   // Prepare variables for the loop on contents
   Int_t messageType = -111;
   const uint64_t* pInBuff = reinterpret_cast<const uint64_t*>( msContent );
   for( uint32_t uIdx = 0; uIdx < uNbMessages; uIdx++ )
   {
      // Fill message
      uint64_t ulData = static_cast<uint64_t>(pInBuff[uIdx]);

      /// Catch the Epoch cycle block which is always the first 64b of the MS
      if( 0 == uIdx )
      {
//         ProcessEpochCycle( ulData );
         continue;
      } // if( 0 == uIdx )

      gdpbv100::Message mess(ulData);
      /// Get message type
      messageType = mess.getMessageType();

      fuGet4Id = mess.getGdpbGenChipId();;
      fuGet4Nr = (fuCurrDpbIdx * fuNrOfGet4PerGdpb) + fuGet4Id;
//      UInt_t uChannelT0 = ( fuGet4Id < 32 ) ? ( fuGet4Id / 8 ) : (fuGet4Id / 8 - 1); /// December 2018 mapping
      UInt_t uChannelT0 = fuGet4Id / 2 + 4 * fuCurrDpbIdx; /// 2019 mapping with 320/640 Mb/s FW

      if( fuNrOfGet4PerGdpb <= fuGet4Id &&
          !mess.isStarTrigger()  &&
          ( gdpbv100::kuChipIdMergedEpoch != fuGet4Id ) )
         LOG(WARNING) << "Message with Get4 ID too high: " << fuGet4Id
                      << " VS " << fuNrOfGet4PerGdpb << " set in parameters." << FairLogger::endl;

      switch( messageType )
      {
         case gdpbv100::MSG_HIT:
         {
            if( mess.getGdpbHitIs24b() )
            {
               LOG(ERROR) << "This event builder does not support 24b hit message!!!."
                          << FairLogger::endl;
               continue;
            } // if( getGdpbHitIs24b() )
               else
               {
                  fhErrorFractEvo->Fill( fdMsTime - fdStartTime, 0.0 );
                  fhLostEvtFractEvo->Fill( fdMsTime - fdStartTime, 0.0 );

                  fhMsgCntEvo->Fill( fdMsTime - fdStartTime );
                  fhHitCntEvo->Fill( fdMsTime - fdStartTime );

                  fvhHitCntEvoChan[ uChannelT0 ]->Fill( fdMsTime - fdStartTime );

                  fvuHitCntChanMs[ uChannelT0 ]++;

                  fhChannelMap->Fill( uChannelT0 );
                  fhHitMapEvo->Fill( fdMsTime - fdStartTime, uChannelT0 );
//                  fvvmEpSupprBuffer[fuCurrDpbIdx].push_back( mess );
               } // else of if( getGdpbHitIs24b() )
            break;
         } // case gdpbv100::MSG_HIT:
         case gdpbv100::MSG_EPOCH:
         {
/*
            if( gdpbv100::kuChipIdMergedEpoch == fuGet4Id )
            {
               ProcessEpoch(mess);
            } // if this epoch message is a merged one valid for all chips
               else
               {
                  /// Should be checked in monitor task, here we just jump it
                  LOG(DEBUG2) << "This event builder does not support unmerged epoch messages!!!."
                             << FairLogger::endl;
                  continue;
               } // if single chip epoch message
*/
            break;
         } // case gdpbv100::MSG_EPOCH:
         case gdpbv100::MSG_SLOWC:
         {
//            fvvmEpSupprBuffer[fuCurrDpbIdx].push_back( mess );
            break;
         } // case gdpbv100::MSG_SLOWC:
         case gdpbv100::MSG_SYST:
         {
            if( gdpbv100::SYS_GET4_ERROR == mess.getGdpbSysSubType() )
            {
               fhErrorFractEvo->Fill( fdMsTime - fdStartTime, 0.0 );
               fhLostEvtFractEvo->Fill( fdMsTime - fdStartTime, 0.0 );

               fhMsgCntEvo->Fill( fdMsTime - fdStartTime );
               fhErrorCntEvo->Fill( fdMsTime - fdStartTime );
               fhErrorFractEvo->Fill( fdMsTime - fdStartTime, 1.0 );

               fvhErrorCntEvoChan[ uChannelT0 ]->Fill( fdMsTime - fdStartTime );

               fvuErrorCntChanMs[ uChannelT0 ]++;
               if( gdpbv100::GET4_V2X_ERR_LOST_EVT == mess.getGdpbSysErrData() )
               {
                  fhLostEvtCntEvo->Fill( fdMsTime - fdStartTime );
                  fhLostEvtFractEvo->Fill( fdMsTime - fdStartTime, 1.0 );

                  fvhEvtLostCntEvoChan[ uChannelT0 ]->Fill( fdMsTime - fdStartTime );

                  fvuEvtLostCntChanMs[ uChannelT0 ]++;
               } // if( gdpbv100::GET4_V2X_ERR_LOST_EVT == mess.getGdpbSysErrData() )
            } // if( gdpbv100::SYS_GET4_ERROR == mess.getGdpbSysSubType() )
//            fvvmEpSupprBuffer[fuCurrDpbIdx].push_back( mess );
            break;
         } // case gdpbv100::MSG_SYST:
         case gdpbv100::MSG_STAR_TRI_A:
         case gdpbv100::MSG_STAR_TRI_B:
         case gdpbv100::MSG_STAR_TRI_C:
         case gdpbv100::MSG_STAR_TRI_D:
         {
            break;
         } // case gdpbv100::MSG_STAR_TRI_A-D
         default:
            LOG(ERROR) << "Message type " << std::hex
                       << std::setw(2) << static_cast<uint16_t>(messageType)
                       << " not included in Get4 unpacker."
                       << FairLogger::endl;
      } // switch( mess.getMessageType() )
   } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)

   /// Fill histograms
   FillHistograms();

   return kTRUE;
}
/*
// -------------------------------------------------------------------------
void CbmMcbm2018MonitorAlgoT0::ProcessEpochCycle( uint64_t ulCycleData )
{
   ULong64_t ulEpochCycleVal = ulCycleData & gdpbv100::kulEpochCycleFieldSz;

   if( !( ulEpochCycleVal == fvulCurrentEpochCycle[fuCurrDpbIdx] ||
          ulEpochCycleVal == fvulCurrentEpochCycle[fuCurrDpbIdx] + 1 ) &&
       0 < fulCurrentMsIdx ) {
      LOG(WARNING) << "CbmMcbm2018MonitorAlgoT0::ProcessEpochCycle => "
                 << " Missmatch in epoch cycles detected for Gdpb " << fuCurrDpbIdx <<", probably fake cycles due to epoch index corruption! "
                 << Form( " Current cycle 0x%09llX New cycle 0x%09llX", fvulCurrentEpochCycle[fuCurrDpbIdx], ulEpochCycleVal )
                 << FairLogger::endl;
   } // if epoch cycle did not stay constant or increase by exactly 1, except if first MS of the TS
   if( ulEpochCycleVal != fvulCurrentEpochCycle[fuCurrDpbIdx] )
   {
      LOG(INFO) << "CbmStar2019EventBuilderEtofAlgo::ProcessEpochCycle => "
                 << " New epoch cycle for Gdpb " << fuCurrDpbIdx
                 << Form( ": Current cycle 0x%09llX New cycle 0x%09llX", fvulCurrentEpochCycle[fuCurrDpbIdx], ulEpochCycleVal )
                 << FairLogger::endl;
   } // if( ulEpochCycleVal != fvulCurrentEpochCycle[fuCurrDpbIdx] )
   fvulCurrentEpochCycle[fuCurrDpbIdx] = ulEpochCycleVal;

   return;
}
void CbmMcbm2018MonitorAlgoT0::ProcessEpoch( gdpbv100::Message mess )
{
   ULong64_t ulEpochNr = mess.getGdpbEpEpochNb();

   fvulCurrentEpoch[ fuCurrDpbIdx ] = ulEpochNr;
   fvulCurrentEpochFull[ fuCurrDpbIdx ] = ulEpochNr + ( gdpbv100::kuEpochCounterSz + 1 ) * fvulCurrentEpochCycle[ fuCurrDpbIdx ];

   /// Re-align the epoch number of the message in case it will be used later:
   /// We received the epoch after the data instead of the one before!
   if( 0 < ulEpochNr )
      mess.setGdpbEpEpochNb( ulEpochNr - 1 );
      else mess.setGdpbEpEpochNb( gdpbv100::kuEpochCounterSz );

   /// Process the corresponding messages buffer for current gDPB
   ProcessEpSupprBuffer();
}
// -------------------------------------------------------------------------
void CbmMcbm2018MonitorAlgoT0::ProcessEpSupprBuffer()
{
   Int_t iBufferSize = fvvmEpSupprBuffer[ fuCurrDpbIdx ].size();

   if( 0 == iBufferSize )
      return;

   LOG(DEBUG) << "Now processing stored messages for for gDPB " << fuCurrDpbIdx << " with epoch number "
              << (fvulCurrentEpoch[fuCurrDpbIdx] - 1) << FairLogger::endl;

   /// Data are sorted between epochs, not inside => Epoch level ordering
   /// Sorting at lower bin precision level
   std::stable_sort( fvvmEpSupprBuffer[ fuCurrDpbIdx ].begin(), fvvmEpSupprBuffer[ fuCurrDpbIdx ].begin() );

   /// Compute original epoch index before epoch suppression
   ULong64_t ulCurEpochGdpbGet4 = fvulCurrentEpochFull[ fuCurrDpbIdx ];

   /// Ignore the first epoch as it should never appear (start delay!!)
   if( 0 == ulCurEpochGdpbGet4 )
      return;

   /// In Ep. Suppr. Mode, receive following epoch instead of previous
   ulCurEpochGdpbGet4 --;

   Int_t messageType = -111;
   for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
   {
      messageType = fvvmEpSupprBuffer[ fuCurrDpbIdx ][ iMsgIdx ].getMessageType();

      fuGet4Id = fUnpackPar->ElinkIdxToGet4Idx( fvvmEpSupprBuffer[ fuCurrDpbIdx ][ iMsgIdx ].getGdpbGenChipId() );
      if( fuDiamondDpbIdx == fuCurrDpbIdx )
         fuGet4Id = fvvmEpSupprBuffer[ fuCurrDpbIdx ][ iMsgIdx ].getGdpbGenChipId();
      fuGet4Nr = (fuCurrDpbIdx * fuNrOfGet4PerGdpb) + fuGet4Id;

      /// Store the full message in the proper buffer
      gdpbv100::FullMessage fullMess( fvvmEpSupprBuffer[ fuCurrDpbIdx ][ iMsgIdx ], ulCurEpochGdpbGet4 );

      /// Do other actions on it if needed
      switch( messageType )
      {
         case gdpbv100::MSG_HIT:
         {
            ProcessHit( fullMess );
            break;
         } // case gdpbv100::MSG_HIT:
         case gdpbv100::MSG_SLOWC:
         {
            ProcessSlCtrl( fullMess );
            break;
         } // case gdpbv100::MSG_SLOWC:
         case gdpbv100::MSG_SYST:
         {
            ProcessSysMess( fullMess );
            break;
         } // case gdpbv100::MSG_SYST:
         case gdpbv100::MSG_EPOCH:
         case gdpbv100::MSG_STAR_TRI_A:
         case gdpbv100::MSG_STAR_TRI_B:
         case gdpbv100::MSG_STAR_TRI_C:
         case gdpbv100::MSG_STAR_TRI_D:
            /// Should never appear there
            break;
         default:
            LOG(ERROR) << "Message type " << std::hex
                       << std::setw(2) << static_cast<uint16_t>(messageType)
                       << " not included in Get4 unpacker."
                       << FairLogger::endl;
      } // switch( mess.getMessageType() )
   } // for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )

   fvvmEpSupprBuffer[ fuCurrDpbIdx ].clear();
}
// -------------------------------------------------------------------------
void CbmMcbm2018MonitorAlgoT0::ProcessHit( gdpbv100::FullMessage mess )
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
   UInt_t uFeeNrInSys        = fuCurrDpbIdx * fuNrOfFeePerGdpb + uFeeNr;
   UInt_t uRemappedChannelNr = uFeeNr * fuNrOfChannelsPerFee + fUnpackPar->Get4ChanToPadiChan( uChannelNrInFee );
   UInt_t uGbtxNr            = (uFeeNr / fUnpackPar->GetNrOfFeePerGbtx());
   UInt_t uFeeInGbtx         = (uFeeNr % fUnpackPar->GetNrOfFeePerGbtx());
   UInt_t uGbtxNrInSys       = fuCurrDpbIdx * fUnpackPar->GetNrOfGbtxPerGdpb() + uGbtxNr;

   UInt_t uChanInSyst = fuCurrDpbIdx * fuNrOfChannelsPerGdpb + uChannelNr;
   UInt_t uRemappedChannelNrInSys = fuCurrDpbIdx * fuNrOfChannelsPerGdpb
                                   + uFeeNr * fuNrOfChannelsPerFee
                                   + fUnpackPar->Get4ChanToPadiChan( uChannelNrInFee );
   /// Diamond FEE have straight connection from Get4 to eLink and from PADI to GET4
   if( fuDiamondDpbIdx == fuCurrDpbIdx )
   {
      uRemappedChannelNr      = uChannelNr;
      uRemappedChannelNrInSys = fuCurrDpbIdx * fUnpackPar->GetNrOfChannelsPerGdpb() + uChannelNr;
   } // if( fuDiamondDpbIdx == fuCurrDpbIdx )

   ULong_t  ulHitTime = mess.getMsgFullTime(  mess.getExtendedEpoch() );
   Double_t dHitTime  = mess.GetFullTimeNs();
   Double_t dHitTot   = uTot;     // in bins

}
// -------------------------------------------------------------------------
void CbmMcbm2018MonitorAlgoT0::ProcessSlCtrl( gdpbv100::FullMessage mess )
{
}
// -------------------------------------------------------------------------
void CbmMcbm2018MonitorAlgoT0::ProcessSysMess( gdpbv100::FullMessage mess )
{
   switch( mess.getGdpbSysSubType() )
   {
      case gdpbv100::SYS_GET4_ERROR:
      {
         ProcessError( mess );
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
         if( mess.getGdpbSysFwErrResync() )
            LOG(INFO) << Form( "GET4 Resynchronization: Get4:0x%04x ", mess.getGdpbGenChipId() ) << fuCurrDpbIdx
                       << FairLogger::endl;
            else LOG(INFO) << "GET4 synchronization pulse missing in gDPB " << fuCurrDpbIdx
                            << FairLogger::endl;
         break;
      } // case gdpbv100::SYS_GET4_SYNC_MISS:
      case gdpbv100::SYS_PATTERN:
      {
         ProcessPattern( mess );
         break;
      } // case gdpbv100::SYS_PATTERN:
      default:
      {
         LOG(INFO) << "Crazy system message, subtype " << mess.getGdpbSysSubType() << FairLogger::endl;
         break;
      } // default
   } // switch( mess.getGdpbSysSubType() )
}
void CbmMcbm2018MonitorAlgoT0::ProcessError( gdpbv100::FullMessage mess )
{
   uint32_t uErrorType = mess.getGdpbSysErrData();

   switch( uErrorType )
   {
      case gdpbv100::GET4_V2X_ERR_READ_INIT:
      case gdpbv100::GET4_V2X_ERR_SYNC:
      case gdpbv100::GET4_V2X_ERR_EP_CNT_SYNC:
      case gdpbv100::GET4_V2X_ERR_EP:
      case gdpbv100::GET4_V2X_ERR_FIFO_WRITE:
      case gdpbv100::GET4_V2X_ERR_CHAN_STATE:
      case gdpbv100::GET4_V2X_ERR_TOK_RING_ST:
      case gdpbv100::GET4_V2X_ERR_TOKEN:
      case gdpbv100::GET4_V2X_ERR_READOUT_ERR:
      case gdpbv100::GET4_V2X_ERR_DLL_LOCK:
      case gdpbv100::GET4_V2X_ERR_DLL_RESET:
         /// Critical errors
         break;
      case gdpbv100::GET4_V2X_ERR_SPI:
         /// Error during SPI communication with slave (e.g. PADI)
         break;
      case gdpbv100::GET4_V2X_ERR_LOST_EVT:
      case gdpbv100::GET4_V2X_ERR_TOT_OVERWRT:
      case gdpbv100::GET4_V2X_ERR_TOT_RANGE:
      case gdpbv100::GET4_V2X_ERR_EVT_DISCARD:
      case gdpbv100::GET4_V2X_ERR_ADD_RIS_EDG:
      case gdpbv100::GET4_V2X_ERR_UNPAIR_FALL:
      case gdpbv100::GET4_V2X_ERR_SEQUENCE_ER:
         /// Input channel realted errors (TOT, shaky signals, etc...)
         break;
      case gdpbv100::GET4_V2X_ERR_EPOCH_OVERF:
         break;
      case gdpbv100::GET4_V2X_ERR_UNKNOWN:
         /// Unrecognised error code from GET4
         break;
      default:
         /// Corrupt error or not yet supported error
         break;
   } // switch( uErrorType )

   return;
}
void CbmMcbm2018MonitorAlgoT0::ProcessPattern( gdpbv100::FullMessage mess )
{
   uint16_t usType   = mess.getGdpbSysPattType();
   uint16_t usIndex  = mess.getGdpbSysPattIndex();
   uint32_t uPattern = mess.getGdpbSysPattPattern();

   switch( usType )
   {
      case gdpbv100::PATT_MISSMATCH:
      {
         LOG(DEBUG) << Form( "Missmatch pattern message => Type %d, Index %2d, Pattern 0x%08X", usType, usIndex, uPattern )
                   << FairLogger::endl;

         break;
      } // case gdpbv100::PATT_MISSMATCH:
      case gdpbv100::PATT_ENABLE:
      {
         LOG(DEBUG2) << Form( "Enable pattern message => Type %d, Index %2d, Pattern 0x%08X", usType, usIndex, uPattern )
                   << FairLogger::endl;

         break;
      } // case gdpbv100::PATT_ENABLE:
      case gdpbv100::PATT_RESYNC:
      {
         LOG(DEBUG) << Form( "RESYNC pattern message => Type %d, Index %2d, Pattern 0x%08X", usType, usIndex, uPattern )
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
*/
Bool_t CbmMcbm2018MonitorAlgoT0::CreateHistograms()
{
   std::string sFolder = "MoniT0";

   LOG(INFO) << "create Histos for T0 monitoring "
	          << FairLogger::endl;

   /*******************************************************************/
   fhChannelMap = new TH1I( "hChannelMap", "Map of hits on T0 detector; Strip; Hits Count []",
                           kuNbChanDiamond, 0., kuNbChanDiamond);
   fhHitMapEvo = new TH2I( "hHitMapEvo", "Map of hits on T0 detector vs time in run; Time in run [s]; Strip; Hits Count []",
                           fuHistoryHistoSize, 0, fuHistoryHistoSize,
                           kuNbChanDiamond, 0., kuNbChanDiamond );
   fhMsgCntEvo = new TH1I( "hMsgCntEvo", "Evolution of Hit & error msgs counts vs time in run; Time in run [s]; Msgs Count []",
                        fuHistoryHistoSize, 0, fuHistoryHistoSize );
   fhHitCntEvo = new TH1I( "hHitCntEvo", "Evolution of Hit counts vs time in run; Time in run [s]; Hits Count []",
                        fuHistoryHistoSize, 0, fuHistoryHistoSize );
   fhErrorCntEvo = new TH1I( "hErrorCntEvo", "Evolution of Error counts vs time in run; Time in run [s]; Error Count []",
                     fuHistoryHistoSize, 0, fuHistoryHistoSize );
   fhLostEvtCntEvo = new TH1I( "hLostEvtCntEvo", "Evolution of LostEvent counts vs time in run; Time in run [s]; LostEvent Count []",
                     fuHistoryHistoSize, 0, fuHistoryHistoSize );

   fhErrorFractEvo = new TProfile( "hErrorFractEvo",
         "Evolution of Error Fraction vs time in run; Time in run [s]; Error Fract []",
         fuHistoryHistoSize, 0, fuHistoryHistoSize );
   fhLostEvtFractEvo = new TProfile( "hLostEvtFractEvo",
         "Evolution of LostEvent Fraction vs time in run; Time in run [s]; LostEvent Fract []",
         fuHistoryHistoSize, 0, fuHistoryHistoSize );

   fhMsgCntPerMsEvo = new TH2I( "hMsgCntPerMsEvo",
         "Evolution of Hit & error msgs counts, per MS vs time in run; Time in run [s]; Hits Count/MS []; MS",
         fuHistoryHistoSize, 0, fuHistoryHistoSize,
         2000, 0, 2000  );
   fhHitCntPerMsEvo = new TH2I( "hHitCntPerMsEvo",
         "Evolution of Hit counts, per MS vs time in run; Time in run [s]; Hits Count/MS []; MS",
         fuHistoryHistoSize, 0, fuHistoryHistoSize,
         1500, 0, 1500  );
   fhErrorCntPerMsEvo = new TH2I( "hErrorCntPerMsEvo",
         "Evolution of Error counts, per MS vs time in run; Time in run [s]; Error Count/MS []; MS",
         fuHistoryHistoSize, 0, fuHistoryHistoSize,
         1500, 0, 1500  );
   fhLostEvtCntPerMsEvo = new TH2I( "hLostEvtCntPerMsEvo",
         "Evolution of LostEvent, per MS counts vs time in run; Time in run [s]; LostEvent Count/MS []; MS",
         fuHistoryHistoSize, 0, fuHistoryHistoSize,
         1000, 0, 1000  );

   fhErrorFractPerMsEvo = new TH2I(
         "hErrorFractPerMsEvo",
         "Evolution of Error Fraction, per MS vs time in run; Time in run [s]; Error Fract/MS []; MS",
         fuHistoryHistoSize, 0, fuHistoryHistoSize,
         1000, 0, 1 );
   fhLostEvtFractPerMsEvo = new TH2I(
         "hLostEvtFractPerMsEvo",
         "Evolution of LostEvent Fraction, per MS vs time in run; Time in run [s]; LostEvent Fract/MS []; MS",
         fuHistoryHistoSize, 0, fuHistoryHistoSize,
         1000, 0, 1 );

   /// Add pointers to the vector with all histo for access by steering class
   AddHistoToVector( fhChannelMap, sFolder );
   AddHistoToVector( fhHitMapEvo, sFolder );

   AddHistoToVector( fhMsgCntEvo, sFolder );
   AddHistoToVector( fhHitCntEvo, sFolder );
   AddHistoToVector( fhErrorCntEvo, sFolder );
   AddHistoToVector( fhLostEvtCntEvo, sFolder );

   AddHistoToVector( fhErrorFractEvo, sFolder );
   AddHistoToVector( fhLostEvtFractEvo, sFolder );

   AddHistoToVector( fhHitCntPerMsEvo, sFolder );
   AddHistoToVector( fhErrorCntPerMsEvo, sFolder );
   AddHistoToVector( fhLostEvtCntPerMsEvo, sFolder );
   AddHistoToVector( fhErrorFractPerMsEvo, sFolder );
   AddHistoToVector( fhLostEvtFractPerMsEvo, sFolder );

   /*******************************************************************/
   for( UInt_t uChan = 0; uChan < kuNbChanDiamond; ++uChan )
   {
      fvhHitCntEvoChan[ uChan ] = new TH1I(
            Form( "hHitCntEvoChan%02u", uChan ),
            Form( "Evolution of Hit counts vs time in run for channel %02u; Time in run [s]; Hits Count []", uChan ),
            fuHistoryHistoSize, 0, fuHistoryHistoSize );
      fvhHitCntPerMsEvoChan[ uChan ] = new TH2I(
            Form( "hHitCntPerMsEvoChan%02u", uChan ),
            Form( "Evolution of Hit counts per MS vs time in run for channel %02u; Time in run [s]; Hits Count/MS []; MS", uChan ),
            fuHistoryHistoSize, 0, fuHistoryHistoSize,
            1000, 0, 1000 );

      fvhErrorCntEvoChan[ uChan ] = new TH1I(
            Form( "hErrorCntEvoChan%02u", uChan ),
            Form( "Evolution of Error counts vs time in run for channel %02u; Time in run [s]; Error Count []", uChan ),
            fuHistoryHistoSize, 0, fuHistoryHistoSize );
      fvhErrorCntPerMsEvoChan[ uChan ] = new TH2I(
            Form( "hErrorCntPerMsEvoChan%02u", uChan ),
            Form( "Evolution of Error counts per MS vs time in run for channel %02u; Time in run [s]; Error Count/MS []; MS", uChan ),
            fuHistoryHistoSize, 0, fuHistoryHistoSize,
            1000, 0, 1000 );

      fvhEvtLostCntEvoChan[ uChan ] = new TH1I(
            Form( "hEvtLostCntEvoChan%02u", uChan ),
            Form( "Evolution of LostEvent counts vs time in run for channel %02u; Time in run [s]; LostEvent Count []", uChan ),
            fuHistoryHistoSize, 0, fuHistoryHistoSize );
      fvhEvtLostCntPerMsEvoChan[ uChan ] = new TH2I(
            Form( "hEvtLostCntPerMsEvoChan%02u", uChan ),
            Form( "Evolution of LostEvent counts per MS vs time in run for channel %02u; Time in run [s]; LostEvent Count/MS []; MS", uChan ),
            fuHistoryHistoSize, 0, fuHistoryHistoSize,
            1000, 0, 1000 );

      fvhErrorFractEvoChan[ uChan ] = new TProfile(
            Form( "hErrorFractEvoChan%02u", uChan ),
            Form( "Evolution of Error Fraction vs time in run for channel %02u; Time in run [s]; Error Fract []", uChan ),
            fuHistoryHistoSize, 0, fuHistoryHistoSize );
      fvhErrorFractPerMsEvoChan[ uChan ] = new TH2I(
            Form( "hErrorFractPerMsEvoChan%02u", uChan ),
            Form( "Evolution of Error Fraction, per MS vs time in run for channel %02u; Time in run [s]; Error Fract/MS []; MS", uChan ),
            fuHistoryHistoSize, 0, fuHistoryHistoSize,
            1000, 0, 1 );

      fvhEvtLostFractEvoChan[ uChan ] = new TProfile(
            Form( "hEvtLostFractEvoChan%02u", uChan ),
            Form( "Evolution of LostEvent Fraction vs time in run for channel %02u; Time in run [s]; LostEvent Fract []", uChan ),
            fuHistoryHistoSize, 0, fuHistoryHistoSize );
      fvhEvtLostFractPerMsEvoChan[ uChan ] = new TH2I(
            Form( "hEvtLostFractPerMsEvoChan%02u", uChan ),
            Form( "Evolution of LostEvent Fraction, per MS vs time in run for channel %02u; Time in run [s]; LostEvent Fract/MS []; MS", uChan ),
            fuHistoryHistoSize, 0, fuHistoryHistoSize,
            1000, 0, 1 );

      /// Add pointers to the vector with all histo for access by steering class
      AddHistoToVector( fvhHitCntEvoChan[ uChan ], sFolder );
      AddHistoToVector( fvhHitCntPerMsEvoChan[ uChan ], sFolder );
      AddHistoToVector( fvhErrorCntEvoChan[ uChan ], sFolder );
      AddHistoToVector( fvhErrorCntPerMsEvoChan[ uChan ], sFolder );
      AddHistoToVector( fvhEvtLostCntEvoChan[ uChan ], sFolder );
      AddHistoToVector( fvhEvtLostCntPerMsEvoChan[ uChan ], sFolder );
      AddHistoToVector( fvhErrorFractEvoChan[ uChan ], sFolder );
      AddHistoToVector( fvhErrorFractPerMsEvoChan[ uChan ], sFolder );
      AddHistoToVector( fvhEvtLostFractEvoChan[ uChan ], sFolder );
      AddHistoToVector( fvhEvtLostFractPerMsEvoChan[ uChan ], sFolder );
   } // for( UInt_t uChan = 0; uChan < kuNbChanDiamond; ++uChan )
   /*******************************************************************/

   /// Canvases
   Double_t w = 10;
   Double_t h = 10;

   /*******************************************************************/
   /// Map of hits over T0 detector and same vs time in run
   fcHitMaps = new TCanvas( "cHitMaps", "Hit maps", w, h);
   fcHitMaps->Divide( 2 );

   fcHitMaps->cd( 1 );
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogy();
   fhChannelMap->Draw();

   fcHitMaps->cd( 2 );
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogz();
   fhHitMapEvo->Draw( "colz" );

   AddCanvasToVector( fcHitMaps, "canvases" );
   /*******************************************************************/

   /*******************************************************************/
   /// General summary: Hit maps, Hit rate vs time in run, error fraction vs time un run
   fcSummary = new TCanvas( "cSummary", "Hit maps, Hit rate, Error fraction", w, h);
   fcSummary->Divide( 2, 2 );

   fcSummary->cd( 1 );
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogy();
   fhChannelMap->Draw();

   fcSummary->cd( 2 );
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogz();
   fhHitMapEvo->Draw( "colz" );

   fcSummary->cd( 3 );
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogy();
   fhHitCntEvo->Draw();

   fcSummary->cd( 4 );
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogz();
   fhErrorFractEvo->Draw( "hist" );

   AddCanvasToVector( fcSummary, "canvases" );
   /*******************************************************************/

   /*******************************************************************/
   /// General summary: Hit maps, Hit rate vs time in run, error fraction vs time un run
   fcGenCntsPerMs = new TCanvas( "cGenCntsPerMs", "Messages and hit cnt per MS, Error and Evt Loss Fract. per MS ", w, h);
   fcGenCntsPerMs->Divide( 2, 2 );

   fcGenCntsPerMs->cd( 1 );
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogy();
   gPad->SetLogz();
   fhMsgCntPerMsEvo->Draw( "colz" );

   fcGenCntsPerMs->cd( 2 );
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogy();
   gPad->SetLogz();
   fhHitCntPerMsEvo->Draw( "colz" );

   fcGenCntsPerMs->cd( 3 );
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogy();
   gPad->SetLogz();
   fhErrorFractPerMsEvo->Draw( "colz" );

   fcGenCntsPerMs->cd( 4 );
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogy();
   gPad->SetLogz();
   fhLostEvtFractPerMsEvo->Draw( "colz" );

   AddCanvasToVector( fcSummary, "canvases" );
   /*******************************************************************/

   return kTRUE;
}
Bool_t CbmMcbm2018MonitorAlgoT0::FillHistograms()
{
   Double_t dFractErrorsInMsChan;
   Double_t dFractLostEvtInMsChan;
   UInt_t   uCountHitsInMs = 0;
   UInt_t   uCountErrorsInMs = 0;
   UInt_t   uCountLostEvtInMs = 0;
   for( UInt_t uChan = 0; uChan < kuNbChanDiamond; ++uChan )
   {
      dFractErrorsInMsChan  = fvuErrorCntChanMs[ uChan ];
      dFractLostEvtInMsChan = fvuEvtLostCntChanMs[ uChan ];

      dFractErrorsInMsChan  /= (fvuHitCntChanMs[ uChan ] + fvuErrorCntChanMs[ uChan ]);
      dFractLostEvtInMsChan /= (fvuHitCntChanMs[ uChan ] + fvuErrorCntChanMs[ uChan ]);

      fvhHitCntPerMsEvoChan[       uChan ]->Fill( fdMsTime - fdStartTime, fvuHitCntChanMs[ uChan ] );
      fvhErrorCntPerMsEvoChan[     uChan ]->Fill( fdMsTime - fdStartTime, fvuErrorCntChanMs[ uChan ] );
      fvhEvtLostCntPerMsEvoChan[   uChan ]->Fill( fdMsTime - fdStartTime, fvuEvtLostCntChanMs[ uChan ] );
      fvhErrorFractPerMsEvoChan[   uChan ]->Fill( fdMsTime - fdStartTime, dFractErrorsInMsChan );
      fvhEvtLostFractPerMsEvoChan[ uChan ]->Fill( fdMsTime - fdStartTime, dFractLostEvtInMsChan );

      uCountHitsInMs    += fvuHitCntChanMs[ uChan ];
      uCountErrorsInMs  += fvuErrorCntChanMs[ uChan ];
      uCountLostEvtInMs += fvuEvtLostCntChanMs[ uChan ];

      fvuHitCntChanMs[ uChan ] = 0;
      fvuErrorCntChanMs[ uChan ] = 0;
      fvuEvtLostCntChanMs[ uChan ] = 0;
   } // for( UInt_t uChan = 0; uChan < kuNbChanDiamond; ++uChan )
   Double_t dFractErrorsInMs  = uCountErrorsInMs;
   Double_t dFractLostEvtInMs = uCountLostEvtInMs;
   dFractErrorsInMs  /= ( uCountHitsInMs + uCountErrorsInMs );
   dFractLostEvtInMs /= ( uCountHitsInMs + uCountErrorsInMs );

   fhMsgCntPerMsEvo->Fill(       fdMsTime - fdStartTime, uCountHitsInMs + uCountErrorsInMs );
   fhHitCntPerMsEvo->Fill(       fdMsTime - fdStartTime, uCountHitsInMs );
   fhErrorCntPerMsEvo->Fill(     fdMsTime - fdStartTime, uCountErrorsInMs );
   fhLostEvtCntPerMsEvo->Fill(   fdMsTime - fdStartTime, uCountLostEvtInMs );
   fhErrorFractPerMsEvo->Fill(   fdMsTime - fdStartTime, dFractErrorsInMs );
   fhLostEvtFractPerMsEvo->Fill( fdMsTime - fdStartTime, dFractLostEvtInMs );

   return kTRUE;
}
Bool_t CbmMcbm2018MonitorAlgoT0::ResetHistograms()
{
   for( UInt_t uChan = 0; uChan < kuNbChanDiamond; ++uChan )
   {
      fvhHitCntEvoChan[ uChan ]->Reset();
      fvhHitCntPerMsEvoChan[ uChan ]->Reset();

      fvhErrorCntEvoChan[ uChan ]->Reset();
      fvhErrorCntPerMsEvoChan[ uChan ]->Reset();

      fvhEvtLostCntEvoChan[ uChan ]->Reset();
      fvhEvtLostCntPerMsEvoChan[ uChan ]->Reset();

      fvhErrorFractEvoChan[ uChan ]->Reset();
      fvhErrorFractPerMsEvoChan[ uChan ]->Reset();

      fvhEvtLostFractEvoChan[ uChan ]->Reset();
      fvhEvtLostFractPerMsEvoChan[ uChan ]->Reset();
   } // for( UInt_t uChan = 0; uChan < kuNbChanDiamond; ++uChan )

   fhChannelMap->Reset();
   fhHitMapEvo->Reset();

   fhMsgCntEvo->Reset();
   fhHitCntEvo->Reset();
   fhErrorCntEvo->Reset();

   fhErrorFractEvo->Reset();
   fhLostEvtFractEvo->Reset();

   fhMsgCntPerMsEvo->Reset();
   fhHitCntPerMsEvo->Reset();
   fhErrorCntPerMsEvo->Reset();
   fhLostEvtCntPerMsEvo->Reset();
   fhErrorFractPerMsEvo->Reset();
   fhLostEvtFractPerMsEvo->Reset();

   /// Also reset the Start time for the evolution plots!
   fdStartTime = -1.0;

   return kTRUE;
}
// -------------------------------------------------------------------------
