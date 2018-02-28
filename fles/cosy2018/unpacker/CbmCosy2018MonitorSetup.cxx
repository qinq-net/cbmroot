// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmCosy2018MonitorSetup                        -----
// -----                Created 27/02/18  by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmCosy2018MonitorSetup.h"

// Data

// CbmRoot
#include "CbmCern2017UnpackParHodo.h"
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

Bool_t bCosy2018ResetSetupHistos_H = kFALSE;
Bool_t bCosy2018WriteSetupHistos_H = kFALSE;

CbmCosy2018MonitorSetup::CbmCosy2018MonitorSetup() :
   CbmTSUnpack(),
   fuOverlapMsNb(0),
   fUnpackParHodo(NULL),
   fUnpackParSts(NULL),
   fuNrOfDpbs(0),
   fDpbIdIndexMap(),
   fuNbElinksPerDpb(0),
   fuNbStsXyters(0),
   fuNbChanPerAsic(0),
   fvuElinkToAsic(),
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
   fvmChanHitsInTs(),
   fdStartTime(-1.0),
   fdStartTimeMsSz(-1.0),
   ftStartTimeUnix( std::chrono::steady_clock::now() ),
   fvmHitsInTs(),
   fLastSortedHit1X(),
   fLastSortedHit1Y(),
   fLastSortedHit2X(),
   fLastSortedHit2Y(),
   fLastSortedHit1N(),
   fLastSortedHit1P(),
   fLastSortedHit2N(),
   fLastSortedHit2P(),
   fuMaxNbMicroslices(100),
   fbLongHistoEnable( kFALSE ),
   fuLongHistoNbSeconds( 0 ),
   fuLongHistoBinSizeSec( 0 ),
   fuLongHistoBinNb( 0 ),
   fHM(new CbmHistManager()),
   fhHodoMessType(NULL),
   fhHodoSysMessType(NULL),
   fhHodoMessTypePerDpb(NULL),
   fhHodoSysMessTypePerDpb(NULL),
   fhHodoMessTypePerElink(NULL),
   fhHodoSysMessTypePerElink(NULL),
   fhHodoChanCntRaw(),
   fhHodoChanAdcRaw(),
   fhHodoChanAdcRawProf(),
   fhHodoChanRawTs(),
   fhHodoChanMissEvt(),
   fhHodoChanHitRateEvo(),
   fhHodoFebRateEvo(),
   fhHodoChanHitRateEvoLong(),
   fhHodoFebRateEvoLong(),
   fdCoincBorderHodo(  50.0 ),
   fdCoincBorderSts(   75.0 ),
   fdCoincBorder(     150.0 ),
   fhSetupSortedDtX1Y1(NULL),
   fhSetupSortedDtX2Y2(NULL),
   fhSetupSortedDtN1P1(NULL),
   fhSetupSortedDtX1Y1X2Y2(NULL),
   fhSetupSortedDtX1Y1X2Y2N1P1(NULL),
   fhSetupSortedMapX1Y1(NULL),
   fhSetupSortedMapX2Y2(NULL),
   fhSetupSortedMapN1P1(NULL),
   fhSetupSortedCntEvoX1Y1(NULL),
   fhSetupSortedCntEvoX2Y2(NULL),
   fhSetupSortedCntEvoN1P1(NULL),
   fhBothHodoSortedDtX1Y1(NULL),
   fhBothHodoSortedDtX2Y2(NULL),
   fhBothHodoSortedDtX1Y1X2Y2N1P1(NULL),
   fhBothHodoSortedMapX1Y1(NULL),
   fhBothHodoSortedMapX2Y2(NULL),
   fhBothHodoSortedCntEvoX1Y1(NULL),
   fhBothHodoSortedCntEvoX2Y2(NULL),
   fhSystSortedDtX1Y1(NULL),
   fhSystSortedDtX2Y2(NULL),
   fhSystSortedDtN1P1(NULL),
   fhSystSortedDtX1Y1X2Y2(NULL),
   fhSystSortedMapX1Y1(NULL),
   fhSystSortedMapX2Y2(NULL),
   fhSystSortedMapN1P1(NULL),
   fhSystSortedCntEvoX1Y1(NULL),
   fhSystSortedCntEvoX2Y2(NULL),
   fhSystSortedCntEvoN1P1(NULL),
   fcMsSizeAll(NULL)
{
}

CbmCosy2018MonitorSetup::~CbmCosy2018MonitorSetup()
{
}

Bool_t CbmCosy2018MonitorSetup::Init()
{
  LOG(INFO) << "Initializing flib StsXyter unpacker for STS" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  return kTRUE;
}

void CbmCosy2018MonitorSetup::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
         << FairLogger::endl;
   fUnpackParHodo = (CbmCern2017UnpackParHodo*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmCern2017UnpackParHodo"));
   fUnpackParSts  = (CbmCern2017UnpackParSts*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmCern2017UnpackParSts"));
}


Bool_t CbmCosy2018MonitorSetup::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
         << FairLogger::endl;

   Bool_t bReInit = ReInitContainers();
   CreateHistograms();

   return bReInit;
}

Bool_t CbmCosy2018MonitorSetup::ReInitContainers()
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

   LOG(INFO) << "ASIC Idx for Hodo 1: " << fUnpackParHodo->GetAsicIndexHodo1()
             << FairLogger::endl;
   LOG(INFO) << "ASIC Idx for Hodo 2: " << fUnpackParHodo->GetAsicIndexHodo2()
             << FairLogger::endl;

   LOG(INFO) << "XY swapped in Hodo 1: " << fUnpackParHodo->IsXySwappedHodo1()
             << FairLogger::endl;
   LOG(INFO) << "XY swapped in Hodo 2: " << fUnpackParHodo->IsXySwappedHodo2()
             << FairLogger::endl;

   LOG(INFO) << "X axis inverted in Hodo 1: " << fUnpackParHodo->IsXInvertedHodo1()
             << FairLogger::endl;
   LOG(INFO) << "X axis inverted in Hodo 1: " << fUnpackParHodo->IsYInvertedHodo1()
             << FairLogger::endl;
   LOG(INFO) << "X axis inverted in Hodo 2: " << fUnpackParHodo->IsXInvertedHodo2()
             << FairLogger::endl;
   LOG(INFO) << "X axis inverted in Hodo 2: " << fUnpackParHodo->IsYInvertedHodo2()
             << FairLogger::endl;

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
   fvmChanHitsInTs.resize( fuNbStsXyters );
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fvulChanLastHitTime[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvdChanLastHitTime[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvuChanNbHitsInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvdChanLastHitTimeInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvusChanLastHitAdcInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvmChanHitsInTs[ uXyterIdx ].resize( fuNbChanPerAsic );
      for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
      {
         fvulChanLastHitTime[ uXyterIdx ][ uChan ] = 0;
         fvdChanLastHitTime[ uXyterIdx ][ uChan ] = -1.0;

         fvuChanNbHitsInMs[ uXyterIdx ][ uChan ].resize( fuMaxNbMicroslices );
         fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ].resize( fuMaxNbMicroslices );
         fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ].resize( fuMaxNbMicroslices );
         fvmChanHitsInTs[ uXyterIdx ][ uChan ].clear();
         for( UInt_t uMsIdx = 0; uMsIdx < fuMaxNbMicroslices; ++uMsIdx )
         {
            fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
            fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = -1.0;
            fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
         } // for( UInt_t uMsIdx = 0; uMsIdx < fuMaxNbMicroslices; ++uMsIdx )
      } // for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   LOG(INFO) << "CbmCosy2018MonitorSetup::ReInitContainers => Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                << FairLogger::endl;
   LOG(INFO) << "CbmCosy2018MonitorSetup::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fuNbChanPerAsic
                << FairLogger::endl;
   LOG(INFO) << "CbmCosy2018MonitorSetup::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuMaxNbMicroslices
                << FairLogger::endl;
   return kTRUE;
}

void CbmCosy2018MonitorSetup::CreateHistograms()
{
   TString sHistName{""};
   TString title{""};

   sHistName = "hHodoMessageType";
   title = "Nb of message for each type; Type";
   fhHodoMessType = new TH1I(sHistName, title, 5, 0., 5.);
   fhHodoMessType->GetXaxis()->SetBinLabel( 1, "Dummy");
   fhHodoMessType->GetXaxis()->SetBinLabel( 2, "Hit");
   fhHodoMessType->GetXaxis()->SetBinLabel( 3, "TsMsb");
   fhHodoMessType->GetXaxis()->SetBinLabel( 4, "Epoch");
   fhHodoMessType->GetXaxis()->SetBinLabel( 5, "Empty");
/* *** Missing int + MessType OP!!!! ****
   fhHodoMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::Dummy,       "Dummy");
   fhHodoMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::Hit,         "Hit");
   fhHodoMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::TsMsb,       "TsMsb");
   fhHodoMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::ReadDataAck, "ReadDataAck");
   fhHodoMessType->GetXaxis()->SetBinLabel(1 + stsxyter::MessType::Ack,         "Ack");
*/
   fHM->Add(sHistName.Data(), fhHodoMessType);

   sHistName = "hHodoSysMessType";
   title = "Nb of system message for each type; System Type";
   fhHodoSysMessType = new TH1I(sHistName, title, 17, 0., 17.);
/*
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   hSysMessType->GetXaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/
   fHM->Add(sHistName.Data(), fhHodoSysMessType);

   sHistName = "hHodoMessageTypePerDpb";
   title = "Nb of message of each type for each DPB; DPB; Type";
   fhHodoMessTypePerDpb = new TH2I(sHistName, title, fuNrOfDpbs, 0, fuNrOfDpbs, 5, 0., 5.);
   fhHodoMessTypePerDpb->GetYaxis()->SetBinLabel( 1, "Dummy");
   fhHodoMessTypePerDpb->GetYaxis()->SetBinLabel( 2, "Hit");
   fhHodoMessTypePerDpb->GetYaxis()->SetBinLabel( 3, "TsMsb");
   fhHodoMessTypePerDpb->GetYaxis()->SetBinLabel( 4, "Epoch");
   fhHodoMessTypePerDpb->GetYaxis()->SetBinLabel( 5, "Empty");
/* *** Missing int + MessType OP!!!! ****
   fhHodoMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Dummy,       "Dummy");
   fhHodoMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Hit,         "Hit");
   fhHodoMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::TsMsb,       "TsMsb");
   fhHodoMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::ReadDataAck, "ReadDataAck");
   fhHodoMessType->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Ack,         "Ack");
*/
   fHM->Add(sHistName.Data(), fhHodoMessTypePerDpb);

   sHistName = "hHodoSysMessTypePerDpb";
   title = "Nb of system message of each type for each DPB; DPB; System Type";
   fhHodoSysMessTypePerDpb = new TH2I(sHistName, title, fuNrOfDpbs, 0, fuNrOfDpbs, 17, 0., 17.);
/*
   hSysMessType->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   hSysMessType->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   hSysMessType->GetYaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/
   fHM->Add(sHistName.Data(), fhHodoSysMessTypePerDpb);

   sHistName = "hHodoMessageTypePerElink";
   title = "Nb of message of each type for each eLink; eLink; Type";
   fhHodoMessTypePerElink = new TH2I(sHistName, title, fuNrOfDpbs * fuNbElinksPerDpb, 0, fuNrOfDpbs * fuNbElinksPerDpb, 5, 0., 5.);
   fhHodoMessTypePerElink->GetYaxis()->SetBinLabel( 1,       "Dummy");
   fhHodoMessTypePerElink->GetYaxis()->SetBinLabel( 2,         "Hit");
   fhHodoMessTypePerElink->GetYaxis()->SetBinLabel( 3,       "TsMsb");
   fhHodoMessTypePerElink->GetYaxis()->SetBinLabel( 4, "ReadDataAck");
   fhHodoMessTypePerElink->GetYaxis()->SetBinLabel( 5,         "Ack");
/* *** Missing int + MessType OP!!!! ****
   fhHodoMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Dummy,       "Dummy");
   fhHodoMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Hit,         "Hit");
   fhHodoMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::TsMsb,       "TsMsb");
   fhHodoMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::ReadDataAck, "ReadDataAck");
   fhHodoMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Ack,         "Ack");
*/
   fHM->Add(sHistName.Data(), fhHodoMessTypePerElink);

   sHistName = "hHodoSysMessTypePerElink";
   title = "Nb of system message of each type for each eLink; eLink; System Type";
   fhHodoSysMessTypePerElink = new TH2I(sHistName, title, fuNrOfDpbs * fuNbElinksPerDpb, 0, fuNrOfDpbs * fuNbElinksPerDpb, 17, 0., 17.);
/*
   fhHodoSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   fhHodoSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   fhHodoSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/
   fHM->Add(sHistName.Data(), fhHodoSysMessTypePerElink);

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
      sHistName = Form( "hHodoChanCntRaw_%03u", uXyterIdx );
      title = Form( "Hits Count per channel, StsXyter #%03u; Channel; Hits []", uXyterIdx );
      fhHodoChanCntRaw.push_back( new TH1I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhHodoChanCntRaw[ uXyterIdx ] );

      // Raw Adc Distribution
      sHistName = Form( "hHodoChanAdcRaw_%03u", uXyterIdx );
      title = Form( "Raw Adc distribution per channel, StsXyter #%03u; Channel []; Adc []; Hits []", uXyterIdx );
      fhHodoChanAdcRaw.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 stsxyter::kuHitNbAdcBins, -0.5, stsxyter::kuHitNbAdcBins -0.5 ) );
      fHM->Add(sHistName.Data(), fhHodoChanAdcRaw[ uXyterIdx ] );

      // Raw Adc Distribution profile
      sHistName = Form( "hHodoChanAdcRawProfc_%03u", uXyterIdx );
      title = Form( "Raw Adc prodile per channel, StsXyter #%03u; Channel []; Adc []", uXyterIdx );
      fhHodoChanAdcRawProf.push_back( new TProfile(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhHodoChanAdcRawProf[ uXyterIdx ] );

      // Raw Ts Distribution
      sHistName = Form( "hHodoChanRawTs_%03u", uXyterIdx );
      title = Form( "Raw Timestamp distribution per channel, StsXyter #%03u; Channel []; Ts []; Hits []", uXyterIdx );
      fhHodoChanRawTs.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 stsxyter::kuHitNbTsBins, -0.5, stsxyter::kuHitNbTsBins -0.5 ) );
      fHM->Add(sHistName.Data(), fhHodoChanRawTs[ uXyterIdx ] );

      // Missed event flag
      sHistName = Form( "hHodoChanMissEvt_%03u", uXyterIdx );
      title = Form( "Missed Event flags per channel, StsXyter #%03u; Channel []; Miss Evt []; Hits []", uXyterIdx );
      fhHodoChanMissEvt.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 2, -0.5, 1.5 ) );
      fHM->Add(sHistName.Data(), fhHodoChanMissEvt[ uXyterIdx ] );

      // Hit rates evo per channel
      sHistName = Form( "hHodoChanRateEvo_%03u", uXyterIdx );
      title = Form( "Hits per second & channel in StsXyter #%03u; Time [s]; Channel []; Hits []", uXyterIdx );
      fhHodoChanHitRateEvo.push_back( new TH2I( sHistName, title,
                                                1800, 0, 1800,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhHodoChanHitRateEvo[ uXyterIdx ] );

      // Hit rates evo per StsXyter
      sHistName = Form( "hHodoFebRateEvo_%03u", uXyterIdx );
      title = Form( "Hits per second in StsXyter #%03u; Time [s]; Hits []", uXyterIdx );
      fhHodoFebRateEvo.push_back( new TH1I(sHistName, title, 1800, 0, 1800 ) );
      fHM->Add(sHistName.Data(), fhHodoFebRateEvo[ uXyterIdx ] );

      // Hit rates evo per channel, 1 minute bins, 24h
      sHistName = Form( "hHodoChanRateEvoLong_%03u", uXyterIdx );
      title = Form( "Hits per second & channel in StsXyter #%03u; Time [min]; Channel []; Hits []", uXyterIdx );
      fhHodoChanHitRateEvoLong.push_back( new TH2D( sHistName, title,
                                                1440, 0, 1440,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhHodoChanHitRateEvoLong[ uXyterIdx ] );

      // Hit rates evo per StsXyter, 1 minute bins, 24h
      sHistName = Form( "hHodoFebRateEvoLong_%03u", uXyterIdx );
      title = Form( "Hits per second in StsXyter #%03u; Time [min]; Hits []", uXyterIdx );
      fhHodoFebRateEvoLong.push_back( new TH1D(sHistName, title, 1440, 0, 1440 ) );
      fHM->Add(sHistName.Data(), fhHodoFebRateEvoLong[ uXyterIdx ] );

/*
      if( kTRUE == fbLongHistoEnable )
      {
         UInt_t uAlignedLimit = fuLongHistoNbSeconds - (fuLongHistoNbSeconds % fuLongHistoBinSizeSec);
         fuLongHistoBinNb = uAlignedLimit / fuLongHistoBinSizeSec;

         sHistName = Form( "hFebRateEvoLong_%03u", uXyterIdx );
         title = Form( "Mean rate VS run time in same MS in StsXyter #%03u; Time in run [s]; Rate [1/s]", uXyterIdx );
         fhFebRateEvoLong.push_back( new TH1D( sHistName, title,
                                                   fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5) );
         fHM->Add(sHistName.Data(), fhFebRateEvoLong[ uXyterIdx ] );
#ifdef USE_HTTP_SERVER
         if( server ) server->Register("/StsRaw", fhFebRateEvoLong[ uXyterIdx ] );
#endif

         sHistName = Form( "hFebChRateEvoLong_%03u", uXyterIdx );
         title = Form( "Mean rate per channel VS run time in StsXyter #%03u; Time in run [s]; Channel []; Rare [1/s]", uXyterIdx );
         fhFebChRateEvoLong.push_back( new TH2D( sHistName, title,
                                                   fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5,
                                                   fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
         fHM->Add(sHistName.Data(), fhFebChRateEvoLong[ uXyterIdx ] );
#ifdef USE_HTTP_SERVER
         if( server ) server->Register("/StsRaw", fhFebChRateEvoLong[ uXyterIdx ] );
#endif
      } // if( kTRUE == fbLongHistoEnable )
*/
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++///

   UInt_t uNbBinEvo = 32768 + 1;
   Double_t dMaxEdgeEvo = stsxyter::kdClockCycleNs
                         * static_cast< Double_t >( uNbBinEvo ) / 2.0;
   Double_t dMinEdgeEvo = dMaxEdgeEvo * -1.0;

   /// Coincidences inside each detector ----------------------------///
   sHistName = "fhSetupSortedDtX1Y1";
   title =  "Time diff for hits Hodo 1 X and Hodo 1 Y; tY1 - tX1 [ns]; Counts";
   fhSetupSortedDtX1Y1 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
   fHM->Add(sHistName.Data(), fhSetupSortedDtX1Y1);

   sHistName = "fhSetupSortedDtX2Y2";
   title =  "Time diff for hits Hodo 2 X and Hodo 2 Y; tY2 - tX2 [ns]; Counts";
   fhSetupSortedDtX2Y2 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
   fHM->Add(sHistName.Data(), fhSetupSortedDtX2Y2);

   sHistName = "fhSetupSortedDtN1P1";
   title =  "Time diff for hits Sts 1 N and Hodo 1 P; tP1 - tN1 [ns]; Counts";
   fhSetupSortedDtN1P1 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
   fHM->Add(sHistName.Data(), fhSetupSortedDtX2Y2);

   sHistName = "fhSetupSortedDtX1Y1X2Y2";
   title =  "Time diff for hits Hodo 1 (X,Y) vs Hodo 2 (X,Y); t<X2,Y2> - t<X1,Y1> [ns]; Counts";
   fhSetupSortedDtX1Y1X2Y2 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
   fHM->Add(sHistName.Data(), fhSetupSortedDtX1Y1X2Y2);

   sHistName = "fhSetupSortedDtX1Y1X2Y2N1P1";
   title =  "Time diff for hits Hodo (X1,Y1,X2,Y2) vs Sts 1 (N,P); t<N1,P1> - t<X1,Y1,X2,Y2> [ns]; Counts";
   fhSetupSortedDtX1Y1X2Y2N1P1 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
   fHM->Add(sHistName.Data(), fhSetupSortedDtX1Y1X2Y2N1P1);

   sHistName = "fhSetupSortedMapX1Y1";
   title = "Sorted hits in coincidence for hodoscope 1 axis X and Y; X channel Hodo 1 []; Y channel Hodo 1 []; hits []";
   fhSetupSortedMapX1Y1 = new TH2I( sHistName, title,
                                  fuNbChanPerAsic/2, -0.5, fuNbChanPerAsic/2 - 0.5,
                                  fuNbChanPerAsic/2, -0.5, fuNbChanPerAsic/2 - 0.5 );
   fHM->Add(sHistName.Data(), fhSetupSortedMapX1Y1 );

   sHistName = "fhSetupSortedMapX2Y2";
   title = "Sorted hits in coincidence for hodoscope 2 axis X and Y; X channel Hodo 2 []; Y channel Hodo 2 []; hits []";
   fhSetupSortedMapX2Y2 = new TH2I( sHistName, title,
                                  fuNbChanPerAsic/2, -0.5, fuNbChanPerAsic/2 - 0.5,
                                  fuNbChanPerAsic/2, -0.5, fuNbChanPerAsic/2 - 0.5 );
   fHM->Add(sHistName.Data(), fhSetupSortedMapX2Y2 );

   sHistName = "fhSetupSortedMapN1P1";
   title = "Sorted hits in coincidence for Sts 1 sides N and P; N channel Sts 1 []; P channel Sts 1 []; hits []";
   fhSetupSortedMapN1P1 = new TH2I( sHistName, title,
                                  fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                  fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 );
   fHM->Add(sHistName.Data(), fhSetupSortedMapX2Y2 );

   // Coincidence counts evolution between some axis of the hodoscopes
   sHistName = "hSetupSortedCntEvoX1Y1";
   title = "Nb of coincidences in both X1 and Y1 per s; Time [s]; X1-Y1 coincidences []";
   fhSetupSortedCntEvoX1Y1 = new TH1I(sHistName, title, 1800, 0, 1800 );
   fHM->Add(sHistName.Data(), fhSetupSortedCntEvoX1Y1 );

   sHistName = "hSetupSortedCntEvoX2Y2";
   title = "Nb of coincidences in both X2 and Y2 per s; Time [s]; X2-Y2 coincidences []";
   fhSetupSortedCntEvoX2Y2 = new TH1I(sHistName, title, 1800, 0, 1800 );
   fHM->Add(sHistName.Data(), fhSetupSortedCntEvoX2Y2 );

   sHistName = "hSetupSortedCntEvoN1P1";
   title = "Nb of coincidences in both N1 and P1 per s; Time [s]; N1-P1 coincidences []";
   fhSetupSortedCntEvoN1P1 = new TH1I(sHistName, title, 1800, 0, 1800 );
   fHM->Add(sHistName.Data(), fhSetupSortedCntEvoX2Y2 );

   /// Coincidences between hodoscopes ------------------------------///
   sHistName = "fhBothHodoSortedDtX1Y1";
   title =  "Time diff for hits Hodo 1 X and Hodo 1 Y; tY1 - tX1 [ns]; Counts";
   fhBothHodoSortedDtX1Y1 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
   fHM->Add(sHistName.Data(), fhBothHodoSortedDtX1Y1);

   sHistName = "fhBothHodoSortedDtX2Y2";
   title =  "Time diff for hits Hodo 2 X and Hodo 2 Y; tY2 - tX2 [ns]; Counts";
   fhBothHodoSortedDtX2Y2 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
   fHM->Add(sHistName.Data(), fhBothHodoSortedDtX2Y2);

   sHistName = "fhBothHodoSortedDtX1Y1X2Y2N1P1";
   title =  "Time diff for hits Hodo (X1,Y1,X2,Y2) vs Sts 1 (N,P); t<N1,P1> - t<X1,Y1,X2,Y2> [ns]; Counts";
   fhBothHodoSortedDtX1Y1X2Y2N1P1 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
   fHM->Add(sHistName.Data(), fhBothHodoSortedDtX1Y1X2Y2N1P1);

   sHistName = "fhBothHodoSortedMapX1Y1";
   title = "Sorted hits in coincidence for hodoscope 1 axis X and Y; X channel Hodo 1 []; Y channel Hodo 1 []; hits []";
   fhBothHodoSortedMapX1Y1 = new TH2I( sHistName, title,
                                  fuNbChanPerAsic/2, -0.5, fuNbChanPerAsic/2 - 0.5,
                                  fuNbChanPerAsic/2, -0.5, fuNbChanPerAsic/2 - 0.5 );
   fHM->Add(sHistName.Data(), fhBothHodoSortedMapX1Y1 );

   sHistName = "fhBothHodoSortedMapX2Y2";
   title = "Sorted hits in coincidence for hodoscope 2 axis X and Y; X channel Hodo 2 []; Y channel Hodo 2 []; hits []";
   fhBothHodoSortedMapX2Y2 = new TH2I( sHistName, title,
                                  fuNbChanPerAsic/2, -0.5, fuNbChanPerAsic/2 - 0.5,
                                  fuNbChanPerAsic/2, -0.5, fuNbChanPerAsic/2 - 0.5 );
   fHM->Add(sHistName.Data(), fhBothHodoSortedMapX2Y2 );

   // Coincidence counts evolution between some axis of the hodoscopes
   sHistName = "hBothHodoSortedCntEvoX1Y1";
   title = "Nb of coincidences in both X1 and Y1 per s; Time [s]; X1-Y1 coincidences []";
   fhBothHodoSortedCntEvoX1Y1 = new TH1I(sHistName, title, 1800, 0, 1800 );
   fHM->Add(sHistName.Data(), fhBothHodoSortedCntEvoX1Y1 );

   sHistName = "hBothHodoSortedCntEvoX2Y2";
   title = "Nb of coincidences in both X2 and Y2 per s; Time [s]; X2-Y2 coincidences []";
   fhBothHodoSortedCntEvoX2Y2 = new TH1I(sHistName, title, 1800, 0, 1800 );
   fHM->Add(sHistName.Data(), fhBothHodoSortedCntEvoX2Y2 );

   /// Coincidences between hodoscopes ans STS ----------------------///
   sHistName = "fhSystSortedDtX1Y1";
   title =  "Time diff for hits Hodo 1 X and Hodo 1 Y; tY1 - tX1 [ns]; Counts";
   fhSystSortedDtX1Y1 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
   fHM->Add(sHistName.Data(), fhSystSortedDtX1Y1);

   sHistName = "fhSystSortedDtX2Y2";
   title =  "Time diff for hits Hodo 2 X and Hodo 2 Y; tY2 - tX2 [ns]; Counts";
   fhSystSortedDtX2Y2 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
   fHM->Add(sHistName.Data(), fhSystSortedDtX2Y2);

   sHistName = "fhSystSortedDtN1P1";
   title =  "Time diff for hits Sts 1 N and Hodo 1 P; tP1 - tN1 [ns]; Counts";
   fhSystSortedDtN1P1 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
   fHM->Add(sHistName.Data(), fhSystSortedDtX2Y2);

   sHistName = "fhSystSortedDtX1Y1X2Y2";
   title =  "Time diff for hits Hodo 1 (X,Y) vs Hodo 2 (X,Y); t<X2,Y2> - t<X1,Y1> [ns]; Counts";
   fhSystSortedDtX1Y1X2Y2 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
   fHM->Add(sHistName.Data(), fhSystSortedDtX1Y1X2Y2);

   sHistName = "fhSystSortedMapX1Y1";
   title = "Sorted hits in coincidence for hodoscope 1 axis X and Y; X channel Hodo 1 []; Y channel Hodo 1 []; hits []";
   fhSystSortedMapX1Y1 = new TH2I( sHistName, title,
                                  fuNbChanPerAsic/2, -0.5, fuNbChanPerAsic/2 - 0.5,
                                  fuNbChanPerAsic/2, -0.5, fuNbChanPerAsic/2 - 0.5 );
   fHM->Add(sHistName.Data(), fhSystSortedMapX1Y1 );

   sHistName = "fhSystSortedMapX2Y2";
   title = "Sorted hits in coincidence for hodoscope 2 axis X and Y; X channel Hodo 2 []; Y channel Hodo 2 []; hits []";
   fhSystSortedMapX2Y2 = new TH2I( sHistName, title,
                                  fuNbChanPerAsic/2, -0.5, fuNbChanPerAsic/2 - 0.5,
                                  fuNbChanPerAsic/2, -0.5, fuNbChanPerAsic/2 - 0.5 );
   fHM->Add(sHistName.Data(), fhSystSortedMapX2Y2 );

   sHistName = "fhSystSortedMapN1P1";
   title = "Sorted hits in coincidence for Sts 1 sides N and P; N channel Sts 1 []; P channel Sts 1 []; hits []";
   fhSystSortedMapN1P1 = new TH2I( sHistName, title,
                                  fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                  fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 );
   fHM->Add(sHistName.Data(), fhSystSortedMapX2Y2 );

   // Coincidence counts evolution between some axis of the hodoscopes
   sHistName = "hSystSortedCntEvoX1Y1";
   title = "Nb of coincidences in both X1 and Y1 per s; Time [s]; X1-Y1 coincidences []";
   fhSystSortedCntEvoX1Y1 = new TH1I(sHistName, title, 1800, 0, 1800 );
   fHM->Add(sHistName.Data(), fhSystSortedCntEvoX1Y1 );

   sHistName = "hSystSortedCntEvoX2Y2";
   title = "Nb of coincidences in both X2 and Y2 per s; Time [s]; X2-Y2 coincidences []";
   fhSystSortedCntEvoX2Y2 = new TH1I(sHistName, title, 1800, 0, 1800 );
   fHM->Add(sHistName.Data(), fhSystSortedCntEvoX2Y2 );

   sHistName = "hSystSortedCntEvoN1P1";
   title = "Nb of coincidences in both N1 and P1 per s; Time [s]; N1-P1 coincidences []";
   fhSystSortedCntEvoN1P1 = new TH1I(sHistName, title, 1800, 0, 1800 );
   fHM->Add(sHistName.Data(), fhSystSortedCntEvoX2Y2 );

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++///
/*
   // Distribution of the TS_MSB per StsXyter
   sHistName = "hHodoFebTsMsb";
   title = "Raw Timestamp Msb distribution per StsXyter; Ts MSB []; StsXyter []; Hits []";
   fhHodoFebTsMsb = new TH2I( sHistName, title, stsxyter::kuTsMsbNbTsBins, -0.5,   stsxyter::kuTsMsbNbTsBins - 0.5,
                                                fuNbStsXyters, -0.5, fuNbStsXyters - 0.5 );
   fHM->Add(sHistName.Data(), fhHodoFebTsMsb );
#ifdef USE_HTTP_SERVER
   if( server ) server->Register("/StsRaw", fhHodoFebTsMsb );
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
      server->Register("/HodoRaw", fhHodoMessType );
      server->Register("/HodoRaw", fhHodoSysMessType );
      server->Register("/HodoRaw", fhHodoMessTypePerDpb );
      server->Register("/HodoRaw", fhHodoSysMessTypePerDpb );
      server->Register("/HodoRaw", fhHodoMessTypePerElink );
      server->Register("/HodoRaw", fhHodoSysMessTypePerElink );
      for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      {
         server->Register("/HodoRaw", fhHodoChanCntRaw[ uXyterIdx ] );
         server->Register("/HodoRaw", fhHodoChanAdcRaw[ uXyterIdx ] );
         server->Register("/HodoRaw", fhHodoChanAdcRawProf[ uXyterIdx ] );
         server->Register("/HodoRaw", fhHodoChanRawTs[ uXyterIdx ] );
         server->Register("/HodoRaw", fhHodoChanMissEvt[ uXyterIdx ] );
         server->Register("/HodoRaw", fhHodoChanHitRateEvo[ uXyterIdx ] );
         server->Register("/HodoRaw", fhHodoFebRateEvo[ uXyterIdx ] );
         server->Register("/HodoRaw", fhHodoChanHitRateEvoLong[ uXyterIdx ] );
         server->Register("/HodoRaw", fhHodoFebRateEvoLong[ uXyterIdx ] );
      } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      server->Register("/Setup", fhSetupSortedDtX1Y1 );
      server->Register("/Setup", fhSetupSortedDtX2Y2 );
      server->Register("/Setup", fhSetupSortedDtN1P1 );
      server->Register("/Setup", fhSetupSortedDtX1Y1X2Y2 );
      server->Register("/Setup", fhSetupSortedDtX1Y1X2Y2N1P1 );
      server->Register("/Setup", fhSetupSortedMapX1Y1 );
      server->Register("/Setup", fhSetupSortedMapX2Y2 );
      server->Register("/Setup", fhSetupSortedMapN1P1 );
      server->Register("/Setup", fhSetupSortedCntEvoX1Y1 );
      server->Register("/Setup", fhSetupSortedCntEvoX2Y2 );
      server->Register("/Setup", fhSetupSortedCntEvoN1P1 );

      server->Register("/HodoBoth", fhBothHodoSortedDtX1Y1 );
      server->Register("/HodoBoth", fhBothHodoSortedDtX2Y2 );
      server->Register("/HodoBoth", fhBothHodoSortedDtX1Y1X2Y2N1P1 );
      server->Register("/HodoBoth", fhBothHodoSortedMapX1Y1 );
      server->Register("/HodoBoth", fhBothHodoSortedMapX2Y2 );
      server->Register("/HodoBoth", fhBothHodoSortedCntEvoX1Y1 );
      server->Register("/HodoBoth", fhBothHodoSortedCntEvoX2Y2 );

      server->Register("/Syst", fhSystSortedDtX1Y1 );
      server->Register("/Syst", fhSystSortedDtX2Y2 );
      server->Register("/Syst", fhSystSortedDtN1P1 );
      server->Register("/Syst", fhSystSortedDtX1Y1X2Y2 );
      server->Register("/Syst", fhSystSortedMapX1Y1 );
      server->Register("/Syst", fhSystSortedMapX2Y2 );
      server->Register("/Syst", fhSystSortedMapN1P1 );
      server->Register("/Syst", fhSystSortedCntEvoX1Y1 );
      server->Register("/Syst", fhSystSortedCntEvoX2Y2 );
      server->Register("/Syst", fhSystSortedCntEvoN1P1 );

      server->RegisterCommand("/Reset_All_Hodo", "bCosy2018ResetSetupHistos_H=kTRUE");
      server->RegisterCommand("/Write_All_Hodo", "bCosy2018WriteSetupHistos_H=kTRUE");

      server->Restrict("/Reset_All_Hodo", "allow=admin");
      server->Restrict("/Write_All_Hodo", "allow=admin");
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
      fhHodoChanCntRaw[ uXyterIdx ]->Draw();

      cStsSumm->cd(2);
      gPad->SetLogz();
      fhHodoChanAdcRaw[ uXyterIdx ]->Draw( "colz" );

      cStsSumm->cd(3);
      gPad->SetLogz();
      fhHodoChanRawTs[ uXyterIdx ]->Draw( "colz" );

      cStsSumm->cd(4);
      gPad->SetLogy();
      fhHodoFebRateEvo[ uXyterIdx ]->Draw();
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

//====================================================================//

//====================================================================//
   TCanvas* cSetupSortedDt = new TCanvas( "cSetupSortedDt",
                                    "Hodoscopes coincidence maps",
                                    w, h);
   cSetupSortedDt->Divide( 3, 3 );

   cSetupSortedDt->cd(1);
   gPad->SetLogy();
   fhSetupSortedDtX1Y1->Draw( "" );

   cSetupSortedDt->cd(2);
   gPad->SetLogy();
   fhSetupSortedDtX2Y2->Draw( "" );

   cSetupSortedDt->cd(3);
   gPad->SetLogy();
   fhSetupSortedDtN1P1->Draw( "" );

   cSetupSortedDt->cd(4);
   gPad->SetLogy();
   fhBothHodoSortedDtX1Y1->Draw( "" );

   cSetupSortedDt->cd(5);
   gPad->SetLogy();
   fhBothHodoSortedDtX2Y2->Draw( "" );
/*
   cSetupSortedDt->cd(6);
   gPad->SetLogy();
   fhSetupSortedDtN1P1->Draw( "" );
*/
   cSetupSortedDt->cd(7);
   gPad->SetLogy();
   fhSystSortedDtX1Y1->Draw( "" );

   cSetupSortedDt->cd(8);
   gPad->SetLogy();
   fhSystSortedDtX2Y2->Draw( "" );

   cSetupSortedDt->cd(9);
   gPad->SetLogy();
   fhSystSortedDtN1P1->Draw( "" );
//====================================================================//

//====================================================================//
   TCanvas* cSetupSortedMaps = new TCanvas( "cSetupSortedMaps",
                                    "Hodoscopes coincidence maps",
                                    w, h);
   cSetupSortedMaps->Divide( 3, 3 );

   cSetupSortedMaps->cd(1);
   gPad->SetLogz();
   fhSetupSortedMapX1Y1->Draw( "colz" );

   cSetupSortedMaps->cd(2);
   gPad->SetLogz();
   fhSetupSortedMapX2Y2->Draw( "colz" );

   cSetupSortedMaps->cd(3);
   gPad->SetLogz();
   fhSetupSortedMapN1P1->Draw( "colz" );

   cSetupSortedMaps->cd(4);
   gPad->SetLogz();
   fhBothHodoSortedMapX1Y1->Draw( "colz" );

   cSetupSortedMaps->cd(5);
   gPad->SetLogz();
   fhBothHodoSortedMapX2Y2->Draw( "colz" );
/*
   cSetupSortedMaps->cd(6);
   gPad->SetLogz();
   fhBothHodoSortedMapN1P1->Draw( "colz" );
*/
   cSetupSortedMaps->cd(7);
   gPad->SetLogz();
   fhSystSortedMapX1Y1->Draw( "colz" );

   cSetupSortedMaps->cd(8);
   gPad->SetLogz();
   fhSystSortedMapX2Y2->Draw( "colz" );

   cSetupSortedMaps->cd(9);
   gPad->SetLogz();
   fhSystSortedMapN1P1->Draw( "colz" );
//====================================================================//

//====================================================================//
   TCanvas* cSetupSortedCoincEvo = new TCanvas( "cSetupSortedCoincEvo",
                                    "Hodoscopes coincidence rate evolution",
                                    w, h);
   cSetupSortedCoincEvo->Divide( 3, 3 );

   cSetupSortedCoincEvo->cd(1);
   gPad->SetGridy();
   gPad->SetLogy();
   fhSetupSortedCntEvoX1Y1->Draw( "" );

   cSetupSortedCoincEvo->cd(2);
   gPad->SetGridy();
   gPad->SetLogy();
   fhSetupSortedCntEvoX2Y2->Draw( "" );

   cSetupSortedCoincEvo->cd(3);
   gPad->SetGridy();
   gPad->SetLogy();
   fhSetupSortedCntEvoN1P1->Draw( "" );

   cSetupSortedCoincEvo->cd(4);
   gPad->SetGridy();
   gPad->SetLogy();
   fhBothHodoSortedCntEvoX1Y1->Draw( "" );

   cSetupSortedCoincEvo->cd(5);
   gPad->SetGridy();
   gPad->SetLogy();
   fhBothHodoSortedCntEvoX2Y2->Draw( "" );
/*
   cSetupSortedCoincEvo->cd(6);
   gPad->SetGridy();
   gPad->SetLogy();
   fhSetupSortedCntEvoN1P1->Draw( "" );
*/
   cSetupSortedCoincEvo->cd(7);
   gPad->SetGridy();
   gPad->SetLogy();
   fhSystSortedCntEvoX1Y1->Draw( "" );

   cSetupSortedCoincEvo->cd(8);
   gPad->SetGridy();
   gPad->SetLogy();
   fhSystSortedCntEvoX2Y2->Draw( "" );

   cSetupSortedCoincEvo->cd(9);
   gPad->SetGridy();
   gPad->SetLogy();
   fhSystSortedCntEvoN1P1->Draw( "" );
//====================================================================//

      // Long duration rate monitoring
/*
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
*/
/*
  Int_t iNbPadsPerDpb = fuNbElinksPerDpb/2 + fuNbElinksPerDpb%2;
  TCanvas* cMuchChCounts = new TCanvas("cMuchChCounts", "MUCH Channels counts", w, h);
  cMuchChCounts->Divide( fNrOfNdpbs/2 + fNrOfNdpbs%2, fNrOfFebsPerNdpb );

  TCanvas* cMuchFebRate = new TCanvas("cMuchFebRate", "MUCH FEB rate", w, h);
  cMuchFebRate->Divide( fNrOfNdpbs/2 + fNrOfNdpbs%2, fNrOfFebsPerNdpb );

   TH1* histPnt = NULL;
   for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)
   {// looping on all the nDPBS IDs
      if( dpbId < fUnpackParHodo->GetNrOfnDpbsModA() )
      {
         sNdpbTag = Form("%04X", fUnpackParHodo->GetNdpbIdA(dpbId) );
      } // if( dpbId < fUnpackParHodo->GetNrOfnDpbsModA() )
         else
         {
            sNdpbTag = Form("%04X", fUnpackParHodo->GetNdpbIdB(dpbId - fNrOfNdpbsA) );
         } // else of if( dpbId < fUnpackParHodo->GetNrOfnDpbsModA() )

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

Bool_t CbmCosy2018MonitorSetup::DoUnpack(const fles::Timeslice& ts, size_t component)
{
#ifdef USE_HTTP_SERVER
  THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
#endif

   if( bCosy2018ResetSetupHistos_H )
   {
      ResetAllHistos();
      bCosy2018ResetSetupHistos_H = kFALSE;
   } // if( bCosy2018ResetSetupHistos_H )
   if( bCosy2018WriteSetupHistos_H )
   {
      SaveAllHistos( "data/HodoHistos.root" );
      bCosy2018WriteSetupHistos_H = kFALSE;
   } // if( bCosy2018WriteSetupHistos_H )

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

//   Int_t messageType = -111;
   // Loop over microslices
   size_t numCompMsInTs = ts.num_microslices(component);
/*
   if( fulCurrentTsIdx + 1 != ts.index() && 0 == component)
      LOG(INFO) << "Missed TS from  " << fulCurrentTsIdx
                << " to " << ts.index()
                << FairLogger::endl;
*/
   fulCurrentTsIdx = ts.index();

   if( fuMaxNbMicroslices < numCompMsInTs )
   {
      fuMaxNbMicroslices = numCompMsInTs;

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
      LOG(INFO) << "CbmCosy2018MonitorSetup::DoUnpack => Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                   << FairLogger::endl;
      LOG(INFO) << "CbmCosy2018MonitorSetup::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fuNbChanPerAsic
                   << FairLogger::endl;
      LOG(INFO) << "CbmCosy2018MonitorSetup::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuMaxNbMicroslices
                   << FairLogger::endl;
   } // if( fuMaxNbMicroslices < numCompMsInTs )

   for( size_t m = 0; m < numCompMsInTs; ++m )
   {
      // Ignore overlap ms if number defined by user
      if( numCompMsInTs - fuOverlapMsNb <= m )
         continue;

      auto msDescriptor = ts.descriptor(component, m);
      fuCurrentEquipmentId = msDescriptor.eq_id;
      const uint8_t* msContent = reinterpret_cast<const uint8_t*>(ts.content(component, m));

      uint32_t size = msDescriptor.size;
      fulCurrentMsIdx = msDescriptor.idx;
      Double_t dMsTime = (1e-9) * static_cast<double>(fulCurrentMsIdx);
      LOG(DEBUG) << "Microslice: " << fulCurrentMsIdx
                 << " from EqId " << std::hex << fuCurrentEquipmentId << std::dec
                 << " has size: " << size << FairLogger::endl;

      fuCurrDpbId  = static_cast< uint32_t >( fuCurrentEquipmentId & 0xFFFF );
      fuCurrDpbIdx = fDpbIdIndexMap[ fuCurrDpbId ];

      if( component < kiMaxNbFlibLinks )
      {
         if( fdStartTimeMsSz < 0 )
            fdStartTimeMsSz = dMsTime;
         fhMsSz[ component ]->Fill( size );
         fhMsSzTime[ component ]->Fill( dMsTime - fdStartTimeMsSz, size);
      } // if( component < kiMaxNbFlibLinks )

      // Store MS time for coincidence plots
      fvdMsTime[ m ] = dMsTime;

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

         stsxyter::Message mess( static_cast< uint32_t >( ulData & 0xFFFFFFFF ) );

         // Print message if requested
         if( fbPrintMessages )
            mess.PrintMess( std::cout, fPrintMessCtrl );

         stsxyter::MessType typeMess = mess.GetMessType();
         fmMsgCounter[ typeMess ] ++;
         fhHodoMessType->Fill( static_cast< uint16_t > (typeMess) );
         fhHodoMessTypePerDpb->Fill( fuCurrDpbIdx, static_cast< uint16_t > (typeMess) );

         switch( typeMess )
         {
            case stsxyter::MessType::Hit :
            {
               // Extract the eLink and Asic indices => Should GO IN the fill method now that obly hits are link/asic specific!
               UShort_t usElinkIdx = mess.GetLinkIndex();
               if( fuNbElinksPerDpb <= usElinkIdx )
               {
                  LOG(FATAL) << "CbmCosy2018MonitorSetup::DoUnpack => "
                             << "eLink index out of bounds!"
                             << usElinkIdx << " VS " << fuNbElinksPerDpb
                             << FairLogger::endl;
               } // if( fuNbElinksPerDpb <= usElinkIdx )
               fhHodoMessTypePerElink->Fill( fuCurrDpbIdx * fuNbElinksPerDpb + usElinkIdx,
                                            static_cast< uint16_t > (typeMess) );

               UInt_t   uAsicIdx   = fvuElinkToAsic[fuCurrDpbIdx][usElinkIdx];

               FillHitInfo( mess, usElinkIdx, uAsicIdx, m );
               break;
            } // case stsxyter::MessType::Hit :
            case stsxyter::MessType::TsMsb :
            {
               FillTsMsbInfo( mess, uIdx, m );
               break;
            } // case stsxyter::MessType::TsMsb :
            case stsxyter::MessType::Epoch :
            {
               // The first message in the TS is a special ones: EPOCH
               FillEpochInfo( mess );

               if( 0 < uIdx )
                  LOG(INFO) << "CbmCosy2018MonitorSetup::DoUnpack => "
                            << "EPOCH message at unexpected position in MS: message "
                            << uIdx << " VS message 0 expected!"
                            << FairLogger::endl;
               break;
            } // case stsxyter::MessType::TsMsb :
            case stsxyter::MessType::Empty :
            {
//               FillTsMsbInfo( mess );
               break;
            } // case stsxyter::MessType::Empty :
            case stsxyter::MessType::Dummy :
            {
               break;
            } // case stsxyter::MessType::Dummy / ReadDataAck / Ack :
            default:
            {
               LOG(FATAL) << "CbmCosy2018MonitorSetup::DoUnpack => "
                          << "Unknown message type, should never happen, stopping here!"
                          << FairLogger::endl;
            }
         } // switch( mess.GetMessType() )
      } // for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )

      // Sort the buffer of hits
      std::sort( fvmHitsInTs.begin(), fvmHitsInTs.end() );

      // Time differences plotting using the fully time sorted hits
      Double_t dLastTimeX1 = -1e12;
      Double_t dLastTimeY1 = -1e12;
      Double_t dLastTimeX2 = -1e12;
      Double_t dLastTimeY2 = -1e12;
      Double_t dLastTimeN1 = -1e12;
      Double_t dLastTimeP1 = -1e12;
      if( 0 < fvmHitsInTs.size() )
      {
         ULong64_t ulLastHitTime = ( *( fvmHitsInTs.rbegin() ) ).GetTs();
         std::vector< stsxyter::FinalHit >::iterator it;

         std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();
         Double_t dUnixTimeInRun = std::chrono::duration_cast< std::chrono::seconds >(tNow - ftStartTimeUnix).count();

         for( it  = fvmHitsInTs.begin();
              it != fvmHitsInTs.end() && (*it).GetTs() < ulLastHitTime - 320; // 32 * 3.125 ns = 1000 ns
              ++it )
         {
            UShort_t usAsicIdx = (*it).GetAsic();
            UShort_t usChanIdx = (*it).GetChan();
            ULong64_t ulHitTs  = (*it).GetTs();

            Bool_t bHitInX = usChanIdx < fuNbChanPerAsic/2;
            UInt_t uFiberIdx = fUnpackParHodo->GetChannelToFiberMap( usChanIdx  );

            Double_t dTimeSinceStartSec = (ulHitTs * stsxyter::kdClockCycleNs - fdStartTime)* 1e-9;

            if( fUnpackParHodo->GetAsicIndexHodo1() == usAsicIdx )
            {
               if( fUnpackParHodo->IsXySwappedHodo1() )
                  bHitInX = !bHitInX;

               if( bHitInX )
               {
                  if( fUnpackParHodo->IsXInvertedHodo1() )
                     uFiberIdx = fuNbChanPerAsic/2 - 1 - uFiberIdx;

                  fLastSortedHit1X = (*it);
                  dLastTimeX1      = ulHitTs;

                  Double_t dDtX1Y1     = ( dLastTimeY1 - dLastTimeX1 ) * stsxyter::kdClockCycleNs;
                  Double_t dDtX1Y1X2Y2 = ( ( dLastTimeX2 + dLastTimeY2 ) - ( dLastTimeX1 + dLastTimeY1 ) )
                                         * stsxyter::kdClockCycleNs / 2.0;
                  Double_t dDtX1Y1X2Y2N1P1 = (  ( dLastTimeN1 + dLastTimeP1 ) / 2.0
                                              - ( dLastTimeX2 + dLastTimeY2 + dLastTimeX1 + dLastTimeY1 ) / 4.0
                                             ) * stsxyter::kdClockCycleNs;

                  fhSetupSortedDtX1Y1->Fill(         dDtX1Y1 );
                  fhSetupSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );
                  fhSetupSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                  if( TMath::Abs( dDtX1Y1 ) < fdCoincBorderHodo )
                  {
                     UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit1Y.GetChan() );
                     if( fUnpackParHodo->IsYInvertedHodo1() )
                        uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                     fhSetupSortedMapX1Y1->Fill( uFiberIdx, uFiberIdxOther );
                     fhSetupSortedCntEvoX1Y1->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtX1Y1 ) < fdCoincBorderHodo )

                  if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )
                  {
                     fhBothHodoSortedDtX1Y1->Fill(         dDtX1Y1 );
                     fhBothHodoSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                     UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit1Y.GetChan() );
                     if( fUnpackParHodo->IsYInvertedHodo1() )
                        uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                     fhBothHodoSortedMapX1Y1->Fill( uFiberIdx, uFiberIdxOther );
                     fhBothHodoSortedCntEvoX1Y1->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )

                  if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
                  {
                     fhSystSortedDtX1Y1->Fill(         dDtX1Y1 );
                     fhSystSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );

                     UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit1Y.GetChan() );
                     if( fUnpackParHodo->IsYInvertedHodo1() )
                        uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                     fhSystSortedMapX1Y1->Fill( uFiberIdx, uFiberIdxOther );
                     fhSystSortedCntEvoX1Y1->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
               } // if( bHitInX )
                  else
                  {
                     if( fUnpackParHodo->IsYInvertedHodo1() )
                        uFiberIdx = fuNbChanPerAsic/2 - 1 - uFiberIdx;

                     fLastSortedHit1Y = (*it);
                     dLastTimeY1      = ulHitTs;

                     Double_t dDtX1Y1     = ( dLastTimeY1 - dLastTimeX1 ) * stsxyter::kdClockCycleNs;
                     Double_t dDtX1Y1X2Y2 = ( ( dLastTimeX2 + dLastTimeY2 ) - ( dLastTimeX1 + dLastTimeY1 ) )
                                            * stsxyter::kdClockCycleNs / 2.0;
                     Double_t dDtX1Y1X2Y2N1P1 = (  ( dLastTimeN1 + dLastTimeP1 ) / 2.0
                                                 - ( dLastTimeX2 + dLastTimeY2 + dLastTimeX1 + dLastTimeY1 ) / 4.0
                                                ) * stsxyter::kdClockCycleNs;

                     fhSetupSortedDtX1Y1->Fill(         dDtX1Y1 );
                     fhSetupSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );
                     fhSetupSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                     if( TMath::Abs( dDtX1Y1 ) < fdCoincBorderHodo )
                     {
                        UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit1X.GetChan() );
                        if( fUnpackParHodo->IsXInvertedHodo1() )
                           uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                        fhSetupSortedMapX1Y1->Fill( uFiberIdxOther, uFiberIdx );
                        fhSetupSortedCntEvoX1Y1->Fill( dTimeSinceStartSec );
                     } // if( TMath::Abs( dDtX1Y1 ) < fdCoincBorderHodo )

                     if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )
                     {
                        fhBothHodoSortedDtX1Y1->Fill(         dDtX1Y1 );
                        fhBothHodoSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                        UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit1X.GetChan() );
                        if( fUnpackParHodo->IsXInvertedHodo1() )
                           uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                        fhBothHodoSortedMapX1Y1->Fill( uFiberIdxOther, uFiberIdx );
                        fhBothHodoSortedCntEvoX1Y1->Fill( dTimeSinceStartSec );
                     } // if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )

                     if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
                     {
                        fhSystSortedDtX1Y1->Fill(         dDtX1Y1 );
                        fhSystSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );

                        UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit1X.GetChan() );
                        if( fUnpackParHodo->IsXInvertedHodo1() )
                           uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                        fhSystSortedMapX1Y1->Fill( uFiberIdxOther, uFiberIdx );
                        fhSystSortedCntEvoX1Y1->Fill( dTimeSinceStartSec );
                     } // if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
                  } // else of if( bHitInX )
            } // if( fUnpackParHodo->GetAsicIndexHodo1() == usAsicIdx )
            else if( fUnpackParHodo->GetAsicIndexHodo2() == usAsicIdx )
            {
               if( fUnpackParHodo->IsXySwappedHodo2() )
                  bHitInX = !bHitInX;

               if( bHitInX )
               {
                  if( fUnpackParHodo->IsXInvertedHodo2() )
                     uFiberIdx = fuNbChanPerAsic/2 - 1 - uFiberIdx;

                  fLastSortedHit2X = (*it);
                  dLastTimeX2      = ulHitTs;

                  Double_t dDtX2Y2 = ( dLastTimeY2 - dLastTimeX2 ) * stsxyter::kdClockCycleNs;
                  Double_t dDtX1Y1X2Y2 = ( ( dLastTimeX2 + dLastTimeY2 ) - ( dLastTimeX1 + dLastTimeY1 ) )
                                         * stsxyter::kdClockCycleNs / 2.0;
                  Double_t dDtX1Y1X2Y2N1P1 = (  ( dLastTimeN1 + dLastTimeP1 ) / 2.0
                                              - ( dLastTimeX2 + dLastTimeY2 + dLastTimeX1 + dLastTimeY1 ) / 4.0
                                             ) * stsxyter::kdClockCycleNs;

                  fhSetupSortedDtX2Y2->Fill( dDtX2Y2 );
                  fhSetupSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );
                  fhSetupSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                  if( TMath::Abs( dDtX2Y2 ) < fdCoincBorderHodo )
                  {
                     UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit2Y.GetChan() );
                     if( fUnpackParHodo->IsYInvertedHodo2() )
                        uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                     fhSetupSortedMapX2Y2->Fill( uFiberIdx, uFiberIdxOther );
                     fhSetupSortedCntEvoX2Y2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtX2Y2 ) < fdCoincBorderHodo )

                  if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )
                  {
                     fhBothHodoSortedDtX2Y2->Fill(         dDtX2Y2 );
                     fhBothHodoSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                     UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit2Y.GetChan() );
                     if( fUnpackParHodo->IsYInvertedHodo2() )
                        uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                     fhBothHodoSortedMapX2Y2->Fill( uFiberIdx, uFiberIdxOther );
                     fhBothHodoSortedCntEvoX2Y2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )

                  if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
                  {
                     fhSystSortedDtX2Y2->Fill(         dDtX2Y2 );
                     fhSystSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );

                     UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit2Y.GetChan() );
                     if( fUnpackParHodo->IsYInvertedHodo2() )
                        uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                     fhSystSortedMapX2Y2->Fill( uFiberIdx, uFiberIdxOther );
                     fhSystSortedCntEvoX2Y2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
               } // if( bHitInX )
                  else
                  {
                     if( fUnpackParHodo->IsYInvertedHodo2() )
                        uFiberIdx = fuNbChanPerAsic/2 - 1 - uFiberIdx;

                     fLastSortedHit2Y = (*it);
                     dLastTimeY2      = ulHitTs;

                     Double_t dDtX2Y2 = ( dLastTimeY2 - dLastTimeX2 ) * stsxyter::kdClockCycleNs;
                     Double_t dDtX1Y1X2Y2 = ( ( dLastTimeX2 + dLastTimeY2 ) - ( dLastTimeX1 + dLastTimeY1 ) )
                                            * stsxyter::kdClockCycleNs / 2.0;
                     Double_t dDtX1Y1X2Y2N1P1 = (  ( dLastTimeN1 + dLastTimeP1 ) / 2.0
                                                 - ( dLastTimeX2 + dLastTimeY2 + dLastTimeX1 + dLastTimeY1 ) / 4.0
                                                ) * stsxyter::kdClockCycleNs;

                     fhSetupSortedDtX2Y2->Fill( dDtX2Y2 );
                     fhSetupSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );
                     fhSetupSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                     if( TMath::Abs( dDtX2Y2 ) < fdCoincBorderHodo )
                     {
                        UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit2X.GetChan() );
                        if( fUnpackParHodo->IsXInvertedHodo2() )
                           uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                        fhSetupSortedMapX2Y2->Fill( uFiberIdxOther, uFiberIdx );
                        fhSetupSortedCntEvoX2Y2->Fill( dTimeSinceStartSec );
                     } // if( TMath::Abs( dDtX2Y2 ) < fdCoincBorderHodo )

                     if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )
                     {
                        fhBothHodoSortedDtX2Y2->Fill(         dDtX2Y2 );
                        fhBothHodoSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                        UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit2X.GetChan() );
                        if( fUnpackParHodo->IsXInvertedHodo2() )
                           uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                        fhBothHodoSortedMapX2Y2->Fill( uFiberIdxOther, uFiberIdx );
                        fhBothHodoSortedCntEvoX2Y2->Fill( dTimeSinceStartSec );
                     } // if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )

                     if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
                     {
                        fhSystSortedDtX2Y2->Fill(         dDtX2Y2 );
                        fhSystSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );

                        UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit2X.GetChan() );
                        if( fUnpackParHodo->IsXInvertedHodo2() )
                           uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                        fhSystSortedMapX2Y2->Fill( uFiberIdxOther, uFiberIdx );
                        fhSystSortedCntEvoX2Y2->Fill( dTimeSinceStartSec );
                     } // if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
                  } // else of if( bHitInX )
            } // else if( fUnpackParHodo->GetAsicIndexHodo2() == usAsicIdx )
            else if( fUnpackParSts->GetAsicIndexSts1N() == usAsicIdx )
            {
               fLastSortedHit1N = (*it);
               dLastTimeN1      = ulHitTs;

               // FIXME: Hack due to single side readout of noisy sensor
               fLastSortedHit1P = (*it);
               dLastTimeP1      = dLastTimeN1;

               Double_t dDtN1P1 = ( dLastTimeP1 - dLastTimeN1 ) * stsxyter::kdClockCycleNs;
               Double_t dDtX1Y1X2Y2N1P1 = (  ( dLastTimeN1 + dLastTimeP1 ) / 2.0
                                           - ( dLastTimeX2 + dLastTimeY2 + dLastTimeX1 + dLastTimeY1 ) / 4.0
                                          ) * stsxyter::kdClockCycleNs;

               fhSetupSortedDtN1P1->Fill( dDtN1P1 );
               fhSetupSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

               if( TMath::Abs( dDtN1P1 ) < fdCoincBorderSts )
               {
                  fhSetupSortedMapN1P1->Fill( usChanIdx, fLastSortedHit1P.GetChan() );
                  fhSetupSortedCntEvoN1P1->Fill( dTimeSinceStartSec );
               } // if( TMath::Abs( dDtN1P1 ) < fdCoincBorderSts )

               if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
               {
                  fhSystSortedMapN1P1->Fill( usChanIdx, fLastSortedHit1P.GetChan() );
                  fhSystSortedCntEvoN1P1->Fill( dTimeSinceStartSec );
               } // if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
            } // if( fUnpackParSts->GetAsicIndexSts1N() == usAsicIdx )
            else if( fUnpackParSts->GetAsicIndexSts1P() == usAsicIdx )
            {
               fLastSortedHit1P = (*it);
               dLastTimeP1      = ulHitTs;

               // FIXME: Hack due to single side readout of noisy sensor
               fLastSortedHit1N = (*it);
               dLastTimeN1      = dLastTimeP1;

               Double_t dDtN1P1 = ( dLastTimeP1 - dLastTimeN1 ) * stsxyter::kdClockCycleNs;
               Double_t dDtX1Y1X2Y2N1P1 = (  ( dLastTimeN1 + dLastTimeP1 ) / 2.0
                                           - ( dLastTimeX2 + dLastTimeY2 + dLastTimeX1 + dLastTimeY1 ) / 4.0
                                          ) * stsxyter::kdClockCycleNs;

               fhSetupSortedDtN1P1->Fill( dDtN1P1 );
               fhSetupSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

               if( TMath::Abs( dDtN1P1 ) < fdCoincBorderSts )
               {
                  fhSetupSortedMapN1P1->Fill( fLastSortedHit1N.GetChan(), usChanIdx );
                  fhSetupSortedCntEvoN1P1->Fill( dTimeSinceStartSec );
               } // if( TMath::Abs( dDtN1P1 ) < fdCoincBorderSts )

               if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
               {
                  fhSystSortedMapN1P1->Fill( fLastSortedHit1N.GetChan(), usChanIdx );
                  fhSystSortedCntEvoN1P1->Fill( dTimeSinceStartSec );
               } // if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
            } // else if( fUnpackParSts->GetAsicIndexSts1P() == usAsicIdx )
         } // loop on hits untils hits within 100 ns of last one or last one itself are reached

         // Remove all hits which were already used
         fvmHitsInTs.erase( fvmHitsInTs.begin(), it );
      } // if( 0 < fvmHitsInTs.size() )
   } // for( size_t m = 0; m < numCompMsInTs; ++m )


   // End of TS, check if stuff to do with the hits inside each MS
   // Usefull for low rate pulser tests
      // Need to do it only when last DPB is processed, as they are done one by one
   if( fuCurrDpbIdx == fuNrOfDpbs - 1 )
   {
      for( UInt_t uMsIdx = 0; uMsIdx < fuMaxNbMicroslices; ++uMsIdx )
      {
         fvdMsTime[ uMsIdx ] = 0.0;
      } // for( UInt_t uMsIdx = 0; uMsIdx < fuMaxNbMicroslices; ++uMsIdx )
/*
      // Sort the buffer of hits
      std::sort( fvmHitsInTs.begin(), fvmHitsInTs.end() );

      // Time differences plotting using the fully time sorted hits
      Double_t dLastTimeX1 = -1e12;
      Double_t dLastTimeY1 = -1e12;
      Double_t dLastTimeX2 = -1e12;
      Double_t dLastTimeY2 = -1e12;
      Double_t dLastTimeN1 = -1e12;
      Double_t dLastTimeP1 = -1e12;
      if( 0 < fvmHitsInTs.size() )
      {
         ULong64_t ulLastHitTime = ( *( fvmHitsInTs.rbegin() ) ).GetTs();
         std::vector< stsxyter::FinalHit >::iterator it;

         std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();
         Double_t dUnixTimeInRun = std::chrono::duration_cast< std::chrono::seconds >(tNow - ftStartTimeUnix).count();

         for( it  = fvmHitsInTs.begin();
              it != fvmHitsInTs.end() && (*it).GetTs() < ulLastHitTime - 320; // 32 * 3.125 ns = 1000 ns
              ++it )
         {
            UShort_t usAsicIdx = (*it).GetAsic();
            UShort_t usChanIdx = (*it).GetChan();
            ULong64_t ulHitTs  = (*it).GetTs();

            Bool_t bHitInX = usChanIdx < fuNbChanPerAsic/2;
            UInt_t uFiberIdx = fUnpackParHodo->GetChannelToFiberMap( usChanIdx  );

            Double_t dTimeSinceStartSec = (ulHitTs * stsxyter::kdClockCycleNs - fdStartTime)* 1e-9;

            if( fUnpackParHodo->GetAsicIndexHodo1() == usAsicIdx )
            {
               if( fUnpackParHodo->IsXySwappedHodo1() )
                  bHitInX = !bHitInX;

               if( bHitInX )
               {
                  if( fUnpackParHodo->IsXInvertedHodo1() )
                     uFiberIdx = fuNbChanPerAsic/2 - 1 - uFiberIdx;

                  fLastSortedHit1X = (*it);
                  dLastTimeX1      = ulHitTs;

                  Double_t dDtX1Y1     = ( dLastTimeY1 - dLastTimeX1 ) * stsxyter::kdClockCycleNs;
                  Double_t dDtX1Y1X2Y2 = ( ( dLastTimeX2 + dLastTimeY2 ) - ( dLastTimeX1 + dLastTimeY1 ) )
                                         * stsxyter::kdClockCycleNs / 2.0;
                  Double_t dDtX1Y1X2Y2N1P1 = (  ( dLastTimeN1 + dLastTimeP1 ) / 2.0
                                              - ( dLastTimeX2 + dLastTimeY2 + dLastTimeX1 + dLastTimeY1 ) / 4.0
                                             ) * stsxyter::kdClockCycleNs;

                  fhSetupSortedDtX1Y1->Fill(         dDtX1Y1 );
                  fhSetupSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );
                  fhSetupSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                  if( TMath::Abs( dDtX1Y1 ) < fdCoincBorderHodo )
                  {
                     UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit1Y.GetChan() );
                     if( fUnpackParHodo->IsYInvertedHodo1() )
                        uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                     fhSetupSortedMapX1Y1->Fill( uFiberIdx, uFiberIdxOther );
                     fhSetupSortedCntEvoX1Y1->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtX1Y1 ) < fdCoincBorderHodo )

                  if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )
                  {
                     fhBothHodoSortedDtX1Y1->Fill(         dDtX1Y1 );
                     fhBothHodoSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                     UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit1Y.GetChan() );
                     if( fUnpackParHodo->IsYInvertedHodo1() )
                        uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                     fhBothHodoSortedMapX1Y1->Fill( uFiberIdx, uFiberIdxOther );
                     fhBothHodoSortedCntEvoX1Y1->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )

                  if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
                  {
                     fhSystSortedDtX1Y1->Fill(         dDtX1Y1 );
                     fhSystSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );

                     UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit1Y.GetChan() );
                     if( fUnpackParHodo->IsYInvertedHodo1() )
                        uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                     fhSystSortedMapX1Y1->Fill( uFiberIdx, uFiberIdxOther );
                     fhSystSortedCntEvoX1Y1->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
               } // if( bHitInX )
                  else
                  {
                     if( fUnpackParHodo->IsYInvertedHodo1() )
                        uFiberIdx = fuNbChanPerAsic/2 - 1 - uFiberIdx;

                     fLastSortedHit1Y = (*it);
                     dLastTimeY1      = ulHitTs;

                     Double_t dDtX1Y1     = ( dLastTimeY1 - dLastTimeX1 ) * stsxyter::kdClockCycleNs;
                     Double_t dDtX1Y1X2Y2 = ( ( dLastTimeX2 + dLastTimeY2 ) - ( dLastTimeX1 + dLastTimeY1 ) )
                                            * stsxyter::kdClockCycleNs / 2.0;
                     Double_t dDtX1Y1X2Y2N1P1 = (  ( dLastTimeN1 + dLastTimeP1 ) / 2.0
                                                 - ( dLastTimeX2 + dLastTimeY2 + dLastTimeX1 + dLastTimeY1 ) / 4.0
                                                ) * stsxyter::kdClockCycleNs;

                     fhSetupSortedDtX1Y1->Fill(         dDtX1Y1 );
                     fhSetupSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );
                     fhSetupSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                     if( TMath::Abs( dDtX1Y1 ) < fdCoincBorderHodo )
                     {
                        UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit1X.GetChan() );
                        if( fUnpackParHodo->IsXInvertedHodo1() )
                           uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                        fhSetupSortedMapX1Y1->Fill( uFiberIdxOther, uFiberIdx );
                        fhSetupSortedCntEvoX1Y1->Fill( dTimeSinceStartSec );
                     } // if( TMath::Abs( dDtX1Y1 ) < fdCoincBorderHodo )

                     if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )
                     {
                        fhBothHodoSortedDtX1Y1->Fill(         dDtX1Y1 );
                        fhBothHodoSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                        UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit1X.GetChan() );
                        if( fUnpackParHodo->IsXInvertedHodo1() )
                           uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                        fhBothHodoSortedMapX1Y1->Fill( uFiberIdxOther, uFiberIdx );
                        fhBothHodoSortedCntEvoX1Y1->Fill( dTimeSinceStartSec );
                     } // if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )

                     if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
                     {
                        fhSystSortedDtX1Y1->Fill(         dDtX1Y1 );
                        fhSystSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );

                        UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit1X.GetChan() );
                        if( fUnpackParHodo->IsXInvertedHodo1() )
                           uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                        fhSystSortedMapX1Y1->Fill( uFiberIdxOther, uFiberIdx );
                        fhSystSortedCntEvoX1Y1->Fill( dTimeSinceStartSec );
                     } // if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
                  } // else of if( bHitInX )
            } // if( fUnpackParHodo->GetAsicIndexHodo1() == usAsicIdx )
            else if( fUnpackParHodo->GetAsicIndexHodo2() == usAsicIdx )
            {
               if( fUnpackParHodo->IsXySwappedHodo2() )
                  bHitInX = !bHitInX;

               if( bHitInX )
               {
                  if( fUnpackParHodo->IsXInvertedHodo2() )
                     uFiberIdx = fuNbChanPerAsic/2 - 1 - uFiberIdx;

                  fLastSortedHit2X = (*it);
                  dLastTimeX2      = ulHitTs;

                  Double_t dDtX2Y2 = ( dLastTimeY2 - dLastTimeX2 ) * stsxyter::kdClockCycleNs;
                  Double_t dDtX1Y1X2Y2 = ( ( dLastTimeX2 + dLastTimeY2 ) - ( dLastTimeX1 + dLastTimeY1 ) )
                                         * stsxyter::kdClockCycleNs / 2.0;
                  Double_t dDtX1Y1X2Y2N1P1 = (  ( dLastTimeN1 + dLastTimeP1 ) / 2.0
                                              - ( dLastTimeX2 + dLastTimeY2 + dLastTimeX1 + dLastTimeY1 ) / 4.0
                                             ) * stsxyter::kdClockCycleNs;

                  fhSetupSortedDtX2Y2->Fill( dDtX2Y2 );
                  fhSetupSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );
                  fhSetupSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                  if( TMath::Abs( dDtX2Y2 ) < fdCoincBorderHodo )
                  {
                     UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit2Y.GetChan() );
                     if( fUnpackParHodo->IsYInvertedHodo2() )
                        uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                     fhSetupSortedMapX2Y2->Fill( uFiberIdx, uFiberIdxOther );
                     fhSetupSortedCntEvoX2Y2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtX2Y2 ) < fdCoincBorderHodo )

                  if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )
                  {
                     fhBothHodoSortedDtX2Y2->Fill(         dDtX2Y2 );
                     fhBothHodoSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                     UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit2Y.GetChan() );
                     if( fUnpackParHodo->IsYInvertedHodo2() )
                        uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                     fhBothHodoSortedMapX2Y2->Fill( uFiberIdx, uFiberIdxOther );
                     fhBothHodoSortedCntEvoX2Y2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )

                  if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
                  {
                     fhSystSortedDtX2Y2->Fill(         dDtX2Y2 );
                     fhSystSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );

                     UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit2Y.GetChan() );
                     if( fUnpackParHodo->IsYInvertedHodo2() )
                        uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                     fhSystSortedMapX2Y2->Fill( uFiberIdx, uFiberIdxOther );
                     fhSystSortedCntEvoX2Y2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
               } // if( bHitInX )
                  else
                  {
                     if( fUnpackParHodo->IsYInvertedHodo2() )
                        uFiberIdx = fuNbChanPerAsic/2 - 1 - uFiberIdx;

                     fLastSortedHit2Y = (*it);
                     dLastTimeY2      = ulHitTs;

                     Double_t dDtX2Y2 = ( dLastTimeY2 - dLastTimeX2 ) * stsxyter::kdClockCycleNs;
                     Double_t dDtX1Y1X2Y2 = ( ( dLastTimeX2 + dLastTimeY2 ) - ( dLastTimeX1 + dLastTimeY1 ) )
                                            * stsxyter::kdClockCycleNs / 2.0;
                     Double_t dDtX1Y1X2Y2N1P1 = (  ( dLastTimeN1 + dLastTimeP1 ) / 2.0
                                                 - ( dLastTimeX2 + dLastTimeY2 + dLastTimeX1 + dLastTimeY1 ) / 4.0
                                                ) * stsxyter::kdClockCycleNs;

                     fhSetupSortedDtX2Y2->Fill( dDtX2Y2 );
                     fhSetupSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );
                     fhSetupSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                     if( TMath::Abs( dDtX2Y2 ) < fdCoincBorderHodo )
                     {
                        UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit2X.GetChan() );
                        if( fUnpackParHodo->IsXInvertedHodo2() )
                           uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                        fhSetupSortedMapX2Y2->Fill( uFiberIdxOther, uFiberIdx );
                        fhSetupSortedCntEvoX2Y2->Fill( dTimeSinceStartSec );
                     } // if( TMath::Abs( dDtX2Y2 ) < fdCoincBorderHodo )

                     if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )
                     {
                        fhBothHodoSortedDtX2Y2->Fill(         dDtX2Y2 );
                        fhBothHodoSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

                        UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit2X.GetChan() );
                        if( fUnpackParHodo->IsXInvertedHodo2() )
                           uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                        fhBothHodoSortedMapX2Y2->Fill( uFiberIdxOther, uFiberIdx );
                        fhBothHodoSortedCntEvoX2Y2->Fill( dTimeSinceStartSec );
                     } // if( TMath::Abs( dDtX1Y1X2Y2 ) < 2 * fdCoincBorderHodo )

                     if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
                     {
                        fhSystSortedDtX2Y2->Fill(         dDtX2Y2 );
                        fhSystSortedDtX1Y1X2Y2->Fill(     dDtX1Y1X2Y2 );

                        UInt_t uFiberIdxOther = fUnpackParHodo->GetChannelToFiberMap( fLastSortedHit2X.GetChan() );
                        if( fUnpackParHodo->IsXInvertedHodo2() )
                           uFiberIdxOther = fuNbChanPerAsic/2 - 1 - uFiberIdxOther;
                        fhSystSortedMapX2Y2->Fill( uFiberIdxOther, uFiberIdx );
                        fhSystSortedCntEvoX2Y2->Fill( dTimeSinceStartSec );
                     } // if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
                  } // else of if( bHitInX )
            } // else if( fUnpackParHodo->GetAsicIndexHodo2() == usAsicIdx )
            else if( fUnpackParSts->GetAsicIndexSts1N() == usAsicIdx )
            {
               fLastSortedHit1N = (*it);
               dLastTimeN1      = ulHitTs;

               // FIXME: Hack due to single side readout of noisy sensor
               fLastSortedHit1P = (*it);
               dLastTimeP1      = dLastTimeN1;

               Double_t dDtN1P1 = ( dLastTimeP1 - dLastTimeN1 ) * stsxyter::kdClockCycleNs;
               Double_t dDtX1Y1X2Y2N1P1 = (  ( dLastTimeN1 + dLastTimeP1 ) / 2.0
                                           - ( dLastTimeX2 + dLastTimeY2 + dLastTimeX1 + dLastTimeY1 ) / 4.0
                                          ) * stsxyter::kdClockCycleNs;

               fhSetupSortedDtN1P1->Fill( dDtN1P1 );
               fhSetupSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

               if( TMath::Abs( dDtN1P1 ) < fdCoincBorderSts )
               {
                  fhSetupSortedMapN1P1->Fill( usChanIdx, fLastSortedHit1P.GetChan() );
                  fhSetupSortedCntEvoN1P1->Fill( dTimeSinceStartSec );
               } // if( TMath::Abs( dDtN1P1 ) < fdCoincBorderSts )

               if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
               {
                  fhSystSortedMapN1P1->Fill( usChanIdx, fLastSortedHit1P.GetChan() );
                  fhSystSortedCntEvoN1P1->Fill( dTimeSinceStartSec );
               } // if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
            } // if( fUnpackParSts->GetAsicIndexSts1N() == usAsicIdx )
            else if( fUnpackParSts->GetAsicIndexSts1P() == usAsicIdx )
            {
               fLastSortedHit1P = (*it);
               dLastTimeP1      = ulHitTs;

               // FIXME: Hack due to single side readout of noisy sensor
               fLastSortedHit1N = (*it);
               dLastTimeN1      = dLastTimeP1;

               Double_t dDtN1P1 = ( dLastTimeP1 - dLastTimeN1 ) * stsxyter::kdClockCycleNs;
               Double_t dDtX1Y1X2Y2N1P1 = (  ( dLastTimeN1 + dLastTimeP1 ) / 2.0
                                           - ( dLastTimeX2 + dLastTimeY2 + dLastTimeX1 + dLastTimeY1 ) / 4.0
                                          ) * stsxyter::kdClockCycleNs;

               fhSetupSortedDtN1P1->Fill( dDtN1P1 );
               fhSetupSortedDtX1Y1X2Y2N1P1->Fill( dDtX1Y1X2Y2N1P1 );

               if( TMath::Abs( dDtN1P1 ) < fdCoincBorderSts )
               {
                  fhSetupSortedMapN1P1->Fill( fLastSortedHit1N.GetChan(), usChanIdx );
                  fhSetupSortedCntEvoN1P1->Fill( dTimeSinceStartSec );
               } // if( TMath::Abs( dDtN1P1 ) < fdCoincBorderSts )

               if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
               {
                  fhSystSortedMapN1P1->Fill( fLastSortedHit1N.GetChan(), usChanIdx );
                  fhSystSortedCntEvoN1P1->Fill( dTimeSinceStartSec );
               } // if( TMath::Abs( dDtX1Y1X2Y2N1P1 ) < fdCoincBorder )
            } // else if( fUnpackParSts->GetAsicIndexSts1P() == usAsicIdx )
         } // loop on hits untils hits within 100 ns of last one or last one itself are reached

         // Remove all hits which were already used
         fvmHitsInTs.erase( fvmHitsInTs.begin(), it );
      } // if( 0 < fvmHitsInTs.size() )
*/
/*
      // Remove all hits from this TS
      for( UInt_t uAsicIdx = 0; uAsicIdx < fuNbStsXyters; ++uAsicIdx )
         for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
            fvmChanHitsInTs[ uAsicIdx ][ uChan ].clear();
*/
   } // if( fuCurrDpbIdx == fuNrOfDpbs - 1 )

   if( 0 == ts.index() % 1000 && fuCurrDpbIdx == fuNrOfDpbs - 1 )
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
   } // if( 0 == ts.index() % 1000 && fuCurrDpbIdx == fuNrOfDpbs - 1 )

  return kTRUE;
}

void CbmCosy2018MonitorSetup::FillHitInfo( stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx )
{
   UShort_t usChan   = mess.GetHitChannel();
   UShort_t usRawAdc = mess.GetHitAdc();
//   UShort_t usFullTs = mess.GetHitTimeFull();
   UShort_t usTsOver = mess.GetHitTimeOver();
   UShort_t usRawTs  = mess.GetHitTime();

   fhHodoChanCntRaw[  uAsicIdx ]->Fill( usChan );
   fhHodoChanAdcRaw[  uAsicIdx ]->Fill( usChan, usRawAdc );
   fhHodoChanAdcRawProf[  uAsicIdx ]->Fill( usChan, usRawAdc );
   fhHodoChanRawTs[   uAsicIdx ]->Fill( usChan, usRawTs );
   fhHodoChanMissEvt[ uAsicIdx ]->Fill( usChan, mess.IsHitMissedEvts() );

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
   fvmHitsInTs.push_back( stsxyter::FinalHit( fvulChanLastHitTime[ uAsicIdx ][ usChan ], usRawAdc, uAsicIdx, usChan ) );

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
   fhHodoChanHitRateEvo[ uAsicIdx ]->Fill( dTimeSinceStartSec , usChan );
   fhHodoFebRateEvo[ uAsicIdx ]->Fill(   dTimeSinceStartSec );
   fhHodoChanHitRateEvoLong[ uAsicIdx ]->Fill( dTimeSinceStartMin, usChan, 1.0/60.0 );
   fhHodoFebRateEvoLong[ uAsicIdx ]->Fill(   dTimeSinceStartMin, 1.0/60.0 );
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

void CbmCosy2018MonitorSetup::FillTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx, UInt_t uMsIdx )
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
}

void CbmCosy2018MonitorSetup::FillEpochInfo( stsxyter::Message mess )
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

void CbmCosy2018MonitorSetup::Reset()
{
}

void CbmCosy2018MonitorSetup::Finish()
{

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   LOG(INFO) << "CbmCosy2018MonitorSetup statistics are " << FairLogger::endl;
   LOG(INFO) << " Hit      messages: " << fmMsgCounter[ stsxyter::MessType::Hit   ] << FairLogger::endl
             << " Ts MSB   messages: " << fmMsgCounter[ stsxyter::MessType::TsMsb ] << FairLogger::endl
             << " Dummy    messages: " << fmMsgCounter[ stsxyter::MessType::Dummy ] << FairLogger::endl
             << " Epoch    messages: " << fmMsgCounter[ stsxyter::MessType::Epoch ] << FairLogger::endl
             << " Empty    messages: " << fmMsgCounter[ stsxyter::MessType::Empty ] << FairLogger::endl;

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;

   SaveAllHistos();

}


void CbmCosy2018MonitorSetup::FillOutput(CbmDigi* /*digi*/)
{
}

void CbmCosy2018MonitorSetup::SaveAllHistos( TString sFileName )
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

   fhHodoMessType->Write();
   fhHodoSysMessType->Write();
   fhHodoMessTypePerDpb->Write();
   fhHodoSysMessTypePerDpb->Write();
   fhHodoMessTypePerElink->Write();
   fhHodoSysMessTypePerElink->Write();

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fhHodoChanCntRaw[ uXyterIdx ]->Write();
      fhHodoChanAdcRaw[ uXyterIdx ]->Write();
      fhHodoChanAdcRawProf[ uXyterIdx ]->Write();
      fhHodoChanRawTs[ uXyterIdx ]->Write();
      fhHodoChanMissEvt[ uXyterIdx ]->Write();
      fhHodoChanHitRateEvo[ uXyterIdx ]->Write();
      fhHodoFebRateEvo[ uXyterIdx ]->Write();
      fhHodoChanHitRateEvoLong[ uXyterIdx ]->Write();
      fhHodoFebRateEvoLong[ uXyterIdx ]->Write();
/*
      if( kTRUE == fbLongHistoEnable )
      {
         fhFebRateEvoLong[ uXyterIdx ]->Write();
         fhFebChRateEvoLong[ uXyterIdx ]->Write();
      } // if( kTRUE == fbLongHistoEnable )
*/
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

   fhSetupSortedDtX1Y1->Write();
   fhSetupSortedDtX2Y2->Write();
   fhSetupSortedDtN1P1->Write();
   fhSetupSortedMapX1Y1->Write();
   fhSetupSortedMapX2Y2->Write();
   fhSetupSortedMapN1P1->Write();
   fhSetupSortedCntEvoX1Y1->Write();
   fhSetupSortedCntEvoX2Y2->Write();
   fhSetupSortedCntEvoN1P1->Write();

   fhBothHodoSortedDtX1Y1->Write();
   fhBothHodoSortedDtX2Y2->Write();
   fhBothHodoSortedMapX1Y1->Write();
   fhBothHodoSortedMapX2Y2->Write();
   fhBothHodoSortedCntEvoX1Y1->Write();
   fhBothHodoSortedCntEvoX2Y2->Write();

   fhSystSortedDtX1Y1->Write();
   fhSystSortedDtX2Y2->Write();
   fhSystSortedDtN1P1->Write();
   fhSystSortedMapX1Y1->Write();
   fhSystSortedMapX2Y2->Write();
   fhSystSortedMapN1P1->Write();
   fhSystSortedCntEvoX1Y1->Write();
   fhSystSortedCntEvoX2Y2->Write();
   fhSystSortedCntEvoN1P1->Write();

//   fhHodoFebTsMsb->Write();

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
void CbmCosy2018MonitorSetup::ResetAllHistos()
{
   LOG(INFO) << "Reseting all STS histograms." << FairLogger::endl;

   fhHodoMessType->Reset();
   fhHodoSysMessType->Reset();
   fhHodoMessTypePerDpb->Reset();
   fhHodoSysMessTypePerDpb->Reset();
   fhHodoMessTypePerElink->Reset();
   fhHodoSysMessTypePerElink->Reset();

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fhHodoChanCntRaw[ uXyterIdx ]->Reset();
      fhHodoChanAdcRaw[ uXyterIdx ]->Reset();
      fhHodoChanAdcRawProf[ uXyterIdx ]->Reset();
      fhHodoChanRawTs[ uXyterIdx ]->Reset();
      fhHodoChanMissEvt[ uXyterIdx ]->Reset();
      fhHodoChanHitRateEvo[ uXyterIdx ]->Reset();
      fhHodoFebRateEvo[ uXyterIdx ]->Reset();
      fhHodoChanHitRateEvoLong[ uXyterIdx ]->Reset();
      fhHodoFebRateEvoLong[ uXyterIdx ]->Reset();
/*
      if( kTRUE == fbLongHistoEnable )
      {
         ftStartTimeUnix = std::chrono::steady_clock::now();
         fhFebRateEvoLong[ uXyterIdx ]->Reset();
         fhFebChRateEvoLong[ uXyterIdx ]->Reset();
      } // if( kTRUE == fbLongHistoEnable )
*/
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

   fhSetupSortedDtX1Y1->Reset();
   fhSetupSortedDtX2Y2->Reset();
   fhSetupSortedDtN1P1->Reset();
   fhSetupSortedMapX1Y1->Reset();
   fhSetupSortedMapX2Y2->Reset();
   fhSetupSortedMapN1P1->Reset();
   fhSetupSortedCntEvoX1Y1->Reset();
   fhSetupSortedCntEvoX2Y2->Reset();
   fhSetupSortedCntEvoN1P1->Reset();

   fhBothHodoSortedDtX1Y1->Reset();
   fhBothHodoSortedDtX2Y2->Reset();
   fhBothHodoSortedMapX1Y1->Reset();
   fhBothHodoSortedMapX2Y2->Reset();
   fhBothHodoSortedCntEvoX1Y1->Reset();
   fhBothHodoSortedCntEvoX2Y2->Reset();

   fhSystSortedDtX1Y1->Reset();
   fhSystSortedDtX2Y2->Reset();
   fhSystSortedDtN1P1->Reset();
   fhSystSortedMapX1Y1->Reset();
   fhSystSortedMapX2Y2->Reset();
   fhSystSortedMapN1P1->Reset();
   fhSystSortedCntEvoX1Y1->Reset();
   fhSystSortedCntEvoX2Y2->Reset();
   fhSystSortedCntEvoN1P1->Reset();

//   fhHodoFebTsMsb->Reset();

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

void CbmCosy2018MonitorSetup::SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize )
{
   TDatime * fRunStartDateTime     = new TDatime( dateIn, timeIn);
   fiRunStartDateTimeSec = fRunStartDateTime->Convert();
   fiBinSizeDatePlots    = iBinSize;

   LOG(INFO) << "Assigned new MUCH Run Start Date-Time: " << fRunStartDateTime->AsString() << FairLogger::endl;
}

void CbmCosy2018MonitorSetup::SetLongDurationLimits( UInt_t uDurationSeconds, UInt_t uBinSize )
{
   fbLongHistoEnable     = kTRUE;
   fuLongHistoNbSeconds  = uDurationSeconds;
   fuLongHistoBinSizeSec = uBinSize;
}

ClassImp(CbmCosy2018MonitorSetup)
