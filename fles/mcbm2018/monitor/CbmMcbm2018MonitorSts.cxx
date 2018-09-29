// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                   CbmMcbm2018MonitorSts                        -----
// -----                Created 11/05/18  by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018MonitorSts.h"

// Data

// CbmRoot
#include "CbmMcbm2018StsPar.h"
#include "CbmHistManager.h"

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

Bool_t bMcbm2018ResetSts = kFALSE;
Bool_t bMcbm2018WriteSts = kFALSE;

CbmMcbm2018MonitorSts::CbmMcbm2018MonitorSts() :
   CbmMcbmUnpack(),
   fvMsComponentsList(),
   fuNbCoreMsPerTs(0),
   fuNbOverMsPerTs(0),
   fbIgnoreOverlapMs(kFALSE),
   fUnpackParSts(NULL),
   fuNbModules(0),
   fviModuleType(),
   fviModAddress(),
   fuNrOfDpbs(0),
   fDpbIdIndexMap(),
   fvbCrobActiveFlag(),
   fuNbFebs(0),
   fuNbStsXyters(0),
   fviFebModuleIdx(),
   fviFebModuleSide(),
   fviFebType(),
/*
   fuNrOfDpbs(0),
   fDpbIdIndexMap(),
   fuNbStsXyters(0),
   fUnpackParSts->GetNbChanPerAsic()(0),
   fuNbFebs(0),
*/
   fsHistoFileFullname( "data/SetupHistos.root" ),
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
   fvdMsTime(),
   fvuChanNbHitsInMs(),
   fvdChanLastHitTimeInMs(),
   fvusChanLastHitAdcInMs(),
//   fvmChanHitsInTs(),
   fdStartTime(-1.0),
   fdStartTimeMsSz(-1.0),
   ftStartTimeUnix( std::chrono::steady_clock::now() ),
   fvmHitsInMs(),
   fvmAsicHitsInMs(),
   fvmFebHitsInMs(),
   fuMaxNbMicroslices(100),
   fbLongHistoEnable( kFALSE ),
   fuLongHistoNbSeconds( 0 ),
   fuLongHistoBinSizeSec( 0 ),
   fuLongHistoBinNb( 0 ),
   fdCoincCenter(   0.0 ),
   fdCoincBorder(  50.0 ),
   fdCoincMin( fdCoincCenter - fdCoincBorder ),
   fdCoincMax( fdCoincCenter + fdCoincBorder ),
   fHM(new CbmHistManager()),
   fhStsMessType(NULL),
   fhStsSysMessType(NULL),
   fhStsMessTypePerDpb(NULL),
   fhStsSysMessTypePerDpb(NULL),
/*
   fhStsChanCntRaw(),
   fhStsChanCntRawGood(),
   fhStsChanAdcRaw(),
   fhStsChanAdcRawProf(),
   fhStsChanRawTs(),
   fhStsChanMissEvt(),
   fhStsChanMissEvtEvo(),
   fhStsChanHitRateEvo(),
   fhStsAsicRateEvo(),
   fhStsAsicMissEvtEvo(),
   fhStsChanHitRateEvoLong(),
   fhStsAsicRateEvoLong(),
*/
   fhStsFebChanCntRaw(),
   fhStsFebChanCntRawGood(),
   fhStsFebChanAdcRaw(),
   fhStsFebChanAdcRawProf(),
   fhStsFebChanRawTs(),
   fhStsFebChanMissEvt(),
   fhStsFebChanMissEvtEvo(),
   fhStsFebAsicMissEvtEvo(),
   fhStsFebMissEvtEvo(),
   fhStsFebChanHitRateEvo(),
   fhStsFebAsicHitRateEvo(),
   fhStsFebHitRateEvo(),
   fhStsFebChanHitRateEvoLong(),
   fhStsFebAsicHitRateEvoLong(),
   fhStsFebHitRateEvoLong(),
   fhStsFebChanCoinc(),
   fhStsModulePNCoincChan(),
   fhStsModulePNCoincAdc(),
   fhStsModuleCoincAdcChanP(),
   fhStsModuleCoincAdcChanN(),
   fhStsModuleCoincMap(),
   fcMsSizeAll(NULL)
{
}

CbmMcbm2018MonitorSts::~CbmMcbm2018MonitorSts()
{
}

Bool_t CbmMcbm2018MonitorSts::Init()
{
  LOG(INFO) << "Initializing flib StsXyter unpacker for STS" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  return kTRUE;
}

void CbmMcbm2018MonitorSts::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
         << FairLogger::endl;
   fUnpackParSts = (CbmMcbm2018StsPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmMcbm2018StsPar"));
}


Bool_t CbmMcbm2018MonitorSts::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
         << FairLogger::endl;

   Bool_t bReInit = ReInitContainers();
   CreateHistograms();

   return bReInit;
}

Bool_t CbmMcbm2018MonitorSts::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
             << FairLogger::endl;

   fuNbModules   = fUnpackParSts->GetNbOfModules();
   LOG(INFO) << "Nr. of STS Modules:    " << fuNbModules
             << FairLogger::endl;

   fviModuleType.resize( fuNbModules );
   fviModAddress.resize( fuNbModules );
   for( UInt_t uModIdx = 0; uModIdx < fuNbModules; ++uModIdx)
   {
      fviModuleType[ uModIdx ] = fUnpackParSts->GetModuleType( uModIdx );
      fviModAddress[ uModIdx ] = fUnpackParSts->GetModuleAddress( uModIdx );
      LOG(INFO) << "Module #" << std::setw(2) << uModIdx
                << " Type " << std::setw(4)  << fviModuleType[ uModIdx ]
                << " Address 0x" << std::setw(8) << std::hex <<fviModAddress[ uModIdx ]
                << std::dec
                << FairLogger::endl;
   } // for( UInt_t uModIdx = 0; uModIdx < fuNbModules; ++uModIdx)

   fuNrOfDpbs = fUnpackParSts->GetNrOfDpbs();
   LOG(INFO) << "Nr. of STS DPBs:       " << fuNrOfDpbs
             << FairLogger::endl;

   fDpbIdIndexMap.clear();
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fDpbIdIndexMap[ fUnpackParSts->GetDpbId( uDpb )  ] = uDpb;
      LOG(INFO) << "Eq. ID for DPB #" << std::setw(2) << uDpb << " = 0x"
                << std::setw(4) << std::hex << fUnpackParSts->GetDpbId( uDpb )
                << std::dec
                << " => " << fDpbIdIndexMap[ fUnpackParSts->GetDpbId( uDpb )  ]
                << FairLogger::endl;
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   fuNbFebs      = fUnpackParSts->GetNrOfFebs();
   LOG(INFO) << "Nr. of FEBs:           " << fuNbFebs
             << FairLogger::endl;

   fuNbStsXyters = fUnpackParSts->GetNrOfAsics();
   LOG(INFO) << "Nr. of StsXyter ASICs: " << fuNbStsXyters
             << FairLogger::endl;

   fvbCrobActiveFlag.resize( fuNrOfDpbs );
   fviFebModuleIdx.resize(   fuNrOfDpbs );
   fviFebModuleSide.resize(  fuNrOfDpbs );
   fviFebType.resize(        fuNrOfDpbs );
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fvbCrobActiveFlag[ uDpb ].resize( fUnpackParSts->GetNbCrobsPerDpb() );
      fviFebModuleIdx[ uDpb ].resize(   fUnpackParSts->GetNbCrobsPerDpb() );
      fviFebModuleSide[ uDpb ].resize(  fUnpackParSts->GetNbCrobsPerDpb() );
      fviFebType[ uDpb ].resize(        fUnpackParSts->GetNbCrobsPerDpb() );
      for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackParSts->GetNbCrobsPerDpb(); ++uCrobIdx )
      {
         fvbCrobActiveFlag[ uDpb ][ uCrobIdx ] = fUnpackParSts->IsCrobActive( uDpb, uCrobIdx );

         fviFebModuleIdx[ uDpb ][ uCrobIdx ].resize(   fUnpackParSts->GetNbFebsPerCrob() );
         fviFebModuleSide[ uDpb ][ uCrobIdx ].resize(  fUnpackParSts->GetNbFebsPerCrob() );
         fviFebType[ uDpb ][ uCrobIdx ].resize(        fUnpackParSts->GetNbFebsPerCrob(), -1 );
         for( UInt_t uFebIdx = 0; uFebIdx < fUnpackParSts->GetNbFebsPerCrob(); ++ uFebIdx )
         {
            fviFebModuleIdx[ uDpb ][ uCrobIdx ][ uFebIdx ] = fUnpackParSts->GetFebModuleIdx( uDpb, uCrobIdx, uFebIdx );
            fviFebModuleSide[ uDpb ][ uCrobIdx ][ uFebIdx ] = fUnpackParSts->GetFebModuleSide( uDpb, uCrobIdx, uFebIdx );

            if( 0 < fviFebModuleSide[ uDpb ][ uCrobIdx ][ uFebIdx ] &&
                fviFebModuleIdx[ uDpb ][ uCrobIdx ][ uFebIdx ] < fuNbModules )
            {
               switch( fviModuleType[ fviFebModuleIdx[ uDpb ][ uCrobIdx ][ uFebIdx ] ] )
               {
                  case 0: // FEB-8-1 with ZIF connector on the right
                  {
                     // P side (0) has type A (0)
                     // N side (1) has type B (1)
                     fviFebType[ uDpb ][ uCrobIdx ][ uFebIdx ] = fUnpackParSts->GetFebModuleSide( uDpb, uCrobIdx, uFebIdx );
                     break;
                  } // case 0: // FEB-8-1 with ZIF connector on the right
                  case 1: // FEB-8-1 with ZIF connector on the left
                  {
                     // P side (0) has type B (1)
                     // N side (1) has type A (0)
                     fviFebType[ uDpb ][ uCrobIdx ][ uFebIdx ] = !(fUnpackParSts->GetFebModuleSide( uDpb, uCrobIdx, uFebIdx ));
                     break;
                  } // case 1: // FEB-8-1 with ZIF connector on the left
                  default:
                     break;
               } // switch( fviModuleType[ fviFebModuleIdx[ uDpb ][ uCrobIdx ][ uFebIdx ] ] )
            } // FEB active and module index OK
         } // for( UInt_t uFebIdx = 0; uFebIdx < fUnpackParSts->GetNbFebsPerCrob(); ++ uFebIdx )
      } // for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackParSts->GetNbCrobsPerDpb(); ++uCrobIdx )
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

/*
   fuNrOfDpbs       = fUnpackParSts->GetNrOfDpbs();
   fuNbStsXyters    = fUnpackParSts->GetNbStsXyters();
   fuNbFebs         = fuNbStsXyters/fUnpackParSts->GetNbAsicsPerFeb();

   LOG(INFO) << "Nr. of STS DPBs:       " << fuNrOfDpbs
             << FairLogger::endl;

   fDpbIdIndexMap.clear();
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fDpbIdIndexMap[ fUnpackParSts->GetDpbId( uDpb )  ] = uDpb;
      LOG(INFO) << "Eq. ID for DPB #" << std::setw(2) << uDpb << " = "
                << std::setw(4) << std::hex << fUnpackParSts->GetDpbId( uDpb )
                << std::dec
                << " => " << fDpbIdIndexMap[ fUnpackParSts->GetDpbId( uDpb )  ]
                << FairLogger::endl;
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   LOG(INFO) << "Nr. of StsXyter ASICs: " << fuNbStsXyters
             << FairLogger::endl;
   LOG(INFO) << "Nb. channels per ASIC: " << fUnpackParSts->GetNbChanPerAsic()
             << FairLogger::endl;
*/
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

   fvulChanLastHitTime.resize( fuNbStsXyters );
   fvdChanLastHitTime.resize( fuNbStsXyters );
   fvdMsTime.resize( fuMaxNbMicroslices );
   fvuChanNbHitsInMs.resize( fuNbStsXyters );
   fvdChanLastHitTimeInMs.resize( fuNbStsXyters );
   fvusChanLastHitAdcInMs.resize( fuNbStsXyters );
   fvmAsicHitsInMs.resize( fuNbStsXyters );

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fvulChanLastHitTime[ uXyterIdx ].resize( fUnpackParSts->GetNbChanPerAsic() );
      fvdChanLastHitTime[ uXyterIdx ].resize( fUnpackParSts->GetNbChanPerAsic() );
      fvuChanNbHitsInMs[ uXyterIdx ].resize( fUnpackParSts->GetNbChanPerAsic() );
      fvdChanLastHitTimeInMs[ uXyterIdx ].resize( fUnpackParSts->GetNbChanPerAsic() );
      fvusChanLastHitAdcInMs[ uXyterIdx ].resize( fUnpackParSts->GetNbChanPerAsic() );
      fvmAsicHitsInMs[ uXyterIdx ].clear();

      for( UInt_t uChan = 0; uChan < fUnpackParSts->GetNbChanPerAsic(); ++uChan )
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
      } // for( UInt_t uChan = 0; uChan < fUnpackParSts->GetNbChanPerAsic(); ++uChan )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

   LOG(INFO) << "CbmMcbm2018MonitorSts::ReInitContainers => Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MonitorSts::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fUnpackParSts->GetNbChanPerAsic()
                << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MonitorSts::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuMaxNbMicroslices
                << FairLogger::endl;

   fvmFebHitsInMs.resize( fuNbFebs );
   for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
      fvmFebHitsInMs[ uFebIdx ].clear();

   return kTRUE;
}

void CbmMcbm2018MonitorSts::AddMsComponentToList( size_t component, UShort_t usDetectorId )
{
   /// Check for duplicates and ignore if it is the case
   for( UInt_t uCompIdx = 0; uCompIdx < fvMsComponentsList.size(); ++uCompIdx )
      if( component == fvMsComponentsList[ uCompIdx ] )
         return;

   /// Add to list
   fvMsComponentsList.push_back( component );

   /// Create MS size monitoring histos
   if( component < kiMaxNbFlibLinks )
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
void CbmMcbm2018MonitorSts::SetNbMsInTs( size_t uCoreMsNb, size_t uOverlapMsNb )
{
   fuNbCoreMsPerTs = uCoreMsNb;
   fuNbOverMsPerTs = uOverlapMsNb;

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
         fvuChanNbHitsInMs[ uXyterIdx ].resize( fUnpackParSts->GetNbChanPerAsic() );
         fvdChanLastHitTimeInMs[ uXyterIdx ].resize( fUnpackParSts->GetNbChanPerAsic() );
         fvusChanLastHitAdcInMs[ uXyterIdx ].resize( fUnpackParSts->GetNbChanPerAsic() );
         for( UInt_t uChan = 0; uChan < fUnpackParSts->GetNbChanPerAsic(); ++uChan )
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
         } // for( UInt_t uChan = 0; uChan < fUnpackParSts->GetNbChanPerAsic(); ++uChan )
      } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      LOG(INFO) << "CbmMcbm2018MonitorSts::DoUnpack => Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                   << FairLogger::endl;
      LOG(INFO) << "CbmMcbm2018MonitorSts::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fUnpackParSts->GetNbChanPerAsic()
                   << FairLogger::endl;
      LOG(INFO) << "CbmMcbm2018MonitorSts::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuMaxNbMicroslices
                   << FairLogger::endl;
   } // if( fuMaxNbMicroslices < uNbMsTotal )
}

void CbmMcbm2018MonitorSts::SetCoincidenceBorder( Double_t dCenterPos, Double_t dBorderVal )
{
   fdCoincCenter = dCenterPos;
   fdCoincBorder = dBorderVal;
   fdCoincMin    = dCenterPos - dBorderVal;
   fdCoincMax    = dCenterPos + dBorderVal;
}

void CbmMcbm2018MonitorSts::CreateHistograms()
{
   TString sHistName{""};
   TString title{""};

   sHistName = "hPulserMessageType";
   title = "Nb of message for each type; Type";
   fhStsMessType = new TH1I(sHistName, title, 5, 0., 5.);
   fhStsMessType->GetXaxis()->SetBinLabel( 1, "Dummy");
   fhStsMessType->GetXaxis()->SetBinLabel( 2, "Hit");
   fhStsMessType->GetXaxis()->SetBinLabel( 3, "TsMsb");
   fhStsMessType->GetXaxis()->SetBinLabel( 4, "Epoch");
   fhStsMessType->GetXaxis()->SetBinLabel( 5, "Empty");
/* *** Missing int + MessType OP!!!! ****
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::Dummy,       "Dummy");
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::Hit,         "Hit");
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::TsMsb,       "TsMsb");
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::ReadDataAck, "ReadDataAck");
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::Ack,         "Ack");
*/

   sHistName = "hPulserSysMessType";
   title = "Nb of system message for each type; System Type";
   fhStsSysMessType = new TH1I(sHistName, title, 17, 0., 17.);
/*
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   hSysMessType->GetXaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/

   sHistName = "hPulserMessageTypePerDpb";
   title = "Nb of message of each type for each DPB; DPB; Type";
   fhStsMessTypePerDpb = new TH2I(sHistName, title, fuNrOfDpbs, 0, fuNrOfDpbs, 5, 0., 5.);
   fhStsMessTypePerDpb->GetYaxis()->SetBinLabel( 1, "Dummy");
   fhStsMessTypePerDpb->GetYaxis()->SetBinLabel( 2, "Hit");
   fhStsMessTypePerDpb->GetYaxis()->SetBinLabel( 3, "TsMsb");
   fhStsMessTypePerDpb->GetYaxis()->SetBinLabel( 4, "Epoch");
   fhStsMessTypePerDpb->GetYaxis()->SetBinLabel( 5, "Empty");
/* *** Missing int + MessType OP!!!! ****
   fhStsMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Dummy,       "Dummy");
   fhStsMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Hit,         "Hit");
   fhStsMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::TsMsb,       "TsMsb");
   fhStsMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::ReadDataAck, "ReadDataAck");
   fhStsMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Ack,         "Ack");
*/

   sHistName = "hPulserSysMessTypePerDpb";
   title = "Nb of system message of each type for each DPB; DPB; System Type";
   fhStsSysMessTypePerDpb = new TH2I(sHistName, title, fuNrOfDpbs, 0, fuNrOfDpbs, 17, 0., 17.);
/*
   hSysMessType->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   hSysMessType->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   hSysMessType->GetYaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/

   // Number of rate bins =
   //      9 for the sub-unit decade
   //    + 9 for each unit of each decade * 10 for the subdecade range
   //    + 1 for the closing bin top edge
   const Int_t iNbDecadesRate    = 9;
   const Int_t iNbStepsDecade    = 9;
   const Int_t iNbSubStepsInStep = 10;
   const Int_t iNbBinsRate = iNbStepsDecade
                           + iNbStepsDecade * iNbSubStepsInStep * iNbDecadesRate
                           + 1;
   Double_t dBinsRate[iNbBinsRate];
      // First fill sub-unit decade
   for( Int_t iSubU = 0; iSubU < iNbStepsDecade; iSubU ++ )
      dBinsRate[ iSubU ] = 0.1 * ( 1 + iSubU );
   std::cout << std::endl;
      // Then fill the main decades
   Double_t dSubstepSize = 1.0 / iNbSubStepsInStep;
   for( Int_t iDecade = 0; iDecade < iNbDecadesRate; iDecade ++)
   {
      Double_t dBase = std::pow( 10, iDecade );
      Int_t iDecadeIdx = iNbStepsDecade
                       + iDecade * iNbStepsDecade * iNbSubStepsInStep;
      for( Int_t iStep = 0; iStep < iNbStepsDecade; iStep++ )
      {
         Int_t iStepIdx = iDecadeIdx + iStep * iNbSubStepsInStep;
         for( Int_t iSubStep = 0; iSubStep < iNbSubStepsInStep; iSubStep++ )
         {
            dBinsRate[ iStepIdx + iSubStep ] = dBase * (1 + iStep)
                                             + dBase * dSubstepSize * iSubStep;
         } // for( Int_t iSubStep = 0; iSubStep < iNbSubStepsInStep; iSubStep++ )
      } // for( Int_t iStep = 0; iStep < iNbStepsDecade; iStep++ )
   } // for( Int_t iDecade = 0; iDecade < iNbDecadesRate; iDecade ++)
   dBinsRate[ iNbBinsRate - 1 ] = std::pow( 10, iNbDecadesRate );

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++///
   UInt_t uAlignedLimit = fuLongHistoNbSeconds - (fuLongHistoNbSeconds % fuLongHistoBinSizeSec);
   fuLongHistoBinNb = uAlignedLimit / fuLongHistoBinSizeSec;

   UInt_t uNbBinEvo = (32768 + 1) * 2;
   Double_t dMaxEdgeEvo = stsxyter::kdClockCycleNs
                         * static_cast< Double_t >( uNbBinEvo ) / 2.0;
   Double_t dMinEdgeEvo = dMaxEdgeEvo * -1.0;

   UInt_t uNbBinDt     = static_cast<UInt_t>( (fdCoincMax - fdCoincMin )/stsxyter::kdClockCycleNs );

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++///
/*
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      // Channel counts
      sHistName = Form( "hStsChanCntRaw_%03u", uXyterIdx );
      title = Form( "Hits Count per channel, StsXyter #%03u; Channel; Hits []", uXyterIdx );
      fhStsChanCntRaw.push_back( new TH1I(sHistName, title,
                                 fUnpackParSts->GetNbChanPerAsic(), -0.5, fUnpackParSts->GetNbChanPerAsic() - 0.5 ) );

      sHistName = Form( "hStsChanCntRawGood_%03u", uXyterIdx );
      title = Form( "Hits Count per channel in good MS, StsXyter #%03u; Channel; Hits []", uXyterIdx );
      fhStsChanCntRawGood.push_back( new TH1I(sHistName, title,
                                 fUnpackParSts->GetNbChanPerAsic(), -0.5, fUnpackParSts->GetNbChanPerAsic() - 0.5 ) );

      // Raw Adc Distribution
      sHistName = Form( "hStsChanAdcRaw_%03u", uXyterIdx );
      title = Form( "Raw Adc distribution per channel, StsXyter #%03u; Channel []; Adc []; Hits []", uXyterIdx );
      fhStsChanAdcRaw.push_back( new TH2I(sHistName, title,
                                 fUnpackParSts->GetNbChanPerAsic(), -0.5, fUnpackParSts->GetNbChanPerAsic() - 0.5,
                                 stsxyter::kuHitNbAdcBins, -0.5, stsxyter::kuHitNbAdcBins -0.5 ) );

      // Raw Adc Distribution profile
      sHistName = Form( "hStsChanAdcRawProfc_%03u", uXyterIdx );
      title = Form( "Raw Adc prodile per channel, StsXyter #%03u; Channel []; Adc []", uXyterIdx );
      fhStsChanAdcRawProf.push_back( new TProfile(sHistName, title,
                                 fUnpackParSts->GetNbChanPerAsic(), -0.5, fUnpackParSts->GetNbChanPerAsic() - 0.5 ) );

      // Raw Ts Distribution
      sHistName = Form( "hStsChanRawTs_%03u", uXyterIdx );
      title = Form( "Raw Timestamp distribution per channel, StsXyter #%03u; Channel []; Ts []; Hits []", uXyterIdx );
      fhStsChanRawTs.push_back( new TH2I(sHistName, title,
                                 fUnpackParSts->GetNbChanPerAsic(), -0.5, fUnpackParSts->GetNbChanPerAsic() - 0.5,
                                 stsxyter::kuHitNbTsBins, -0.5, stsxyter::kuHitNbTsBins -0.5 ) );

      // Missed event flag
      sHistName = Form( "hStsChanMissEvt_%03u", uXyterIdx );
      title = Form( "Missed Event flags per channel, StsXyter #%03u; Channel []; Miss Evt []; Hits []", uXyterIdx );
      fhStsChanMissEvt.push_back( new TH2I(sHistName, title,
                                 fUnpackParSts->GetNbChanPerAsic(), -0.5, fUnpackParSts->GetNbChanPerAsic() - 0.5,
                                 2, -0.5, 1.5 ) );

      // Missed event flag counts evolution
      sHistName = Form( "hStsChanMissEvtEvo_%03u", uXyterIdx );
      title = Form( "Missed Evt flags per second & channel in StsXyter #%03u; Time [s]; Channel []; Missed Evt flags []", uXyterIdx );
      fhStsChanMissEvtEvo.push_back( new TH2I( sHistName, title,
                                                1800, 0, 1800,
                                                fUnpackParSts->GetNbChanPerAsic(), -0.5, fUnpackParSts->GetNbChanPerAsic() - 0.5 ) );

      // Missed event flag counts evo per StsXyter
      sHistName = Form( "hStsAsicMissEvtEvo%03u", uXyterIdx );
      title = Form( "Missed Evt flags per second in StsXyter #%03u; Time [s]; Missed Evt flags []", uXyterIdx );
      fhStsAsicMissEvtEvo.push_back( new TH1I(sHistName, title, 1800, 0, 1800 ) );

      // Hit rates evo per channel
      sHistName = Form( "hStsChanRateEvo_%03u", uXyterIdx );
      title = Form( "Hits per second & channel in StsXyter #%03u; Time [s]; Channel []; Hits []", uXyterIdx );
      fhStsChanHitRateEvo.push_back( new TH2I( sHistName, title,
                                                1800, 0, 1800,
                                                fUnpackParSts->GetNbChanPerAsic(), -0.5, fUnpackParSts->GetNbChanPerAsic() - 0.5 ) );

      // Hit rates evo per StsXyter
      sHistName = Form( "hStsAsicRateEvo_%03u", uXyterIdx );
      title = Form( "Hits per second in StsXyter #%03u; Time [s]; Hits []", uXyterIdx );
      fhStsAsicRateEvo.push_back( new TH1I(sHistName, title, 1800, 0, 1800 ) );

      // Hit rates evo per channel, 1 minute bins, 24h
      sHistName = Form( "hStsChanRateEvoLong_%03u", uXyterIdx );
      title = Form( "Hits per second & channel in StsXyter #%03u; Time [min]; Channel []; Hits []", uXyterIdx );
      fhStsChanHitRateEvoLong.push_back( new TH2D( sHistName, title,
                                                fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5,
                                                fUnpackParSts->GetNbChanPerAsic(), -0.5, fUnpackParSts->GetNbChanPerAsic() - 0.5 ) );

      // Hit rates evo per StsXyter, 1 minute bins, 24h
      sHistName = Form( "hStsAsicRateEvoLong_%03u", uXyterIdx );
      title = Form( "Hits per second in StsXyter #%03u; Time [min]; Hits []", uXyterIdx );
      fhStsAsicRateEvoLong.push_back( new TH1D(sHistName, title,
                                                fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5 ) );
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
*/

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++///
   /// FEB-8 plots
   /// All histos per FEB: with channels or ASIC as axis!!
   fhStsFebChanCoinc.resize( fuNbFebs );
   for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
   {
      /// Channel counts
      sHistName = Form( "hStsFebChanCntRaw_%03u", uFebIdx );
      title = Form( "Hits Count per channel, FEB #%03u; Channel; Hits []", uFebIdx );
      fhStsFebChanCntRaw.push_back( new TH1I(sHistName, title,
                                  fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5 ) );

      sHistName = Form( "hStsFebChanCntRawGood_%03u", uFebIdx );
      title = Form( "Hits Count per channel in good MS (SX2 bug flag off), FEB #%03u; Channel; Hits []", uFebIdx );
      fhStsFebChanCntRawGood.push_back( new TH1I(sHistName, title,
                                  fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5 ) );

      /// Raw Adc Distribution
      sHistName = Form( "hStsFebChanAdcRaw_%03u", uFebIdx );
      title = Form( "Raw Adc distribution per channel, FEB #%03u; Channel []; Adc []; Hits []", uFebIdx );
      fhStsFebChanAdcRaw.push_back( new TH2I(sHistName, title,
                                 fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5,
                                 stsxyter::kuHitNbAdcBins, -0.5, stsxyter::kuHitNbAdcBins -0.5 ) );

      /// Raw Adc Distribution profile
      sHistName = Form( "hStsFebChanAdcRawProfc_%03u", uFebIdx );
      title = Form( "Raw Adc prodile per channel, FEB #%03u; Channel []; Adc []", uFebIdx );
      fhStsFebChanAdcRawProf.push_back( new TProfile(sHistName, title,
                                 fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5 ) );

      /// Raw Ts Distribution
      sHistName = Form( "hStsFebChanRawTs_%03u", uFebIdx );
      title = Form( "Raw Timestamp distribution per channel, FEB #%03u; Channel []; Ts []; Hits []", uFebIdx );
      fhStsFebChanRawTs.push_back( new TH2I(sHistName, title,
                                 fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5,
                                 stsxyter::kuHitNbTsBins, -0.5, stsxyter::kuHitNbTsBins -0.5 ) );

      /// Missed event flag
      sHistName = Form( "hStsFebChanMissEvt_%03u", uFebIdx );
      title = Form( "Missed Event flags per channel, FEB #%03u; Channel []; Miss Evt []; Hits []", uFebIdx );
      fhStsFebChanMissEvt.push_back( new TH2I(sHistName, title,
                                             fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5,
                                             2, -0.5, 1.5 ) );

      /// Missed event flag counts evolution
      sHistName = Form( "hStsFebChanMissEvtEvo_%03u", uFebIdx );
      title = Form( "Missed Evt flags per second & channel in FEB #%03u; Time [s]; Channel []; Missed Evt flags []", uFebIdx );
      fhStsFebChanMissEvtEvo.push_back( new TH2I( sHistName, title,
                                                1800, 0, 1800,
                                                fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5 ) );

      /// Missed event flag counts evolution
      sHistName = Form( "hStsFebAsicMissEvtEvo_%03u", uFebIdx );
      title = Form( "Missed Evt flags per second & StsXyter in FEB #%03u; Time [s]; Asic []; Missed Evt flags []", uFebIdx );
      fhStsFebAsicMissEvtEvo.push_back( new TH2I( sHistName, title,
                                                1800, 0, 1800,
                                                fUnpackParSts->GetNbAsicsPerFeb(), -0.5, fUnpackParSts->GetNbAsicsPerFeb() - 0.5 ) );

      /// Missed event flag counts evolution
      sHistName = Form( "hStsFebMissEvtEvo_%03u", uFebIdx );
      title = Form( "Missed Evt flags per second & channel in FEB #%03u; Time [s]; Missed Evt flags []", uFebIdx );
      fhStsFebMissEvtEvo.push_back( new TH1I( sHistName, title, 1800, 0, 1800 ) );

      /// Hit rates evo per channel
      sHistName = Form( "hStsFebChanRateEvo_%03u", uFebIdx );
      title = Form( "Hits per second & channel in FEB #%03u; Time [s]; Channel []; Hits []", uFebIdx );
      fhStsFebChanHitRateEvo.push_back( new TH2I( sHistName, title,
                                                1800, 0, 1800,
                                                fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5 ) );

      /// Hit rates evo per StsXyter
      sHistName = Form( "hStsFebAsicRateEvo_%03u", uFebIdx );
      title = Form( "Hits per second & StsXyter in FEB #%03u; Time [s]; Asic []; Hits []", uFebIdx );
      fhStsFebAsicHitRateEvo.push_back( new TH2I( sHistName, title, 1800, 0, 1800,
                                               fUnpackParSts->GetNbAsicsPerFeb(), -0.5, fUnpackParSts->GetNbAsicsPerFeb() - 0.5  ) );

      /// Hit rates evo per FEB
      sHistName = Form( "hStsFebRateEvo_%03u", uFebIdx );
      title = Form( "Hits per second in FEB #%03u; Time [s]; Hits []", uFebIdx );
      fhStsFebHitRateEvo.push_back( new TH1I(sHistName, title, 1800, 0, 1800 ) );

      /// Hit rates evo per channel, 1 minute bins, 24h
      sHistName = Form( "hStsFebChanRateEvoLong_%03u", uFebIdx );
      title = Form( "Hits per second & channel in FEB #%03u; Time [min]; Channel []; Hits []", uFebIdx );
      fhStsFebChanHitRateEvoLong.push_back( new TH2D( sHistName, title,
                                                fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5,
                                                fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5 ) );

      /// Hit rates evo per channel, 1 minute bins, 24h
      sHistName = Form( "hStsFebAsicRateEvoLong_%03u", uFebIdx );
      title = Form( "Hits per second & StsXyter in FEB #%03u; Time [min]; Asic []; Hits []", uFebIdx );
      fhStsFebAsicHitRateEvoLong.push_back( new TH2D( sHistName, title,
                                                fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5,
                                                fUnpackParSts->GetNbAsicsPerFeb(), -0.5, fUnpackParSts->GetNbAsicsPerFeb() - 0.5 ) );

      /// Hit rates evo per FEB, 1 minute bins, 24h
      sHistName = Form( "hStsFebRateEvoLong_%03u", uFebIdx );
      title = Form( "Hits per second in FEB #%03u; Time [min]; Hits []", uFebIdx );
      fhStsFebHitRateEvoLong.push_back( new TH1D(sHistName, title,
                                                fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5 ) );

      /// Channels coincidences between FEBs
      fhStsFebChanCoinc[ uFebIdx ].resize( fuNbFebs, nullptr );
      for( UInt_t uFebIdxB = uFebIdx; uFebIdxB < fuNbFebs; ++uFebIdxB )
      {
         sHistName = Form( "hStsFebChanCoinc_%03u_%03u", uFebIdx, uFebIdxB );
         title = Form( "Channel coincidences between FEB #%03u and  FEB #%03u; Channel FEB #%03u []; Channel FEB #%03u []; Coinc. []",
                        uFebIdx, uFebIdxB, uFebIdx, uFebIdxB );
         fhStsFebChanCoinc[ uFebIdx ][ uFebIdxB ] = new TH2I( sHistName, title,
                                                fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5,
                                                fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5 );
      } // for( UInt_t uFebIdxB = uFebIdx; uFebIdxB < fuNbFebs; ++uFebIdxB )
   } // for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++///
   /// Modules plots
   Double_t dSensorMinX = - fUnpackParSts->GetSensorSzX() / 2.0;
   Double_t dSensorMaxX =   fUnpackParSts->GetSensorSzX() / 2.0;
   Double_t dSensorMinY = - fUnpackParSts->GetSensorSzY() / 2.0;
   Double_t dSensorMaxY =   fUnpackParSts->GetSensorSzY() / 2.0;
   for( UInt_t uModIdx = 0; uModIdx < fuNbModules; ++ uModIdx )
   {
      /// Chan VS chan map for PN coincidences
      sHistName = Form( "hStsModulePNCoincChan_%03u", uModIdx );
      title = Form( "P-N channel coincidences in module #%03u; Channel P []; Channel N []; Cnt []", uModIdx );
      fhStsModulePNCoincChan.push_back( new TH2D( sHistName, title,
                                                fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5,
                                                fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5 ) );

      /// Adc VS Adc map for PN channel coincidences
      sHistName = Form( "hStsModulePNCoincAdc_%03u", uModIdx );
      title = Form( "Adc values of P-N coincidences in module #%03u; ADC Channel P [bin]; ADC Channel N [bin]; Cnt []", uModIdx );
      fhStsModulePNCoincAdc.push_back( new TH2D( sHistName, title,
                                                 stsxyter::kuHitNbAdcBins, -0.5, stsxyter::kuHitNbAdcBins - 0.5,
                                                 stsxyter::kuHitNbAdcBins, -0.5, stsxyter::kuHitNbAdcBins - 0.5 ) );
      /// Adc VS P Channel for PN channel coincidences
      sHistName = Form( "hStsModuleCoincAdcChanP_%03u", uModIdx );
      title = Form( "Adc values of P chan in P-N coincidences in module #%03u; Channel P [bin]; ADC val. [bin]; Cnt []", uModIdx );
      fhStsModuleCoincAdcChanP.push_back( new TH2D( sHistName, title,
                                                 fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5,
                                                 stsxyter::kuHitNbAdcBins, -0.5, stsxyter::kuHitNbAdcBins - 0.5 ) );
      /// Adc VS N Channel for PN channel coincidences
      sHistName = Form( "hStsModuleCoincAdcChanN_%03u", uModIdx );
      title = Form( "Adc values of N chan in P-N coincidences in module #%03u; Channel N [bin]; ADC val. [bin]; Cnt []", uModIdx );
      fhStsModuleCoincAdcChanN.push_back( new TH2D( sHistName, title,
                                                 fUnpackParSts->GetNbChanPerFeb(), -0.5, fUnpackParSts->GetNbChanPerFeb() - 0.5,
                                                 stsxyter::kuHitNbAdcBins, -0.5, stsxyter::kuHitNbAdcBins - 0.5 ) );

      /// X-Y map for PN coincidences
      sHistName = Form( "hStsModuleCoincMap_%03u", uModIdx );
      title = Form( "X-Y map of P-N coincidences in module #%03u; Pos. X []; Pos. Y []; Cnt []", uModIdx );
      fhStsModuleCoincMap.push_back( new TH2D( sHistName, title,
                                                fUnpackParSts->GetSensorSzX(), dSensorMinX, dSensorMaxX,
                                                fUnpackParSts->GetSensorSzY(), dSensorMinY, dSensorMaxY ) );
   } // for( UInt_t uModIdx = 0; uModIdx < fuNbModules; ++ uModIdx )

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++///

   // Miscroslice properties histos
   for( Int_t component = 0; component < kiMaxNbFlibLinks; component ++ )
   {
      fhMsSz[ component ] = NULL;
      fhMsSzTime[ component ] = NULL;
   } // for( Int_t component = 0; component < kiMaxNbFlibLinks; component ++ )

   // Online histo browser commands
#ifdef USE_HTTP_SERVER
   THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
   if( server )
   {
      server->Register("/StsRaw", fhStsMessType );
      server->Register("/StsRaw", fhStsSysMessType );
      server->Register("/StsRaw", fhStsMessTypePerDpb );
      server->Register("/StsRaw", fhStsSysMessTypePerDpb );
/*
      for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      {
         server->Register("/StsRaw", fhStsChanCntRaw[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanCntRawGood[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanAdcRaw[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanAdcRawProf[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanRawTs[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanMissEvt[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanMissEvtEvo[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsAsicMissEvtEvo[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanHitRateEvo[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsAsicRateEvo[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanHitRateEvoLong[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsAsicRateEvoLong[ uXyterIdx ] );

      } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
*/
      for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
      {
         server->Register("/StsFeb", fhStsFebChanCntRaw[ uFebIdx ] );
         server->Register("/StsFeb", fhStsFebChanCntRawGood[ uFebIdx ] );
         server->Register("/StsFeb", fhStsFebChanAdcRaw[ uFebIdx ] );
         server->Register("/StsFeb", fhStsFebChanAdcRawProf[ uFebIdx ] );
         server->Register("/StsFeb", fhStsFebChanRawTs[ uFebIdx ] );
         server->Register("/StsFeb", fhStsFebChanMissEvt[ uFebIdx ] );
         server->Register("/StsFeb", fhStsFebChanMissEvtEvo[ uFebIdx ] );
         server->Register("/StsFeb", fhStsFebAsicMissEvtEvo[ uFebIdx ] );
         server->Register("/StsFeb", fhStsFebMissEvtEvo[ uFebIdx ] );
         server->Register("/StsFeb", fhStsFebChanHitRateEvo[ uFebIdx ] );
         server->Register("/StsFeb", fhStsFebAsicHitRateEvo[ uFebIdx ] );
         server->Register("/StsFeb", fhStsFebHitRateEvo[ uFebIdx ] );
         server->Register("/StsFeb", fhStsFebChanHitRateEvoLong[ uFebIdx ] );
         server->Register("/StsFeb", fhStsFebAsicHitRateEvoLong[ uFebIdx ] );
         server->Register("/StsFeb", fhStsFebHitRateEvoLong[ uFebIdx ] );
         for( UInt_t uFebIdxB = uFebIdx; uFebIdxB < fuNbFebs; ++uFebIdxB )
            server->Register("/StsFeb", fhStsFebChanCoinc[ uFebIdx ][ uFebIdxB ] );
      } // for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )

      for( UInt_t uModIdx = 0; uModIdx < fuNbModules; ++ uModIdx )
      {
         server->Register("/StsMod", fhStsModulePNCoincChan[ uModIdx ] );
         server->Register("/StsMod", fhStsModulePNCoincAdc[ uModIdx ] );
         server->Register("/StsMod", fhStsModuleCoincAdcChanP[ uModIdx ] );
         server->Register("/StsMod", fhStsModuleCoincAdcChanN[ uModIdx ] );
         server->Register("/StsMod", fhStsModuleCoincMap[ uModIdx ] );
      } // for( UInt_t uModIdx = 0; uModIdx < fuNbModules; ++ uModIdx )


      server->RegisterCommand("/Reset_All_Pulser", "bMcbm2018ResetSts=kTRUE");
      server->RegisterCommand("/Write_All_Pulser", "bMcbm2018WriteSts=kTRUE");

      server->Restrict("/Reset_All_Pulser", "allow=admin");
      server->Restrict("/Write_All_Pulser", "allow=admin");
   } // if( server )
#endif

   /** Create summary Canvases for CERN 2017 **/
   Double_t w = 10;
   Double_t h = 10;
/*
      // Summary per StsXyter
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      TCanvas* cStsSumm = new TCanvas( Form("cStsSum_%03u", uXyterIdx ),
                                       Form("Summary plots for StsXyter %03u", uXyterIdx ),
                                       w, h);
      cStsSumm->Divide( 2, 2 );

      cStsSumm->cd(1);
      gPad->SetLogy();
      fhStsChanCntRaw[ uXyterIdx ]->Draw();

      cStsSumm->cd(2);
      gPad->SetLogz();
      fhStsChanAdcRaw[ uXyterIdx ]->Draw( "colz" );

      cStsSumm->cd(3);
      gPad->SetLogz();
      fhStsChanHitRateEvo[ uXyterIdx ]->Draw( "colz" );

      cStsSumm->cd(4);
//      gPad->SetLogy();
      fhStsChanAdcRawProf[ uXyterIdx ]->Draw();
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
*/
//====================================================================//

//====================================================================//
  /** Recovers/Create Ms Size Canvase for CERN 2016 **/
  // Try to recover canvas in case it was created already by another monitor
  // If not existing, create it
  fcMsSizeAll = dynamic_cast<TCanvas *>( gROOT->FindObject( "cMsSizeAll" ) );
  if( NULL == fcMsSizeAll )
  {
     fcMsSizeAll = new TCanvas("cMsSizeAll", "Evolution of MS size in last 300 s", w, h);
     fcMsSizeAll->Divide( 4, 4 );
      LOG(INFO) << "Created MS size canvas in STS monitor" << FairLogger::endl;
  } // if( NULL == fcMsSizeAll )
      else LOG(INFO) << "Recovered MS size canvas in STS monitor" << FairLogger::endl;
//====================================================================//

  /*****************************/
}

Bool_t CbmMcbm2018MonitorSts::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   if( bMcbm2018ResetSts )
   {
      ResetAllHistos();
      bMcbm2018ResetSts = kFALSE;
   } // if( bMcbm2018ResetSts )
   if( bMcbm2018WriteSts )
   {
      SaveAllHistos( fsHistoFileFullname );
      bMcbm2018WriteSts = kFALSE;
   } // if( bMcbm2018WriteSts )

   LOG(DEBUG) << "Timeslice contains " << ts.num_microslices(component)
              << "microslices." << FairLogger::endl;
   fulCurrentTsIdx = ts.index();

   // Ignore overlap ms if flag set by user
   UInt_t uNbMsLoop = fuNbCoreMsPerTs;
   if( kFALSE == fbIgnoreOverlapMs )
      uNbMsLoop += fuNbOverMsPerTs;

   // Loop over core microslices (and overlap ones if chosen)
   for( UInt_t uMsIdx = 0; uMsIdx < uNbMsLoop; uMsIdx ++ )
   {
      // Loop over registered components
      for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsList.size(); ++uMsCompIdx )
      {
         UInt_t uMsComp = fvMsComponentsList[ uMsCompIdx ];
         auto msDescriptor = ts.descriptor( uMsComp, uMsIdx );
         fuCurrentEquipmentId = msDescriptor.eq_id;
         const uint8_t* msContent = reinterpret_cast<const uint8_t*>( ts.content( uMsComp, uMsIdx ) );

         uint32_t uSize  = msDescriptor.size;
         fulCurrentMsIdx = msDescriptor.idx;
         Double_t dMsTime = (1e-9) * static_cast<double>(fulCurrentMsIdx);
         LOG(DEBUG) << "Microslice: " << fulCurrentMsIdx
                    << " from EqId " << std::hex << fuCurrentEquipmentId << std::dec
                    << " has size: " << uSize << FairLogger::endl;

         fuCurrDpbId  = static_cast< uint32_t >( fuCurrentEquipmentId & 0xFFFF );
         fuCurrDpbIdx = fDpbIdIndexMap[ fuCurrDpbId ];

         if( uMsComp < kiMaxNbFlibLinks )
         {
            if( fdStartTimeMsSz < 0 )
               fdStartTimeMsSz = dMsTime;
            fhMsSz[ uMsComp ]->Fill( uSize );
            fhMsSzTime[ uMsComp ]->Fill( dMsTime - fdStartTimeMsSz, uSize);
         } // if( uMsComp < kiMaxNbFlibLinks )

         // Store MS time for coincidence plots
         fvdMsTime[ uMsCompIdx ] = dMsTime;

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
            fhStsMessType->Fill( static_cast< uint16_t > (typeMess) );
            fhStsMessTypePerDpb->Fill( fuCurrDpbIdx, static_cast< uint16_t > (typeMess) );

            switch( typeMess )
            {
               case stsxyter::MessType::Hit :
               {
                  // Extract the eLink and Asic indices => Should GO IN the fill method now that obly hits are link/asic specific!
                  UShort_t usElinkIdx = mess.GetLinkIndex();
                  UInt_t   uCrobIdx   = usElinkIdx / fUnpackParSts->GetNbElinkPerCrob();
                  UInt_t   uFebIdx    = usElinkIdx % fUnpackParSts->GetNbElinkPerCrob();
                  UInt_t   uAsicIdx   = ( fuCurrDpbIdx * fUnpackParSts->GetNbCrobsPerDpb() + uCrobIdx
                                        ) * fUnpackParSts->GetNbAsicsPerCrob()
                                       + fUnpackParSts->ElinkIdxToAsicIdx( fviFebType[ fuCurrDpbIdx ][ uCrobIdx ][ uFebIdx ],
                                                                           uFebIdx );

                  FillHitInfo( mess, usElinkIdx, uAsicIdx, uMsIdx );
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
                     LOG(INFO) << "CbmMcbm2018MonitorSts::DoUnpack => "
                               << "EPOCH message at unexpected position in MS: message "
                               << uIdx << " VS message 0 expected!"
                               << FairLogger::endl;
                  break;
               } // case stsxyter::MessType::TsMsb :
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
                  LOG(FATAL) << "CbmMcbm2018MonitorSts::DoUnpack => "
                             << "Unknown message type, should never happen, stopping here!"
                             << FairLogger::endl;
               }
            } // switch( mess.GetMessType() )
         } // for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )
      } // for( UInt_t uMsComp = 0; uMsComp < fvMsComponentsList.size(); ++uMsComp )

      /// Pulses time difference calculation and plotting
      // Sort the buffer of hits
      std::sort( fvmHitsInMs.begin(), fvmHitsInMs.end() );

      // Time differences plotting using the fully time sorted hits
      if( 0 < fvmHitsInMs.size() )
      {
         // Make sure we analyse only MS where all ASICs were detected (remove noise and MS borders)
         if( fuNbStsXyters != fvmHitsInMs.size() )
            fvmHitsInMs.erase( fvmHitsInMs.begin(), fvmHitsInMs.end() );

         ULong64_t ulLastHitTime = ( *( fvmHitsInMs.rbegin() ) ).GetTs();
         std::vector< stsxyter::FinalHit >::iterator itA;
         std::vector< stsxyter::FinalHit >::iterator itB;

         std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();
         Double_t dUnixTimeInRun = std::chrono::duration_cast< std::chrono::seconds >(tNow - ftStartTimeUnix).count();

         for( itA  = fvmHitsInMs.begin();
              itA != fvmHitsInMs.end();
//              itA != fvmHitsInMs.end() && (*itA).GetTs() < ulLastHitTime - 320; // 320 * 3.125 ns = 1000 ns
              ++itA )
         {
            UShort_t  usAsicIdx = (*itA).GetAsic();
            UShort_t  usChanIdx = (*itA).GetChan();
            ULong64_t ulHitTs   = (*itA).GetTs();
            UShort_t  usHitAdc  = (*itA).GetAdc();
            UShort_t  usFebIdx  = usAsicIdx / fUnpackParSts->GetNbAsicsPerFeb();

            Double_t dTimeSinceStartSec = (ulHitTs * stsxyter::kdClockCycleNs - fdStartTime)* 1e-9;

            fvmAsicHitsInMs[ usAsicIdx ].push_back( (*itA) );
            fvmFebHitsInMs[ usFebIdx ].push_back( (*itA) );
         } // loop on time sorted hits and split per asic/feb

         // Remove all hits which were already used
         fvmHitsInMs.erase( fvmHitsInMs.begin(), itA );

         for( UInt_t uAsic = 0; uAsic < fuNbStsXyters; uAsic++)
         {
/*
            for( itA  = fvmAsicHitsInMs[ uAsic ].begin(); itA != fvmAsicHitsInMs[ uAsic ].end(); ++itA )
            {
               UShort_t usChanIdx = (*itA).GetChan();
               if( 0.0 == fdStartTs )
                  fdStartTs = (*itA).GetTs() * stsxyter::kdClockCycleNs;
               Double_t dTimeSinceStartSec = ( (*itA).GetTs() * stsxyter::kdClockCycleNs - fdStartTs ) * 1e-9;

               for( UInt_t uAsicB = uAsic; uAsicB < fuNbStsXyters; uAsicB++)
               {
                  for( itB  = fvmAsicHitsInMs[ uAsicB ].begin(); itB != fvmAsicHitsInMs[ uAsicB ].end(); ++itB )
                  {
                     UShort_t usChanIdxB = (*itB).GetChan();
                     Double_t dDtClk = static_cast< Double_t >( (*itB).GetTs() ) - static_cast< Double_t >( (*itA).GetTs() );
                     Double_t dDt = dDtClk * stsxyter::kdClockCycleNs;
                     Double_t dDtRaw = ( static_cast< Double_t >( (*itB).GetTs() % stsxyter::kuTsCycleNbBins )
                                       - static_cast< Double_t >( (*itA).GetTs() % stsxyter::kuTsCycleNbBins )
                                    ) * stsxyter::kdClockCycleNs;

                     fhStsTimeDiffPerAsic[ uAsic ]->Fill( dDt, uAsicB );
                     fhStsTimeDiffPerAsicPair[ uAsic ][ uAsicB ]->Fill( dDt );
                     fhStsTimeDiffClkPerAsicPair[ uAsic ][ uAsicB ]->Fill( dDtClk );
                     fhStsTimeDiffEvoPerAsicPair[ uAsic ][ uAsicB ]->Fill( dTimeSinceStartSec, dDt );
                     fhStsTimeDiffEvoPerAsicPairProf[ uAsic ][ uAsicB ]->Fill( dTimeSinceStartSec, dDt );
                     fhStsRawTimeDiffEvoPerAsicPairProf[ uAsic ][ uAsicB ]->Fill( dTimeSinceStartSec, dDtRaw );

                     fhStsTsLsbMatchPerAsicPair[ uAsic ][ uAsicB ]->Fill(   (*itA ).GetTs() & 0x000FF,
                                                                            (*itB ).GetTs() & 0x000FF );
                     fhStsTsMsbMatchPerAsicPair[ uAsic ][ uAsicB ]->Fill( ( (*itA ).GetTs() & 0x03F00 ) >> 8,
                                                                          ( (*itB ).GetTs() & 0x03F00 ) >> 8 );

                     if( ( kdPulserPeriod * -0.5 < dDt && dDt < kdPulserPeriod * 0.5 &&
                           10 < (*itA).GetAdc() && 10 < (*itB).GetAdc() )// &&
//                         !( 2 == uAsic && 3 == uAsicB && 240 < dDt )
                       )
                     {
                        UpdatePairMeanValue( uAsic, uAsicB, dDt );
                     } // if( kdPulserPeriod * -0.5 < dDt && dDt < kdPulserPeriod * 0.5 )
                  } // for( itA  = fvmAsicHitsInMs[ uAsicB ].begin(); itA != fvmAsicHitsInMs[ uAsicB ].end(); ++itA )
               } // for( UInt_t uAsic = 0; uAsic < fuNbStsXyters; uAsic++)

               Double_t dDtPulse = ( static_cast< Double_t >( (*it).GetTs() ) - static_cast< Double_t >( fvmLastHitAsic[ uAsic ].GetTs() )
                              ) * stsxyter::kdClockCycleNs;
               fhStsIntervalAsic[ uAsic ]->Fill( dDtPulse );
               fhStsIntervalLongAsic[ uAsic ]->Fill( dDtPulse );
               fvmLastHitAsic[ uAsic ] = (*it);
            } // for( it  = fvmAsicHitsInMs[ uAsic ].begin(); it != fvmAsicHitsInMs[ uAsic ].end(); ++it )
*/

            /// Data in vector are not needed anymore as all possible matches are already checked
            fvmAsicHitsInMs[ uAsic ].clear();
         } // for( UInt_t uAsic = 0; uAsic < fuNbStsXyters; uAsic++)

         for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
         {
            for( itA  = fvmFebHitsInMs[ uFebIdx ].begin(); itA != fvmFebHitsInMs[ uFebIdx ].end(); ++itA )
            {
               UShort_t  usAsicIdxA  = (*itA).GetAsic();
               UShort_t  usChanIdxA  = (*itA).GetChan();
               UInt_t    uChanInFebA = usChanIdxA + fUnpackParSts->GetNbChanPerAsic() * (usAsicIdxA % fUnpackParSts->GetNbAsicsPerFeb());
               ULong64_t ulHitTsA    = (*itA).GetTs();
               Double_t  dHitTsA     = ulHitTsA * stsxyter::kdClockCycleNs;

               for( UInt_t uFebIdxB  = uFebIdx; uFebIdxB < fuNbFebs; ++uFebIdxB )
               {
                  for( itB  = fvmFebHitsInMs[ uFebIdxB ].begin(); itB != fvmFebHitsInMs[ uFebIdxB ].end(); ++itB )
                  {
                     UShort_t  usAsicIdxB  = (*itB).GetAsic();
                     UShort_t  usChanIdxB  = (*itB).GetChan();
                     UInt_t    uChanInFebB = usChanIdxB + fUnpackParSts->GetNbChanPerAsic() * (usAsicIdxB % fUnpackParSts->GetNbAsicsPerFeb());
                     ULong64_t ulHitTsB    = (*itB).GetTs();
                     Double_t  dHitTsB     = ulHitTsB * stsxyter::kdClockCycleNs;
                     Double_t dDtClk = static_cast< Double_t >( ulHitTsB ) - static_cast< Double_t >( ulHitTsA );
                     Double_t dDt    = dDtClk * stsxyter::kdClockCycleNs;

                     /// Check if we have a channel coincidence
                     if( -1.0 * kdFebChanCoincidenceLimit < dDt )
                     {
                        /// If out of coincidence, later hits will also be out => break the HitB loop
                        if( kdFebChanCoincidenceLimit < dDt )
                           break;

                        fhStsFebChanCoinc[ uFebIdx ][ uFebIdxB ]->Fill( uChanInFebA, uChanInFebB );
                     } // if( -1.0 * kdFebChanCoincidenceLimit < dDt )
                  } // for( itB  = fvmFebHitsInMs[ uFebIdxB ].begin(); itB != fvmFebHitsInMs[ uFebIdxB ].end(); ++itB )
               } // for( UInt_t uFebIdxB = uFebIdx; uFebIdxB < fuNbFebs; ++uFebIdxB )
            } // for( itA  = fvmFebHitsInMs[ uFebIdx ].begin(); itA != fvmFebHitsInMs[ uFebIdx ].end(); ++itA )

            /// Data in vector are not needed anymore as all possible matches are already checked
            fvmFebHitsInMs[ uFebIdx ].clear();
         } // for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
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

   if( 0 == ts.index() % 10000 )
      SaveAllHistos( "data/PulserPeriodicHistosSave.root");

  return kTRUE;
}

void CbmMcbm2018MonitorSts::FillHitInfo( stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx )
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
   UInt_t uFebIdx = uAsicIdx / fUnpackParSts->GetNbAsicsPerFeb();
   UInt_t uAsicInFeb = uAsicIdx % fUnpackParSts->GetNbAsicsPerFeb();
   UInt_t uChanInFeb = usChan + fUnpackParSts->GetNbChanPerAsic() * (uAsicIdx % fUnpackParSts->GetNbAsicsPerFeb());

   fhStsFebChanCntRaw[  uFebIdx ]->Fill( uChanInFeb );
   fhStsFebChanAdcRaw[  uFebIdx ]->Fill( uChanInFeb, usRawAdc );
   fhStsFebChanAdcRawProf[  uFebIdx ]->Fill( uChanInFeb, usRawAdc );
   fhStsFebChanRawTs[   uFebIdx ]->Fill( usChan, usRawTs );
   fhStsFebChanMissEvt[ uFebIdx ]->Fill( usChan, mess.IsHitMissedEvts() );

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
/*
   // If needed fill the hit interval plots
   if( fbChanHitDtEna )
   {
      Double_t dDeltaT = dHitTimeNs - fvdChanLastHitTime[ uAsicIdx ][ usChan ];
      if( 0 == dDeltaT )
         fhStsChanHitDtNeg[ uAsicIdx ]->Fill( 1, usChan );
         else if( 0 < dDeltaT )
            fhStsChanHitDt[ uAsicIdx ]->Fill( dDeltaT, usChan );
         else fhStsChanHitDtNeg[ uAsicIdx ]->Fill( -dDeltaT, usChan );
   } // if( fbChanHitDtEna )
*/
   // Store new value of Hit time in ns
   fvdChanLastHitTime[ uAsicIdx ][ usChan ] = fvulChanLastHitTime[ uAsicIdx ][ usChan ] * stsxyter::kdClockCycleNs;
/*
   LOG(INFO) << " Asic " << std::setw( 2 ) << uAsicIdx
             << " Channel " << std::setw( 3 ) << usChan
             << " Diff to last hit " << std::setw( 12 ) << ( fvulChanLastHitTime[ uAsicIdx ][ usChan ] - ulOldHitTime)
             << " in s " << std::setw( 12 ) << ( fvdChanLastHitTime[ uAsicIdx ][ usChan ] - dOldHitTime) * 1e-9
             << FairLogger::endl;
*/
   // Pulser and MS
   fvuChanNbHitsInMs[      uAsicIdx ][ usChan ][ uMsIdx ] ++;
   fvdChanLastHitTimeInMs[ uAsicIdx ][ usChan ][ uMsIdx ] = dHitTimeNs;
   fvusChanLastHitAdcInMs[ uAsicIdx ][ usChan ][ uMsIdx ] = usRawAdc;
/*
   fvmChanHitsInTs[        uAsicIdx ][ usChan ].insert( stsxyter::FinalHit( fvulChanLastHitTime[ uAsicIdx ][ usChan ],
                                                                            usRawAdc, uAsicIdx, usChan ) );
*/
   fvmHitsInMs.push_back( stsxyter::FinalHit( fvulChanLastHitTime[ uAsicIdx ][ usChan ], usRawAdc, uAsicIdx, usChan ) );

/*
   if( ( 214514 < fulCurrentTsIdx && fulCurrentTsIdx < 214517 ) ||
       ( 260113 < fulCurrentTsIdx && fulCurrentTsIdx < 260116 ) ||
       ( 388109 < fulCurrentTsIdx && fulCurrentTsIdx < 388114 ) ||
       ( 573361 < fulCurrentTsIdx && fulCurrentTsIdx < 573364 ) ||
       ( 661731 < fulCurrentTsIdx && fulCurrentTsIdx < 661734 ) ||
       ( 712982 < fulCurrentTsIdx && fulCurrentTsIdx < 712985 ) ||
       ( 713857 < fulCurrentTsIdx && fulCurrentTsIdx < 713860 ) ||
       ( 739365 < fulCurrentTsIdx && fulCurrentTsIdx < 739368 ))
   {
      LOG(INFO) << " TS " << std::setw( 12 ) << fulCurrentTsIdx
                << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                << " MsInTs " << std::setw( 3 ) << uMsIdx
                << " Asic " << std::setw( 2 ) << uAsicIdx
                << " Channel " << std::setw( 3 ) << usChan
                << " ADC " << std::setw( 3 ) << usRawAdc
                << " TS " << std::setw( 3 )  << usRawTs // 9 bits TS
                << " SX TsMsb " << std::setw( 2 ) << ( fvulCurrentTsMsb[fuCurrDpbIdx] & 0x1F ) // Total StsXyter TS = 14 bits => 9b Hit TS + lower 5b TS_MSB after DPB
                << " DPB TsMsb " << std::setw( 6 ) << ( fvulCurrentTsMsb[fuCurrDpbIdx] >> 5 ) // Total StsXyter TS = 14 bits => 9b Hit TS + lower 5b of TS_MSB after DPB
                << " TsMsb " << std::setw( 7 ) << fvulCurrentTsMsb[fuCurrDpbIdx]
                << " TS(b) " << std::bitset<9>(usRawTs)
                << " TSM(b) " << std::bitset<24>(fvulCurrentTsMsb[fuCurrDpbIdx])
                << " MsbCy " << std::setw( 4 ) << fvuCurrentTsMsbCycle[fuCurrDpbIdx]
                << " Time " << std::setw ( 12 ) << fvulChanLastHitTime[ uAsicIdx ][ usChan ]
                << FairLogger::endl;
   }
*/
   // Check Starting point of histos with time as X axis
   if( -1 == fdStartTime )
      fdStartTime = fvdChanLastHitTime[ uAsicIdx ][ usChan ];

   // Fill histos with time as X axis
   Double_t dTimeSinceStartSec = (fvdChanLastHitTime[ uAsicIdx ][ usChan ] - fdStartTime)* 1e-9;
   Double_t dTimeSinceStartMin = dTimeSinceStartSec / 60.0;

   fhStsFebChanHitRateEvo[ uFebIdx ]->Fill( dTimeSinceStartSec, uChanInFeb );
   fhStsFebAsicHitRateEvo[ uFebIdx ]->Fill( dTimeSinceStartSec, uAsicInFeb );
   fhStsFebHitRateEvo[ uFebIdx ]->Fill(     dTimeSinceStartSec );
   fhStsFebChanHitRateEvoLong[ uFebIdx ]->Fill( dTimeSinceStartMin, uChanInFeb, 1.0/60.0 );
   fhStsFebAsicHitRateEvoLong[ uFebIdx ]->Fill( dTimeSinceStartMin, uAsicInFeb,   1.0/60.0 );
   fhStsFebHitRateEvoLong[ uFebIdx ]->Fill(     dTimeSinceStartMin,             1.0/60.0 );
   if( mess.IsHitMissedEvts() )
   {
      fhStsFebChanMissEvtEvo[ uFebIdx ]->Fill( dTimeSinceStartSec, usChan );
      fhStsFebAsicMissEvtEvo[ uFebIdx ]->Fill( dTimeSinceStartSec, uAsicInFeb );
      fhStsFebMissEvtEvo[ uFebIdx ]->Fill(     dTimeSinceStartSec );
   } // if( mess.IsHitMissedEvts() )
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

void CbmMcbm2018MonitorSts::FillTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx, UInt_t uMsIdx )
{
   UInt_t uVal    = mess.GetTsMsbVal();
/*
   if( ( 419369 < fulCurrentTsIdx && fulCurrentTsIdx < 419371 ) )
      LOG(INFO) << " TS " << std::setw( 12 ) << fulCurrentTsIdx
                << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                << " MsInTs " << std::setw( 3 ) << uMsIdx
                << " DPB "    << std::setw( 2 ) << fuCurrDpbIdx
                << " Mess "   << std::setw( 5 ) << uMessIdx
                << " TsMsb "  << std::setw( 5 ) << uVal
                << FairLogger::endl;
*/
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

void CbmMcbm2018MonitorSts::FillEpochInfo( stsxyter::Message mess )
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

void CbmMcbm2018MonitorSts::Reset()
{
}

void CbmMcbm2018MonitorSts::Finish()
{

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MonitorSts statistics are " << FairLogger::endl;
   LOG(INFO) << " Hit      messages: " << fmMsgCounter[ stsxyter::MessType::Hit   ] << FairLogger::endl
             << " Ts MSB   messages: " << fmMsgCounter[ stsxyter::MessType::TsMsb ] << FairLogger::endl
             << " Dummy    messages: " << fmMsgCounter[ stsxyter::MessType::Dummy ] << FairLogger::endl
             << " Epoch    messages: " << fmMsgCounter[ stsxyter::MessType::Epoch ] << FairLogger::endl
             << " Empty    messages: " << fmMsgCounter[ stsxyter::MessType::Empty ] << FairLogger::endl;

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;

   SaveAllHistos( fsHistoFileFullname );
   SaveAllHistos();

}


void CbmMcbm2018MonitorSts::FillOutput(CbmDigi* /*digi*/)
{
}

void CbmMcbm2018MonitorSts::SaveAllHistos( TString sFileName )
{
   TDirectory * oldDir = NULL;
   TFile * histoFile = NULL;
   if( "" != sFileName )
   {
      // Store current directory position to allow restore later
      oldDir = gDirectory;
      // open separate histo file in recreate mode
      histoFile = new TFile( sFileName , "RECREATE");
      histoFile->cd();
   } // if( "" != sFileName )

   /***************************/
   gDirectory->mkdir("Sts_Raw");
   gDirectory->cd("Sts_Raw");

   fhStsMessType->Write();
   fhStsSysMessType->Write();
   fhStsMessTypePerDpb->Write();
   fhStsSysMessTypePerDpb->Write();

/*
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fhStsChanCntRaw[ uXyterIdx ]->Write();
      fhStsChanAdcRaw[ uXyterIdx ]->Write();
      fhStsChanAdcRawProf[ uXyterIdx ]->Write();
      fhStsChanRawTs[ uXyterIdx ]->Write();
      fhStsChanMissEvt[ uXyterIdx ]->Write();
      fhStsChanMissEvtEvo[ uXyterIdx ]->Write();
      fhStsAsicMissEvtEvo[ uXyterIdx ]->Write();
      fhStsChanHitRateEvo[ uXyterIdx ]->Write();
      fhStsAsicRateEvo[ uXyterIdx ]->Write();
      fhStsChanHitRateEvoLong[ uXyterIdx ]->Write();
      fhStsAsicRateEvoLong[ uXyterIdx ]->Write();

      if( kTRUE == fbLongHistoEnable )
      {
         fhFebRateEvoLong[ uXyterIdx ]->Write();
         fhFebChRateEvoLong[ uXyterIdx ]->Write();
      } // if( kTRUE == fbLongHistoEnable )

   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
*/
   gDirectory->cd("..");
   /***************************/

   /***************************/
   gDirectory->mkdir("Sts_Feb");
   gDirectory->cd("Sts_Feb");
   for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
   {
      fhStsFebChanCntRaw[ uFebIdx ]->Write();
      fhStsFebChanCntRawGood[ uFebIdx ]->Write();
      fhStsFebChanAdcRaw[ uFebIdx ]->Write();
      fhStsFebChanAdcRawProf[ uFebIdx ]->Write();
      fhStsFebChanRawTs[ uFebIdx ]->Write();
      fhStsFebChanMissEvt[ uFebIdx ]->Write();
      fhStsFebChanMissEvtEvo[ uFebIdx ]->Write();
      fhStsFebAsicMissEvtEvo[ uFebIdx ]->Write();
      fhStsFebMissEvtEvo[ uFebIdx ]->Write();
      fhStsFebChanHitRateEvo[ uFebIdx ]->Write();
      fhStsFebAsicHitRateEvo[ uFebIdx ]->Write();
      fhStsFebHitRateEvo[ uFebIdx ]->Write();
      fhStsFebChanHitRateEvoLong[ uFebIdx ]->Write();
      fhStsFebAsicHitRateEvoLong[ uFebIdx ]->Write();
      fhStsFebHitRateEvoLong[ uFebIdx ]->Write();
      for( UInt_t uFebIdxB = uFebIdx; uFebIdxB < fuNbFebs; ++uFebIdxB )
         fhStsFebChanCoinc[ uFebIdx ][ uFebIdxB ]->Write();
   } // for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
   gDirectory->cd("..");
   /***************************/

   /***************************/
   gDirectory->mkdir("Sts_Module");
   gDirectory->cd("Sts_Module");
   for( UInt_t uModIdx = 0; uModIdx < fuNbModules; ++ uModIdx )
   {
      fhStsModulePNCoincChan[ uModIdx ]->Reset();
      fhStsModulePNCoincAdc[ uModIdx ]->Reset();
      fhStsModuleCoincAdcChanP[ uModIdx ]->Reset();
      fhStsModuleCoincAdcChanN[ uModIdx ]->Reset();
      fhStsModuleCoincMap[ uModIdx ]->Reset();
   } // for( UInt_t uModIdx = 0; uModIdx < fuNbModules; ++ uModIdx )
   gDirectory->cd("..");
   /***************************/

   /***************************/
   // Flib Histos
   gDirectory->mkdir("Flib_Raw");
   gDirectory->cd("Flib_Raw");
   for( UInt_t uLinks = 0; uLinks < kiMaxNbFlibLinks; uLinks ++)
   {
      TString sMsSzName = Form("MsSz_link_%02u", uLinks);
      if( fHM->Exists(sMsSzName.Data() ) )
         fHM->H1( sMsSzName.Data() )->Write();

      sMsSzName = Form("MsSzTime_link_%02u", uLinks);
      if( fHM->Exists(sMsSzName.Data() ) )
         fHM->P1( sMsSzName.Data() )->Write();
   } // for( UInt_t uLinks = 0; uLinks < 16; uLinks ++)

   TH1 * pMissedTsH1    = dynamic_cast< TH1 * >( gROOT->FindObjectAny( "Missed_TS" ) );
   if( NULL != pMissedTsH1 )
      pMissedTsH1->Write();

   TProfile * pMissedTsEvoP = dynamic_cast< TProfile * >( gROOT->FindObjectAny( "Missed_TS_Evo" ) );
   if( NULL != pMissedTsEvoP )
      pMissedTsEvoP->Write();

   gDirectory->cd("..");
   /***************************/

   if( "" != sFileName )
   {
      // Restore original directory position
      histoFile->Close();
      oldDir->cd();
   } // if( "" != sFileName )

}
/*
void CbmMcbm2018MonitorSts::SavePulserHistos( TString sFileName )
{
   TDirectory * oldDir = NULL;
   TFile * histoFile = NULL;
   if( "" != sFileName )
   {
      // Store current directory position to allow restore later
      oldDir = gDirectory;
      // open separate histo file in recreate mode
      histoFile = new TFile( sFileName , "RECREATE");
      histoFile->cd();
   } // if( "" != sFileName )

   gDirectory->mkdir("Sts_Pulser");
   gDirectory->cd("Sts_Pulser");

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
      {
         fhStsTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Write();
         fhStsRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Write();
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

   gDirectory->cd("..");

   if( "" != sFileName )
   {
      // Restore original directory position
      histoFile->Close();
      oldDir->cd();
   } // if( "" != sFileName )

}
*/
void CbmMcbm2018MonitorSts::ResetAllHistos()
{
   LOG(INFO) << "Reseting all STS histograms." << FairLogger::endl;

   fhStsMessType->Reset();
   fhStsSysMessType->Reset();
   fhStsMessTypePerDpb->Reset();
   fhStsSysMessTypePerDpb->Reset();

/*
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fhStsChanCntRaw[ uXyterIdx ]->Reset();
      fhStsChanAdcRaw[ uXyterIdx ]->Reset();
      fhStsChanAdcRawProf[ uXyterIdx ]->Reset();
      fhStsChanRawTs[ uXyterIdx ]->Reset();
      fhStsChanMissEvt[ uXyterIdx ]->Reset();
      fhStsChanMissEvtEvo[ uXyterIdx ]->Reset();
      fhStsAsicMissEvtEvo[ uXyterIdx ]->Reset();
      fhStsChanHitRateEvo[ uXyterIdx ]->Reset();
      fhStsAsicRateEvo[ uXyterIdx ]->Reset();
      fhStsChanHitRateEvoLong[ uXyterIdx ]->Reset();
      fhStsAsicRateEvoLong[ uXyterIdx ]->Reset();

      if( kTRUE == fbLongHistoEnable )
      {
         ftStartTimeUnix = std::chrono::steady_clock::now();
         fhFebRateEvoLong[ uXyterIdx ]->Reset();
         fhFebChRateEvoLong[ uXyterIdx ]->Reset();
      } // if( kTRUE == fbLongHistoEnable )

   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
*/

   for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
   {
      fhStsFebChanCntRaw[ uFebIdx ]->Reset();
      fhStsFebChanCntRawGood[ uFebIdx ]->Reset();
      fhStsFebChanAdcRaw[ uFebIdx ]->Reset();
      fhStsFebChanAdcRawProf[ uFebIdx ]->Reset();
      fhStsFebChanRawTs[ uFebIdx ]->Reset();
      fhStsFebChanMissEvt[ uFebIdx ]->Reset();
      fhStsFebChanMissEvtEvo[ uFebIdx ]->Reset();
      fhStsFebAsicMissEvtEvo[ uFebIdx ]->Reset();
      fhStsFebMissEvtEvo[ uFebIdx ]->Reset();
      fhStsFebChanHitRateEvo[ uFebIdx ]->Reset();
      fhStsFebAsicHitRateEvo[ uFebIdx ]->Reset();
      fhStsFebHitRateEvo[ uFebIdx ]->Reset();
      fhStsFebChanHitRateEvoLong[ uFebIdx ]->Reset();
      fhStsFebAsicHitRateEvoLong[ uFebIdx ]->Reset();
      fhStsFebHitRateEvoLong[ uFebIdx ]->Reset();
      for( UInt_t uFebIdxB = uFebIdx; uFebIdxB < fuNbFebs; ++uFebIdxB )
         fhStsFebChanCoinc[ uFebIdx ][ uFebIdxB ]->Reset();
   } // for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )

   for( UInt_t uModIdx = 0; uModIdx < fuNbModules; ++ uModIdx )
   {
      fhStsModulePNCoincChan[ uModIdx ]->Reset();
      fhStsModulePNCoincAdc[ uModIdx ]->Reset();
      fhStsModuleCoincAdcChanP[ uModIdx ]->Reset();
      fhStsModuleCoincAdcChanN[ uModIdx ]->Reset();
      fhStsModuleCoincMap[ uModIdx ]->Reset();
   } // for( UInt_t uModIdx = 0; uModIdx < fuNbModules; ++ uModIdx )

   for( UInt_t uLinks = 0; uLinks < kiMaxNbFlibLinks; ++uLinks )
   {
      TString sMsSzName = Form("MsSz_link_%02u", uLinks);
      if( fHM->Exists(sMsSzName.Data() ) )
         fHM->H1( sMsSzName.Data() )->Reset();

      sMsSzName = Form("MsSzTime_link_%02u", uLinks);
      if( fHM->Exists(sMsSzName.Data() ) )
         fHM->P1( sMsSzName.Data() )->Reset();
   } // for( UInt_t uLinks = 0; uLinks < kiMaxNbFlibLinks; ++uLinks )

  fdStartTime = -1;
  fdStartTimeMsSz = -1;
}
/*
void CbmMcbm2018MonitorSts::UpdatePairMeanValue( UInt_t uAsicA, UInt_t uAsicB, Double_t dNewValue )
{
   if( fvuLastTimeDiffSlotAsicPair.size() < uAsicA )
   {
      LOG(WARNING) << "CbmMcbm2018MonitorSts::UpdatePairMeanValue => wrong ASIC A value: " << uAsicA
                   << " VS " << fvuLastTimeDiffSlotAsicPair.size()
                   << FairLogger::endl;
      return;
   } // if( fvuLastTimeDiffSlotAsicPair.size() < uAsicA )
   if( fvuLastTimeDiffSlotAsicPair[ uAsicA ].size() < uAsicB )
   {
      LOG(WARNING) << "CbmMcbm2018MonitorSts::UpdatePairMeanValue => wrong ASIC B value: " << uAsicB
                   << " VS " << fvuLastTimeDiffSlotAsicPair[ uAsicA ].size()
                   << FairLogger::endl;
      return;
   } // if( fvuLastTimeDiffSlotAsicPair[ uAsicA ].size() < uAsicB )

   if( kuNbValuesForTimeDiffMean == fvdLastTimeDiffValuesAsicPair[ uAsicA ][ uAsicB ].size() )
   {
      fvdLastTimeDiffValuesAsicPair[ uAsicA ][ uAsicB ][ fvuLastTimeDiffSlotAsicPair[ uAsicA ][ uAsicB ] ] = dNewValue;
      fvuLastTimeDiffSlotAsicPair[ uAsicA ][ uAsicB ] = ( fvuLastTimeDiffSlotAsicPair[ uAsicA ][ uAsicB ] + 1 )
                                                         % kuNbValuesForTimeDiffMean;
   } // if( kuNbValuesForTimeDiffMean == fvdLastTimeDiffValuesAsicPair[ uAsicA ][ uAsicB ].size() )
      else fvdLastTimeDiffValuesAsicPair[ uAsicA ][ uAsicB ].push_back( dNewValue );

   Double_t dNewMean = 0.0;
   UInt_t uNbVal = fvdLastTimeDiffValuesAsicPair[ uAsicA ][ uAsicB ].size();
   for( UInt_t uIdx = 0; uIdx < uNbVal; ++uIdx )
      dNewMean += fvdLastTimeDiffValuesAsicPair[ uAsicA ][ uAsicB ][ uIdx ];
   dNewMean /= uNbVal;

   fvdMeanTimeDiffAsicPair[ uAsicA ][ uAsicB ] = dNewMean;
}
*/
void CbmMcbm2018MonitorSts::SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize )
{
   TDatime * fRunStartDateTime     = new TDatime( dateIn, timeIn);
   fiRunStartDateTimeSec = fRunStartDateTime->Convert();
   fiBinSizeDatePlots    = iBinSize;

   LOG(INFO) << "Assigned new MUCH Run Start Date-Time: " << fRunStartDateTime->AsString() << FairLogger::endl;
}

void CbmMcbm2018MonitorSts::SetLongDurationLimits( UInt_t uDurationSeconds, UInt_t uBinSize )
{
   fbLongHistoEnable     = kTRUE;
   fuLongHistoNbSeconds  = uDurationSeconds;
   fuLongHistoBinSizeSec = uBinSize;
}

ClassImp(CbmMcbm2018MonitorSts)
