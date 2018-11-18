// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                  CbmStar2019EventBuilderEtofAlgo                  -----
// -----               Created 03.11.2018 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmStar2019EventBuilderEtofAlgo.h"

#include "CbmStar2019TofPar.h"
#include "CbmHistManager.h"

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

#include <iostream>
#include <stdint.h>
#include <iomanip>
#include <fstream>

// -------------------------------------------------------------------------
CbmStar2019EventBuilderEtofAlgo::CbmStar2019EventBuilderEtofAlgo() :
   CbmStar2019Algo(),
   /// From the class itself
   fbMonitorMode( kFALSE ),
   fbDebugMonitorMode( kFALSE ),
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
   fuNrOfGbtx( 0 ),
   fuNrOfModules( 0 ),
   fviNrOfRpc(),
   fviRpcType(),
   fviRpcSide(),
   fviModuleId(),
   fdAllowedTriggersSpread( -1.0 ),
   fdStarTriggerDeadtime(),
   fdStarTriggerDelay(),
   fdStarTriggerWinSize(),
   fulCurrentTsIndex( 0 ),
   fdTsStartTime( -1.0 ),
   fuCurrentMs( 0 ),
   fdMsIndex( -1.0 ),
   fuGdpbId( 0 ),
   fuGdpbNr( 0 ),
   fuGet4Id( 0 ),
   fuGet4Nr( 0 ),
   fiEquipmentId( 0 ),
   fvulCurrentEpoch(),
   fvulCurrentEpochCycle(),
   fvulCurrentEpochFull(),
   fvvmEpSupprBuffer(),
   fvvBufferMajorAsicErrors(),
   fvvBufferMessages(),
   fvvBufferTriggers(),
   fvulGdpbTsMsb(),
   fvulGdpbTsLsb(),
   fvulStarTsMsb(),
   fvulStarTsMid(),
   fvulGdpbTsFullLast(),
   fvulStarTsFullLast(),
   fvuStarTokenLast(),
   fvuStarDaqCmdLast(),
   fvuStarTrigCmdLast(),
   fvulGdpbTsFullLastCore(),
   fvulStarTsFullLastCore(),
   fvuStarTokenLastCore(),
   fvuStarDaqCmdLastCore(),
   fvuStarTrigCmdLastCore(),
   fvdMessCandidateTimeStart(),
   fvdMessCandidateTimeStop(),
   fvdTrigCandidateTimeStart(),
   fvhHitsTimeToTriggerRaw(),
   fvhHitsTimeToTriggerSel(),
   fvhHitsTimeToTriggerSelVsDaq(),
   fvhHitsTimeToTriggerSelVsTrig(),
   fvhTriggerDt(),
   fhEventSizeDistribution( nullptr ),
   fhEventSizeEvolution( nullptr ),
   fhEventNbEvolution( nullptr ),
   fhEventNbDistributionInTs( nullptr ),
   fhEventSizeDistributionInTs( nullptr ),
   fhRawTriggersStats( nullptr ),
   fhMissingTriggersEvolution( nullptr )
{
}
CbmStar2019EventBuilderEtofAlgo::~CbmStar2019EventBuilderEtofAlgo()
{
   /// Clear buffers
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      fvvmEpSupprBuffer[ uGdpb ].clear();
      fvvBufferMajorAsicErrors[ uGdpb ].clear();
      fvvBufferMessages[ uGdpb ].clear();
      fvvBufferTriggers[ uGdpb ].clear();
   } // for (Int_t iGdpb = 0; iGdpb < fuNrOfGdpbs; ++iGdpb)
}

// -------------------------------------------------------------------------
Bool_t CbmStar2019EventBuilderEtofAlgo::Init()
{
   LOG(INFO) << "Initializing STAR eTOF 2019 event builder algo" << FairLogger::endl;

   return kTRUE;
}
void CbmStar2019EventBuilderEtofAlgo::Reset()
{
}
void CbmStar2019EventBuilderEtofAlgo::Finish()
{
   /// Printout Goodbye message and stats

   /// Write Output histos

}

// -------------------------------------------------------------------------
Bool_t CbmStar2019EventBuilderEtofAlgo::InitContainers()
{
   LOG(INFO) << "Init parameter containers for CbmStar2019EventBuilderEtofAlgo"
             << FairLogger::endl;
   Bool_t initOK = ReInitContainers();

   return initOK;
}
Bool_t CbmStar2019EventBuilderEtofAlgo::ReInitContainers()
{
   LOG(INFO) << "**********************************************"
             << FairLogger::endl;
   LOG(INFO) << "ReInit parameter containers for CbmStar2019EventBuilderEtofAlgo"
             << FairLogger::endl;

   fUnpackPar = (CbmStar2019TofPar*)fParCList->FindObject("CbmStar2019TofPar");
   if( nullptr == fUnpackPar )
      return kFALSE;

   Bool_t initOK = InitParameters();

   return initOK;
}
TList* CbmStar2019EventBuilderEtofAlgo::GetParList()
{
   if( nullptr == fParCList )
      fParCList = new TList();
   fUnpackPar = new CbmStar2019TofPar("CbmStar2019TofPar");
   fParCList->Add(fUnpackPar);

   return fParCList;
}
Bool_t CbmStar2019EventBuilderEtofAlgo::InitParameters()
{
   /// Control flags
   fbMonitorMode = fUnpackPar->GetMonitorMode();
   LOG(INFO) << "Monitor mode:       "
             << ( fbMonitorMode ? "ON" : "OFF" )
             << FairLogger::endl;

   fbDebugMonitorMode = fUnpackPar->GetDebugMonitorMode();
   LOG(INFO) << "Debug Monitor mode: "
             << ( fbDebugMonitorMode ? "ON" : "OFF" )
             << FairLogger::endl;

   /// Parameters for variables dimensionning
   fuNrOfGdpbs = fUnpackPar->GetNrOfGdpbs();
   LOG(INFO) << "Nr. of Tof GDPBs: " << fuNrOfGdpbs << FairLogger::endl;

   fuNrOfFeePerGdpb = fUnpackPar->GetNrOfFeePerGdpb();
   LOG(INFO) << "Nr. of FEEs per Tof GDPB: " << fuNrOfFeePerGdpb
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
      LOG(INFO) << "GDPB Id of TOF  " << Form( "%02d", i) << " : "
                 << std::hex << Form( "0x%04x", fUnpackPar->GetGdpbId(i) )
                 << std::dec << FairLogger::endl;
   } // for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )

   fuNrOfGbtx  =  fUnpackPar->GetNrOfGbtx();
   LOG(INFO) << "Nr. of GBTx: " << fuNrOfGbtx << FairLogger::endl;

   fuNrOfModules  =  fUnpackPar->GetNrOfModules();
   LOG(INFO) << "Nr. of GBTx: " << fuNrOfModules << FairLogger::endl;

   /// Parameters for detector mapping
   fviRpcType.resize(  fuNrOfGbtx );
   fviModuleId.resize( fuNrOfGbtx );
   fviNrOfRpc.resize(  fuNrOfGbtx );
   fviRpcSide.resize(  fuNrOfGbtx );
   for( UInt_t uGbtx = 0; uGbtx < fuNrOfGbtx; ++uGbtx)
   {
      fviNrOfRpc[ uGbtx ]  = fUnpackPar->GetNrOfRpc( uGbtx );
      fviRpcType[ uGbtx ]  = fUnpackPar->GetRpcType( uGbtx );
      fviRpcSide[ uGbtx ]  = fUnpackPar->GetRpcSide( uGbtx );
      fviModuleId[ uGbtx ] = fUnpackPar->GetModuleId( uGbtx );
   } // for( UInt_t uGbtx = 0; uGbtx < uNrOfGbtx; ++uGbtx)

   LOG(INFO) << "Nr. of RPCs per GBTx: ";
   for( UInt_t uGbtx = 0; uGbtx < fuNrOfGbtx; ++uGbtx)
      LOG(INFO) << Form(" %2d", fviNrOfRpc[ uGbtx ] );
   LOG(INFO) << FairLogger::endl;

   LOG(INFO) << "RPC type per GBTx:    ";
   for( UInt_t uGbtx = 0; uGbtx < fuNrOfGbtx; ++uGbtx)
      LOG(INFO) << Form(" %2d", fviRpcType[ uGbtx ] );
   LOG(INFO) << FairLogger::endl;

   LOG(INFO) << "RPC side per GBTx:    ";
   for( UInt_t uGbtx = 0; uGbtx < fuNrOfGbtx; ++uGbtx)
      LOG(INFO) << Form(" %2d", fviRpcSide[ uGbtx ] );
   LOG(INFO) << FairLogger::endl;

   LOG(INFO) << "Module ID per GBTx:   ";
   for( UInt_t uGbtx = 0; uGbtx < fuNrOfGbtx; ++uGbtx)
      LOG(INFO) << Form(" %2d", fviModuleId[ uGbtx ] );
   LOG(INFO) << FairLogger::endl;

   /// Parameters for FLES containers processing
   fdMsSizeInNs  = fUnpackPar->GetSizeMsInNs();
   LOG(INFO) << "Timeslice parameters: each MS is "
             << fdMsSizeInNs << " ns"
             << FairLogger::endl;

   /// Parameters for STAR trigger & event building
   fdAllowedTriggersSpread = fUnpackPar->GetStarTriggAllowedSpread();
   fdStarTriggerDeadtime.resize( fuNrOfGdpbs, 0.0 );
   fdStarTriggerDelay.resize( fuNrOfGdpbs, 0.0 );
   fdStarTriggerWinSize.resize( fuNrOfGdpbs, 0.0 );
   fvdMessCandidateTimeStart.resize( fuNrOfGdpbs, 0.0 );
   fvdMessCandidateTimeStop.resize( fuNrOfGdpbs, 0.0 );
   fvdTrigCandidateTimeStart.resize( fuNrOfGdpbs, 0.0 );
   fvdTrigCandidateTimeStop.resize( fuNrOfGdpbs, 0.0 );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      fdStarTriggerDeadtime[ uGdpb ] = fUnpackPar->GetStarTriggDeadtime( uGdpb );
      fdStarTriggerDelay[ uGdpb ]    = fUnpackPar->GetStarTriggDelay( uGdpb );
      fdStarTriggerWinSize[ uGdpb ]  = fUnpackPar->GetStarTriggWinSize( uGdpb );
      LOG(INFO) << Form( "Trigger window parameters for gDPB %2u are: ", uGdpb)
                << fdStarTriggerDeadtime[ uGdpb ] << " ns deadtime, "
                << fdStarTriggerDelay[ uGdpb ] << " ns delay, "
                << fdStarTriggerWinSize[ uGdpb ] << " ns window width"
                << FairLogger::endl;
   } // for (Int_t iGdpb = 0; iGdpb < fuNrOfGdpbs; ++iGdpb)

   ///
   fvulCurrentEpoch.resize( fuNrOfGdpbs, 0 );
   fvulCurrentEpochCycle.resize( fuNrOfGdpbs, 0 );
   fvulCurrentEpochFull.resize( fuNrOfGdpbs, 0 );

   ///
   fvvmEpSupprBuffer.resize( fuNrOfGdpbs );
   fvvBufferMajorAsicErrors.resize( fuNrOfGdpbs );
   fvvBufferMessages.resize( fuNrOfGdpbs );
   fvvBufferTriggers.resize( fuNrOfGdpbs );

   /// STAR Trigger decoding and monitoring
   fvulGdpbTsMsb.resize(  fuNrOfGdpbs, 0 );
   fvulGdpbTsLsb.resize(  fuNrOfGdpbs, 0 );
   fvulStarTsMsb.resize(  fuNrOfGdpbs, 0 );
   fvulStarTsMid.resize(  fuNrOfGdpbs, 0 );
   fvulGdpbTsFullLast.resize(  fuNrOfGdpbs, 0 );
   fvulStarTsFullLast.resize(  fuNrOfGdpbs, 0 );
   fvuStarTokenLast.resize(    fuNrOfGdpbs, 0 );
   fvuStarDaqCmdLast.resize(   fuNrOfGdpbs, 0 );
   fvuStarTrigCmdLast.resize(  fuNrOfGdpbs, 0 );
   fvulGdpbTsFullLastCore.resize(  fuNrOfGdpbs, 0 );
   fvulStarTsFullLastCore.resize(  fuNrOfGdpbs, 0 );
   fvuStarTokenLastCore.resize(    fuNrOfGdpbs, 0 );
   fvuStarDaqCmdLastCore.resize(   fuNrOfGdpbs, 0 );
   fvuStarTrigCmdLastCore.resize(  fuNrOfGdpbs, 0 );

	return kTRUE;
}
// -------------------------------------------------------------------------

void CbmStar2019EventBuilderEtofAlgo::AddMsComponentToList( size_t component, UShort_t usDetectorId )
{
   /// Check for duplicates and ignore if it is the case
   for( UInt_t uCompIdx = 0; uCompIdx < fvMsComponentsList.size(); ++uCompIdx )
      if( component == fvMsComponentsList[ uCompIdx ] )
         return;

   /// Add to list
   fvMsComponentsList.push_back( component );

   LOG(INFO) << "CbmStar2019EventBuilderEtofAlgo::AddMsComponentToList => Component "
             << component << " with detector ID 0x"
             << std::hex << usDetectorId << std::dec << " added to list"
             << FairLogger::endl;

}
// -------------------------------------------------------------------------

Bool_t CbmStar2019EventBuilderEtofAlgo::ProcessTs( const fles::Timeslice& ts )
{
   fulCurrentTsIndex = ts.index();
   fdTsStartTime = static_cast< Double_t >( ts.descriptor( 0, 0 ).idx );

   /// On first TS, extract the TS parameters from header (by definition stable over time)
   if( -1.0 == fdTsCoreSizeInNs )
   {
      fuNbCoreMsPerTs = ts.num_core_microslices();
      fuNbOverMsPerTs = ts.num_microslices( 0 ) - ts.num_core_microslices();
      fdTsCoreSizeInNs = fdMsSizeInNs * fuNbCoreMsPerTs;
      fdTsFullSizeInNs = fdMsSizeInNs * (fuNbCoreMsPerTs + fuNbOverMsPerTs);
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
   Double_t dTsStopTimeCore = fdTsStartTime + fdTsCoreSizeInNs;

   /// Compute the limits for accepting hits and trigger in this TS, for each gDPB/sector
   /// => Avoid special cases and buffering for the overlap MS
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      if( fdStarTriggerDelay[ uGdpb ] < 0.0 )
      {
         /// Event window for this gDPB starts before the trigger
         if( fdStarTriggerDelay[ uGdpb ] + fdStarTriggerWinSize[ uGdpb ] < 0.0 )
         {
            /// Event window for this gDPB is fully before the trigger
            fvdMessCandidateTimeStart[ uGdpb ] = fdTsStartTime;
            // << Accept more than needed as this should be safer and small amounts >>
            fvdMessCandidateTimeStop[ uGdpb ]  = dTsStopTimeCore + fdAllowedTriggersSpread + 2.0 * fdStarTriggerWinSize[ uGdpb ]; // + fdStarTriggerWinSize[ uGdpb ];
            fvdTrigCandidateTimeStart[ uGdpb ] = fdTsStartTime   + fdAllowedTriggersSpread - fdStarTriggerDelay[ uGdpb ];
            fvdTrigCandidateTimeStop[ uGdpb ]  = dTsStopTimeCore + fdAllowedTriggersSpread - fdStarTriggerDelay[ uGdpb ];

         } // if( fdStarTriggerDelay[ uGdpb ] + fdStarTriggerWinSize[ uGdpb ] < 0.0 )
            else
            {
               /// Event window for this gDPB is on both sides of the trigger
               fvdMessCandidateTimeStart[ uGdpb ] = fdTsStartTime;
               // << Accept more than needed as this should be safer and small amounts >>
               fvdMessCandidateTimeStop[ uGdpb ]  = dTsStopTimeCore + fdAllowedTriggersSpread + 2.0 * fdStarTriggerWinSize[ uGdpb ]; // + fdStarTriggerDelay[ uGdpb ];
               fvdTrigCandidateTimeStart[ uGdpb ] = fdTsStartTime   + fdAllowedTriggersSpread - fdStarTriggerDelay[ uGdpb ];
               fvdTrigCandidateTimeStop[ uGdpb ]  = dTsStopTimeCore + fdAllowedTriggersSpread - fdStarTriggerDelay[ uGdpb ];
            } // else of if( fdStarTriggerDelay[ uGdpb ] + fdStarTriggerWinSize[ uGdpb ] < 0.0 )
      } // if( fdStarTriggerDeadtime[ uGdpb ] < 0.0 )
         else
         {
            /// Event window for this gDPB starts after the trigger => fully after
             // << Accept more than needed as this should be safer and small amounts >>
            fvdMessCandidateTimeStart[ uGdpb ] = fdTsStartTime; // - fdAllowedTriggersSpread + fdStarTriggerDelay[ uGdpb ];
            fvdMessCandidateTimeStop[ uGdpb ]  = dTsStopTimeCore + fdAllowedTriggersSpread + fdStarTriggerDelay[ uGdpb ] + fdStarTriggerWinSize[ uGdpb ];
            fvdTrigCandidateTimeStart[ uGdpb ] = fdTsStartTime   + fdAllowedTriggersSpread;
            fvdTrigCandidateTimeStop[ uGdpb ]  = dTsStopTimeCore + fdAllowedTriggersSpread;
         } // else of if( fdStarTriggerDelay[ uGdpb ] < 0.0 )
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

   /// Loop over registered components
   for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsList.size(); ++uMsCompIdx )
   {
      UInt_t uMsComp = fvMsComponentsList[ uMsCompIdx ];

      /// Loop over core microslices (and overlap ones if chosen)
      for( UInt_t uMsIdx = 0; uMsIdx < fuNbMsLoop; uMsIdx ++ )
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

   /// Clear event buffer from previous TS
   /// ... Hopefully whatever had to be done with it was done before calling Process TS again ^^'
   fvEventsBuffer.clear();

   /// Build events from all triggers and data found in this TS core MS + part of the overlap MS
   if( kFALSE == BuildEvents() )
   {
      LOG(ERROR) << "Failed to build events in ts " << fulCurrentTsIndex
                 << FairLogger::endl;
      return kFALSE;
   } // if( kFALSE == BuildEvents() )

   /// Clear buffers to prepare for the next TS
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      fvvmEpSupprBuffer[ uGdpb ].clear();
      fvvBufferMajorAsicErrors[ uGdpb ].clear();
      fvvBufferMessages[ uGdpb ].clear();
      fvvBufferTriggers[ uGdpb ].clear();
   } // for (Int_t iGdpb = 0; iGdpb < fuNrOfGdpbs; ++iGdpb)

   /// Fill plots if in monitor mode
   if( fbMonitorMode )
   {
      if( kFALSE == FillHistograms() )
      {
         LOG(ERROR) << "Failed to fill histos in ts " << fulCurrentTsIndex
                    << FairLogger::endl;
         return kFALSE;
      } // if( kFALSE == FillHistograms() )
   } // if( fbMonitorMode )

   return kTRUE;
}

Bool_t CbmStar2019EventBuilderEtofAlgo::ProcessMs( const fles::Timeslice& ts, size_t uMsCompIdx, size_t uMsIdx )
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

/*
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
*/

   /// If not integer number of message in input buffer, print warning/error
   if (0 != (size % fUnpackPar->GetNbByteMessage()))
      LOG(ERROR) << "The input microslice buffer does NOT "
                 << "contain only complete nDPB messages!"
                 << FairLogger::endl;

   /// Compute the number of complete messages in the input microslice buffer
   uint32_t uNbMessages = ( size - ( size % fUnpackPar->GetNbByteMessage() ) )
                          / fUnpackPar->GetNbByteMessage();

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
      return kFALSE;
   } // if( it == fGdpbIdIndexMap.end() )
      else fuGdpbNr = fGdpbIdIndexMap[ fuGdpbId ];

   /// Store the last STAR trigger values for the core MS when reaching the first overlap MS, needed to check for clones
   if( fuNbCoreMsPerTs == fuCurrentMs )
   {
      fvulGdpbTsFullLastCore[ fuGdpbNr ] = fvulGdpbTsFullLast[ fuGdpbNr ];
      fvulStarTsFullLastCore[ fuGdpbNr ] = fvulStarTsFullLast[ fuGdpbNr ];
      fvuStarTokenLastCore[ fuGdpbNr ]   = fvuStarTokenLast[ fuGdpbNr ];
      fvuStarDaqCmdLastCore[ fuGdpbNr ]  = fvuStarDaqCmdLast[ fuGdpbNr ];
      fvuStarTrigCmdLastCore[ fuGdpbNr ] = fvuStarTrigCmdLast[ fuGdpbNr ];
   } // if( fuNbCoreMsPerTs == fuCurrentMs )

   /// Restore the last STAR trigger values for the core MS when reaching the first core MS, needed to check for clones
   if( 0 == fuCurrentMs )
   {
      fvulGdpbTsFullLast[ fuGdpbNr ] = fvulGdpbTsFullLastCore[ fuGdpbNr ];
      fvulStarTsFullLast[ fuGdpbNr ] = fvulStarTsFullLastCore[ fuGdpbNr ];
      fvuStarTokenLast[ fuGdpbNr ]   = fvuStarTokenLastCore[ fuGdpbNr ];
      fvuStarDaqCmdLast[ fuGdpbNr ]  = fvuStarDaqCmdLastCore[ fuGdpbNr ];
      fvuStarTrigCmdLast[ fuGdpbNr ] = fvuStarTrigCmdLastCore[ fuGdpbNr ];
   } // if( 0 == fuCurrentMs )

   // Prepare variables for the loop on contents
   Int_t messageType = -111;
   const uint64_t* pInBuff = reinterpret_cast<const uint64_t*>(msContent);
   for( uint32_t uIdx = 0; uIdx < uNbMessages; uIdx++ )
   {
      // Fill message
      uint64_t ulData = static_cast<uint64_t>(pInBuff[uIdx]);

      /// Catch the Epoch cycle block which is always the first 64b of the MS
      if( 0 == uIdx )
      {
//         std::cout << Form( "gDPB %2d", fuGdpbNr) << " Epoch cycle " << Form( "0x%012lx", ulData ) << std::endl;
         ProcessEpochCycle( ulData );
//         continue;
      } // if( 0 == uIdx )

      gdpbv100::Message mess(ulData);
/*
         mess.printDataCout();
         continue;
*/
      /// Get message type
      messageType = mess.getMessageType();

      fuGet4Id = fUnpackPar->ElinkIdxToGet4Idx( mess.getGdpbGenChipId() );
      fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + fuGet4Id;

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
               LOG(FATAL) << "This event builder does not support 24b hit message!!!."
                          << FairLogger::endl;
            } // if( getGdpbHitIs24b() )
               else
               {
                  fvvmEpSupprBuffer[fuGdpbNr].push_back( mess );
               } // else of if( getGdpbHitIs24b() )
            break;
         } // case gdpbv100::MSG_HIT:
         case gdpbv100::MSG_EPOCH:
         {
            if( gdpbv100::kuChipIdMergedEpoch == fuGet4Id )
            {
               ProcessEpoch(mess);
/*
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
*/
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
            fvvmEpSupprBuffer[fuGdpbNr].push_back( mess );
            break;
         } // case gdpbv100::MSG_SLOWC:
         case gdpbv100::MSG_SYST:
         {
            fvvmEpSupprBuffer[fuGdpbNr].push_back( mess );
            break;
         } // case gdpbv100::MSG_SYST:
         case gdpbv100::MSG_STAR_TRI_A:
         case gdpbv100::MSG_STAR_TRI_B:
         case gdpbv100::MSG_STAR_TRI_C:
         case gdpbv100::MSG_STAR_TRI_D:
         {
            ProcessStarTrigger( mess );

            /// If A message, check that the following ones are B, C, D
            /// ==> TBD only if necessary
/*
            if( gdpbv100::MSG_STAR_TRI_A == messageType )
            {
            } // if( gdpbv100::MSG_STAR_TRI_A == messageType )
*/
            break;
         } // case gdpbv100::MSG_STAR_TRI_A-D
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
void CbmStar2019EventBuilderEtofAlgo::ProcessEpochCycle( uint64_t ulCycleData )
{
   ULong64_t ulEpochCycleVal = ulCycleData & gdpbv100::kulEpochCycleFieldSz;
/*
   if( fuRawDataPrintMsgIdx < fuRawDataPrintMsgNb || gLogger->IsLogNeeded(DEBUG2) )
   {
      LOG(INFO) << "CbmMcbm2018MonitorTof::ProcessEpochCyle => "
                 << Form( " TS %5d MS %3d In data 0x%016X Cycle 0x%016X",
                           fulCurrentTsIndex, fuCurrentMs, ulCycleData, ulEpochCycleVal )
                 << FairLogger::endl;
      fuRawDataPrintMsgIdx ++;
   } // if( fuRawDataPrintMsgIdx < fuRawDataPrintMsgNb || gLogger->IsLogNeeded(DEBUG2) )
*/
   if( !( ulEpochCycleVal == fvulCurrentEpochCycle[fuGdpbNr] ||
          ulEpochCycleVal == fvulCurrentEpochCycle[fuGdpbNr] + 1 ) )
      LOG(FATAL) << "CbmStar2019EventBuilderEtofAlgo::ProcessEpochCycle => "
                 << " Missmatch in epoch cycles detected, probably fake cycles due to epoch index corruption! "
                 << Form( " Current cycle 0x%09llX New cycle 0x%09llX", fvulCurrentEpochCycle[fuGdpbNr], ulEpochCycleVal )
                 << FairLogger::endl;

   fvulCurrentEpochCycle[fuGdpbNr] = ulEpochCycleVal;

   return;
}

void CbmStar2019EventBuilderEtofAlgo::ProcessEpoch( gdpbv100::Message mess )
{
   ULong64_t ulEpochNr = mess.getGdpbEpEpochNb();
/*
 * /// FIXME: Need proper handling of overlap MS
   if( 0 < fvulCurrentEpoch[ fuGdpbNr ] && ulEpochNr < fvulCurrentEpoch[ fuGdpbNr ] )
   {
      std::cout << Form( "gDPB %2d", fuGdpbNr) << " New Epoch cycle "
                << Form( "0x%012llx old Ep %08llx new Ep %08llx", fvulCurrentEpochCycle[ fuGdpbNr ], fvulCurrentEpoch[ fuGdpbNr ], ulEpochNr )
                << std::endl;
      fvulCurrentEpochCycle[ fuGdpbNr ]++;
   } // if( 0 < fvulCurrentEpoch[ fuGdpbNr ] && ulEpochNr < fvulCurrentEpoch[ fuGdpbNr ] )
*/
   fvulCurrentEpoch[ fuGdpbNr ] = ulEpochNr;
   fvulCurrentEpochFull[ fuGdpbNr ] = ulEpochNr + gdpbv100::kulEpochCycleBins * fvulCurrentEpochCycle[ fuGdpbNr ];

//   fulCurrentEpochTime = mess.getMsgFullTime(ulEpochNr);

   /// Re-align the epoch number of the message in case it will be used later:
   /// We received the epoch after the data instead of the one before!
   if( 0 < ulEpochNr )
      mess.setGdpbEpEpochNb( ulEpochNr - 1 );
      else mess.setGdpbEpEpochNb( gdpbv100::kuEpochCounterSz );

   /// Process the corresponding messages buffer
   ProcessEpSupprBuffer( fuGdpbNr );
}

void CbmStar2019EventBuilderEtofAlgo::ProcessStarTrigger( gdpbv100::Message mess )
{
   Int_t iMsgIndex = mess.getStarTrigMsgIndex();

   switch( iMsgIndex )
   {
      case 0:
         fvulGdpbTsMsb[fuGdpbNr] = mess.getGdpbTsMsbStarA();
         if( fbMonitorMode && fbDebugMonitorMode )
            fhRawTriggersStats->Fill( 0., fUnpackPar->GetGdpbToSectorOffset() + fuGdpbNr );
         break;
      case 1:
         fvulGdpbTsLsb[fuGdpbNr] = mess.getGdpbTsLsbStarB();
         fvulStarTsMsb[fuGdpbNr] = mess.getStarTsMsbStarB();
         if( fbMonitorMode && fbDebugMonitorMode )
            fhRawTriggersStats->Fill( 1., fUnpackPar->GetGdpbToSectorOffset() + fuGdpbNr );
         break;
      case 2:
         fvulStarTsMid[fuGdpbNr] = mess.getStarTsMidStarC();
         if( fbMonitorMode && fbDebugMonitorMode )
            fhRawTriggersStats->Fill( 2., fUnpackPar->GetGdpbToSectorOffset() + fuGdpbNr );
         break;
      case 3:
      {
         ULong64_t ulNewGdpbTsFull = ( fvulGdpbTsMsb[fuGdpbNr] << 24 )
                                   + ( fvulGdpbTsLsb[fuGdpbNr]       );
         ULong64_t ulNewStarTsFull = ( fvulStarTsMsb[fuGdpbNr] << 48 )
                                   + ( fvulStarTsMid[fuGdpbNr] <<  8 )
                                   + mess.getStarTsLsbStarD();
         UInt_t uNewToken  = mess.getStarTokenStarD();
         UInt_t uNewDaqCmd  = mess.getStarDaqCmdStarD();
         UInt_t uNewTrigCmd = mess.getStarTrigCmdStarD();

         if( fbMonitorMode && fbDebugMonitorMode )
            fhRawTriggersStats->Fill( 3., fUnpackPar->GetGdpbToSectorOffset() + fuGdpbNr );

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

         if( ( uNewToken == fvuStarTokenLast[fuGdpbNr] ) && ( ulNewGdpbTsFull == fvulGdpbTsFullLast[fuGdpbNr] ) &&
             ( ulNewStarTsFull == fvulStarTsFullLast[fuGdpbNr] ) && ( uNewDaqCmd == fvuStarDaqCmdLast[fuGdpbNr] ) &&
             ( uNewTrigCmd == fvuStarTrigCmdLast[fuGdpbNr] ) )
         {
            UInt_t uTrigWord =  ( (fvuStarTrigCmdLast[fuGdpbNr] & 0x00F) << 16 )
                     + ( (fvuStarDaqCmdLast[fuGdpbNr]   & 0x00F) << 12 )
                     + ( (fvuStarTokenLast[fuGdpbNr]    & 0xFFF)       );
            LOG(WARNING) << "Possible error: identical STAR tokens found twice in a row => ignore 2nd! "
                         << " TS " << fulCurrentTsIndex
                         << " gDBB #" << fuGdpbNr << " "
                         << Form("token = %5u ", fvuStarTokenLast[fuGdpbNr] )
                         << Form("gDPB ts  = %12llu ", fvulGdpbTsFullLast[fuGdpbNr] )
                         << Form("STAR ts = %12llu ", fvulStarTsFullLast[fuGdpbNr] )
                         << Form("DAQ cmd = %2u ", fvuStarDaqCmdLast[fuGdpbNr] )
                         << Form("TRG cmd = %2u ", fvuStarTrigCmdLast[fuGdpbNr] )
                         << Form("TRG Wrd = %5x ", uTrigWord )
                         << FairLogger::endl;
            return;
         } // if exactly same message repeated

         // GDPB TS counter reset detection
         if( ulNewGdpbTsFull < fvulGdpbTsFullLast[fuGdpbNr] )
            LOG(DEBUG) << "Probable reset of the GDPB TS: old = " << Form("%16llu", fvulGdpbTsFullLast[fuGdpbNr])
                       << " new = " << Form("%16llu", ulNewGdpbTsFull)
                       << " Diff = -" << Form("%8llu", fvulGdpbTsFullLast[fuGdpbNr] - ulNewGdpbTsFull)
                       << " GDPB #" << Form( "%2u", fuGdpbNr)
                       << FairLogger::endl;

         // STAR TS counter reset detection
         if( ulNewStarTsFull < fvulStarTsFullLast[fuGdpbNr] )
            LOG(DEBUG) << "Probable reset of the STAR TS: old = " << Form("%16llu", fvulStarTsFullLast[fuGdpbNr])
                       << " new = " << Form("%16llu", ulNewStarTsFull)
                       << " Diff = -" << Form("%8llu", fvulStarTsFullLast[fuGdpbNr] - ulNewStarTsFull)
                       << " GDPB #" << Form( "%2u", fuGdpbNr)
                       << FairLogger::endl;

/*
         LOG(INFO) << "Updating  trigger token for " << fuGdpbNr
                   << " " << fvuStarTokenLast[fuGdpbNr] << " " << uNewToken
                   << FairLogger::endl;
*/

         /// Generate Trigger object and store it for event building ///
         CbmTofStarTrigger2019 newTrig( fvulGdpbTsFullLast[fuGdpbNr], fvulStarTsFullLast[fuGdpbNr], fvuStarTokenLast[fuGdpbNr],
                                        fvuStarDaqCmdLast[fuGdpbNr], fvuStarTrigCmdLast[fuGdpbNr],
                                        fuGdpbId );
         Double_t dTriggerTime = newTrig.GetFullGdpbTs() * gdpbv100::kdClockCycleSizeNs;
         if( fvdTrigCandidateTimeStart[ fuGdpbNr ] < dTriggerTime &&
             dTriggerTime < fvdTrigCandidateTimeStop[ fuGdpbNr ] )
            fvvBufferTriggers[fuGdpbNr].push_back( newTrig );

         if( fuCurrentMs < fuNbCoreMsPerTs )
         {
            ULong64_t ulGdpbTsDiff = ulNewGdpbTsFull - fvulGdpbTsFullLast[fuGdpbNr];
            fvulGdpbTsFullLast[fuGdpbNr] = ulNewGdpbTsFull;
            fvulStarTsFullLast[fuGdpbNr] = ulNewStarTsFull;
            fvuStarTokenLast[fuGdpbNr]   = uNewToken;
            fvuStarDaqCmdLast[fuGdpbNr]  = uNewDaqCmd;
            fvuStarTrigCmdLast[fuGdpbNr] = uNewTrigCmd;
         }
         ///---------------------------------------------------------///
         /// Full Trigger only if we reached this point
         if( fbMonitorMode && fbDebugMonitorMode )
            fhRawTriggersStats->Fill( 4., fUnpackPar->GetGdpbToSectorOffset() + fuGdpbNr );

         break;
      } // case 3
      default:
         LOG(ERROR) << "Unknown Star Trigger messageindex: " << iMsgIndex << FairLogger::endl;
   } // switch( iMsgIndex )
}

// -------------------------------------------------------------------------
void CbmStar2019EventBuilderEtofAlgo::ProcessEpSupprBuffer( uint32_t uGdpbNr )
{
   Int_t iBufferSize = fvvmEpSupprBuffer[ fuGdpbNr ].size();

   if( 0 == iBufferSize )
      return;

   LOG(DEBUG) << "Now processing stored messages for for gDPB " << fuGdpbNr << " with epoch number "
              << (fvulCurrentEpoch[fuGdpbNr] - 1) << FairLogger::endl;

   /// Data are sorted between epochs, not inside => Epoch level ordering
   /// Sorting at lower bin precision level
   std::stable_sort( fvvmEpSupprBuffer[ fuGdpbNr ].begin(), fvvmEpSupprBuffer[ fuGdpbNr ].begin() );

   /// Compute original epoch index before epoch suppression
   ULong64_t ulCurEpochGdpbGet4 = fvulCurrentEpochFull[ fuGdpbNr ];

   /// Ignore the first epoch as it should never appear (start delay!!)
   if( 0 == ulCurEpochGdpbGet4 )
      return;

   /// In Ep. Suppr. Mode, receive following epoch instead of previous
   ulCurEpochGdpbGet4 --;

   Int_t messageType = -111;
   for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
   {

      messageType = fvvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ].getMessageType();

      fuGet4Id = fUnpackPar->ElinkIdxToGet4Idx( fvvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ].getGdpbGenChipId() );
      fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + fuGet4Id;

      /// Store the full message in the proper buffer
      gdpbv100::FullMessage fullMess( fvvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ], ulCurEpochGdpbGet4 );

      /// Do other actions on it if needed
      switch( messageType )
      {
         case gdpbv100::MSG_HIT:
         {
            ProcessHit( fvvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ], ulCurEpochGdpbGet4 );
            break;
         } // case gdpbv100::MSG_HIT:
         case gdpbv100::MSG_SLOWC:
         {
            ProcessSlCtrl( fvvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ], ulCurEpochGdpbGet4 );
            break;
         } // case gdpbv100::MSG_SLOWC:
         case gdpbv100::MSG_SYST:
         {
            if( gdpbv100::SYS_PATTERN == fvvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ].getGdpbSysSubType() )
            {
               ProcessPattern( fvvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ], ulCurEpochGdpbGet4 );
            } // if( gdpbv100::SYS_PATTERN == fvvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ].getGdpbSysSubType() )
               else ProcessSysMess( fvvmEpSupprBuffer[ fuGdpbNr ][ iMsgIdx ], ulCurEpochGdpbGet4 );
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

   fvvmEpSupprBuffer[ fuGdpbNr ].clear();
}
void CbmStar2019EventBuilderEtofAlgo::StoreMessageInBuffer( gdpbv100::FullMessage fullMess, uint32_t uGdpbNr )
{
   /// Store in the major error buffer only if GET4 error not channel related
   uint16_t usG4ErrorType = fullMess.getGdpbSysErrData();
   if( gdpbv100::MSG_SYST == fullMess.getMessageType() &&
       gdpbv100::SYS_GET4_ERROR == fullMess.getGdpbSysSubType() &&
       ( usG4ErrorType < gdpbv100::GET4_V2X_ERR_TOT_OVERWRT ||
         usG4ErrorType > gdpbv100::GET4_V2X_ERR_SEQUENCE_ER )
      )
   {
      fvvBufferMajorAsicErrors[fuGdpbNr].push_back( fullMess );
      return;
   } // if GET4 error out of TOT/hit building error range

   if( fvdMessCandidateTimeStart[ fuGdpbNr ] < fullMess.GetFullTimeNs() &&
       fullMess.GetFullTimeNs() < fvdMessCandidateTimeStop[ fuGdpbNr ] )
      fvvBufferMessages[fuGdpbNr].push_back( fullMess );
}

// -------------------------------------------------------------------------
void CbmStar2019EventBuilderEtofAlgo::ProcessHit( gdpbv100::Message mess, uint64_t ulCurEpochGdpbGet4 )
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
   UInt_t uRemappedChannelNr = uFeeNr * fuNrOfChannelsPerFee + fUnpackPar->Get4ChanToPadiChan( uChannelNrInFee );
   UInt_t uGbtxNr            = (uFeeNr / fUnpackPar->GetNrOfFeePerGbtx());
   UInt_t uFeeInGbtx         = (uFeeNr % fUnpackPar->GetNrOfFeePerGbtx());
   UInt_t uGbtxNrInSys       = fuGdpbNr * fUnpackPar->GetNrOfGbtxPerGdpb() + uGbtxNr;

   ULong_t  ulhitTime = mess.getMsgFullTime(  ulCurEpochGdpbGet4 );
   Double_t dHitTime  = mess.getMsgFullTimeD( ulCurEpochGdpbGet4 );
   Double_t dHitTot   = uTot;     // in bins

   UInt_t uFebIdx     = (fuGet4Id / fUnpackPar->GetNrOfGet4PerFee());
   UInt_t uFullFebIdx = (fuGdpbNr * fUnpackPar->GetNrOfFeePerGdpb()) + uFebIdx;

   UInt_t uChanInGdpb = fuGet4Id * fuNrOfChannelsPerGet4 + uChannel;
   UInt_t uChanInSyst = fuGdpbNr * fuNrOfChannelsPerGdpb + uChanInGdpb;
   if( fUnpackPar->GetNumberOfChannels() < uChanInSyst )
   {
      LOG(ERROR) << "Invalid mapping index " << uChanInSyst
                 << " VS " << fUnpackPar->GetNumberOfChannels()
                 <<", from " << fuGdpbNr
                 <<", " << fuGet4Id
                 <<", " << uChannel
                 << FairLogger::endl;
      return;
   } // if( fUnpackPar->GetNumberOfChannels() < uChanUId )

}

void CbmStar2019EventBuilderEtofAlgo::ProcessSlCtrl( gdpbv100::Message mess, uint64_t ulCurEpochGdpbGet4 )
{
}

void CbmStar2019EventBuilderEtofAlgo::ProcessSysMess( gdpbv100::Message mess, uint64_t ulCurEpochGdpbGet4 )
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

void CbmStar2019EventBuilderEtofAlgo::ProcessPattern( gdpbv100::Message mess, uint64_t ulCurEpochGdpbGet4 )
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

Bool_t CbmStar2019EventBuilderEtofAlgo::BuildEvents()
{
   /// Get an iterator to the 1st trigger of each gDPB/sector
   /// + Use iterators to keep track of where we stand in the message and error buffers
   /// => Allow to avoid needless looping and to properly deal with user defined deadtime (double counting)
   std::vector< std::vector< CbmTofStarTrigger2019 >::iterator > itTrigger; /// [sector]
   std::vector< std::vector< gdpbv100::FullMessage >::iterator > itErrorMessStart; /// [sector]
   std::vector< std::vector< gdpbv100::FullMessage >::iterator > itMessStart; /// [sector]
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      itTrigger.push_back( fvvBufferTriggers[ uGdpb ].begin() );
      itErrorMessStart.push_back( fvvBufferMajorAsicErrors[ uGdpb ].begin() );
      itMessStart.push_back( fvvBufferMessages[ uGdpb ].begin() );
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

   /// Check that all gDPB/sectors are not straight at end of their trigger buffer (TS w/o triggers)
   Bool_t bAllSectAllTriggDone = kTRUE;
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      /// => Check can be stopped ad soon as a gDPB/sector not at end is found!
      if( fvvBufferTriggers[ uGdpb ].end() != itTrigger[ uGdpb ] )
      {
         bAllSectAllTriggDone = kFALSE;
         break;
      } // if( fvvBufferTriggers[ uGdpb ].end() != (*itTrigger[ uGdpb ]) )
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

   /// while at least one gDPB not at end of its triger buffer
   /// ==> Need to include a TS edge check? Should be taken care of be the fdAllowedTriggersSpread
   ///     but check here if problem of split events on TS edge
   while( kFALSE == bAllSectAllTriggDone )
   {
      /// Check if all @ same trigger ID and CMD fields
      Bool_t bAllSectorsMatch = kTRUE;
      UInt_t   uFirstStarToken    = (*itTrigger[ 0 ]).GetStarToken();
      UShort_t usFirstStarDaqCmd  = (*itTrigger[ 0 ]).GetStarDaqCmd();
      UShort_t usFirstStarTrigCmd = (*itTrigger[ 0 ]).GetStarTrigCmd();
      for( UInt_t uGdpb = 1; uGdpb < fuNrOfGdpbs; ++uGdpb )
      {
         if(   itTrigger[ uGdpb ] == fvvBufferTriggers[ uGdpb ].end()     ||
             (*itTrigger[ uGdpb ]).GetStarToken()   != uFirstStarToken    ||
             (*itTrigger[ uGdpb ]).GetStarDaqCmd()  != usFirstStarDaqCmd  ||
             (*itTrigger[ uGdpb ]).GetStarTrigCmd() != usFirstStarTrigCmd
            )
         {
            bAllSectorsMatch = kFALSE;
            break;
         } // If end of buffer or any field differs for any gDPB/sector current trigger, not all matched!
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

      if( kTRUE == bAllSectorsMatch )
      {
         /// Yes = Complete event
         /// Create event
         CbmTofStarSubevent2019 starSubEvent; /// Mean trigger is set latter, SourceId 0 for the full wheel!

         /// Loop gDPB/sector
         Double_t dMeanTriggerGdpbTs = 0;
         Double_t dMeanTriggerStarTs = 0;
         for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
         {
            /// Prepare mean trigger for event header
            dMeanTriggerGdpbTs += (*itTrigger[ uGdpb ]).GetFullGdpbTs();
            dMeanTriggerStarTs += (*itTrigger[ uGdpb ]).GetFullStarTs();

            if( fbMonitorMode && fbDebugMonitorMode )
               fvhTriggerDt[ uGdpb ]->Fill(  (*itTrigger[ uGdpb ]).GetFullGdpbTs()
                                           - (*itTrigger[ 0 ]).GetFullGdpbTs() );

            /// Convert STAR trigger to 4 full messages and insert in buffer
            std::vector< gdpbv100::FullMessage > vTrigMess = (*itTrigger[ uGdpb ]).GetGdpbMessages();
            for( std::vector< gdpbv100::FullMessage >::iterator itMess = vTrigMess.begin(); itMess != vTrigMess.end(); ++ itMess )
            {
               starSubEvent.AddMsg( (*itMess) );
            } // for( std::vector< gdpbv100::FullMessage >::iterator itMess = vTrigMess.begin(); itMess != vTrigMess.end(); ++ itMess )

            /// Prepare trigger window limits in ns for data selection
            Double_t dWinBeg = gdpbv100::kdClockCycleSizeNs * (*itTrigger[ uGdpb ]).GetFullGdpbTs()
                              - fdStarTriggerDelay[ uGdpb ];
            Double_t dWinEnd = gdpbv100::kdClockCycleSizeNs * (*itTrigger[ uGdpb ]).GetFullGdpbTs()
                              - fdStarTriggerDelay[ uGdpb ]
                              + fdStarTriggerWinSize[ uGdpb ];
            Double_t dDeadEnd = gdpbv100::kdClockCycleSizeNs * (*itTrigger[ uGdpb ]).GetFullGdpbTs()
                               + fdStarTriggerDeadtime[ uGdpb ];

            /// Loop on important errors buffer and select all from "last event" to "end of trigger window"
            while( itErrorMessStart[ uGdpb ] != fvvBufferMajorAsicErrors[ uGdpb ].end() &&
                   (*itErrorMessStart[ uGdpb ]).GetFullTimeNs() < dWinEnd
                  )
            {
               starSubEvent.AddMsg( (*itErrorMessStart[ uGdpb ]) );
               ++itErrorMessStart[ uGdpb ];
            } // while( not at buffer end && (*itErrorMessStart[ uGdpb ]).GetFullTimeNs() < dWinEnd )

            /// Loop on data and select data fitting the trigger window
            /// Also advance start iterator to end of deadtime
            std::vector< gdpbv100::FullMessage >::iterator itFirstMessOutOfDeadtime = itMessStart[ uGdpb ];
            while( itMessStart[ uGdpb ] != fvvBufferMessages[ uGdpb ].end() &&
                   ( (*itMessStart[ uGdpb ]).GetFullTimeNs() < dDeadEnd ||
                     (*itMessStart[ uGdpb ]).GetFullTimeNs() < dWinEnd
                    )
                  )
            {
               Double_t dMessTime = (*itMessStart[ uGdpb ]).GetFullTimeNs();

               /// Fill Histo if monitor mode
               if( fbMonitorMode )
                  fvhHitsTimeToTriggerRaw[ uGdpb ]->Fill( dMessTime );

               /// If in trigger window, add to event
               if( dWinBeg < dMessTime && dMessTime < dWinEnd )
               {
                  starSubEvent.AddMsg( (*itMessStart[ uGdpb ]) );
                  /// Fill Histo if monitor mode
                  if( fbMonitorMode )
                  {
                     fvhHitsTimeToTriggerSel[ uGdpb ]->Fill( dMessTime );
                     if( fbDebugMonitorMode )
                     {
                        fvhHitsTimeToTriggerSelVsDaq[ uGdpb ]->Fill(  dMessTime, usFirstStarDaqCmd );
                        fvhHitsTimeToTriggerSelVsTrig[ uGdpb ]->Fill( dMessTime, usFirstStarTrigCmd );
                     } // if( fbDebugMonitorMode )
                  } // if( fbMonitorMode )
               } // if( dWinBeg < dMessTime && dMessTime < dWinEnd )

               /// Needed to catch the case where deadtime finishes before the trigger window
               if( dMessTime < dDeadEnd )
                  ++itFirstMessOutOfDeadtime;

               ++itMessStart[ uGdpb ];
            } // while( not at buffer end && (not out of deadtime || not out of trigg win ) )
            itMessStart[ uGdpb ] = itFirstMessOutOfDeadtime;

            /// Advance iterator on trigger
            ++itTrigger[ uGdpb ];
         } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

         /// Set Mean trigger in event header
         dMeanTriggerGdpbTs /= fuNrOfGdpbs;
         dMeanTriggerStarTs /= fuNrOfGdpbs;
         CbmTofStarTrigger2019 meanTrigger( static_cast< ULong64_t >( dMeanTriggerGdpbTs ), static_cast< ULong64_t >( dMeanTriggerStarTs ),
                                            uFirstStarToken, usFirstStarDaqCmd, usFirstStarTrigCmd );
         starSubEvent.SetTrigger( meanTrigger );

         /// Add event to event buffer
         fvEventsBuffer.push_back( starSubEvent );
      } // if( kTRUE == bAllSectorsMatch )
         else
         {
            /// No = at least 1 missed a trigger
            /// Check which gDPB/sector has the trigger with the smallest gDPB TS
            std::vector< CbmTofStarTrigger2019 >::iterator itEarliestTrigger;
            ULong64_t ulEarliestGdpbTs = 0xFFFFFFFFFFFFFFFFUL;
            for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
            {
               if(   itTrigger[ uGdpb ] != fvvBufferTriggers[ uGdpb ].end() &&
                   (*itTrigger[ uGdpb ]).GetFullGdpbTs() < ulEarliestGdpbTs )
               {
                  itEarliestTrigger = itTrigger[ uGdpb ];
                  ulEarliestGdpbTs  = (*itTrigger[ uGdpb ]).GetFullGdpbTs();
               } // if( not at end of buffer && (*itTrigger[ uGdpb ]).GetFullGdpbTs() < ulEarliestGdpbTs )
            } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
            UInt_t   uEarliestStarToken    = (*itEarliestTrigger).GetStarToken();
            UShort_t usEarliestStarDaqCmd  = (*itEarliestTrigger).GetStarDaqCmd();
            UShort_t usEarliestStarTrigCmd = (*itEarliestTrigger).GetStarTrigCmd();

            /// Find all gDPB with a matching trigger
            UInt_t uNrOfMatchedGdpbs = 0;
            std::vector< Bool_t > vbMatchingTrigger( fuNrOfGdpbs, kFALSE );  /// [sector]
            Double_t dMeanTriggerGdpbTs = 0;
            Double_t dMeanTriggerStarTs = 0;
            for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
            {
               if(   itTrigger[ uGdpb ] != fvvBufferTriggers[ uGdpb ].end()     &&
                   (*itTrigger[ uGdpb ]).GetStarToken()   == uEarliestStarToken  &&
                   (*itTrigger[ uGdpb ]).GetStarDaqCmd()  == usEarliestStarDaqCmd &&
                   (*itTrigger[ uGdpb ]).GetStarTrigCmd() == usEarliestStarTrigCmd
                  )
               {
                  uNrOfMatchedGdpbs++;
                  vbMatchingTrigger[ uGdpb ] = kTRUE;

                  if( fbMonitorMode && fbDebugMonitorMode )
                     fvhTriggerDt[ uGdpb ]->Fill(  (*itTrigger[ uGdpb ]).GetFullGdpbTs()
                                                 - (*itTrigger[ 0 ]).GetFullGdpbTs() );

                  dMeanTriggerGdpbTs += (*itTrigger[ uGdpb ]).GetFullGdpbTs();
                  dMeanTriggerStarTs += (*itTrigger[ uGdpb ]).GetFullStarTs();
               } // if matching trigger
            } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

            /// Compute the mean trigger from all matching triggers
            dMeanTriggerGdpbTs /= uNrOfMatchedGdpbs;
            dMeanTriggerStarTs /= uNrOfMatchedGdpbs;
            CbmTofStarTrigger2019 meanTrigger( static_cast< ULong64_t >( dMeanTriggerGdpbTs ), static_cast< ULong64_t >( dMeanTriggerStarTs ),
                                               (*itEarliestTrigger).GetStarToken(),
                                               (*itEarliestTrigger).GetStarDaqCmd(),
                                               (*itEarliestTrigger).GetStarTrigCmd() );

            /// Create event & add to header the mean trigger, the "incomplete event" flag
            CbmTofStarSubevent2019 starSubEvent; /// Mean trigger is set latter, SourceId 0 for the full wheel!
            starSubEvent.SetTrigger( meanTrigger );
            starSubEvent.SetIncompleteEventFlag();

            /// Loop gDPB/sector
            for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
            {
               /// Check if this gDPB/sector has a matching trigger
                  /// Select trigger time => If matching trigger, use its TS, otherwise use the calulated mean trigger TS
               ULong64_t ulTriggerTime = static_cast< ULong64_t >( dMeanTriggerGdpbTs );
               if( kTRUE == vbMatchingTrigger[ uGdpb ] )
               {
                  /// Select trigger time => If matching trigger, use its TS, otherwise use the calulated mean trigger TS
                  ulTriggerTime = (*itTrigger[ uGdpb ]).GetFullGdpbTs();

                  /// If matching trigger, convert STAR trigger to 4 full messages and insert in buffer
                  std::vector< gdpbv100::FullMessage > vTrigMess = (*itTrigger[ uGdpb ]).GetGdpbMessages();
                  for( std::vector< gdpbv100::FullMessage >::iterator itMess = vTrigMess.begin(); itMess != vTrigMess.end(); ++ itMess )
                  {
                     starSubEvent.AddMsg( (*itMess) );
                  } // for( std::vector< gdpbv100::FullMessage >::iterator itMess = vTrigMess.begin(); itMess != vTrigMess.end(); ++ itMess )
               } // if( kTRUE == vbMatchingTrigger[ uGdpb ] )
                  else if( fbMonitorMode && fbDebugMonitorMode )
                  {
                     /// Fill missing trigger per sector evolution plot only in debug monitor mode
                     fhMissingTriggersEvolution->Fill( dMeanTriggerGdpbTs / 1e9 / 60.0,
                                                       fUnpackPar->GetGdpbToSectorOffset() + uGdpb );
                  } // else if( fbMonitorMode && fbDebugMonitorMode )


               /// Prepare trigger window limits in ns for data selection
               Double_t dWinBeg = gdpbv100::kdClockCycleSizeNs * ulTriggerTime
                                 - fdStarTriggerDelay[ uGdpb ];
               Double_t dWinEnd = gdpbv100::kdClockCycleSizeNs * ulTriggerTime
                                 - fdStarTriggerDelay[ uGdpb ]
                                 + fdStarTriggerWinSize[ uGdpb ];
               Double_t dDeadEnd = gdpbv100::kdClockCycleSizeNs * ulTriggerTime
                                  + fdStarTriggerDeadtime[ uGdpb ];

               /// Loop on important errors buffer and select all from "last event" to "end of trigger window"
               while( itErrorMessStart[ uGdpb ] != fvvBufferMajorAsicErrors[ uGdpb ].end() &&
                      (*itErrorMessStart[ uGdpb ]).GetFullTimeNs() < dWinEnd )
               {
                  starSubEvent.AddMsg( (*itErrorMessStart[ uGdpb ]) );
                  ++itErrorMessStart[ uGdpb ];
               } // while( not at buffer end && (*itErrorMessStart[ uGdpb ]).GetFullTimeNs() < dWinEnd )

               /// Loop on data and select data fitting the trigger window
               /// Also advance start iterator to end of deadtime
               std::vector< gdpbv100::FullMessage >::iterator itFirstMessOutOfDeadtime = itMessStart[ uGdpb ];
               while( itMessStart[ uGdpb ] != fvvBufferMessages[ uGdpb ].end() &&
                      ( (*itMessStart[ uGdpb ]).GetFullTimeNs() < dDeadEnd ||
                        (*itMessStart[ uGdpb ]).GetFullTimeNs() < dWinEnd
                       )
                     )
               {
                  Double_t dMessTime = (*itMessStart[ uGdpb ]).GetFullTimeNs();

                  /// Fill Histo if monitor mode
                  if( fbMonitorMode )
                     fvhHitsTimeToTriggerRaw[ uGdpb ]->Fill( dMessTime );

                  /// If in trigger window, add to event
                  if( dWinBeg < dMessTime && dMessTime < dWinEnd )
                  {
                     starSubEvent.AddMsg( (*itMessStart[ uGdpb ]) );
                     /// Fill Histo if monitor mode
                     if( fbMonitorMode )
                     {
                        fvhHitsTimeToTriggerSel[ uGdpb ]->Fill( dMessTime );
                        if( fbDebugMonitorMode )
                        {
                           fvhHitsTimeToTriggerSelVsDaq[ uGdpb ]->Fill(  dMessTime, usEarliestStarDaqCmd );
                           fvhHitsTimeToTriggerSelVsTrig[ uGdpb ]->Fill( dMessTime, usEarliestStarTrigCmd );
                        } // if( fbDebugMonitorMode )
                     } // if( fbMonitorMode )
                  } // if( dWinBeg < dMessTime && dMessTime < dWinEnd )

                  /// Needed to catch the case where deadtime finishes before the trigger window
                  if( dMessTime < dDeadEnd )
                     ++itFirstMessOutOfDeadtime;

                  ++itMessStart[ uGdpb ];
               } // while( not at buffer end && (not out of deadtime || not out of trigg win ) )
               itMessStart[ uGdpb ] = itFirstMessOutOfDeadtime;

               /// Advance iterator for gDPB/sector with matching trigger
               if( kTRUE == vbMatchingTrigger[ uGdpb ] )
                  ++itTrigger[ uGdpb ];
            } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

            /// Add event to event buffer
            fvEventsBuffer.push_back( starSubEvent );
         } // else of if( kTRUE == bAllSectorsMatch )

      /// Check if all gDPB/sectors are at end of their trigger buffer
      bAllSectAllTriggDone = kTRUE;
      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      {
         /// => Check can be stopped ad soon as a gDPB/sector not at end is found!
         if( fvvBufferTriggers[ uGdpb ].end() != itTrigger[ uGdpb ] )
         {
            bAllSectAllTriggDone = kFALSE;
            break;
         } // if( fvvBufferTriggers[ uGdpb ].end() != (*itTrigger[ uGdpb ]) )
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   } // while( kFALSE == bAllSectAllTriggDone )

   return kTRUE;
}

// -------------------------------------------------------------------------

Bool_t CbmStar2019EventBuilderEtofAlgo::CreateHistograms()
{
   /// Create sector related histograms
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      UInt_t uSector = fUnpackPar->GetGdpbToSectorOffset() + uGdpb;
      std::string sFolder = Form( "sector%2u", uSector);

      LOG(INFO) << "gDPB " << uGdpb << " is " << sFolder
                 << FairLogger::endl;

      fvhHitsTimeToTriggerRaw.push_back( new TH1D(
         Form( "hHitsTimeToTriggerRawSect%2u", uSector ),
         Form( "Time to trigger for all neighboring hits in sector %2u; t - Ttrigg [ns]; Hits []", uSector ),
         2000, -5000, 5000  ) );

      UInt_t uNbBinsDtSel = fdStarTriggerWinSize[ uGdpb ];
      Double_t dMaxDtSel = fdStarTriggerDelay[ uGdpb ] + fdStarTriggerWinSize[ uGdpb ];
      fvhHitsTimeToTriggerSel.push_back( new TH1D(
         Form( "hHitsTimeToTriggerSelSect%2u", uSector ),
         Form( "Time to trigger for all selected hits in sector %2u; t - Ttrigg [ns]; Hits []", uSector ),
         uNbBinsDtSel, fdStarTriggerDelay[ uGdpb ], dMaxDtSel ) );

      /// Add pointers to the vector with all histo for access by steering class
      AddHistoToVector( fvhHitsTimeToTriggerRaw[ uGdpb ], sFolder );
      AddHistoToVector( fvhHitsTimeToTriggerSel[ uGdpb ], sFolder );

      if( kTRUE == fbDebugMonitorMode )
      {
         fvhHitsTimeToTriggerSelVsDaq.push_back( new TH2D(
            Form( "hHitsTimeToTriggerSelVsDaqSect%2u", uSector ),
            Form( "Time to trigger for all selected hits vs DAQ CMD in sector %2u; t - Ttrigg [ns]; DAQ CMD []; Hits []", uSector ),
            uNbBinsDtSel, fdStarTriggerDelay[ uGdpb ], dMaxDtSel,
            16, 0., 16.  ) );

         fvhHitsTimeToTriggerSelVsTrig.push_back( new TH2D(
            Form( "hHitsTimeToTriggerSelVsTrigSect%2u", uSector ),
            Form( "Time to trigger for all selected hits vs TRIG CMD in sector %2u; t - Ttrigg [ns]; TRIG CMD []; Hits []", uSector ),
            uNbBinsDtSel, fdStarTriggerDelay[ uGdpb ], dMaxDtSel,
            16, 0., 16.  ) );

         fvhTriggerDt.push_back( new TH1I(
            Form( "hhTriggerDtSect%2u", uSector ),
            Form( "Trigger time difference between sector %2u and the first sector, full events only; Ttrigg%2u - TtriggRef [ns]; events []",
                  uSector, uSector ),
            200, -100, 100  ) );

         /// Add pointers to the vector with all histo for access by steering class
         AddHistoToVector( fvhHitsTimeToTriggerSelVsDaq[ uGdpb ],  sFolder );
         AddHistoToVector( fvhHitsTimeToTriggerSelVsTrig[ uGdpb ], sFolder );
         AddHistoToVector( fvhTriggerDt[ uGdpb ],                  sFolder );
      } // if( kTRUE == fbDebugMonitorMode )
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

   /// Create event builder related histograms
   fhEventNbPerTs = new TH1I( "hEventNbPerTs",
      "Number of Events per TS; Events []; TS []",
      1000 , 0, 1000 );

   fhEventSizeDistribution = new TH1I( "hEventSizeDistribution",
      "Event size distribution; Event size [byte]; Events []",
      CbmTofStarSubevent2019::GetMaxOutputSize()/8 , 0, CbmTofStarSubevent2019::GetMaxOutputSize() );

   fhEventSizeEvolution    = new TProfile( "hEventSizeEvolution",
      "Event size evolution; Time in run [min]; mean Event size [byte];",
       14400, 0, 14400 );

   fhEventNbEvolution      = new TH1I( "hEventNbEvolution",
      "Event number evolution; Time in run [min]; Events [];",
       14400, 0, 14400 );

   /// Add pointers to the vector with all histo for access by steering class
   AddHistoToVector( fhEventNbPerTs,          "eventbuilder" );
   AddHistoToVector( fhEventSizeDistribution, "eventbuilder" );
   AddHistoToVector( fhEventSizeEvolution,    "eventbuilder" );
   AddHistoToVector( fhEventNbEvolution,      "eventbuilder" );

   if( kTRUE == fbDebugMonitorMode )
   {
      /// FIXME: hardcoded nb of MS in TS (include overlap)
      /// as this number is known only later when 1st TS is received
      UInt_t uNbBinsInTs = fdMsSizeInNs * 101 / 1000. / 10.;

      fhEventNbDistributionInTs   = new TH1I( "hEventNbDistributionInTs",
         "Event number distribution inside TS; Time in TS [us]; Events [];",
          uNbBinsInTs, -0.5, fdMsSizeInNs * 101 / 1000. - 0.5 );

      fhEventSizeDistributionInTs = new TProfile( "hEventSizeDistributionInTs",
         "Event size distribution inside TS; Time in TS [us]; mean Event size [Byte];",
          uNbBinsInTs, -0.5, fdMsSizeInNs * 101 / 1000. - 0.5 );

      fhRawTriggersStats = new TH2I(
         "hRawTriggersStats",
         "Raw triggers statistics per sector; ; Sector []; Messages []",
         5, 0, 5,
         12, 13, 25  );
      fhRawTriggersStats->GetXaxis()->SetBinLabel( 1, "A");
      fhRawTriggersStats->GetXaxis()->SetBinLabel( 2, "B");
      fhRawTriggersStats->GetXaxis()->SetBinLabel( 3, "C");
      fhRawTriggersStats->GetXaxis()->SetBinLabel( 4, "D");
      fhRawTriggersStats->GetXaxis()->SetBinLabel( 5, "F");

      fhMissingTriggersEvolution = new TH2I(
         "hMissingTriggersEvolution",
         "Missing trigger counts per sector vs time in run; Time in run [min]; Sector []; Missing triggers []",
         14400, 0, 14400,
         12, 13, 25  );

      /// Add pointers to the vector with all histo for access by steering class
      AddHistoToVector( fhEventNbDistributionInTs,   "eventbuilder" );
      AddHistoToVector( fhEventSizeDistributionInTs, "eventbuilder" );
      AddHistoToVector( fhRawTriggersStats,          "eventbuilder" );
      AddHistoToVector( fhMissingTriggersEvolution, "eventbuilder" );
   } // if( kTRUE == fbDebugMonitorMode )

   return kTRUE;
}
Bool_t CbmStar2019EventBuilderEtofAlgo::FillHistograms()
{
   UInt_t uNbEvents = fvEventsBuffer.size();
   fhEventNbPerTs->Fill( uNbEvents );

   for( UInt_t uEvent = 0; uEvent < uNbEvents; ++uEvent )
   {
      UInt_t uEventSize       = fvEventsBuffer[ uEvent ].GetEventSize();
      Double_t dEventTimeSec  = fvEventsBuffer[ uEvent ].GetEventTimeSec();
      Double_t dEventTimeMin  = dEventTimeSec / 60.0;

      fhEventSizeDistribution->Fill( uEventSize );
      fhEventSizeEvolution->Fill( dEventTimeMin, uEventSize );
      fhEventNbEvolution->Fill( dEventTimeMin );

      if( kTRUE == fbDebugMonitorMode )
      {
         Double_t dEventTimeInTs = ( fvEventsBuffer[ uEvent ].GetTrigger().GetFullGdpbTs() * gdpbv100::kdClockCycleSizeNs
                                    - fdTsStartTime ) / 1000.0;

         fhEventNbDistributionInTs->Fill( dEventTimeInTs  );
         fhEventSizeDistributionInTs->Fill( dEventTimeInTs, uEventSize );
      } // if( kTRUE == fbDebugMonitorMode )
   } // for( UInt_t uEvent = 0; uEvent < uNbEvents; ++uEvent )

   return kTRUE;
}
Bool_t CbmStar2019EventBuilderEtofAlgo::ResetHistograms()
{
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      fvhHitsTimeToTriggerRaw[ uGdpb ]->Reset();
      fvhHitsTimeToTriggerSel[ uGdpb ]->Reset();

      if( kTRUE == fbDebugMonitorMode )
      {
         fvhHitsTimeToTriggerSelVsDaq[ uGdpb ]->Reset();
         fvhHitsTimeToTriggerSelVsTrig[ uGdpb ]->Reset();
         fvhTriggerDt[ uGdpb ]->Reset();
      } // if( kTRUE == fbDebugMonitorMode )
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

   /// Create event builder related histograms
   fhEventNbPerTs->Reset();
   fhEventSizeDistribution->Reset();
   fhEventSizeEvolution->Reset();
   fhEventNbEvolution->Reset();

   if( kTRUE == fbDebugMonitorMode )
   {
      fhEventNbDistributionInTs->Reset();
      fhEventSizeDistributionInTs->Reset();
      fhRawTriggersStats->Reset();
      fhMissingTriggersEvolution->Reset();
   } // if( kTRUE == fbDebugMonitorMode )

   return kTRUE;
}
// -------------------------------------------------------------------------
