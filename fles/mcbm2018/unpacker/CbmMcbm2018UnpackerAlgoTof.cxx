// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                  CbmMcbm2018UnpackerAlgoTof                       -----
// -----               Created 10.02.2019 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018UnpackerAlgoTof.h"

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
CbmMcbm2018UnpackerAlgoTof::CbmMcbm2018UnpackerAlgoTof() :
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
   fuNrOfGbtx( 0 ),
   fuNrOfModules( 0 ),
   fviNrOfRpc(),
   fviRpcType(),
   fviRpcSide(),
   fviModuleId(),
   fviRpcChUId(),
   fdTimeOffsetNs( 0.0 ),
   fdTShiftRef( 0.0 ),
   fuDiamondDpbIdx( 2 ),
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
   fdStartTime( 0.0 ),
   fdStartTimeMsSz( 0.0 ),
   ftStartTimeUnix( std::chrono::steady_clock::now() ),
   fvvmEpSupprBuffer(),
   fvmHitsInMs(),
   fvulGdpbTsMsb(),
   fvulGdpbTsLsb(),
   fvulStarTsMsb(),
   fvulStarTsMid(),
   fvulGdpbTsFullLast(),
   fvulStarTsFullLast(),
   fvuStarTokenLast(),
   fvuStarDaqCmdLast(),
   fvuStarTrigCmdLast(),
   fdRefTime(0.),
   fdLastDigiTime(0.),
   fdFirstDigiTimeDif(0.),
   fdEvTime0(0.),
   fhRawTDigEvT0( nullptr ),
   fhRawTDigRef0( nullptr ),
   fhRawTDigRef( nullptr ),
   fhRawTRefDig0( nullptr ),
   fhRawTRefDig1( nullptr ),
   fhRawDigiLastDigi( nullptr ),
   fhRawTotCh(),
   fhChCount(),
   fhChCountRemap(),
   fvbChanThere(),
   fhChanCoinc(),
   fhDetChanCoinc( nullptr )
{
}
CbmMcbm2018UnpackerAlgoTof::~CbmMcbm2018UnpackerAlgoTof()
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
Bool_t CbmMcbm2018UnpackerAlgoTof::Init()
{
   LOG(INFO) << "Initializing mCBM TOF 2019 unpacker algo" << FairLogger::endl;

   return kTRUE;
}
void CbmMcbm2018UnpackerAlgoTof::Reset()
{
}
void CbmMcbm2018UnpackerAlgoTof::Finish()
{
   /// Printout Goodbye message and stats

   /// Write Output histos

}

// -------------------------------------------------------------------------
Bool_t CbmMcbm2018UnpackerAlgoTof::InitContainers()
{
   LOG(INFO) << "Init parameter containers for CbmMcbm2018UnpackerAlgoTof"
             << FairLogger::endl;
   Bool_t initOK = ReInitContainers();

   return initOK;
}
Bool_t CbmMcbm2018UnpackerAlgoTof::ReInitContainers()
{
   LOG(INFO) << "**********************************************"
             << FairLogger::endl;
   LOG(INFO) << "ReInit parameter containers for CbmMcbm2018UnpackerAlgoTof"
             << FairLogger::endl;

   fUnpackPar = (CbmMcbm2018TofPar*)fParCList->FindObject("CbmMcbm2018TofPar");
   if( nullptr == fUnpackPar )
      return kFALSE;

   Bool_t initOK = InitParameters();

   return initOK;
}
TList* CbmMcbm2018UnpackerAlgoTof::GetParList()
{
   if( nullptr == fParCList )
      fParCList = new TList();
   fUnpackPar = new CbmMcbm2018TofPar("CbmMcbm2018TofPar");
   fParCList->Add(fUnpackPar);

   return fParCList;
}
Bool_t CbmMcbm2018UnpackerAlgoTof::InitParameters()
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

   fuNrOfGbtx  =  fUnpackPar->GetNrOfGbtx();
   LOG(INFO) << "Nr. of GBTx: " << fuNrOfGbtx << FairLogger::endl;

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
   } // for( UInt_t uGbtx = 0; uGbtx < fuNrOfGbtx; ++uGbtx)

   UInt_t uNrOfChannels = fuNrOfGet4 * fuNrOfChannelsPerGet4;
   LOG(INFO) << "Nr. of possible Tof channels: " << uNrOfChannels
             << FairLogger::endl;

   CbmTofDetectorId* fTofId = new CbmTofDetectorId_v14a();
   fviRpcChUId.resize( uNrOfChannels );
   UInt_t iCh= 0;
   for(Int_t iGbtx= 0; iGbtx < fuNrOfGbtx; ++iGbtx )
   {
      /// Special Treatment for the T0 diamond
      if( 5 == fviRpcType[iGbtx] )
      {
         for( UInt_t uFee = 0; uFee < fUnpackPar->GetNrOfFeePerGbtx(); ++uFee )
         {
            for( UInt_t uCh = 0; uCh < fUnpackPar->GetNrOfChannelsPerFee(); ++uCh )
            {
               if( uFee < 4 && 0 == uCh )
                  fviRpcChUId[ iCh ] = CbmTofAddress::GetUniqueAddress(
                                             fviModuleId[iGbtx],
                                             0, uFee + 4 * fviRpcSide[iGbtx],
                                             0, fviRpcType[iGbtx] );
                  else fviRpcChUId[ iCh ] = 0;

               iCh++;
            } // for( UInt_t uCh = 0; uCh < fUnpackPar->GetNrOfChannelsPerFee(); ++uCh )
         } // for( UInt_t uFee = 0; uFee < fUnpackPar->GetNrOfFeePerGbtx(); ++uFee )
         continue;
      } // if( 5 == fviRpcType[iGbtx] )

      for(Int_t iRpc= 0; iRpc < fviNrOfRpc[iGbtx]; ++iRpc )
      {
         /// FIXME: Harcoded numbers will fail for detectors with different mapping!!!!
         for(Int_t iStr= 0; iStr < 32; iStr++)
         {
            Int_t iStrMap = iStr;
            if( fviRpcSide[ iGbtx ] == 1 )
               iStrMap = 31 - iStr;
            fviRpcChUId[ iCh ] = CbmTofAddress::GetUniqueAddress(
                                       fviModuleId[iGbtx],
                                       iRpc,iStrMap,
                                       fviRpcSide[iGbtx],
                                       fviRpcType[iGbtx] );

             LOG(DEBUG1) << Form( "Map Ch %d to Address 0x%08x", iCh, fviRpcChUId[iCh] )
                         << FairLogger::endl;

            iCh++;
         } // for(Int_t iStr= 0; iStr < 32; iStr++)
      } // for(Int_t iRpc= 0; iRpc < fviNrOfRpc[iGbtx]; ++iRpc )
   } // for(Int_t iGbtx= 0; iGbtx < fuNrOfGbtx; ++iGbtx )

   for( UInt_t uCh = 0; uCh < uNrOfChannels; ++uCh )
   {
      if( 0 == uCh % 8 )
         LOG(INFO) << FairLogger::endl;
      LOG(INFO) << Form(" 0x%08x", fviRpcChUId[ uCh ] );
   } // for( UInt_t i = 0; i < uNrOfChannels; ++i)
   LOG(INFO) << FairLogger::endl;

   /// Internal status initialization
   fvulCurrentEpoch.resize( fuNrOfGdpbs, 0 );
   fvulCurrentEpochCycle.resize( fuNrOfGdpbs, 0 );
   fvulCurrentEpochFull.resize( fuNrOfGdpbs, 0 );

   /// STAR Trigger decoding and monitoring
   fvulGdpbTsMsb.resize(  fuNrOfGdpbs );
   fvulGdpbTsLsb.resize(  fuNrOfGdpbs );
   fvulStarTsMsb.resize(  fuNrOfGdpbs );
   fvulStarTsMid.resize(  fuNrOfGdpbs );
   fvulGdpbTsFullLast.resize(  fuNrOfGdpbs );
   fvulStarTsFullLast.resize(  fuNrOfGdpbs );
   fvuStarTokenLast.resize(  fuNrOfGdpbs );
   fvuStarDaqCmdLast.resize(  fuNrOfGdpbs );
   fvuStarTrigCmdLast.resize(  fuNrOfGdpbs );
   for (UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb)
   {
      fvulGdpbTsMsb[ uGdpb ] = 0;
      fvulGdpbTsLsb[ uGdpb ] = 0;
      fvulStarTsMsb[ uGdpb ] = 0;
      fvulStarTsMid[ uGdpb ] = 0;
      fvulGdpbTsFullLast[ uGdpb ] = 0;
      fvulStarTsFullLast[ uGdpb ] = 0;
      fvuStarTokenLast[ uGdpb ]   = 0;
      fvuStarDaqCmdLast[ uGdpb ]  = 0;
      fvuStarTrigCmdLast[ uGdpb ] = 0;
   } // for (Int_t iGdpb = 0; iGdpb < fuNrOfGdpbs; ++iGdpb)

   /// Buffer initialization
   fvvmEpSupprBuffer.resize( fuNrOfGdpbs );

	return kTRUE;
}
// -------------------------------------------------------------------------

void CbmMcbm2018UnpackerAlgoTof::AddMsComponentToList( size_t component, UShort_t usDetectorId )
{
   /// Check for duplicates and ignore if it is the case
   for( UInt_t uCompIdx = 0; uCompIdx < fvMsComponentsList.size(); ++uCompIdx )
      if( component == fvMsComponentsList[ uCompIdx ] )
         return;

   /// Add to list
   fvMsComponentsList.push_back( component );

   LOG(INFO) << "CbmMcbm2018UnpackerAlgoTof::AddMsComponentToList => Component "
             << component << " with detector ID 0x"
             << std::hex << usDetectorId << std::dec << " added to list"
             << FairLogger::endl;

}
// -------------------------------------------------------------------------

Bool_t CbmMcbm2018UnpackerAlgoTof::ProcessTs( const fles::Timeslice& ts )
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

Bool_t CbmMcbm2018UnpackerAlgoTof::ProcessMs( const fles::Timeslice& ts, size_t uMsCompIdx, size_t uMsIdx )
{
   auto msDescriptor = ts.descriptor( uMsCompIdx, uMsIdx );
   fuCurrentEquipmentId = msDescriptor.eq_id;
   const uint8_t* msContent = reinterpret_cast<const uint8_t*>( ts.content( uMsCompIdx, uMsIdx ) );

   uint32_t uSize  = msDescriptor.size;
   fulCurrentMsIdx = msDescriptor.idx;
   Double_t dMsTime = (1e-9) * static_cast<double>(fulCurrentMsIdx);
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
         ProcessEpochCycle( ulData );
         continue;
      } // if( 0 == uIdx )

      gdpbv100::Message mess(ulData);
      /// Get message type
      messageType = mess.getMessageType();

      fuGet4Id = fUnpackPar->ElinkIdxToGet4Idx( mess.getGdpbGenChipId() );
      fuGet4Nr = (fuCurrDpbIdx * fuNrOfGet4PerGdpb) + fuGet4Id;

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
                  fvvmEpSupprBuffer[fuCurrDpbIdx].push_back( mess );
               } // else of if( getGdpbHitIs24b() )
            break;
         } // case gdpbv100::MSG_HIT:
         case gdpbv100::MSG_EPOCH:
         {
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
            break;
         } // case gdpbv100::MSG_EPOCH:
         case gdpbv100::MSG_SLOWC:
         {
            fvvmEpSupprBuffer[fuCurrDpbIdx].push_back( mess );
            break;
         } // case gdpbv100::MSG_SLOWC:
         case gdpbv100::MSG_SYST:
         {
            fvvmEpSupprBuffer[fuCurrDpbIdx].push_back( mess );
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
void CbmMcbm2018UnpackerAlgoTof::ProcessEpochCycle( uint64_t ulCycleData )
{
   ULong64_t ulEpochCycleVal = ulCycleData & gdpbv100::kulEpochCycleFieldSz;

   if( !( ulEpochCycleVal == fvulCurrentEpochCycle[fuCurrDpbIdx] ||
          ulEpochCycleVal == fvulCurrentEpochCycle[fuCurrDpbIdx] + 1 ) &&
       0 < fulCurrentMsIdx ) {
      LOG(WARNING) << "CbmMcbm2018UnpackerAlgoTof::ProcessEpochCycle => "
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
void CbmMcbm2018UnpackerAlgoTof::ProcessEpoch( gdpbv100::Message mess )
{
   ULong64_t ulEpochNr = mess.getGdpbEpEpochNb();
/*
 * /// FIXME: Need proper handling of overlap MS
   if( 0 < fvulCurrentEpoch[ fuCurrDpbIdx ] && ulEpochNr < fvulCurrentEpoch[ fuCurrDpbIdx ] )
   {
      std::cout << Form( "gDPB %2d", fuCurrDpbIdx) << " New Epoch cycle "
                << Form( "0x%012llx old Ep %08llx new Ep %08llx", fvulCurrentEpochCycle[ fuCurrDpbIdx ], fvulCurrentEpoch[ fuCurrDpbIdx ], ulEpochNr )
                << std::endl;
      fvulCurrentEpochCycle[ fuCurrDpbIdx ]++;
   } // if( 0 < fvulCurrentEpoch[ fuCurrDpbIdx ] && ulEpochNr < fvulCurrentEpoch[ fuCurrDpbIdx ] )
*/
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
void CbmMcbm2018UnpackerAlgoTof::ProcessStarTrigger(gdpbv100::Message mess)
{
  Int_t iMsgIndex = mess.getStarTrigMsgIndex();

  switch( iMsgIndex )
  {
      case 0:
         fvulGdpbTsMsb[fuCurrDpbIdx] = mess.getGdpbTsMsbStarA();
         break;
      case 1:
         fvulGdpbTsLsb[fuCurrDpbIdx] = mess.getGdpbTsLsbStarB();
         fvulStarTsMsb[fuCurrDpbIdx] = mess.getStarTsMsbStarB();
         break;
      case 2:
         fvulStarTsMid[fuCurrDpbIdx] = mess.getStarTsMidStarC();
         break;
      case 3:
      {
         ULong64_t ulNewGdpbTsFull = ( fvulGdpbTsMsb[fuCurrDpbIdx] << 24 )
                                   + ( fvulGdpbTsLsb[fuCurrDpbIdx]       );
         ULong64_t ulNewStarTsFull = ( fvulStarTsMsb[fuCurrDpbIdx] << 48 )
                                   + ( fvulStarTsMid[fuCurrDpbIdx] <<  8 )
                                   + mess.getStarTsLsbStarD();
         UInt_t uNewToken  = mess.getStarTokenStarD();
         UInt_t uNewDaqCmd  = mess.getStarDaqCmdStarD();
         UInt_t uNewTrigCmd = mess.getStarTrigCmdStarD();

         if( ( uNewToken == fvuStarTokenLast[fuCurrDpbIdx] ) && ( ulNewGdpbTsFull == fvulGdpbTsFullLast[fuCurrDpbIdx] ) &&
             ( ulNewStarTsFull == fvulStarTsFullLast[fuCurrDpbIdx] ) && ( uNewDaqCmd == fvuStarDaqCmdLast[fuCurrDpbIdx] ) &&
             ( uNewTrigCmd == fvuStarTrigCmdLast[fuCurrDpbIdx] ) )
         {
            UInt_t uTrigWord =  ( (fvuStarTrigCmdLast[fuCurrDpbIdx] & 0x00F) << 16 )
                     + ( (fvuStarDaqCmdLast[fuCurrDpbIdx]   & 0x00F) << 12 )
                     + ( (fvuStarTokenLast[fuCurrDpbIdx]    & 0xFFF)       );
            LOG(WARNING) << "Possible error: identical STAR tokens found twice in a row => ignore 2nd! "
                         << " TS " << fulCurrentTsIdx
                         << " gDBB #" << fuCurrDpbIdx << " "
                         << Form("token = %5u ", fvuStarTokenLast[fuCurrDpbIdx] )
                         << Form("gDPB ts  = %12llu ", fvulGdpbTsFullLast[fuCurrDpbIdx] )
                         << Form("STAR ts = %12llu ", fvulStarTsFullLast[fuCurrDpbIdx] )
                         << Form("DAQ cmd = %2u ", fvuStarDaqCmdLast[fuCurrDpbIdx] )
                         << Form("TRG cmd = %2u ", fvuStarTrigCmdLast[fuCurrDpbIdx] )
                         << Form("TRG Wrd = %5x ", uTrigWord )
                         << FairLogger::endl;
            return;
         } // if exactly same message repeated

         // GDPB TS counter reset detection
         if( ulNewGdpbTsFull < fvulGdpbTsFullLast[fuCurrDpbIdx] )
            LOG(DEBUG) << "Probable reset of the GDPB TS: old = " << Form("%16llu", fvulGdpbTsFullLast[fuCurrDpbIdx])
                       << " new = " << Form("%16llu", ulNewGdpbTsFull)
                       << " Diff = -" << Form("%8llu", fvulGdpbTsFullLast[fuCurrDpbIdx] - ulNewGdpbTsFull)
                       << " GDPB #" << Form( "%2u", fuCurrDpbIdx)
                       << FairLogger::endl;

         // STAR TS counter reset detection
         if( ulNewStarTsFull < fvulStarTsFullLast[fuCurrDpbIdx] )
            LOG(DEBUG) << "Probable reset of the STAR TS: old = " << Form("%16llu", fvulStarTsFullLast[fuCurrDpbIdx])
                       << " new = " << Form("%16llu", ulNewStarTsFull)
                       << " Diff = -" << Form("%8llu", fvulStarTsFullLast[fuCurrDpbIdx] - ulNewStarTsFull)
                       << " GDPB #" << Form( "%2u", fuCurrDpbIdx)
                       << FairLogger::endl;

         /// Check needed to avoid double counting
         if( fulCurrentMsIdx < fuNbCoreMsPerTs )
         {
            fvulGdpbTsFullLast[fuCurrDpbIdx] = ulNewGdpbTsFull;
            fvulStarTsFullLast[fuCurrDpbIdx] = ulNewStarTsFull;
            fvuStarTokenLast[fuCurrDpbIdx]   = uNewToken;
            fvuStarDaqCmdLast[fuCurrDpbIdx]  = uNewDaqCmd;
            fvuStarTrigCmdLast[fuCurrDpbIdx] = uNewTrigCmd;
         } // if( fuCurrentMs < fuNbCoreMsPerTs )

         /// FIXME: for now do nothing with it!
/*
         Double_t dTot = 1.;
         Double_t dTime = fulGdpbTsFullLast * 6.25;
         if( 0. == fdFirstDigiTimeDif && 0. != fdLastDigiTime )
         {
            fdFirstDigiTimeDif = dTime - fdLastDigiTime;
            LOG(INFO) << "Reference fake digi time shift initialized to " << fdFirstDigiTimeDif
		      << ", default: " << fdTShiftRef
                      <<FairLogger::endl;
         } // if( 0. == fdFirstDigiTimeDif && 0. != fdLastDigiTime )

	 //         dTime -= fdFirstDigiTimeDif;
         // dTime += fdTShiftRef;

         LOG(DEBUG) << "Insert fake digi with time " << dTime << ", Tot " << dTot
                    << FairLogger::endl;
         fhRawTRefDig0->Fill( dTime - fdLastDigiTime);
         fhRawTRefDig1->Fill( dTime - fdLastDigiTime);

         fDigi = new CbmTofDigiExp(0x00005006, dTime, dTot); // fake start counter signal
         fBuffer->InsertData(fDigi);
*/
         break;
	  } // case 3
      default:
         LOG(ERROR) << "Unknown Star Trigger messageindex: " << iMsgIndex << FairLogger::endl;
  } // switch( iMsgIndex )
}
// -------------------------------------------------------------------------
void CbmMcbm2018UnpackerAlgoTof::ProcessEpSupprBuffer()
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
void CbmMcbm2018UnpackerAlgoTof::ProcessHit( gdpbv100::FullMessage mess )
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

   // Histograms filling
   if( kTRUE == fbMonitorMode )
   {
      fhRawTotCh[ fuCurrDpbIdx ]->Fill( uRemappedChannelNr, dHitTot);
      fhChCount[ fuCurrDpbIdx ] ->Fill( uChannelNr );
      fhChCountRemap[ fuCurrDpbIdx ] ->Fill( uRemappedChannelNr );
   } // if( kTRUE == fbMonitorMode )

/*
   if( fUnpackPar->GetNumberOfChannels() < uChanInSyst )
   {
      LOG(FATAL) << "Invalid mapping index " << uChanInSyst
                 << " VS " << fUnpackPar->GetNumberOfChannels()
                 <<", from " << fuCurrDpbIdx
                 <<", " << fuGet4Id
                 <<", " << uChannel
                 << FairLogger::endl;
      return;
   } // if( fUnpackPar->GetNumberOfChannels() < uChanUId )
*/
   if( fviRpcChUId.size() < uRemappedChannelNrInSys )
   {
      LOG(FATAL) << "Invalid mapping index " << uRemappedChannelNrInSys
                 << " VS " << fviRpcChUId.size()
                 << ", from GdpbNr " << fuCurrDpbIdx
                 << ", Get4 " << fuGet4Id
                 << ", Ch " << uChannel
                 << ", ChNr " << uChannelNr
                 << ", ChNrIF " << uChannelNrInFee
                 << ", FiS " << uFeeNrInSys
                 << FairLogger::endl;
      return;
   } // if( fviRpcChUId.size() < uRemappedChannelNrInSys )

   UInt_t uChanUId =  fviRpcChUId[ uRemappedChannelNrInSys ];
/*
   if( 5 == fviRpcType[uGbtxNrInSys] )
      LOG(INFO) << "T0 mapping index " << uRemappedChannelNrInSys
                 << " UID " << std::hex << std::setw(8) << uChanUId << std::dec
                 << ", from GdpbNr " << fuCurrDpbIdx
                 << ", Get4 " << fuGet4Id
                 << ", Ch " << uChannel
                 << ", ChNr " << uChannelNr
                 << ", ChNrIF " << uChannelNrInFee
                 << ", FiS " << uFeeNrInSys
                 << ", GBTx " << uGbtxNrInSys
                 << FairLogger::endl;
*/
   if( 0 == uChanUId )
      return;   // Hit not mapped to digi

   /// FIXME: Hardcoded value?!?
   if( (uChanUId & kuDetMask) != 0x00005006 )
      dHitTime += fdTShiftRef;

   LOG(DEBUG) << Form("Insert 0x%08x digi with time ", uChanUId ) << dHitTime << Form(", Tot %4.0f",dHitTot)
//              << " into buffer with " << fBuffer->GetSize() << " data from "
//              << Form("%11.1f to %11.1f ", fBuffer->GetTimeFirst(), fBuffer->GetTimeLast())
              << " at epoch " << mess.getExtendedEpoch()
              << FairLogger::endl;

   CbmTofDigiExp digi( uChanUId, dHitTime, dHitTot );

   fDigiVect.push_back( digi );
}
// -------------------------------------------------------------------------
void CbmMcbm2018UnpackerAlgoTof::ProcessSlCtrl( gdpbv100::FullMessage mess )
{
}
// -------------------------------------------------------------------------
void CbmMcbm2018UnpackerAlgoTof::ProcessSysMess( gdpbv100::FullMessage mess )
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
         LOG(INFO) << "GET4 synchronization pulse missing in gDPB " << fuCurrDpbIdx
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
void CbmMcbm2018UnpackerAlgoTof::ProcessError( gdpbv100::FullMessage mess )
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
void CbmMcbm2018UnpackerAlgoTof::ProcessPattern( gdpbv100::FullMessage mess )
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
               fhPatternEnable->Fill( 32 * usIndex + uBit, fuCurrDpbIdx );
               fvhGdpbPatternEnableEvo[ fuCurrDpbIdx ]->Fill( fulCurrentTsIndex, 32 * usIndex + uBit );
            } // if( ( uPattern >> uBit ) & 0x1 )
*/
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

Bool_t CbmMcbm2018UnpackerAlgoTof::CreateHistograms()
{
   std::string sFolder = "Tof_Raw_gDPB";

   LOG(INFO) << "create Histos for " << fuNrOfGdpbs <<" gDPBs "
	          << FairLogger::endl;

   fhRawTDigEvT0 = new TH1F( Form("Raw_TDig-EvT0"),
                             Form("Raw digi time difference to 1st digi ; time [ns]; cts"),
                             500, 0, 100.);

   fhRawTDigRef0 = new TH1F( Form("Raw_TDig-Ref0"),
                             Form("Raw digi time difference to Ref ; time [ns]; cts"),
                             6000, -10000, 50000);

   fhRawTDigRef = new TH1F( Form("Raw_TDig-Ref"),
                            Form("Raw digi time difference to Ref ; time [ns]; cts"),
                            6000, -1000, 5000);

   fhRawTRefDig0 = new TH1F( Form("Raw_TRef-Dig0"),
                             Form("Raw Ref time difference to last digi  ; time [ns]; cts"),
                             9999, -50000, 50000);

   fhRawTRefDig1 = new TH1F( Form("Raw_TRef-Dig1"),
                             Form("Raw Ref time difference to last digi  ; time [ns]; cts"),
                             9999, -5000, 5000);

   fhRawDigiLastDigi = new TH1F( Form("Raw_Digi-LastDigi"),
                                 Form("Raw Digi time difference to last digi  ; time [ns]; cts"),
                                 9999, -5000, 5000);

   /// Add pointers to the vector with all histo for access by steering class
   AddHistoToVector( fhRawTDigEvT0, sFolder );
   AddHistoToVector( fhRawTDigRef0, sFolder );
   AddHistoToVector( fhRawTDigRef, sFolder );
   AddHistoToVector( fhRawTRefDig0, sFolder );
   AddHistoToVector( fhRawTRefDig1, sFolder );
   AddHistoToVector( fhRawDigiLastDigi, sFolder );

   fhRawTotCh.resize( fuNrOfGdpbs );
   fhChCount.resize(  fuNrOfGdpbs );
   fhChCountRemap.resize(  fuNrOfGdpbs );
   fhChanCoinc.resize( fuNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
   {
      fhRawTotCh[ uGdpb ] = new TH2F( Form("Raw_Tot_gDPB_%02u", uGdpb),
                                      Form("Raw TOT gDPB %02u; channel; TOT [bin]", uGdpb),
                                      fuNrOfGet4PerGdpb*4, 0., fuNrOfGet4PerGdpb*4,
                                      256, 0., 256. );

      fhChCount[ uGdpb ] = new TH1I( Form("ChCount_gDPB_%02u", uGdpb),
                                     Form("Channel counts gDPB %02u; channel; Hits", uGdpb),
                                     fuNrOfChannelsPerGdpb, 0., fuNrOfChannelsPerGdpb );

      fhChCountRemap[ uGdpb ] = new TH1I( Form("ChCountRemap_gDPB_%02u", uGdpb),
                                     Form("Remapped channel counts gDPB %02u; Remapped channel; Hits", uGdpb),
                                     fuNrOfChannelsPerGdpb, 0., fuNrOfChannelsPerGdpb );

      fhChanCoinc[ uGdpb ] = new TH2F( Form("fhChanCoinc_%02u", uGdpb),
                                      Form("Channels Coincidence %02u; Left; Right", uGdpb),
                                      fuNrOfChannelsPerGdpb, 0., fuNrOfChannelsPerGdpb,
                                      fuNrOfChannelsPerGdpb, 0., fuNrOfChannelsPerGdpb );

      /// Add pointers to the vector with all histo for access by steering class
      AddHistoToVector( fhRawTotCh[ uGdpb ], sFolder );
      AddHistoToVector( fhChCount[ uGdpb ], sFolder );
      AddHistoToVector( fhChCountRemap[ uGdpb ], sFolder );
      AddHistoToVector( fhChanCoinc[ uGdpb ], sFolder );
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
/*
   /// Canvases
   Double_t w = 10;
   Double_t h = 10;

      /// Event building process summary and statistics
   fcEventBuildStats = new TCanvas( "cEvtBuildStats", "Event building statistics", w, h);
   if( kTRUE == fbDebugMonitorMode )
      fcEventBuildStats->Divide( 2, 3 );
      else fcEventBuildStats->Divide( 2, 2 );

   fcEventBuildStats->cd( 1 );
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogy();
   fhEventNbPerTs->Draw();

   fcEventBuildStats->cd( 2 );
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogy();
   fhEventSizeDistribution->Draw();

   fcEventBuildStats->cd( 3 );
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogy();
   fhEventSizeEvolution->Draw();

   fcEventBuildStats->cd( 4 );
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogy();
   fhEventNbEvolution->Draw();

   if( kTRUE == fbDebugMonitorMode )
   {
      fcEventBuildStats->cd( 5 );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogy();
      fhEventNbDistributionInTs->Draw();

      fcEventBuildStats->cd( 6 );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogy();
      fhEventSizeDistributionInTs->Draw();
   } // if( kTRUE == fbDebugMonitorMode )

   AddCanvasToVector( fcEventBuildStats, "canvases" );
*/
   return kTRUE;
}
Bool_t CbmMcbm2018UnpackerAlgoTof::FillHistograms()
{
/*
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
*/
   return kTRUE;
}
Bool_t CbmMcbm2018UnpackerAlgoTof::ResetHistograms()
{
/*
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      fvhHitsTimeToTriggerRaw[ uGdpb ]->Reset();
      fvhHitsTimeToTriggerSel[ uGdpb ]->Reset();

      if( kTRUE == fbDebugMonitorMode )
      {
         fvhHitsTimeToTriggerSelVsDaq[ uGdpb ]->Reset();
         fvhHitsTimeToTriggerSelVsTrig[ uGdpb ]->Reset();
         fvhTriggerDt[ uGdpb ]->Reset();
         fvhTriggerDistributionInTs[ uGdpb ]->Reset();
         fvhTriggerDistributionInMs[ uGdpb ]->Reset();
         fvhMessDistributionInMs[ uGdpb ]->Reset();
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
*/
   return kTRUE;
}
// -------------------------------------------------------------------------
