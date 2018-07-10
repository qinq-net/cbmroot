// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                   CbmMcbm2018MonitorStsSync                        -----
// -----                Created 11/05/18  by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018MonitorStsSync.h"

// Data

// CbmRoot
#include "CbmCern2017UnpackParHodo.h"
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

// C/C++
#include <iostream>
#include <stdint.h>
#include <iomanip>

Bool_t bMcbm2018ResetSync = kFALSE;
Bool_t bMcbm2018WriteSync = kFALSE;

CbmMcbm2018MonitorStsSync::CbmMcbm2018MonitorStsSync() :
   CbmMcbmUnpack(),
   fvMsComponentsList(),
   fuNbCoreMsPerTs(0),
   fuNbOverMsPerTs(0),
   fbIgnoreOverlapMs(kFALSE),
   fUnpackParHodo(NULL),
   fuNrOfDpbs(0),
   fDpbIdIndexMap(),
   fuNbElinksPerDpb(0),
   fuNbStsXyters(0),
   fuNbChanPerAsic(0),
   fvuElinkToAsic(),
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
   fhPulserMessType(NULL),
   fhPulserSysMessType(NULL),
   fhPulserMessTypePerDpb(NULL),
   fhPulserSysMessTypePerDpb(NULL),
   fhPulserMessTypePerElink(NULL),
   fhPulserSysMessTypePerElink(NULL),
   fhPulserChanCntRaw(),
   fhPulserChanCntRawGood(),
   fhPulserChanAdcRaw(),
   fhPulserChanAdcRawProf(),
   fhPulserChanRawTs(),
   fhPulserChanMissEvt(),
   fhPulserChanMissEvtEvo(),
   fhPulserChanHitRateEvo(),
   fhPulserFebRateEvo(),
   fhPulserFebMissEvtEvo(),
   fhPulserChanHitRateEvoLong(),
   fhPulserFebRateEvoLong(),
   fcMsSizeAll(NULL),
   fdStartTs( 0.0 ),
   fvmLastHitAsic(),
   fhPulserTimeDiffPerAsic(),
   fhPulserTimeDiffPerAsicPair(),
   fhPulserTimeDiffEvoPerAsicPair(),
   fhPulserTimeDiffEvoPerAsicPairProf(),
   fhPulserRawTimeDiffEvoPerAsicPairProf(),
   fhPulserTsLsbMatchPerAsicPair(),
   fhPulserTsMsbMatchPerAsicPair(),
   fhPulserIntervalAsic(),
   fhPulserIntervalLongAsic(),
   fvdLastTimeDiffValuesAsicPair(),
   fvuLastTimeDiffSlotAsicPair(),
   fvdMeanTimeDiffAsicPair()
{
}

CbmMcbm2018MonitorStsSync::~CbmMcbm2018MonitorStsSync()
{
}

Bool_t CbmMcbm2018MonitorStsSync::Init()
{
  LOG(INFO) << "Initializing flib StsXyter unpacker for STS" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  return kTRUE;
}

void CbmMcbm2018MonitorStsSync::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
         << FairLogger::endl;
   fUnpackParHodo = (CbmCern2017UnpackParHodo*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmCern2017UnpackParHodo"));
}


Bool_t CbmMcbm2018MonitorStsSync::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
         << FairLogger::endl;

   Bool_t bReInit = ReInitContainers();
   CreateHistograms();

   return bReInit;
}

Bool_t CbmMcbm2018MonitorStsSync::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
         << FairLogger::endl;

   fuNrOfDpbs       = fUnpackParHodo->GetNrOfDpbs();
   fuNbElinksPerDpb = fUnpackParHodo->GetNbElinksPerDpb();
   fuNbStsXyters    = fUnpackParHodo->GetNbStsXyters();
   fuNbChanPerAsic  = fUnpackParHodo->GetNbChanPerAsic();


   LOG(INFO) << "Nr. of STS DPBs:       " << fuNrOfDpbs
             << FairLogger::endl;

   fDpbIdIndexMap.clear();
   fvuElinkToAsic.resize( fuNrOfDpbs );
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fDpbIdIndexMap[ fUnpackParHodo->GetDpbId( uDpb )  ] = uDpb;
      LOG(INFO) << "Eq. ID for DPB #" << std::setw(2) << uDpb << " = "
                << std::setw(4) << std::hex << fUnpackParHodo->GetDpbId( uDpb )
                << std::dec
                << " => " << fDpbIdIndexMap[ fUnpackParHodo->GetDpbId( uDpb )  ]
                << FairLogger::endl;

      fvuElinkToAsic[uDpb].resize( fuNbElinksPerDpb );
      for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
         fvuElinkToAsic[uDpb][uElink] = fUnpackParHodo->GetElinkToAsicIdx( uDpb * fuNbElinksPerDpb + uElink );
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   LOG(INFO) << "Nr. eLinks per DPB:    " << fuNbElinksPerDpb
             << FairLogger::endl;
   LOG(INFO) << "Nr. of StsXyter ASICs: " << fuNbStsXyters
             << FairLogger::endl;
   LOG(INFO) << "Nb. channels per ASIC: " << fuNbChanPerAsic
             << FairLogger::endl;

   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      LOG(INFO) << "ASIC ID for eLinks in DPB #" << std::setw(2) << uDpb << ": ";

      for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
      {
         if( 0 == uElink % 10 )
            LOG(INFO) << FairLogger::endl
                      << "------> ";

         LOG(INFO) << std::setw( 5 ) << fvuElinkToAsic[uDpb][uElink] << " ";
      } // for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
      LOG(INFO) << FairLogger::endl;
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   // Internal status initialization
   fvulCurrentTsMsb.resize( fuNrOfDpbs );
   fvuCurrentTsMsbCycle.resize( fuNrOfDpbs );
   fvuElinkLastTsHit.resize( fuNrOfDpbs );
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fvulCurrentTsMsb[uDpb]     = 0;
      fvuCurrentTsMsbCycle[uDpb] = 0;
/*
      fvuElinkLastTsHit[uDpb].resize( fuNbElinksPerDpb );
      for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
      {
         fvuElinkLastTsHit[uDpb][uElink]  = 0;
      } // for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
*/
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   fvulChanLastHitTime.resize( fuNbStsXyters );
   fvdChanLastHitTime.resize( fuNbStsXyters );
   fvdMsTime.resize( fuMaxNbMicroslices );
   fvuChanNbHitsInMs.resize( fuNbStsXyters );
   fvdChanLastHitTimeInMs.resize( fuNbStsXyters );
   fvusChanLastHitAdcInMs.resize( fuNbStsXyters );
   fvmAsicHitsInMs.resize( fuNbStsXyters );
   fvmLastHitAsic.resize( fuNbStsXyters );
   fhPulserTimeDiffPerAsicPair.resize( fuNbStsXyters );
   fhPulserTimeDiffEvoPerAsicPair.resize( fuNbStsXyters );
   fhPulserTimeDiffEvoPerAsicPairProf.resize( fuNbStsXyters );
   fhPulserRawTimeDiffEvoPerAsicPairProf.resize( fuNbStsXyters );
   fhPulserTsLsbMatchPerAsicPair.resize( fuNbStsXyters );
   fhPulserTsMsbMatchPerAsicPair.resize( fuNbStsXyters );
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fvulChanLastHitTime[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvdChanLastHitTime[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvuChanNbHitsInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvdChanLastHitTimeInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvusChanLastHitAdcInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvmAsicHitsInMs[ uXyterIdx ].clear();
      fhPulserTimeDiffPerAsicPair[ uXyterIdx ].clear();
      fhPulserTimeDiffEvoPerAsicPair[ uXyterIdx ].clear();
      fhPulserTimeDiffEvoPerAsicPairProf[ uXyterIdx ].clear();
      fhPulserRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ].clear();
      fhPulserTsLsbMatchPerAsicPair[ uXyterIdx ].clear();
      fhPulserTsMsbMatchPerAsicPair[ uXyterIdx ].clear();
      for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
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
      } // for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   LOG(INFO) << "CbmMcbm2018MonitorStsSync::ReInitContainers => Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MonitorStsSync::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fuNbChanPerAsic
                << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MonitorStsSync::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuMaxNbMicroslices
                << FairLogger::endl;

   return kTRUE;
}

void CbmMcbm2018MonitorStsSync::AddMsComponentToList( size_t component )
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
void CbmMcbm2018MonitorStsSync::SetNbMsInTs( size_t uCoreMsNb, size_t uOverlapMsNb )
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
         fvuChanNbHitsInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
         fvdChanLastHitTimeInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
         fvusChanLastHitAdcInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
         for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
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
         } // for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
      } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      LOG(INFO) << "CbmMcbm2018MonitorStsSync::DoUnpack => Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                   << FairLogger::endl;
      LOG(INFO) << "CbmMcbm2018MonitorStsSync::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fuNbChanPerAsic
                   << FairLogger::endl;
      LOG(INFO) << "CbmMcbm2018MonitorStsSync::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuMaxNbMicroslices
                   << FairLogger::endl;
   } // if( fuMaxNbMicroslices < uNbMsTotal )
}

void CbmMcbm2018MonitorStsSync::SetCoincidenceBorder( Double_t dCenterPos, Double_t dBorderVal )
{
   fdCoincCenter = dCenterPos;
   fdCoincBorder = dBorderVal;
   fdCoincMin    = dCenterPos - dBorderVal;
   fdCoincMax    = dCenterPos + dBorderVal;
}

void CbmMcbm2018MonitorStsSync::CreateHistograms()
{
   TString sHistName{""};
   TString title{""};

   sHistName = "hPulserMessageType";
   title = "Nb of message for each type; Type";
   fhPulserMessType = new TH1I(sHistName, title, 5, 0., 5.);
   fhPulserMessType->GetXaxis()->SetBinLabel( 1, "Dummy");
   fhPulserMessType->GetXaxis()->SetBinLabel( 2, "Hit");
   fhPulserMessType->GetXaxis()->SetBinLabel( 3, "TsMsb");
   fhPulserMessType->GetXaxis()->SetBinLabel( 4, "Epoch");
   fhPulserMessType->GetXaxis()->SetBinLabel( 5, "Empty");
/* *** Missing int + MessType OP!!!! ****
   fhPulserMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::Dummy,       "Dummy");
   fhPulserMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::Hit,         "Hit");
   fhPulserMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::TsMsb,       "TsMsb");
   fhPulserMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::ReadDataAck, "ReadDataAck");
   fhPulserMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::Ack,         "Ack");
*/

   sHistName = "hPulserSysMessType";
   title = "Nb of system message for each type; System Type";
   fhPulserSysMessType = new TH1I(sHistName, title, 17, 0., 17.);
/*
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   hSysMessType->GetXaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/

   sHistName = "hPulserMessageTypePerDpb";
   title = "Nb of message of each type for each DPB; DPB; Type";
   fhPulserMessTypePerDpb = new TH2I(sHistName, title, fuNrOfDpbs, 0, fuNrOfDpbs, 5, 0., 5.);
   fhPulserMessTypePerDpb->GetYaxis()->SetBinLabel( 1, "Dummy");
   fhPulserMessTypePerDpb->GetYaxis()->SetBinLabel( 2, "Hit");
   fhPulserMessTypePerDpb->GetYaxis()->SetBinLabel( 3, "TsMsb");
   fhPulserMessTypePerDpb->GetYaxis()->SetBinLabel( 4, "Epoch");
   fhPulserMessTypePerDpb->GetYaxis()->SetBinLabel( 5, "Empty");
/* *** Missing int + MessType OP!!!! ****
   fhPulserMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Dummy,       "Dummy");
   fhPulserMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Hit,         "Hit");
   fhPulserMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::TsMsb,       "TsMsb");
   fhPulserMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::ReadDataAck, "ReadDataAck");
   fhPulserMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Ack,         "Ack");
*/

   sHistName = "hPulserSysMessTypePerDpb";
   title = "Nb of system message of each type for each DPB; DPB; System Type";
   fhPulserSysMessTypePerDpb = new TH2I(sHistName, title, fuNrOfDpbs, 0, fuNrOfDpbs, 17, 0., 17.);
/*
   hSysMessType->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   hSysMessType->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   hSysMessType->GetYaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/

   sHistName = "hPulserMessageTypePerElink";
   title = "Nb of message of each type for each eLink; eLink; Type";
   fhPulserMessTypePerElink = new TH2I(sHistName, title, fuNrOfDpbs * fuNbElinksPerDpb, 0, fuNrOfDpbs * fuNbElinksPerDpb, 5, 0., 5.);
   fhPulserMessTypePerElink->GetYaxis()->SetBinLabel( 1,       "Dummy");
   fhPulserMessTypePerElink->GetYaxis()->SetBinLabel( 2,         "Hit");
   fhPulserMessTypePerElink->GetYaxis()->SetBinLabel( 3,       "TsMsb");
   fhPulserMessTypePerElink->GetYaxis()->SetBinLabel( 4, "ReadDataAck");
   fhPulserMessTypePerElink->GetYaxis()->SetBinLabel( 5,         "Ack");
/* *** Missing int + MessType OP!!!! ****
   fhPulserMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Dummy,       "Dummy");
   fhPulserMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Hit,         "Hit");
   fhPulserMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::TsMsb,       "TsMsb");
   fhPulserMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::ReadDataAck, "ReadDataAck");
   fhPulserMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Ack,         "Ack");
*/

   sHistName = "hPulserSysMessTypePerElink";
   title = "Nb of system message of each type for each eLink; eLink; System Type";
   fhPulserSysMessTypePerElink = new TH2I(sHistName, title, fuNrOfDpbs * fuNbElinksPerDpb, 0, fuNrOfDpbs * fuNbElinksPerDpb, 17, 0., 17.);
/*
   fhPulserSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   fhPulserSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   fhPulserSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
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

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      // Channel counts
      sHistName = Form( "hPulserChanCntRaw_%03u", uXyterIdx );
      title = Form( "Hits Count per channel, StsXyter #%03u; Channel; Hits []", uXyterIdx );
      fhPulserChanCntRaw.push_back( new TH1I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );

      sHistName = Form( "hPulserChanCntRawGood_%03u", uXyterIdx );
      title = Form( "Hits Count per channel in good MS, StsXyter #%03u; Channel; Hits []", uXyterIdx );
      fhPulserChanCntRawGood.push_back( new TH1I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );

      // Raw Adc Distribution
      sHistName = Form( "hPulserChanAdcRaw_%03u", uXyterIdx );
      title = Form( "Raw Adc distribution per channel, StsXyter #%03u; Channel []; Adc []; Hits []", uXyterIdx );
      fhPulserChanAdcRaw.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 stsxyter::kuHitNbAdcBins, -0.5, stsxyter::kuHitNbAdcBins -0.5 ) );

      // Raw Adc Distribution profile
      sHistName = Form( "hPulserChanAdcRawProfc_%03u", uXyterIdx );
      title = Form( "Raw Adc prodile per channel, StsXyter #%03u; Channel []; Adc []", uXyterIdx );
      fhPulserChanAdcRawProf.push_back( new TProfile(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );

      // Raw Ts Distribution
      sHistName = Form( "hPulserChanRawTs_%03u", uXyterIdx );
      title = Form( "Raw Timestamp distribution per channel, StsXyter #%03u; Channel []; Ts []; Hits []", uXyterIdx );
      fhPulserChanRawTs.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 stsxyter::kuHitNbTsBins, -0.5, stsxyter::kuHitNbTsBins -0.5 ) );

      // Missed event flag
      sHistName = Form( "hPulserChanMissEvt_%03u", uXyterIdx );
      title = Form( "Missed Event flags per channel, StsXyter #%03u; Channel []; Miss Evt []; Hits []", uXyterIdx );
      fhPulserChanMissEvt.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 2, -0.5, 1.5 ) );

      // Missed event flag counts evolution

      sHistName = Form( "hPulserChanMissEvtEvo_%03u", uXyterIdx );
      title = Form( "Missed Evt flags per second & channel in StsXyter #%03u; Time [s]; Channel []; Missed Evt flags []", uXyterIdx );
      fhPulserChanMissEvtEvo.push_back( new TH2I( sHistName, title,
                                                1800, 0, 1800,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );

      // Missed event flag counts evo per StsXyter

      sHistName = Form( "hPulserFebMissEvtEvo%03u", uXyterIdx );
      title = Form( "Missed Evt flags per second in StsXyter #%03u; Time [s]; Missed Evt flags []", uXyterIdx );
      fhPulserFebMissEvtEvo.push_back( new TH1I(sHistName, title, 1800, 0, 1800 ) );

      // Hit rates evo per channel
      sHistName = Form( "hPulserChanRateEvo_%03u", uXyterIdx );
      title = Form( "Hits per second & channel in StsXyter #%03u; Time [s]; Channel []; Hits []", uXyterIdx );
      fhPulserChanHitRateEvo.push_back( new TH2I( sHistName, title,
                                                1800, 0, 1800,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );

      // Hit rates evo per StsXyter
      sHistName = Form( "hPulserFebRateEvo_%03u", uXyterIdx );
      title = Form( "Hits per second in StsXyter #%03u; Time [s]; Hits []", uXyterIdx );
      fhPulserFebRateEvo.push_back( new TH1I(sHistName, title, 1800, 0, 1800 ) );

      // Hit rates evo per channel, 1 minute bins, 24h
      sHistName = Form( "hPulserChanRateEvoLong_%03u", uXyterIdx );
      title = Form( "Hits per second & channel in StsXyter #%03u; Time [min]; Channel []; Hits []", uXyterIdx );
      fhPulserChanHitRateEvoLong.push_back( new TH2D( sHistName, title,
                                                fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );

      // Hit rates evo per StsXyter, 1 minute bins, 24h
      sHistName = Form( "hPulserFebRateEvoLong_%03u", uXyterIdx );
      title = Form( "Hits per second in StsXyter #%03u; Time [min]; Hits []", uXyterIdx );
      fhPulserFebRateEvoLong.push_back( new TH1D(sHistName, title,
                                                fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5 ) );

      /// Coincidences inside each detector ----------------------------///
      sHistName = Form( "fhPulserTimeDiffPerAsic_%03u", uXyterIdx );
      title =  Form( "Time diff for pulser hits between ASIC %03u and other ASICs; tn - t%03u [ns]; ASIC n; Counts", uXyterIdx, uXyterIdx );
      fhPulserTimeDiffPerAsic.push_back( new TH2I( sHistName, title,
                                                   uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo,
                                                   fuNbStsXyters, -0.5, fuNbStsXyters - 0.5 )
                                       );

      for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
      {
         if( uXyterIdxB == uXyterIdx )
         {
            sHistName = Form( "fhPulserTimeDiffSameAsic_%03u", uXyterIdx );
            title =  Form( "Time diff for consecutive hits in ASIC %03u; tn - t [ns]; Counts", uXyterIdx );
         } // if( uXyterIdxB == uXyterIdx )
            else
            {
               sHistName = Form( "fhPulserTimeDiffPerAsicPair_%03u_%03u", uXyterIdx, uXyterIdxB );
               title =  Form( "Time diff for pulser hits in ASIC %03u and %03u; tn - t [ns]; Counts", uXyterIdx, uXyterIdxB );
            } // else of if( uXyterIdxB == uXyterIdx )
         fhPulserTimeDiffPerAsicPair[ uXyterIdx ].push_back( new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo ) );

         if( uXyterIdxB == uXyterIdx )
         {
            sHistName = Form( "fhPulserTimeDiffEvoSameAsic_%03u", uXyterIdx );
            title =  Form( "Time diff for consecutive hits in ASIC %03u; Time in run [s]; tn - t [ns]; Counts", uXyterIdx );
         } // if( uXyterIdxB == uXyterIdx )
            else
            {
               sHistName = Form( "fhPulserTimeDiffEvoPerAsicPair_%03u_%03u", uXyterIdx, uXyterIdxB );
               title =  Form( "Time diff for pulser hits in ASIC %03u and %03u; Time in run [s]; tn - t [ns]; Counts", uXyterIdx, uXyterIdxB );
            } // else of if( uXyterIdxB == uXyterIdx )
         fhPulserTimeDiffEvoPerAsicPair[ uXyterIdx ].push_back( new TH2I(sHistName, title,
                                                                3600, 0, 18000,
                                                                200, -100 * stsxyter::kdClockCycleNs, 100 * stsxyter::kdClockCycleNs ) );

         if( uXyterIdxB == uXyterIdx )
         {
            sHistName = Form( "fhPulserTimeDiffEvoSameAsicProf_%03u", uXyterIdx );
            title =  Form( "Time diff for consecutive hits in ASIC %03u; Time in run [s]; tn - t [ns]", uXyterIdx );
         } // if( uXyterIdxB == uXyterIdx )
            else
            {
               sHistName = Form( "fhPulserTimeDiffEvoPerAsicPairProf_%03u_%03u", uXyterIdx, uXyterIdxB );
               title =  Form( "Time diff for pulser hits in ASIC %03u and %03u; Time in run [s]; tn - t [ns]", uXyterIdx, uXyterIdxB );
            } // else of if( uXyterIdxB == uXyterIdx )
         fhPulserTimeDiffEvoPerAsicPairProf[ uXyterIdx ].push_back( new TProfile(sHistName, title, 7200, 0, 36000 ) );

         if( uXyterIdxB == uXyterIdx )
         {
            sHistName = Form( "fhPulserRawTimeDiffEvoSameAsicProf_%03u", uXyterIdx );
            title =  Form( "Time diff for consecutive hits in ASIC %03u; Time in run [s]; tn - t [ns]", uXyterIdx );
         } // if( uXyterIdxB == uXyterIdx )
            else
            {
               sHistName = Form( "fhPulserRawTimeDiffEvoPerAsicPairProf_%03u_%03u", uXyterIdx, uXyterIdxB );
               title =  Form( "Time diff for pulser hits in ASIC %03u and %03u; Time in run [s]; tn - t [ns]", uXyterIdx, uXyterIdxB );
            } // else of if( uXyterIdxB == uXyterIdx )
         fhPulserRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ].push_back( new TProfile(sHistName, title, 7200, 0, 36000 ) );

         sHistName = Form( "fhPulserTsLsbMatchPerAsicPair_%03u_%03u", uXyterIdx, uXyterIdxB );
         title =  Form( "TS LSB for pulser hits in ASIC %03u and %03u; TS LSB %03u [bin]; TS LSB %03u [bin]",
                        uXyterIdx, uXyterIdxB, uXyterIdx, uXyterIdxB );
         fhPulserTsLsbMatchPerAsicPair[ uXyterIdx ].push_back( new TH2I(sHistName, title, 256, -0.5, 255.5, 256, -0.5, 255.5 ) );

         sHistName = Form( "fhPulserTsMsbMatchPerAsicPair_%03u_%03u", uXyterIdx, uXyterIdxB );
         title =  Form( "TS MSB for pulser hits in ASIC %03u and %03u; TS MSB %03u [bin]; TS MSB %03u [bin]",
                        uXyterIdx, uXyterIdxB, uXyterIdx, uXyterIdxB );
         fhPulserTsMsbMatchPerAsicPair[ uXyterIdx ].push_back( new TH2I(sHistName, title, 64, -0.5, 63.5, 64, -0.5, 63.5 ) );
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )

      sHistName = Form( "fhPulserIntervalAsic_%03u", uXyterIdx );
      title =  Form( "Time diff between consecutive hits in ASIC %03us; dt [ns];  Counts", uXyterIdx, uXyterIdx );
      fhPulserIntervalAsic.push_back( new TH1I( sHistName, title, 200, 0, 200 * stsxyter::kdClockCycleNs ) );

      sHistName = Form( "fhPulserIntervalLongAsic_%03u", uXyterIdx );
      title =  Form( "Time diff between consecutive hits in ASIC %03us; dt [ns];  Counts", uXyterIdx, uXyterIdx );
      fhPulserIntervalLongAsic.push_back( new TH1I( sHistName, title, 1e5, 0, 1e6 * stsxyter::kdClockCycleNs ) );

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++///
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   fvdLastTimeDiffValuesAsicPair.resize( fhPulserTimeDiffPerAsicPair.size() );
   fvuLastTimeDiffSlotAsicPair.resize( fhPulserTimeDiffPerAsicPair.size() );
   fvdMeanTimeDiffAsicPair.resize( fhPulserTimeDiffPerAsicPair.size() );
   for( UInt_t uAsicA = 0; uAsicA < fhPulserTimeDiffPerAsicPair.size(); ++uAsicA )
   {
      fvdLastTimeDiffValuesAsicPair[uAsicA].resize( fhPulserTimeDiffPerAsicPair[uAsicA].size() );
      fvuLastTimeDiffSlotAsicPair[uAsicA].resize( fhPulserTimeDiffPerAsicPair[uAsicA].size(), 0 );
      fvdMeanTimeDiffAsicPair[uAsicA].resize( fhPulserTimeDiffPerAsicPair[uAsicA].size(), 0.0 );
   } // for( UInt_t uAsicA = 0; uAsicA < fhPulserTimeDiffPerAsicPair.size(); ++uAsicA )


///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++///

/*
   // Distribution of the TS_MSB per StsXyter
   sHistName = "hPulserFebTsMsb";
   title = "Raw Timestamp Msb distribution per StsXyter; Ts MSB []; StsXyter []; Hits []";
   fhPulserFebTsMsb = new TH2I( sHistName, title, stsxyter::kuTsMsbNbTsBins, -0.5,   stsxyter::kuTsMsbNbTsBins - 0.5,
                                                fuNbStsXyters, -0.5, fuNbStsXyters - 0.5 );
#ifdef USE_HTTP_SERVER
   if( server ) server->Register("/StsRaw", fhPulserFebTsMsb );
#endif
*/
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
      server->Register("/StsRaw", fhPulserMessType );
      server->Register("/StsRaw", fhPulserSysMessType );
      server->Register("/StsRaw", fhPulserMessTypePerDpb );
      server->Register("/StsRaw", fhPulserSysMessTypePerDpb );
      server->Register("/StsRaw", fhPulserMessTypePerElink );
      server->Register("/StsRaw", fhPulserSysMessTypePerElink );
      for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      {
         server->Register("/StsRaw", fhPulserChanCntRaw[ uXyterIdx ] );
         server->Register("/StsRaw", fhPulserChanCntRawGood[ uXyterIdx ] );
         server->Register("/StsRaw", fhPulserChanAdcRaw[ uXyterIdx ] );
         server->Register("/StsRaw", fhPulserChanAdcRawProf[ uXyterIdx ] );
         server->Register("/StsRaw", fhPulserChanRawTs[ uXyterIdx ] );
         server->Register("/StsRaw", fhPulserChanMissEvt[ uXyterIdx ] );
         server->Register("/StsRaw", fhPulserChanMissEvtEvo[ uXyterIdx ] );
         server->Register("/StsRaw", fhPulserFebMissEvtEvo[ uXyterIdx ] );
         server->Register("/StsRaw", fhPulserChanHitRateEvo[ uXyterIdx ] );
         server->Register("/StsRaw", fhPulserFebRateEvo[ uXyterIdx ] );
         server->Register("/StsRaw", fhPulserChanHitRateEvoLong[ uXyterIdx ] );
         server->Register("/StsRaw", fhPulserFebRateEvoLong[ uXyterIdx ] );

         server->Register("/DtAsic", fhPulserTimeDiffPerAsic[ uXyterIdx ] );
         for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
         {
            if( uXyterIdxB == uXyterIdx )
            {
               server->Register("/DtChan", fhPulserTimeDiffPerAsicPair[ uXyterIdx ][uXyterIdxB] );
               server->Register("/DtChan", fhPulserTimeDiffEvoPerAsicPair[ uXyterIdx ][uXyterIdxB] );
               server->Register("/DtChan", fhPulserTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB] );
               server->Register("/DtChan", fhPulserRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB] );
            } // if( uXyterIdxB == uXyterIdx )
               else
               {
                  server->Register("/DtAsicPair", fhPulserTimeDiffPerAsicPair[ uXyterIdx ][uXyterIdxB] );
                  server->Register("/DtAsicPair", fhPulserTimeDiffEvoPerAsicPair[ uXyterIdx ][uXyterIdxB] );
                  server->Register("/DtAsicPair", fhPulserTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB] );
                  server->Register("/DtAsicPair", fhPulserRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB] );
               } // else of if( uXyterIdxB == uXyterIdx )
            server->Register("/TsMatch", fhPulserTsLsbMatchPerAsicPair[ uXyterIdx ][uXyterIdxB] );
            server->Register("/TsMatch", fhPulserTsMsbMatchPerAsicPair[ uXyterIdx ][uXyterIdxB] );
         } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
         server->Register("/DtPulses", fhPulserIntervalAsic[ uXyterIdx ] );
         server->Register("/DtPulses", fhPulserIntervalLongAsic[ uXyterIdx ] );

      } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

      server->RegisterCommand("/Reset_All_Pulser", "bMcbm2018ResetSync=kTRUE");
      server->RegisterCommand("/Write_All_Pulser", "bMcbm2018WriteSync=kTRUE");

      server->Restrict("/Reset_All_Pulser", "allow=admin");
      server->Restrict("/Write_All_Pulser", "allow=admin");
   } // if( server )
#endif

   /** Create summary Canvases for CERN 2017 **/
   Double_t w = 10;
   Double_t h = 10;

      // Summary per StsXyter
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      TCanvas* cStsSumm = new TCanvas( Form("cStsSum_%03u", uXyterIdx ),
                                       Form("Summary plots for StsXyter %03u", uXyterIdx ),
                                       w, h);
      cStsSumm->Divide( 2, 2 );

      cStsSumm->cd(1);
      gPad->SetLogy();
      fhPulserChanCntRaw[ uXyterIdx ]->Draw();

      cStsSumm->cd(2);
      gPad->SetLogz();
      fhPulserChanAdcRaw[ uXyterIdx ]->Draw( "colz" );

      cStsSumm->cd(3);
      gPad->SetLogz();
      fhPulserChanHitRateEvo[ uXyterIdx ]->Draw( "colz" );

      cStsSumm->cd(4);
//      gPad->SetLogy();
      fhPulserChanAdcRawProf[ uXyterIdx ]->Draw();
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

//====================================================================//

//====================================================================//
   TCanvas* cDtPerAsic = new TCanvas( "cDtPerAsic",
                                      "Time Differences per ASIC",
                                    w, h);
   cDtPerAsic->Divide( fuNbStsXyters / 2 + fuNbStsXyters % 2, 2 );

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      cDtPerAsic->cd(1 + uXyterIdx);
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogz();
      fhPulserTimeDiffPerAsic[ uXyterIdx ]->Draw( " colz" );
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
//====================================================================//

//====================================================================//
   TCanvas* cDtInAsic = new TCanvas( "cDtInAsic",
                                      "Time Differences in ASIC",
                                    w, h);
   cDtInAsic->Divide( fuNbStsXyters / 2 + fuNbStsXyters % 2, 2 );

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      cDtInAsic->cd(1 + uXyterIdx);
      gPad->SetGridx();
      gPad->SetLogy();
      fhPulserTimeDiffPerAsicPair[ uXyterIdx ][ uXyterIdx ]->Draw( "hist" );
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
//====================================================================//

//====================================================================//
   TCanvas* cDtAsicPairs = new TCanvas( "cDtAsicPairs",
                                      "Time Differences in ASIC",
                                    w, h);
   cDtAsicPairs->Divide( 2, 3 );
   UInt_t uHistoIdx = 0;
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters && uXyterIdx < 3; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = uXyterIdx + 1; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
      {
         cDtAsicPairs->cd( 1 + uHistoIdx );
         gPad->SetGridx();
         gPad->SetLogy();
         fhPulserTimeDiffPerAsicPair[ uXyterIdx ][uXyterIdxB]->Draw( "hist" );

         uHistoIdx++;
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
//====================================================================//

//====================================================================//
   TCanvas* cDtAsicPairsEvo = new TCanvas( "cDtAsicPairsEvo",
                                      "Time Differences Evo in ASIC",
                                    w, h);
   cDtAsicPairsEvo->Divide( 2, 3 );
   uHistoIdx = 0;
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters && uXyterIdx < 3; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = uXyterIdx + 1; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
      {
         cDtAsicPairsEvo->cd( 1 + uHistoIdx );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();
         fhPulserTimeDiffEvoPerAsicPair[ uXyterIdx ][uXyterIdxB]->Draw( "colz" );

         uHistoIdx++;
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
//====================================================================//

//====================================================================//
   TCanvas* cDtAsicPairsEvoProf = new TCanvas( "cDtAsicPairsEvoProf",
                                      "Time Differences Evo in ASIC",
                                    w, h);
   cDtAsicPairsEvoProf->Divide( 2, 3 );
   uHistoIdx = 0;
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters && uXyterIdx < 3; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = uXyterIdx + 1; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
      {
         cDtAsicPairsEvoProf->cd( 1 + uHistoIdx );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();
         fhPulserTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Draw( "hist e0" );

         uHistoIdx++;
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
//====================================================================//

//====================================================================//
   TCanvas* cDtAsicPairsEvoProfRaw = new TCanvas( "cDtAsicPairsEvoProfRaw",
                                      "Time Differences Evo in ASIC",
                                    w, h);
   cDtAsicPairsEvoProfRaw->Divide( 2, 3 );
   uHistoIdx = 0;
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters && uXyterIdx < 3; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = uXyterIdx + 1; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
      {
         cDtAsicPairsEvoProfRaw->cd( 1 + uHistoIdx );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();
         fhPulserRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Draw( "hist e0" );

         uHistoIdx++;
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
//====================================================================//

//====================================================================//
   TCanvas* cTsLsbAsicPairs = new TCanvas( "cTsLsbAsicPairs",
                                      "Time Differences in ASIC",
                                    w, h);
   cTsLsbAsicPairs->Divide( 2, 3 );
   uHistoIdx = 0;
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters && uXyterIdx < 3; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = uXyterIdx + 1; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
      {
         cTsLsbAsicPairs->cd( 1 +uHistoIdx );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();
         fhPulserTsLsbMatchPerAsicPair[ uXyterIdx ][uXyterIdxB]->Draw( "colz" );

         uHistoIdx++;
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
//====================================================================//

//====================================================================//
   TCanvas* cTsMsbAsicPairs = new TCanvas( "cTsMsbAsicPairs",
                                      "Time Differences in ASIC",
                                    w, h);
//   cDtAsicPairs->Divide( fuNbStsXyters / 3, 3 );
   cTsMsbAsicPairs->Divide( 2, 3 );
   uHistoIdx = 0;
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters && uXyterIdx < 3; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = uXyterIdx + 1; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
      {
         cTsMsbAsicPairs->cd( 1 + uHistoIdx );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();
         fhPulserTsMsbMatchPerAsicPair[ uXyterIdx ][uXyterIdxB]->Draw( "colz" );

         uHistoIdx++;
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
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

Bool_t CbmMcbm2018MonitorStsSync::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   if( bMcbm2018ResetSync )
   {
      ResetAllHistos();
      bMcbm2018ResetSync = kFALSE;
   } // if( bMcbm2018ResetSync )
   if( bMcbm2018WriteSync )
   {
      SaveAllHistos( fsHistoFileFullname );
      bMcbm2018WriteSync = kFALSE;
   } // if( bMcbm2018WriteSync )

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
            fhPulserMessType->Fill( static_cast< uint16_t > (typeMess) );
            fhPulserMessTypePerDpb->Fill( fuCurrDpbIdx, static_cast< uint16_t > (typeMess) );

            switch( typeMess )
            {
               case stsxyter::MessType::Hit :
               {
                  // Extract the eLink and Asic indices => Should GO IN the fill method now that obly hits are link/asic specific!
                  UShort_t usElinkIdx = mess.GetLinkIndex();
                  if( fuNbElinksPerDpb <= usElinkIdx )
                  {
                     LOG(FATAL) << "CbmMcbm2018MonitorStsSync::DoUnpack => "
                                << "eLink index out of bounds!"
                                << usElinkIdx << " VS " << fuNbElinksPerDpb
                                << FairLogger::endl;
                  } // if( fuNbElinksPerDpb <= usElinkIdx )
                  fhPulserMessTypePerElink->Fill( fuCurrDpbIdx * fuNbElinksPerDpb + usElinkIdx,
                                                  static_cast< uint16_t > (typeMess) );

                  UInt_t   uAsicIdx   = fvuElinkToAsic[fuCurrDpbIdx][usElinkIdx];

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
                     LOG(INFO) << "CbmMcbm2018MonitorStsSync::DoUnpack => "
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
                  LOG(FATAL) << "CbmMcbm2018MonitorStsSync::DoUnpack => "
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
         ULong64_t ulLastHitTime = ( *( fvmHitsInMs.rbegin() ) ).GetTs();
         std::vector< stsxyter::FinalHit >::iterator it;
         std::vector< stsxyter::FinalHit >::iterator itB;

         std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();
         Double_t dUnixTimeInRun = std::chrono::duration_cast< std::chrono::seconds >(tNow - ftStartTimeUnix).count();

         for( it  = fvmHitsInMs.begin();
              it != fvmHitsInMs.end();
//              it != fvmHitsInMs.end() && (*it).GetTs() < ulLastHitTime - 320; // 320 * 3.125 ns = 1000 ns
              ++it )
         {
            UShort_t usAsicIdx = (*it).GetAsic();
            UShort_t usChanIdx = (*it).GetChan();
            ULong64_t ulHitTs  = (*it).GetTs();
            UShort_t  usHitAdc = (*it).GetAdc();

            Double_t dTimeSinceStartSec = (ulHitTs * stsxyter::kdClockCycleNs - fdStartTime)* 1e-9;

            fvmAsicHitsInMs[ usAsicIdx ].push_back( (*it) );
         } // loop on time sorted hits and split per asic

         // Remove all hits which were already used
         fvmHitsInMs.erase( fvmHitsInMs.begin(), it );

         for( UInt_t uAsic = 0; uAsic < fuNbStsXyters; uAsic++)
         {
            for( it  = fvmAsicHitsInMs[ uAsic ].begin(); it != fvmAsicHitsInMs[ uAsic ].end(); ++it )
            {
               UShort_t usChanIdx = (*it).GetChan();
               if( 0.0 == fdStartTs )
                  fdStartTs = (*it).GetTs() * stsxyter::kdClockCycleNs;
               Double_t dTimeSinceStartSec = ( (*it).GetTs() * stsxyter::kdClockCycleNs - fdStartTs ) * 1e-9;

               for( UInt_t uAsicB = uAsic; uAsicB < fuNbStsXyters; uAsicB++)
               {
                  for( itB  = fvmAsicHitsInMs[ uAsicB ].begin(); itB != fvmAsicHitsInMs[ uAsicB ].end(); ++itB )
                  {
                     UShort_t usChanIdxB = (*itB).GetChan();
                     Double_t dDt = ( static_cast< Double_t >( (*itB).GetTs() ) - static_cast< Double_t >( (*it).GetTs() )
                                    ) * stsxyter::kdClockCycleNs;
                     Double_t dDtRaw = ( static_cast< Double_t >( (*itB).GetTs() % stsxyter::kuTsCycleNbBins )
                                       - static_cast< Double_t >( (*it).GetTs()  % stsxyter::kuTsCycleNbBins )
                                    ) * stsxyter::kdClockCycleNs;

                     fhPulserTimeDiffPerAsic[ uAsic ]->Fill( dDt, uAsicB );
                     fhPulserTimeDiffPerAsicPair[ uAsic ][ uAsicB ]->Fill( dDt );
                     fhPulserTimeDiffEvoPerAsicPair[ uAsic ][ uAsicB ]->Fill( dTimeSinceStartSec, dDt );
                     fhPulserTimeDiffEvoPerAsicPairProf[ uAsic ][ uAsicB ]->Fill( dTimeSinceStartSec, dDt );
                     fhPulserRawTimeDiffEvoPerAsicPairProf[ uAsic ][ uAsicB ]->Fill( dTimeSinceStartSec, dDtRaw );

                     fhPulserTsLsbMatchPerAsicPair[ uAsic ][ uAsicB ]->Fill(   (*it ).GetTs() & 0x000FF,
                                                                               (*itB).GetTs() & 0x000FF );
                     fhPulserTsMsbMatchPerAsicPair[ uAsic ][ uAsicB ]->Fill( ( (*it ).GetTs() & 0x03F00 ) >> 8,
                                                                             ( (*itB).GetTs() & 0x03F00 ) >> 8 );

                     if( kdPulserPeriod * -0.5 < dDt && dDt < kdPulserPeriod * 0.5 )
                     {
                        if( dDt < fvdMeanTimeDiffAsicPair[ uAsic ][ uAsicB ] - kdTimeDiffToMeanMargin ||
                            dDt > fvdMeanTimeDiffAsicPair[ uAsic ][ uAsicB ] + kdTimeDiffToMeanMargin
                          )
                        {
                           LOG(INFO) << "CbmCosy2018MonitorPulser::DoUnpack => "
                                     << " TS " << std::setw( 12 ) << fulCurrentTsIdx
                                     << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                                     << " Potential jump in timestamp for ASIC "
                                     << uAsic << " and " << uAsicB
                                     << ": time diff is " << std::setw( 8 ) << dDt
                                     << " while mean time diff of last " << kuNbValuesForTimeDiffMean
                                     << " pairs is " << std::setw( 8 ) << fvdMeanTimeDiffAsicPair[ uAsic ][ uAsicB ]
                                     << FairLogger::endl;
                           LOG(INFO) << "CbmCosy2018MonitorPulser::DoUnpack => "
                                     << " Buffer slot idx is  " << std::setw( 3 ) << fvuLastTimeDiffSlotAsicPair[ uAsic ][ uAsicB ]
                                     << " Buffer size is " << std::setw( 3 ) << fvdLastTimeDiffValuesAsicPair[ uAsic ][ uAsicB ].size()
                                     << FairLogger::endl;

                        } // if dDt outside of MeanTimeDiffPair +/- margin

                        UpdatePairMeanValue( uAsic, uAsicB, dDt );
                     } // if( kdPulserPeriod * -0.5 < dDt && dDt < kdPulserPeriod * 0.5 )
                  } // for( it  = fvmAsicHitsInMs[ uAsicB ].begin(); it != fvmAsicHitsInMs[ uAsicB ].end(); ++it )
               } // for( UInt_t uAsic = 0; uAsic < fuNbStsXyters; uAsic++)

               Double_t dDtPulse = ( static_cast< Double_t >( (*it).GetTs() ) - static_cast< Double_t >( fvmLastHitAsic[ uAsic ].GetTs() )
                              ) * stsxyter::kdClockCycleNs;
               fhPulserIntervalAsic[ uAsic ]->Fill( dDtPulse );
               fhPulserIntervalLongAsic[ uAsic ]->Fill( dDtPulse );
               fvmLastHitAsic[ uAsic ] = (*it);
            } // for( it  = fvmAsicHitsInMs[ uAsic ].begin(); it != fvmAsicHitsInMs[ uAsic ].end(); ++it )

            /// Data in vector are not needed anymore as all possible matches are already checked
            fvmAsicHitsInMs[ uAsic ].clear();
         } // for( UInt_t uAsic = 0; uAsic < fuNbStsXyters; uAsic++)
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
      SavePulserHistos( "data/PulserPeriodicHistosSave.root");

  return kTRUE;
}

void CbmMcbm2018MonitorStsSync::FillHitInfo( stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx )
{
   UShort_t usChan   = mess.GetHitChannel();
   UShort_t usRawAdc = mess.GetHitAdc();
//   UShort_t usFullTs = mess.GetHitTimeFull();
   UShort_t usTsOver = mess.GetHitTimeOver();
   UShort_t usRawTs  = mess.GetHitTime();

   fhPulserChanCntRaw[  uAsicIdx ]->Fill( usChan );
   fhPulserChanAdcRaw[  uAsicIdx ]->Fill( usChan, usRawAdc );
   fhPulserChanAdcRawProf[  uAsicIdx ]->Fill( usChan, usRawAdc );
   fhPulserChanRawTs[   uAsicIdx ]->Fill( usChan, usRawTs );
   fhPulserChanMissEvt[ uAsicIdx ]->Fill( usChan, mess.IsHitMissedEvts() );

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
                << " MsbCy " << std::setw( 4 ) << fvuCurrentTsMsbCycle[fuCurrDpbIdx]
                << " Time " << std::setw ( 12 ) << fvulChanLastHitTime[ uAsicIdx ][ usChan ]
                << FairLogger::endl;
*/
   // Check Starting point of histos with time as X axis
   if( -1 == fdStartTime )
      fdStartTime = fvdChanLastHitTime[ uAsicIdx ][ usChan ];

   // Fill histos with time as X axis
   Double_t dTimeSinceStartSec = (fvdChanLastHitTime[ uAsicIdx ][ usChan ] - fdStartTime)* 1e-9;
   Double_t dTimeSinceStartMin = dTimeSinceStartSec / 60.0;
   fhPulserChanHitRateEvo[ uAsicIdx ]->Fill( dTimeSinceStartSec , usChan );
   fhPulserFebRateEvo[ uAsicIdx ]->Fill(   dTimeSinceStartSec );
   fhPulserChanHitRateEvoLong[ uAsicIdx ]->Fill( dTimeSinceStartMin, usChan, 1.0/60.0 );
   fhPulserFebRateEvoLong[ uAsicIdx ]->Fill(   dTimeSinceStartMin, 1.0/60.0 );
   if( mess.IsHitMissedEvts() )
   {
      fhPulserChanMissEvtEvo[ uAsicIdx ]->Fill( dTimeSinceStartSec , usChan );
      fhPulserFebMissEvtEvo[ uAsicIdx ]->Fill( dTimeSinceStartSec );
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

void CbmMcbm2018MonitorStsSync::FillTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx, UInt_t uMsIdx )
{
   UInt_t uVal    = mess.GetTsMsbVal();
/*
   if( 0 == fuCurrDpbIdx )
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

void CbmMcbm2018MonitorStsSync::FillEpochInfo( stsxyter::Message mess )
{
   UInt_t uVal    = mess.GetTsMsbVal();
/*
   // Update Status counters
   if( usVal < fvulCurrentTsMsb[fuCurrDpbIdx] )
      fvuCurrentTsMsbCycle[fuCurrDpbIdx] ++;
   fvulCurrentTsMsb[fuCurrDpbIdx] = usVal;

//   fhStsAsicTsMsb->Fill( fvulCurrentTsMsb[fuCurrDpbIdx], uAsicIdx );
*/
}

void CbmMcbm2018MonitorStsSync::Reset()
{
}

void CbmMcbm2018MonitorStsSync::Finish()
{

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MonitorStsSync statistics are " << FairLogger::endl;
   LOG(INFO) << " Hit      messages: " << fmMsgCounter[ stsxyter::MessType::Hit   ] << FairLogger::endl
             << " Ts MSB   messages: " << fmMsgCounter[ stsxyter::MessType::TsMsb ] << FairLogger::endl
             << " Dummy    messages: " << fmMsgCounter[ stsxyter::MessType::Dummy ] << FairLogger::endl
             << " Epoch    messages: " << fmMsgCounter[ stsxyter::MessType::Epoch ] << FairLogger::endl
             << " Empty    messages: " << fmMsgCounter[ stsxyter::MessType::Empty ] << FairLogger::endl;

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;

   SaveAllHistos( fsHistoFileFullname );
   SaveAllHistos();

}


void CbmMcbm2018MonitorStsSync::FillOutput(CbmDigi* /*digi*/)
{
}

void CbmMcbm2018MonitorStsSync::SaveAllHistos( TString sFileName )
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

   fhPulserMessType->Write();
   fhPulserSysMessType->Write();
   fhPulserMessTypePerDpb->Write();
   fhPulserSysMessTypePerDpb->Write();
   fhPulserMessTypePerElink->Write();
   fhPulserSysMessTypePerElink->Write();

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fhPulserChanCntRaw[ uXyterIdx ]->Write();
      fhPulserChanAdcRaw[ uXyterIdx ]->Write();
      fhPulserChanAdcRawProf[ uXyterIdx ]->Write();
      fhPulserChanRawTs[ uXyterIdx ]->Write();
      fhPulserChanMissEvt[ uXyterIdx ]->Write();
      fhPulserChanMissEvtEvo[ uXyterIdx ]->Write();
      fhPulserFebMissEvtEvo[ uXyterIdx ]->Write();
      fhPulserChanHitRateEvo[ uXyterIdx ]->Write();
      fhPulserFebRateEvo[ uXyterIdx ]->Write();
      fhPulserChanHitRateEvoLong[ uXyterIdx ]->Write();
      fhPulserFebRateEvoLong[ uXyterIdx ]->Write();
/*
      if( kTRUE == fbLongHistoEnable )
      {
         fhFebRateEvoLong[ uXyterIdx ]->Write();
         fhFebChRateEvoLong[ uXyterIdx ]->Write();
      } // if( kTRUE == fbLongHistoEnable )
*/
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

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
void CbmMcbm2018MonitorStsSync::SavePulserHistos( TString sFileName )
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
   gDirectory->mkdir("Sts_Pulser");
   gDirectory->cd("Sts_Pulser");

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
      {
         fhPulserTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Write();
         fhPulserRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Write();
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

   gDirectory->cd("..");
   /***************************/

   if( "" != sFileName )
   {
      // Restore original directory position
      histoFile->Close();
      oldDir->cd();
   } // if( "" != sFileName )

}
void CbmMcbm2018MonitorStsSync::ResetAllHistos()
{
   LOG(INFO) << "Reseting all STS histograms." << FairLogger::endl;

   fhPulserMessType->Reset();
   fhPulserSysMessType->Reset();
   fhPulserMessTypePerDpb->Reset();
   fhPulserSysMessTypePerDpb->Reset();
   fhPulserMessTypePerElink->Reset();
   fhPulserSysMessTypePerElink->Reset();

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fhPulserChanCntRaw[ uXyterIdx ]->Reset();
      fhPulserChanAdcRaw[ uXyterIdx ]->Reset();
      fhPulserChanAdcRawProf[ uXyterIdx ]->Reset();
      fhPulserChanRawTs[ uXyterIdx ]->Reset();
      fhPulserChanMissEvt[ uXyterIdx ]->Reset();
      fhPulserChanMissEvtEvo[ uXyterIdx ]->Reset();
      fhPulserFebMissEvtEvo[ uXyterIdx ]->Reset();
      fhPulserChanHitRateEvo[ uXyterIdx ]->Reset();
      fhPulserFebRateEvo[ uXyterIdx ]->Reset();
      fhPulserChanHitRateEvoLong[ uXyterIdx ]->Reset();
      fhPulserFebRateEvoLong[ uXyterIdx ]->Reset();
/*
      if( kTRUE == fbLongHistoEnable )
      {
         ftStartTimeUnix = std::chrono::steady_clock::now();
         fhFebRateEvoLong[ uXyterIdx ]->Reset();
         fhFebChRateEvoLong[ uXyterIdx ]->Reset();
      } // if( kTRUE == fbLongHistoEnable )
*/

      fhPulserTimeDiffPerAsic[ uXyterIdx ]->Reset();
      for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
      {
         fhPulserTimeDiffPerAsicPair[ uXyterIdx ][uXyterIdxB]->Reset();
         fhPulserTimeDiffEvoPerAsicPair[ uXyterIdx ][uXyterIdxB]->Reset();
         fhPulserTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Reset();
         fhPulserRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Reset();
         fhPulserTsLsbMatchPerAsicPair[ uXyterIdx ][uXyterIdxB]->Reset();
         fhPulserTsMsbMatchPerAsicPair[ uXyterIdx ][uXyterIdxB]->Reset();
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
      fhPulserIntervalAsic[ uXyterIdx ]->Reset();
      fhPulserIntervalLongAsic[ uXyterIdx ]->Reset();
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

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

void CbmMcbm2018MonitorStsSync::UpdatePairMeanValue( UInt_t uAsicA, UInt_t uAsicB, Double_t dNewValue )
{
   if( fvuLastTimeDiffSlotAsicPair.size() < uAsicA )
   {
      LOG(WARNING) << "CbmMcbm2018MonitorStsSync::UpdatePairMeanValue => wrong ASIC A value: " << uAsicA
                   << " VS " << fvuLastTimeDiffSlotAsicPair.size()
                   << FairLogger::endl;
      return;
   } // if( fvuLastTimeDiffSlotAsicPair.size() < uAsicA )
   if( fvuLastTimeDiffSlotAsicPair[ uAsicA ].size() < uAsicB )
   {
      LOG(WARNING) << "CbmMcbm2018MonitorStsSync::UpdatePairMeanValue => wrong ASIC B value: " << uAsicB
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

void CbmMcbm2018MonitorStsSync::SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize )
{
   TDatime * fRunStartDateTime     = new TDatime( dateIn, timeIn);
   fiRunStartDateTimeSec = fRunStartDateTime->Convert();
   fiBinSizeDatePlots    = iBinSize;

   LOG(INFO) << "Assigned new MUCH Run Start Date-Time: " << fRunStartDateTime->AsString() << FairLogger::endl;
}

void CbmMcbm2018MonitorStsSync::SetLongDurationLimits( UInt_t uDurationSeconds, UInt_t uBinSize )
{
   fbLongHistoEnable     = kTRUE;
   fuLongHistoNbSeconds  = uDurationSeconds;
   fuLongHistoBinSizeSec = uBinSize;
}

ClassImp(CbmMcbm2018MonitorStsSync)
