// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                   CbmMcbm2018MuchUnpacker                        -----
// -----                Created 11/05/18  by P.-A. Loizeau                 -----
// -----                Modified 11/05/18  by Ajit kumar                  -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018MuchUnpacker.h"

// Data

// CbmRoot
#include "CbmMcbm2018MuchPar.h"
#include "CbmHistManager.h"
#include "CbmTbDaqBuffer.h"
#include "CbmMuchAddress.h"

// FairRoot
#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRunOnline.h"

// Root
#include "TClonesArray.h"
#include "TString.h"
#include "TRandom.h"
#include "THttpServer.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TMath.h"

// C++11
#include <bitset>

// C/C++
#include <iostream>
#include <stdint.h>
#include <iomanip>
class CbmMuchAddress;
using std::hex;
using std::dec;
using namespace std;


CbmMcbm2018MuchUnpacker::CbmMcbm2018MuchUnpacker() :
   CbmMcbmUnpack(),
   fbMuchMode(kFALSE),
   fvMsComponentsList(),
   fuNbCoreMsPerTs(0),
   fuNbOverMsPerTs(0),
   fbIgnoreOverlapMs(kFALSE),
   fUnpackParMuch(NULL),
   fuNrOfDpbs(0),
   fDpbIdIndexMap(),
   fvbCrobActiveFlag(),
   fuNbFebs(0),
   fuNbStsXyters(0),
   fbPrintMessages( kFALSE ),
   fPrintMessCtrl( stsxyter::MessagePrintMask::msg_print_Human ),
   fulCurrentTsIdx( 0 ),
   fulCurrentMsIdx( 0 ),
   fmMsgCounter(),
   fuCurrentEquipmentId(0),
   fuCurrDpbId(0),
   fuCurrDpbIdx(0),
   fiRunStartDateTimeSec(-1),
   fiBinSizeDatePlots(-1),
   fvulCurrentTsMsb(),
   fvuCurrentTsMsbCycle(),
   fvuInitialHeaderDone(),
   fvuInitialTsMsbCycleHeader(),
   fvuElinkLastTsHit(),
   fvulChanLastHitTime(),
   fvdChanLastHitTime(),
   fvdPrevMsTime(),
   fvdMsTime(),
   fvuChanNbHitsInMs(),
   fvdChanLastHitTimeInMs(),
   fvusChanLastHitAdcInMs(),
   fdStartTime(-1.0),
   fdStartTimeMsSz(-1.0),
   ftStartTimeUnix( std::chrono::steady_clock::now() ),
   fvmHitsInMs(),
   fvmAsicHitsInMs(),
   fvmFebHitsInMs(),
   fuMaxNbMicroslices(100),
   fiTimeIntervalRateUpdate( 10 ),
   fviFebTimeSecLastRateUpdate(),
   fviFebCountsSinceLastRateUpdate(),
   fvdFebChanCountsSinceLastRateUpdate(),
   fMuchRaw(new TClonesArray("CbmNxyterRawMessageForMUCH", 10)),
   fMuchDigi(new TClonesArray("CbmMuchBeamTimeDigi", 10)),
   fHM(new CbmHistManager()),
   fDigi(NULL),
   fBuffer(CbmTbDaqBuffer::Instance()),
   fCreateRawMessage(kFALSE),
   fcMsSizeAll(NULL)
{
}

CbmMcbm2018MuchUnpacker::~CbmMcbm2018MuchUnpacker()
{
}

Bool_t CbmMcbm2018MuchUnpacker::Init()
{
  LOG(INFO) << "Initializing flib StsXyter unpacker for MUCH" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }
  if (fCreateRawMessage) {
    ioman->Register("MuchRawMessage", "MUCH raw data", fMuchRaw, kTRUE);
  }
  ioman->Register("MuchDigi", "Stsxyter based Much digi", fMuchDigi, kTRUE);

  return kTRUE;
}

void CbmMcbm2018MuchUnpacker::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
         << FairLogger::endl;
   fUnpackParMuch = (CbmMcbm2018MuchPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmMcbm2018MuchPar"));
}


Bool_t CbmMcbm2018MuchUnpacker::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
         << FairLogger::endl;

   Bool_t bInit = InitMuchParameters();
   if( kTRUE == bInit )
      //CreateHistograms();

   return bInit;
}

Bool_t CbmMcbm2018MuchUnpacker::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
             << FairLogger::endl;

   return InitMuchParameters();
}

Bool_t CbmMcbm2018MuchUnpacker::InitMuchParameters()
{

   fuNrOfDpbs = fUnpackParMuch->GetNrOfDpbs();
   LOG(INFO) << "Nr. of MUCH DPBs:       " << fuNrOfDpbs
             << FairLogger::endl;

   fDpbIdIndexMap.clear();
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fDpbIdIndexMap[ fUnpackParMuch->GetDpbId( uDpb )  ] = uDpb;
      LOG(INFO) << "Eq. ID for DPB #" << std::setw(2) << uDpb << " = 0x"
                << std::setw(4) << std::hex << fUnpackParMuch->GetDpbId( uDpb )
                << std::dec
                << " => " << fDpbIdIndexMap[ fUnpackParMuch->GetDpbId( uDpb )  ]
                << FairLogger::endl;
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   fuNbFebs      = fUnpackParMuch->GetNrOfFebs();
   LOG(INFO) << "Nr. of FEBs:           " << fuNbFebs
             << FairLogger::endl;

   fuNbStsXyters = fUnpackParMuch->GetNrOfAsics();
   LOG(INFO) << "Nr. of StsXyter ASICs: " << fuNbStsXyters
             << FairLogger::endl;

   fvbCrobActiveFlag.resize( fuNrOfDpbs );
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fvbCrobActiveFlag[ uDpb ].resize( fUnpackParMuch->GetNbCrobsPerDpb() );
      for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackParMuch->GetNbCrobsPerDpb(); ++uCrobIdx )
      {
         fvbCrobActiveFlag[ uDpb ][ uCrobIdx ] = fUnpackParMuch->IsCrobActive( uDpb, uCrobIdx );
      } // for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackParMuch->GetNbCrobsPerDpb(); ++uCrobIdx )
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      LOG(INFO) << Form( "DPB #%02u CROB Active ?:       ", uDpb);
      for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackParMuch->GetNbCrobsPerDpb(); ++uCrobIdx )
      {
         LOG(INFO) <<  fvbCrobActiveFlag[ uDpb ][ uCrobIdx ] << " ";
      } // for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackParMuch->GetNbCrobsPerDpb(); ++uCrobIdx )
      LOG(INFO) << FairLogger::endl;
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   // Internal status initialization
   fvulCurrentTsMsb.resize( fuNrOfDpbs );
   fvuCurrentTsMsbCycle.resize( fuNrOfDpbs );
   fvuInitialHeaderDone.resize( fuNrOfDpbs );
   fvuInitialTsMsbCycleHeader.resize( fuNrOfDpbs );
   fvuElinkLastTsHit.resize( fuNrOfDpbs );
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fvulCurrentTsMsb[uDpb]     = 0;
      fvuCurrentTsMsbCycle[uDpb] = 0;
      fvuInitialHeaderDone[ uDpb ] = kFALSE;
      fvuInitialTsMsbCycleHeader[uDpb] = 0;
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   fvdPrevMsTime.resize( kiMaxNbFlibLinks );

   fvulChanLastHitTime.resize( fuNbStsXyters );
   fvdChanLastHitTime.resize( fuNbStsXyters );
   fvdMsTime.resize( fuMaxNbMicroslices );
   fvuChanNbHitsInMs.resize( fuNbStsXyters );
   fvdChanLastHitTimeInMs.resize( fuNbStsXyters );
   fvusChanLastHitAdcInMs.resize( fuNbStsXyters );
   fvmAsicHitsInMs.resize( fuNbStsXyters );

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fvulChanLastHitTime[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
      fvdChanLastHitTime[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
      fvuChanNbHitsInMs[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
      fvdChanLastHitTimeInMs[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
      fvusChanLastHitAdcInMs[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
      fvmAsicHitsInMs[ uXyterIdx ].clear();

      for( UInt_t uChan = 0; uChan < fUnpackParMuch->GetNbChanPerAsic(); ++uChan )
      {
         fvulChanLastHitTime[ uXyterIdx ][ uChan ] = 0;
         fvdChanLastHitTime[ uXyterIdx ][ uChan ] = -1.0;

         fvuChanNbHitsInMs[ uXyterIdx ][ uChan ].resize( fuMaxNbMicroslices );
         fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ].resize( fuMaxNbMicroslices );
         fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ].resize( fuMaxNbMicroslices );
         for( UInt_t uMsIdx = 0; uMsIdx < fuMaxNbMicroslices; ++uMsIdx )
         {
            fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
            fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = -1.0;
            fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
         } // for( UInt_t uMsIdx = 0; uMsIdx < fuMaxNbMicroslices; ++uMsIdx )
      } // for( UInt_t uChan = 0; uChan < fUnpackParMuch->GetNbChanPerAsic(); ++uChan )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

   LOG(INFO) << "CbmMcbm2018MuchUnpacker::ReInitContainers => Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MuchUnpacker::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fUnpackParMuch->GetNbChanPerAsic()
                << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MuchUnpacker::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuMaxNbMicroslices
                << FairLogger::endl;

   fvmFebHitsInMs.resize( fuNbFebs );
   fviFebTimeSecLastRateUpdate.resize( fuNbFebs, -1 );
   fviFebCountsSinceLastRateUpdate.resize( fuNbFebs, -1 );
   fvdFebChanCountsSinceLastRateUpdate.resize( fuNbFebs );
   for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
   {
      fvmFebHitsInMs[ uFebIdx ].clear();
      fvdFebChanCountsSinceLastRateUpdate[ uFebIdx ].resize( fUnpackParMuch->GetNbChanPerFeb(), 0.0 );
   } // for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )

   return kTRUE;
}

void CbmMcbm2018MuchUnpacker::AddMsComponentToList( size_t component, UShort_t usDetectorId )
{
   /// Check for duplicates and ignore if it is the case
   for( UInt_t uCompIdx = 0; uCompIdx < fvMsComponentsList.size(); ++uCompIdx )
      if( component == fvMsComponentsList[ uCompIdx ] )
         return;

   /// Check if this does not go above hardcoded limits
   if( kiMaxNbFlibLinks <= component  )
   {
      LOG(ERROR) << "CbmMcbm2018MuchUnpacker::AddMsComponentToList => "
                 << "Ignored the addition of component " << component
                 << " as it is above the hadcoded limit of " << static_cast<const Int_t>(kiMaxNbFlibLinks)
                 << " !!!!!!!!! "
                 << FairLogger::endl
                 << "         To change this behavior check kiMaxNbFlibLinks in CbmMcbm2018MuchUnpacker.cxx"
                 << FairLogger::endl;
      return;
   } // if( kiMaxNbFlibLinks <= component  )


   /// Add to list
   fvMsComponentsList.push_back( component );
   LOG(INFO) << "CbmMcbm2018MuchUnpacker::AddMsComponentToList => Added component: " << component
             << FairLogger::endl;

   /// Create MS size monitoring histos
   if( NULL == fhMsSz[ component ] )
   {
      TString sMsSzName = Form("MsSz_link_%02lu", component);
      TString sMsSzTitle = Form("Size of MS for nDPB of link %02lu; Ms Size [bytes]", component);
      fhMsSz[ component ] = new TH1F( sMsSzName.Data(), sMsSzTitle.Data(), 160000, 0., 20000. );
      fHM->Add(sMsSzName.Data(), fhMsSz[ component ] );

      sMsSzName = Form("MsSzTime_link_%02lu", component);
      sMsSzTitle = Form("Size of MS vs time for gDPB of link %02lu; Time[s] ; Ms Size [bytes]", component);
      fhMsSzTime[ component ] =  new TProfile( sMsSzName.Data(), sMsSzTitle.Data(), 15000, 0., 300. );
      fHM->Add( sMsSzName.Data(), fhMsSzTime[ component ] );

      if( NULL != fcMsSizeAll )
      {
         fcMsSizeAll->cd( 1 + component );
         gPad->SetLogy();
         fhMsSzTime[ component ]->Draw("hist le0");
      } // if( NULL != fcMsSizeAll )
      LOG(INFO) << "Added MS size histo for component: " << component
                << " (DPB)" << FairLogger::endl;

#ifdef USE_HTTP_SERVER
      THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
      if( server )
      {
         server->Register("/FlibRaw", fhMsSz[ component ] );
         server->Register("/FlibRaw", fhMsSzTime[ component ] );
      } // if( server )
#endif
   } // if( NULL == fhMsSz[ component ] )
}
void CbmMcbm2018MuchUnpacker::SetNbMsInTs( size_t uCoreMsNb, size_t uOverlapMsNb )
{
   fuNbCoreMsPerTs = uCoreMsNb;
   fuNbOverMsPerTs = uOverlapMsNb;
   //LOG(INFO) <<" fuNbCoreMsPerTs "<<fuNbCoreMsPerTs<<" fuNbOverMsPerTs "<<fuNbOverMsPerTs<<FairLogger::endl;
   UInt_t uNbMsTotal = fuNbCoreMsPerTs + fuNbOverMsPerTs;

   if( fuMaxNbMicroslices < uNbMsTotal )
   {
      fuMaxNbMicroslices = uNbMsTotal;

      fvdMsTime.resize( fuMaxNbMicroslices );
      fvuChanNbHitsInMs.resize( fuNbStsXyters );
      fvdChanLastHitTimeInMs.resize( fuNbStsXyters );
      fvusChanLastHitAdcInMs.resize( fuNbStsXyters );
      for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      {
         fvuChanNbHitsInMs[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
         fvdChanLastHitTimeInMs[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
         fvusChanLastHitAdcInMs[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
         for( UInt_t uChan = 0; uChan < fUnpackParMuch->GetNbChanPerAsic(); ++uChan )
         {
            fvuChanNbHitsInMs[ uXyterIdx ][ uChan ].resize( fuMaxNbMicroslices );
            fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ].resize( fuMaxNbMicroslices );
            fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ].resize( fuMaxNbMicroslices );
            for( UInt_t uMsIdx = 0; uMsIdx < fuMaxNbMicroslices; ++uMsIdx )
            {
               fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
               fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = -1.0;
               fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
            } // for( UInt_t uMsIdx = 0; uMsIdx < fuMaxNbMicroslices; ++uMsIdx )
         } // for( UInt_t uChan = 0; uChan < fUnpackParMuch->GetNbChanPerAsic(); ++uChan )
      } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      LOG(INFO) << "CbmMcbm2018MuchUnpacker::DoUnpack => Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                   << FairLogger::endl;
      LOG(INFO) << "CbmMcbm2018MuchUnpacker::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fUnpackParMuch->GetNbChanPerAsic()
                   << FairLogger::endl;
      LOG(INFO) << "CbmMcbm2018MuchUnpacker::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuMaxNbMicroslices
                   << FairLogger::endl;
   } // if( fuMaxNbMicroslices < uNbMsTotal )
}


Bool_t CbmMcbm2018MuchUnpacker::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   LOG(DEBUG) << "Timeslice contains " << ts.num_microslices(component)
              << " microslices." << FairLogger::endl;
   fulCurrentTsIdx = ts.index();

   // Ignore overlap ms if flag set by user
   UInt_t uNbMsLoop = fuNbCoreMsPerTs;
   if( kFALSE == fbIgnoreOverlapMs )
      uNbMsLoop += fuNbOverMsPerTs;

   //LOG(INFO) <<" uNbMsLoop "<<uNbMsLoop<<FairLogger::endl;
   // Loop over core microslices (and overlap ones if chosen)
   for( UInt_t uMsIdx = 0; uMsIdx < uNbMsLoop; uMsIdx ++ )
   {
      Double_t dMsTime = (1e-9) * static_cast<double>( ts.descriptor( fvMsComponentsList[ 0 ], uMsIdx ).idx );

      // Loop over registered components
      for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsList.size(); ++uMsCompIdx )
      {
         UInt_t uMsComp = fvMsComponentsList[ uMsCompIdx ];

         if( kFALSE == ProcessStsMs( ts, uMsComp, uMsIdx ) )
            return kFALSE;

      } // for( UInt_t uMsComp = 0; uMsComp < fvMsComponentsList.size(); ++uMsComp )

      /// Pulses time difference calculation and plotting
      // Sort the buffer of hits
      std::sort( fvmHitsInMs.begin(), fvmHitsInMs.end() );

      // Time differences plotting using the fully time sorted hits
      if( 0 < fvmHitsInMs.size() )
      {
         ULong64_t ulLastHitTime = ( *( fvmHitsInMs.rbegin() ) ).GetTs();
         std::vector< stsxyter::FinalHit >::iterator itA;
         std::vector< stsxyter::FinalHit >::iterator itB;

         std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();
         Double_t dUnixTimeInRun = std::chrono::duration_cast< std::chrono::seconds >(tNow - ftStartTimeUnix).count();
         //LOG(INFO) <<" ulLastHitTime "<<ulLastHitTime<<" dUnixTimeInRun "<<dUnixTimeInRun<<FairLogger::endl;
         for( itA  = fvmHitsInMs.begin();
              itA != fvmHitsInMs.end();
//              itA != fvmHitsInMs.end() && (*itA).GetTs() < ulLastHitTime - 320; // 320 * 3.125 ns = 1000 ns
              ++itA )
         {
            UShort_t  usAsicIdx = (*itA).GetAsic();
            UShort_t  usChanIdx = (*itA).GetChan();
            ULong64_t ulHitTs   = (*itA).GetTs();
            UShort_t  usHitAdc  = (*itA).GetAdc();
            UShort_t  usFebIdx    = usAsicIdx / fUnpackParMuch->GetNbAsicsPerFeb();
            UShort_t  usAsicInFeb = usAsicIdx % fUnpackParMuch->GetNbAsicsPerFeb();
			//LOG(INFO) <<" usAsicIdx "<<usAsicIdx<<" usChanIdx "<<usChanIdx<<" ulHitTs "<<ulHitTs<<" usHitAdc "<<usHitAdc<<" usFebIdx "<<usFebIdx<<" usAsicInFeb "<<usAsicInFeb<<FairLogger::endl;
            Double_t dTimeSinceStartSec = (ulHitTs * stsxyter::kdClockCycleNs - fdStartTime)* 1e-9;
            //LOG(INFO) <<" dTimeSinceStartSec "<<dTimeSinceStartSec<<FairLogger::endl;
            fvmAsicHitsInMs[ usAsicIdx ].push_back( (*itA) );
            fvmFebHitsInMs[ usFebIdx ].push_back( (*itA) );
/*
            if( 1000 == fulCurrentTsIdx )
            {
               LOG(INFO) << Form( "FEB %02u ASIC %u Chan %03u TS %12u ADC %2u Time %8.3f",
                                  usFebIdx, usAsicInFeb, usChanIdx, ulHitTs, usHitAdc,
                                  ulHitTs* stsxyter::kdClockCycleNs )
                         << FairLogger::endl;
            } // if( 0 == fulCurrentTsIdx )
*/
         } // loop on time sorted hits and split per asic/feb

         // Remove all hits which were already used
//         fvmHitsInMs.erase( fvmHitsInMs.begin(), itA );
         fvmHitsInMs.clear();

///----------------- SXM 2.0 Logic Error Tagging --------------------///
         Bool_t bHitCopyInThisMs[fuNbStsXyters];
         Bool_t bHitCopySameAdcInThisMs[fuNbStsXyters];
         Bool_t bFlagOnInThisMs[fuNbStsXyters];
         for( UInt_t uAsic = 0; uAsic < fuNbStsXyters; uAsic++)
         {
            bHitCopyInThisMs[ uAsic ] = kFALSE;
            bHitCopySameAdcInThisMs[ uAsic ] = kFALSE;
            bFlagOnInThisMs[ uAsic ] = kFALSE;
         } // for( UInt_t uAsic = 0; uAsic < fuNbStsXyters; uAsic++)
///------------------------------------------------------------------///

         for( UInt_t uAsic = 0; uAsic < fuNbStsXyters; uAsic++)
         {
            UInt_t  uFebIdx    = uAsic / fUnpackParMuch->GetNbAsicsPerFeb();
            UInt_t  uAsicInFeb = uAsic % fUnpackParMuch->GetNbAsicsPerFeb();

            std::vector< ULong64_t > vulLastHitTs( fUnpackParMuch->GetNbChanPerAsic(), 0 );
            std::vector< UShort_t >  vusLastHitAdc( fUnpackParMuch->GetNbChanPerAsic(), 0 );

            for( itA  = fvmAsicHitsInMs[ uAsic ].begin(); itA != fvmAsicHitsInMs[ uAsic ].end(); ++itA )
            {
               UShort_t usAsicIdx = (*itA).GetAsic();
               UShort_t usChanIdx = (*itA).GetChan();
               ULong64_t ulHitTs  = (*itA).GetTs();
               UShort_t  usHitAdc = (*itA).GetAdc();

               UInt_t    uChanInFeb = usChanIdx + fUnpackParMuch->GetNbChanPerAsic() * uAsicInFeb;

///----------------- SXM 2.0 Logic Error Tagging --------------------///
               /// Check if hit copy
               Bool_t bIsNotCopy = kTRUE;
               if( vulLastHitTs[ usChanIdx ] == ulHitTs)
               {
                  bIsNotCopy = kFALSE;
                  bHitCopyInThisMs[ uAsic ] = kTRUE;
                  if( vusLastHitAdc[ usChanIdx ] == usHitAdc )
                     bHitCopySameAdcInThisMs[ uAsic ] = kTRUE;
               } // if( vulLastHitTs[ usChanIdx ] == ulHitTs)

               vulLastHitTs[ usChanIdx ]   = ulHitTs;
               vusLastHitAdc[ usChanIdx ]  = usHitAdc;

///------------------------------------------------------------------///
            } // for( it  = fvmAsicHitsInMs[ uAsic ].begin(); it != fvmAsicHitsInMs[ uAsic ].end(); ++it )

            /// Data in vector are not needed anymore as all possible matches are already checked
            fvmAsicHitsInMs[ uAsic ].clear();
         } // for( UInt_t uAsic = 0; uAsic < fuNbStsXyters; uAsic++)

///----------------- SXM 2.0 Logic Error Tagging --------------------///
         std::vector< Bool_t > vbCopyOnAnyAsicMs( fuNbFebs, kFALSE );
         std::vector< Bool_t > vbCopySameAdcOnAnyAsicMs( fuNbFebs, kFALSE );
         std::vector< Bool_t > vbFlagOnAnyAsicMs( fuNbFebs, kFALSE );
         Bool_t bCopyOnAnyMs = kFALSE;
         Bool_t bCopySameAdcOnAnyMs = kFALSE;
         Bool_t bFlagOnAnyMs = kFALSE;
         for( UInt_t uAsic = 0; uAsic < fuNbStsXyters; uAsic++)
         {
            UInt_t  uFebIdx    = uAsic / fUnpackParMuch->GetNbAsicsPerFeb();
            UInt_t  uAsicInFeb = uAsic % fUnpackParMuch->GetNbAsicsPerFeb();

            vbCopyOnAnyAsicMs[ uFebIdx ] = vbCopyOnAnyAsicMs[ uFebIdx ] || bHitCopyInThisMs[uAsic];
            vbCopySameAdcOnAnyAsicMs[ uFebIdx ] = vbCopySameAdcOnAnyAsicMs[ uFebIdx ] || bHitCopySameAdcInThisMs[uAsic];
            vbFlagOnAnyAsicMs[ uFebIdx ] = vbFlagOnAnyAsicMs[ uFebIdx ] || bFlagOnInThisMs[uAsic];

            bCopyOnAnyMs |= bHitCopyInThisMs[uAsic];
            bCopySameAdcOnAnyMs |= bHitCopySameAdcInThisMs[uAsic];
            bFlagOnAnyMs |= bFlagOnInThisMs[uAsic];
         } // for( UInt_t uAsic = 0; uAsic < fuNbStsXyters; uAsic++)

///------------------------------------------------------------------///

         for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
            fvmFebHitsInMs[ uFebIdx ].clear();
      } // if( 0 < fvmHitsInMs.size() )
   } // for( UInt_t uMsIdx = 0; uMsIdx < uNbMsLoop; uMsIdx ++ )

   for( UInt_t uMsIdx = 0; uMsIdx < fuMaxNbMicroslices; ++uMsIdx )
   {
      fvdMsTime[ uMsIdx ] = 0.0;
   } // for( UInt_t uMsIdx = 0; uMsIdx < fuMaxNbMicroslices; ++uMsIdx )

   if( 0 == ts.index() % 1000 )
   {
      for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
      {
         Double_t dTsMsbTime =
               static_cast<ULong64_t>( stsxyter::kuHitNbTsBins )
             * static_cast<ULong64_t>( fvulCurrentTsMsb[fuCurrDpbIdx])
             + static_cast<ULong64_t>( stsxyter::kuTsCycleNbBins )
             * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx] );
         dTsMsbTime *= stsxyter::kdClockCycleNs * 1e-9;

         LOG(INFO) << "End of TS " << std::setw(7) << ts.index()
                   << " eDPB "   << std::setw(2) << uDpb
                   << " current TS MSB counter is " << std::setw(12) << fvulCurrentTsMsb[uDpb]
                   << " current TS MSB cycle counter is " << std::setw(12) << fvuCurrentTsMsbCycle[uDpb]
                   << " current TS MSB time is " << std::setw(12) << dTsMsbTime << " s"
                   << FairLogger::endl;
      }
   } // if( 0 == ts.index() % 1000 )

  return kTRUE;
}

Bool_t CbmMcbm2018MuchUnpacker::ProcessStsMs( const fles::Timeslice& ts, size_t uMsComp, UInt_t uMsIdx )
{
   auto msDescriptor = ts.descriptor( uMsComp, uMsIdx );
   fuCurrentEquipmentId = msDescriptor.eq_id;
   //LOG(INFO) << " fuCurrentEquipmentId "<<fuCurrentEquipmentId<<FairLogger::endl;
   const uint8_t* msContent = reinterpret_cast<const uint8_t*>( ts.content( uMsComp, uMsIdx ) );

   uint32_t uSize  = msDescriptor.size;
   fulCurrentMsIdx = msDescriptor.idx;
   Double_t dMsTime = (1e-9) * static_cast<double>(fulCurrentMsIdx);
   LOG(DEBUG) << "Microslice: " << fulCurrentMsIdx
              << " from EqId " << std::hex << fuCurrentEquipmentId << std::dec
              << " has size: " << uSize << FairLogger::endl;

   fuCurrDpbId  = static_cast< uint32_t >( fuCurrentEquipmentId & 0xFFFF );
   fuCurrDpbIdx = fDpbIdIndexMap[ fuCurrDpbId ];
   //LOG(INFO) <<" fuCurrDpbIdx "<<fuCurrDpbIdx<<" fuCurrDpbId "<<fuCurrDpbId<<FairLogger::endl;

   /// Plots in [X/s] update
   if( static_cast<Int_t>( fvdPrevMsTime[ uMsComp ] ) < static_cast<Int_t>( dMsTime )  )
   {
      /// "new second"
      UInt_t uFebIdxOffset = fUnpackParMuch->GetNbFebsPerDpb() * fuCurrDpbIdx;
      for( UInt_t uFebIdx = 0; uFebIdx < fUnpackParMuch->GetNbFebsPerDpb(); ++uFebIdx )
      {
         UInt_t uFebIdxInSyst = uFebIdxOffset + uFebIdx;

         /// Ignore first interval is not clue how late the data taking was started
         if( 0 == fviFebTimeSecLastRateUpdate[uFebIdxInSyst] )
         {
            fviFebTimeSecLastRateUpdate[uFebIdxInSyst] = static_cast<Int_t>( dMsTime );
            fviFebCountsSinceLastRateUpdate[uFebIdxInSyst] = 0;
            for( UInt_t uChan = 0; uChan < fUnpackParMuch->GetNbChanPerFeb(); ++uChan )
               fvdFebChanCountsSinceLastRateUpdate[uFebIdxInSyst][uChan] = 0.0;
            continue;
         } // if( 0 == fviFebTimeSecLastRateUpdate[uFebIdxInSyst] )

         Int_t iTimeInt = static_cast<Int_t>( dMsTime ) - fviFebTimeSecLastRateUpdate[uFebIdxInSyst];
         if( fiTimeIntervalRateUpdate <= iTimeInt )
         {
            /// Jump empty FEBs without looping over channels
            if( 0 == fviFebCountsSinceLastRateUpdate[uFebIdxInSyst] )
            {
               fviFebTimeSecLastRateUpdate[uFebIdxInSyst] = static_cast<Int_t>( dMsTime );
               continue;
            } // if( 0 == fviFebCountsSinceLastRateUpdate[uFebIdxInSyst] )

            for( UInt_t uChan = 0; uChan < fUnpackParMuch->GetNbChanPerFeb(); ++uChan )
            {
               fvdFebChanCountsSinceLastRateUpdate[uFebIdxInSyst][uChan] = 0.0;
            } // for( UInt_t uChan = 0; uChan < fUnpackParMuch->GetNbChanPerFeb(); ++uChan )

            fviFebTimeSecLastRateUpdate[uFebIdxInSyst] = static_cast<Int_t>( dMsTime );
            fviFebCountsSinceLastRateUpdate[uFebIdxInSyst] = 0;
         } // if( fiTimeIntervalRateUpdate <= iTimeInt )
      } // for( UInt_t uFebIdx = 0; uFebIdx < fUnpackParMuch->GetNbFebsPerDpb(); ++uFebIdx )
   } // if( static_cast<Int_t>( fvdMsTime[ uMsCompIdx ] ) < static_cast<Int_t>( dMsTime )  )

   // Store MS time for coincidence plots
   fvdPrevMsTime[ uMsComp ] = dMsTime;

   /// Check Flags field of MS header
   uint16_t uMsHeaderFlags = msDescriptor.flags;

   /** Check the current TS_MSb cycle and correct it if wrong **/
   UInt_t uTsMsbCycleHeader = std::floor( fulCurrentMsIdx /
                                          ( stsxyter::kuTsCycleNbBins * stsxyter::kdClockCycleNs ) )
                              - fvuInitialTsMsbCycleHeader[ fuCurrDpbIdx ];
   if( kFALSE == fvuInitialHeaderDone[ fuCurrDpbIdx ] )
   {
      fvuInitialTsMsbCycleHeader[ fuCurrDpbIdx ] = uTsMsbCycleHeader;
      fvuInitialHeaderDone[ fuCurrDpbIdx ] = kTRUE;
   } // if( kFALSE == fvuInitialHeaderDone[ fuCurrDpbIdx ] )
   else if( uTsMsbCycleHeader != fvuCurrentTsMsbCycle[ fuCurrDpbIdx ] &&
            4194303 != fvulCurrentTsMsb[fuCurrDpbIdx] )
   {
      /*LOG(WARNING) << "TS MSB cycle from MS header does not match current cycle from data "
                    << "for TS " << std::setw( 12 ) << fulCurrentTsIdx
                    << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                    << " MsInTs " << std::setw( 3 ) << uMsIdx
                    << " ====> " << fvuCurrentTsMsbCycle[ fuCurrDpbIdx ]
                    << " VS " << uTsMsbCycleHeader
                    << FairLogger::endl;*/
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

   for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )
   {
      // Fill message
      uint32_t ulData = static_cast<uint32_t>( pInBuff[uIdx] );

      stsxyter::Message mess( static_cast< uint32_t >( ulData & 0xFFFFFFFF ) );

      // Print message if requested
      if( fbPrintMessages )
         mess.PrintMess( std::cout, fPrintMessCtrl );

      stsxyter::MessType typeMess = mess.GetMessType();
      fmMsgCounter[ typeMess ] ++;

      switch( typeMess )
      {
         case stsxyter::MessType::Hit :
         {
            // Extract the eLink and Asic indices => Should GO IN the fill method now that obly hits are link/asic specific!
            UShort_t usElinkIdx = mess.GetLinkIndex();
            UInt_t   uCrobIdx   = usElinkIdx / fUnpackParMuch->GetNbElinkPerCrob();
            Int_t   uFebIdx    = fUnpackParMuch->ElinkIdxToFebIdx( usElinkIdx );
//            if(usElinkIdx!=0)
			//LOG(INFO) <<" usElinkIdx "<<usElinkIdx<<" uCrobIdx "<<uCrobIdx<<" uFebIdx "<<uFebIdx<<FairLogger::endl;
            if( kTRUE == fbMuchMode )
               uFebIdx = usElinkIdx;
            if( -1 == uFebIdx )
            {
               /*LOG(WARNING) << "CbmMcbm2018MuchUnpacker::DoUnpack => "
                         << "Wrong elink Idx! Elink raw "
                         << Form("%d remap %d", usElinkIdx, uFebIdx )
                         << FairLogger::endl;*/
               continue;
            } // if( -1 == uFebIdx )
            //LOG(INFO) << " uCrobIdx "<<uCrobIdx<<" fUnpackParMuch->ElinkIdxToAsicIdx( usElinkIdx ) "<<fUnpackParMuch->ElinkIdxToAsicIdx( usElinkIdx )<<" usElinkIdx "<<usElinkIdx<<FairLogger::endl;
            UInt_t   uAsicIdx   = ( fuCurrDpbIdx * fUnpackParMuch->GetNbCrobsPerDpb() + uCrobIdx
                                  ) * fUnpackParMuch->GetNbAsicsPerCrob()
                                 + fUnpackParMuch->ElinkIdxToAsicIdx( usElinkIdx );
            //LOG(INFO) << " uCrobIdx "<<uCrobIdx<<" fUnpackParMuch->ElinkIdxToAsicIdx( usElinkIdx ) "<<fUnpackParMuch->ElinkIdxToAsicIdx( usElinkIdx )<<" usElinkIdx "<<usElinkIdx<<" uAsicIdx "<<uAsicIdx<<FairLogger::endl;
            //FillHitInfo( mess, usElinkIdx, uAsicIdx, uMsIdx );
            FillHitInfo( mess, usElinkIdx, uAsicIdx, uMsIdx, fuCurrDpbId );
            break;
         } // case stsxyter::MessType::Hit :
         case stsxyter::MessType::TsMsb :
         {
            FillTsMsbInfo( mess, uIdx, uMsIdx );
            break;
         } // case stsxyter::MessType::TsMsb :
         case stsxyter::MessType::Epoch :
         {
            // The first message in the TS is a special ones: EPOCH
            FillEpochInfo( mess );

            if( 0 < uIdx )
               LOG(INFO) << "CbmMcbm2018MuchUnpacker::DoUnpack => "
                         << "EPOCH message at unexpected position in MS: message "
                         << uIdx << " VS message 0 expected!"
                         << FairLogger::endl;
            break;
         } // case stsxyter::MessType::TsMsb :
         case stsxyter::MessType::Status :
         {
            UShort_t usElinkIdx    = mess.GetStatusLink();
            UInt_t   uCrobIdx   = usElinkIdx / fUnpackParMuch->GetNbElinkPerCrob();
            Int_t   uFebIdx    = fUnpackParMuch->ElinkIdxToFebIdx( usElinkIdx );
            UInt_t   uAsicIdx   = ( fuCurrDpbIdx * fUnpackParMuch->GetNbCrobsPerDpb() + uCrobIdx
                                  ) * fUnpackParMuch->GetNbAsicsPerCrob()
                                 + fUnpackParMuch->ElinkIdxToAsicIdx( usElinkIdx );

            UShort_t usStatusField = mess.GetStatusStatus();

            /// Always print status messages... or not?
            if( fbPrintMessages )
            {
               std::cout << Form("DPB %2u TS %12u MS %12u mess %5u ", fuCurrDpbIdx, fulCurrentTsIdx, fulCurrentMsIdx, uIdx );
               mess.PrintMess( std::cout, fPrintMessCtrl );
            } // if( fbPrintMessages )
//                   FillTsMsbInfo( mess );
            break;
         } // case stsxyter::MessType::Status
         case stsxyter::MessType::Empty :
         {
//                   FillTsMsbInfo( mess );
            break;
         } // case stsxyter::MessType::Empty :
         case stsxyter::MessType::Dummy :
         {
            break;
         } // case stsxyter::MessType::Dummy / ReadDataAck / Ack :
         default:
         {
            LOG(FATAL) << "CbmMcbm2018MuchUnpacker::DoUnpack => "
                       << "Unknown message type, should never happen, stopping here! Type found was: "
                       << static_cast< int >( typeMess )
                       << FairLogger::endl;
         }
      } // switch( mess.GetMessType() )
   } // for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )

  return kTRUE;
}

void CbmMcbm2018MuchUnpacker::FillHitInfo( stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx, const UInt_t & uDpbId )
{
   UShort_t usChan   = mess.GetHitChannel();
   UShort_t usRawAdc = mess.GetHitAdc();
//   UShort_t usFullTs = mess.GetHitTimeFull();
   UShort_t usTsOver = mess.GetHitTimeOver();
   UShort_t usRawTs  = mess.GetHitTime();

   /// Cheat needed only for modules with FEB at bottom of module or the Test module
//   usChan = 127 - usChan;

   UInt_t uCrobIdx   = usElinkIdx / fUnpackParMuch->GetNbElinkPerCrob();
   UInt_t uFebIdx    = uAsicIdx / fUnpackParMuch->GetNbAsicsPerFeb();
   UInt_t uAsicInFeb = uAsicIdx % fUnpackParMuch->GetNbAsicsPerFeb();
   UInt_t uChanInFeb = usChan + fUnpackParMuch->GetNbChanPerAsic() * (uAsicIdx % fUnpackParMuch->GetNbAsicsPerFeb());

   // For generating MUCH ADDRESS
   Short_t station = 0; // for mCBM setup
   Short_t layer = 0; // for time being 0 but will have 3 layer in mCBM
   Short_t layerside = 0; // 0 in mCBM
   Short_t module = 0; // 0 in mCBM
   //Short_t sector  = fUnpackPar->GetPadX(uAsicIdx, usChan);
   //Short_t channel = fUnpackPar->GetPadY(uAsicIdx, usChan);
   Short_t sector  = 0;
   Short_t channel = 0;

   //Creating Unique address of the particular channel of GEM
   UInt_t address = CbmMuchAddress::GetAddress(station, layer, layerside, module, sector, channel);



  // LOG(INFO) <<" usElinkIdx "<<usElinkIdx<<" uAsicIdx "<<uAsicIdx<<" uMsIdx "<<uMsIdx<<" uCrobIdx "<<uCrobIdx<<" uFebIdx "<<uFebIdx<<" uAsicInFeb "<<uAsicInFeb<<" uChanInFeb "<<uChanInFeb<<" usChan "<<usChan<< FairLogger::endl;

   // Compute the Full time stamp
   Long64_t ulOldHitTime = fvulChanLastHitTime[ uAsicIdx ][ usChan ];
   Double_t dOldHitTime  = fvdChanLastHitTime[ uAsicIdx ][ usChan ];

      // Use TS w/o overlap bits as they will anyway come from the TS_MSB
   fvulChanLastHitTime[ uAsicIdx ][ usChan ] = usRawTs;

   fvulChanLastHitTime[ uAsicIdx ][ usChan ] +=
               static_cast<ULong64_t>( stsxyter::kuHitNbTsBins )
             * static_cast<ULong64_t>( fvulCurrentTsMsb[fuCurrDpbIdx])
             + static_cast<ULong64_t>( stsxyter::kuTsCycleNbBins )
             * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx] )
             ;

//   fvuElinkLastTsHit[fuCurrDpbIdx] = usRawTs;

   // Convert the Hit time in bins to Hit time in ns
   Double_t dHitTimeNs = fvulChanLastHitTime[ uAsicIdx ][ usChan ] * stsxyter::kdClockCycleNs;
   // Store new value of Hit time in ns
   fvdChanLastHitTime[ uAsicIdx ][ usChan ] = fvulChanLastHitTime[ uAsicIdx ][ usChan ] * stsxyter::kdClockCycleNs;
   // Pulser and MS
   fvuChanNbHitsInMs[      uAsicIdx ][ usChan ][ uMsIdx ] ++;
   fvdChanLastHitTimeInMs[ uAsicIdx ][ usChan ][ uMsIdx ] = dHitTimeNs;
   fvusChanLastHitAdcInMs[ uAsicIdx ][ usChan ][ uMsIdx ] = usRawAdc;
   fvmHitsInMs.push_back( stsxyter::FinalHit( fvulChanLastHitTime[ uAsicIdx ][ usChan ], usRawAdc, uAsicIdx, usChan ) );

   // Check Starting point of histos with time as X axis
   if( -1 == fdStartTime )
      fdStartTime = fvdChanLastHitTime[ uAsicIdx ][ usChan ];

   // Fill histos with time as X axis
   Double_t dTimeSinceStartSec = (fvdChanLastHitTime[ uAsicIdx ][ usChan ] - fdStartTime)* 1e-9;
   Double_t dTimeSinceStartMin = dTimeSinceStartSec / 60.0;

   // --- Create digi
   fDigi = new CbmMuchBeamTimeDigi(address, usRawAdc, dHitTimeNs);
   fDigi->SetPadX(sector);
   fDigi->SetPadY(channel);
   fDigi->SetRocId(uDpbId);
   fDigi->SetNxId(uAsicIdx);
   fDigi->SetNxCh(usChan);

   fviFebCountsSinceLastRateUpdate[uFebIdx]++;
   fvdFebChanCountsSinceLastRateUpdate[uFebIdx][uChanInFeb] += 1;

   if(!fBuffer) cout << "------------- NULL pointer ------------- " << endl;
   if(!fBuffer) cout << "------------- NULL pointer ------------- " << endl;
   if(!fBuffer) {cout << "------------- NULL pointer ------------- " << endl;return;}

   fBuffer->InsertData(fDigi);

}

void CbmMcbm2018MuchUnpacker::FillTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx, UInt_t uMsIdx )
{
   UInt_t uVal    = mess.GetTsMsbVal();
   // Update Status counters
   if( uVal < fvulCurrentTsMsb[fuCurrDpbIdx] )
   {
      /*LOG(INFO) << " TS " << std::setw( 12 ) << fulCurrentTsIdx
                << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                << " MS Idx " << std::setw( 4 ) << uMsIdx
                << " Msg Idx " << std::setw( 5 ) << uMessIdx
                << " DPB " << std::setw( 2 ) << fuCurrDpbIdx
                << " Old TsMsb " << std::setw( 5 ) << fvulCurrentTsMsb[fuCurrDpbIdx]
                << " Old MsbCy " << std::setw( 5 ) << fvuCurrentTsMsbCycle[fuCurrDpbIdx]
                << " new TsMsb " << std::setw ( 5 ) << uVal
                << FairLogger::endl;*/
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

   ULong64_t ulNewTsMsbTime =  static_cast< ULong64_t >( stsxyter::kuHitNbTsBins )
                             * static_cast< ULong64_t >( fvulCurrentTsMsb[fuCurrDpbIdx])
                             + static_cast< ULong64_t >( stsxyter::kuTsCycleNbBins )
                             * static_cast< ULong64_t >( fvuCurrentTsMsbCycle[fuCurrDpbIdx] );
}

void CbmMcbm2018MuchUnpacker::FillEpochInfo( stsxyter::Message mess )
{
   UInt_t uVal    = mess.GetEpochVal();
   UInt_t uCurrentCycle = uVal % stsxyter::kuTsCycleNbBins;

}

void CbmMcbm2018MuchUnpacker::Reset()
{
  if (fCreateRawMessage) {
    fMuchRaw->Clear();
  }
  fMuchDigi->Clear();

}

void CbmMcbm2018MuchUnpacker::Finish()
{

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MuchUnpacker statistics are " << FairLogger::endl;
   LOG(INFO) << " Hit      messages: " << fmMsgCounter[ stsxyter::MessType::Hit   ] << FairLogger::endl
             << " Ts MSB   messages: " << fmMsgCounter[ stsxyter::MessType::TsMsb ] << FairLogger::endl
             << " Dummy    messages: " << fmMsgCounter[ stsxyter::MessType::Dummy ] << FairLogger::endl
             << " Epoch    messages: " << fmMsgCounter[ stsxyter::MessType::Epoch ] << FairLogger::endl
             << " Empty    messages: " << fmMsgCounter[ stsxyter::MessType::Empty ] << FairLogger::endl;

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;


}


void CbmMcbm2018MuchUnpacker::FillOutput(CbmDigi* digi)
{
  LOG(DEBUG) << "VS: FillOutput Called and should create a digi array:-" << FairLogger::endl;

  new( (*fMuchDigi)[fMuchDigi->GetEntriesFast()] ) CbmMuchBeamTimeDigi(*(dynamic_cast<CbmMuchBeamTimeDigi*>(digi)));

  // Delete object pointed by digi after using it
//  digi->Delete();
  delete digi;
}

ClassImp(CbmMcbm2018MuchUnpacker)
