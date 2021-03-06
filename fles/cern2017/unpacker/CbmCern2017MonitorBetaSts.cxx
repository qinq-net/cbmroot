// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmCern2017MonitorBetaSts                         -----
// -----                Created 24/11/17  by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmCern2017MonitorBetaSts.h"

// Data

// CbmRoot
#include "CbmCern2017UnpackParSts.h"
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

// C++11

// C/C++
#include <iostream>
#include <stdint.h>
#include <iomanip>

Bool_t bCern2017ResetStsHistosBeta = kFALSE;
Bool_t bCern2017WriteStsHistosBeta = kFALSE;

CbmCern2017MonitorBetaSts::CbmCern2017MonitorBetaSts() :
   CbmTSUnpack(),
   fuOverlapMsNb(0),
   fUnpackPar(NULL),
   fuNrOfDpbs(0),
   fDpbIdIndexMap(),
   fuNbElinksPerDpb(0),
   fuNbStsXyters(0),
   fuNbChanPerAsic(0),
   fvuElinkToAsic(),
   fbPrintMessages( kFALSE ),
   fPrintMessCtrl( stsxyter::BetaMessagePrintMask::msg_print_Human ),
   fbChanHitDtEna( kFALSE ),
   fmMsgCounter(),
   fuCurrentEquipmentId(0),
   fuCurrDpbId(0),
   fuCurrDpbIdx(0),
   fiRunStartDateTimeSec(-1),
   fiBinSizeDatePlots(-1),
   fvuCurrentTsMsb(),
   fvuCurrentTsMsbCycle(),
   fvuCurrentTsMsbOver(),
   fvulChanLastHitTime(),
   fvdChanLastHitTime(),
   fvuChanNbHitsInMs(),
   fvdChanLastHitTimeInMs(),
   fvusChanLastHitAdcInMs(),
   fvmChanHitsInTs(),
   fdStartTime(-1.0),
   fdStartTimeMsSz(-1.0),
   ftStartTimeUnix( std::chrono::steady_clock::now() ),
   fbBetaFormat( kFALSE ),
   fvuElinkLastTsHit(),
   fvmHitsInTs(),
   fvulChanLastSortedHitTime(),
   fHM(new CbmHistManager()),
   fhStsMessType(NULL),
   fhStsSysMessType(NULL),
   fhStsMessTypePerDpb(NULL),
   fhStsSysMessTypePerDpb(NULL),
   fhStsMessTypePerElink(NULL),
   fhStsSysMessTypePerElink(NULL),
   fhStsChanCounts(),
   fhStsChanRawAdc(),
   fhStsChanRawAdcProf(),
   fhStsChanRawTs(),
   fhStsChanMissEvt(),
   fhStsChanOverDiff(),
   fhStsChanHitRateEvo(),
   fhStsXyterRateEvo(),
   fhStsChanHitRateEvoLong(),
   fhStsXyterRateEvoLong(),
   fhStsChanHitDt(),
   fhStsChanHitDtNeg(),
   fhStsChanHitsPerMs(),
   fhStsChanSameMs(),
   fpStsChanSameMsTimeDiff(),
   fhStsChanSameMsTimeDiff(),
   fbPulserTimeDiffOn(kFALSE),
   fuPulserMaxNbMicroslices(100),
   fvuPulserAsic(),
   fvuPulserChan(),
   fhStsPulserChansTimeDiff(),
   fhStsPulserChansTimeDiffEvo(),
   fhStsPulserChansSortedTimeDiff(),
   fhStsPulserChansSortedTimeDiffEvo(),
   fhStsAsicTsMsb(NULL),
   fhStsAsicTsMsbMaj(NULL),
   fhStsElinkTsMsbCrc(),
   fhStsElinkTsMsbMaj(),
   fbLongHistoEnable( kFALSE ),
   fuLongHistoNbSeconds( 0 ),
   fuLongHistoBinSizeSec( 0 ),
   fuLongHistoBinNb( 0 ),
   fhFebRateEvoLong(),
   fhFebChRateEvoLong(),
   fcMsSizeAll(NULL)
{
}

CbmCern2017MonitorBetaSts::~CbmCern2017MonitorBetaSts()
{
}

Bool_t CbmCern2017MonitorBetaSts::Init()
{
  LOG(INFO) << "Initializing flib StsXyter unpacker for STS" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  return kTRUE;
}

void CbmCern2017MonitorBetaSts::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
         << FairLogger::endl;
   fUnpackPar = (CbmCern2017UnpackParSts*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmCern2017UnpackParSts"));
}


Bool_t CbmCern2017MonitorBetaSts::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
         << FairLogger::endl;

   Bool_t bReInit = ReInitContainers();
   CreateHistograms();

   return bReInit;
}

Bool_t CbmCern2017MonitorBetaSts::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
         << FairLogger::endl;

   fuNrOfDpbs       = fUnpackPar->GetNrOfDpbs();
   fuNbElinksPerDpb = fUnpackPar->GetNbElinksPerDpb();
   fuNbStsXyters    = fUnpackPar->GetNbStsXyters();
   fuNbChanPerAsic  = fUnpackPar->GetNbChanPerAsic();


   LOG(INFO) << "Nr. of STS DPBs:       " << fuNrOfDpbs
             << FairLogger::endl;

   fDpbIdIndexMap.clear();
   fvuElinkToAsic.resize( fuNrOfDpbs );
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fDpbIdIndexMap[ fUnpackPar->GetDpbId( uDpb )  ] = uDpb;
      LOG(INFO) << "Eq. ID for DPB #" << std::setw(2) << uDpb << " = "
                << std::setw(4) << std::hex << fUnpackPar->GetDpbId( uDpb )
                << std::dec
                << " => " << fDpbIdIndexMap[ fUnpackPar->GetDpbId( uDpb )  ]
                << FairLogger::endl;

      fvuElinkToAsic[uDpb].resize( fuNbElinksPerDpb );
      for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
         fvuElinkToAsic[uDpb][uElink] = fUnpackPar->GetElinkToAsicIdx( uDpb * fuNbElinksPerDpb + uElink );
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
   fvuCurrentTsMsb.resize( fuNrOfDpbs );
   fvuCurrentTsMsbCycle.resize( fuNrOfDpbs );
   fvuCurrentTsMsbOver.resize( fuNrOfDpbs );
   fvuElinkLastTsHit.resize( fuNrOfDpbs );
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fvuCurrentTsMsb[uDpb].resize( fuNbElinksPerDpb );
      fvuCurrentTsMsbCycle[uDpb].resize( fuNbElinksPerDpb );
      fvuCurrentTsMsbOver[uDpb].resize( fuNbElinksPerDpb );
      fvuElinkLastTsHit[uDpb].resize( fuNbElinksPerDpb );
      for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
      {
         fvuCurrentTsMsb[uDpb][uElink]      = 0;
         fvuCurrentTsMsbCycle[uDpb][uElink] = 0;
         fvuCurrentTsMsbOver[uDpb][uElink]  = 0;
         fvuElinkLastTsHit[uDpb][uElink]  = 0;
      } // for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   fvulChanLastHitTime.resize( fuNbStsXyters );
   fvdChanLastHitTime.resize( fuNbStsXyters );
   fvuChanNbHitsInMs.resize( fuNbStsXyters );
   fvdChanLastHitTimeInMs.resize( fuNbStsXyters );
   fvusChanLastHitAdcInMs.resize( fuNbStsXyters );
   fvmChanHitsInTs.resize( fuNbStsXyters );
   fvulChanLastSortedHitTime.resize( fuNbStsXyters );
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fvulChanLastHitTime[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvdChanLastHitTime[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvuChanNbHitsInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvdChanLastHitTimeInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvusChanLastHitAdcInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvmChanHitsInTs[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvulChanLastSortedHitTime[ uXyterIdx ].resize( fuNbChanPerAsic );
      for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
      {
         fvulChanLastHitTime[ uXyterIdx ][ uChan ] = 0;
         fvdChanLastHitTime[ uXyterIdx ][ uChan ] = -1.0;
         fvulChanLastSortedHitTime[ uXyterIdx ][ uChan ] = 0;

         fvuChanNbHitsInMs[ uXyterIdx ][ uChan ].resize( fuPulserMaxNbMicroslices );
         fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ].resize( fuPulserMaxNbMicroslices );
         fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ].resize( fuPulserMaxNbMicroslices );
         fvmChanHitsInTs[ uXyterIdx ][ uChan ].clear();
         for( UInt_t uMsIdx = 0; uMsIdx < fuPulserMaxNbMicroslices; ++uMsIdx )
         {
            fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
            fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = -1.0;
            fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
         } // for( UInt_t uMsIdx = 0; uMsIdx < fuPulserMaxNbMicroslices; ++uMsIdx )
      } // for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   LOG(INFO) << "CbmCern2017MonitorBetaSts::ReInitContainers => Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                << FairLogger::endl;
   LOG(INFO) << "CbmCern2017MonitorBetaSts::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fuNbChanPerAsic
                << FairLogger::endl;
   LOG(INFO) << "CbmCern2017MonitorBetaSts::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuPulserMaxNbMicroslices
                << FairLogger::endl;
   return kTRUE;
}

void CbmCern2017MonitorBetaSts::CreateHistograms()
{
   THttpServer* server = FairRunOnline::Instance()->GetHttpServer();

   TString sHistName{""};
   TString title{""};

   sHistName = "hStsMessageType";
   title = "Nb of message for each type; Type";
   fhStsMessType = new TH1I(sHistName, title, 5, 0., 5.);
   fhStsMessType->GetXaxis()->SetBinLabel( 1,       "Dummy");
   fhStsMessType->GetXaxis()->SetBinLabel( 2,         "Hit");
   fhStsMessType->GetXaxis()->SetBinLabel( 3,       "TsMsb");
   fhStsMessType->GetXaxis()->SetBinLabel( 4, "ReadDataAck");
   fhStsMessType->GetXaxis()->SetBinLabel( 5,         "Ack");
/* *** Missing int + MessType OP!!!! ****
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::BetaMessType::Dummy,       "Dummy");
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::BetaMessType::Hit,         "Hit");
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::BetaMessType::TsMsb,       "TsMsb");
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::BetaMessType::ReadDataAck, "ReadDataAck");
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::BetaMessType::Ack,         "Ack");
*/
   fHM->Add(sHistName.Data(), fhStsMessType);
   if( server ) server->Register("/StsRaw", fhStsMessType );

   sHistName = "hStsSysMessType";
   title = "Nb of system message for each type; System Type";
   fhStsSysMessType = new TH1I(sHistName, title, 17, 0., 17.);
/*
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   hSysMessType->GetXaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/
   fHM->Add(sHistName.Data(), fhStsSysMessType);
   if( server ) server->Register("/StsRaw", fhStsSysMessType );

   sHistName = "hStsMessageTypePerDpb";
   title = "Nb of message of each type for each DPB; DPB; Type";
   fhStsMessTypePerDpb = new TH2I(sHistName, title, fuNrOfDpbs, 0, fuNrOfDpbs, 5, 0., 5.);
   fhStsMessTypePerDpb->GetYaxis()->SetBinLabel( 1,       "Dummy");
   fhStsMessTypePerDpb->GetYaxis()->SetBinLabel( 2,         "Hit");
   fhStsMessTypePerDpb->GetYaxis()->SetBinLabel( 3,       "TsMsb");
   fhStsMessTypePerDpb->GetYaxis()->SetBinLabel( 4, "ReadDataAck");
   fhStsMessTypePerDpb->GetYaxis()->SetBinLabel( 5,         "Ack");
/* *** Missing int + MessType OP!!!! ****
   fhStsMessType->GetYaxis()->SetBinLabel(1 + stsxyter::BetaMessType::Dummy,       "Dummy");
   fhStsMessType->GetYaxis()->SetBinLabel(1 + stsxyter::BetaMessType::Hit,         "Hit");
   fhStsMessType->GetYaxis()->SetBinLabel(1 + stsxyter::BetaMessType::TsMsb,       "TsMsb");
   fhStsMessType->GetYaxis()->SetBinLabel(1 + stsxyter::BetaMessType::ReadDataAck, "ReadDataAck");
   fhStsMessType->GetYaxis()->SetBinLabel(1 + stsxyter::BetaMessType::Ack,         "Ack");
*/
   fHM->Add(sHistName.Data(), fhStsMessTypePerDpb);
   if( server ) server->Register("/StsRaw", fhStsMessTypePerDpb );

   sHistName = "hStsSysMessTypePerDpb";
   title = "Nb of system message of each type for each DPB; DPB; System Type";
   fhStsSysMessTypePerDpb = new TH2I(sHistName, title, fuNrOfDpbs, 0, fuNrOfDpbs, 17, 0., 17.);
/*
   hSysMessType->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   hSysMessType->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   hSysMessType->GetYaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/
   fHM->Add(sHistName.Data(), fhStsSysMessTypePerDpb);
   if( server ) server->Register("/StsRaw", fhStsSysMessTypePerDpb );

   sHistName = "hStsMessageTypePerElink";
   title = "Nb of message of each type for each eLink; eLink; Type";
   fhStsMessTypePerElink = new TH2I(sHistName, title, fuNrOfDpbs * fuNbElinksPerDpb, 0, fuNrOfDpbs * fuNbElinksPerDpb, 5, 0., 5.);
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel( 1,       "Dummy");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel( 2,         "Hit");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel( 3,       "TsMsb");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel( 4, "ReadDataAck");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel( 5,         "Ack");
/* *** Missing int + MessType OP!!!! ****
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::BetaMessType::Dummy,       "Dummy");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::BetaMessType::Hit,         "Hit");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::BetaMessType::TsMsb,       "TsMsb");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::BetaMessType::ReadDataAck, "ReadDataAck");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::BetaMessType::Ack,         "Ack");
*/
   fHM->Add(sHistName.Data(), fhStsMessTypePerElink);
   if( server ) server->Register("/StsRaw", fhStsMessTypePerElink );

   sHistName = "hStsSysMessTypePerElink";
   title = "Nb of system message of each type for each eLink; eLink; System Type";
   fhStsSysMessTypePerElink = new TH2I(sHistName, title, fuNrOfDpbs * fuNbElinksPerDpb, 0, fuNrOfDpbs * fuNbElinksPerDpb, 17, 0., 17.);
/*
   fhStsSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   fhStsSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   fhStsSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/
   fHM->Add(sHistName.Data(), fhStsSysMessTypePerElink);
   if( server ) server->Register("/StsRaw", fhStsSysMessTypePerElink );

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

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      // Channel counts
      sHistName = Form( "hStsChanCounts_%03u", uXyterIdx );
      title = Form( "Hits Count per channel, StsXyter #%03u; Channel; Hits []", uXyterIdx );
      fhStsChanCounts.push_back( new TH1I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanCounts[ uXyterIdx ] );
      if( server ) server->Register("/StsRaw", fhStsChanCounts[ uXyterIdx ] );

      // Raw Adc Distribution
      sHistName = Form( "hStsChanRawAdc_%03u", uXyterIdx );
      title = Form( "Raw Adc distribution per channel, StsXyter #%03u; Channel []; Adc []; Hits []", uXyterIdx );
      fhStsChanRawAdc.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 stsxyter::kuBetaHitNbAdcBins, -0.5, stsxyter::kuBetaHitNbAdcBins -0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanRawAdc[ uXyterIdx ] );
      if( server ) server->Register("/StsRaw", fhStsChanRawAdc[ uXyterIdx ] );

      // Raw Adc Distribution profile
      sHistName = Form( "hStsChanRawAdcProfc_%03u", uXyterIdx );
      title = Form( "Raw Adc prodile per channel, StsXyter #%03u; Channel []; Adc []", uXyterIdx );
      fhStsChanRawAdcProf.push_back( new TProfile(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanRawAdcProf[ uXyterIdx ] );
      if( server ) server->Register("/StsRaw", fhStsChanRawAdcProf[ uXyterIdx ] );

      // Raw Ts Distribution
      sHistName = Form( "hStsChanRawTs_%03u", uXyterIdx );
      title = Form( "Raw Timestamp distribution per channel, StsXyter #%03u; Channel []; Ts []; Hits []", uXyterIdx );
      fhStsChanRawTs.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 stsxyter::kuBetaHitNbTsBins, -0.5, stsxyter::kuBetaHitNbTsBins -0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanRawTs[ uXyterIdx ] );
      if( server ) server->Register("/StsRaw", fhStsChanRawTs[ uXyterIdx ] );

      // Missed event flag
      sHistName = Form( "hStsChanMissEvt_%03u", uXyterIdx );
      title = Form( "Missed Event flags per channel, StsXyter #%03u; Channel []; Miss Evt []; Hits []", uXyterIdx );
      fhStsChanMissEvt.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 2, -0.5, 1.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanMissEvt[ uXyterIdx ] );
      if( server ) server->Register("/StsRaw", fhStsChanMissEvt[ uXyterIdx ] );

      // MSB correction from Overlap difference
      sHistName = Form( "hStsChanOverDiff_%03u", uXyterIdx );
      title = Form( "MSB correction from Overlap difference, per channel, StsXyter #%03u; Channel []; Over. diff. [MSB corr bin]; Hits []", uXyterIdx );
      fhStsChanOverDiff.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 stsxyter::kuBetaHitNbOverBins, -0.5, stsxyter::kuBetaHitNbOverBins - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanOverDiff[ uXyterIdx ] );
      if( server ) server->Register("/StsRaw", fhStsChanOverDiff[ uXyterIdx ] );

      // Hit rates evo per channel
      sHistName = Form( "hStsChanRateEvo_%03u", uXyterIdx );
      title = Form( "Hits per second & channel in StsXyter #%03u; Time [s]; Channel []; Hits []", uXyterIdx );
      fhStsChanHitRateEvo.push_back( new TH2I( sHistName, title,
                                                1800, 0, 1800,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanHitRateEvo[ uXyterIdx ] );
      if( server ) server->Register("/StsRaw", fhStsChanHitRateEvo[ uXyterIdx ] );

      // Hit rates evo per StsXyter
      sHistName = Form( "hStsXyterRateEvo_%03u", uXyterIdx );
      title = Form( "Hits per second in StsXyter #%03u; Time [s]; Hits []", uXyterIdx );
      fhStsXyterRateEvo.push_back( new TH1I(sHistName, title, 1800, 0, 1800 ) );
      fHM->Add(sHistName.Data(), fhStsXyterRateEvo[ uXyterIdx ] );
      if( server ) server->Register("/StsRaw", fhStsXyterRateEvo[ uXyterIdx ] );

      // Hit rates evo per channel, 1 minute bins, 24h
      sHistName = Form( "hStsChanRateEvoLong_%03u", uXyterIdx );
      title = Form( "Hits per second & channel in StsXyter #%03u; Time [min]; Channel []; Hits []", uXyterIdx );
      fhStsChanHitRateEvoLong.push_back( new TH2D( sHistName, title,
                                                1440, 0, 1440,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanHitRateEvoLong[ uXyterIdx ] );
      if( server ) server->Register("/StsRaw", fhStsChanHitRateEvoLong[ uXyterIdx ] );

      // Hit rates evo per StsXyter, 1 minute bins, 24h
      sHistName = Form( "hStsXyterRateEvoLong_%03u", uXyterIdx );
      title = Form( "Hits per second in StsXyter #%03u; Time [min]; Hits []", uXyterIdx );
      fhStsXyterRateEvoLong.push_back( new TH1D(sHistName, title, 1440, 0, 1440 ) );
      fHM->Add(sHistName.Data(), fhStsXyterRateEvoLong[ uXyterIdx ] );
      if( server ) server->Register("/StsRaw", fhStsXyterRateEvoLong[ uXyterIdx ] );

      // Hit distance in time for each channel
      if( fbChanHitDtEna )
      {
         sHistName = Form( "hStsChanHitDt_%03u", uXyterIdx );
         title = Form( "Time diff between hits on same channel in StsXyter #%03u; t_hit - t_prev [ns]; Channel []; Hits []", uXyterIdx );
         fhStsChanHitDt.push_back( new TH2I( sHistName, title,
                                                   iNbBinsRate - 1, dBinsRate,
                                                   fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
         fHM->Add(sHistName.Data(), fhStsChanHitDt[ uXyterIdx ] );
         if( server ) server->Register("/StsRaw", fhStsChanHitDt[ uXyterIdx ] );
         sHistName = Form( "hStsChanHitDtNeg_%03u", uXyterIdx );
         title = Form( "Time diff between hits on same channel in StsXyter #%03u; t_prev - t_hit [ns]; Channel []; Hits []", uXyterIdx );
         fhStsChanHitDtNeg.push_back( new TH2I( sHistName, title,
                                                   iNbBinsRate - 1, dBinsRate,
                                                   fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
         fHM->Add(sHistName.Data(), fhStsChanHitDtNeg[ uXyterIdx ] );
         if( server ) server->Register("/StsRaw", fhStsChanHitDtNeg[ uXyterIdx ] );

      } // if( fbChanHitDtEna )

      sHistName = Form( "hStsChanHitsPerMs_%03u", uXyterIdx );
      title = Form( "Nb of hits per channel in each MS in StsXyter #%03u; Nb Hits in MS []; Channel []; MS []", uXyterIdx );
      fhStsChanHitsPerMs.push_back( new TH2I( sHistName, title,
                                                100, -0.5, 99.5,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanHitsPerMs[ uXyterIdx ] );
      if( server ) server->Register("/StsRaw", fhStsChanHitsPerMs[ uXyterIdx ] );

      sHistName = Form( "hStsChanSameMs_%03u", uXyterIdx );
      title = Form( "Nb of MS with hits in both channels in StsXyter #%03u; Channel A []; Channel B []; Coinc. MS []", uXyterIdx );
      fhStsChanSameMs.push_back( new TH2I( sHistName, title,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanHitsPerMs[ uXyterIdx ] );
      if( server ) server->Register("/StsRaw", fhStsChanSameMs[ uXyterIdx ] );

      sHistName = Form( "pStsChanSameMsTimeDiff_%03u", uXyterIdx );
      title = Form( "Mean Time difference of channels when hits in same MS in StsXyter #%03u; Channel A []; Channel B []; Mean time diff [bins]", uXyterIdx );
      fpStsChanSameMsTimeDiff.push_back( new TProfile2D( sHistName, title,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );

      sHistName = Form( "hStsChanSameMsTimeDiff_%03u", uXyterIdx );
      title = Form( "Mean Time difference of channels when hits in same MS in StsXyter #%03u; Channel A []; Channel B []; Mean time diff [bins]", uXyterIdx );
      fhStsChanSameMsTimeDiff.push_back( new TH2D( sHistName, title,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanSameMsTimeDiff[ uXyterIdx ] );
      if( server ) server->Register("/StsRaw", fhStsChanSameMsTimeDiff[ uXyterIdx ] );

      if( kTRUE == fbLongHistoEnable )
      {
         UInt_t uAlignedLimit = fuLongHistoNbSeconds - (fuLongHistoNbSeconds % fuLongHistoBinSizeSec);
         fuLongHistoBinNb = uAlignedLimit / fuLongHistoBinSizeSec;

         sHistName = Form( "hFebRateEvoLong_%03u", uXyterIdx );
         title = Form( "Mean rate VS run time in same MS in StsXyter #%03u; Time in run [s]; Rate [1/s]", uXyterIdx );
         fhFebRateEvoLong.push_back( new TH1D( sHistName, title,
                                                   fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5) );
         fHM->Add(sHistName.Data(), fhFebRateEvoLong[ uXyterIdx ] );
         if( server ) server->Register("/StsRaw", fhFebRateEvoLong[ uXyterIdx ] );

         sHistName = Form( "hFebChRateEvoLong_%03u", uXyterIdx );
         title = Form( "Mean rate per channel VS run time in StsXyter #%03u; Time in run [s]; Channel []; Rare [1/s]", uXyterIdx );
         fhFebChRateEvoLong.push_back( new TH2D( sHistName, title,
                                                   fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5,
                                                   fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
         fHM->Add(sHistName.Data(), fhFebChRateEvoLong[ uXyterIdx ] );
         if( server ) server->Register("/StsRaw", fhFebChRateEvoLong[ uXyterIdx ] );
      } // if( kTRUE == fbLongHistoEnable )

   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

   if( kTRUE == fbPulserTimeDiffOn )
   {
      UInt_t uNbPulserChans = fvuPulserAsic.size();
      UInt_t uNbBinEvo = 32768 + 1;
      Double_t dMaxEdgeEvo = stsxyter::kdBetaClockCycleNs
                            * static_cast< Double_t >( uNbBinEvo ) / 2.0;
      Double_t dMinEdgeEvo = dMaxEdgeEvo * -1.0;
      fhStsPulserChansSortedTimeDiff.resize( uNbPulserChans - 1 );
      fhStsPulserChansSortedTimeDiffEvo.resize( uNbPulserChans - 1 );
      for( UInt_t uChA = 0; uChA < uNbPulserChans - 1; ++uChA )
      {
         fhStsPulserChansSortedTimeDiff[ uChA ].resize( uNbPulserChans );
         fhStsPulserChansSortedTimeDiffEvo[ uChA ].resize( uNbPulserChans );

         for( UInt_t uChB = uChA + 1; uChB < uNbPulserChans; ++uChB )
         {
            sHistName = Form( "hStsPulserChansTimeDiff_%02u_%03u_%02u_%03u",
                              fvuPulserAsic[uChA], fvuPulserChan[uChA],
                              fvuPulserAsic[uChB], fvuPulserChan[uChB] );
            title = Form( "Time diff for hits in same MS from ASIC %02d ch %03d and ASIC %02d ch %03d; tB - tA [ns]; Counts",
                              fvuPulserAsic[uChA], fvuPulserChan[uChA],
                              fvuPulserAsic[uChB], fvuPulserChan[uChB] );
//            TH1 * pHist = new TH1I(sHistName, title, 16385, -51203.125, 51203.125);
            TH1 * pHist = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );

            fHM->Add(sHistName.Data(), pHist);
            if( server ) server->Register("/StsRaw", pHist );
            fhStsPulserChansTimeDiff.push_back( pHist );

            sHistName = Form( "hStsPulserChansSortedTimeDiff_%02u_%03u_%02u_%03u",
                              fvuPulserAsic[uChA], fvuPulserChan[uChA],
                              fvuPulserAsic[uChB], fvuPulserChan[uChB] );
            title = Form( "Time diff for hits in same MS from ASIC %02d ch %03d and ASIC %02d ch %03d; tB - tA [ns]; Counts",
                              fvuPulserAsic[uChA], fvuPulserChan[uChA],
                              fvuPulserAsic[uChB], fvuPulserChan[uChB] );
//            TH1 * pHist = new TH1I(sHistName, title, 16385, -51203.125, 51203.125);
            pHist = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );

            fHM->Add(sHistName.Data(), pHist);
            if( server ) server->Register("/StsRaw", pHist );
            fhStsPulserChansSortedTimeDiff[ uChA ][ uChB ] = pHist;

            if( kTRUE == fbLongHistoEnable )
            {
               UInt_t uAlignedLimit = fuLongHistoNbSeconds - (fuLongHistoNbSeconds % fuLongHistoBinSizeSec);
               sHistName = Form( "hStsPulserChansTimeDiffEvo_%02u_%03u_%02u_%03u",
                              fvuPulserAsic[uChA], fvuPulserChan[uChA],
                              fvuPulserAsic[uChB], fvuPulserChan[uChB] );
               title = Form( "Time diff for hits in same MS from ASIC %02d ch %03d and ASIC %02d ch %03d, vs time in run; Time in run [min]; tB - tA [ns]; Counts",
                                 fvuPulserAsic[uChA], fvuPulserChan[uChA],
                                 fvuPulserAsic[uChB], fvuPulserChan[uChB] );
               TH2 * pHistEvo = new TH2I( sHistName, title,
                                       fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5,
                                       uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );

               fHM->Add(sHistName.Data(), pHistEvo);
               if( server ) server->Register("/StsRaw", pHistEvo );
               fhStsPulserChansTimeDiffEvo.push_back( pHistEvo );

               sHistName = Form( "hStsPulserChansSortedTimeDiffEvo_%02u_%03u_%02u_%03u",
                              fvuPulserAsic[uChA], fvuPulserChan[uChA],
                              fvuPulserAsic[uChB], fvuPulserChan[uChB] );
               title = Form( "Time diff for hits in same MS from ASIC %02d ch %03d and ASIC %02d ch %03d, vs time in run; Time in run [min]; tB - tA [ns]; Counts",
                                 fvuPulserAsic[uChA], fvuPulserChan[uChA],
                                 fvuPulserAsic[uChB], fvuPulserChan[uChB] );
               pHistEvo = new TH2I( sHistName, title,
                                       fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5,
                                       uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );

               fHM->Add(sHistName.Data(), pHistEvo);
               if( server ) server->Register("/StsRaw", pHistEvo );
               fhStsPulserChansSortedTimeDiffEvo[ uChA ][ uChB ] = pHistEvo;
            } // if( kTRUE == fbLongHistoEnable )
         } // for( UInt_t uChB = uChA + 1; uChB < uNbPulserChans; ++uChB )
      } // for( UInt_t uChA = 0; uChA < uNbPulserChans - 1; ++uChA )
   } // if( kTRUE == fbPulserTimeDiffOn )

   // Distribution of the TS_MSB per StsXyter
   sHistName = "hStsAsicTsMsb";
   title = "Raw Timestamp Msb distribution per StsXyter; Ts MSB []; StsXyter []; Hits []";
   fhStsAsicTsMsb = new TH2I( sHistName, title, stsxyter::kuBetaTsMsbNbTsBins, -0.5,   stsxyter::kuBetaTsMsbNbTsBins - 0.5,
                                                fuNbStsXyters, -0.5, fuNbStsXyters - 0.5 );
   fHM->Add(sHistName.Data(), fhStsAsicTsMsb );
   if( server ) server->Register("/StsRaw", fhStsAsicTsMsb );

   // Nb values in agreement in TS MSB messages, per StsXyter
   sHistName = "hStsAsicTsMsbMaj";
   title = "Nb values in agreement in TS MSB messages, per StsXyter; StsXyter []; Majority? []; Mess. []";
   fhStsAsicTsMsbMaj = new TH2I( sHistName, title, fuNbStsXyters, -0.5, fuNbStsXyters - 0.5,
                                                   3, 0.5, 3.5);
   fHM->Add(sHistName.Data(), fhStsAsicTsMsbMaj );
   if( server ) server->Register("/StsRaw", fhStsAsicTsMsbMaj );

   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      // CRC good or bad in TS MSB messages, per (DPB, eLink) pairs
      sHistName = Form( "hStsElinkTsMsbCrc_d%02u", uDpb );
      title = Form( "CRC test in TS MSB, per eLink, DPB #%02u; eLink []; CRC Match? []; Mess. []", uDpb);
      fhStsElinkTsMsbCrc.push_back( new TH2I( sHistName, title, fuNbElinksPerDpb, -0.5, fuNbElinksPerDpb - 0.5,
                                                                2, -0.5, 1.5) );
      fHM->Add(sHistName.Data(), fhStsElinkTsMsbCrc[ uDpb ] );
      if( server ) server->Register("/StsRaw", fhStsElinkTsMsbCrc[ uDpb ] );

      // Nb values in agreement in TS MSB messages, per (DPB, eLink) pairs
      sHistName = Form( "hStsElinkTsMsbMaj_d%02u", uDpb );
      title = Form( "Nb values in agreement in TS MSB, per eLink, DPB #%02u; eLink []; Majority? []; Mess. []", uDpb);
      fhStsElinkTsMsbMaj.push_back( new TH2I( sHistName, title, fuNbElinksPerDpb, -0.5, fuNbElinksPerDpb - 0.5,
                                                                3, 0.5, 3.5) );
      fHM->Add(sHistName.Data(), fhStsElinkTsMsbMaj[ uDpb ] );
      if( server ) server->Register("/StsRaw", fhStsElinkTsMsbMaj[ uDpb ] );
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   // Miscroslice properties histos
   for( Int_t component = 0; component < kiMaxNbFlibLinks; component ++ )
   {
      fhMsSz[ component ] = NULL;
      fhMsSzTime[ component ] = NULL;
   } // for( Int_t component = 0; component < kiMaxNbFlibLinks; component ++ )

   // Online histo browser commands
   if( server )
   {
      server->RegisterCommand("/Reset_All_Sts", "bCern2017ResetStsHistosBeta=kTRUE");
      server->RegisterCommand("/Write_All_Sts", "bCern2017WriteStsHistosBeta=kTRUE");

      server->Restrict("/Reset_All_Sts", "allow=admin");
      server->Restrict("/Write_All_Sts", "allow=admin");
   } // if( server )

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
      fhStsChanCounts[ uXyterIdx ]->Draw();

      cStsSumm->cd(2);
      gPad->SetLogz();
      fhStsChanRawAdc[ uXyterIdx ]->Draw( "colz" );

      cStsSumm->cd(3);
      gPad->SetLogz();
      fhStsChanRawTs[ uXyterIdx ]->Draw( "colz" );

      cStsSumm->cd(4);
      gPad->SetLogy();
      fhStsXyterRateEvo[ uXyterIdx ]->Draw();
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

      // Pulser testing
   if( kTRUE == fbPulserTimeDiffOn )
   {
      UInt_t uNbPulserPlots = fhStsPulserChansTimeDiff.size();
      TCanvas* cStsPulser = new TCanvas( "cStsPulser" , "Summary plots for StsXyter pulser testing",
                                         w, h);
      cStsPulser->Divide( 2, uNbPulserPlots/2 + uNbPulserPlots%2 );

      for( UInt_t uPulserPlot = 0; uPulserPlot < uNbPulserPlots; ++uPulserPlot)
      {
         cStsPulser->cd( 1 + uPulserPlot );
         gPad->SetLogy();
         gPad->SetGridx();
         gPad->SetGridy();
         fhStsPulserChansTimeDiff[ uPulserPlot ]->Draw();
         gStyle->SetOptStat("emruo");
         fhStsPulserChansTimeDiff[ uPulserPlot ]->SetStats(1);
      } // for( UInt_t uPulserPlot = 0; uPulserPlot < uNbPulserPlots; ++uPulserPlot)

      if( kTRUE == fbLongHistoEnable )
      {
         TCanvas* cStsPulserEvo = new TCanvas( "cStsPulserEvo" , "Summary plots for StsXyter pulser testing vs time",
                                            w, h);
         cStsPulserEvo->Divide( 2, uNbPulserPlots/2 + uNbPulserPlots%2 );

         for( UInt_t uPulserPlot = 0; uPulserPlot < uNbPulserPlots; ++uPulserPlot)
         {
            cStsPulserEvo->cd( 1 + uPulserPlot );
            gPad->SetGridx();
            gPad->SetGridy();
            fhStsPulserChansTimeDiffEvo[ uPulserPlot ]->Draw( "colz" );
            gStyle->SetOptStat("emruo");
            fhStsPulserChansTimeDiffEvo[ uPulserPlot ]->SetStats(1);
         } // for( UInt_t uPulserPlot = 0; uPulserPlot < uNbPulserPlots; ++uPulserPlot)
      } // if( kTRUE == fbLongHistoEnable )
   } // if( kTRUE == fbPulserTimeDiffOn )

      // Long duration rate monitoring
   if( kTRUE == fbLongHistoEnable )
   {
      TCanvas* cStsLongRate = new TCanvas( "cStsLongRate" , "Long duration rate plots for StsXyter",
                                           w, h);
      cStsLongRate->Divide( 2, fuNbStsXyters );

      for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      {
         cStsLongRate->cd( 1 + 2 * uXyterIdx );
         gPad->SetLogy();
         gPad->SetGridx();
         gPad->SetGridy();
         fhFebRateEvoLong[ uXyterIdx ]->Draw( "hist" );

         cStsLongRate->cd( 2 + 2 * uXyterIdx );
         gPad->SetLogz();
         gPad->SetGridx();
         gPad->SetGridy();
         fhFebChRateEvoLong[ uXyterIdx ]->Draw( "colz" );
      } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   } // if( kTRUE == fbLongHistoEnable )

/*
  Int_t iNbPadsPerDpb = fuNbElinksPerDpb/2 + fuNbElinksPerDpb%2;
  TCanvas* cMuchChCounts = new TCanvas("cMuchChCounts", "MUCH Channels counts", w, h);
  cMuchChCounts->Divide( fNrOfNdpbs/2 + fNrOfNdpbs%2, fNrOfFebsPerNdpb );

  TCanvas* cMuchFebRate = new TCanvas("cMuchFebRate", "MUCH FEB rate", w, h);
  cMuchFebRate->Divide( fNrOfNdpbs/2 + fNrOfNdpbs%2, fNrOfFebsPerNdpb );

   TH1* histPnt = NULL;
   for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)
   {// looping on all the nDPBS IDs
      if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
      {
         sNdpbTag = Form("%04X", fUnpackPar->GetNdpbIdA(dpbId) );
      } // if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
         else
         {
            sNdpbTag = Form("%04X", fUnpackPar->GetNdpbIdB(dpbId - fNrOfNdpbsA) );
         } // else of if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )

      for( Int_t febId = 0; febId < fNrOfFebsPerNdpb; febId++)
      {// looping on all the FEB IDs
         cMuchChCounts->cd( 1 + dpbId * iNbPadsPerDpb + febId/2 );
         gPad->SetLogy();
         sHistName = Form("Chan_Counts_Much_n%s_f%1u", sNdpbTag.Data(), febId);
         histPnt = fHM->H1(sHistName.Data());

         if( 0 == febId%2 )
         {
            histPnt->SetLineColor( kRed );  // => Change color for 1st of the 2/pad!
            histPnt->Draw();
         } // if( 0 == febId%2 )
            else
            {
               histPnt->SetLineColor( kBlue );  // => Change color for 1nd of the 2/pad!
               histPnt->Draw("same");
            } // if( 0 == febId%2 )

         cMuchFebRate->cd( 1 + dpbId * iNbPadsPerDpb + febId/2 );
         gPad->SetLogy();
         sHistName = Form("FebRate_n%s_f%1u", sNdpbTag.Data(), febId);
         histPnt = fHM->H1(sHistName.Data());

         if( 0 == febId%2 )
         {
            histPnt->SetLineColor( kRed );  // => Change color for 1st of the 2/pad!
            histPnt->Draw();
         } // if( 0 == febId%2 )
            else
            {
               histPnt->SetLineColor( kBlue );  // => Change color for 1nd of the 2/pad!
               histPnt->Draw("same");
            } // if( 0 == febId%2 )
      } // for( Int_t febId = 0; febId < fNrOfFebsPerNdpb; febId++)
   } // for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)
*/

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

  /*****************************/
}

Bool_t CbmCern2017MonitorBetaSts::DoUnpack(const fles::Timeslice& ts, size_t component)
{
  THttpServer* server = FairRunOnline::Instance()->GetHttpServer();

   if( bCern2017ResetStsHistosBeta )
   {
      ResetAllHistos();
      bCern2017ResetStsHistosBeta = kFALSE;
   } // if( bCern2017ResetStsHistosBeta )
   if( bCern2017WriteStsHistosBeta )
   {
      SaveAllHistos( "data/StsHistos.root" );
      bCern2017WriteStsHistosBeta = kFALSE;
   } // if( bCern2017WriteStsHistosBeta )

   LOG(DEBUG) << "Timeslice contains " << ts.num_microslices(component)
              << "microslices." << FairLogger::endl;

   if( component < kiMaxNbFlibLinks )
      if( NULL == fhMsSz[ component ] )
      {
         TString sMsSzName = Form("MsSz_link_%02lu", component);
         TString sMsSzTitle = Form("Size of MS for nDPB of link %02lu; Ms Size [bytes]", component);
         fhMsSz[ component ] = new TH1F( sMsSzName.Data(), sMsSzTitle.Data(), 160000, 0., 20000. );
         fHM->Add(sMsSzName.Data(), fhMsSz[ component ] );
         if (server) server->Register("/FlibRaw", fhMsSz[ component ] );
         sMsSzName = Form("MsSzTime_link_%02lu", component);
         sMsSzTitle = Form("Size of MS vs time for gDPB of link %02lu; Time[s] ; Ms Size [bytes]", component);
         fhMsSzTime[ component ] =  new TProfile( sMsSzName.Data(), sMsSzTitle.Data(), 15000, 0., 300. );
         fHM->Add( sMsSzName.Data(), fhMsSzTime[ component ] );
         if (server) server->Register("/FlibRaw", fhMsSzTime[ component ] );
         if( NULL != fcMsSizeAll )
         {
            fcMsSizeAll->cd( 1 + component );
            gPad->SetLogy();
            fhMsSzTime[ component ]->Draw("hist le0");
         } // if( NULL != fcMsSizeAll )
         LOG(INFO) << "Added MS size histo for component: " << component
                   << " (DPB)" << FairLogger::endl;
      } // if( NULL == fhMsSz[ component ] )

//   Int_t messageType = -111;
   // Loop over microslices
   size_t numCompMsInTs = ts.num_microslices(component);

   if( fuPulserMaxNbMicroslices < numCompMsInTs )
   {
      fuPulserMaxNbMicroslices = numCompMsInTs;

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
            fvuChanNbHitsInMs[ uXyterIdx ][ uChan ].resize( fuPulserMaxNbMicroslices );
            fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ].resize( fuPulserMaxNbMicroslices );
            fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ].resize( fuPulserMaxNbMicroslices );
            for( UInt_t uMsIdx = 0; uMsIdx < fuPulserMaxNbMicroslices; ++uMsIdx )
            {
               fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
               fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = -1.0;
               fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
            } // for( UInt_t uMsIdx = 0; uMsIdx < fuPulserMaxNbMicroslices; ++uMsIdx )
         } // for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
      } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      LOG(INFO) << "CbmCern2017MonitorBetaSts::DoUnpack => Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                   << FairLogger::endl;
      LOG(INFO) << "CbmCern2017MonitorBetaSts::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fuNbChanPerAsic
                   << FairLogger::endl;
      LOG(INFO) << "CbmCern2017MonitorBetaSts::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuPulserMaxNbMicroslices
                   << FairLogger::endl;
   } // if( fuPulserMaxNbMicroslices < numCompMsInTs )

   for( size_t m = 0; m < numCompMsInTs; ++m )
   {
      // Ignore overlap ms if number defined by user
      if( numCompMsInTs - fuOverlapMsNb <= m )
         continue;

      auto msDescriptor = ts.descriptor(component, m);
      fuCurrentEquipmentId = msDescriptor.eq_id;
      const uint8_t* msContent = reinterpret_cast<const uint8_t*>(ts.content(component, m));

      uint32_t size = msDescriptor.size;
      LOG(DEBUG) << "Microslice: " << msDescriptor.idx
                 << " from EqId " << std::hex << fuCurrentEquipmentId << std::dec
                 << " has size: " << size << FairLogger::endl;

      fuCurrDpbId  = static_cast< uint32_t >( fuCurrentEquipmentId & 0xFFFF );
      fuCurrDpbIdx = fDpbIdIndexMap[ fuCurrDpbId ];

      if( component < kiMaxNbFlibLinks )
      {
         if( fdStartTimeMsSz < 0 )
            fdStartTimeMsSz = (1e-9) * static_cast<double>(msDescriptor.idx);
         fhMsSz[ component ]->Fill( size );
         fhMsSzTime[ component ]->Fill( (1e-9) * static_cast<double>( msDescriptor.idx) - fdStartTimeMsSz, size);
      } // if( component < kiMaxNbFlibLinks )

      if( 0 == component && 0 == ( ( msDescriptor.idx / 1000000000 ) % 10 )
                         && 0 == ( ( msDescriptor.idx / 100000 ) ) %50000 )
      {
         for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
         {
            fhStsChanSameMsTimeDiff[uXyterIdx]->Reset();
            TH2* tempProj = fpStsChanSameMsTimeDiff[uXyterIdx]->ProjectionXY( );
            fhStsChanSameMsTimeDiff[uXyterIdx]->Add( tempProj );
            delete tempProj;
/*
            for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
               for( UInt_t uChanB = uChan + 1; uChanB < fuNbChanPerAsic; ++uChanB )
                  if( 0 < fpStsChanSameMsTimeDiff[uXyterIdx]->GetBinEntries( fpStsChanSameMsTimeDiff[uXyterIdx]->GetBin( uChan, uChanB ) ) )
                  {
                     LOG(DEBUG) << "Microslice Index =  " << (msDescriptor.idx / 1000000000)
                                << "s, ASIC " << uXyterIdx << " Chan " << uChan << " VS " << uChanB
                                << " Entries " << fpStsChanSameMsTimeDiff[uXyterIdx]->GetBinEntries( fpStsChanSameMsTimeDiff[uXyterIdx]->GetBin( uChan, uChanB ) )
                                << " Mean time diff "<< fpStsChanSameMsTimeDiff[uXyterIdx]->GetBinContent( uChan, uChanB )
                                << " Error " << fpStsChanSameMsTimeDiff[uXyterIdx]->GetBinError( uChan, uChanB )
                                << FairLogger::endl;
                  } // if( 0 < fpStsChanSameMsTimeDiff[uXyterIdx]->GetBinEntries( uChan, uChanB ) )
*/
         }  // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      } // if( 0 == ( ( msDescriptor.idx / 1e-9 ) % 10 )  )

      // If not integer number of message in input buffer, print warning/error
      if( 0 != (size % kuBytesPerMessage) )
         LOG(ERROR) << "The input microslice buffer does NOT "
                    << "contain only complete nDPB messages!"
                    << FairLogger::endl;

      // Compute the number of complete messages in the input microslice buffer
      uint32_t uNbMessages = (size - (size % kuBytesPerMessage) )
                              / kuBytesPerMessage;

      // Prepare variables for the loop on contents
      const uint32_t* pInBuff = reinterpret_cast<const uint32_t*>( msContent );
      for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )
      {
         // Fill message
         uint32_t ulData = static_cast<uint32_t>(pInBuff[uIdx]);

         stsxyter::BetaMessage mess( static_cast< uint32_t >( ulData & 0xFFFFFFFF ) );

         // Print message if requested
         if( fbPrintMessages )
            mess.PrintMess( std::cout, fPrintMessCtrl );

         // Extract the eLink and Asic indices
         UShort_t usElinkIdx = mess.GetLinkIndex();
         if( fuNbElinksPerDpb <= usElinkIdx )
         {
            LOG(FATAL) << "CbmCern2017MonitorBetaSts::DoUnpack => "
                       << "eLink index out of bounds!"
                       << FairLogger::endl;
         } // if( fuNbElinksPerDpb <= usElinkIdx )
         UInt_t   uAsicIdx   = fvuElinkToAsic[fuCurrDpbIdx][usElinkIdx];

         stsxyter::BetaMessType typeMess = mess.GetMessType();
         fmMsgCounter[ typeMess ] ++;
         fhStsMessType->Fill( static_cast< uint16_t > (typeMess) );
         fhStsMessTypePerDpb->Fill( fuCurrDpbIdx, static_cast< uint16_t > (typeMess) );
         fhStsMessTypePerElink->Fill( fuCurrDpbIdx * fuNbElinksPerDpb + usElinkIdx,
                                      static_cast< uint16_t > (typeMess) );

         switch( typeMess )
         {
            case stsxyter::BetaMessType::Hit :
            {
               FillHitInfo( mess, usElinkIdx, uAsicIdx, m );
               break;
            } // case stsxyter::BetaMessType::Hit :
            case stsxyter::BetaMessType::TsMsb :
            {
               FillTsMsbInfo( mess, usElinkIdx, uAsicIdx );
               break;
            } // case stsxyter::BetaMessType::TsMsb :
            case stsxyter::BetaMessType::Dummy :
            {
               if( kTRUE == fbBetaFormat )
               {
                  // In beta data format, a dummy hit indicates a TS overflow => TS_MSB increase by 1
                  fvuCurrentTsMsb[fuCurrDpbIdx][0] ++;
                  fvuElinkLastTsHit[fuCurrDpbIdx][0] = 0;
               } // if( kTRUE == fbBetaFormat )
               break;
            } // case stsxyter::BetaMessType::Dummy / ReadDataAck / Ack :
            default:
            {
               LOG(FATAL) << "CbmCern2017MonitorBetaSts::DoUnpack => "
                          << "Unknown message type, should never happen, stopping here!"
                          << FairLogger::endl;
            }
         } // switch( mess.GetMessType() )
      } // for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )
   } // for( size_t m = 0; m < numCompMsInTs; ++m )


   // End of TS, check if stuff to do with the hits inside each MS
   // Usefull for low rate pulser tests
      // Need to do it only when last DPB is processed, as they are done one by one
   if( fuCurrDpbIdx == fuNrOfDpbs - 1 )
   {
      for( UInt_t uMsIdx = 0; uMsIdx < fuPulserMaxNbMicroslices; ++uMsIdx )
      {
/*
         if( kTRUE == fbPulserTimeDiffOn )
         {
            UInt_t uNbPulserChans = fvuPulserAsic.size();
            UInt_t uHistoIdx = 0;
            for( UInt_t uChA = 0; uChA < uNbPulserChans; ++uChA )
               for( UInt_t uChB = uChA + 1; uChB < uNbPulserChans; ++uChB )
               {
                  if( 0 < fvuChanNbHitsInMs[ fvuPulserAsic[uChA] ][ fvuPulserChan[uChA] ][ uMsIdx ] &&
                      0 < fvuChanNbHitsInMs[ fvuPulserAsic[uChB] ][ fvuPulserChan[uChB] ][ uMsIdx ] )
                  {
                     Double_t dTimeDiff = fvdChanLastHitTimeInMs[ fvuPulserAsic[uChB] ][ fvuPulserChan[uChB] ][ uMsIdx ]
                                        - fvdChanLastHitTimeInMs[ fvuPulserAsic[uChA] ][ fvuPulserChan[uChA] ][ uMsIdx ];
                     Short_t sAdcDiff = static_cast< Short_t >( fvusChanLastHitAdcInMs[ fvuPulserAsic[uChB] ][ fvuPulserChan[uChB] ][ uMsIdx ] )
                                      - static_cast< Short_t >( fvusChanLastHitAdcInMs[ fvuPulserAsic[uChA] ][ fvuPulserChan[uChA] ][ uMsIdx ] );

                     if( dTimeDiff < -102406.25 || 102406.25 < dTimeDiff )
                        LOG(INFO) << "CbmCern2017MonitorBetaSts::DoUnpack =>"
                                  << " ASIC A" << fvuPulserAsic[uChA] << " chan A " << fvuPulserChan[uChA]
                                  << " tA " << fvdChanLastHitTimeInMs[ fvuPulserAsic[uChA] ][ fvuPulserChan[uChA] ][ uMsIdx ]
                                  << " ASIC B" << fvuPulserAsic[uChB] << " chan B " << fvuPulserChan[uChB]
                                  << " tB " << fvdChanLastHitTimeInMs[ fvuPulserAsic[uChB] ][ fvuPulserChan[uChB] ][ uMsIdx ]
                                  << " dt " << dTimeDiff
                                  << FairLogger::endl;

                     if( dTimeDiff < -102406.25 )
                        dTimeDiff += 102406.25;
                     else if( 102406.25 < dTimeDiff )
                        dTimeDiff -= 102406.25;

                     fhStsPulserChansTimeDiff[ uHistoIdx ]->Fill( dTimeDiff );
                  } // Both pulser channels got data in last MS
                  uHistoIdx ++;
               } // Loop on channel pairs
         } // if( kTRUE == fbPulserTimeDiffOn )
*/
         for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
         {
            for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
            {
               fhStsChanHitsPerMs[uXyterIdx]->Fill( fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ], uChan );

               // Coincidences between
               if( fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ] )
                  for( UInt_t uChanB = uChan + 1; uChanB < fuNbChanPerAsic; ++uChanB )
                     if( fvuChanNbHitsInMs[ uXyterIdx ][ uChanB ][ uMsIdx ] )
                     {
                        fhStsChanSameMs[ uXyterIdx ]->Fill( uChan, uChanB );
                        Double_t dTimeDiff = fvdChanLastHitTimeInMs[ uXyterIdx ][ uChanB ][ uMsIdx ]
                                           - fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ][ uMsIdx ];

                        fpStsChanSameMsTimeDiff[ uXyterIdx ]->Fill( uChan, uChanB, dTimeDiff );
                     } // if( fvuChanNbHitsInMs[ uXyterIdx ][ uChanB ] )

/*
               // Pulser test: Coincidences between different FEEs (maybe on different DPBs)
               for( UInt_t uXyterIdxB = uXyterIdx; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
               {
                  for( UInt_t uChanB = 0; uChanB < fuNbChanPerAsic; ++uChanB )
                  {
                  } // for( UInt_t uChanB = 0; uChanB < fuNbChanPerAsic; ++uChanB )
               } // for( UInt_t uXyterIdxB = uXyterIdx; uXyterIdxB < fuNbStsXyters; ++uXyterIdxB )
*/
               // Ok to reset as we only compare to channels with higher indices (or channels in ASICs with higher indices)
               fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
               fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = -1.0;
               fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
            } // for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
         } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      } // for( UInt_t uMsIdx = 0; uMsIdx < fuPulserMaxNbMicroslices; ++uMsIdx )

      if( kTRUE == fbPulserTimeDiffOn )
      {
         UInt_t uNbPulserChans = fvuPulserAsic.size();
         UInt_t uHistoIdx = 0;
         std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();
         Double_t dUnixTimeInRun = std::chrono::duration_cast< std::chrono::seconds >(tNow - ftStartTimeUnix).count();
         for( UInt_t uChA = 0; uChA < uNbPulserChans; ++uChA )
            for( UInt_t uChB = uChA + 1; uChB < uNbPulserChans; ++uChB )
            {
               if( 0 < fvmChanHitsInTs[ fvuPulserAsic[uChA] ][ fvuPulserChan[uChA] ].size() &&
                   0 < fvmChanHitsInTs[ fvuPulserAsic[uChB] ][ fvuPulserChan[uChB] ].size() )
               {
                  std::multiset< stsxyter::BetaHit >::iterator itClosestHitB = fvmChanHitsInTs[ fvuPulserAsic[uChB] ][ fvuPulserChan[uChB] ].begin();
                  for( std::multiset< stsxyter::BetaHit >::iterator itHitA = fvmChanHitsInTs[ fvuPulserAsic[uChA] ][ fvuPulserChan[uChA] ].begin();
                       itHitA != fvmChanHitsInTs[ fvuPulserAsic[uChA] ][ fvuPulserChan[uChA] ].end();
                       ++itHitA )
                  {
                     Double_t dClosestTimeDiff = 1e12;

                     for( std::multiset< stsxyter::BetaHit >::iterator itHitB = itClosestHitB ;
                          itHitB != fvmChanHitsInTs[ fvuPulserAsic[uChB] ][ fvuPulserChan[uChB] ].end();
                          ++itHitB )
                     {
                        Double_t dTimeDiff = static_cast< Double_t >( (*itHitB).GetTs() )
                                           - static_cast< Double_t >( (*itHitA).GetTs() );

                        if( TMath::Abs( dTimeDiff ) <  TMath::Abs( dClosestTimeDiff ) )
                        {
                           dClosestTimeDiff = dTimeDiff;
                           itClosestHitB = itHitB;
                        } // if( TMath::Abs( dTimeDiff ) <  TMath::Abs( dClosestTimeDiff ) )
                     } // Loop on hits in channel B, starting from match to previous channel A hit

                     Double_t dTimeDiff = dClosestTimeDiff * stsxyter::kdBetaClockCycleNs;
                     Short_t sAdcDiff = static_cast< Short_t >( (*itClosestHitB).GetAdc() )
                                      - static_cast< Short_t >( (*itHitA).GetAdc() );

                     fhStsPulserChansTimeDiff[ uHistoIdx ]->Fill( dTimeDiff );
                     if( fbLongHistoEnable )
                     {
                        Double_t dTimeSinceStartSec = ( (*itHitA).GetTs() * stsxyter::kdBetaClockCycleNs - fdStartTime )* 1e-9;
                        fhStsPulserChansTimeDiffEvo[ uHistoIdx ]->Fill( dUnixTimeInRun, dTimeDiff );
                     } // if( fbLongHistoEnable )
                  } // Loop on hits in channel A
               } // Both pulser channels got data in last TS
               uHistoIdx ++;
            } // Loop on channel pairs

         // Clean up the buffers now that we used the data
         for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
            for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
               fvmChanHitsInTs[ uXyterIdx ][ uChan ].clear();
      } // if( kTRUE == fbPulserTimeDiffOn )

      // Time differences plotting using the fully time sorted hits
      if( 0 < fvmHitsInTs.size() )
      {
         ULong64_t ulLastHitTime = ( *( fvmHitsInTs.rbegin() ) ).GetTs();
         std::multiset< stsxyter::BetaHit >::iterator it;
         UInt_t uNbPulserChans = fvuPulserAsic.size();
         std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();
         Double_t dUnixTimeInRun = std::chrono::duration_cast< std::chrono::seconds >(tNow - ftStartTimeUnix).count();
         for( it  = fvmHitsInTs.begin();
              it != fvmHitsInTs.end() && (*it).GetTs() < ulLastHitTime - 320; // 32 * 3.125 ns = 1000 ns
              ++it )
         {
            UShort_t usAsicIdx = (*it).GetAsic();
            UShort_t usChanIdx = (*it).GetChan();
            fvulChanLastSortedHitTime[ usAsicIdx ][ usChanIdx ] = (*it).GetTs();

            for( UInt_t uChA = 0; uChA < uNbPulserChans; ++uChA )
               if( fvuPulserAsic[uChA] == usAsicIdx && fvuPulserChan[uChA] == usChanIdx )
               {
                  for( UInt_t uChB = 0; uChB < uNbPulserChans; ++uChB )
                  {
                     if( uChB == uChA )
                        continue;

                     if( 0 == fvulChanLastSortedHitTime[ fvuPulserAsic[uChB] ][ fvuPulserChan[uChB] ] )
                        continue;

                     Double_t dTimeDiff;
                     if( uChA < uChB )
                        dTimeDiff = fvulChanLastSortedHitTime[ fvuPulserAsic[uChB] ][ fvuPulserChan[uChB] ]
                                  - fvulChanLastSortedHitTime[ usAsicIdx ][ usChanIdx ];
                        else dTimeDiff = fvulChanLastSortedHitTime[ usAsicIdx ][ usChanIdx ]
                                       - fvulChanLastSortedHitTime[ fvuPulserAsic[uChB] ][ fvuPulserChan[uChB] ];
                     dTimeDiff *= stsxyter::kdBetaClockCycleNs;

                     if( uChA < uChB )
                     {
                        fhStsPulserChansSortedTimeDiff[ uChA ][ uChB ]->Fill( dTimeDiff );
                        if( fbLongHistoEnable )
                           fhStsPulserChansSortedTimeDiffEvo[ uChA ][ uChB ]->Fill( dUnixTimeInRun, dTimeDiff );
                     } // if( uChA < uChB )
                        else
                        {
                           fhStsPulserChansSortedTimeDiff[ uChB ][ uChA ]->Fill( dTimeDiff );
                           if( fbLongHistoEnable )
                              fhStsPulserChansSortedTimeDiffEvo[ uChB ][ uChA ]->Fill( dUnixTimeInRun, dTimeDiff );
                        } // else of if( uChA < uChB )
                  } // for( UInt_t uChB = 0; uChB < uNbPulserChans; ++uChB )

                  break; // leaves the loop
               } // if( fvuPulserAsic[uChA] == usAsicIdx && fvuPulserChan[uChA] == usChanIdx )
         } // loop on hits untils hits within 100 ns of last one or last one itself are reached

         // Remove all hits which were already used
         fvmHitsInTs.erase( fvmHitsInTs.begin(), it );
      } // if( 0 < fvmHitsInTs.size() )
   } // if( fuCurrDpbIdx == fuNrOfDpbs - 1 )

   if( kTRUE == fbBetaFormat && 0 == ts.index() % 1000 && fuCurrDpbIdx == fuNrOfDpbs - 1)
   {
      for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
         LOG(INFO) << "eDPB "   << std::setw(2) << uDpb
                   << " eLink " << std::setw(2) << 0
                   << " current TS cycle counter is " << std::setw(12) << fvuCurrentTsMsb[uDpb][0]
                      << FairLogger::endl
                   << " current sorted buffer size is " << fvmHitsInTs.size()
                   << " First hit TS is " << ( *( fvmHitsInTs.begin() ) ).GetTs()
                   << " Last hit TS is " << ( *( fvmHitsInTs.rbegin() ) ).GetTs()
                      << FairLogger::endl;
   } // if( kTRUE == fbBetaFormat && 0 == ts.descriptor.index % 10000 )

  return kTRUE;
}

void CbmCern2017MonitorBetaSts::FillHitInfo( stsxyter::BetaMessage mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx )
{
   UShort_t usChan   = mess.GetHitChannel();
   UShort_t usRawAdc = mess.GetHitAdc();
//   UShort_t usFullTs = mess.GetHitTimeFull();
   UShort_t usTsOver = mess.GetHitTimeOver();
   UShort_t usRawTs  = mess.GetHitTime();

   if( 0xFFFF == uAsicIdx )
      return;

   fhStsChanCounts[  uAsicIdx ]->Fill( usChan );
   fhStsChanRawAdc[  uAsicIdx ]->Fill( usChan, usRawAdc );
   fhStsChanRawAdcProf[  uAsicIdx ]->Fill( usChan, usRawAdc );
   fhStsChanRawTs[   uAsicIdx ]->Fill( usChan, usRawTs );
   fhStsChanMissEvt[ uAsicIdx ]->Fill( usChan, mess.IsHitMissedEvts() );

   // Compute the Full time stamp
/*
   if( fvuCurrentTsMsbOver[fuCurrDpbIdx][usElinkIdx] == usTsOver )
   {
      // Hit stamped in same TsMsb as sent to eLink
      // Need TS cycle counter as otherwise period of only 102.400 us!!!
      fvulChanLastHitTime[ uAsicIdx ][ usChan ] = usRawTs
                                                + stsxyter::kuBetaHitNbTsBins   * static_cast<ULong64_t>( fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] )
                                                + stsxyter::kuTsCycleNbBins * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] );
   } // if( (fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] & stsxyter::kusBetaMaskTsMsbOver) == usTsOver )
      else if( (fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] & stsxyter::kusBetaMaskTsMsbOver)
               == ((usTsOver + 1) % (stsxyter::kusBetaMaskTsMsbOver + 1)) )
      {
         // Hit stamped in previous TsMsb compared to last Ts_MSB on same eLink
         // Need TS cycle counter as otherwise period of only 102.400 us!!!
         fvulChanLastHitTime[ uAsicIdx ][ usChan ] = usRawTs;
         if( 0 == fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] )
         {
            if( 0 == fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] )
            {
               // TO BE CHECKED
               LOG(WARNING) << "CbmCern2017MonitorBetaSts::FillHitInfo => "
                          << "Ignore Hit as coming from TS_MSB before system start!"
                          << FairLogger::endl;
               return;
            } // if( 0 == fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] )

            fvulChanLastHitTime[ uAsicIdx ][ usChan ] +=
                        stsxyter::kuBetaHitNbTsBins   * static_cast<ULong64_t>(  fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx]
                                                                           + stsxyter::kuTsCycleNbBins - 1 )
                      + stsxyter::kuTsCycleNbBins * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] - 1 );
         } // if( 0 == fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] )
            else fvulChanLastHitTime[ uAsicIdx ][ usChan ] +=
                        stsxyter::kuBetaHitNbTsBins   * static_cast<ULong64_t>( fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] - 1 )
                      + stsxyter::kuTsCycleNbBins * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] );

      } // else if overlap bit from hit are exactly 1 unit behind the one from last Ts_Msb
      else
      {
         // Overlap is neither matching nor 1 less than bits from TS_MSB
         // Should not happen if I understood properly how the ASIC behaves ?!?
         LOG(FATAL) << "CbmCern2017MonitorBetaSts::FillHitInfo => "
                    << "TS overlap bits from hit not fitting the ones from last TS MSB on this eLink! \n"
                    << "Hit Overlap: 0x" << std::hex << usTsOver << " Full TS: 0x" << usFullTs << "\n"
                    << "MSB Overlap: 0x" << (fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] & stsxyter::kusBetaMaskTsMsbOver)
                    << " Ts Msb: 0x" << fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] << "\n"
                    << "Other eLinks: \n";
         for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
            LOG(FATAL) << std::setw(3) << std::dec << uElink << " => "
                       << std::hex << "0x" << (fvuCurrentTsMsb[fuCurrDpbIdx][uElink] & stsxyter::kusBetaMaskTsMsbOver)
                       << " => " << "0x" << (fvuCurrentTsMsb[fuCurrDpbIdx][uElink] & stsxyter::kusBetaMaskTsMsbOver) << "\n";
         LOG(FATAL) << std::dec << FairLogger::endl;
         return;
      } // else of overlap checks
*/
      // Use TS w/o overlap bits as they will anyway come from the TS_MSB
   fvulChanLastHitTime[ uAsicIdx ][ usChan ] = usRawTs;

   if( kFALSE == fbBetaFormat )
   {
         // Overlap need to be used to correct the TS_MSB
         // Formula used here gives the following correction table
         /*
          *      |  TS MSB Over
          *      | 0 | 1 | 2 | 3
          * _____|___|___|___|___
          * H  0 | 0 | 1 | 2 | 3
          * I ___|___|___|___|___
          * T  1 | 3 | 0 | 1 | 2
          *  ____|___|___|___|___
          * O  2 | 2 | 3 | 0 | 1
          * V ___|___|___|___|___
          * E  3 | 1 | 2 | 3 | 0
          * R    |   |   |   |
          */
      UInt_t uTsMsbCorr = ( stsxyter::kuBetaHitNbOverBins + fvuCurrentTsMsbOver[fuCurrDpbIdx][usElinkIdx] - usTsOver )
                         % stsxyter::kuBetaHitNbOverBins;
      fhStsChanOverDiff[ uAsicIdx ]->Fill( usChan, uTsMsbCorr );

         // Need TS cycle counter as otherwise period of only 102.400 us!!!
         // => Check if we need also a TS MSB cycle correction
      if( fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] < uTsMsbCorr )
      {
         if( 0 == fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] )
         {
            // TO BE CHECKED
            LOG(WARNING) << "CbmCern2017MonitorBetaSts::FillHitInfo => "
                       << "Ignore Hit as coming from TS_MSB before system start!"
                       << FairLogger::endl;
            return;
         } // if( 0 == fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] )

         fvulChanLastHitTime[ uAsicIdx ][ usChan ] +=
                     stsxyter::kuBetaHitNbTsBins   * static_cast<ULong64_t>(  fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx]
                                                                        + stsxyter::kuBetaHitNbTsBins - uTsMsbCorr )
   //                + stsxyter::kuTsCycleNbBins * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] - 1 )
                   ;
      } // if( fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] < uTsMsbCorr )
         else fvulChanLastHitTime[ uAsicIdx ][ usChan ] +=
                     stsxyter::kuBetaHitNbTsBins   * static_cast<ULong64_t>( fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] - uTsMsbCorr )
   //                + stsxyter::kuTsCycleNbBins * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] )
                   ;
   } // if( kFALSE == fbBetaFormat )
      else
      {
//         if( usRawTs < fvuElinkLastTsHit[fuCurrDpbIdx][usElinkIdx] )
//            fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx]++;

         fvulChanLastHitTime[ uAsicIdx ][ usChan ] +=
                     static_cast<ULong64_t>( stsxyter::kuBetaHitNbTsBins )
                   * static_cast<ULong64_t>( fvuCurrentTsMsb[fuCurrDpbIdx][0]);

         fvuElinkLastTsHit[fuCurrDpbIdx][usElinkIdx] = usRawTs;
      } // else of if( kFALSE == fbBetaFormat )

   // Convert the Hit time in bins to Hit time in ns
   Double_t dHitTimeNs = fvulChanLastHitTime[ uAsicIdx ][ usChan ] * stsxyter::kdBetaClockCycleNs;


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

   // Store new value of Hit time in ns
   fvdChanLastHitTime[ uAsicIdx ][ usChan ] = fvulChanLastHitTime[ uAsicIdx ][ usChan ];

   // Pulser and MS
   fvuChanNbHitsInMs[      uAsicIdx ][ usChan ][ uMsIdx ] ++;
   fvdChanLastHitTimeInMs[ uAsicIdx ][ usChan ][ uMsIdx ] = dHitTimeNs;
   fvusChanLastHitAdcInMs[ uAsicIdx ][ usChan ][ uMsIdx ] = usRawAdc;
   fvmChanHitsInTs[        uAsicIdx ][ usChan ].insert( stsxyter::BetaHit( fvulChanLastHitTime[ uAsicIdx ][ usChan ], usRawAdc ) );
   fvmHitsInTs.insert( stsxyter::BetaHit( fvulChanLastHitTime[ uAsicIdx ][ usChan ], usRawAdc, uAsicIdx, usChan ) );

   // Check Starting point of histos with time as X axis
   if( -1 == fdStartTime )
      fdStartTime = fvdChanLastHitTime[ uAsicIdx ][ usChan ];

   // Fill histos with time as X axis
   Double_t dTimeSinceStartSec = (fvdChanLastHitTime[ uAsicIdx ][ usChan ] - fdStartTime)* 1e-9;
   Double_t dTimeSinceStartMin = dTimeSinceStartSec / 60.0;
   fhStsChanHitRateEvo[ uAsicIdx ]->Fill( dTimeSinceStartSec , usChan );
   fhStsXyterRateEvo[ uAsicIdx ]->Fill(   dTimeSinceStartSec );
   fhStsChanHitRateEvoLong[ uAsicIdx ]->Fill( dTimeSinceStartMin, usChan, 1.0/60.0 );
   fhStsXyterRateEvoLong[ uAsicIdx ]->Fill(   dTimeSinceStartMin, 1.0/60.0 );

   if( kTRUE == fbLongHistoEnable )
   {
      std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();
      Double_t dUnixTimeInRun = std::chrono::duration_cast< std::chrono::seconds >(tNow - ftStartTimeUnix).count();
      fhFebRateEvoLong[ uAsicIdx ]->Fill( dUnixTimeInRun , 1.0 / fuLongHistoBinSizeSec );
      fhFebChRateEvoLong[ uAsicIdx ]->Fill( dUnixTimeInRun , usChan, 1.0 / fuLongHistoBinSizeSec );
   } // if( kTRUE == fbLongHistoEnable )

}

void CbmCern2017MonitorBetaSts::FillTsMsbInfo( stsxyter::BetaMessage mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx )
{
   UShort_t usVal    = mess.GetTsMsbVal();

   // Update Status counters
   if( usVal < fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] )
      fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] ++;
   fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] = usVal;

   fhStsAsicTsMsb->Fill( fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx], uAsicIdx );
   fhStsAsicTsMsbMaj->Fill( uAsicIdx, 3 );
   fhStsElinkTsMsbMaj[fuCurrDpbIdx]->Fill( usElinkIdx, 3 );

   // Update the overlap bits for this eLink
   fvuCurrentTsMsbOver[fuCurrDpbIdx][usElinkIdx] =  fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx]
                                                  & stsxyter::kusBetaMaskTsMsbOver;
}

void CbmCern2017MonitorBetaSts::Reset()
{
}

void CbmCern2017MonitorBetaSts::Finish()
{

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   LOG(INFO) << "CbmCern2017MonitorBetaSts statistics are " << FairLogger::endl;
   LOG(INFO) << " Hit      messages: " << fmMsgCounter[ stsxyter::BetaMessType::Hit   ] << FairLogger::endl
             << " Ts MSB   messages: " << fmMsgCounter[ stsxyter::BetaMessType::TsMsb ] << FairLogger::endl
             << " Dummy    messages: " << fmMsgCounter[ stsxyter::BetaMessType::Dummy ] << FairLogger::endl;

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;

   SaveAllHistos();

}


void CbmCern2017MonitorBetaSts::FillOutput(CbmDigi* /*digi*/)
{
}

void CbmCern2017MonitorBetaSts::SaveAllHistos( TString sFileName )
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

   gDirectory->mkdir("Sts_Raw");
   gDirectory->cd("Sts_Raw");

   fhStsMessType->Write();
   fhStsSysMessType->Write();
   fhStsMessTypePerDpb->Write();
   fhStsSysMessTypePerDpb->Write();
   fhStsMessTypePerElink->Write();
   fhStsSysMessTypePerElink->Write();

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fhStsChanCounts[ uXyterIdx ]->Write();
      fhStsChanRawAdc[ uXyterIdx ]->Write();
      fhStsChanRawAdcProf[ uXyterIdx ]->Write();
      fhStsChanRawTs[ uXyterIdx ]->Write();
      fhStsChanMissEvt[ uXyterIdx ]->Write();
      fhStsChanOverDiff[ uXyterIdx ]->Write();
      fhStsChanHitRateEvo[ uXyterIdx ]->Write();
      fhStsXyterRateEvo[ uXyterIdx ]->Write();
      fhStsChanHitRateEvoLong[ uXyterIdx ]->Write();
      fhStsXyterRateEvoLong[ uXyterIdx ]->Write();
      if( fbChanHitDtEna )
      {
         fhStsChanHitDt[ uXyterIdx ]->Write();
         fhStsChanHitDtNeg[ uXyterIdx ]->Write();
      } // if( fbChanHitDtEna )
      fhStsChanHitsPerMs[ uXyterIdx ]->Write();
      fhStsChanSameMs[ uXyterIdx ]->Write();
      fpStsChanSameMsTimeDiff[ uXyterIdx ]->Write();
      fhStsChanSameMsTimeDiff[ uXyterIdx ]->Write();
      if( kTRUE == fbLongHistoEnable )
      {
         fhFebRateEvoLong[ uXyterIdx ]->Write();
         fhFebChRateEvoLong[ uXyterIdx ]->Write();
      } // if( kTRUE == fbLongHistoEnable )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

   if( kTRUE == fbPulserTimeDiffOn )
   {
      UInt_t uNbPulserChans = fvuPulserAsic.size();
      UInt_t uHistoIdx = 0;
      for( UInt_t uChA = 0; uChA < uNbPulserChans - 1; ++uChA )
         for( UInt_t uChB = uChA + 1; uChB < uNbPulserChans; ++uChB )
         {
            fhStsPulserChansTimeDiff[ uHistoIdx ]->Write();
            if( fbLongHistoEnable )
               fhStsPulserChansTimeDiffEvo[ uHistoIdx ]->Write();
            uHistoIdx ++;

            fhStsPulserChansSortedTimeDiff[ uChA ][ uChB ]->Write();
            fhStsPulserChansSortedTimeDiffEvo[ uChA ][ uChB ]->Write();
         } // Loop on channel pairs
   } // if( kTRUE == fbPulserTimeDiffOn )

   fhStsAsicTsMsb->Write();
   fhStsAsicTsMsbMaj->Write();

   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fhStsElinkTsMsbCrc[ uDpb ]->Write();
      fhStsElinkTsMsbMaj[ uDpb ]->Write();
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   gDirectory->cd("..");

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
   gDirectory->cd("..");

   if( "" != sFileName )
   {
      // Restore original directory position
      histoFile->Close();
      oldDir->cd();
   } // if( "" != sFileName )

}
void CbmCern2017MonitorBetaSts::ResetAllHistos()
{
   LOG(INFO) << "Reseting all STS histograms." << FairLogger::endl;

   fhStsMessType->Reset();
   fhStsSysMessType->Reset();
   fhStsMessTypePerDpb->Reset();
   fhStsSysMessTypePerDpb->Reset();
   fhStsMessTypePerElink->Reset();
   fhStsSysMessTypePerElink->Reset();

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fhStsChanCounts[ uXyterIdx ]->Reset();
      fhStsChanRawAdc[ uXyterIdx ]->Reset();
      fhStsChanRawAdcProf[ uXyterIdx ]->Reset();
      fhStsChanRawTs[ uXyterIdx ]->Reset();
      fhStsChanMissEvt[ uXyterIdx ]->Reset();
      fhStsChanOverDiff[ uXyterIdx ]->Reset();
      fhStsChanHitRateEvo[ uXyterIdx ]->Reset();
      fhStsXyterRateEvo[ uXyterIdx ]->Reset();
      fhStsChanHitRateEvoLong[ uXyterIdx ]->Reset();
      fhStsXyterRateEvoLong[ uXyterIdx ]->Reset();
      if( fbChanHitDtEna )
      {
         fhStsChanHitDt[ uXyterIdx ]->Reset();
         fhStsChanHitDtNeg[ uXyterIdx ]->Reset();
      } // if( fbChanHitDtEna )
      fhStsChanHitsPerMs[ uXyterIdx ]->Reset();
      fhStsChanSameMs[ uXyterIdx ]->Reset();
      fpStsChanSameMsTimeDiff[ uXyterIdx ]->Reset();
      fhStsChanSameMsTimeDiff[ uXyterIdx ]->Reset();
      if( kTRUE == fbLongHistoEnable )
      {
         ftStartTimeUnix = std::chrono::steady_clock::now();
         fhFebRateEvoLong[ uXyterIdx ]->Reset();
         fhFebChRateEvoLong[ uXyterIdx ]->Reset();
      } // if( kTRUE == fbLongHistoEnable )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

   if( kTRUE == fbPulserTimeDiffOn )
   {
      UInt_t uNbPulserChans = fvuPulserAsic.size();
      UInt_t uHistoIdx = 0;
      for( UInt_t uChA = 0; uChA < uNbPulserChans - 1; ++uChA )
         for( UInt_t uChB = uChA + 1; uChB < uNbPulserChans; ++uChB )
         {
            fhStsPulserChansTimeDiff[ uHistoIdx ]->Reset();
            if( fbLongHistoEnable )
               fhStsPulserChansTimeDiffEvo[ uHistoIdx ]->Reset();
            uHistoIdx ++;

            fhStsPulserChansSortedTimeDiff[ uChA ][ uChB ]->Write();
            fhStsPulserChansSortedTimeDiffEvo[ uChA ][ uChB ]->Write();
         } // Loop on channel pairs
   } // if( kTRUE == fbPulserTimeDiffOn )

   fhStsAsicTsMsb->Reset();
   fhStsAsicTsMsbMaj->Reset();

   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fhStsElinkTsMsbCrc[ uDpb ]->Reset();
      fhStsElinkTsMsbMaj[ uDpb ]->Reset();
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

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

void CbmCern2017MonitorBetaSts::SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize )
{
   TDatime * fRunStartDateTime     = new TDatime( dateIn, timeIn);
   fiRunStartDateTimeSec = fRunStartDateTime->Convert();
   fiBinSizeDatePlots    = iBinSize;

   LOG(INFO) << "Assigned new MUCH Run Start Date-Time: " << fRunStartDateTime->AsString() << FairLogger::endl;
}
void CbmCern2017MonitorBetaSts::SetPulserChannels( UInt_t uAsicA, UInt_t uChanA, UInt_t uAsicB, UInt_t uChanB,
                                               UInt_t uAsicC, UInt_t uChanC, UInt_t uAsicD, UInt_t uChanD,
                                               UInt_t uMaxNbMicroslices )
{
/*
   THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
   TString sHistName{""};
   TString title{""};
*/
   fuPulserMaxNbMicroslices = uMaxNbMicroslices;

   UInt_t uNbChans = 4;

   fvuPulserAsic.resize( uNbChans );
   fvuPulserChan.resize( uNbChans );
   fvuPulserAsic[0] = uAsicA;
   fvuPulserChan[0] = uChanA;
   fvuPulserAsic[1] = uAsicB;
   fvuPulserChan[1] = uChanB;
   fvuPulserAsic[2] = uAsicC;
   fvuPulserChan[2] = uChanC;
   fvuPulserAsic[3] = uAsicD;
   fvuPulserChan[3] = uChanD;
/*
   // Remove old histos
   if( kTRUE == fbPulserTimeDiffOn )
   {
      for( Int_t iIdx = 0; iIdx < fhStsPulserChansTimeDiff.size(); ++iIdx )
         delete fhStsPulserChansTimeDiff[ iIdx ];
   } // if( kTRUE == fbPulserTimeDiffOn )
   fhStsPulserChansTimeDiff.clear();

   for( UInt_t uChA = 0; uChA < uNbChans; ++uChA )
      for( UInt_t uChB = uChA + 1; uChB < uNbChans; ++uChB )
      {
         sHistName = Form( "hStsPulserTimeDiff_%02u_%03u_%02u_%03u",
                           fvuPulserAsic[uChA], fvuPulserChan[uChA],
                           fvuPulserAsic[uChB], fvuPulserChan[uChB] );
         title = "Time diff for hits in same MS from ASIC %02d ch %03d and ASIC %02d ch %03d; tB - tA [ns]; Counts";
         TH1 * pHist = new TH1I(sHistName, title, 16385, -51203.125, 51203.125);

         fHM->Add(sHistName.Data(), pHist);
         if( server ) server->Register("/StsRaw", pHist );
         fhStsPulserChansTimeDiff.push_back( pHist );
      } // Loop on pairs of channels
*/
   fbPulserTimeDiffOn = kTRUE;
}

void CbmCern2017MonitorBetaSts::SetLongDurationLimits( UInt_t uDurationSeconds, UInt_t uBinSize )
{
   fbLongHistoEnable     = kTRUE;
   fuLongHistoNbSeconds  = uDurationSeconds;
   fuLongHistoBinSizeSec = uBinSize;
}

ClassImp(CbmCern2017MonitorBetaSts)
