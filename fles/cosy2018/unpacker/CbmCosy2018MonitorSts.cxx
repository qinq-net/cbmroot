// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmCosy2018MonitorSts                         -----
// -----                Created 15/02/18  by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmCosy2018MonitorSts.h"

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

Bool_t bCosy2018ResetStsHistos = kFALSE;
Bool_t bCosy2018WriteStsHistos = kFALSE;

CbmCosy2018MonitorSts::CbmCosy2018MonitorSts() :
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
   fbDualStsEna( kFALSE ),
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
   fLastSortedHit1N(),
   fLastSortedHit1P(),
   fLastSortedHit2N(),
   fLastSortedHit2P(),
   fuMaxNbMicroslices(100),
   fHM(new CbmHistManager()),
   fhStsMessType(NULL),
   fhStsSysMessType(NULL),
   fhStsMessTypePerDpb(NULL),
   fhStsSysMessTypePerDpb(NULL),
   fhStsDpbRawTsMsb(NULL),
   fhStsDpbRawTsMsbSx(NULL),
   fhStsDpbRawTsMsbDpb(NULL),
   fhStsMessTypePerElink(NULL),
   fhStsSysMessTypePerElink(NULL),
   fhStsChanCounts(),
   fhStsChanRawAdc(),
   fhStsChanRawAdcProf(),
   fhStsChanRawTs(),
   fhStsChanMissEvt(),
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
/*
   fbPulserTimeDiffOn(kFALSE),
   fuPulserMaxNbMicroslices(100),
   fvuPulserAsic(),
   fvuPulserChan(),
   fhStsPulserChansTimeDiff(),
   fhStsPulserChansTimeDiffEvo(),
   fhStsPulserChansTimeDiffAdc(),
*/
   fhStsAsicTsMsb(NULL),
   fbLongHistoEnable( kFALSE ),
   fuLongHistoNbSeconds( 0 ),
   fuLongHistoBinSizeSec( 0 ),
   fuLongHistoBinNb( 0 ),
   fhFebRateEvoLong(),
   fhFebChRateEvoLong(),
   fhStsSameMs1NP(NULL),
   fhStsSameMs2NP(NULL),
   fhStsSameMsN1N2(NULL),
   fhStsSameMsP1P2(NULL),
   fhStsSameMsN1P2(NULL),
   fhStsSameMsP1N2(NULL),
   fhStsSameMsCntEvoN1P1(NULL),
   fhStsSameMsCntEvoN2P2(NULL),
   fhStsSameMsCntEvoN1N2(NULL),
   fhStsSameMsCntEvoP1P2(NULL),
   fhStsSameMsCntEvoN1P2(NULL),
   fhStsSameMsCntEvoP1N2(NULL),
   fhStsSameMsCntEvoN1P1N2P2(NULL),
   fhStsSortedDtN1P1(NULL),
   fhStsSortedDtN2P2(NULL),
   fhStsSortedDtN1N2(NULL),
   fhStsSortedDtP1P2(NULL),
   fhStsSortedDtN1P2(NULL),
   fhStsSortedDtP1N2(NULL),
   fhStsSortedMapN1P1(NULL),
   fhStsSortedMapN2P2(NULL),
   fhStsSortedMapN1N2(NULL),
   fhStsSortedMapP1P2(NULL),
   fhStsSortedMapN1P2(NULL),
   fhStsSortedMapP1N2(NULL),
   fhStsSortedCntEvoN1P1(NULL),
   fhStsSortedCntEvoN2P2(NULL),
   fhStsSortedCntEvoN1N2(NULL),
   fhStsSortedCntEvoP1P2(NULL),
   fhStsSortedCntEvoN1P2(NULL),
   fhStsSortedCntEvoP1N2(NULL),
   fcMsSizeAll(NULL)
{
}

CbmCosy2018MonitorSts::~CbmCosy2018MonitorSts()
{
}

Bool_t CbmCosy2018MonitorSts::Init()
{
  LOG(INFO) << "Initializing flib StsXyter unpacker for STS" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  return kTRUE;
}

void CbmCosy2018MonitorSts::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
         << FairLogger::endl;
   fUnpackPar = (CbmCern2017UnpackParSts*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmCern2017UnpackParSts"));
}


Bool_t CbmCosy2018MonitorSts::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
         << FairLogger::endl;

   Bool_t bReInit = ReInitContainers();
   CreateHistograms();

   return bReInit;
}

Bool_t CbmCosy2018MonitorSts::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
         << FairLogger::endl;

   LOG(INFO) << "Nr. of Overlap MS:     " << fuOverlapMsNb
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

   LOG(INFO) << "Counter size for TS:     " << stsxyter::kuHitNbTsBins
             << FairLogger::endl;
   LOG(INFO) << "Counter size for TS_MSB: " << stsxyter::kuTsMsbNbTsBins
             << FairLogger::endl;
   LOG(INFO) << "Counter size for cycles: " << stsxyter::kuTsCycleNbBins
             << FairLogger::endl;

   LOG(INFO) << "ASIC Idx for STS 1 N: " << fUnpackPar->GetAsicIndexSts1N()
             << FairLogger::endl;
   LOG(INFO) << "ASIC Idx for STS 1 P: " << fUnpackPar->GetAsicIndexSts1P()
             << FairLogger::endl;
   LOG(INFO) << "ASIC Idx for STS 2 N: " << fUnpackPar->GetAsicIndexSts2N()
             << FairLogger::endl;
   LOG(INFO) << "ASIC Idx for STS 2 P: " << fUnpackPar->GetAsicIndexSts2P()
             << FairLogger::endl;

   // Internal status initialization
   fvulCurrentTsMsb.resize( fuNrOfDpbs );
   fvuCurrentTsMsbCycle.resize( fuNrOfDpbs );
   fvuElinkLastTsHit.resize( fuNrOfDpbs );
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fvulCurrentTsMsb[uDpb]     = 0;
      fvuCurrentTsMsbCycle[uDpb] = 0;
      fvuElinkLastTsHit[uDpb]    = 0;
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   fvulChanLastHitTime.resize( fuNbStsXyters );
   fvdChanLastHitTime.resize( fuNbStsXyters );
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
   LOG(INFO) << "CbmCosy2018MonitorSts::ReInitContainers => Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                << FairLogger::endl;
   LOG(INFO) << "CbmCosy2018MonitorSts::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fuNbChanPerAsic
                << FairLogger::endl;
   LOG(INFO) << "CbmCosy2018MonitorSts::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuMaxNbMicroslices
                << FairLogger::endl;
   return kTRUE;
}

void CbmCosy2018MonitorSts::CreateHistograms()
{

   TString sHistName{""};
   TString title{""};

   sHistName = "hStsMessageType";
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
   fHM->Add(sHistName.Data(), fhStsMessType);

   sHistName = "hStsSysMessType";
   title = "Nb of system message for each type; System Type";
   fhStsSysMessType = new TH1I(sHistName, title, 17, 0., 17.);
/*
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   hSysMessType->GetXaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/
   fHM->Add(sHistName.Data(), fhStsSysMessType);

   sHistName = "hStsMessageTypePerDpb";
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
   fHM->Add(sHistName.Data(), fhStsMessTypePerDpb);

   sHistName = "hStsSysMessTypePerDpb";
   title = "Nb of system message of each type for each DPB; DPB; System Type";
   fhStsSysMessTypePerDpb = new TH2I(sHistName, title, fuNrOfDpbs, 0, fuNrOfDpbs, 17, 0., 17.);
/*
   hSysMessType->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   hSysMessType->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   hSysMessType->GetYaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/
   fHM->Add(sHistName.Data(), fhStsSysMessTypePerDpb);

   sHistName = "hStsDpbRawTsMsb";
   title = "Raw TsMsb distribution for each DPB; DPB; TS MSB [Bins]";
   fhStsDpbRawTsMsb = new TH2I( sHistName, title,
                                fuNrOfDpbs, 0, fuNrOfDpbs,
                                stsxyter::kuTsMsbNbTsBins, 0., stsxyter::kuTsMsbNbTsBins);

   sHistName = "hStsDpbRawTsMsbSx";
   title = "Raw TsMsb distribution for each DPB; DPB; TS MSB [Bins]";
   fhStsDpbRawTsMsbSx = new TH2I( sHistName, title,
                                  fuNrOfDpbs, 0, fuNrOfDpbs,
                                  0x1F , 0., 0x1F );

   sHistName = "hStsDpbRawTsMsbDpb";
   title = "Raw TsMsb distribution for each DPB; DPB; TS MSB [Bins]";
   fhStsDpbRawTsMsbDpb = new TH2I( sHistName, title,
                                   fuNrOfDpbs, 0, fuNrOfDpbs,
                                   stsxyter::kuTsMsbNbTsBins >> 5, 0., stsxyter::kuTsMsbNbTsBins >> 5 );

   sHistName = "hStsMessageTypePerElink";
   title = "Nb of message of each type for each eLink; eLink; Type";
   fhStsMessTypePerElink = new TH2I(sHistName, title, fuNrOfDpbs * fuNbElinksPerDpb, 0, fuNrOfDpbs * fuNbElinksPerDpb, 5, 0., 5.);
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel( 1,       "Dummy");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel( 2,         "Hit");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel( 3,       "TsMsb");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel( 4, "ReadDataAck");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel( 5,         "Ack");
/* *** Missing int + MessType OP!!!! ****
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Dummy,       "Dummy");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Hit,         "Hit");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::TsMsb,       "TsMsb");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::ReadDataAck, "ReadDataAck");
   fhStsMessTypePerElink->GetYaxis()->SetBinLabel(1 + stsxyter::MessType::Ack,         "Ack");
*/
   fHM->Add(sHistName.Data(), fhStsMessTypePerElink);

   sHistName = "hStsSysMessTypePerElink";
   title = "Nb of system message of each type for each eLink; eLink; System Type";
   fhStsSysMessTypePerElink = new TH2I(sHistName, title, fuNrOfDpbs * fuNbElinksPerDpb, 0, fuNrOfDpbs * fuNbElinksPerDpb, 17, 0., 17.);
/*
   fhStsSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   fhStsSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   fhStsSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/
   fHM->Add(sHistName.Data(), fhStsSysMessTypePerElink);

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

      // Raw Adc Distribution
      sHistName = Form( "hStsChanRawAdc_%03u", uXyterIdx );
      title = Form( "Raw Adc distribution per channel, StsXyter #%03u; Channel []; Adc []; Hits []", uXyterIdx );
      fhStsChanRawAdc.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 stsxyter::kuHitNbAdcBins, -0.5, stsxyter::kuHitNbAdcBins -0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanRawAdc[ uXyterIdx ] );

      // Raw Adc Distribution profile
      sHistName = Form( "hStsChanRawAdcProfc_%03u", uXyterIdx );
      title = Form( "Raw Adc prodile per channel, StsXyter #%03u; Channel []; Adc []", uXyterIdx );
      fhStsChanRawAdcProf.push_back( new TProfile(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanRawAdcProf[ uXyterIdx ] );

      // Raw Ts Distribution
      sHistName = Form( "hStsChanRawTs_%03u", uXyterIdx );
      title = Form( "Raw Timestamp distribution per channel, StsXyter #%03u; Channel []; Ts []; Hits []", uXyterIdx );
      fhStsChanRawTs.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 stsxyter::kuHitNbTsBins, -0.5, stsxyter::kuHitNbTsBins -0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanRawTs[ uXyterIdx ] );

      // Missed event flag
      sHistName = Form( "hStsChanMissEvt_%03u", uXyterIdx );
      title = Form( "Missed Event flags per channel, StsXyter #%03u; Channel []; Miss Evt []; Hits []", uXyterIdx );
      fhStsChanMissEvt.push_back( new TH2I(sHistName, title,
                                 fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                 2, -0.5, 1.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanMissEvt[ uXyterIdx ] );

      // Hit rates evo per channel
      sHistName = Form( "hStsChanRateEvo_%03u", uXyterIdx );
      title = Form( "Hits per second & channel in StsXyter #%03u; Time [s]; Channel []; Hits []", uXyterIdx );
      fhStsChanHitRateEvo.push_back( new TH2I( sHistName, title,
                                                1800, 0, 1800,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanHitRateEvo[ uXyterIdx ] );

      // Hit rates evo per StsXyter
      sHistName = Form( "hStsXyterRateEvo_%03u", uXyterIdx );
      title = Form( "Hits per second in StsXyter #%03u; Time [s]; Hits []", uXyterIdx );
      fhStsXyterRateEvo.push_back( new TH1I(sHistName, title, 1800, 0, 1800 ) );
      fHM->Add(sHistName.Data(), fhStsXyterRateEvo[ uXyterIdx ] );

      // Hit rates evo per channel, 1 minute bins, 24h
      sHistName = Form( "hStsChanRateEvoLong_%03u", uXyterIdx );
      title = Form( "Hits per second & channel in StsXyter #%03u; Time [min]; Channel []; Hits []", uXyterIdx );
      fhStsChanHitRateEvoLong.push_back( new TH2D( sHistName, title,
                                                1440, 0, 1440,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanHitRateEvoLong[ uXyterIdx ] );

      // Hit rates evo per StsXyter, 1 minute bins, 24h
      sHistName = Form( "hStsXyterRateEvoLong_%03u", uXyterIdx );
      title = Form( "Hits per second in StsXyter #%03u; Time [min]; Hits []", uXyterIdx );
      fhStsXyterRateEvoLong.push_back( new TH1D(sHistName, title, 1440, 0, 1440 ) );
      fHM->Add(sHistName.Data(), fhStsXyterRateEvoLong[ uXyterIdx ] );

      // Hit distance in time for each channel
      if( fbChanHitDtEna )
      {
         sHistName = Form( "hStsChanHitDt_%03u", uXyterIdx );
         title = Form( "Time diff between hits on same channel in StsXyter #%03u; t_hit - t_prev [ns]; Channel []; Hits []", uXyterIdx );
         fhStsChanHitDt.push_back( new TH2I( sHistName, title,
                                                   iNbBinsRate - 1, dBinsRate,
                                                   fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
         fHM->Add(sHistName.Data(), fhStsChanHitDt[ uXyterIdx ] );
         sHistName = Form( "hStsChanHitDtNeg_%03u", uXyterIdx );
         title = Form( "Time diff between hits on same channel in StsXyter #%03u; t_prev - t_hit [ns]; Channel []; Hits []", uXyterIdx );
         fhStsChanHitDtNeg.push_back( new TH2I( sHistName, title,
                                                   iNbBinsRate - 1, dBinsRate,
                                                   fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
         fHM->Add(sHistName.Data(), fhStsChanHitDtNeg[ uXyterIdx ] );

      } // if( fbChanHitDtEna )

      sHistName = Form( "hStsChanHitsPerMs_%03u", uXyterIdx );
      title = Form( "Nb of hits per channel in each MS in StsXyter #%03u; Nb Hits in MS []; Channel []; MS []", uXyterIdx );
      fhStsChanHitsPerMs.push_back( new TH2I( sHistName, title,
                                                100, -0.5, 99.5,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanHitsPerMs[ uXyterIdx ] );

      sHistName = Form( "hStsChanSameMs_%03u", uXyterIdx );
      title = Form( "Nb of MS with hits in both channels in StsXyter #%03u; Channel A []; Channel B []; Coinc. MS []", uXyterIdx );
      fhStsChanSameMs.push_back( new TH2I( sHistName, title,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                                fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
      fHM->Add(sHistName.Data(), fhStsChanHitsPerMs[ uXyterIdx ] );

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

      if( kTRUE == fbLongHistoEnable )
      {
         UInt_t uAlignedLimit = fuLongHistoNbSeconds - (fuLongHistoNbSeconds % fuLongHistoBinSizeSec);
         fuLongHistoBinNb = uAlignedLimit / fuLongHistoBinSizeSec;

         sHistName = Form( "hFebRateEvoLong_%03u", uXyterIdx );
         title = Form( "Mean rate VS run time in same MS in StsXyter #%03u; Time in run [s]; Rate [1/s]", uXyterIdx );
         fhFebRateEvoLong.push_back( new TH1D( sHistName, title,
                                                   fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5) );
         fHM->Add(sHistName.Data(), fhFebRateEvoLong[ uXyterIdx ] );

         sHistName = Form( "hFebChRateEvoLong_%03u", uXyterIdx );
         title = Form( "Mean rate per channel VS run time in StsXyter #%03u; Time in run [s]; Channel []; Rare [1/s]", uXyterIdx );
         fhFebChRateEvoLong.push_back( new TH2D( sHistName, title,
                                                   fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5,
                                                   fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 ) );
         fHM->Add(sHistName.Data(), fhFebChRateEvoLong[ uXyterIdx ] );
      } // if( kTRUE == fbLongHistoEnable )

   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
/*
   if( kTRUE == fbPulserTimeDiffOn )
   {
      UInt_t uNbPulserChans = fvuPulserAsic.size();
      UInt_t uNbBinEvo = 32768 + 1;
      Double_t dMaxEdgeEvo = stsxyter::kdClockCycleNs
                            * static_cast< Double_t >( uNbBinEvo ) / 2.0;
      Double_t dMinEdgeEvo = dMaxEdgeEvo * -1.0;
      for( UInt_t uChA = 0; uChA < uNbPulserChans; ++uChA )
         for( UInt_t uChB = uChA + 1; uChB < uNbPulserChans; ++uChB )
         {
            sHistName = Form( "hStsPulserTimeDiff_%02u_%03u_%02u_%03u",
                              fvuPulserAsic[uChA], fvuPulserChan[uChA],
                              fvuPulserAsic[uChB], fvuPulserChan[uChB] );
            title = Form( "Time diff for hits in same MS from ASIC %02d ch %03d and ASIC %02d ch %03d; tB - tA [ns]; Counts",
                              fvuPulserAsic[uChA], fvuPulserChan[uChA],
                              fvuPulserAsic[uChB], fvuPulserChan[uChB] );
//            TH1 * pHist = new TH1I(sHistName, title, 16385, -51203.125, 51203.125);
            TH1 * pHist = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );

            fHM->Add(sHistName.Data(), pHist);
#ifdef USE_HTTP_SERVER
            if( server ) server->Register("/StsRaw", pHist );
#endif
            fhStsPulserChansTimeDiff.push_back( pHist );

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
#ifdef USE_HTTP_SERVER
               if( server ) server->Register("/StsRaw", pHistEvo );
#endif
               fhStsPulserChansTimeDiffEvo.push_back( pHistEvo );
            } // if( kTRUE == fbLongHistoEnable )

            sHistName = Form( "hStsPulserChansTimeDiffAdc_%02u_%03u_%02u_%03u",
                              fvuPulserAsic[uChA], fvuPulserChan[uChA],
                              fvuPulserAsic[uChB], fvuPulserChan[uChB] );
            title = Form( "Time diff vs Adc Diff for hits in same MS from ASIC %02d ch %03d and ASIC %02d ch %03d; tB - tA [ns];  Adc diff [a.u.]; Counts",
                              fvuPulserAsic[uChA], fvuPulserChan[uChA],
                              fvuPulserAsic[uChB], fvuPulserChan[uChB] );
            TH2 * pHistAdc = new TH2I( sHistName, title,
                                        uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo,
                                        2*stsxyter::kuHitNbAdcBins + 1, -0.5 - stsxyter::kuHitNbAdcBins, stsxyter::kuHitNbAdcBins + 0.5 );
            fhStsPulserChansTimeDiffAdc.push_back( pHistAdc );
#ifdef USE_HTTP_SERVER
            if( server ) server->Register("/StsRaw", pHistAdc );
#endif
         } // Loop on pairs of channels
   } // if( kTRUE == fbPulserTimeDiffOn )
*/
   // Coincidence map for each hodoscope
   sHistName = "hStsSameMs1NP";
   title = "MS with hits in both channels for Sts 1 axis N and P; N channel []; P channel []; MS []";
   fhStsSameMs1NP = new TH2I( sHistName, title,
                                  fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                  fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 );
   fHM->Add(sHistName.Data(), fhStsSameMs1NP );

   if( kTRUE == fbDualStsEna )
   {
      sHistName = "hStsSameMs2NP";
      title = "MS with hits in both channels for Sts 2 axis N and P; N channel []; P channel []; MS []";
      fhStsSameMs2NP = new TH2I( sHistName, title,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 );
      fHM->Add(sHistName.Data(), fhStsSameMs2NP );

      // Coincidence map between some axis of the Sts
      sHistName = "hStsSameMsN1N2";
      title = "MS with hits in both channels for Sts 1 and 2 axis N; N channel Sts 1 []; N channel Sts 2 []; MS []";
      fhStsSameMsN1N2 = new TH2I( sHistName, title,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 );
      fHM->Add(sHistName.Data(), fhStsSameMsN1N2 );
      sHistName = "fhStsSameMsP1P2";
      title = "MS with hits in both channels for Sts 1 and 2 axis P; P channel Sts 1 []; P channel Sts 2 []; MS []";
      fhStsSameMsP1P2 = new TH2I( sHistName, title,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 );
      fHM->Add(sHistName.Data(), fhStsSameMsP1P2 );
      sHistName = "hStsSameMsN1P2";
      title = "MS with hits in both channels for Sts 1 axis N and 2 axis P; N channel Sts 1 []; P channel Sts 2 []; MS []";
      fhStsSameMsN1P2 = new TH2I( sHistName, title,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 );
      fHM->Add(sHistName.Data(), fhStsSameMsN1P2 );
      sHistName = "fhStsSameMsP1N2";
      title = "MS with hits in both channels for Sts 1 axis P and 2 axis N; P channel Sts 1 []; N channel Sts 2 []; MS []";
      fhStsSameMsP1N2 = new TH2I( sHistName, title,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 );
      fHM->Add(sHistName.Data(), fhStsSameMsP1N2 );
   } // if( kTRUE == fbDualStsEna )

   // Coincidence counts evolution between some axis of the Sts
   sHistName = "hStsSameMsCntEvoN1P1";
   title = "Nb of MS with hits in both N1 and P1 per s; Time [s]; MS with both []";
   fhStsSameMsCntEvoN1P1 = new TH1I(sHistName, title, 1800, 0, 1800 );
   fHM->Add(sHistName.Data(), fhStsSameMsCntEvoN1P1 );

   if( kTRUE == fbDualStsEna )
   {
      sHistName = "hStsSameMsCntEvoN2P2";
      title = "Nb of MS with hits in both N2 and P2 per s; Time [s]; MS with both []";
      fhStsSameMsCntEvoN2P2 = new TH1I(sHistName, title, 1800, 0, 1800 );
      fHM->Add(sHistName.Data(), fhStsSameMsCntEvoN2P2 );

      sHistName = "fhStsSameMsCntEvoN1N2";
      title = "Nb of MS with hits in both N1 and N2 per s; Time [s]; MS with both []";
      fhStsSameMsCntEvoN1N2 = new TH1I(sHistName, title, 1800, 0, 1800 );
      fHM->Add(sHistName.Data(), fhStsSameMsCntEvoN1N2 );

      sHistName = "fhStsSameMsCntEvoP1P2";
      title = "Nb of MS with hits in both P1 and P2 per s; Time [s]; MS with both []";
      fhStsSameMsCntEvoP1P2 = new TH1I(sHistName, title, 1800, 0, 1800 );
      fHM->Add(sHistName.Data(), fhStsSameMsCntEvoP1P2 );

      sHistName = "hStsSameMsCntEvoN1P2";
      title = "Nb of MS with hits in both N1 and P2 per s; Time [s]; MS with both []";
      fhStsSameMsCntEvoN1P2 = new TH1I(sHistName, title, 1800, 0, 1800 );
      fHM->Add(sHistName.Data(), fhStsSameMsCntEvoN1P2 );

      sHistName = "hStsSameMsCntEvoP1N2";
      title = "Nb of MS with hits in both P1 and N2 per s; Time [s]; MS with both []";
      fhStsSameMsCntEvoP1N2 = new TH1I(sHistName, title, 1800, 0, 1800 );
      fHM->Add(sHistName.Data(), fhStsSameMsCntEvoP1N2 );

      sHistName = "hStsSameMsCntEvoN1P1N2P2";
      title = "Nb of MS with hits in both N1, P1, N2 and P2 per s; Time [s]; MS with both []";
      fhStsSameMsCntEvoN1P1N2P2 = new TH1I(sHistName, title, 1800, 0, 1800 );
      fHM->Add(sHistName.Data(), fhStsSameMsCntEvoN1P1N2P2 );
   } // if( kTRUE == fbDualStsEna )

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++///

   UInt_t uNbBinEvo = 32768 + 1;
   Double_t dMaxEdgeEvo = stsxyter::kdClockCycleNs
                         * static_cast< Double_t >( uNbBinEvo ) / 2.0;
   Double_t dMinEdgeEvo = dMaxEdgeEvo * -1.0;

   sHistName = "fhStsSortedDtN1P1";
   title =  "Time diff for hits Sts 1 N and Sts 1 P; tP1 - tN1 [ns]; Counts";
   fhStsSortedDtN1P1 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
   fHM->Add(sHistName.Data(), fhStsSortedDtN1P1);

   if( kTRUE == fbDualStsEna )
   {
      sHistName = "fhStsSortedDtN2P2";
      title =  "Time diff for hits Sts 2 N and Sts 2 P; tP2 - tN2 [ns]; Counts";
      fhStsSortedDtN2P2 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
      fHM->Add(sHistName.Data(), fhStsSortedDtN2P2);

      sHistName = "fhStsSortedDtN1N2";
      title =  "Time diff for hits Sts 1 N and Sts 2 N; tN2 - tN1 [ns]; Counts";
      fhStsSortedDtN1N2 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
      fHM->Add(sHistName.Data(), fhStsSortedDtN1N2);

      sHistName = "fhStsSortedDtP1P2";
      title =  "Time diff for hits Sts 1 P and Sts 2 P; tP2 - tP1 [ns]; Counts";
      fhStsSortedDtP1P2 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
      fHM->Add(sHistName.Data(), fhStsSortedDtP1P2);

      sHistName = "fhStsSortedDtN1P2";
      title =  "Time diff for hits Sts 1 N and Sts 2 P; tP2 - tN1 [ns]; Counts";
      fhStsSortedDtN1P2 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
      fHM->Add(sHistName.Data(), fhStsSortedDtN1P2);

      sHistName = "fhStsSortedDtP1N2";
      title =  "Time diff for hits Sts 1 P and Sts 2 N; tP2 - tP1 [ns]; Counts";
      fhStsSortedDtP1N2 = new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo );
      fHM->Add(sHistName.Data(), fhStsSortedDtP1N2);
   } // if( kTRUE == fbDualStsEna )

   sHistName = "fhStsSortedMapN1P1";
   title = "Sorted hits in coincidence for Sts 1 axis N and P; N channel Sts 1 []; P channel Sts 1 []; MS []";
   fhStsSortedMapN1P1 = new TH2I( sHistName, title,
                                  fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                  fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 );
   fHM->Add(sHistName.Data(), fhStsSortedMapN1P1 );

   if( kTRUE == fbDualStsEna )
   {
      sHistName = "fhStsSortedMapN2P2";
      title = "Sorted hits in coincidence for Sts 2 axis N and P; N channel Sts 2 []; P channel Sts 2 []; MS []";
      fhStsSortedMapN2P2 = new TH2I( sHistName, title,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 );
      fHM->Add(sHistName.Data(), fhStsSortedMapN2P2 );

      sHistName = "fhStsSortedMapN1N2";
      title = "Sorted hits in coincidence for Sts 1 axis N and 2 axis N; N channel Sts 1 []; N channel Sts 2 []; MS []";
      fhStsSortedMapN1N2 = new TH2I( sHistName, title,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 );
      fHM->Add(sHistName.Data(), fhStsSortedMapN1N2 );

      sHistName = "fhStsSortedMapP1P2";
      title = "Sorted hits in coincidence for Sts 1 axis P and 2 axis P; P channel Sts 1 []; P channel Sts 2 []; MS []";
      fhStsSortedMapP1P2 = new TH2I( sHistName, title,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 );
      fHM->Add(sHistName.Data(), fhStsSortedMapP1P2 );

      sHistName = "fhStsSortedMapN1P2";
      title = "Sorted hits in coincidence for Sts 1 axis N and 2 axis P; N channel Sts 1 []; P channel Sts 2 []; MS []";
      fhStsSortedMapN1P2 = new TH2I( sHistName, title,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 );
      fHM->Add(sHistName.Data(), fhStsSortedMapN1P2 );

      sHistName = "fhStsSortedMapP1N2";
      title = "Sorted hits in coincidence for Sts 1 axis P and 2 axis N; P channel Sts 1 []; N channel Sts 2 []; MS []";
      fhStsSortedMapP1N2 = new TH2I( sHistName, title,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5,
                                     fuNbChanPerAsic, -0.5, fuNbChanPerAsic - 0.5 );
      fHM->Add(sHistName.Data(), fhStsSortedMapP1N2 );
   } // if( kTRUE == fbDualStsEna )

   // Coincidence counts evolution between some axis of the sensors
   sHistName = "hStsSortedCntEvoN1P1";
   title = "Nb of coincidences in both N1 and P1 per s; Time [s]; N1-P1 coincidences []";
   fhStsSortedCntEvoN1P1 = new TH1I(sHistName, title, 1800, 0, 1800 );
   fHM->Add(sHistName.Data(), fhStsSortedCntEvoN1P1 );

   if( kTRUE == fbDualStsEna )
   {
      sHistName = "hStsSortedCntEvoN2P2";
      title = "Nb of coincidences in both N2 and P2 per s; Time [s]; N2-P2 coincidences []";
      fhStsSortedCntEvoN2P2 = new TH1I(sHistName, title, 1800, 0, 1800 );
      fHM->Add(sHistName.Data(), fhStsSortedCntEvoN2P2 );

      sHistName = "fhStsSortedCntEvoN1N2";
      title = "Nb of coincidences in both N1 and N2 per s; Time [s]; N1-N2 coincidences []";
      fhStsSortedCntEvoN1N2 = new TH1I(sHistName, title, 1800, 0, 1800 );
      fHM->Add(sHistName.Data(), fhStsSortedCntEvoN1N2 );

      sHistName = "fhStsSortedCntEvoP1P2";
      title = "Nb of coincidences in both P1 and P2 per s; Time [s]; P1-P2 coincidences []";
      fhStsSortedCntEvoP1P2 = new TH1I(sHistName, title, 1800, 0, 1800 );
      fHM->Add(sHistName.Data(), fhStsSortedCntEvoP1P2 );

      sHistName = "hStsSortedCntEvoN1P2";
      title = "Nb of coincidences in both N1 and P2 per s; Time [s]; N1-P2 coincidences []";
      fhStsSortedCntEvoN1P2 = new TH1I(sHistName, title, 1800, 0, 1800 );
      fHM->Add(sHistName.Data(), fhStsSortedCntEvoN1P2 );

      sHistName = "hStsSortedCntEvoP1N2";
      title = "Nb of coincidences in both P1 and N2 per s; Time [s]; P1-N2 coincidences []";
      fhStsSortedCntEvoP1N2 = new TH1I(sHistName, title, 1800, 0, 1800 );
      fHM->Add(sHistName.Data(), fhStsSortedCntEvoP1N2 );
   } // if( kTRUE == fbDualStsEna )

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++///

   // Distribution of the TS_MSB per StsXyter
   sHistName = "hStsAsicTsMsb";
   title = "Raw Timestamp Msb distribution per StsXyter; Ts MSB []; StsXyter []; Hits []";
   fhStsAsicTsMsb = new TH2I( sHistName, title, stsxyter::kuTsMsbNbTsBins, -0.5,   stsxyter::kuTsMsbNbTsBins - 0.5,
                                                fuNbStsXyters, -0.5, fuNbStsXyters - 0.5 );
   fHM->Add(sHistName.Data(), fhStsAsicTsMsb );

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
      server->Register("/StsRaw", fhStsDpbRawTsMsb );
      server->Register("/StsRaw", fhStsDpbRawTsMsbSx );
      server->Register("/StsRaw", fhStsDpbRawTsMsbDpb );
      server->Register("/StsRaw", fhStsMessTypePerElink );
      server->Register("/StsRaw", fhStsSysMessTypePerElink );
      for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      {
         server->Register("/StsRaw", fhStsChanCounts[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanRawAdc[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanRawAdcProf[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanRawTs[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanMissEvt[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanHitRateEvo[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsXyterRateEvo[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanHitRateEvoLong[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsXyterRateEvoLong[ uXyterIdx ] );
         if( fbChanHitDtEna )
         {
            server->Register("/StsRaw", fhStsChanHitDt[ uXyterIdx ] );
            server->Register("/StsRaw", fhStsChanHitDtNeg[ uXyterIdx ] );
         } // if( fbChanHitDtEna )
         server->Register("/StsRaw", fhStsChanHitsPerMs[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanSameMs[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanSameMsTimeDiff[ uXyterIdx ] );
         if( kTRUE == fbLongHistoEnable )
         {
            server->Register("/StsRaw", fhFebRateEvoLong[ uXyterIdx ] );
            server->Register("/StsRaw", fhFebChRateEvoLong[ uXyterIdx ] );
         } // if( kTRUE == fbLongHistoEnable )
      } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

      server->Register("/StsRaw", fhStsSameMs1NP );
      if( kTRUE == fbDualStsEna )
      {
         server->Register("/StsRaw", fhStsSameMs2NP );
         server->Register("/StsRaw", fhStsSameMsN1N2 );
         server->Register("/StsRaw", fhStsSameMsP1P2 );
         server->Register("/StsRaw", fhStsSameMsN1P2 );
         server->Register("/StsRaw", fhStsSameMsP1N2 );
      } // if( kTRUE == fbDualStsEna )

      server->Register("/StsRaw", fhStsSameMsCntEvoN1P1 );
      if( kTRUE == fbDualStsEna )
      {
         server->Register("/StsRaw", fhStsSameMsCntEvoN2P2 );
         server->Register("/StsRaw", fhStsSameMsCntEvoN1N2 );
         server->Register("/StsRaw", fhStsSameMsCntEvoP1P2 );
         server->Register("/StsRaw", fhStsSameMsCntEvoN1P2 );
         server->Register("/StsRaw", fhStsSameMsCntEvoP1N2 );
         server->Register("/StsRaw", fhStsSameMsCntEvoN1P1N2P2 );
      } // if( kTRUE == fbDualStsEna )

      server->Register("/StsRaw", fhStsSortedDtN1P1 );
      if( kTRUE == fbDualStsEna )
      {
         server->Register("/StsRaw", fhStsSortedDtN2P2 );
         server->Register("/StsRaw", fhStsSortedDtN1N2 );
         server->Register("/StsRaw", fhStsSortedDtP1P2 );
         server->Register("/StsRaw", fhStsSortedDtN1P2 );
         server->Register("/StsRaw", fhStsSortedDtP1N2 );
      } // if( kTRUE == fbDualStsEna )

      server->Register("/StsRaw", fhStsSortedMapN1P1 );
      if( kTRUE == fbDualStsEna )
      {
         server->Register("/StsRaw", fhStsSortedMapN2P2 );
         server->Register("/StsRaw", fhStsSortedMapN1N2 );
         server->Register("/StsRaw", fhStsSortedMapP1P2 );
         server->Register("/StsRaw", fhStsSortedMapN1P2 );
         server->Register("/StsRaw", fhStsSortedMapP1N2 );
      } // if( kTRUE == fbDualStsEna )

      server->Register("/StsRaw", fhStsSortedCntEvoN1P1 );
      if( kTRUE == fbDualStsEna )
      {
         server->Register("/StsRaw", fhStsSortedCntEvoN2P2 );
         server->Register("/StsRaw", fhStsSortedCntEvoN1N2 );
         server->Register("/StsRaw", fhStsSortedCntEvoP1P2 );
         server->Register("/StsRaw", fhStsSortedCntEvoN1P2 );
         server->Register("/StsRaw", fhStsSortedCntEvoP1N2 );
      } // if( kTRUE == fbDualStsEna )

      if( server ) server->Register("/StsRaw", fhStsAsicTsMsb );
      server->RegisterCommand("/Reset_All_Sts", "bCosy2018ResetStsHistos=kTRUE");
      server->RegisterCommand("/Write_All_Sts", "bCosy2018WriteStsHistos=kTRUE");

      server->Restrict("/Reset_All_Sts", "allow=admin");
      server->Restrict("/Write_All_Sts", "allow=admin");
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
/*
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
*/
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


//====================================================================//
   TCanvas* cStsMaps = new TCanvas( "cStsMaps",
                                    "Sts coincidence maps",
                                    w, h);
   if( kTRUE == fbDualStsEna )
   {
      cStsMaps->Divide( 2, 3 );
   } // if( kTRUE == fbDualStsEna )

   cStsMaps->cd(1);
   gPad->SetLogz();
   fhStsSameMs1NP->Draw( "colz" );

   if( kTRUE == fbDualStsEna )
   {
      cStsMaps->cd(2);
      gPad->SetLogz();
      fhStsSameMs2NP->Draw( "colz" );

      cStsMaps->cd(3);
      gPad->SetLogz();
      fhStsSameMsN1N2->Draw( "colz" );

      cStsMaps->cd(4);
      gPad->SetLogz();
      fhStsSameMsP1P2->Draw( "colz" );

      cStsMaps->cd(5);
      gPad->SetLogz();
      fhStsSameMsN1P2->Draw( "colz" );

      cStsMaps->cd(6);
      gPad->SetLogz();
      fhStsSameMsP1N2->Draw( "colz" );
   } // if( kTRUE == fbDualStsEna )
//====================================================================//

//====================================================================//
   TCanvas* cStsCoincEvo = new TCanvas( "cStsCoincEvo",
                                    "Sts coincidence rate evolution",
                                    w, h);
   if( kTRUE == fbDualStsEna )
   {
      cStsCoincEvo->Divide( 2, 4 );
   } // if( kTRUE == fbDualStsEna )

   cStsCoincEvo->cd(1);
   gPad->SetLogz();
   fhStsSameMsCntEvoN1P1->Draw( "colz" );

   if( kTRUE == fbDualStsEna )
   {
      cStsCoincEvo->cd(2);
      gPad->SetLogz();
      fhStsSameMsCntEvoN2P2->Draw( "colz" );

      cStsCoincEvo->cd(3);
      gPad->SetLogz();
      fhStsSameMsCntEvoN1N2->Draw( "colz" );

      cStsCoincEvo->cd(4);
      gPad->SetLogz();
      fhStsSameMsCntEvoP1P2->Draw( "colz" );

      cStsCoincEvo->cd(5);
      gPad->SetLogz();
      fhStsSameMsCntEvoN1P2->Draw( "colz" );

      cStsCoincEvo->cd(6);
      gPad->SetLogz();
      fhStsSameMsCntEvoP1N2->Draw( "colz" );

      cStsCoincEvo->cd(7);
      gPad->SetLogz();
      fhStsSameMsCntEvoN1P1N2P2->Draw( "colz" );
   } // if( kTRUE == fbDualStsEna )
//====================================================================//

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

Bool_t CbmCosy2018MonitorSts::DoUnpack(const fles::Timeslice& ts, size_t component)
{
#ifdef USE_HTTP_SERVER
  THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
#endif

   if( bCosy2018ResetStsHistos )
   {
      ResetAllHistos();
      bCosy2018ResetStsHistos = kFALSE;
   } // if( bCosy2018ResetStsHistos )
   if( bCosy2018WriteStsHistos )
   {
      SaveAllHistos( "data/StsHistos.root" );
      bCosy2018WriteStsHistos = kFALSE;
   } // if( bCosy2018WriteStsHistos )

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
      LOG(INFO) << "CbmCosy2018MonitorSts::DoUnpack => Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                   << FairLogger::endl;
      LOG(INFO) << "CbmCosy2018MonitorSts::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fuNbChanPerAsic
                   << FairLogger::endl;
      LOG(INFO) << "CbmCosy2018MonitorSts::DoUnpack =>  Changed fvuChanNbHitsInMs size "
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
      LOG(DEBUG) << "Microslice: " << fulCurrentMsIdx
                 << " from EqId " << std::hex << fuCurrentEquipmentId << std::dec
                 << " has size: " << size << FairLogger::endl;

      fuCurrDpbId  = static_cast< uint32_t >( fuCurrentEquipmentId & 0xFFFF );
      fuCurrDpbIdx = fDpbIdIndexMap[ fuCurrDpbId ];

      if( component < kiMaxNbFlibLinks )
      {
         if( fdStartTimeMsSz < 0 )
            fdStartTimeMsSz = (1e-9) * static_cast<double>(fulCurrentMsIdx);
         fhMsSz[ component ]->Fill( size );
         fhMsSzTime[ component ]->Fill( (1e-9) * static_cast<double>( fulCurrentMsIdx) - fdStartTimeMsSz, size);
      } // if( component < kiMaxNbFlibLinks )

      if( 0 == component && 0 == ( ( fulCurrentMsIdx / 1000000000 ) % 10 )
                         && 0 == ( ( fulCurrentMsIdx / 100000 ) ) %50000 )
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
                     LOG(DEBUG) << "Microslice Index =  " << (fulCurrentMsIdx / 1000000000)
                                << "s, ASIC " << uXyterIdx << " Chan " << uChan << " VS " << uChanB
                                << " Entries " << fpStsChanSameMsTimeDiff[uXyterIdx]->GetBinEntries( fpStsChanSameMsTimeDiff[uXyterIdx]->GetBin( uChan, uChanB ) )
                                << " Mean time diff "<< fpStsChanSameMsTimeDiff[uXyterIdx]->GetBinContent( uChan, uChanB )
                                << " Error " << fpStsChanSameMsTimeDiff[uXyterIdx]->GetBinError( uChan, uChanB )
                                << FairLogger::endl;
                  } // if( 0 < fpStsChanSameMsTimeDiff[uXyterIdx]->GetBinEntries( uChan, uChanB ) )
*/
         }  // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      } // if( 0 == ( ( fulCurrentMsIdx / 1e-9 ) % 10 )  )

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
         fhStsMessType->Fill( static_cast< uint16_t > (typeMess) );
         fhStsMessTypePerDpb->Fill( fuCurrDpbIdx, static_cast< uint16_t > (typeMess) );

         switch( typeMess )
         {
            case stsxyter::MessType::Hit :
            {
               // Extract the eLink and Asic indices => Should GO IN the fill method now that obly hits are link/asic specific!
               UShort_t usElinkIdx = mess.GetLinkIndex();
               if( fuNbElinksPerDpb <= usElinkIdx )
               {
                  LOG(FATAL) << "CbmCosy2018MonitorSts::DoUnpack => "
                             << "eLink index out of bounds!"
                             << usElinkIdx << " VS " << fuNbElinksPerDpb
                             << FairLogger::endl;
               } // if( fuNbElinksPerDpb <= usElinkIdx )
               fhStsMessTypePerElink->Fill( fuCurrDpbIdx * fuNbElinksPerDpb + usElinkIdx,
                                            static_cast< uint16_t > (typeMess) );

               UInt_t   uAsicIdx   = fvuElinkToAsic[fuCurrDpbIdx][usElinkIdx];
               if( ! ( fUnpackPar->GetAsicIndexSts1N() == uAsicIdx ||
                       fUnpackPar->GetAsicIndexSts1P() == uAsicIdx ) )
               {
                  if( kFALSE == fbDualStsEna )
                     continue;

                  if( ! ( fUnpackPar->GetAsicIndexSts2N() == uAsicIdx ||
                          fUnpackPar->GetAsicIndexSts2P() == uAsicIdx ) )
                     continue;
               } // If neither P nor n side of STS sensor 1

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
                  LOG(INFO) << "CbmCosy2018MonitorSts::DoUnpack => "
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
               LOG(FATAL) << "CbmCosy2018MonitorSts::DoUnpack => "
                          << "Unknown message type, should never happen, stopping here!"
                          << FairLogger::endl;
            }
         } // switch( mess.GetMessType() )

      } // for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )
/*
      LOG(INFO) << " Finished MS " << std::setw( 12 ) << fulCurrentMsIdx
                << " for DPB " << std::setw( 2 ) << fuCurrDpbIdx
                << FairLogger::endl;
*/
   } // for( size_t m = 0; m < numCompMsInTs; ++m )


   // End of TS, check if stuff to do with the hits inside each MS
   // Usefull for low rate pulser tests
      // Need to do it only when last DPB is processed, as they are done one by one
   if( fuCurrDpbIdx == fuNrOfDpbs - 1 )
   {
      for( UInt_t uMsIdx = 0; uMsIdx < fuMaxNbMicroslices; ++uMsIdx )
      {
         std::vector< Bool_t> bSts1N( fuNbChanPerAsic, kFALSE);
         std::vector< Bool_t> bSts1P( fuNbChanPerAsic, kFALSE);
         std::vector< Bool_t> bSts2N( fuNbChanPerAsic, kFALSE);
         std::vector< Bool_t> bSts2P( fuNbChanPerAsic, kFALSE);
         for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
         {
            if( fUnpackPar->GetAsicIndexSts1N() == uXyterIdx )
            {
               // Sts 1 N
               for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
                  if( fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ] )
                     bSts1N[ uChan ] = kTRUE;
            } // if( fUnpackPar->GetAsicIndexSts1N() == uXyterIdx )
            else if( fUnpackPar->GetAsicIndexSts1P() == uXyterIdx )
            {
               // Sts 1 P
               for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
                  if( fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ] )
                     bSts1P[ uChan ] = kTRUE;
            } // if( fUnpackPar->GetAsicIndexSts1P() == uXyterIdx )
            else if( kTRUE == fbDualStsEna )
            {
               if( fUnpackPar->GetAsicIndexSts2N() == uXyterIdx )
               {
                  // Sts 1 N
                  for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
                     if( fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ] )
                        bSts2N[ uChan ] = kTRUE;
               } // if( fUnpackPar->GetAsicIndexSts2N() == uXyterIdx )
               else if( fUnpackPar->GetAsicIndexSts2P() == uXyterIdx )
               {
                  // Sts 1 P
                  for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
                     if( fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ] )
                        bSts2P[ uChan ] = kTRUE;
               } // if( fUnpackPar->GetAsicIndexSts2P() == uXyterIdx )
            } // if( kTRUE == fbDualStsEna )

            for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
            {
               // Reset counters
               fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
               fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = -1.0;
               fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
            } // for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
         } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

         Bool_t bHitInSts1N = kFALSE;
         Bool_t bHitInSts1P = kFALSE;
         Bool_t bHitInSts2N = kFALSE;
         Bool_t bHitInSts2P = kFALSE;
         for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
         {
            if( bSts1N[ uChan ] )
               bHitInSts1N = kTRUE;

            if( bSts1P[ uChan ] )
               bHitInSts1P = kTRUE;

            if( kTRUE == fbDualStsEna )
            {
               if( bSts2N[ uChan ] )
                  bHitInSts2N = kTRUE;

               if( bSts2P[ uChan ] )
                  bHitInSts2P = kTRUE;
            } // if( kTRUE == fbDualStsEna )

            for( UInt_t uChanB = 0; uChanB < fuNbChanPerAsic; ++uChanB )
            {
               if( bSts1N[ uChan ] && bSts1P[ uChanB ] )
                  fhStsSameMs1NP->Fill( uChan, uChanB);

               if( kTRUE == fbDualStsEna )
               {
                  if( bSts2N[ uChan ] && bSts2P[ uChanB ] )
                     fhStsSameMs2NP->Fill( uChan, uChanB );

                  if( bSts1N[ uChan ] && bSts2N[ uChanB ] )
                     fhStsSameMsN1N2->Fill( uChan, uChanB );

                  if( bSts1P[ uChan ] && bSts2P[ uChanB ] )
                     fhStsSameMsP1P2->Fill( uChan, uChanB );

                  if( bSts1N[ uChan ] && bSts2P[ uChanB ] )
                     fhStsSameMsN1P2->Fill( uChan, uChanB );

                  if( bSts1P[ uChan ] && bSts2N[ uChanB ] )
                     fhStsSameMsP1N2->Fill( uChan, uChanB );
                  } // if( kTRUE == fbDualStsEna )
            } // for( UInt_t uChanB = 0; uChanB < fuNbChanPerAsic; ++uChanB )
         } // for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )

         if( bHitInSts1N && bHitInSts1P )
            fhStsSameMsCntEvoN1P1->Fill( fvdMsTime[ uMsIdx ] - fdStartTimeMsSz );

         if( kTRUE == fbDualStsEna )
         {
            if( bHitInSts2N && bHitInSts2P )
               fhStsSameMsCntEvoN2P2->Fill( fvdMsTime[ uMsIdx ] - fdStartTimeMsSz );

            if( bHitInSts1N && bHitInSts2N )
               fhStsSameMsCntEvoN1N2->Fill( fvdMsTime[ uMsIdx ] - fdStartTimeMsSz );

            if( bHitInSts1P && bHitInSts2P )
               fhStsSameMsCntEvoP1P2->Fill( fvdMsTime[ uMsIdx ] - fdStartTimeMsSz );

            if( bHitInSts1N && bHitInSts2P )
               fhStsSameMsCntEvoN1P2->Fill( fvdMsTime[ uMsIdx ] - fdStartTimeMsSz );

            if( bHitInSts1P && bHitInSts2N )
               fhStsSameMsCntEvoP1N2->Fill( fvdMsTime[ uMsIdx ] - fdStartTimeMsSz );

            if( bHitInSts1N && bHitInSts1P && bHitInSts2N && bHitInSts2P)
               fhStsSameMsCntEvoN1P1N2P2->Fill( fvdMsTime[ uMsIdx ] - fdStartTimeMsSz );
         } // if( kTRUE == fbDualStsEna )

         fvdMsTime[ uMsIdx ] = 0.0;
      } // for( UInt_t uMsIdx = 0; uMsIdx < fuMaxNbMicroslices; ++uMsIdx )


      // Time differences plotting using the fully time sorted hits
      if( 0 < fvmHitsInTs.size() )
      {
         ULong64_t ulLastHitTime = ( *( fvmHitsInTs.rbegin() ) ).GetTs();
         std::multiset< stsxyter::FinalHit >::iterator it;

         std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();
         Double_t dUnixTimeInRun = std::chrono::duration_cast< std::chrono::seconds >(tNow - ftStartTimeUnix).count();

         Double_t dCoincBorder = 50.0; // ns, +/-

         for( it  = fvmHitsInTs.begin();
              it != fvmHitsInTs.end() && (*it).GetTs() < ulLastHitTime - 320; // 32 * 3.125 ns = 1000 ns
              ++it )
         {
            UShort_t usAsicIdx = (*it).GetAsic();
            UShort_t usChanIdx = (*it).GetChan();
            ULong64_t ulHitTs  = (*it).GetTs();

            Double_t dTimeSinceStartSec = (ulHitTs * stsxyter::kdClockCycleNs - fdStartTime)* 1e-9;

            if( fUnpackPar->GetAsicIndexSts1N() == usAsicIdx )
            {
               fLastSortedHit1N = (*it);

               Double_t dDtN1P1 = ( fLastSortedHit1P.GetTs() * stsxyter::kdClockCycleNs - ulHitTs * stsxyter::kdClockCycleNs );
               fhStsSortedDtN1P1->Fill( dDtN1P1 );
               if( TMath::Abs( dDtN1P1 ) < dCoincBorder )
               {
                  fhStsSortedMapN1P1->Fill( usChanIdx, fLastSortedHit1P.GetChan() );
                  fhStsSortedCntEvoN1P1->Fill( dTimeSinceStartSec );
               } // if( TMath::Abs( dDtN1P1 ) < dCoincBorder )

               if( kTRUE == fbDualStsEna )
               {
                  Double_t dDtN1N2 = ( fLastSortedHit2N.GetTs() * stsxyter::kdClockCycleNs - ulHitTs * stsxyter::kdClockCycleNs );
                  Double_t dDtN1P2 = ( fLastSortedHit2P.GetTs() * stsxyter::kdClockCycleNs - ulHitTs * stsxyter::kdClockCycleNs );

                  fhStsSortedDtN1N2->Fill( dDtN1N2 );
                  fhStsSortedDtN1P2->Fill( dDtN1P2 );

                  if( TMath::Abs( dDtN1N2 ) < dCoincBorder )
                  {
                     fhStsSortedMapN1N2->Fill( usChanIdx, fLastSortedHit2N.GetChan() );
                     fhStsSortedCntEvoN1N2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtN1N2 ) < dCoincBorder )
                  if( TMath::Abs( dDtN1P2 ) < dCoincBorder )
                  {
                     fhStsSortedMapN1P2->Fill( usChanIdx, fLastSortedHit2P.GetChan() );
                     fhStsSortedCntEvoN1P2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtN1P2 ) < dCoincBorder )
               } // if( kTRUE == fbDualStsEna )
            } // if( fUnpackPar->GetAsicIndexSts1N() == usAsicIdx )
            else if( fUnpackPar->GetAsicIndexSts1P() == usAsicIdx )
            {
               fLastSortedHit1P = (*it);

               Double_t dDtN1P1 = ( ulHitTs - fLastSortedHit1N.GetTs() ) * stsxyter::kdClockCycleNs;
               fhStsSortedDtN1P1->Fill( dDtN1P1 );
               if( TMath::Abs( dDtN1P1 ) < dCoincBorder )
               {
                  fhStsSortedMapN1P1->Fill( fLastSortedHit1N.GetChan(), usChanIdx );
                  fhStsSortedCntEvoN1P1->Fill( dTimeSinceStartSec );
               }

               if( kTRUE == fbDualStsEna )
               {
                  Double_t dDtP1P2 = ( fLastSortedHit2P.GetTs() * stsxyter::kdClockCycleNs - ulHitTs * stsxyter::kdClockCycleNs );
                  Double_t dDtP1N2 = ( fLastSortedHit2N.GetTs() * stsxyter::kdClockCycleNs - ulHitTs * stsxyter::kdClockCycleNs );

                  fhStsSortedDtP1P2->Fill( dDtP1P2 );
                  fhStsSortedDtP1N2->Fill( dDtP1N2 );

                  if( TMath::Abs( dDtP1P2 ) < dCoincBorder )
                  {
                     fhStsSortedMapP1P2->Fill( usChanIdx, fLastSortedHit2P.GetChan() );
                     fhStsSortedCntEvoP1P2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtP1P2 ) < dCoincBorder )
                  if( TMath::Abs( dDtP1N2 ) < dCoincBorder )
                  {
                     fhStsSortedMapP1N2->Fill( usChanIdx, fLastSortedHit2N.GetChan() );
                     fhStsSortedCntEvoP1N2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtP1N2 ) < dCoincBorder )
               } // if( kTRUE == fbDualStsEna )
            } // else if( fUnpackPar->GetAsicIndexSts1P() == usAsicIdx )
            else if( kTRUE == fbDualStsEna )
            {
               if( fUnpackPar->GetAsicIndexSts2N() == usAsicIdx )
               {
                  fLastSortedHit2N = (*it);

                  Double_t dDtN2P2 = ( fLastSortedHit2P.GetTs() * stsxyter::kdClockCycleNs - ulHitTs * stsxyter::kdClockCycleNs );
                  Double_t dDtN1N2 = ( ulHitTs * stsxyter::kdClockCycleNs - fLastSortedHit1N.GetTs() * stsxyter::kdClockCycleNs );
                  Double_t dDtP1N2 = ( ulHitTs * stsxyter::kdClockCycleNs - fLastSortedHit1P.GetTs() * stsxyter::kdClockCycleNs ) * stsxyter::kdClockCycleNs;

                  fhStsSortedDtN2P2->Fill( dDtN2P2 );
                  fhStsSortedDtN1N2->Fill( dDtN1N2 );
                  fhStsSortedDtP1N2->Fill( dDtP1N2 );

                  if( TMath::Abs( dDtN2P2 ) < dCoincBorder )
                  {
                     fhStsSortedMapN2P2->Fill( usChanIdx, fLastSortedHit2P.GetChan() );
                     fhStsSortedCntEvoN2P2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtN2P2 ) < dCoincBorder )
                  if( TMath::Abs( dDtN1N2 ) < dCoincBorder )
                  {
                     fhStsSortedMapN1N2->Fill( fLastSortedHit1N.GetChan(), usChanIdx );
                     fhStsSortedCntEvoN1N2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtN1N2 ) < dCoincBorder )
                  if( TMath::Abs( dDtP1N2 ) < dCoincBorder )
                  {
                     fhStsSortedMapP1N2->Fill( fLastSortedHit1P.GetChan(), usChanIdx );
                     fhStsSortedCntEvoP1N2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtP1N2 ) < dCoincBorder )
               } // if( fUnpackPar->GetAsicIndexSts2N() == usAsicIdx )
               else if( fUnpackPar->GetAsicIndexSts2P() == usAsicIdx )
               {
                  fLastSortedHit2P = (*it);

                  Double_t dDtN2P2 = ( ulHitTs * stsxyter::kdClockCycleNs - fLastSortedHit1N.GetTs() * stsxyter::kdClockCycleNs );
                  Double_t dDtP1P2 = ( ulHitTs * stsxyter::kdClockCycleNs - fLastSortedHit1P.GetTs() * stsxyter::kdClockCycleNs );
                  Double_t dDtN1P2 = ( ulHitTs * stsxyter::kdClockCycleNs - fLastSortedHit2N.GetTs() * stsxyter::kdClockCycleNs );

                  fhStsSortedDtN2P2->Fill( dDtN2P2 );
                  fhStsSortedDtP1P2->Fill( dDtP1P2 );
                  fhStsSortedDtN1P2->Fill( dDtN1P2 );

                  if( TMath::Abs( dDtN2P2 ) < dCoincBorder )
                  {
                     fhStsSortedMapN2P2->Fill( fLastSortedHit2N.GetChan(), usChanIdx );
                     fhStsSortedCntEvoN2P2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtN2P2 ) < dCoincBorder )
                  if( TMath::Abs( dDtP1P2 ) < dCoincBorder )
                  {
                     fhStsSortedMapP1P2->Fill( fLastSortedHit1P.GetChan(), usChanIdx );
                     fhStsSortedCntEvoP1P2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtP1P2 ) < dCoincBorder )
                  if( TMath::Abs( dDtN1P2 ) < dCoincBorder )
                  {
                     fhStsSortedMapN1P2->Fill( fLastSortedHit1N.GetChan(), usChanIdx );
                     fhStsSortedCntEvoN1P2->Fill( dTimeSinceStartSec );
                  } // if( TMath::Abs( dDtN1P2 ) < dCoincBorder )
               } // else if( fUnpackPar->GetAsicIndexSts2P() == usAsicIdx )
            } // else if( kTRUE == fbDualStsEna )
         } // loop on hits untils hits within 100 ns of last one or last one itself are reached

         // Remove all hits which were already used
         fvmHitsInTs.erase( fvmHitsInTs.begin(), it );
      } // if( 0 < fvmHitsInTs.size() )
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

void CbmCosy2018MonitorSts::FillHitInfo( stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx )
{
   UShort_t usChan   = mess.GetHitChannel();
   UShort_t usRawAdc = mess.GetHitAdc();
//   UShort_t usFullTs = mess.GetHitTimeFull();
   UShort_t usTsOver = mess.GetHitTimeOver();
   UShort_t usRawTs  = mess.GetHitTime();

   fhStsChanCounts[  uAsicIdx ]->Fill( usChan );
   fhStsChanRawAdc[  uAsicIdx ]->Fill( usChan, usRawAdc );
   fhStsChanRawAdcProf[  uAsicIdx ]->Fill( usChan, usRawAdc );
   fhStsChanRawTs[   uAsicIdx ]->Fill( usChan, usRawTs );
   fhStsChanMissEvt[ uAsicIdx ]->Fill( usChan, mess.IsHitMissedEvts() );

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

   fvuElinkLastTsHit[fuCurrDpbIdx] = usRawTs;

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
   fvmChanHitsInTs[        uAsicIdx ][ usChan ].insert( stsxyter::FinalHit( fvulChanLastHitTime[ uAsicIdx ][ usChan ], usRawAdc, uAsicIdx, usChan ) );

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

void CbmCosy2018MonitorSts::FillTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx, UInt_t uMsIdx )
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

   if( 1 < uMessIdx )
   {
      fhStsDpbRawTsMsb->Fill( fuCurrDpbIdx,      fvulCurrentTsMsb[fuCurrDpbIdx] );
      fhStsDpbRawTsMsbSx->Fill( fuCurrDpbIdx,  ( fvulCurrentTsMsb[fuCurrDpbIdx] & 0x1F ) );
      fhStsDpbRawTsMsbDpb->Fill( fuCurrDpbIdx, ( fvulCurrentTsMsb[fuCurrDpbIdx] >> 5 ) );
   } // if( 0 < uMessIdx )

//   fhStsAsicTsMsb->Fill( fvulCurrentTsMsb[fuCurrDpbIdx], uAsicIdx );
}

void CbmCosy2018MonitorSts::FillEpochInfo( stsxyter::Message mess )
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

void CbmCosy2018MonitorSts::Reset()
{
}

void CbmCosy2018MonitorSts::Finish()
{

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   LOG(INFO) << "CbmCosy2018MonitorSts statistics are " << FairLogger::endl;
   LOG(INFO) << " Hit      messages: " << fmMsgCounter[ stsxyter::MessType::Hit   ] << FairLogger::endl
             << " Ts MSB   messages: " << fmMsgCounter[ stsxyter::MessType::TsMsb ] << FairLogger::endl
             << " Dummy    messages: " << fmMsgCounter[ stsxyter::MessType::Dummy ] << FairLogger::endl
             << " Epoch    messages: " << fmMsgCounter[ stsxyter::MessType::Epoch ] << FairLogger::endl
             << " Empty    messages: " << fmMsgCounter[ stsxyter::MessType::Empty ] << FairLogger::endl;

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;

   SaveAllHistos();

}


void CbmCosy2018MonitorSts::FillOutput(CbmDigi* /*digi*/)
{
}

void CbmCosy2018MonitorSts::SaveAllHistos( TString sFileName )
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
   fhStsDpbRawTsMsb->Write();
   fhStsDpbRawTsMsbSx->Write();
   fhStsDpbRawTsMsbDpb->Write();
   fhStsMessTypePerElink->Write();
   fhStsSysMessTypePerElink->Write();

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fhStsChanCounts[ uXyterIdx ]->Write();
      fhStsChanRawAdc[ uXyterIdx ]->Write();
      fhStsChanRawAdcProf[ uXyterIdx ]->Write();
      fhStsChanRawTs[ uXyterIdx ]->Write();
      fhStsChanMissEvt[ uXyterIdx ]->Write();
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
/*
   if( kTRUE == fbPulserTimeDiffOn )
   {
      UInt_t uNbPulserChans = fvuPulserAsic.size();
      UInt_t uHistoIdx = 0;
      for( UInt_t uChA = 0; uChA < uNbPulserChans; ++uChA )
         for( UInt_t uChB = uChA + 1; uChB < uNbPulserChans; ++uChB )
         {
            fhStsPulserChansTimeDiff[ uHistoIdx ]->Write();
            uHistoIdx ++;
         } // Loop on channel pairs
   } // if( kTRUE == fbPulserTimeDiffOn )
*/
   fhStsAsicTsMsb->Write();

   gDirectory->cd("..");

   gDirectory->mkdir("Sts_Map");
   gDirectory->cd("Sts_Map");
   // Coincidences in same MS (unsorted hits)
      // Coincidences in sorted hits
   fhStsSameMs1NP->Write();
   fhStsSameMsCntEvoN1P1->Write();
   fhStsSortedDtN1P1->Write();
   fhStsSortedMapN1P1->Write();
   if( kTRUE == fbDualStsEna )
   {
      fhStsSameMs2NP->Write();
      fhStsSameMsN1N2->Write();
      fhStsSameMsP1P2->Write();
      fhStsSameMsN1P2->Write();
      fhStsSameMsP1N2->Write();
      fhStsSameMsCntEvoN2P2->Write();
      fhStsSameMsCntEvoN1N2->Write();
      fhStsSameMsCntEvoP1P2->Write();
      fhStsSameMsCntEvoN1P2->Write();
      fhStsSameMsCntEvoP1N2->Write();
      fhStsSameMsCntEvoN1P1N2P2->Write();
      fhStsSortedDtN2P2->Write();
      fhStsSortedDtN1N2->Write();
      fhStsSortedDtP1P2->Write();
      fhStsSortedDtN1P2->Write();
      fhStsSortedDtP1N2->Write();
      fhStsSortedMapN2P2->Write();
      fhStsSortedMapN1N2->Write();
      fhStsSortedMapP1P2->Write();
      fhStsSortedMapN1P2->Write();
      fhStsSortedMapP1N2->Write();
   } // if( kTRUE == fbDualStsEna )
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
void CbmCosy2018MonitorSts::ResetAllHistos()
{
   LOG(INFO) << "Reseting all STS histograms." << FairLogger::endl;

   fhStsMessType->Reset();
   fhStsSysMessType->Reset();
   fhStsMessTypePerDpb->Reset();
   fhStsSysMessTypePerDpb->Reset();
   fhStsDpbRawTsMsb->Reset();
   fhStsDpbRawTsMsbSx->Reset();
   fhStsDpbRawTsMsbDpb->Reset();
   fhStsMessTypePerElink->Reset();
   fhStsSysMessTypePerElink->Reset();

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fhStsChanCounts[ uXyterIdx ]->Reset();
      fhStsChanRawAdc[ uXyterIdx ]->Reset();
      fhStsChanRawAdcProf[ uXyterIdx ]->Reset();
      fhStsChanRawTs[ uXyterIdx ]->Reset();
      fhStsChanMissEvt[ uXyterIdx ]->Reset();
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
/*
   if( kTRUE == fbPulserTimeDiffOn )
   {
      UInt_t uNbPulserChans = fvuPulserAsic.size();
      UInt_t uHistoIdx = 0;
      for( UInt_t uChA = 0; uChA < uNbPulserChans; ++uChA )
         for( UInt_t uChB = uChA + 1; uChB < uNbPulserChans; ++uChB )
         {
            fhStsPulserChansTimeDiff[ uHistoIdx ]->Reset();
            uHistoIdx ++;
         } // Loop on channel pairs
   } // if( kTRUE == fbPulserTimeDiffOn )
*/
   fhStsAsicTsMsb->Reset();

   // Coincidences in same MS (unsorted hits)
      // Coincidences in sorted hits
   fhStsSameMs1NP->Reset();
   fhStsSameMsCntEvoN1P1->Reset();
   fhStsSortedDtN1P1->Reset();
   fhStsSortedMapN1P1->Reset();

   if( kTRUE == fbDualStsEna )
   {
      fhStsSameMs2NP->Reset();
      fhStsSameMsN1N2->Reset();
      fhStsSameMsP1P2->Reset();
      fhStsSameMsN1P2->Reset();
      fhStsSameMsP1N2->Reset();
      fhStsSameMsCntEvoN2P2->Reset();
      fhStsSameMsCntEvoN1N2->Reset();
      fhStsSameMsCntEvoP1P2->Reset();
      fhStsSameMsCntEvoN1P2->Reset();
      fhStsSameMsCntEvoP1N2->Reset();
      fhStsSameMsCntEvoN1P1N2P2->Reset();
      fhStsSortedDtN2P2->Reset();
      fhStsSortedDtN1N2->Reset();
      fhStsSortedDtP1P2->Reset();
      fhStsSortedDtN1P2->Reset();
      fhStsSortedDtP1N2->Reset();
      fhStsSortedMapN2P2->Reset();
      fhStsSortedMapN1N2->Reset();
      fhStsSortedMapP1P2->Reset();
      fhStsSortedMapN1P2->Reset();
      fhStsSortedMapP1N2->Reset();
   } // if( kTRUE == fbDualStsEna )

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

void CbmCosy2018MonitorSts::SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize )
{
   TDatime * fRunStartDateTime     = new TDatime( dateIn, timeIn);
   fiRunStartDateTimeSec = fRunStartDateTime->Convert();
   fiBinSizeDatePlots    = iBinSize;

   LOG(INFO) << "Assigned new MUCH Run Start Date-Time: " << fRunStartDateTime->AsString() << FairLogger::endl;
}

void CbmCosy2018MonitorSts::SetLongDurationLimits( UInt_t uDurationSeconds, UInt_t uBinSize )
{
   fbLongHistoEnable     = kTRUE;
   fuLongHistoNbSeconds  = uDurationSeconds;
   fuLongHistoBinSizeSec = uBinSize;
}

ClassImp(CbmCosy2018MonitorSts)