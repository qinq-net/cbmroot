// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmCern2017MonitorSts                         -----
// -----                Created 25/07/17  by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmCern2017MonitorSts.h"

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

// C++11

// C/C++
#include <iostream>
#include <stdint.h>
#include <iomanip>

Bool_t bCern2017ResetStsHistos = kFALSE;
Bool_t bCern2017WriteStsHistos = kFALSE;

CbmCern2017MonitorSts::CbmCern2017MonitorSts() :
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
   fPrintMessCtrl( stsxyter::MessagePrintMask::msg_print_Human ),
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
   fdStartTime(-1.0),
   fdStartTimeMsSz(-1.0),
   fHM(new CbmHistManager()),
   fhStsMessType(NULL),
   fhStsSysMessType(NULL),
   fhStsChanCounts(),
   fhStsChanRawAdc(),
   fhStsChanRawTs(),
   fhStsChanMissEvt(),
   fhStsChanOverDiff(),
   fhStsChanHitRateEvo(),
   fhStsXyterRateEvo(),
   fhStsChanHitRateEvoLong(),
   fhStsXyterRateEvoLong(),
   fhStsChanHitDt(),
   fhStsChanHitDtNeg(),
   fhStsAsicTsMsb(NULL),
   fhStsAsicTsMsbMaj(NULL),
   fhStsElinkTsMsbCrc(),
   fhStsElinkTsMsbMaj(),
   fcMsSizeAll(NULL)
{
}

CbmCern2017MonitorSts::~CbmCern2017MonitorSts()
{
}

Bool_t CbmCern2017MonitorSts::Init()
{
  LOG(INFO) << "Initializing flib StsXyter unpacker for STS" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  return kTRUE;
}

void CbmCern2017MonitorSts::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
         << FairLogger::endl;
   fUnpackPar = (CbmCern2017UnpackParSts*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmCern2017UnpackParSts"));
}


Bool_t CbmCern2017MonitorSts::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
         << FairLogger::endl;

   Bool_t bReInit = ReInitContainers();
   CreateHistograms();

   return bReInit;
}

Bool_t CbmCern2017MonitorSts::ReInitContainers()
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
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fvuCurrentTsMsb[uDpb].resize( fuNbElinksPerDpb );
      fvuCurrentTsMsbCycle[uDpb].resize( fuNbElinksPerDpb );
      fvuCurrentTsMsbOver[uDpb].resize( fuNbElinksPerDpb );
      for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
      {
         fvuCurrentTsMsb[uDpb][uElink]      = 0;
         fvuCurrentTsMsbCycle[uDpb][uElink] = 0;
         fvuCurrentTsMsbOver[uDpb][uElink]  = 0;
      } // for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   fvulChanLastHitTime.resize( fuNbStsXyters );
   fvdChanLastHitTime.resize( fuNbStsXyters );
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fvulChanLastHitTime[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvdChanLastHitTime[ uXyterIdx ].resize( fuNbChanPerAsic );
      for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
      {
         fvulChanLastHitTime[ uXyterIdx ][ uChan ] = 0;
         fvdChanLastHitTime[ uXyterIdx ][ uChan ] = -1.0;
      } // for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

   return kTRUE;
}

void CbmCern2017MonitorSts::CreateHistograms()
{
#ifdef USE_HTTP_SERVER
   THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
#endif

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
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::Dummy,       "Dummy");
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::Hit,         "Hit");
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::TsMsb,       "TsMsb");
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::ReadDataAck, "ReadDataAck");
   fhStsMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::Ack,         "Ack");
*/

/*
   hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_HIT,      "HIT");
   hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_EPOCH,    "EPOCH");
   hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_SYNC,     "SYNC");
   hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_AUX,      "AUX");
   hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_EPOCH2,   "EPOCH2");
   hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4,     "GET4");
   hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_SYS,      "SYS");
   hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4_SLC, "MSG_GET4_SLC");
   hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4_32B, "MSG_GET4_32B");
   hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4_SYS, "MSG_GET4_SYS");
   hMessageType->GetXaxis()->SetBinLabel(1 + 15, "GET4 Hack 32B");
   hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_NOP,      "NOP");
*/
   fHM->Add(sHistName.Data(), fhStsMessType);
#ifdef USE_HTTP_SERVER
   if( server ) server->Register("/StsRaw", fhStsMessType );
#endif

   sHistName = "hStsSysMessType";
   title = "Nb of system message for each type; System Type";
   fhStsSysMessType = new TH1I(sHistName, title, 17, 0., 17.);
/*
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_NX_PARITY,       "NX PARITY");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_SYNC_PARITY,     "SYNC PARITY");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_RESUME,      "DAQ RESUME");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_FIFO_RESET,      "FIFO RESET");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_USER,            "USER");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_PCTIME,          "PCTIME");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_ADC,             "ADC");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_PACKETLOST,      "PACKET LOST");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_GET4_EVENT,      "GET4 ERROR");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_CLOSYSYNC_ERROR, "CLOSYSYNC ERROR");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_TS156_SYNC,        "TS156 SYNC");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_GDPB_UNKWN,        "UNKW GET4 MSG");
   hSysMessType->GetXaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/
   fHM->Add(sHistName.Data(), fhStsSysMessType);
#ifdef USE_HTTP_SERVER
   if( server ) server->Register("/StsRaw", fhStsSysMessType );
#endif

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
#ifdef USE_HTTP_SERVER
      if( server ) server->Register("/StsRaw", fhStsChanCounts[ uXyterIdx ] );
#endif

      // Raw Adc Distribution
      sHistName = Form( "hStsChanRawAdc_%03u", uXyterIdx );
      title = Form( "Raw Adc distribution per channel, StsXyter #%03u; Channel []; Adc []; Hits []", uXyterIdx );
      fhStsChanRawAdc.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 stsxyter::kuHitNbAdcBins, -0.5, stsxyter::kuHitNbAdcBins -0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanRawAdc[ uXyterIdx ] );
#ifdef USE_HTTP_SERVER
      if( server ) server->Register("/StsRaw", fhStsChanRawAdc[ uXyterIdx ] );
#endif

      // Raw Ts Distribution
      sHistName = Form( "hStsChanRawTs_%03u", uXyterIdx );
      title = Form( "Raw Timestamp distribution per channel, StsXyter #%03u; Channel []; Ts []; Hits []", uXyterIdx );
      fhStsChanRawTs.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 stsxyter::kuHitNbTsBins, -0.5, stsxyter::kuHitNbTsBins -0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanRawTs[ uXyterIdx ] );
#ifdef USE_HTTP_SERVER
      if( server ) server->Register("/StsRaw", fhStsChanRawTs[ uXyterIdx ] );
#endif

      // Missed event flag
      sHistName = Form( "hStsChanMissEvt_%03u", uXyterIdx );
      title = Form( "Missed Event flags per channel, StsXyter #%03u; Channel []; Miss Evt []; Hits []", uXyterIdx );
      fhStsChanMissEvt.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 2, -0.5, 1.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanMissEvt[ uXyterIdx ] );
#ifdef USE_HTTP_SERVER
      if( server ) server->Register("/StsRaw", fhStsChanMissEvt[ uXyterIdx ] );
#endif

      // MSB correction from Overlap difference
      sHistName = Form( "hStsChanOverDiff_%03u", uXyterIdx );
      title = Form( "MSB correction from Overlap difference, per channel, StsXyter #%03u; Channel []; Over. diff. [MSB corr bin]; Hits []", uXyterIdx );
      fhStsChanOverDiff.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 stsxyter::kuHitNbOverBins, -0.5, stsxyter::kuHitNbOverBins - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanOverDiff[ uXyterIdx ] );
#ifdef USE_HTTP_SERVER
      if( server ) server->Register("/StsRaw", fhStsChanOverDiff[ uXyterIdx ] );
#endif

      // Hit rates evo per channel
      sHistName = Form( "hStsChanRateEvo_%03u", uXyterIdx );
      title = Form( "Hits per second & channel in StsXyter #%03u; Time [s]; Channel []; Hits []", uXyterIdx );
      fhStsChanHitRateEvo.push_back( new TH2I( sHistName, title,
                                                1800, 0, 1800,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanHitRateEvo[ uXyterIdx ] );
#ifdef USE_HTTP_SERVER
      if( server ) server->Register("/StsRaw", fhStsChanHitRateEvo[ uXyterIdx ] );
#endif

      // Hit rates evo per StsXyter
      sHistName = Form( "hStsXyterRateEvo_%03u", uXyterIdx );
      title = Form( "Hits per second in StsXyter #%03u; Time [s]; Hits []", uXyterIdx );
      fhStsXyterRateEvo.push_back( new TH1I(sHistName, title, 1800, 0, 1800 ) );
      fHM->Add(sHistName.Data(), fhStsXyterRateEvo[ uXyterIdx ] );
#ifdef USE_HTTP_SERVER
      if( server ) server->Register("/StsRaw", fhStsXyterRateEvo[ uXyterIdx ] );
#endif

      // Hit rates evo per channel, 1 minute bins, 24h
      sHistName = Form( "hStsChanRateEvoLong_%03u", uXyterIdx );
      title = Form( "Hits per second & channel in StsXyter #%03u; Time [min]; Channel []; Hits []", uXyterIdx );
      fhStsChanHitRateEvoLong.push_back( new TH2D( sHistName, title,
                                                1440, 0, 1440,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanHitRateEvoLong[ uXyterIdx ] );
#ifdef USE_HTTP_SERVER
      if( server ) server->Register("/StsRaw", fhStsChanHitRateEvoLong[ uXyterIdx ] );
#endif

      // Hit rates evo per StsXyter, 1 minute bins, 24h
      sHistName = Form( "hStsXyterRateEvoLong_%03u", uXyterIdx );
      title = Form( "Hits per second in StsXyter #%03u; Time [min]; Hits []", uXyterIdx );
      fhStsXyterRateEvoLong.push_back( new TH1D(sHistName, title, 1440, 0, 1440 ) );
      fHM->Add(sHistName.Data(), fhStsXyterRateEvoLong[ uXyterIdx ] );
#ifdef USE_HTTP_SERVER
      if( server ) server->Register("/StsRaw", fhStsXyterRateEvoLong[ uXyterIdx ] );
#endif

      // Hit distance in time for each channel
      if( fbChanHitDtEna )
      {
         sHistName = Form( "hStsChanHitDt_%03u", uXyterIdx );
         title = Form( "Time diff between hits on same channel in StsXyter #%03u; t_hit - t_prev [ns]; Channel []; Hits []", uXyterIdx );
         fhStsChanHitDt.push_back( new TH2I( sHistName, title,
                                                   iNbBinsRate - 1, dBinsRate,
                                                   fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
         fHM->Add(sHistName.Data(), fhStsChanHitDt[ uXyterIdx ] );
#ifdef USE_HTTP_SERVER
         if( server ) server->Register("/StsRaw", fhStsChanHitDt[ uXyterIdx ] );
#endif
         sHistName = Form( "hStsChanHitDtNeg_%03u", uXyterIdx );
         title = Form( "Time diff between hits on same channel in StsXyter #%03u; t_prev - t_hit [ns]; Channel []; Hits []", uXyterIdx );
         fhStsChanHitDtNeg.push_back( new TH2I( sHistName, title,
                                                   iNbBinsRate - 1, dBinsRate,
                                                   fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
         fHM->Add(sHistName.Data(), fhStsChanHitDtNeg[ uXyterIdx ] );
#ifdef USE_HTTP_SERVER
         if( server ) server->Register("/StsRaw", fhStsChanHitDtNeg[ uXyterIdx ] );
#endif

      } // if( fbChanHitDtEna )

   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

   // Distribution of the TS_MSB per StsXyter
   sHistName = "hStsAsicTsMsb";
   title = "Raw Timestamp Msb distribution per StsXyter; Ts MSB []; StsXyter []; Hits []";
   fhStsAsicTsMsb = new TH2I( sHistName, title, stsxyter::kuTsMsbNbTsBins, -0.5,   stsxyter::kuTsMsbNbTsBins - 0.5,
                                                fuNbStsXyters, -0.5, fuNbStsXyters - 0.5 );
   fHM->Add(sHistName.Data(), fhStsAsicTsMsb );
#ifdef USE_HTTP_SERVER
   if( server ) server->Register("/StsRaw", fhStsAsicTsMsb );
#endif

   // Nb values in agreement in TS MSB messages, per StsXyter
   sHistName = "hStsAsicTsMsbMaj";
   title = "Nb values in agreement in TS MSB messages, per StsXyter; StsXyter []; Majority? []; Mess. []";
   fhStsAsicTsMsbMaj = new TH2I( sHistName, title, fuNbStsXyters, -0.5, fuNbStsXyters - 0.5,
                                                   3, 0.5, 3.5);
   fHM->Add(sHistName.Data(), fhStsAsicTsMsbMaj );
#ifdef USE_HTTP_SERVER
   if( server ) server->Register("/StsRaw", fhStsAsicTsMsbMaj );
#endif

   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      // CRC good or bad in TS MSB messages, per (DPB, eLink) pairs
      sHistName = Form( "hStsElinkTsMsbCrc_d%02u", uDpb );
      title = Form( "CRC test in TS MSB, per eLink, DPB #%02u; eLink []; CRC Match? []; Mess. []", uDpb);
      fhStsElinkTsMsbCrc.push_back( new TH2I( sHistName, title, fuNbElinksPerDpb, -0.5, fuNbElinksPerDpb - 0.5,
                                                                2, -0.5, 1.5) );
      fHM->Add(sHistName.Data(), fhStsElinkTsMsbCrc[ uDpb ] );
#ifdef USE_HTTP_SERVER
      if( server ) server->Register("/StsRaw", fhStsElinkTsMsbCrc[ uDpb ] );
#endif

      // Nb values in agreement in TS MSB messages, per (DPB, eLink) pairs
      sHistName = Form( "hStsElinkTsMsbMaj_d%02u", uDpb );
      title = Form( "Nb values in agreement in TS MSB, per eLink, DPB #%02u; eLink []; Majority? []; Mess. []", uDpb);
      fhStsElinkTsMsbMaj.push_back( new TH2I( sHistName, title, fuNbElinksPerDpb, -0.5, fuNbElinksPerDpb - 0.5,
                                                                3, 0.5, 3.5) );
      fHM->Add(sHistName.Data(), fhStsElinkTsMsbMaj[ uDpb ] );
#ifdef USE_HTTP_SERVER
      if( server ) server->Register("/StsRaw", fhStsElinkTsMsbMaj[ uDpb ] );
#endif
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   // Miscroslice properties histos
   for( Int_t component = 0; component < kiMaxNbFlibLinks; component ++ )
   {
      fhMsSz[ component ] = NULL;
      fhMsSzTime[ component ] = NULL;
   } // for( Int_t component = 0; component < kiMaxNbFlibLinks; component ++ )

   // Online histo browser commands
#ifdef USE_HTTP_SERVER
   if( server )
   {
      server->RegisterCommand("/Reset_All_Sts", "bCern2017ResetStsHistos=kTRUE");
      server->RegisterCommand("/Write_All_Sts", "bCern2017WriteStsHistos=kTRUE");

      server->Restrict("/Reset_All_Sts", "allow=admin");
      server->Restrict("/Write_All_Sts", "allow=admin");
   } // if( server )
#endif

  /** Create summary Canvases for CERN 2017 **/
  Double_t w = 10;
  Double_t h = 10;
  Int_t iNbPadsPerDpb = fuNbElinksPerDpb/2 + fuNbElinksPerDpb%2;
/*
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

Bool_t CbmCern2017MonitorSts::DoUnpack(const fles::Timeslice& ts, size_t component)
{
#ifdef USE_HTTP_SERVER
  THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
#endif

   if( bCern2017ResetStsHistos )
   {
      ResetAllHistos();
      bCern2017ResetStsHistos = kFALSE;
   } // if( bCern2017ResetStsHistos )
   if( bCern2017WriteStsHistos )
   {
      SaveAllHistos( "data/StsHistos.root" );
      bCern2017WriteStsHistos = kFALSE;
   } // if( bCern2017WriteStsHistos )

   LOG(DEBUG) << "Timeslice contains " << ts.num_microslices(component)
              << "microslices." << FairLogger::endl;

   if( component < kiMaxNbFlibLinks )
      if( NULL == fhMsSz[ component ] )
      {
         TString sMsSzName = Form("MsSz_link_%02lu", component);
         TString sMsSzTitle = Form("Size of MS for nDPB of link %02lu; Ms Size [bytes]", component);
         fhMsSz[ component ] = new TH1F( sMsSzName.Data(), sMsSzTitle.Data(), 160000, 0., 20000. );
         fHM->Add(sMsSzName.Data(), fhMsSz[ component ] );
#ifdef USE_HTTP_SERVER
         if (server) server->Register("/FlibRaw", fhMsSz[ component ] );
#endif
         sMsSzName = Form("MsSzTime_link_%02lu", component);
         sMsSzTitle = Form("Size of MS vs time for gDPB of link %02lu; Time[s] ; Ms Size [bytes]", component);
         fhMsSzTime[ component ] =  new TProfile( sMsSzName.Data(), sMsSzTitle.Data(), 15000, 0., 300. );
         fHM->Add( sMsSzName.Data(), fhMsSzTime[ component ] );
#ifdef USE_HTTP_SERVER
         if (server) server->Register("/FlibRaw", fhMsSzTime[ component ] );
#endif
         if( NULL != fcMsSizeAll )
         {
            fcMsSizeAll->cd( 1 + component );
            gPad->SetLogy();
            fhMsSzTime[ component ]->Draw("hist le0");
         } // if( NULL != fcMsSizeAll )
         LOG(INFO) << "Added MS size histo for component: " << component
                   << " (DPB)" << FairLogger::endl;
      } // if( NULL == fhMsSz[ component ] )

   Int_t messageType = -111;
   // Loop over microslices
   size_t numCompMsInTs = ts.num_microslices(component);
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
                 << " has size: " << size << FairLogger::endl;

      if( component < kiMaxNbFlibLinks )
      {
         if( fdStartTimeMsSz < 0 )
            fdStartTimeMsSz = (1e-9) * static_cast<double>(msDescriptor.idx);
         fhMsSz[ component ]->Fill( size );
         fhMsSzTime[ component ]->Fill( (1e-9) * static_cast<double>( msDescriptor.idx) - fdStartTimeMsSz, size);
      } // if( component < kiMaxNbFlibLinks )

      // If not integer number of message in input buffer, print warning/error
      if( 0 != (size % kuBytesPerMessage) )
         LOG(ERROR) << "The input microslice buffer does NOT "
                    << "contain only complete nDPB messages!"
                    << FairLogger::endl;

      // Compute the number of complete messages in the input microslice buffer
      uint32_t uNbMessages = (size - (size % kuBytesPerMessage) )
                              / kuBytesPerMessage;

      // Prepare variables for the loop on contents
      const uint64_t* pInBuff = reinterpret_cast<const uint64_t*>( msContent );
      for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )
      {
         // Fill message
         uint64_t ulData = static_cast<uint64_t>(pInBuff[uIdx]);

         // temp solution until the DPB ID is moved to the equipment ID field in MS header
         // The ID is the same in all messages, so no need to do it for each
         if( 0 == uIdx )
         {
            fuCurrDpbId  = static_cast< uint32_t >( (ulData >> 32) & 0xFFFFFFFF );
            fuCurrDpbIdx = fDpbIdIndexMap[ fuCurrDpbId ];
         } // if( 0 == uIdx )

         stsxyter::Message mess( static_cast< uint32_t >( ulData & 0xFFFFFFFF ) );

         // Print message if requested
         if( fbPrintMessages )
            mess.PrintMess( std::cout, fPrintMessCtrl );

         // Extract the eLink and Asic indices
         UShort_t usElinkIdx = mess.GetLinkIndex();
         if( fuNbElinksPerDpb <= usElinkIdx )
         {
            LOG(FATAL) << "CbmCern2017MonitorSts::DoUnpack => "
                       << "eLink index out of bounds!"
                       << FairLogger::endl;
         } // if( fuNbElinksPerDpb <= usElinkIdx )
         UInt_t   uAsicIdx   = fvuElinkToAsic[fuCurrDpbIdx][usElinkIdx];

         stsxyter::MessType typeMess = mess.GetMessType();
         fmMsgCounter[ typeMess ] ++;
         fhStsMessType->Fill( static_cast< uint16_t > (typeMess) );

         switch( typeMess )
         {
            case stsxyter::MessType::Hit :
            {
               FillHitInfo( mess, usElinkIdx, uAsicIdx );
               break;
            } // case stsxyter::MessType::Hit :
            case stsxyter::MessType::TsMsb :
            {
               FillTsMsbInfo( mess, usElinkIdx, uAsicIdx );
               break;
            } // case stsxyter::MessType::TsMsb :
            case stsxyter::MessType::Dummy :
            case stsxyter::MessType::ReadDataAck :
            case stsxyter::MessType::Ack :
            {
               break;
            } // case stsxyter::MessType::Dummy / ReadDataAck / Ack :
            default:
            {
               LOG(FATAL) << "CbmCern2017MonitorSts::DoUnpack => "
                          << "Unknown message type, should never happen, stopping here!"
                          << FairLogger::endl;
            }
         } // switch( mess.GetMessType() )
      } // for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )
   } // for( size_t m = 0; m < numCompMsInTs; ++m )


  return kTRUE;
}

void CbmCern2017MonitorSts::FillHitInfo( stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx )
{
   UShort_t usChan   = mess.GetHitChannel();
   UShort_t usRawAdc = mess.GetHitAdc();
   UShort_t usFullTs = mess.GetHitTimeFull();
   UShort_t usTsOver = mess.GetHitTimeOver();
   UShort_t usRawTs  = mess.GetHitTime();

   fhStsChanCounts[  uAsicIdx ]->Fill( usChan );
   fhStsChanRawAdc[  uAsicIdx ]->Fill( usChan, usRawAdc );
   fhStsChanRawTs[   uAsicIdx ]->Fill( usChan, usRawTs );
   fhStsChanMissEvt[ uAsicIdx ]->Fill( usChan, mess.IsHitMissedEvts() );

   // Compute the Full time stamp
/*
   if( fvuCurrentTsMsbOver[fuCurrDpbIdx][usElinkIdx] == usTsOver )
   {
      // Hit stamped in same TsMsb as sent to eLink
      // Need TS cycle counter as otherwise period of only 102.400 us!!!
      fvulChanLastHitTime[ uAsicIdx ][ usChan ] = usRawTs
                                                + stsxyter::kuHitNbTsBins   * static_cast<ULong64_t>( fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] )
                                                + stsxyter::kuTsCycleNbBins * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] );
   } // if( (fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] & stsxyter::kusMaskTsMsbOver) == usTsOver )
      else if( (fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] & stsxyter::kusMaskTsMsbOver)
               == ((usTsOver + 1) % (stsxyter::kusMaskTsMsbOver + 1)) )
      {
         // Hit stamped in previous TsMsb compared to last Ts_MSB on same eLink
         // Need TS cycle counter as otherwise period of only 102.400 us!!!
         fvulChanLastHitTime[ uAsicIdx ][ usChan ] = usRawTs;
         if( 0 == fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] )
         {
            if( 0 == fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] )
            {
               // TO BE CHECKED
               LOG(WARNING) << "CbmCern2017MonitorSts::FillHitInfo => "
                          << "Ignore Hit as coming from TS_MSB before system start!"
                          << FairLogger::endl;
               return;
            } // if( 0 == fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] )

            fvulChanLastHitTime[ uAsicIdx ][ usChan ] +=
                        stsxyter::kuHitNbTsBins   * static_cast<ULong64_t>(  fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx]
                                                                           + stsxyter::kuTsCycleNbBins - 1 )
                      + stsxyter::kuTsCycleNbBins * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] - 1 );
         } // if( 0 == fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] )
            else fvulChanLastHitTime[ uAsicIdx ][ usChan ] +=
                        stsxyter::kuHitNbTsBins   * static_cast<ULong64_t>( fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] - 1 )
                      + stsxyter::kuTsCycleNbBins * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] );

      } // else if overlap bit from hit are exactly 1 unit behind the one from last Ts_Msb
      else
      {
         // Overlap is neither matching nor 1 less than bits from TS_MSB
         // Should not happen if I understood properly how the ASIC behaves ?!?
         LOG(FATAL) << "CbmCern2017MonitorSts::FillHitInfo => "
                    << "TS overlap bits from hit not fitting the ones from last TS MSB on this eLink! \n"
                    << "Hit Overlap: 0x" << std::hex << usTsOver << " Full TS: 0x" << usFullTs << "\n"
                    << "MSB Overlap: 0x" << (fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] & stsxyter::kusMaskTsMsbOver)
                    << " Ts Msb: 0x" << fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] << "\n"
                    << "Other eLinks: \n";
         for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
            LOG(FATAL) << std::setw(3) << std::dec << uElink << " => "
                       << std::hex << "0x" << (fvuCurrentTsMsb[fuCurrDpbIdx][uElink] & stsxyter::kusMaskTsMsbOver)
                       << " => " << "0x" << (fvuCurrentTsMsb[fuCurrDpbIdx][uElink] & stsxyter::kusMaskTsMsbOver) << "\n";
         LOG(FATAL) << std::dec << FairLogger::endl;
         return;
      } // else of overlap checks
*/
      // Use TS w/o overlap bits as they will anyway come from the TS_MSB
   fvulChanLastHitTime[ uAsicIdx ][ usChan ] = usRawTs;
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
   UInt_t uTsMsbCorr = ( stsxyter::kuHitNbOverBins + fvuCurrentTsMsbOver[fuCurrDpbIdx][usElinkIdx] - usTsOver )
                      % stsxyter::kuHitNbOverBins;
   fhStsChanOverDiff[ uAsicIdx ]->Fill( usChan, uTsMsbCorr );

      // Need TS cycle counter as otherwise period of only 102.400 us!!!
      // => Check if we need also a TS MSB cycle correction
   if( fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] < uTsMsbCorr )
   {
      if( 0 == fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] )
      {
         // TO BE CHECKED
         LOG(WARNING) << "CbmCern2017MonitorSts::FillHitInfo => "
                    << "Ignore Hit as coming from TS_MSB before system start!"
                    << FairLogger::endl;
         return;
      } // if( 0 == fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] )

      fvulChanLastHitTime[ uAsicIdx ][ usChan ] +=
                  stsxyter::kuHitNbTsBins   * static_cast<ULong64_t>(  fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx]
                                                                     + stsxyter::kuHitNbTsBins - uTsMsbCorr )
                + stsxyter::kuTsCycleNbBins * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] - 1 );
   } // if( fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] < uTsMsbCorr )
      else fvulChanLastHitTime[ uAsicIdx ][ usChan ] +=
                  stsxyter::kuHitNbTsBins   * static_cast<ULong64_t>( fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] - uTsMsbCorr )
                + stsxyter::kuTsCycleNbBins * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] );

   // Convert the Hit time in bins to Hit time in ns
   Double_t dHitTimeNs = fvulChanLastHitTime[ uAsicIdx ][ usChan ] * stsxyter::kdClockCycleNs;

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
   fvdChanLastHitTime[ uAsicIdx ][ usChan ] = dHitTimeNs;

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

}

void CbmCern2017MonitorSts::FillTsMsbInfo( stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx )
{
   UShort_t usValA    = mess.GetTsMsbValA();
   UShort_t usValB    = mess.GetTsMsbValB();
   UShort_t usValC    = mess.GetTsMsbValC();
   bool     bCrcCheck = mess.TsMsbCrcCheck();

   // Fill CRC check histo as independent of other data processing
   fhStsElinkTsMsbCrc[fuCurrDpbIdx]->Fill( usElinkIdx, ( bCrcCheck ? 1 : 0 ) );

   if( bCrcCheck )
   {
      // Everything ok

      // Update Status counters
      if( usValA < fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] )
         fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] ++;
      fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] = usValA;

      fhStsAsicTsMsb->Fill( fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx], uAsicIdx );
      fhStsAsicTsMsbMaj->Fill( uAsicIdx, 3 );
      fhStsElinkTsMsbMaj[fuCurrDpbIdx]->Fill( usElinkIdx, 3 );
   } // if( bCrcCheck )
      else
      {
         // Either nor 3 agreement or bad CRC
         UShort_t uMajVal = 0;
         mess.GetTsMsbValCorr( uMajVal );
         if( stsxyter::kusInvalidTsMsb == uMajVal )
         {
            // No majority => value cannot be recovered & used
            LOG(WARNING) << "CbmCern2017MonitorSts::FillTsMsbInfo => "
                         << " Ignoring value for DPB " << fuCurrDpbIdx
                         << " and link " << usElinkIdx << " (ASIC " << uAsicIdx << ") "
                         << " as not majority can be found!! CRC check " << bCrcCheck
                         << FairLogger::endl;

            fhStsAsicTsMsbMaj->Fill( uAsicIdx, 1 );
            fhStsElinkTsMsbMaj[fuCurrDpbIdx]->Fill( usElinkIdx, 1 );

            return;
         } // if( stsxyter::kusInvalidTsMsb == uMajVal )

         // Update Status counters
         if( uMajVal < fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] )
            fvuCurrentTsMsbCycle[fuCurrDpbIdx][usElinkIdx] ++;
         fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx] = uMajVal;

         fhStsAsicTsMsb->Fill( fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx], uAsicIdx );

         // Get majority count used (should be 2 or 3)
         UShort_t uMajCnt = mess. GetTsMsbMajCnt();
         fhStsAsicTsMsbMaj->Fill( uAsicIdx, uMajCnt );
         fhStsElinkTsMsbMaj[fuCurrDpbIdx]->Fill( usElinkIdx, uMajCnt );
      } // else of if( bCrcCheck )

   // Update the overlap bits for this eLink
   fvuCurrentTsMsbOver[fuCurrDpbIdx][usElinkIdx] =  fvuCurrentTsMsb[fuCurrDpbIdx][usElinkIdx]
                                                  & stsxyter::kusMaskTsMsbOver;
}

void CbmCern2017MonitorSts::Reset()
{
}

void CbmCern2017MonitorSts::Finish()
{

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   LOG(INFO) << "CbmCern2017MonitorSts statistics are " << FairLogger::endl;
   LOG(INFO) << " Hit      messages: " << fmMsgCounter[ stsxyter::MessType::Hit   ] << FairLogger::endl
             << " Ts MSB   messages: " << fmMsgCounter[ stsxyter::MessType::TsMsb ] << FairLogger::endl
             << " Dummy    messages: " << fmMsgCounter[ stsxyter::MessType::TsMsb ] << FairLogger::endl
             << " Read Ack messages: " << fmMsgCounter[ stsxyter::MessType::TsMsb ] << FairLogger::endl
             << " Ack      messages: " << fmMsgCounter[ stsxyter::MessType::TsMsb ] << FairLogger::endl;

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;

   SaveAllHistos();

}


void CbmCern2017MonitorSts::FillOutput(CbmDigi* digi)
{
}

void CbmCern2017MonitorSts::SaveAllHistos( TString sFileName )
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

   fhStsMessType->Reset();
   fhStsSysMessType->Reset();

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fhStsChanCounts[ uXyterIdx ]->Write();
      fhStsChanRawAdc[ uXyterIdx ]->Write();
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
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

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
void CbmCern2017MonitorSts::ResetAllHistos()
{
   LOG(INFO) << "Reseting all STS histograms." << FairLogger::endl;

   fhStsMessType->Reset();
   fhStsSysMessType->Reset();

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fhStsChanCounts[ uXyterIdx ]->Reset();
      fhStsChanRawAdc[ uXyterIdx ]->Reset();
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
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

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

void CbmCern2017MonitorSts::SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize )
{
   TDatime * fRunStartDateTime     = new TDatime( dateIn, timeIn);
   fiRunStartDateTimeSec = fRunStartDateTime->Convert();
   fiBinSizeDatePlots    = iBinSize;

   LOG(INFO) << "Assigned new MUCH Run Start Date-Time: " << fRunStartDateTime->AsString() << FairLogger::endl;
}

ClassImp(CbmCern2017MonitorSts)
