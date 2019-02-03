// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                  CbmMcbm2018UnpackerAlgoSts                       -----
// -----               Created 26.01.2019 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018UnpackerAlgoSts.h"

#include "CbmMcbm2018StsPar.h"
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
#include "TCanvas.h"

#include <iostream>
#include <stdint.h>
#include <iomanip>
#include <fstream>

// -------------------------------------------------------------------------
CbmMcbm2018UnpackerAlgoSts::CbmMcbm2018UnpackerAlgoSts() :
   CbmStar2019Algo(),
   /// From the class itself
   fbMonitorMode( kFALSE ),
   fbDebugMonitorMode( kFALSE ),
   fvbMaskedComponents(),
   fUnpackPar( nullptr ),
   fuNbModules( 0 ),
   fviModuleType(),
   fviModAddress(),
   fuNrOfDpbs( 0 ),
   fDpbIdIndexMap(),
   fvbCrobActiveFlag(),
   fuNbFebs( 0 ),
   fuNbStsXyters( 0 ),
   fviFebModuleIdx(),
   fviFebModuleSide(),
   fviFebType(),
   fvdFebAdcGain(),
   fvdFebAdcOffs(),
   fviFebAddress(),
   fdTimeOffsetNs( 0.0 ),
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
   fvulCurrentTsMsb(),
   fdStartTime( 0.0 ),
   fdStartTimeMsSz( 0.0 ),
   ftStartTimeUnix( std::chrono::steady_clock::now() ),
   fvmHitsInMs()
/*
   fvhHitsTimeToTriggerRaw(),
   fvhMessDistributionInMs(),
   fhEventNbPerTs( nullptr ),
   fcTimeToTrigRaw( nullptr )
*/
{
}
CbmMcbm2018UnpackerAlgoSts::~CbmMcbm2018UnpackerAlgoSts()
{
   /// Clear buffers
   fvmHitsInMs.clear();
/*
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fvmHitsInMs[ uDpb ].clear();
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
*/
}

// -------------------------------------------------------------------------
Bool_t CbmMcbm2018UnpackerAlgoSts::Init()
{
   LOG(INFO) << "Initializing STAR eTOF 2019 event builder algo" << FairLogger::endl;

   return kTRUE;
}
void CbmMcbm2018UnpackerAlgoSts::Reset()
{
}
void CbmMcbm2018UnpackerAlgoSts::Finish()
{
   /// Printout Goodbye message and stats

   /// Write Output histos

}

// -------------------------------------------------------------------------
Bool_t CbmMcbm2018UnpackerAlgoSts::InitContainers()
{
   LOG(INFO) << "Init parameter containers for CbmMcbm2018UnpackerAlgoSts"
             << FairLogger::endl;
   Bool_t initOK = ReInitContainers();

   return initOK;
}
Bool_t CbmMcbm2018UnpackerAlgoSts::ReInitContainers()
{
   LOG(INFO) << "**********************************************"
             << FairLogger::endl;
   LOG(INFO) << "ReInit parameter containers for CbmMcbm2018UnpackerAlgoSts"
             << FairLogger::endl;

   fUnpackPar = (CbmMcbm2018StsPar*)fParCList->FindObject("CbmMcbm2018StsPar");
   if( nullptr == fUnpackPar )
      return kFALSE;

   Bool_t initOK = InitParameters();

   return initOK;
}
TList* CbmMcbm2018UnpackerAlgoSts::GetParList()
{
   if( nullptr == fParCList )
      fParCList = new TList();
   fUnpackPar = new CbmMcbm2018StsPar("CbmMcbm2018StsPar");
   fParCList->Add(fUnpackPar);

   return fParCList;
}
Bool_t CbmMcbm2018UnpackerAlgoSts::InitParameters()
{
   fuNbModules   = fUnpackPar->GetNbOfModules();
   LOG(INFO) << "Nr. of STS Modules:    " << fuNbModules
             << FairLogger::endl;

   fviModuleType.resize( fuNbModules );
   fviModAddress.resize( fuNbModules );
   for( UInt_t uModIdx = 0; uModIdx < fuNbModules; ++uModIdx)
   {
      fviModuleType[ uModIdx ] = fUnpackPar->GetModuleType( uModIdx );
      fviModAddress[ uModIdx ] = fUnpackPar->GetModuleAddress( uModIdx );
      LOG(INFO) << "Module #" << std::setw(2) << uModIdx
                << " Type " << std::setw(4)  << fviModuleType[ uModIdx ]
                << " Address 0x" << std::setw(8) << std::hex <<fviModAddress[ uModIdx ]
                << std::dec
                << FairLogger::endl;
   } // for( UInt_t uModIdx = 0; uModIdx < fuNbModules; ++uModIdx)

   fuNrOfDpbs = fUnpackPar->GetNrOfDpbs();
   LOG(INFO) << "Nr. of STS DPBs:       " << fuNrOfDpbs
             << FairLogger::endl;

   fDpbIdIndexMap.clear();
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fDpbIdIndexMap[ fUnpackPar->GetDpbId( uDpb )  ] = uDpb;
      LOG(INFO) << "Eq. ID for DPB #" << std::setw(2) << uDpb << " = 0x"
                << std::setw(4) << std::hex << fUnpackPar->GetDpbId( uDpb )
                << std::dec
                << " => " << fDpbIdIndexMap[ fUnpackPar->GetDpbId( uDpb )  ]
                << FairLogger::endl;
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   fuNbFebs      = fUnpackPar->GetNrOfFebs();
   LOG(INFO) << "Nr. of FEBs:           " << fuNbFebs
             << FairLogger::endl;

   fuNbStsXyters = fUnpackPar->GetNrOfAsics();
   LOG(INFO) << "Nr. of StsXyter ASICs: " << fuNbStsXyters
             << FairLogger::endl;

   fvbCrobActiveFlag.resize( fuNrOfDpbs );
   fviFebModuleIdx.resize(   fuNrOfDpbs );
   fviFebModuleSide.resize(  fuNrOfDpbs );
   fviFebType.resize(        fuNrOfDpbs );
   fvdFebAdcGain.resize(     fuNrOfDpbs );
   fvdFebAdcOffs.resize(     fuNrOfDpbs );
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fvbCrobActiveFlag[ uDpb ].resize( fUnpackPar->GetNbCrobsPerDpb() );
      fviFebModuleIdx[ uDpb ].resize(   fUnpackPar->GetNbCrobsPerDpb() );
      fviFebModuleSide[ uDpb ].resize(  fUnpackPar->GetNbCrobsPerDpb() );
      fviFebType[ uDpb ].resize(        fUnpackPar->GetNbCrobsPerDpb() );
      fvdFebAdcGain[ uDpb ].resize(        fUnpackPar->GetNbCrobsPerDpb() );
      fvdFebAdcOffs[ uDpb ].resize(        fUnpackPar->GetNbCrobsPerDpb() );
      for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackPar->GetNbCrobsPerDpb(); ++uCrobIdx )
      {
         fvbCrobActiveFlag[ uDpb ][ uCrobIdx ] = fUnpackPar->IsCrobActive( uDpb, uCrobIdx );

         fviFebModuleIdx[ uDpb ][ uCrobIdx ].resize(   fUnpackPar->GetNbFebsPerCrob() );
         fviFebModuleSide[ uDpb ][ uCrobIdx ].resize(  fUnpackPar->GetNbFebsPerCrob() );
         fviFebType[ uDpb ][ uCrobIdx ].resize(        fUnpackPar->GetNbFebsPerCrob(), -1 );
         fvdFebAdcGain[ uDpb ][ uCrobIdx ].resize(     fUnpackPar->GetNbFebsPerCrob(), 0.0 );
         fvdFebAdcOffs[ uDpb ][ uCrobIdx ].resize(     fUnpackPar->GetNbFebsPerCrob(), 0.0 );
         for( UInt_t uFebIdx = 0; uFebIdx < fUnpackPar->GetNbFebsPerCrob(); ++ uFebIdx )
         {
            fviFebModuleIdx[ uDpb ][ uCrobIdx ][ uFebIdx ]  = fUnpackPar->GetFebModuleIdx( uDpb, uCrobIdx, uFebIdx );
            fviFebModuleSide[ uDpb ][ uCrobIdx ][ uFebIdx ] = fUnpackPar->GetFebModuleSide( uDpb, uCrobIdx, uFebIdx );
            fvdFebAdcGain[ uDpb ][ uCrobIdx ][ uFebIdx ]    = fUnpackPar->GetFebAdcGain( uDpb, uCrobIdx, uFebIdx );
            fvdFebAdcOffs[ uDpb ][ uCrobIdx ][ uFebIdx ]    = fUnpackPar->GetFebAdcOffset( uDpb, uCrobIdx, uFebIdx );

            if( 0 <= fviFebModuleSide[ uDpb ][ uCrobIdx ][ uFebIdx ] &&
                fviFebModuleIdx[ uDpb ][ uCrobIdx ][ uFebIdx ] < fuNbModules )
            {
               switch( fviModuleType[ fviFebModuleIdx[ uDpb ][ uCrobIdx ][ uFebIdx ] ] )
               {
                  case 0: // FEB-8-1 with ZIF connector on the right
                  {
                     // P side (0) has type A (0)
                     // N side (1) has type B (1)
                     fviFebType[ uDpb ][ uCrobIdx ][ uFebIdx ] = fviFebModuleSide[ uDpb ][ uCrobIdx ][ uFebIdx ];

                     ///! FIXME: 1) Geometry is using front/back while we are using P/N !!!!
                     ///!            => Assuming that front facing mdules have connectors on right side
                     ///!        2) No accessor/setter to change only the side field of an STS address
                     ///!            => hardcode the shift
                     fviFebAddress.push_back( fviModAddress[ fviFebModuleIdx[ uDpb ][ uCrobIdx ][ uFebIdx ] ]
                                              + ( fviFebModuleSide[ uDpb ][ uCrobIdx ][ uFebIdx ] << 25 ) );

                     break;
                  } // case 0: // FEB-8-1 with ZIF connector on the right
                  case 1: // FEB-8-1 with ZIF connector on the left
                  {
                     // P side (0) has type B (1)
                     // N side (1) has type A (0)
                     fviFebType[ uDpb ][ uCrobIdx ][ uFebIdx ] = !(fviFebModuleSide[ uDpb ][ uCrobIdx ][ uFebIdx ]);

                     ///! FIXME: 1) Geometry is using front/back while we are using P/N !!!!
                     ///!            => Assuming that front facing mdules have connectors on right side
                     ///!        2) No accessor/setter to change only the side field of an STS address
                     ///!            => hardcode the shift
                     fviFebAddress.push_back( fviModAddress[ fviFebModuleIdx[ uDpb ][ uCrobIdx ][ uFebIdx ] ]
                                              + ( (!fviFebModuleSide[ uDpb ][ uCrobIdx ][ uFebIdx ]) << 25 ) );
                     break;
                  } // case 1: // FEB-8-1 with ZIF connector on the left
                  default:
                     break;
               } // switch( fviModuleType[ fviFebModuleIdx[ uDpb ][ uCrobIdx ][ uFebIdx ] ] )
            } // FEB active and module index OK
         } // for( UInt_t uFebIdx = 0; uFebIdx < fUnpackPar->GetNbFebsPerCrob(); ++ uFebIdx )
      } // for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackPar->GetNbCrobsPerDpb(); ++uCrobIdx )
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      LOG(INFO) << Form( "DPB #%02u CROB Active ?:       ", uDpb);
      for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackPar->GetNbCrobsPerDpb(); ++uCrobIdx )
      {
         LOG(INFO) <<  fvbCrobActiveFlag[ uDpb ][ uCrobIdx ] << " ";
      } // for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackPar->GetNbCrobsPerDpb(); ++uCrobIdx )
      LOG(INFO) << FairLogger::endl;
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackPar->GetNbCrobsPerDpb(); ++uCrobIdx )
      {
         LOG(INFO) << Form( "DPB #%02u CROB #%u:       ", uDpb, uCrobIdx)
                   << FairLogger::endl;
         for( UInt_t uFebIdx = 0; uFebIdx < fUnpackPar->GetNbFebsPerCrob(); ++ uFebIdx )
            if( 0 <= fviFebModuleIdx[ uDpb ][ uCrobIdx ][ uFebIdx ] )
         {
            LOG(INFO) << Form( "      FEB #%02u: Mod. Idx = %03d Side %c (%2d) Type %c (%2d) ADC gain %4.0f e- ADC Offs %5.0f e-",
                                 uFebIdx,
                                 fviFebModuleIdx[ uDpb ][ uCrobIdx ][ uFebIdx ],
                                 1 == fviFebModuleSide[ uDpb ][ uCrobIdx ][ uFebIdx ] ? 'N': 'P',
                                 fviFebModuleSide[ uDpb ][ uCrobIdx ][ uFebIdx ],
                                 1 == fviFebType[ uDpb ][ uCrobIdx ][ uFebIdx ] ? 'B' : 'A',
                                 fviFebType[ uDpb ][ uCrobIdx ][ uFebIdx ],
                                 fvdFebAdcGain[ uDpb ][ uCrobIdx ][ uFebIdx ],
                                 fvdFebAdcOffs[ uDpb ][ uCrobIdx ][ uFebIdx ]
                              )
                      << FairLogger::endl;
         } // for( UInt_t uFebIdx = 0; uFebIdx < fUnpackPar->GetNbFebsPerCrob(); ++ uFebIdx )
      } // for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackPar->GetNbCrobsPerDpb(); ++uCrobIdx )
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   // Internal status initialization
   fvulCurrentTsMsb.resize( fuNrOfDpbs );
   fvuCurrentTsMsbCycle.resize( fuNrOfDpbs );
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fvulCurrentTsMsb[uDpb]     = 0;
      fvuCurrentTsMsbCycle[uDpb] = 0;
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

	return kTRUE;
}
// -------------------------------------------------------------------------

void CbmMcbm2018UnpackerAlgoSts::AddMsComponentToList( size_t component, UShort_t usDetectorId )
{
   /// Check for duplicates and ignore if it is the case
   for( UInt_t uCompIdx = 0; uCompIdx < fvMsComponentsList.size(); ++uCompIdx )
      if( component == fvMsComponentsList[ uCompIdx ] )
         return;

   /// Add to list
   fvMsComponentsList.push_back( component );

   LOG(INFO) << "CbmMcbm2018UnpackerAlgoSts::AddMsComponentToList => Component "
             << component << " with detector ID 0x"
             << std::hex << usDetectorId << std::dec << " added to list"
             << FairLogger::endl;

}
// -------------------------------------------------------------------------

Bool_t CbmMcbm2018UnpackerAlgoSts::ProcessTs( const fles::Timeslice& ts )
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

         fDigiVect.push_back( CbmStsDigi( fviFebAddress[ uFebIdx ], uChanInFeb, ulTimeInNs, itHitIn->GetAdc() ) );
      } // for( auto itHitIn = fvmHitsInMs.begin(); itHitIn < fvmHitsInMs.end(); ++itHitIn )

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

Bool_t CbmMcbm2018UnpackerAlgoSts::ProcessMs( const fles::Timeslice& ts, size_t uMsCompIdx, size_t uMsIdx )
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
   auto it = fDpbIdIndexMap.find( fuCurrDpbId );
   if( it == fDpbIdIndexMap.end() )
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
         LOG(WARNING) << "Could not find the sDPB index for AFCK id 0x"
                   << std::hex << fuCurrDpbId << std::dec
                   << " in timeslice " << fulCurrentTsIdx
                   << " in microslice " << uMsIdx
                   << " component " << uMsCompIdx
                   << "\n"
                   << "If valid this index has to be added in the STS parameter file in the DbpIdArray field"
                   << FairLogger::endl;
         fvbMaskedComponents[ uMsCompIdx ] = kTRUE;
      } // if( kFALSE == fvbMaskedComponents[ uMsComp ] )
         else return kTRUE;

      return kFALSE;
   } // if( it == fDpbIdIndexMap.end() )
      else fuCurrDpbIdx = fDpbIdIndexMap[ fuCurrDpbId ];

   /** Check the current TS_MSb cycle and correct it if wrong **/
   UInt_t uTsMsbCycleHeader = std::floor( fulCurrentMsIdx /
                                          ( stsxyter::kuTsCycleNbBins * stsxyter::kdClockCycleNs ) );

   if( 0 == uMsIdx )
   {
      fvuCurrentTsMsbCycle[ fuCurrDpbIdx ] = uTsMsbCycleHeader;
      fvulCurrentTsMsb[fuCurrDpbIdx] = 0;
   } // if( 0 == uMsIdx )
   else if( uTsMsbCycleHeader != fvuCurrentTsMsbCycle[ fuCurrDpbIdx ] &&
            4194303 != fvulCurrentTsMsb[fuCurrDpbIdx] )
   {
      LOG(WARNING) << "TS MSB cycle from MS header does not match current cycle from data "
                    << "for TS " << std::setw( 12 ) << fulCurrentTsIdx
                    << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                    << " MsInTs " << std::setw( 3 ) << uMsIdx
                    << " ====> " << fvuCurrentTsMsbCycle[ fuCurrDpbIdx ]
                    << " VS " << uTsMsbCycleHeader
                    << FairLogger::endl;
      fvuCurrentTsMsbCycle[ fuCurrDpbIdx ] = uTsMsbCycleHeader;
   }

   // If not integer number of message in input buffer, print warning/error
   if( 0 != ( uSize % kuBytesPerMessage ) )
      LOG(ERROR) << "The input microslice buffer does NOT "
                 << "contain only complete nDPB messages!"
                 << FairLogger::endl;

   // Compute the number of complete messages in the input microslice buffer
   uint32_t uNbMessages = ( uSize - ( uSize % kuBytesPerMessage ) )
                          / kuBytesPerMessage;

   // Prepare variables for the loop on contents
   const uint32_t* pInBuff = reinterpret_cast<const uint32_t*>( msContent );
   for( uint32_t uIdx = 0; uIdx < uNbMessages; uIdx++ )
   {
      // Fill message
      uint32_t ulData = static_cast<uint32_t>( pInBuff[uIdx] );

      stsxyter::Message mess( static_cast< uint32_t >( ulData & 0xFFFFFFFF ) );

      /// Get message type
      stsxyter::MessType typeMess = mess.GetMessType();
      fmMsgCounter[ typeMess ] ++;
//      fhStsMessType->Fill( static_cast< uint16_t > (typeMess) );
//      fhStsMessTypePerDpb->Fill( fuCurrDpbIdx, static_cast< uint16_t > (typeMess) );

      switch( typeMess )
      {
         case stsxyter::MessType::Hit :
         {
            // Extract the eLink and Asic indices => Should GO IN the fill method now that obly hits are link/asic specific!
            UShort_t usElinkIdx = mess.GetLinkIndex();
//            fhStsMessTypePerElink->Fill( usElinkIdx, static_cast< uint16_t > (typeMess) );
//            fhStsHitsElinkPerDpb->Fill( fuCurrDpbIdx, usElinkIdx );

            UInt_t   uCrobIdx   = usElinkIdx / fUnpackPar->GetNbElinkPerCrob();
            Int_t   uFebIdx    = fUnpackPar->ElinkIdxToFebIdx( usElinkIdx );

            if( -1 == uFebIdx )
            {
               LOG(WARNING) << "CbmMcbm2018UnpackerAlgoSts::DoUnpack => "
                         << "Wrong elink Idx! Elink raw "
                         << Form("%d remap %d", usElinkIdx, uFebIdx )
                         << FairLogger::endl;
               continue;
            } // if( -1 == uFebIdx )

            UInt_t   uAsicIdx   = ( fuCurrDpbIdx * fUnpackPar->GetNbCrobsPerDpb() + uCrobIdx
                                  ) * fUnpackPar->GetNbAsicsPerCrob()
                                 + fUnpackPar->ElinkIdxToAsicIdx( 1 == fviFebType[ fuCurrDpbIdx ][ uCrobIdx ][ uFebIdx ],
                                                                     usElinkIdx );

            ProcessHitInfo( mess, usElinkIdx, uAsicIdx, uMsIdx );
            break;
         } // case stsxyter::MessType::Hit :
         case stsxyter::MessType::TsMsb :
         {
//            fhStsMessTypePerElink->Fill( fuCurrDpbIdx * fUnpackPar->GetNbElinkPerDpb(), static_cast< uint16_t > (typeMess) );

            ProcessTsMsbInfo( mess, uIdx, uMsIdx );
            break;
         } // case stsxyter::MessType::TsMsb :
         case stsxyter::MessType::Epoch :
         {
//            fhStsMessTypePerElink->Fill( fuCurrDpbIdx * fUnpackPar->GetNbElinkPerDpb(), static_cast< uint16_t > (typeMess) );

            // The first message in the TS is a special ones: EPOCH
            ProcessEpochInfo( mess );

            if( 0 < uIdx )
               LOG(INFO) << "CbmMcbm2018UnpackerAlgoSts::DoUnpack => "
                         << "EPOCH message at unexpected position in MS: message "
                         << uIdx << " VS message 0 expected!"
                         << FairLogger::endl;
            break;
         } // case stsxyter::MessType::TsMsb :
         case stsxyter::MessType::Status :
         {
//            UShort_t usElinkIdx    = mess.GetStatusLink();
//            fhStsMessTypePerElink->Fill( usElinkIdx, static_cast< uint16_t > (typeMess) );
            ProcessStatusInfo( mess );
            break;
         } // case stsxyter::MessType::Status
         case stsxyter::MessType::Empty :
         {
//            fhStsMessTypePerElink->Fill( fuCurrDpbIdx * fUnpackPar->GetNbElinkPerDpb(), static_cast< uint16_t > (typeMess) );
//                   FillTsMsbInfo( mess );
            break;
         } // case stsxyter::MessType::Empty :
         case stsxyter::MessType::Dummy :
         {
//            fhStsMessTypePerElink->Fill( fuCurrDpbIdx * fUnpackPar->GetNbElinkPerDpb(), static_cast< uint16_t > (typeMess) );
            break;
         } // case stsxyter::MessType::Dummy / ReadDataAck / Ack :
         default:
         {
            LOG(FATAL) << "CbmMcbm2018UnpackerAlgoSts::DoUnpack => "
                       << "Unknown message type, should never happen, stopping here! Type found was: "
                       << static_cast< int >( typeMess )
                       << FairLogger::endl;
         }
      } // switch( typeMess )
   } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)

   return kTRUE;
}

// -------------------------------------------------------------------------
void CbmMcbm2018UnpackerAlgoSts::ProcessHitInfo( stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx )
{
   UShort_t usChan   = mess.GetHitChannel();
   UShort_t usRawAdc = mess.GetHitAdc();
//   UShort_t usFullTs = mess.GetHitTimeFull();
   UShort_t usTsOver = mess.GetHitTimeOver();
   UShort_t usRawTs  = mess.GetHitTime();

   /// Cheat needed only for modules with FEB at bottom of module or the Test module
//   usChan = 127 - usChan;

/*
   fhStsChanCntRaw[  uAsicIdx ]->Fill( usChan );
   fhStsChanAdcRaw[  uAsicIdx ]->Fill( usChan, usRawAdc );
   fhStsChanAdcRawProf[  uAsicIdx ]->Fill( usChan, usRawAdc );
   fhStsChanRawTs[   uAsicIdx ]->Fill( usChan, usRawTs );
   fhStsChanMissEvt[ uAsicIdx ]->Fill( usChan, mess.IsHitMissedEvts() );
*/
   UInt_t uCrobIdx   = usElinkIdx / fUnpackPar->GetNbElinkPerCrob();
   UInt_t uFebIdx    = uAsicIdx / fUnpackPar->GetNbAsicsPerFeb();
   UInt_t uAsicInFeb = uAsicIdx % fUnpackPar->GetNbAsicsPerFeb();
   UInt_t uChanInFeb = usChan + fUnpackPar->GetNbChanPerAsic() * (uAsicIdx % fUnpackPar->GetNbAsicsPerFeb());

   Double_t dCalAdc = fvdFebAdcOffs[ fuCurrDpbIdx ][ uCrobIdx ][ uFebIdx ]
                     + (usRawAdc - 1)* fvdFebAdcGain[ fuCurrDpbIdx ][ uCrobIdx ][ uFebIdx ];
/*
   fhStsFebChanCntRaw[  uFebIdx ]->Fill( uChanInFeb );
   fhStsFebChanAdcRaw[  uFebIdx ]->Fill( uChanInFeb, usRawAdc );
   fhStsFebChanAdcRawProf[  uFebIdx ]->Fill( uChanInFeb, usRawAdc );
   fhStsFebChanAdcCal[  uFebIdx ]->Fill(     uChanInFeb, dCalAdc );
   fhStsFebChanAdcCalProf[  uFebIdx ]->Fill( uChanInFeb, dCalAdc );
   fhStsFebChanRawTs[   uFebIdx ]->Fill( usChan, usRawTs );
   fhStsFebChanMissEvt[ uFebIdx ]->Fill( usChan, mess.IsHitMissedEvts() );
*/
   // Compute the Full time stamp
      // Use TS w/o overlap bits as they will anyway come from the TS_MSB
   Long64_t ulHitTime = usRawTs;
   ulHitTime +=
               static_cast<ULong64_t>( stsxyter::kuHitNbTsBins )
             * static_cast<ULong64_t>( fvulCurrentTsMsb[fuCurrDpbIdx])
             + static_cast<ULong64_t>( stsxyter::kuTsCycleNbBins )
             * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx] );

   // Convert the Hit time in bins to Hit time in ns
   Double_t dHitTimeNs = ulHitTime * stsxyter::kdClockCycleNs;

   fvmHitsInMs.push_back( stsxyter::FinalHit( ulHitTime, usRawAdc, uAsicIdx, usChan, fuCurrDpbIdx, uCrobIdx ) );

   // Check Starting point of histos with time as X axis
   if( -1 == fdStartTime )
      fdStartTime = dHitTimeNs;
/*
   // Fill histos with time as X axis
   Double_t dTimeSinceStartSec = (dHitTimeNs - fdStartTime)* 1e-9;
   Double_t dTimeSinceStartMin = dTimeSinceStartSec / 60.0;

   fviFebCountsSinceLastRateUpdate[uFebIdx]++;
   fvdFebChanCountsSinceLastRateUpdate[uFebIdx][uChanInFeb] += 1;

   fhStsFebChanHitRateEvo[ uFebIdx ]->Fill( dTimeSinceStartSec, uChanInFeb );
   fhStsFebAsicHitRateEvo[ uFebIdx ]->Fill( dTimeSinceStartSec, uAsicInFeb );
   fhStsFebHitRateEvo[ uFebIdx ]->Fill(     dTimeSinceStartSec );
   fhStsFebChanHitRateEvoLong[ uFebIdx ]->Fill( dTimeSinceStartMin, uChanInFeb, 1.0/60.0 );
   fhStsFebAsicHitRateEvoLong[ uFebIdx ]->Fill( dTimeSinceStartMin, uAsicInFeb,   1.0/60.0 );
   fhStsFebHitRateEvoLong[ uFebIdx ]->Fill(     dTimeSinceStartMin,             1.0/60.0 );
   if( mess.IsHitMissedEvts() )
   {
      fhStsFebChanMissEvtEvo[ uFebIdx ]->Fill( dTimeSinceStartSec, uChanInFeb );
      fhStsFebAsicMissEvtEvo[ uFebIdx ]->Fill( dTimeSinceStartSec, uAsicInFeb );
      fhStsFebMissEvtEvo[ uFebIdx ]->Fill(     dTimeSinceStartSec );
   } // if( mess.IsHitMissedEvts() )
*/
/*
   if( kTRUE == fbLongHistoEnable )
   {
      std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();
      Double_t dUnixTimeInRun = std::chrono::duration_cast< std::chrono::seconds >(tNow - ftStartTimeUnix).count();
      fhFebRateEvoLong[ uAsicIdx ]->Fill( dUnixTimeInRun , 1.0 / fuLongHistoBinSizeSec );
      fhFebChRateEvoLong[ uAsicIdx ]->Fill( dUnixTimeInRun , usChan, 1.0 / fuLongHistoBinSizeSec );
   } // if( kTRUE == fbLongHistoEnable )
*/

}

void CbmMcbm2018UnpackerAlgoSts::ProcessTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx, UInt_t uMsIdx )
{
   UInt_t uVal    = mess.GetTsMsbVal();

/*
   if( (uVal != fvulCurrentTsMsb[fuCurrDpbIdx] + 1) && 0 < uVal  &&
       !( 1 == uMessIdx && usVal == fvulCurrentTsMsb[fuCurrDpbIdx] ) ) // 1st TS_MSB in MS is always a repeat of the last one in previous MS!
   {
      LOG(INFO) << "TS MSB not increasing by 1!  TS " << std::setw( 12 ) << fulCurrentTsIdx
                << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                << " MsInTs " << std::setw( 3 ) << uMsIdx
                << " DPB " << std::setw( 2 ) << fuCurrDpbIdx
                << " Mess " << std::setw( 5 ) << uMessIdx
                << " Old TsMsb " << std::setw( 5 ) << fvulCurrentTsMsb[fuCurrDpbIdx]
                << " new TsMsb " << std::setw( 5 ) << uVal
                << " Diff " << std::setw( 5 ) << uVal - fvulCurrentTsMsb[fuCurrDpbIdx]
                << " Old MsbCy " << std::setw( 5 ) << fvuCurrentTsMsbCycle[fuCurrDpbIdx]
                << FairLogger::endl;
   } // if( (uVal != fvulCurrentTsMsb[fuCurrDpbIdx] + 1) && 0 < uVal )
*/

   // Update Status counters
   if( uVal < fvulCurrentTsMsb[fuCurrDpbIdx] )
   {

      LOG(INFO) << " TS " << std::setw( 12 ) << fulCurrentTsIdx
                << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                << " MS Idx " << std::setw( 4 ) << uMsIdx
                << " Msg Idx " << std::setw( 5 ) << uMessIdx
                << " DPB " << std::setw( 2 ) << fuCurrDpbIdx
                << " Old TsMsb " << std::setw( 5 ) << fvulCurrentTsMsb[fuCurrDpbIdx]
                << " Old MsbCy " << std::setw( 5 ) << fvuCurrentTsMsbCycle[fuCurrDpbIdx]
                << " new TsMsb " << std::setw ( 5 ) << uVal
                << FairLogger::endl;

      fvuCurrentTsMsbCycle[fuCurrDpbIdx] ++;
   } // if( uVal < fvulCurrentTsMsb[fuCurrDpbIdx] )
   if( uVal != fvulCurrentTsMsb[fuCurrDpbIdx] + 1 &&
       0 != uVal && 4194303 != fvulCurrentTsMsb[fuCurrDpbIdx] &&
       1 != uMessIdx )
   {
      LOG(INFO) << "TS MSb Jump in "
                << " TS " << std::setw( 12 ) << fulCurrentTsIdx
                << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                << " MS Idx " << std::setw( 4 ) << uMsIdx
                << " Msg Idx " << std::setw( 5 ) << uMessIdx
                << " DPB " << std::setw( 2 ) << fuCurrDpbIdx
                << " => Old TsMsb " << std::setw( 5 ) << fvulCurrentTsMsb[fuCurrDpbIdx]
                << " new TsMsb " << std::setw ( 5 ) << uVal
                << FairLogger::endl;
   } // if( uVal + 1 != fvulCurrentTsMsb[fuCurrDpbIdx] && 4194303 != uVal && 0 != fvulCurrentTsMsb[fuCurrDpbIdx] )
   fvulCurrentTsMsb[fuCurrDpbIdx] = uVal;
/*
   if( 1 < uMessIdx )
   {
      fhStsDpbRawTsMsb->Fill( fuCurrDpbIdx,      fvulCurrentTsMsb[fuCurrDpbIdx] );
      fhStsDpbRawTsMsbSx->Fill( fuCurrDpbIdx,  ( fvulCurrentTsMsb[fuCurrDpbIdx] & 0x1F ) );
      fhStsDpbRawTsMsbDpb->Fill( fuCurrDpbIdx, ( fvulCurrentTsMsb[fuCurrDpbIdx] >> 5 ) );
   } // if( 0 < uMessIdx )
*/
//   fhStsAsicTsMsb->Fill( fvulCurrentTsMsb[fuCurrDpbIdx], uAsicIdx );

   ULong64_t ulNewTsMsbTime =  static_cast< ULong64_t >( stsxyter::kuHitNbTsBins )
                             * static_cast< ULong64_t >( fvulCurrentTsMsb[fuCurrDpbIdx])
                             + static_cast< ULong64_t >( stsxyter::kuTsCycleNbBins )
                             * static_cast< ULong64_t >( fvuCurrentTsMsbCycle[fuCurrDpbIdx] );
}

void CbmMcbm2018UnpackerAlgoSts::ProcessEpochInfo( stsxyter::Message mess )
{
   UInt_t uVal    = mess.GetEpochVal();
   UInt_t uCurrentCycle = uVal % stsxyter::kuTsCycleNbBins;

/*
   // Update Status counters
   if( usVal < fvulCurrentTsMsb[fuCurrDpbIdx] )
      fvuCurrentTsMsbCycle[fuCurrDpbIdx] ++;
   fvulCurrentTsMsb[fuCurrDpbIdx] = usVal;

//   fhStsAsicTsMsb->Fill( fvulCurrentTsMsb[fuCurrDpbIdx], uAsicIdx );
*/
}

void CbmMcbm2018UnpackerAlgoSts::ProcessStatusInfo( stsxyter::Message mess )
{
/*
   UInt_t   uCrobIdx   = usElinkIdx / fUnpackPar->GetNbElinkPerCrob();
   Int_t   uFebIdx    = fUnpackPar->ElinkIdxToFebIdx( usElinkIdx );
   UInt_t   uAsicIdx   = ( fuCurrDpbIdx * fUnpackPar->GetNbCrobsPerDpb() + uCrobIdx
                         ) * fUnpackPar->GetNbAsicsPerCrob()
                        + fUnpackPar->ElinkIdxToAsicIdx( 1 == fviFebType[ fuCurrDpbIdx ][ uCrobIdx ][ uFebIdx ],
                                                            usElinkIdx );

   UShort_t usStatusField = mess.GetStatusStatus();

   fhPulserStatusMessType->Fill( uAsicIdx, usStatusField );
   /// Always print status messages... or not?
   if( fbPrintMessages )
   {
      std::cout << Form("DPB %2u TS %12u MS %12u mess %5u ", fuCurrDpbIdx, fulCurrentTsIdx, fulCurrentMsIdx, uIdx );
      mess.PrintMess( std::cout, fPrintMessCtrl );
   } // if( fbPrintMessages )
*/
}

// -------------------------------------------------------------------------

// -------------------------------------------------------------------------

Bool_t CbmMcbm2018UnpackerAlgoSts::CreateHistograms()
{
/*
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
            Form( "hTriggerDtSect%2u", uSector ),
            Form( "Trigger time difference between sector %2u and the first sector, full events only; Ttrigg%2u - TtriggRef [Clk]; events []",
                  uSector, uSector ),
            200, -100, 100  ) );

         /// FIXME: hardcoded nb of MS in TS (include overlap)
         /// as this number is known only later when 1st TS is received
         UInt_t uNbBinsInTs = fdMsSizeInNs * 111 / 1000. / 10.;
         UInt_t uNbBinsInMs = fdMsSizeInNs * 20  / 1000. / 10.;

         fvhTriggerDistributionInTs.push_back( new TH1I( Form( "hTriggerDistInTsSect%2u", uSector ),
             Form( "Trigger distribution inside TS in sector %2u; Time in TS [us]; Trigger [];", uSector ),
             uNbBinsInTs, -0.5 - fdMsSizeInNs * 10 / 1000., fdMsSizeInNs * 101 / 1000. - 0.5 ) );

         fvhTriggerDistributionInMs.push_back( new TH1I( Form( "hTriggerDistInMsSect%2u", uSector ),
             Form( "Trigger distribution inside MS in sector %2u; Time in MS [us]; Trigger [];", uSector ),
             uNbBinsInMs, -0.5 - fdMsSizeInNs * 10 / 1000., fdMsSizeInNs * 10 / 1000. - 0.5 ) );

         fvhMessDistributionInMs.push_back( new TH1I( Form( "hMessDistInMsSect%2u", uSector ),
             Form( "Messages distribution inside MS in sector %2u; Time in MS [us]; Trigger [];", uSector ),
             uNbBinsInMs, -0.5 - fdMsSizeInNs * 10 / 1000., fdMsSizeInNs * 10 / 1000. - 0.5 ) );

         /// Add pointers to the vector with all histo for access by steering class
         AddHistoToVector( fvhHitsTimeToTriggerSelVsDaq[ uGdpb ],  sFolder );
         AddHistoToVector( fvhHitsTimeToTriggerSelVsTrig[ uGdpb ], sFolder );
         AddHistoToVector( fvhTriggerDt[ uGdpb ],                  sFolder );
         AddHistoToVector( fvhTriggerDistributionInTs[ uGdpb ],    sFolder );
         AddHistoToVector( fvhTriggerDistributionInMs[ uGdpb ],    sFolder );
         AddHistoToVector( fvhMessDistributionInMs[ uGdpb ],       sFolder );
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
      AddHistoToVector( fhMissingTriggersEvolution,  "eventbuilder" );
   } // if( kTRUE == fbDebugMonitorMode )

   /// Canvases
   Double_t w = 10;
   Double_t h = 10;

      /// Raw Time to trig for all sectors
   fcTimeToTrigRaw = new TCanvas( "cTimeToTrigRaw", "Raw Time to trig for all sectors", w, h);
   fcTimeToTrigRaw->Divide( 2, fuNrOfGdpbs / 2 );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      fcTimeToTrigRaw->cd( 1 + uGdpb );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogy();
      fvhHitsTimeToTriggerRaw[ uGdpb ]->Draw();
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

      /// Selected Time to trig for all sectors
   fcTimeToTrigSel = new TCanvas( "cTimeToTrigSel", "Selected Time to trig for all sectors", w, h);
   fcTimeToTrigSel->Divide( 2, fuNrOfGdpbs / 2 );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      fcTimeToTrigSel->cd( 1 + uGdpb );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogy();
      fvhHitsTimeToTriggerSel[ uGdpb ]->Draw();
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

   if( kTRUE == fbDebugMonitorMode )
   {
      /// Trigger time to MS start for all sectors
      fcTrigDistMs = new TCanvas( "cTrigDistMs", "Trigger time to MS start for all sectors", w, h);
      fcTrigDistMs->Divide( 2, fuNrOfGdpbs / 2 );
      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      {
         fcTrigDistMs->cd( 1 + uGdpb );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogy();
         fvhTriggerDistributionInMs[ uGdpb ]->Draw();
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

      /// Message time to MS start for all sectors
      fcMessDistMs = new TCanvas( "cMessDistMs", "Message time to MS start for all sectors", w, h);
      fcMessDistMs->Divide( 2, fuNrOfGdpbs / 2 );
      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      {
         fcMessDistMs->cd( 1 + uGdpb );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogy();
         fvhMessDistributionInMs[ uGdpb ]->Draw();
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   } // if( kTRUE == fbDebugMonitorMode )

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
Bool_t CbmMcbm2018UnpackerAlgoSts::FillHistograms()
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
Bool_t CbmMcbm2018UnpackerAlgoSts::ResetHistograms()
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
