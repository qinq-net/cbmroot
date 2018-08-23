// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                   CbmMcbm2018MonitorMcbmSync                      -----
// -----                Created 11/05/18  by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018MonitorMcbmSync.h"

// Data

// CbmRoot
#include "CbmCern2017UnpackParHodo.h"
#include "CbmTofStar2018Par.h"
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
#include "TF1.h"

// C++11

// C/C++
#include <iostream>
#include <stdint.h>
#include <iomanip>

Bool_t bMcbm2018ResetSync = kFALSE;
Bool_t bMcbm2018WriteSync = kFALSE;
/*
Bool_t bMcbm2018ResetSync = kFALSE;
Bool_t bMcbm2018WriteSync = kFALSE;
*/
Bool_t bMcbm2018SyncResetHistosTof = kFALSE;
Bool_t bMcbm2018SyncSaveHistosTof  = kFALSE;
Bool_t bMcbm2018SyncUpdateZoomedFit = kFALSE;


CbmMcbm2018MonitorMcbmSync::CbmMcbm2018MonitorMcbmSync() :
   CbmMcbmUnpack(),
   fvMsComponentsListSts(),
   fvMsComponentsListTof(),
   fuNbCoreMsPerTs(0),
   fuNbOverMsPerTs(0),
   fbIgnoreOverlapMs(kFALSE),
   fUnpackParHodo(nullptr),
   fuStsNrOfDpbs(0),
   fmStsDpbIdIndexMap(),
   fuStsNbElinksPerDpb(0),
   fuStsNbStsXyters(0),
   fuStsNbChanPerAsic(0),
   fvuStsElinkToAsic(),
   fUnpackParTof(nullptr),
   fuTofNrOfGdpbs(0),
   fuTofNrOfFeePerGdpb(0),
   fuTofNrOfGet4PerFee(0),
   fuTofNrOfChannelsPerGet4(0),
   fuTofNrOfChannelsPerFee(0),
   fuTofNrOfFee(0),
   fuTofNrOfGet4(0),
   fuTofNrOfGet4PerGdpb(0),
   fuTofNrOfChannelsPerGdpb(0),
   fsHistoFileFullname( "data/mCBMsyncHistos.root" ),
   fbPrintMessages( kFALSE ),
   fPrintMessCtrlSts( stsxyter::MessagePrintMask::msg_print_Human ),
   fulCurrentTsIdx( 0 ),
   fulCurrentMsIdx( 0 ),
   fmMsgCounter(),
   fuCurrentEquipmentId(0),
   fuCurrDpbId(0),
   fuCurrDpbIdx(0),
   fiRunStartDateTimeSec(-1),
   fiBinSizeDatePlots(-1),
   fvulStsCurrentTsMsb(),
   fvuStsCurrentTsMsbCycle(),
   fvulStsChanLastHitTime(),
   fvdStsChanLastHitTime(),
   fdStsStartTime(-1.0),
   fdStsStartTimeMsSz(-1.0),
   ftStartTimeUnix( std::chrono::steady_clock::now() ),
   fvmStsHitsInMs(),
   fvmStsAsicHitsInMs(),
   fbLongHistoEnable( kFALSE ),
   fuLongHistoNbSeconds( 0 ),
   fuLongHistoBinSizeSec( 0 ),
   fuLongHistoBinNb( 0 ),
   fdStsCoincCenter(   0.0 ),
   fdStsCoincBorder(  50.0 ),
   fdStsCoincMin( fdStsCoincCenter - fdStsCoincBorder ),
   fdStsCoincMax( fdStsCoincCenter + fdStsCoincBorder ),
   fhStsMessType(nullptr),
   fhStsSysMessType(nullptr),
   fhStsMessTypePerDpb(nullptr),
   fhStsSysMessTypePerDpb(nullptr),
   fhStsMessTypePerElink(nullptr),
   fhStsSysMessTypePerElink(nullptr),
   fhStsChanCntRaw(),
   fhStsChanCntRawGood(),
   fhStsChanAdcRaw(),
   fhStsChanAdcRawProf(),
   fhStsChanRawTs(),
   fhStsChanMissEvt(),
   fhStsChanMissEvtEvo(),
   fhStsChanHitRateEvo(),
   fhStsFebRateEvo(),
   fhStsFebMissEvtEvo(),
   fhStsChanHitRateEvoLong(),
   fhStsFebRateEvoLong(),
   fdStsStartTs( 0.0 ),
   fvmStsLastHitAsic(),
   fhStsTimeDiffPerAsic(),
   fhStsTimeDiffPerAsicPair(),
   fhStsTimeDiffClkPerAsicPair(),
   fhStsTimeDiffEvoPerAsicPair(),
   fhStsTimeDiffEvoPerAsicPairProf(),
   fhStsRawTimeDiffEvoPerAsicPairProf(),
   fhStsTsLsbMatchPerAsicPair(),
   fhStsTsMsbMatchPerAsicPair(),
   fhStsIntervalAsic(),
   fhStsIntervalLongAsic(),
   fulTofCurrentTsIndex( 0 ),
   fuTofCurrentMs( 0 ),
   fdTofMsIndex( 0 ),
   fuTofGdpbId( 0 ),
   fuTofGdpbNr( 0 ),
   fuTofGet4Id( 0 ),
   fuTofGet4Nr( 0 ),
   fiTofEquipmentId( 0 ),
   fviTofMsgCounter( 1 + gdpbv100::MSG_STAR_TRI_D, 0),
   fvulTofGdpbTsMsb(),
   fvulTofGdpbTsLsb(),
   fvulTofStarTsMsb(),
   fvulTofStarTsMid(),
   fvulTofGdpbTsFullLast(),
   fvulTofStarTsFullLast(),
   fvuTofStarTokenLast(),
   fvuTofStarDaqCmdLast(),
   fvuTofStarTrigCmdLast(),
   fvulTofCurrentEpoch(),
   fvbTofFirstEpochSeen(),
   fulTofCurrentEpochTime( 0 ),
   fmTofGdpbIdIndexMap(),
   fvmTofEpSupprBuffer(),
   fvdTofTsLastPulserHit(),
   fvmTofFeeHitsInMs(),
   fdTofMinDt(-1.*(kuTofNbBinsDt*gdpbv100::kdBinSize/2.) - gdpbv100::kdBinSize/2.),
   fdTofMaxDt(1.*(kuTofNbBinsDt*gdpbv100::kdBinSize/2.) + gdpbv100::kdBinSize/2.),
   fuTofNbFeePlot(2),
   fuTofNbFeePlotsPerGdpb(0),
   fdTofTsStartTime(-1.),
   fdTofStartTime(-1.),
   fdTofStartTimeLong(-1.),
   fuTofHistoryHistoSize( 1800 ),
   fuTofHistoryHistoSizeLong( 6400 ),
   fdTofLastRmsUpdateTime(0.0),
   fdTofFitZoomWidthPs(0.0),
   fhTofMessType( nullptr ),
   fhTofSysMessType( nullptr ),
   fhTofGet4MessType( nullptr ),
   fhTofGet4ChanScm( nullptr ),
   fhTofGet4ChanErrors( nullptr ),
   fhTofGet4EpochFlags( nullptr ),
   fhTofScmScalerCounters( nullptr ),
   fhTofScmDeadtimeCounters( nullptr ),
   fhTofScmSeuCounters( nullptr ),
   fhTofScmSeuCountersEvo( nullptr ),
   fvhTofRawFt_gDPB(),
   fvhTofRawTot_gDPB(),
   fvhTofChCount_gDPB(),
   fvhTofChannelRate_gDPB(),
   fvhTofRemapTot_gDPB(),
   fvhTofRemapChCount_gDPB(),
   fvhTofRemapChRate_gDPB(),
   fvhTofFeeRate_gDPB(),
   fvhTofFeeErrorRate_gDPB(),
   fvhTofFeeErrorRatio_gDPB(),
   fvhTofFeeRateLong_gDPB(),
   fvhTofFeeErrorRateLong_gDPB(),
   fvhTofFeeErrorRatioLong_gDPB(),
   fvhTofTokenMsgType(),
   fvhTofTriggerRate(),
   fvhTofCmdDaqVsTrig(),
   fvhTofStarTokenEvo(),
   fvhTofStarTrigGdpbTsEvo(),
   fvhTofStarTrigStarTsEvo(),
   fvhTofTimeDiffPulser(),
   fhTofTimeRmsPulser( nullptr ),
   fhTofTimeRmsZoomFitPuls( nullptr ),
   fhTofTimeResFitPuls( nullptr ),
   fvuPadiToGet4(),
   fvuGet4ToPadi(),
   fvuElinkToGet4(),
   fvuGet4ToElink()
{
}

CbmMcbm2018MonitorMcbmSync::~CbmMcbm2018MonitorMcbmSync()
{
}

Bool_t CbmMcbm2018MonitorMcbmSync::Init()
{
  LOG(INFO) << "Initializing flib StsXyter unpacker for STS" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == nullptr) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  return kTRUE;
}

void CbmMcbm2018MonitorMcbmSync::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
         << FairLogger::endl;
   fUnpackParHodo = (CbmCern2017UnpackParHodo*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmCern2017UnpackParHodo"));
   fUnpackParTof  = (CbmTofStar2018Par*) (FairRun::Instance()->GetRuntimeDb()->getContainer( "CbmTofStar2018Par") );
}


Bool_t CbmMcbm2018MonitorMcbmSync::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
         << FairLogger::endl;

   Bool_t bReInit = ReInitContainers();
   CreateStsHistograms();
   CreateTofHistograms();
   CreateMcbmHistograms();

   return bReInit;
}

Bool_t CbmMcbm2018MonitorMcbmSync::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
         << FairLogger::endl;

/***************** STS parameters *************************************/
   fuStsNrOfDpbs       = fUnpackParHodo->GetNrOfDpbs();
   fuStsNbElinksPerDpb = fUnpackParHodo->GetNbElinksPerDpb();
   fuStsNbStsXyters    = fUnpackParHodo->GetNbStsXyters();
   fuStsNbChanPerAsic  = fUnpackParHodo->GetNbChanPerAsic();


   LOG(INFO) << "Nr. of STS DPBs:       " << fuStsNrOfDpbs
             << FairLogger::endl;

   fmStsDpbIdIndexMap.clear();
   fvuStsElinkToAsic.resize( fuStsNrOfDpbs );
   for( UInt_t uDpb = 0; uDpb < fuStsNrOfDpbs; ++uDpb )
   {
      fmStsDpbIdIndexMap[ fUnpackParHodo->GetDpbId( uDpb )  ] = uDpb;
      LOG(INFO) << "Eq. ID for DPB #" << std::setw(2) << uDpb << " = "
                << std::setw(4) << std::hex << fUnpackParHodo->GetDpbId( uDpb )
                << std::dec
                << " => " << fmStsDpbIdIndexMap[ fUnpackParHodo->GetDpbId( uDpb )  ]
                << FairLogger::endl;

      fvuStsElinkToAsic[uDpb].resize( fuStsNbElinksPerDpb );
      for( UInt_t uElink = 0; uElink < fuStsNbElinksPerDpb; ++uElink )
         fvuStsElinkToAsic[uDpb][uElink] = fUnpackParHodo->GetElinkToAsicIdx( uDpb * fuStsNbElinksPerDpb + uElink );
   } // for( UInt_t uDpb = 0; uDpb < fuStsNrOfDpbs; ++uDpb )

   LOG(INFO) << "Nr. eLinks per DPB:    " << fuStsNbElinksPerDpb
             << FairLogger::endl;
   LOG(INFO) << "Nr. of StsXyter ASICs: " << fuStsNbStsXyters
             << FairLogger::endl;
   LOG(INFO) << "Nb. channels per ASIC: " << fuStsNbChanPerAsic
             << FairLogger::endl;

   for( UInt_t uDpb = 0; uDpb < fuStsNrOfDpbs; ++uDpb )
   {
      LOG(INFO) << "ASIC ID for eLinks in DPB #" << std::setw(2) << uDpb << ": ";

      for( UInt_t uElink = 0; uElink < fuStsNbElinksPerDpb; ++uElink )
      {
         if( 0 == uElink % 10 )
            LOG(INFO) << FairLogger::endl
                      << "------> ";

         LOG(INFO) << std::setw( 5 ) << fvuStsElinkToAsic[uDpb][uElink] << " ";
      } // for( UInt_t uElink = 0; uElink < fuStsNbElinksPerDpb; ++uElink )
      LOG(INFO) << FairLogger::endl;
   } // for( UInt_t uDpb = 0; uDpb < fuStsNrOfDpbs; ++uDpb )

   // Internal status initialization
   fvulStsCurrentTsMsb.resize( fuStsNrOfDpbs );
   fvuStsCurrentTsMsbCycle.resize( fuStsNrOfDpbs );
   for( UInt_t uDpb = 0; uDpb < fuStsNrOfDpbs; ++uDpb )
   {
      fvulStsCurrentTsMsb[uDpb]     = 0;
      fvuStsCurrentTsMsbCycle[uDpb] = 0;
   } // for( UInt_t uDpb = 0; uDpb < fuStsNrOfDpbs; ++uDpb )

   fvulStsChanLastHitTime.resize( fuStsNbStsXyters );
   fvdStsChanLastHitTime.resize( fuStsNbStsXyters );
   fvmStsAsicHitsInMs.resize( fuStsNbStsXyters );
   fvmStsLastHitAsic.resize( fuStsNbStsXyters );
   fhStsTimeDiffPerAsicPair.resize( fuStsNbStsXyters );
   fhStsTimeDiffClkPerAsicPair.resize( fuStsNbStsXyters );
   fhStsTimeDiffEvoPerAsicPair.resize( fuStsNbStsXyters );
   fhStsTimeDiffEvoPerAsicPairProf.resize( fuStsNbStsXyters );
   fhStsRawTimeDiffEvoPerAsicPairProf.resize( fuStsNbStsXyters );
   fhStsTsLsbMatchPerAsicPair.resize( fuStsNbStsXyters );
   fhStsTsMsbMatchPerAsicPair.resize( fuStsNbStsXyters );
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
   {
      fvulStsChanLastHitTime[ uXyterIdx ].resize( fuStsNbChanPerAsic );
      fvdStsChanLastHitTime[ uXyterIdx ].resize( fuStsNbChanPerAsic );
      fvmStsAsicHitsInMs[ uXyterIdx ].clear();
      fhStsTimeDiffPerAsicPair[ uXyterIdx ].clear();
      fhStsTimeDiffClkPerAsicPair[ uXyterIdx ].clear();
      fhStsTimeDiffEvoPerAsicPair[ uXyterIdx ].clear();
      fhStsTimeDiffEvoPerAsicPairProf[ uXyterIdx ].clear();
      fhStsRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ].clear();
      fhStsTsLsbMatchPerAsicPair[ uXyterIdx ].clear();
      fhStsTsMsbMatchPerAsicPair[ uXyterIdx ].clear();
      for( UInt_t uChan = 0; uChan < fuStsNbChanPerAsic; ++uChan )
      {
         fvulStsChanLastHitTime[ uXyterIdx ][ uChan ] = 0;
         fvdStsChanLastHitTime[ uXyterIdx ][ uChan ] = -1.0;
      } // for( UInt_t uChan = 0; uChan < fuStsNbChanPerAsic; ++uChan )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
/***************** STS parameters *************************************/

/***************** TOF parameters *************************************/
   fuTofNrOfGdpbs = fUnpackParTof->GetNrOfRocs();
   LOG(INFO) << "Nr. of Tof GDPBs: " << fuTofNrOfGdpbs << FairLogger::endl;

   fuTofNrOfFeePerGdpb = fUnpackParTof->GetNrOfFebsPerGdpb();
   LOG(INFO) << "Nr. of FEBS per Tof GDPB: " << fuTofNrOfFeePerGdpb
               << FairLogger::endl;

   fuTofNrOfGet4PerFee = fUnpackParTof->GetNrOfGet4PerFeb();
   LOG(INFO) << "Nr. of GET4 per Tof FEB: " << fuTofNrOfGet4PerFee
               << FairLogger::endl;

   fuTofNrOfChannelsPerGet4 = fUnpackParTof->GetNrOfChannelsPerGet4();
   LOG(INFO) << "Nr. of channels per GET4: " << fuTofNrOfChannelsPerGet4
               << FairLogger::endl;

   fuTofNrOfChannelsPerFee = fuTofNrOfGet4PerFee * fuTofNrOfChannelsPerGet4;
   LOG(INFO) << "Nr. of channels per FEET: " << fuTofNrOfChannelsPerFee
               << FairLogger::endl;

   fuTofNrOfFee = fuTofNrOfGdpbs * fuTofNrOfFeePerGdpb;
   LOG(INFO) << "Nr. of FEEs: " << fuTofNrOfFee << FairLogger::endl;

   fuTofNrOfGet4 = fuTofNrOfFee * fuTofNrOfGet4PerFee;
   LOG(INFO) << "Nr. of GET4s: " << fuTofNrOfGet4 << FairLogger::endl;

   fuTofNrOfGet4PerGdpb = fuTofNrOfFeePerGdpb * fuTofNrOfGet4PerFee;
   LOG(INFO) << "Nr. of GET4s per GDPB: " << fuTofNrOfGet4PerGdpb
               << FairLogger::endl;

   fuTofNrOfChannelsPerGdpb = fuTofNrOfGet4PerGdpb * fuTofNrOfChannelsPerGet4;
   LOG(INFO) << "Nr. of channels per GDPB: " << fuTofNrOfChannelsPerGdpb
               << FairLogger::endl;

   fmTofGdpbIdIndexMap.clear();
   for( UInt_t i = 0; i < fuTofNrOfGdpbs; ++i )
   {
      fmTofGdpbIdIndexMap[fUnpackParTof->GetRocId(i)] = i;
      LOG(INFO) << "GDPB Id of TOF  " << i << " : " << std::hex << fUnpackParTof->GetRocId(i)
                 << std::dec << FairLogger::endl;
   } // for( UInt_t i = 0; i < fuTofNrOfGdpbs; ++i )
   UInt_t uNrOfChannels = fUnpackParTof->GetNumberOfChannels();
   LOG(INFO) << "Nr. of mapped Tof channels: " << uNrOfChannels;
   for( UInt_t i = 0; i < uNrOfChannels; ++i)
   {
      if (i % 8 == 0)
         LOG(INFO) << FairLogger::endl;
      LOG(INFO) << Form(" 0x%08x", fUnpackParTof->GetChannelToDetUIdMap(i) );
   } // for( UInt_t i = 0; i < uNrOfChannels; ++i)
   LOG(INFO) << FairLogger::endl;
/*
   fuTotalMsNb   = fUnpackParTof->GetNbMsTot();
   fuOverlapMsNb = fUnpackParTof->GetNbMsOverlap();
   fuCoreMs      = fuTotalMsNb - fuOverlapMsNb;
   fdMsSizeInNs  = fUnpackParTof->GetSizeMsInNs();
   fdTsCoreSizeInNs = fdMsSizeInNs * fuCoreMs;
   LOG(INFO) << "Timeslice parameters: "
             << fuTotalMsNb << " MS per link, of which "
             << fuOverlapMsNb << " overlap MS, each MS is "
             << fdMsSizeInNs << " ns"
             << FairLogger::endl;
*/
   /// STAR Trigger decoding and monitoring
   fvulTofGdpbTsMsb.resize(  fuTofNrOfGdpbs );
   fvulTofGdpbTsLsb.resize(  fuTofNrOfGdpbs );
   fvulTofStarTsMsb.resize(  fuTofNrOfGdpbs );
   fvulTofStarTsMid.resize(  fuTofNrOfGdpbs );
   fvulTofGdpbTsFullLast.resize(  fuTofNrOfGdpbs );
   fvulTofStarTsFullLast.resize(  fuTofNrOfGdpbs );
   fvuTofStarTokenLast.resize(  fuTofNrOfGdpbs );
   fvuTofStarDaqCmdLast.resize(  fuTofNrOfGdpbs );
   fvuTofStarTrigCmdLast.resize(  fuTofNrOfGdpbs );
   for (UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb)
   {
      fvulTofGdpbTsMsb[ uGdpb ] = 0;
      fvulTofGdpbTsLsb[ uGdpb ] = 0;
      fvulTofStarTsMsb[ uGdpb ] = 0;
      fvulTofStarTsMid[ uGdpb ] = 0;
      fvulTofGdpbTsFullLast[ uGdpb ] = 0;
      fvulTofStarTsFullLast[ uGdpb ] = 0;
      fvuTofStarTokenLast[ uGdpb ]   = 0;
      fvuTofStarDaqCmdLast[ uGdpb ]  = 0;
      fvuTofStarTrigCmdLast[ uGdpb ] = 0;
   } // for (Int_t iGdpb = 0; iGdpb < fuTofNrOfGdpbs; ++iGdpb)

   /// Suppressed epoch buffering (epoch after data
   fvmTofEpSupprBuffer.resize( fuTofNrOfGet4 );


   /// Epoch indedx book-keeping
   fvulTofCurrentEpoch.resize( fuTofNrOfGdpbs * fuTofNrOfGet4PerGdpb );
   fvbTofFirstEpochSeen.resize( fuTofNrOfGdpbs * fuTofNrOfGet4PerGdpb );
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      for( UInt_t uGet4 = 0; uGet4 < fuTofNrOfGet4PerGdpb; ++uGet4 )
      {
         fvulTofCurrentEpoch[  GetArrayIndexGet4(uGdpb, uGet4) ] = 0;
         fvbTofFirstEpochSeen[ GetArrayIndexGet4(uGdpb, uGet4) ] = kFALSE;
      } // for( UInt_t uGet4 = 0; uGet4 < fuTofNrOfGet4PerGdpb; ++uGet4 )
   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )

   /// Pulser monitoring
   fvdTofTsLastPulserHit.resize( fuTofNrOfFee, 0.0 );

   /// System sync monitoring
   fvmTofFeeHitsInMs.resize( fuTofNrOfFee );

/// TODO: move these constants somewhere shared, e.g the parameter file

   /// PADI channel to GET4 channel mapping and reverse
   fvuPadiToGet4.resize( fuTofNrOfChannelsPerFee );
   fvuGet4ToPadi.resize( fuTofNrOfChannelsPerFee );
   UInt_t uGet4topadi[32] = {
        4,  3,  2,  1,  // provided by Jochen
      24, 23, 22, 21,
       8,  7,  6,  5,
      28, 27, 26, 25,
      12, 11, 10,  9,
      32, 31, 30, 29,
      16, 15, 14, 13,
      20, 19, 18, 17 };

  UInt_t uPaditoget4[32] = {
       4,  3,  2,  1,  // provided by Jochen
      12, 11, 10,  9,
      20, 19, 18, 17,
      28, 27, 26, 25,
      32, 31, 30, 29,
       8,  7,  6,  5,
      16, 15, 14, 13,
      24, 23, 22, 21 };

   for( UInt_t uChan = 0; uChan < fuTofNrOfChannelsPerFee; ++uChan )
   {
      fvuPadiToGet4[ uChan ] = uPaditoget4[ uChan ] - 1;
      fvuGet4ToPadi[ uChan ] = uGet4topadi[ uChan ] - 1;
   } // for( UInt_t uChan = 0; uChan < fuTofNrOfChannelsPerFee; ++uChan )

   /// GET4 to eLink mapping and reverse
   fvuElinkToGet4.resize( kuNbGet4PerGbtx );
   fvuGet4ToElink.resize( kuNbGet4PerGbtx );
   UInt_t kuElinkToGet4[ kuNbGet4PerGbtx ] = { 27,  2,  7,  3, 31, 26, 30,  1,
                                               33, 37, 32, 13,  9, 14, 10, 15,
                                               17, 21, 16, 35, 34, 38, 25, 24,
                                                0,  6, 20, 23, 18, 22, 28,  4,
                                               29,  5, 19, 36, 39,  8, 12, 11
                                              };
   UInt_t kuGet4ToElink[ kuNbGet4PerGbtx ] = { 24,  7,  1,  3, 31, 33, 25,  2,
                                               37, 12, 14, 39, 38, 11, 13, 15,
                                               18, 16, 28, 34, 26, 17, 29, 27,
                                               23, 22,  5,  0, 30, 32,  6,  4,
                                               10,  8, 20, 19, 35,  9, 21, 36
                                              };

   for( UInt_t uLinkAsic = 0; uLinkAsic < kuNbGet4PerGbtx; ++uLinkAsic )
   {
      fvuElinkToGet4[ uLinkAsic ] = kuElinkToGet4[ uLinkAsic ];
      fvuGet4ToElink[ uLinkAsic ] = kuGet4ToElink[ uLinkAsic ];
   } // for( UInt_t uChan = 0; uChan < fuTofNrOfChannelsPerFee; ++uChan )

/***************** TOF parameters *************************************/

   return kTRUE;
}

void CbmMcbm2018MonitorMcbmSync::AddMsComponentToList( size_t component, UShort_t usDetectorId )
{
   switch( usDetectorId )
   {
      case 0x10: //fles::SubsystemIdentifier::STS:
      {
         AddMsComponentToListSts( component );
         break;
      } // case 0x10:
      case 0x60: //fles::SubsystemIdentifier::RPC:
      {
         AddMsComponentToListTof( component );
         break;
      } // case 0x60:
      default:
      {
         LOG(WARNING) << "CbmMcbm2018MonitorStsSync::AddMsComponentToList => "
                      << "Ignore component " << component
                      << " as detector id "<< usDetectorId << " is not supported by this unpacker."
                      << FairLogger::endl;
         break;
      } // default:
   } // switch( iDetectorId )
}
void CbmMcbm2018MonitorMcbmSync::AddMsComponentToListSts( size_t component )
{
   /// Check for duplicates and ignore if it is the case
   for( UInt_t uCompIdx = 0; uCompIdx < fvMsComponentsListSts.size(); ++uCompIdx )
      if( component == fvMsComponentsListSts[ uCompIdx ] )
         return;

   /// Add to list
   fvMsComponentsListSts.push_back( component );
}
void CbmMcbm2018MonitorMcbmSync::AddMsComponentToListTof( size_t component )
{
   /// Check for duplicates and ignore if it is the case
   for( UInt_t uCompIdx = 0; uCompIdx < fvMsComponentsListTof.size(); ++uCompIdx )
      if( component == fvMsComponentsListTof[ uCompIdx ] )
         return;

   /// Add to list
   fvMsComponentsListTof.push_back( component );
}
void CbmMcbm2018MonitorMcbmSync::SetNbMsInTs( size_t uCoreMsNb, size_t uOverlapMsNb )
{
   fuNbCoreMsPerTs = uCoreMsNb;
   fuNbOverMsPerTs = uOverlapMsNb;

   UInt_t uNbMsTotal = fuNbCoreMsPerTs + fuNbOverMsPerTs;
}

/***************** STS Histograms *************************************/
void CbmMcbm2018MonitorMcbmSync::SetStsCoincidenceBorder( Double_t dCenterPos, Double_t dBorderVal )
{
   fdStsCoincCenter = dCenterPos;
   fdStsCoincBorder = dBorderVal;
   fdStsCoincMin    = dCenterPos - dBorderVal;
   fdStsCoincMax    = dCenterPos + dBorderVal;
}

void CbmMcbm2018MonitorMcbmSync::CreateStsHistograms()
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
   fhStsMessTypePerDpb = new TH2I(sHistName, title, fuStsNrOfDpbs, 0, fuStsNrOfDpbs, 5, 0., 5.);
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
   fhStsSysMessTypePerDpb = new TH2I(sHistName, title, fuStsNrOfDpbs, 0, fuStsNrOfDpbs, 17, 0., 17.);
/*
   hSysMessType->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   hSysMessType->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   hSysMessType->GetYaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
*/

   sHistName = "hPulserMessageTypePerElink";
   title = "Nb of message of each type for each eLink; eLink; Type";
   fhStsMessTypePerElink = new TH2I(sHistName, title, fuStsNrOfDpbs * fuStsNbElinksPerDpb, 0, fuStsNrOfDpbs * fuStsNbElinksPerDpb, 5, 0., 5.);
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

   sHistName = "hPulserSysMessTypePerElink";
   title = "Nb of system message of each type for each eLink; eLink; System Type";
   fhStsSysMessTypePerElink = new TH2I(sHistName, title, fuStsNrOfDpbs * fuStsNbElinksPerDpb, 0, fuStsNrOfDpbs * fuStsNbElinksPerDpb, 17, 0., 17.);
/*
   fhStsSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
   fhStsSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   fhStsSysMessTypePerElink->GetYaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
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

   UInt_t uNbBinDt     = static_cast<UInt_t>( (fdStsCoincMax - fdStsCoincMin )/stsxyter::kdClockCycleNs );

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
   {
      // Channel counts
      sHistName = Form( "hPulserChanCntRaw_%03u", uXyterIdx );
      title = Form( "Hits Count per channel, StsXyter #%03u; Channel; Hits []", uXyterIdx );
      fhStsChanCntRaw.push_back( new TH1I(sHistName, title,
                                 fuStsNbChanPerAsic, -0.5, fuStsNbChanPerAsic - 0.5 ) );

      sHistName = Form( "hPulserChanCntRawGood_%03u", uXyterIdx );
      title = Form( "Hits Count per channel in good MS, StsXyter #%03u; Channel; Hits []", uXyterIdx );
      fhStsChanCntRawGood.push_back( new TH1I(sHistName, title,
                                 fuStsNbChanPerAsic, -0.5, fuStsNbChanPerAsic - 0.5 ) );

      // Raw Adc Distribution
      sHistName = Form( "hPulserChanAdcRaw_%03u", uXyterIdx );
      title = Form( "Raw Adc distribution per channel, StsXyter #%03u; Channel []; Adc []; Hits []", uXyterIdx );
      fhStsChanAdcRaw.push_back( new TH2I(sHistName, title,
                                 fuStsNbChanPerAsic, -0.5, fuStsNbChanPerAsic - 0.5,
                                 stsxyter::kuHitNbAdcBins, -0.5, stsxyter::kuHitNbAdcBins -0.5 ) );

      // Raw Adc Distribution profile
      sHistName = Form( "hPulserChanAdcRawProfc_%03u", uXyterIdx );
      title = Form( "Raw Adc prodile per channel, StsXyter #%03u; Channel []; Adc []", uXyterIdx );
      fhStsChanAdcRawProf.push_back( new TProfile(sHistName, title,
                                 fuStsNbChanPerAsic, -0.5, fuStsNbChanPerAsic - 0.5 ) );

      // Raw Ts Distribution
      sHistName = Form( "hPulserChanRawTs_%03u", uXyterIdx );
      title = Form( "Raw Timestamp distribution per channel, StsXyter #%03u; Channel []; Ts []; Hits []", uXyterIdx );
      fhStsChanRawTs.push_back( new TH2I(sHistName, title,
                                 fuStsNbChanPerAsic, -0.5, fuStsNbChanPerAsic - 0.5,
                                 stsxyter::kuHitNbTsBins, -0.5, stsxyter::kuHitNbTsBins -0.5 ) );

      // Missed event flag
      sHistName = Form( "hPulserChanMissEvt_%03u", uXyterIdx );
      title = Form( "Missed Event flags per channel, StsXyter #%03u; Channel []; Miss Evt []; Hits []", uXyterIdx );
      fhStsChanMissEvt.push_back( new TH2I(sHistName, title,
                                 fuStsNbChanPerAsic, -0.5, fuStsNbChanPerAsic - 0.5,
                                 2, -0.5, 1.5 ) );

      // Missed event flag counts evolution

      sHistName = Form( "hPulserChanMissEvtEvo_%03u", uXyterIdx );
      title = Form( "Missed Evt flags per second & channel in StsXyter #%03u; Time [s]; Channel []; Missed Evt flags []", uXyterIdx );
      fhStsChanMissEvtEvo.push_back( new TH2I( sHistName, title,
                                                1800, 0, 1800,
                                                fuStsNbChanPerAsic, -0.5, fuStsNbChanPerAsic - 0.5 ) );

      // Missed event flag counts evo per StsXyter

      sHistName = Form( "hPulserFebMissEvtEvo%03u", uXyterIdx );
      title = Form( "Missed Evt flags per second in StsXyter #%03u; Time [s]; Missed Evt flags []", uXyterIdx );
      fhStsFebMissEvtEvo.push_back( new TH1I(sHistName, title, 1800, 0, 1800 ) );

      // Hit rates evo per channel
      sHistName = Form( "hPulserChanRateEvo_%03u", uXyterIdx );
      title = Form( "Hits per second & channel in StsXyter #%03u; Time [s]; Channel []; Hits []", uXyterIdx );
      fhStsChanHitRateEvo.push_back( new TH2I( sHistName, title,
                                                1800, 0, 1800,
                                                fuStsNbChanPerAsic, -0.5, fuStsNbChanPerAsic - 0.5 ) );

      // Hit rates evo per StsXyter
      sHistName = Form( "hPulserFebRateEvo_%03u", uXyterIdx );
      title = Form( "Hits per second in StsXyter #%03u; Time [s]; Hits []", uXyterIdx );
      fhStsFebRateEvo.push_back( new TH1I(sHistName, title, 1800, 0, 1800 ) );

      // Hit rates evo per channel, 1 minute bins, 24h
      sHistName = Form( "hPulserChanRateEvoLong_%03u", uXyterIdx );
      title = Form( "Hits per second & channel in StsXyter #%03u; Time [min]; Channel []; Hits []", uXyterIdx );
      fhStsChanHitRateEvoLong.push_back( new TH2D( sHistName, title,
                                                fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5,
                                                fuStsNbChanPerAsic, -0.5, fuStsNbChanPerAsic - 0.5 ) );

      // Hit rates evo per StsXyter, 1 minute bins, 24h
      sHistName = Form( "hPulserFebRateEvoLong_%03u", uXyterIdx );
      title = Form( "Hits per second in StsXyter #%03u; Time [min]; Hits []", uXyterIdx );
      fhStsFebRateEvoLong.push_back( new TH1D(sHistName, title,
                                                fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5 ) );

      /// Coincidences inside each detector ----------------------------///
      sHistName = Form( "fhStsTimeDiffPerAsic_%03u", uXyterIdx );
      title =  Form( "Time diff for pulser hits between ASIC %03u and other ASICs; tn - t%03u [ns]; ASIC n; Counts", uXyterIdx, uXyterIdx );
      fhStsTimeDiffPerAsic.push_back( new TH2I( sHistName, title,
                                                   uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo,
                                                   fuStsNbStsXyters, -0.5, fuStsNbStsXyters - 0.5 )
                                       );

      for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
      {
         if( uXyterIdxB == uXyterIdx )
         {
            sHistName = Form( "fhStsTimeDiffSameAsic_%03u", uXyterIdx );
            title =  Form( "Time diff for consecutive hits in ASIC %03u; tn - t [ns]; Counts", uXyterIdx );
         } // if( uXyterIdxB == uXyterIdx )
            else
            {
               sHistName = Form( "fhStsTimeDiffPerAsicPair_%03u_%03u", uXyterIdx, uXyterIdxB );
               title =  Form( "Time diff for pulser hits in ASIC %03u and %03u; tn - t [ns]; Counts", uXyterIdx, uXyterIdxB );
            } // else of if( uXyterIdxB == uXyterIdx )
         fhStsTimeDiffPerAsicPair[ uXyterIdx ].push_back( new TH1I(sHistName, title, uNbBinEvo, dMinEdgeEvo, dMaxEdgeEvo ) );

         if( uXyterIdxB == uXyterIdx )
         {
            sHistName = Form( "fhStsTimeDiffClkSameAsic_%03u", uXyterIdx );
            title =  Form( "Time diff for consecutive hits in ASIC %03u; tn - t [Clk]; Counts", uXyterIdx );
         } // if( uXyterIdxB == uXyterIdx )
            else
            {
               sHistName = Form( "fhStsTimeDiffClkPerAsicPair_%03u_%03u", uXyterIdx, uXyterIdxB );
               title =  Form( "Time diff for pulser hits in ASIC %03u and %03u; tn - t [Clk]; Counts", uXyterIdx, uXyterIdxB );
            } // else of if( uXyterIdxB == uXyterIdx )
         fhStsTimeDiffClkPerAsicPair[ uXyterIdx ].push_back( new TH1I(sHistName, title, 601, -300.5, 300.5 ) );

         if( uXyterIdxB == uXyterIdx )
         {
            sHistName = Form( "fhStsTimeDiffEvoSameAsic_%03u", uXyterIdx );
            title =  Form( "Time diff for consecutive hits in ASIC %03u; Time in run [s]; tn - t [ns]; Counts", uXyterIdx );
         } // if( uXyterIdxB == uXyterIdx )
            else
            {
               sHistName = Form( "fhStsTimeDiffEvoPerAsicPair_%03u_%03u", uXyterIdx, uXyterIdxB );
               title =  Form( "Time diff for pulser hits in ASIC %03u and %03u; Time in run [s]; tn - t [ns]; Counts", uXyterIdx, uXyterIdxB );
            } // else of if( uXyterIdxB == uXyterIdx )
         fhStsTimeDiffEvoPerAsicPair[ uXyterIdx ].push_back( new TH2I(sHistName, title,
                                                                3600, 0, 18000,
                                                                200, -100 * stsxyter::kdClockCycleNs, 100 * stsxyter::kdClockCycleNs ) );

         if( uXyterIdxB == uXyterIdx )
         {
            sHistName = Form( "fhStsTimeDiffEvoSameAsicProf_%03u", uXyterIdx );
            title =  Form( "Time diff for consecutive hits in ASIC %03u; Time in run [s]; tn - t [ns]", uXyterIdx );
         } // if( uXyterIdxB == uXyterIdx )
            else
            {
               sHistName = Form( "fhStsTimeDiffEvoPerAsicPairProf_%03u_%03u", uXyterIdx, uXyterIdxB );
               title =  Form( "Time diff for pulser hits in ASIC %03u and %03u; Time in run [s]; tn - t [ns]", uXyterIdx, uXyterIdxB );
            } // else of if( uXyterIdxB == uXyterIdx )
         fhStsTimeDiffEvoPerAsicPairProf[ uXyterIdx ].push_back( new TProfile(sHistName, title, 7200, 0, 36000 ) );

         if( uXyterIdxB == uXyterIdx )
         {
            sHistName = Form( "fhStsRawTimeDiffEvoSameAsicProf_%03u", uXyterIdx );
            title =  Form( "Time diff for consecutive hits in ASIC %03u; Time in run [s]; tn - t [ns]", uXyterIdx );
         } // if( uXyterIdxB == uXyterIdx )
            else
            {
               sHistName = Form( "fhStsRawTimeDiffEvoPerAsicPairProf_%03u_%03u", uXyterIdx, uXyterIdxB );
               title =  Form( "Time diff for pulser hits in ASIC %03u and %03u; Time in run [s]; tn - t [ns]", uXyterIdx, uXyterIdxB );
            } // else of if( uXyterIdxB == uXyterIdx )
         fhStsRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ].push_back( new TProfile(sHistName, title, 7200, 0, 36000 ) );

         sHistName = Form( "fhStsTsLsbMatchPerAsicPair_%03u_%03u", uXyterIdx, uXyterIdxB );
         title =  Form( "TS LSB for pulser hits in ASIC %03u and %03u; TS LSB %03u [bin]; TS LSB %03u [bin]",
                        uXyterIdx, uXyterIdxB, uXyterIdx, uXyterIdxB );
         fhStsTsLsbMatchPerAsicPair[ uXyterIdx ].push_back( new TH2I(sHistName, title, 256, -0.5, 255.5, 256, -0.5, 255.5 ) );

         sHistName = Form( "fhStsTsMsbMatchPerAsicPair_%03u_%03u", uXyterIdx, uXyterIdxB );
         title =  Form( "TS MSB for pulser hits in ASIC %03u and %03u; TS MSB %03u [bin]; TS MSB %03u [bin]",
                        uXyterIdx, uXyterIdxB, uXyterIdx, uXyterIdxB );
         fhStsTsMsbMatchPerAsicPair[ uXyterIdx ].push_back( new TH2I(sHistName, title, 64, -0.5, 63.5, 64, -0.5, 63.5 ) );
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )

      sHistName = Form( "fhStsIntervalAsic_%03u", uXyterIdx );
      title =  Form( "Time diff between consecutive hits in ASIC %03us; dt [ns];  Counts", uXyterIdx );
      fhStsIntervalAsic.push_back( new TH1I( sHistName, title, 200, 0, 200 * stsxyter::kdClockCycleNs ) );

      sHistName = Form( "fhStsIntervalLongAsic_%03u", uXyterIdx );
      title =  Form( "Time diff between consecutive hits in ASIC %03us; dt [ns];  Counts", uXyterIdx );
      fhStsIntervalLongAsic.push_back( new TH1I( sHistName, title, 1e5, 0, 1e6 * stsxyter::kdClockCycleNs ) );

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++///
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )

   // Online histo browser commands
#ifdef USE_HTTP_SERVER
   THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
   if( server )
   {
      server->Register("/StsRaw", fhStsMessType );
      server->Register("/StsRaw", fhStsSysMessType );
      server->Register("/StsRaw", fhStsMessTypePerDpb );
      server->Register("/StsRaw", fhStsSysMessTypePerDpb );
      server->Register("/StsRaw", fhStsMessTypePerElink );
      server->Register("/StsRaw", fhStsSysMessTypePerElink );
      for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
      {
         server->Register("/StsRaw", fhStsChanCntRaw[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanCntRawGood[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanAdcRaw[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanAdcRawProf[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanRawTs[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanMissEvt[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanMissEvtEvo[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsFebMissEvtEvo[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanHitRateEvo[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsFebRateEvo[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsChanHitRateEvoLong[ uXyterIdx ] );
         server->Register("/StsRaw", fhStsFebRateEvoLong[ uXyterIdx ] );

         server->Register("/DtAsic", fhStsTimeDiffPerAsic[ uXyterIdx ] );
         for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
         {
            if( uXyterIdxB == uXyterIdx )
            {
               server->Register("/DtChan", fhStsTimeDiffPerAsicPair[ uXyterIdx ][uXyterIdxB] );
               server->Register("/DtChan", fhStsTimeDiffClkPerAsicPair[ uXyterIdx ][uXyterIdxB] );
               server->Register("/DtChan", fhStsTimeDiffEvoPerAsicPair[ uXyterIdx ][uXyterIdxB] );
               server->Register("/DtChan", fhStsTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB] );
               server->Register("/DtChan", fhStsRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB] );
            } // if( uXyterIdxB == uXyterIdx )
               else
               {
                  server->Register("/DtAsicPair", fhStsTimeDiffPerAsicPair[ uXyterIdx ][uXyterIdxB] );
                  server->Register("/DtAsicPair", fhStsTimeDiffClkPerAsicPair[ uXyterIdx ][uXyterIdxB] );
                  server->Register("/DtAsicPair", fhStsTimeDiffEvoPerAsicPair[ uXyterIdx ][uXyterIdxB] );
                  server->Register("/DtAsicPair", fhStsTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB] );
                  server->Register("/DtAsicPair", fhStsRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB] );
               } // else of if( uXyterIdxB == uXyterIdx )
            server->Register("/TsMatch", fhStsTsLsbMatchPerAsicPair[ uXyterIdx ][uXyterIdxB] );
            server->Register("/TsMatch", fhStsTsMsbMatchPerAsicPair[ uXyterIdx ][uXyterIdxB] );
         } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
         server->Register("/DtPulses", fhStsIntervalAsic[ uXyterIdx ] );
         server->Register("/DtPulses", fhStsIntervalLongAsic[ uXyterIdx ] );

      } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )

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
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
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
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )

//====================================================================//

//====================================================================//
   TCanvas* cStsDtPerAsic = new TCanvas( "cStsDtPerAsic",
                                      "Time Differences per ASIC",
                                    w, h);
   cStsDtPerAsic->Divide( fuStsNbStsXyters / 2 + fuStsNbStsXyters % 2, 2 );

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
   {
      cStsDtPerAsic->cd(1 + uXyterIdx);
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogz();
      fhStsTimeDiffPerAsic[ uXyterIdx ]->Draw( " colz" );
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
//====================================================================//

//====================================================================//
   TCanvas* cStsDtInAsic = new TCanvas( "cStsDtInAsic",
                                      "Time Differences in ASIC",
                                    w, h);
   cStsDtInAsic->Divide( fuStsNbStsXyters / 2 + fuStsNbStsXyters % 2, 2 );

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
   {
      cStsDtInAsic->cd(1 + uXyterIdx);
      gPad->SetGridx();
      gPad->SetLogy();
//      gStyle->SetOptStat("emrou");
      fhStsTimeDiffPerAsicPair[ uXyterIdx ][ uXyterIdx ]->Draw( "hist" );
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
//====================================================================//

//====================================================================//
   TCanvas* cStsDtAsicPairs = new TCanvas( "cStsDtAsicPairs",
                                      "Time Differences in ASIC",
                                    w, h);
   cStsDtAsicPairs->Divide( 2, 3 );
   UInt_t uHistoIdx = 0;
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters && uXyterIdx < 3; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = uXyterIdx + 1; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
      {
         cStsDtAsicPairs->cd( 1 + uHistoIdx );
         gPad->SetGridx();
         gPad->SetLogy();
//         gStyle->SetOptStat("emrou");
         fhStsTimeDiffPerAsicPair[ uXyterIdx ][uXyterIdxB]->Draw( "hist" );

         uHistoIdx++;
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
//====================================================================//

//====================================================================//
   TCanvas* cDtClkAsicPairs = new TCanvas( "cDtClkAsicPairs",
                                      "Time Differences in ASIC",
                                    w, h);
   cDtClkAsicPairs->Divide( fuStsNbStsXyters - 1 );
   for( UInt_t uXyterIdxB = 1; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
   {
      cDtClkAsicPairs->cd( uXyterIdxB );
      gPad->SetGridx();
      gPad->SetLogy();
//         gStyle->SetOptStat("emrou");
      fhStsTimeDiffClkPerAsicPair[ 0 ][uXyterIdxB]->Draw( "hist" );
   } // for( UInt_t uXyterIdxB = 1; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
//====================================================================//

//====================================================================//
   TCanvas* cStsDtAsicPairsEvo = new TCanvas( "cStsDtAsicPairsEvo",
                                      "Time Differences Evo in ASIC",
                                    w, h);
   cStsDtAsicPairsEvo->Divide( 2, 3 );
   uHistoIdx = 0;
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters && uXyterIdx < 3; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = uXyterIdx + 1; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
      {
         cStsDtAsicPairsEvo->cd( 1 + uHistoIdx );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();
         fhStsTimeDiffEvoPerAsicPair[ uXyterIdx ][uXyterIdxB]->Draw( "colz" );

         uHistoIdx++;
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
//====================================================================//

//====================================================================//
   TCanvas* cStsDtAsicPairsEvoProf = new TCanvas( "cStsDtAsicPairsEvoProf",
                                      "Time Differences Evo in ASIC",
                                    w, h);
   cStsDtAsicPairsEvoProf->Divide( 2, 3 );
   uHistoIdx = 0;
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters && uXyterIdx < 3; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = uXyterIdx + 1; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
      {
         cStsDtAsicPairsEvoProf->cd( 1 + uHistoIdx );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();
         fhStsTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Draw( "hist e0" );

         uHistoIdx++;
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
//====================================================================//

//====================================================================//
   TCanvas* cStsDtAsicPairsEvoProfRaw = new TCanvas( "cStsDtAsicPairsEvoProfRaw",
                                      "Time Differences Evo in ASIC",
                                    w, h);
   cStsDtAsicPairsEvoProfRaw->Divide( 2, 3 );
   uHistoIdx = 0;
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters && uXyterIdx < 3; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = uXyterIdx + 1; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
      {
         cStsDtAsicPairsEvoProfRaw->cd( 1 + uHistoIdx );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();
         fhStsRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Draw( "hist e0" );

         uHistoIdx++;
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
//====================================================================//

//====================================================================//
   TCanvas* cStsTsLsbAsicPairs = new TCanvas( "cStsTsLsbAsicPairs",
                                      "Time Differences in ASIC",
                                    w, h);
   cStsTsLsbAsicPairs->Divide( 2, 3 );
   uHistoIdx = 0;
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters && uXyterIdx < 3; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = uXyterIdx + 1; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
      {
         cStsTsLsbAsicPairs->cd( 1 +uHistoIdx );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();
         fhStsTsLsbMatchPerAsicPair[ uXyterIdx ][uXyterIdxB]->Draw( "colz" );

         uHistoIdx++;
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
//====================================================================//

//====================================================================//
   TCanvas* cStsTsMsbAsicPairs = new TCanvas( "cStsTsMsbAsicPairs",
                                      "Time Differences in ASIC",
                                    w, h);

   cStsTsMsbAsicPairs->Divide( 2, 3 );
   uHistoIdx = 0;
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters && uXyterIdx < 3; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = uXyterIdx + 1; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
      {
         cStsTsMsbAsicPairs->cd( 1 + uHistoIdx );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();
         fhStsTsMsbMatchPerAsicPair[ uXyterIdx ][uXyterIdxB]->Draw( "colz" );

         uHistoIdx++;
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
//====================================================================//

  /*****************************/
  LOG(INFO) << "Done Creating STS Histograms" << FairLogger::endl;
}
/***************** STS Histograms *************************************/

/***************** TOF Histograms *************************************/
void CbmMcbm2018MonitorMcbmSync::CreateTofHistograms()
{
   TString sHistName{""};
   TString title{""};

   // Full Fee time difference test
   UInt_t uNbBinsDt = kuTofNbBinsDt + 1; // To account for extra bin due to shift by 1/2 bin of both ranges

   fuTofNbFeePlotsPerGdpb = fuTofNrOfFeePerGdpb/fuTofNbFeePlot + ( 0 != fuTofNrOfFeePerGdpb%fuTofNbFeePlot ? 1 : 0 );
   Double_t dBinSzG4v2 = (6250. / 112.);
   fdTofMinDt     = -1.*(kuTofNbBinsDt*dBinSzG4v2/2.) - dBinSzG4v2/2.;
   fdTofMaxDt     =  1.*(kuTofNbBinsDt*dBinSzG4v2/2.) + dBinSzG4v2/2.;

   /*******************************************************************/
   sHistName = "hMessageType";
   title = "Nb of message for each type; Type";
   // Test Big Data readout with plotting
   fhTofMessType = new TH1I(sHistName, title, 1 + gdpbv100::MSG_STAR_TRI_A, 0., 1 + gdpbv100::MSG_STAR_TRI_A);
   fhTofMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_HIT,        "HIT");
   fhTofMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_EPOCH,      "EPOCH");
   fhTofMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_SLOWC,      "SLOWC");
   fhTofMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_SYST,       "SYST");
   fhTofMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_STAR_TRI_A, "MSG_STAR_TRI");

   /*******************************************************************/
   sHistName = "hSysMessType";
   title = "Nb of system message for each type; System Type";
   fhTofSysMessType = new TH1I(sHistName, title, 1 + gdpbv100::SYS_SYNC_ERROR, 0., 1 + gdpbv100::SYS_SYNC_ERROR);
   fhTofSysMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::SYS_GET4_ERROR,     "GET4 ERROR");
   fhTofSysMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::SYS_GDPB_UNKWN,     "UNKW GET4 MSG");
   fhTofSysMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::SYS_GET4_SYNC_MISS, "SYS_GET4_SYNC_MISS");
   fhTofSysMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::SYS_SYNC_ERROR,     "SYNC ERROR");

   /*******************************************************************/
   sHistName = "hGet4MessType";
   title = "Nb of message for each type per GET4; GET4 chip # ; Type";
   fhTofGet4MessType = new TH2I(sHistName, title, fuTofNrOfGet4, 0., fuTofNrOfGet4, 6, 0., 6.);
   fhTofGet4MessType->GetYaxis()->SetBinLabel( 1, "DATA 32b");
   fhTofGet4MessType->GetYaxis()->SetBinLabel( 2, "EPOCH");
   fhTofGet4MessType->GetYaxis()->SetBinLabel( 3, "S.C. M");
   fhTofGet4MessType->GetYaxis()->SetBinLabel( 4, "ERROR");
   fhTofGet4MessType->GetYaxis()->SetBinLabel( 5, "DATA 24b");
   fhTofGet4MessType->GetYaxis()->SetBinLabel( 6, "STAR Trigger");

   /*******************************************************************/
   sHistName = "hGet4ChanScm";
   title = "SC messages per GET4 channel; GET4 channel # ; SC type";
   fhTofGet4ChanScm =  new TH2I(sHistName, title,
         2 * fuTofNrOfGet4 * fuTofNrOfChannelsPerGet4, 0., fuTofNrOfGet4 * fuTofNrOfChannelsPerGet4,
         5, 0., 5.);
   fhTofGet4ChanScm->GetYaxis()->SetBinLabel( 1, "Hit Scal" );
   fhTofGet4ChanScm->GetYaxis()->SetBinLabel( 2, "Deadtime" );
   fhTofGet4ChanScm->GetYaxis()->SetBinLabel( 3, "SPI" );
   fhTofGet4ChanScm->GetYaxis()->SetBinLabel( 4, "SEU Scal" );
   fhTofGet4ChanScm->GetYaxis()->SetBinLabel( 5, "START" );

   /*******************************************************************/
   sHistName = "hGet4ChanErrors";
   title = "Error messages per GET4 channel; GET4 channel # ; Error";
   fhTofGet4ChanErrors = new TH2I(sHistName, title,
            fuTofNrOfGet4 * fuTofNrOfChannelsPerGet4, 0., fuTofNrOfGet4 * fuTofNrOfChannelsPerGet4,
            21, 0., 21.);
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel( 1, "0x00: Readout Init    ");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel( 2, "0x01: Sync            ");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel( 3, "0x02: Epoch count sync");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel( 4, "0x03: Epoch           ");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel( 5, "0x04: FIFO Write      ");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel( 6, "0x05: Lost event      ");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel( 7, "0x06: Channel state   ");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel( 8, "0x07: Token Ring state");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel( 9, "0x08: Token           ");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel(10, "0x09: Error Readout   ");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel(11, "0x0a: SPI             ");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel(12, "0x0b: DLL Lock error  "); // <- From GET4 v1.2
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel(13, "0x0c: DLL Reset invoc."); // <- From GET4 v1.2
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel(14, "0x11: Overwrite       ");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel(15, "0x12: ToT out of range");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel(16, "0x13: Event Discarded ");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel(17, "0x14: Add. Rising edge"); // <- From GET4 v1.3
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel(18, "0x15: Unpaired Falling"); // <- From GET4 v1.3
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel(19, "0x16: Sequence error  "); // <- From GET4 v1.3
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel(20, "0x7f: Unknown         ");
   fhTofGet4ChanErrors->GetYaxis()->SetBinLabel(21, "Corrupt/unsuprtd error");

   /*******************************************************************/
   sHistName = "hGet4EpochFlags";
   title = "Epoch flags per GET4; GET4 chip # ; Type";
   fhTofGet4EpochFlags = new TH2I(sHistName, title, fuTofNrOfGet4, 0., fuTofNrOfGet4, 4, 0., 4.);
   fhTofGet4EpochFlags->GetYaxis()->SetBinLabel(1, "SYNC");
   fhTofGet4EpochFlags->GetYaxis()->SetBinLabel(2, "Ep LOSS");
   fhTofGet4EpochFlags->GetYaxis()->SetBinLabel(3, "Da LOSS");
   fhTofGet4EpochFlags->GetYaxis()->SetBinLabel(4, "MISSMAT");

   /*******************************************************************/
      // Slow control messages analysis
   sHistName = "hScmScalerCounters";
   title = "Content of Scaler counter SC messages; Scaler counter [hit]; Channel []";
   fhTofScmScalerCounters = new TH2I(sHistName, title, fuTofNrOfGet4 * fuTofNrOfChannelsPerGet4 * 2, 0., fuTofNrOfGet4 * fuTofNrOfChannelsPerGet4,
                                               8192, 0., 8192.);

   sHistName = "hScmDeadtimeCounters";
   title = "Content of Deadtime counter SC messages; Deadtime [Clk Cycles]; Channel []";
   fhTofScmDeadtimeCounters = new TH2I(sHistName, title, fuTofNrOfGet4 * fuTofNrOfChannelsPerGet4 * 2, 0., fuTofNrOfGet4 * fuTofNrOfChannelsPerGet4,
                                               8192, 0., 8192.);

   sHistName = "hScmSeuCounters";
   title = "Content of SEU counter SC messages; SEU []; Channel []";
   fhTofScmSeuCounters = new TH2I(sHistName, title, fuTofNrOfGet4 * fuTofNrOfChannelsPerGet4 * 2, 0., fuTofNrOfGet4 * fuTofNrOfChannelsPerGet4,
                                               8192, 0., 8192.);

   sHistName = "hScmSeuCountersEvo";
   title = "SEU counter rate from SC messages; Time in Run [s]; Channel []; SEU []";
   fhTofScmSeuCountersEvo = new TH2I(sHistName, title, fuTofNrOfGet4 * fuTofNrOfChannelsPerGet4 * 2, 0., fuTofNrOfGet4 * fuTofNrOfChannelsPerGet4,
                                               fuTofHistoryHistoSize, 0., fuTofHistoryHistoSize);

   /*******************************************************************/
  for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
  {
      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
      sHistName = Form("RawFt_gDPB_%02u", uGdpb);
      title = Form("Raw FineTime gDPB %02u Plot 0; channel; FineTime [bin]", uGdpb);
      fvhTofRawFt_gDPB.push_back(
         new TH2F(sHistName.Data(), title.Data(),
            fuTofNrOfChannelsPerGdpb, 0, fuTofNrOfChannelsPerGdpb,
            128, 0, 128 ) );

      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
/*
      sHistName = Form("RawTot_gDPB_%02u_0", uGdpb);
      title = Form("Raw TOT gDPB %02u Plot 0; channel; TOT [bin]", uGdpb);
      fvhTofRawTot_gDPB.push_back(
         new TH2F(sHistName.Data(), title.Data(),
            fuTofNbFeePlot*fuTofNrOfChannelsPerFee, 0*fuTofNbFeePlot*fuTofNrOfChannelsPerFee, 1*fuTofNbFeePlot*fuTofNrOfChannelsPerFee,
            256, 0, 256 ) );

      if( fuTofNbFeePlot < fuTofNrOfFeePerGdpb )
      {
         sHistName = Form("RawTot_gDPB_%02u_1", uGdpb);
         title = Form("Raw TOT gDPB %02u Plot 1; channel; TOT [bin]", uGdpb);
         fvhTofRawTot_gDPB.push_back(
            new TH2F(sHistName.Data(), title.Data(),
               fuTofNbFeePlot*fuTofNrOfChannelsPerFee, 1*fuTofNbFeePlot*fuTofNrOfChannelsPerFee, 2*fuTofNbFeePlot*fuTofNrOfChannelsPerFee,
               256, 0, 256));
      } // if( fuTofNbFeePlot < fuTofNrOfFeePerGdpb )
      if( 2 * fuTofNbFeePlot < fuTofNrOfFeePerGdpb )
      {
         sHistName = Form("RawTot_gDPB_%02u_2", uGdpb);
         title = Form("Raw TOT gDPB %02u Plot 2; channel; TOT [bin]", uGdpb);
         fvhTofRawTot_gDPB.push_back(
            new TH2F(sHistName.Data(), title.Data(),
               fuTofNbFeePlot*fuTofNrOfChannelsPerFee, 2*fuTofNbFeePlot*fuTofNrOfChannelsPerFee, 3*fuTofNbFeePlot*fuTofNrOfChannelsPerFee,
               256, 0, 256));
      } // if( 2 * fuTofNbFeePlot < fuTofNrOfFeePerGdpb )
*/
      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
       sHistName = Form("ChCount_gDPB_%02u", uGdpb);
       title = Form("Channel counts gDPB %02u; channel; Hits", uGdpb);
       fvhTofChCount_gDPB.push_back( new TH1I(sHistName.Data(), title.Data(),
                fuTofNrOfFeePerGdpb * fuTofNrOfChannelsPerFee, 0, fuTofNrOfFeePerGdpb * fuTofNrOfChannelsPerFee) );

      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
       sHistName = Form("ChRate_gDPB_%02u", uGdpb);
       title = Form("Channel rate gDPB %02u; Time in run [s]; channel; Rate [1/s]", uGdpb);
       fvhTofChannelRate_gDPB.push_back( new TH2D(sHistName.Data(), title.Data(),
                fuTofHistoryHistoSize, 0, fuTofHistoryHistoSize,
                fuTofNrOfFeePerGdpb * fuTofNrOfChannelsPerFee, 0, fuTofNrOfFeePerGdpb * fuTofNrOfChannelsPerFee ) );

      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
/*
      sHistName = Form("RemapTot_gDPB_%02u_0", uGdpb);
      title = Form("Raw TOT gDPB %02u remapped Plot 0; PADI channel; TOT [bin]", uGdpb);
      fvhTofRemapTot_gDPB.push_back(
         new TH2F(sHistName.Data(), title.Data(),
            fuTofNbFeePlot*fuTofNrOfChannelsPerFee, 0*fuTofNbFeePlot*fuTofNrOfChannelsPerFee, 1*fuTofNbFeePlot*fuTofNrOfChannelsPerFee,
            256, 0, 256 ) );

      if( fuTofNbFeePlot < fuTofNrOfFeePerGdpb )
      {
         sHistName = Form("RemapTot_gDPB_%02u_1", uGdpb);
         title = Form("Raw TOT gDPB %02u remapped Plot 1; PADI channel; TOT [bin]", uGdpb);
         fvhTofRemapTot_gDPB.push_back(
            new TH2F(sHistName.Data(), title.Data(),
               fuTofNbFeePlot*fuTofNrOfChannelsPerFee, 1*fuTofNbFeePlot*fuTofNrOfChannelsPerFee, 2*fuTofNbFeePlot*fuTofNrOfChannelsPerFee,
               256, 0, 256));
      } // if( fuTofNbFeePlot < fuTofNrOfFeePerGdpb )
      if( 2 * fuTofNbFeePlot < fuTofNrOfFeePerGdpb )
      {
         sHistName = Form("RemapTot_gDPB_%02u_2", uGdpb);
         title = Form("Raw TOT gDPB %02u remapped Plot 2; PADI channel; TOT [bin]", uGdpb);
         fvhTofRemapTot_gDPB.push_back(
            new TH2F(sHistName.Data(), title.Data(),
               fuTofNbFeePlot*fuTofNrOfChannelsPerFee, 2*fuTofNbFeePlot*fuTofNrOfChannelsPerFee, 3*fuTofNbFeePlot*fuTofNrOfChannelsPerFee,
               256, 0, 256));
      } // if( 2 * fuTofNbFeePlot < fuTofNrOfFeePerGdpb )
*/
      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
       sHistName = Form("RemapChCount_gDPB_%02u", uGdpb);
       title = Form("Channel counts gDPB %02u remapped; PADI channel; Hits", uGdpb);
       fvhTofRemapChCount_gDPB.push_back( new TH1I(sHistName.Data(), title.Data(),
                fuTofNrOfFeePerGdpb * fuTofNrOfChannelsPerFee, 0, fuTofNrOfFeePerGdpb * fuTofNrOfChannelsPerFee) );

      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
       sHistName = Form("RemapChRate_gDPB_%02u", uGdpb);
       title = Form("PADI channel rate gDPB %02u; Time in run [s]; PADI channel; Rate [1/s]", uGdpb);
       fvhTofRemapChRate_gDPB.push_back( new TH2D(sHistName.Data(), title.Data(),
                fuTofHistoryHistoSize, 0, fuTofHistoryHistoSize,
                fuTofNrOfFeePerGdpb * fuTofNrOfChannelsPerFee, 0, fuTofNrOfFeePerGdpb * fuTofNrOfChannelsPerFee ) );

      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
      for (UInt_t uFee = 0; uFee < fuTofNrOfFeePerGdpb; ++ uFee)
      {
         sHistName = Form("FeeRate_gDPB_g%02u_f%1u", uGdpb, uFee);
         title = Form(
             "Counts per second in Fee %1u of gDPB %02u; Time[s] ; Counts", uFee,
             uGdpb);
         fvhTofFeeRate_gDPB.push_back( new TH1D(sHistName.Data(), title.Data(), fuTofHistoryHistoSize, 0, fuTofHistoryHistoSize) );

         sHistName = Form("FeeErrorRate_gDPB_g%02u_f%1u", uGdpb, uFee);
         title = Form(
             "Error Counts per second in Fee %1u of gDPB %02u; Time[s] ; Error Counts", uFee,
             uGdpb);
         fvhTofFeeErrorRate_gDPB.push_back( new TH1D(sHistName.Data(), title.Data(), fuTofHistoryHistoSize, 0, fuTofHistoryHistoSize) );

         sHistName = Form("FeeErrorRatio_gDPB_g%02u_f%1u", uGdpb, uFee);
         title = Form(
             "Error to data ratio per second in Fee %1u of gDPB %02u; Time[s] ; Error ratio[]", uFee,
             uGdpb);
         fvhTofFeeErrorRatio_gDPB.push_back( new TProfile(sHistName.Data(), title.Data(), fuTofHistoryHistoSize, 0, fuTofHistoryHistoSize) );

         sHistName = Form("FeeRateLong_gDPB_g%02u_f%1u", uGdpb, uFee);
         title = Form(
             "Counts per minutes in Fee %1u of gDPB %02u; Time[min] ; Counts", uFee,
             uGdpb);
         fvhTofFeeRateLong_gDPB.push_back( new TH1D(sHistName.Data(), title.Data(), fuTofHistoryHistoSizeLong, 0, fuTofHistoryHistoSizeLong) );

         sHistName = Form("FeeErrorRateLong_gDPB_g%02u_f%1u", uGdpb, uFee);
         title = Form(
             "Error Counts per minutes in Fee %1u of gDPB %02u; Time[min] ; Error Counts", uFee,
             uGdpb);
         fvhTofFeeErrorRateLong_gDPB.push_back( new TH1D(sHistName.Data(), title.Data(), fuTofHistoryHistoSizeLong, 0, fuTofHistoryHistoSizeLong) );

         sHistName = Form("FeeErrorRatioLong_gDPB_g%02u_f%1u", uGdpb, uFee);
         title = Form(
             "Error to data ratio per minutes in Fee %1u of gDPB %02u; Time[min] ; Error ratio[]", uFee,
             uGdpb);
         fvhTofFeeErrorRatioLong_gDPB.push_back( new TProfile(sHistName.Data(), title.Data(), fuTofHistoryHistoSizeLong, 0, fuTofHistoryHistoSizeLong) );
      } // for (UInt_t uFee = 0; uFee < fuTofNrOfFeePerGdpb; uFee++)

      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
      /// STAR Trigger decoding and monitoring
      sHistName = Form( "hTokenMsgType_gDPB_%02u", uGdpb);
      title = Form( "STAR trigger Messages type gDPB %02u; Type ; Counts", uGdpb);
      fvhTofTokenMsgType.push_back(  new TH1F(sHistName, title, 4, 0, 4) );
      fvhTofTokenMsgType[ uGdpb ]->GetXaxis()->SetBinLabel( 1, "A"); // gDPB TS high
      fvhTofTokenMsgType[ uGdpb ]->GetXaxis()->SetBinLabel( 2, "B"); // gDPB TS low, STAR TS high
      fvhTofTokenMsgType[ uGdpb ]->GetXaxis()->SetBinLabel( 3, "C"); // STAR TS mid
      fvhTofTokenMsgType[ uGdpb ]->GetXaxis()->SetBinLabel( 4, "D"); // STAR TS low, token, CMDs

      sHistName = Form( "hTriggerRate_gDPB_%02u", uGdpb);
      title = Form( "STAR trigger signals per second gDPB %02u; Time[s] ; Counts", uGdpb);
      fvhTofTriggerRate.push_back(  new TH1F(sHistName, title, fuTofHistoryHistoSize, 0, fuTofHistoryHistoSize) );

      sHistName = Form( "hCmdDaqVsTrig_gDPB_%02u", uGdpb);
      title = Form( "STAR daq command VS STAR trigger command gDPB %02u; DAQ ; TRIGGER", uGdpb);
      fvhTofCmdDaqVsTrig.push_back( new TH2I(sHistName, title, 16, 0, 16, 16, 0, 16 ) );
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 1, "0x0: no-trig "); // idle link
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 2, "0x1: clear   "); // clears redundancy counters on the readout boards
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 3, "0x2: mast-rst"); // general reset of the whole front-end logic
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 4, "0x3: spare   "); // reserved
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 5, "0x4: trigg. 0"); // Default physics readout, all det support required
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 6, "0x5: trigg. 1"); //
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 7, "0x6: trigg. 2"); //
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 8, "0x7: trigg. 3"); //
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 9, "0x8: puls.  0"); //
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(10, "0x9: puls.  1"); //
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(11, "0xA: puls.  2"); //
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(12, "0xB: puls.  3"); //
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(13, "0xC: config  "); // housekeeping trigger: return geographic info of FE
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(14, "0xD: abort   "); // aborts and clears an active event
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(15, "0xE: L1accept"); //
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(16, "0xF: L2accept"); //
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 1, "0x0:  0"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 2, "0x1:  1"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 3, "0x2:  2"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 4, "0x3:  3"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 5, "0x4:  4"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 6, "0x5:  5"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 7, "0x6:  6"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 8, "0x7:  7"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 9, "0x8:  8"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(10, "0x9:  9"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(11, "0xA: 10"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(12, "0xB: 11"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(13, "0xC: 12"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(14, "0xD: 13"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(15, "0xE: 14"); // To be filled at STAR
      fvhTofCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(16, "0xF: 15"); // To be filled at STAR

      sHistName = Form( "hStarTokenEvo_gDPB_%02u", uGdpb);
      title = Form( "STAR token value VS time gDPB %02u; Time in Run [s] ; STAR Token; Counts", uGdpb);
      fvhTofStarTokenEvo.push_back( new TH2I(sHistName, title, fuTofHistoryHistoSize, 0, fuTofHistoryHistoSize, 410, 0, 4100 ) );


      sHistName = Form( "hStarTrigGdpbTsEvo_gDPB_%02u", uGdpb);
      title = Form( "gDPB TS in STAR triger tokens for gDPB %02u; Time in Run [s] ; gDPB TS;", uGdpb);
      fvhTofStarTrigGdpbTsEvo.push_back( new TProfile(sHistName, title, fuTofHistoryHistoSize, 0, fuTofHistoryHistoSize ) );

      sHistName = Form( "hStarTrigStarTsEvo_gDPB_%02u", uGdpb);
      title = Form( "STAR TS in STAR triger tokens for gDPB %02u; Time in Run [s] ; STAR TS;", uGdpb);
      fvhTofStarTrigStarTsEvo.push_back( new TProfile(sHistName, title, fuTofHistoryHistoSize, 0, fuTofHistoryHistoSize ) );
   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )

   /*******************************************************************/
   /// FEET pulser test channels
   fvhTofTimeDiffPulser.resize( fuTofNrOfFee );
   for( UInt_t uFeeA = 0; uFeeA < fuTofNrOfFee; uFeeA++)
   {
      fvhTofTimeDiffPulser[uFeeA].resize( fuTofNrOfFee );
      for( UInt_t uFeeB = 0; uFeeB < fuTofNrOfFee; uFeeB++)
      {
         if( uFeeA < uFeeB )
         {
            UInt_t uGdpbA  = uFeeA / ( fuTofNrOfFeePerGdpb );
            UInt_t uFeeIdA = uFeeA - ( fuTofNrOfFeePerGdpb * uGdpbA );
            UInt_t uGdpbB  = uFeeB / ( fuTofNrOfFeePerGdpb );
            UInt_t uFeeIdB = uFeeB - ( fuTofNrOfFeePerGdpb * uGdpbB );
            fvhTofTimeDiffPulser[uFeeA][uFeeB] = new TH1I(
               Form("hTimeDiffPulser_g%02u_f%1u_g%02u_f%1u", uGdpbA, uFeeIdA, uGdpbB, uFeeIdB),
               Form("Time difference for pulser on gDPB %02u FEE %1u and gDPB %02u FEE %1u; DeltaT [ps]; Counts",
                     uGdpbA, uFeeIdA, uGdpbB, uFeeIdB ),
               uNbBinsDt, fdTofMinDt, fdTofMaxDt);
         } // if( uFeeA < uFeeB )
            else fvhTofTimeDiffPulser[uFeeA][uFeeB] = nullptr;
      } // for( UInt_t uFeeB = uFeeA; uFeeB < fuTofNrOfFee - 1; uFeeB++)
   } // for( UInt_t uFeeA = 0; uFeeA < kuNbChanTest - 1; uFeeA++)

   sHistName = "hTimeRmsPulser";
   fhTofTimeRmsPulser = new TH2D( sHistName.Data(),
         "Time difference RMS for each FEE pairs; FEE A; FEE B ; [ps]",
         fuTofNrOfFee - 1, -0.5, fuTofNrOfFee - 1.5,
         fuTofNrOfFee - 1,  0.5, fuTofNrOfFee - 0.5 );

   sHistName = "hTimeRmsZoomFitPuls";
   fhTofTimeRmsZoomFitPuls = new TH2D( sHistName.Data(),
         "Time difference RMS after zoom for each FEE pairs; FEE A; FEE B ; RMS [ps]",
         fuTofNrOfFee - 1, -0.5, fuTofNrOfFee - 1.5,
         fuTofNrOfFee - 1,  0.5, fuTofNrOfFee - 0.5 );

   sHistName = "hTimeResFitPuls";
   fhTofTimeResFitPuls = new TH2D( sHistName.Data(),
         "Time difference Res from fit for each FEE pairs; FEE A; FEE B ; Sigma [ps]",
         fuTofNrOfFee - 1, -0.5, fuTofNrOfFee - 1.5,
         fuTofNrOfFee - 1,  0.5, fuTofNrOfFee - 0.5 );

#ifdef USE_HTTP_SERVER
   THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
   if( server )
   {
      server->Register("/TofRaw", fhTofMessType );
      server->Register("/TofRaw", fhTofSysMessType );
      server->Register("/TofRaw", fhTofGet4MessType );
      server->Register("/TofRaw", fhTofGet4ChanScm );
      server->Register("/TofRaw", fhTofGet4ChanErrors );
      server->Register("/TofRaw", fhTofGet4EpochFlags );
      server->Register("/TofRaw", fhTofScmScalerCounters );
      server->Register("/TofRaw", fhTofScmDeadtimeCounters );
      server->Register("/TofRaw", fhTofScmSeuCounters );
      server->Register("/TofRaw", fhTofScmSeuCountersEvo );
/*
      for( UInt_t uTotPlot = 0; uTotPlot < fvhTofRawTot_gDPB.size(); ++uTotPlot )
         server->Register("/TofRaw", fvhTofRawTot_gDPB[ uTotPlot ] );

      for( UInt_t uTotPlot = 0; uTotPlot < fvhTofRemapTot_gDPB.size(); ++uTotPlot )
         server->Register("/TofRaw", fvhTofRemapTot_gDPB[ uTotPlot ] );
*/
      for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
      {
         server->Register("/TofRaw", fvhTofRawFt_gDPB[ uGdpb ] );
         server->Register("/TofRaw", fvhTofChCount_gDPB[ uGdpb ] );
         server->Register("/TofRates", fvhTofChannelRate_gDPB[ uGdpb ] );
         server->Register("/TofRaw", fvhTofRemapChCount_gDPB[ uGdpb ] );
         server->Register("/TofRates", fvhTofRemapChRate_gDPB[ uGdpb ] );

         for (UInt_t uFee = 0; uFee < fuTofNrOfFeePerGdpb; ++ uFee)
         {
            server->Register("/TofRates", fvhTofFeeRate_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFee ] );
            server->Register("/TofRates", fvhTofFeeErrorRate_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFee ] );
            server->Register("/TofRates", fvhTofFeeErrorRatio_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFee ] );
            server->Register("/TofRates", fvhTofFeeRateLong_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFee ] );
            server->Register("/TofRates", fvhTofFeeErrorRateLong_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFee ] );
            server->Register("/TofRates", fvhTofFeeErrorRatioLong_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFee ] );
         } // for (UInt_t uFee = 0; uFee < fuTofNrOfFeePerGdpb; ++ uFee)

         server->Register("/StarRaw", fvhTofTokenMsgType[ uGdpb ] );
         server->Register("/StarRaw", fvhTofTriggerRate[ uGdpb ] );
         server->Register("/StarRaw", fvhTofCmdDaqVsTrig[ uGdpb ] );
         server->Register("/StarRaw", fvhTofStarTokenEvo[ uGdpb ] );
         server->Register("/StarRaw", fvhTofStarTrigGdpbTsEvo[ uGdpb ] );
         server->Register("/StarRaw", fvhTofStarTrigStarTsEvo[ uGdpb ] );
      } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )

      for( UInt_t uFeeA = 0; uFeeA < fuTofNrOfFee; uFeeA++)
         for( UInt_t uFeeB = 0; uFeeB < fuTofNrOfFee; uFeeB++)
            if( nullptr != fvhTofTimeDiffPulser[uFeeA][uFeeB] )
               server->Register("/TofDt", fvhTofTimeDiffPulser[uFeeA][uFeeB] );

      server->Register("/TofRaw", fhTofTimeRmsPulser );
      server->Register("/TofRaw", fhTofTimeRmsZoomFitPuls );
      server->Register("/TofRaw", fhTofTimeResFitPuls );

      server->RegisterCommand("/Reset_All_TOF",  "bMcbm2018SyncResetHistosTof=kTRUE");
      server->RegisterCommand("/Save_All_Tof",   "bMcbm2018SyncSaveHistosTof=kTRUE");
      server->RegisterCommand("/Update_PulsFit", "bMcbm2018SyncUpdateZoomedFit=kTRUE");

      server->Restrict("/Reset_All_Tof", "allow=admin");
      server->Restrict("/Save_All_Tof",  "allow=admin");
      server->Restrict("/Update_PulsFit", "allow=admin");
   } // if( server )
#endif

   /** Create summary Canvases for STAR 2018 **/
   Double_t w = 10;
   Double_t h = 10;
   TCanvas* cSummary = new TCanvas("cSummary", "gDPB Monitoring Summary", w, h);
   cSummary->Divide(2, 3);

   // 1st Column: Messages types
   cSummary->cd(1);
   gPad->SetLogy();
   fhTofMessType->Draw();

   cSummary->cd(2);
   gPad->SetLogy();
   fhTofSysMessType->Draw();

   cSummary->cd(3);
   gPad->SetLogz();
   fhTofGet4MessType->Draw("colz");

   // 2nd Column: GET4 Errors + Epoch flags + SCm
   cSummary->cd(4);
   gPad->SetLogz();
   fhTofGet4ChanErrors->Draw("colz");

   cSummary->cd(5);
   gPad->SetLogz();
   fhTofGet4EpochFlags->Draw("colz");

   cSummary->cd(6);
   fhTofGet4ChanScm->Draw("colz");
   /*****************************/

   /** Create FEET rates Canvas for STAR 2018 **/
   TCanvas* cFeeRates = new TCanvas("cFeeRates", "gDPB Monitoring FEET rates", w, h);
   cFeeRates->Divide(fuTofNrOfFeePerGdpb, fuTofNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      for (UInt_t uFee = 0; uFee < fuTofNrOfFeePerGdpb; ++uFee )
      {
         cFeeRates->cd( 1 + uGdpb * fuTofNrOfFeePerGdpb + uFee );
         gPad->SetLogy();
         fvhTofFeeRate_gDPB[uGdpb * fuTofNrOfFeePerGdpb + uFee]->Draw("hist");

         fvhTofFeeErrorRate_gDPB[uGdpb * fuTofNrOfFeePerGdpb + uFee]->SetLineColor( kRed );
         fvhTofFeeErrorRate_gDPB[uGdpb * fuTofNrOfFeePerGdpb + uFee]->Draw("same hist");
      } // for (UInt_t uFee = 0; uFee < fuTofNrOfFeePerGdpb; ++uFee )
   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   /*****************************/

   /** Create FEET error ratio Canvas for STAR 2018 **/
   TCanvas* cFeeErrRatio = new TCanvas("cFeeErrRatio", "gDPB Monitoring FEET error ratios", w, h);
   cFeeErrRatio->Divide(fuTofNrOfFeePerGdpb, fuTofNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      for (UInt_t uFee = 0; uFee < fuTofNrOfFeePerGdpb; ++uFee )
      {
         cFeeErrRatio->cd( 1 + uGdpb * fuTofNrOfFeePerGdpb + uFee );
         gPad->SetLogy();
         fvhTofFeeErrorRatio_gDPB[uGdpb * fuTofNrOfFeePerGdpb + uFee]->Draw( "hist le0");
      } // for (UInt_t uFee = 0; uFee < fuTofNrOfFeePerGdpb; ++uFee )
   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   /*****************************/


   /** Create FEET rates long Canvas for STAR 2018 **/
   TCanvas* cFeeRatesLong = new TCanvas("cFeeRatesLong", "gDPB Monitoring FEET rates", w, h);
   cFeeRatesLong->Divide(fuTofNrOfFeePerGdpb, fuTofNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      for (UInt_t uFee = 0; uFee < fuTofNrOfFeePerGdpb; ++uFee )
      {
         cFeeRatesLong->cd( 1 + uGdpb * fuTofNrOfFeePerGdpb + uFee );
         gPad->SetLogy();
         fvhTofFeeRateLong_gDPB[uGdpb]->Draw( "hist" );

         fvhTofFeeErrorRateLong_gDPB[uGdpb * fuTofNrOfFeePerGdpb + uFee]->SetLineColor( kRed );
         fvhTofFeeErrorRateLong_gDPB[uGdpb * fuTofNrOfFeePerGdpb + uFee]->Draw("same hist");
      } // for (UInt_t uFee = 0; uFee < fuTofNrOfFeePerGdpb; ++uFee )
   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   /*****************************/

   /** Create FEET error ratio long Canvas for STAR 2018 **/
   TCanvas* cFeeErrRatioLong = new TCanvas("cFeeErrRatioLong", "gDPB Monitoring FEET error ratios", w, h);
   cFeeErrRatioLong->Divide(fuTofNrOfFeePerGdpb, fuTofNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      for (UInt_t uFee = 0; uFee < fuTofNrOfFeePerGdpb; ++uFee )
      {
         cFeeErrRatioLong->cd( 1 + uGdpb * fuTofNrOfFeePerGdpb + uFee );
         gPad->SetLogy();
         fvhTofFeeErrorRatioLong_gDPB[uGdpb * fuTofNrOfFeePerGdpb + uFee]->Draw( "hist le0");
      } // for (UInt_t uFee = 0; uFee < fuTofNrOfFeePerGdpb; ++uFee )
   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   /*****************************/

   /** Create channel count Canvas for STAR 2018 **/
   TCanvas* cGdpbChannelCount = new TCanvas("cGdpbChannelCount", "Integrated Get4 channel counts per gDPB", w, h);
   cGdpbChannelCount->Divide( 1, fuTofNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      cGdpbChannelCount->cd( 1 + uGdpb );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogy();
      fvhTofChCount_gDPB[ uGdpb ]->Draw();
   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   /*****************************/

   /** Create remapped channel count Canvas for STAR 2018 **/
   TCanvas* cGdpbRemapChCount = new TCanvas("cGdpbRemapChCount", "Integrated PADI channel counts per gDPB", w, h);
   cGdpbRemapChCount->Divide( 1, fuTofNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      cGdpbRemapChCount->cd( 1 + uGdpb );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogy();
      fvhTofRemapChCount_gDPB[ uGdpb ]->Draw();
   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   /*****************************/

   /** Create channel rate Canvas for STAR 2018 **/
   TCanvas* cGdpbChannelRate = new TCanvas("cGdpbChannelRate", "Get4 channel rate per gDPB", w, h);
   cGdpbChannelRate->Divide( 1, fuTofNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      cGdpbChannelRate->cd( 1 + uGdpb );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogz();
      fvhTofChannelRate_gDPB[ uGdpb ]->Draw( "colz" );
   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   /*****************************/

   /** Create remapped rate count Canvas for STAR 2018 **/
   TCanvas* cGdpbRemapChRate = new TCanvas("cGdpbRemapChRate", "PADI channel rate per gDPB", w, h);
   cGdpbRemapChRate->Divide( 1, fuTofNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      cGdpbRemapChRate->cd( 1 + uGdpb );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogz();
      fvhTofRemapChRate_gDPB[ uGdpb ]->Draw( "colz" );
   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   /*****************************/

   /** Create TOT Canvas(es) for STAR 2018 **/
/*
   TCanvas* cTotPnt = nullptr;
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      cTotPnt = new TCanvas( Form("cTot_g%02u", uGdpb),
                             Form("gDPB %02u TOT distributions", uGdpb),
                             w, h);
      cTotPnt->Divide( fuTofNbFeePlotsPerGdpb );

      for( UInt_t uFeePlot = 0; uFeePlot < fuTofNbFeePlotsPerGdpb; ++uFeePlot )
      {
         cTotPnt->cd( 1 + uFeePlot );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();

         fvhTofRawTot_gDPB[ uGdpb * fuTofNbFeePlotsPerGdpb + uFeePlot ]->Draw( "colz" );
      } // for (UInt_t uFee = 0; uFee < fuTofNbFeePlotsPerGdpb; ++uFee )
   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   cTotPnt  = new TCanvas( "cTot_all", "TOT distributions", w, h);
   cTotPnt->Divide( fuTofNrOfGdpbs, fuTofNbFeePlotsPerGdpb );
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
      for( UInt_t uFeePlot = 0; uFeePlot < fuTofNbFeePlotsPerGdpb; ++uFeePlot )
      {
         cTotPnt->cd( 1 + uGdpb + fuTofNrOfGdpbs * uFeePlot );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();

         fvhTofRawTot_gDPB[ uGdpb * fuTofNbFeePlotsPerGdpb + uFeePlot]->Draw( "colz" );
      } // for (UInt_t uFee = 0; uFee < fuTofNbFeePlotsPerGdpb; ++uFee )
*/
   /**************************************************/

   /** Create PADI TOT Canvas(es) for STAR 2018 **/
/*
   cTotPnt = nullptr;
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      cTotPnt = new TCanvas( Form("cTotRemap_g%02u", uGdpb),
                             Form("PADI ch gDPB %02u TOT distributions", uGdpb),
                             w, h);
      cTotPnt->Divide( fuTofNbFeePlotsPerGdpb );

      for( UInt_t uFeePlot = 0; uFeePlot < fuTofNbFeePlotsPerGdpb; ++uFeePlot )
      {
         cTotPnt->cd( 1 + uFeePlot );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();

         fvhTofRemapTot_gDPB[ uGdpb * fuTofNbFeePlotsPerGdpb + uFeePlot ]->Draw( "colz" );
      } // for (UInt_t uFee = 0; uFee < fuTofNbFeePlotsPerGdpb; ++uFee )
   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   cTotPnt  = new TCanvas( "cTotRemap_all", "TOT distributions", w, h);
   cTotPnt->Divide( fuTofNrOfGdpbs, fuTofNbFeePlotsPerGdpb );
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
      for( UInt_t uFeePlot = 0; uFeePlot < fuTofNbFeePlotsPerGdpb; ++uFeePlot )
      {
         cTotPnt->cd( 1 + uGdpb + fuTofNrOfGdpbs * uFeePlot );
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();

         fvhTofRemapTot_gDPB[ uGdpb * fuTofNbFeePlotsPerGdpb + uFeePlot]->Draw( "colz" );
      } // for (UInt_t uFee = 0; uFee < fuTofNbFeePlotsPerGdpb; ++uFee )
*/
   /**************************************************/

   /** Create STAR token Canvas for STAR 2018 **/
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; uGdpb ++)
   {
      TCanvas* cStarToken = new TCanvas( Form("cStarToken_g%02u", uGdpb),
                                           Form("STAR token detection info for gDPB %02u", uGdpb),
                                           w, h);
      cStarToken->Divide( 2, 2 );

      cStarToken->cd(1);
      fvhTofTriggerRate[uGdpb]->Draw();

      cStarToken->cd(2);
      fvhTofCmdDaqVsTrig[uGdpb]->Draw( "colz" );

      cStarToken->cd(3);
      fvhTofStarTokenEvo[uGdpb]->Draw();

      cStarToken->cd(4);
      fvhTofStarTrigGdpbTsEvo[uGdpb]->Draw( "hist le0" );
      fvhTofStarTrigStarTsEvo[uGdpb]->SetLineColor( kRed );
      fvhTofStarTrigStarTsEvo[uGdpb]->Draw( "same hist le0" );
   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; uGdpb ++)
   /*****************************/

   /** Create Pulser check Canvas for STAR 2018 **/
   TCanvas* cPulser = new TCanvas("cPulser", "Time difference RMS for pulser channels when FEE pulser mode is ON", w, h);
   cPulser->Divide( 3 );

   cPulser->cd(1);
   gPad->SetGridx();
   gPad->SetGridy();
   fhTofTimeRmsPulser->Draw( "colz" );

   cPulser->cd(2);
   gPad->SetGridx();
   gPad->SetGridy();
   fhTofTimeRmsZoomFitPuls->Draw( "colz" );

   cPulser->cd(3);
   gPad->SetGridx();
   gPad->SetGridy();
   fhTofTimeResFitPuls->Draw( "colz" );
   /*****************************/
  LOG(INFO) << "Done Creating TOF Histograms" << FairLogger::endl;
}
/***************** TOF Histograms *************************************/

/***************** mCBM Histograms ************************************/
void CbmMcbm2018MonitorMcbmSync::CreateMcbmHistograms()
{
   TString sHistName{""};
   TString sHistTitle{""};

   // Full Fee time difference test
   UInt_t uNbBinsDt = kuTofNbBinsDt + 1; // To account for extra bin due to shift by 1/2 bin of both ranges

   fuTofNbFeePlotsPerGdpb = fuTofNrOfFeePerGdpb/fuTofNbFeePlot + ( 0 != fuTofNrOfFeePerGdpb%fuTofNbFeePlot ? 1 : 0 );
   Double_t dBinSzG4v2 = (6250. / 112.);
   fdTofMinDt     = -1.*(kuTofNbBinsDt*dBinSzG4v2/2.) - dBinSzG4v2/2.;
   fdTofMaxDt     =  1.*(kuTofNbBinsDt*dBinSzG4v2/2.) + dBinSzG4v2/2.;

   /*******************************************************************/
   for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)
   {
      for( UInt_t uFee = 0; uFee < fuTofNrOfFee; uFee++)
      {
         sHistName  = Form( "hMcbmTimeDiffStsTof_%03u_%03u", uAsic, uFee);
         sHistTitle = Form( "Time difference between STS ASIC %03u and TOF FEE %03u; tTOF - tSTS [ns]; Counts", uAsic, uFee);
         fvhMcbmTimeDiffStsTof[uAsic][uFee] = new TH1D( sHistName, sHistTitle,
                                                        uNbBinsDt, fdTofMinDt, fdTofMaxDt);

         sHistName  = Form( "hMcbmTimeDiffStsTofEvo_%03u_%03u", uAsic, uFee);
         sHistTitle = Form( "Evolution of time difference between STS ASIC %03u and TOF FEE %03u; Time in run [s]; tTOF - tSTS [ns]", uAsic, uFee);
         fvhMcbmTimeDiffStsTofEvo[uAsic][uFee] = new TProfile( sHistName, sHistTitle, 3600, 0., 3600.);
      } // for( UInt_t uFee = 0; uFee < fuTofNrOfFee; uFee++)
   } // for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)

   sHistName  = "hMcbmTimeDiffStsTofMean";
   sHistTitle = "Mean Time difference between STS ASIC and TOF FEE pairs; STS ASIC []; TOF FEE []; <tTOF - tSTS> [ns]";
   fhMcbmTimeDiffStsTofMean = new TH2D( sHistName, sHistTitle,
                                        fuStsNbStsXyters, 0., fuStsNbStsXyters,
                                        fuTofNrOfFee, 0., fuTofNrOfFee);
   sHistName  = "fhMcbmTimeDiffStsTofRMS";
   sHistTitle = "width of Time difference between STS ASIC and TOF FEE pairs; STS ASIC []; TOF FEE []; d(tTOF - tSTS) [ns]";
   fhMcbmTimeDiffStsTofRMS = new TH2D( sHistName, sHistTitle,
                                       fuStsNbStsXyters, 0., fuStsNbStsXyters,
                                       fuTofNrOfFee, 0., fuTofNrOfFee);

#ifdef USE_HTTP_SERVER
   THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
   if( server )
   {
      server->Register("/mCbmDt", fhMcbmTimeDiffStsTofMean );
      server->Register("/mCbmDt", fhMcbmTimeDiffStsTofRMS );

      for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)
      {
         for( UInt_t uFee = 0; uFee < fuTofNrOfFee; uFee++)
         {
            server->Register("/mCbmDt", fvhMcbmTimeDiffStsTof[uAsic][uFee] );
            server->Register("/mCbmDt", fvhMcbmTimeDiffStsTof[uAsic][uFee] );
         } // for( UInt_t uFee = 0; uFee < fuTofNrOfFee; uFee++)
      } // for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)
/*
      server->RegisterCommand("/Reset_All_TOF",  "bMcbm2018SyncResetHistosTof=kTRUE");
      server->RegisterCommand("/Save_All_Tof",   "bMcbm2018SyncSaveHistosTof=kTRUE");
      server->RegisterCommand("/Update_PulsFit", "bMcbm2018SyncUpdateZoomedFit=kTRUE");

      server->Restrict("/Reset_All_Tof", "allow=admin");
      server->Restrict("/Save_All_Tof",  "allow=admin");
      server->Restrict("/Update_PulsFit", "allow=admin");
*/
   } // if( server )
#endif

   Double_t w = 10;
   Double_t h = 10;
   /** Create Pulser check Canvas for STS vs TOF **/
   TCanvas* cPulserMcbm = new TCanvas("cPulserMcbm", "Time difference Mean and RMS for STS ASIC vs TOF FEE", w, h);
   cPulserMcbm->Divide( 2);

   cPulserMcbm->cd(1);
   gPad->SetGridx();
   gPad->SetGridy();
   fhMcbmTimeDiffStsTofMean->Draw( "colz" );

   cPulserMcbm->cd(2);
   gPad->SetGridx();
   gPad->SetGridy();
   fhMcbmTimeDiffStsTofRMS->Draw( "colz" );
   /*****************************/
  LOG(INFO) << "Done Creating mCBM Histograms" << FairLogger::endl;
}
/***************** mCBM Histograms ************************************/

Bool_t CbmMcbm2018MonitorMcbmSync::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   /// General commands
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

   /// STS commands

   /// TOF commands
   if( bMcbm2018SyncResetHistosTof )
   {
      LOG(INFO) << "Reset TOF histos " << FairLogger::endl;
      ResetAllHistos();
      bMcbm2018SyncResetHistosTof = kFALSE;
   } // if( bMcbm2018SyncResetHistosTof )
   if( bMcbm2018SyncSaveHistosTof )
   {
      LOG(INFO) << "Start saving TOF histos " << FairLogger::endl;
      SaveAllHistos( "data/histosMcbmTof.root" );
      bMcbm2018SyncSaveHistosTof = kFALSE;
   } // if( bMcbm2018SyncSaveHistosTof )
   if( bMcbm2018SyncUpdateZoomedFit )
   {
      UpdateZoomedFitTof();
      bMcbm2018SyncUpdateZoomedFit = kFALSE;
   } // if (bMcbm2018SyncUpdateZoomedFit)

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
      // Loop over registered STS components
      for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsListSts.size(); ++uMsCompIdx )
      {
         UInt_t uMsComp = fvMsComponentsListSts[ uMsCompIdx ];

         if( kFALSE == ProcessStsMs( ts, uMsComp, uMsIdx ) )
            return kFALSE;
      } // for( UInt_t uMsComp = 0; uMsComp < fvMsComponentsListSts.size(); ++uMsComp )

      // Loop over registered TOF components
      for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsListTof.size(); ++uMsCompIdx )
      {
         UInt_t uMsComp = fvMsComponentsListTof[ uMsCompIdx ];

         if( kFALSE == ProcessTofMs( ts, uMsComp, uMsIdx ) )
            return kFALSE;
      } // for( UInt_t uMsComp = 0; uMsComp < fvMsComponentsListSts.size(); ++uMsComp )

/****************** STS Sync ******************************************/
      /// Pulses time difference calculation and plotting
      // Sort the buffer of hits
      std::sort( fvmStsHitsInMs.begin(), fvmStsHitsInMs.end() );

      // Time differences plotting using the fully time sorted hits
      if( 0 < fvmStsHitsInMs.size() )
      {
         // Make sure we analyse only MS where all ASICs were detected (remove noise and MS borders)
         if( fuStsNbStsXyters != fvmStsHitsInMs.size() )
            fvmStsHitsInMs.erase( fvmStsHitsInMs.begin(), fvmStsHitsInMs.end() );

         ULong64_t ulLastHitTime = ( *( fvmStsHitsInMs.rbegin() ) ).GetTs();
         std::vector< stsxyter::FinalHit >::iterator it;
         std::vector< stsxyter::FinalHit >::iterator itB;

         std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();
         Double_t dUnixTimeInRun = std::chrono::duration_cast< std::chrono::seconds >(tNow - ftStartTimeUnix).count();

         for( it  = fvmStsHitsInMs.begin();
              it != fvmStsHitsInMs.end();
              ++it )
         {
            UShort_t usAsicIdx = (*it).GetAsic();
            UShort_t usChanIdx = (*it).GetChan();
            ULong64_t ulHitTs  = (*it).GetTs();
            UShort_t  usHitAdc = (*it).GetAdc();

            Double_t dTimeSinceStartSec = (ulHitTs * stsxyter::kdClockCycleNs - fdStsStartTime)* 1e-9;

            fvmStsAsicHitsInMs[ usAsicIdx ].push_back( (*it) );
         } // loop on time sorted hits and split per asic

         // Remove all hits which were already used
         fvmStsHitsInMs.erase( fvmStsHitsInMs.begin(), it );

         for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)
         {
            for( it  = fvmStsAsicHitsInMs[ uAsic ].begin(); it != fvmStsAsicHitsInMs[ uAsic ].end(); ++it )
            {
               UShort_t usChanIdx = (*it).GetChan();
               if( 0.0 == fdStsStartTs )
                  fdStsStartTs = (*it).GetTs() * stsxyter::kdClockCycleNs;
               Double_t dTimeSinceStartSec = ( (*it).GetTs() * stsxyter::kdClockCycleNs - fdStsStartTs ) * 1e-9;

               for( UInt_t uAsicB = uAsic; uAsicB < fuStsNbStsXyters; uAsicB++)
               {
                  for( itB  = fvmStsAsicHitsInMs[ uAsicB ].begin(); itB != fvmStsAsicHitsInMs[ uAsicB ].end(); ++itB )
                  {
                     UShort_t usChanIdxB = (*itB).GetChan();
                     Double_t dDtClk = static_cast< Double_t >( (*itB).GetTs() ) - static_cast< Double_t >( (*it).GetTs() );
                     Double_t dDt = dDtClk * stsxyter::kdClockCycleNs;
                     Double_t dDtRaw = ( static_cast< Double_t >( (*itB).GetTs() % stsxyter::kuTsCycleNbBins )
                                       - static_cast< Double_t >( (*it).GetTs()  % stsxyter::kuTsCycleNbBins )
                                    ) * stsxyter::kdClockCycleNs;

                     fhStsTimeDiffPerAsic[ uAsic ]->Fill( dDt, uAsicB );
                     fhStsTimeDiffPerAsicPair[ uAsic ][ uAsicB ]->Fill( dDt );
                     fhStsTimeDiffClkPerAsicPair[ uAsic ][ uAsicB ]->Fill( dDtClk );
                     fhStsTimeDiffEvoPerAsicPair[ uAsic ][ uAsicB ]->Fill( dTimeSinceStartSec, dDt );
                     fhStsTimeDiffEvoPerAsicPairProf[ uAsic ][ uAsicB ]->Fill( dTimeSinceStartSec, dDt );
                     fhStsRawTimeDiffEvoPerAsicPairProf[ uAsic ][ uAsicB ]->Fill( dTimeSinceStartSec, dDtRaw );

                     fhStsTsLsbMatchPerAsicPair[ uAsic ][ uAsicB ]->Fill(   (*it ).GetTs() & 0x000FF,
                                                                               (*itB).GetTs() & 0x000FF );
                     fhStsTsMsbMatchPerAsicPair[ uAsic ][ uAsicB ]->Fill( ( (*it ).GetTs() & 0x03F00 ) >> 8,
                                                                             ( (*itB).GetTs() & 0x03F00 ) >> 8 );
                  } // for( it  = fvmStsAsicHitsInMs[ uAsicB ].begin(); it != fvmStsAsicHitsInMs[ uAsicB ].end(); ++it )
               } // for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)

               Double_t dDtPulse = ( static_cast< Double_t >( (*it).GetTs() ) - static_cast< Double_t >( fvmStsLastHitAsic[ uAsic ].GetTs() )
                              ) * stsxyter::kdClockCycleNs;
               fhStsIntervalAsic[ uAsic ]->Fill( dDtPulse );
               fhStsIntervalLongAsic[ uAsic ]->Fill( dDtPulse );
               fvmStsLastHitAsic[ uAsic ] = (*it);
            } // for( it  = fvmStsAsicHitsInMs[ uAsic ].begin(); it != fvmStsAsicHitsInMs[ uAsic ].end(); ++it )
         } // for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)
      } // if( 0 < fvmStsHitsInMs.size() )
/****************** STS Sync ******************************************/

/****************** TOF Sync ******************************************/

      // Update RMS plots only every 10s in data
      if( 10.0 < fdTofTsStartTime - fdTofLastRmsUpdateTime )
      {
         // Reset summary histograms for safety
         fhTofTimeRmsPulser->Reset();

         for( UInt_t uFeeA = 0; uFeeA < fuTofNrOfFee; uFeeA++)
            for( UInt_t uFeeB = 0; uFeeB < fuTofNrOfFee; uFeeB++)
               if( NULL != fvhTofTimeDiffPulser[uFeeA][uFeeB] )
               {
                  fhTofTimeRmsPulser->Fill( uFeeA, uFeeB, fvhTofTimeDiffPulser[uFeeA][uFeeB]->GetRMS() );
               } // if( NULL != fvhTimeDiffPulser[uFeeA][uFeeB] )
         fdTofLastRmsUpdateTime = fdTofTsStartTime;
      } // if( 10.0 < fdTofTsStartTime - fdTofLastRmsUpdateTime )
/****************** TOF Sync ******************************************/

/****************** mCBM Sync *****************************************/
      /// Build time difference for each pair of (StsXyter, Fee)
      for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)
      {
         std::vector< stsxyter::FinalHit >::iterator itSts;
         for( itSts = fvmStsAsicHitsInMs[ uAsic ].begin(); itSts != fvmStsAsicHitsInMs[ uAsic ].end(); ++itSts )
         {
            UShort_t usChanIdxSts = (*itSts).GetChan();
            Double_t dStsHitTime = (*itSts).GetTs() * stsxyter::kdClockCycleNs;
            if( 0.0 == fdMcbmStartTs )
            {
               fdMcbmStartTs = dStsHitTime;
            } // if( 0.0 == fdMcbmStartTs )
            Double_t dTimeSinceStartSec = ( dStsHitTime - fdMcbmStartTs ) * 1e-9;

            for( UInt_t uFee = 0; uFee < fuTofNrOfFee; uFee++)
            {
               std::vector< gdpbv100::FullMessage >::iterator itTof;
               for( itTof  = fvmTofFeeHitsInMs[ uFee ].begin(); itTof != fvmTofFeeHitsInMs[ uFee ].end(); ++itTof )
               {
                  Double_t dDtNs = (*itTof).GetFullTimeNs()
                                  - static_cast< Double_t >( (*itSts).GetTs() )* stsxyter::kdClockCycleNs;

                  fvhMcbmTimeDiffStsTof[ uAsic ][ uFee ]->Fill( dDtNs );
                  fvhMcbmTimeDiffStsTofEvo[ uAsic ][ uFee ]->Fill( dTimeSinceStartSec, dDtNs );
               } // for( itTof  = fvmTofFeeHitsInMs[ uAsic ].begin(); itTof != fvmTofFeeHitsInMs[ uAsic ].end(); ++itTof )

            } // for( UInt_t uFee = 0; uFee < fuTofNrOfFee; uFee++)
         } // for( itSts = fvmStsAsicHitsInMs[ uAsic ].begin(); itSts != fvmStsAsicHitsInMs[ uAsic ].end(); ++itSts )


         /// Data in vector are not needed anymore as all possible matches are already checked
         fvmStsAsicHitsInMs[ uAsic ].clear();
      } // for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)

      /// Data in vectors are not needed anymore as all possible matches are already checked
      for( UInt_t uFee = 0; uFee < fuTofNrOfFee; uFee++)
         fvmTofFeeHitsInMs[ uFee ].clear();

      /// Update RMS plots only every 10s in data
      if( 10.0 < fdTofTsStartTime - fdMcbmLastRmsUpdateTime )
      {
         /// Reset summary histograms for safety
         fhMcbmTimeDiffStsTofMean->Reset();
         fhMcbmTimeDiffStsTofMean->Reset();

         for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)
            for( UInt_t uFee = 0; uFee < fuTofNrOfFee; uFee++)
               if( NULL != fvhMcbmTimeDiffStsTof[uAsic][uFee] )
               {
                  fhMcbmTimeDiffStsTofMean->Fill( uAsic, uFee, fvhMcbmTimeDiffStsTof[uAsic][uFee]->GetMean() );
                  fhMcbmTimeDiffStsTofRMS->Fill( uAsic, uFee, fvhMcbmTimeDiffStsTof[uAsic][uFee]->GetRMS() );
               } // if( NULL != fvhTimeDiffPulser[uFeeA][uFeeB] )
         fdMcbmLastRmsUpdateTime = fdTofTsStartTime;
      } // if( 10.0 < fdTofTsStartTime - fdMcbmLastRmsUpdateTime )
/****************** mCBM Sync *****************************************/
   } // for( UInt_t uMsIdx = 0; uMsIdx < uNbMsLoop; uMsIdx ++ )

   if( 0 == ts.index() % 1000 )
   {
      for( UInt_t uDpb = 0; uDpb < fuStsNrOfDpbs; ++uDpb )
      {
         Double_t dTsMsbTime =
               static_cast<ULong64_t>( stsxyter::kuHitNbTsBins )
             * static_cast<ULong64_t>( fvulStsCurrentTsMsb[fuCurrDpbIdx])
             + static_cast<ULong64_t>( stsxyter::kuTsCycleNbBins )
             * static_cast<ULong64_t>( fvuStsCurrentTsMsbCycle[fuCurrDpbIdx] );
         dTsMsbTime *= stsxyter::kdClockCycleNs * 1e-9;

         LOG(INFO) << "End of TS " << std::setw(7) << ts.index()
                   << " eDPB "   << std::setw(2) << uDpb
                   << " current TS MSB counter is " << std::setw(12) << fvulStsCurrentTsMsb[uDpb]
                   << " current TS MSB cycle counter is " << std::setw(12) << fvuStsCurrentTsMsbCycle[uDpb]
                   << " current TS MSB time is " << std::setw(12) << dTsMsbTime << " s"
                   << FairLogger::endl;
      }
   } // if( 0 == ts.index() % 1000 )

   if( 0 == ts.index() % 10000 )
      SavePulserHistos( "data/PulserPeriodicHistosSave.root");

  return kTRUE;
}

/****************** STS Sync ******************************************/
Bool_t CbmMcbm2018MonitorMcbmSync::ProcessStsMs(const fles::Timeslice& ts, size_t uMsComp, UInt_t uMsIdx)
{
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
   fuCurrDpbIdx = fmStsDpbIdIndexMap[ fuCurrDpbId ];

   /*** TODO for TOF, should be changed ?!? ***/
   if( 0 == uMsIdx )
      fdTofTsStartTime = (1e-9) * fulCurrentMsIdx;
   /*** TODO for TOF, should be changed ?!? ***/

   if( fdStsStartTimeMsSz < 0 )
      fdStsStartTimeMsSz = dMsTime;

   // If not integer number of message in input buffer, print warning/error
   if( 0 != ( uSize % kuStsBytesPerMessage ) )
      LOG(ERROR) << "The input microslice buffer does NOT "
                 << "contain only complete nDPB messages!"
                 << FairLogger::endl;

   // Compute the number of complete messages in the input microslice buffer
   uint32_t uNbMessages = ( uSize - ( uSize % kuStsBytesPerMessage ) )
                          / kuStsBytesPerMessage;

   // Prepare variables for the loop on contents
   const uint32_t* pInBuff = reinterpret_cast<const uint32_t*>( msContent );

   for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )
   {
      // Fill message
      uint32_t ulData = static_cast<uint32_t>( pInBuff[uIdx] );

      stsxyter::Message mess( static_cast< uint32_t >( ulData & 0xFFFFFFFF ) );

      // Print message if requested
      if( fbPrintMessages )
         mess.PrintMess( std::cout, fPrintMessCtrlSts );

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
            if( fuStsNbElinksPerDpb <= usElinkIdx )
            {
               LOG(FATAL) << "CbmMcbm2018MonitorMcbmSync::DoUnpack => "
                          << "eLink index out of bounds!"
                          << usElinkIdx << " VS " << fuStsNbElinksPerDpb
                          << FairLogger::endl;
            } // if( fuStsNbElinksPerDpb <= usElinkIdx )
            fhStsMessTypePerElink->Fill( fuCurrDpbIdx * fuStsNbElinksPerDpb + usElinkIdx,
                                            static_cast< uint16_t > (typeMess) );

            UInt_t   uAsicIdx   = fvuStsElinkToAsic[fuCurrDpbIdx][usElinkIdx];

            FillStsHitInfo( mess, usElinkIdx, uAsicIdx, uMsIdx );
            break;
         } // case stsxyter::MessType::Hit :
         case stsxyter::MessType::TsMsb :
         {
            FillStsTsMsbInfo( mess, uIdx, uMsIdx );
            break;
         } // case stsxyter::MessType::TsMsb :
         case stsxyter::MessType::Epoch :
         {
            // The first message in the TS is a special ones: EPOCH
            FillStsEpochInfo( mess );

            if( 0 < uIdx )
               LOG(INFO) << "CbmMcbm2018MonitorMcbmSync::DoUnpack => "
                         << "EPOCH message at unexpected position in MS: message "
                         << uIdx << " VS message 0 expected!"
                         << FairLogger::endl;
            break;
         } // case stsxyter::MessType::TsMsb :
         case stsxyter::MessType::Empty :
         {
//            FillStsTsMsbInfo( mess );
            break;
         } // case stsxyter::MessType::Empty :
         case stsxyter::MessType::Dummy :
         {
            break;
         } // case stsxyter::MessType::Dummy / ReadDataAck / Ack :
         default:
         {
            LOG(FATAL) << "CbmMcbm2018MonitorMcbmSync::DoUnpack => "
                       << "Unknown message type, should never happen, stopping here!"
                       << FairLogger::endl;
         }
      } // switch( mess.GetMessType() )
   } // for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )
   return kTRUE;
}

void CbmMcbm2018MonitorMcbmSync::FillStsHitInfo( stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx )
{
   UShort_t usChan   = mess.GetHitChannel();
   UShort_t usRawAdc = mess.GetHitAdc();
//   UShort_t usFullTs = mess.GetHitTimeFull();
   UShort_t usTsOver = mess.GetHitTimeOver();
   UShort_t usRawTs  = mess.GetHitTime();

   fhStsChanCntRaw[  uAsicIdx ]->Fill( usChan );
   fhStsChanAdcRaw[  uAsicIdx ]->Fill( usChan, usRawAdc );
   fhStsChanAdcRawProf[  uAsicIdx ]->Fill( usChan, usRawAdc );
   fhStsChanRawTs[   uAsicIdx ]->Fill( usChan, usRawTs );
   fhStsChanMissEvt[ uAsicIdx ]->Fill( usChan, mess.IsHitMissedEvts() );

   // Compute the Full time stamp
   Long64_t ulOldHitTime = fvulStsChanLastHitTime[ uAsicIdx ][ usChan ];
   Double_t dOldHitTime  = fvdStsChanLastHitTime[ uAsicIdx ][ usChan ];

      // Use TS w/o overlap bits as they will anyway come from the TS_MSB
   fvulStsChanLastHitTime[ uAsicIdx ][ usChan ] = usRawTs;

   fvulStsChanLastHitTime[ uAsicIdx ][ usChan ] +=
               static_cast<ULong64_t>( stsxyter::kuHitNbTsBins )
             * static_cast<ULong64_t>( fvulStsCurrentTsMsb[fuCurrDpbIdx])
             + static_cast<ULong64_t>( stsxyter::kuTsCycleNbBins )
             * static_cast<ULong64_t>( fvuStsCurrentTsMsbCycle[fuCurrDpbIdx] )
             ;

   // Convert the Hit time in bins to Hit time in ns
   Double_t dHitTimeNs = fvulStsChanLastHitTime[ uAsicIdx ][ usChan ] * stsxyter::kdClockCycleNs;
/*
   // If needed fill the hit interval plots
   if( fbChanHitDtEna )
   {
      Double_t dDeltaT = dHitTimeNs - fvdStsChanLastHitTime[ uAsicIdx ][ usChan ];
      if( 0 == dDeltaT )
         fhStsChanHitDtNeg[ uAsicIdx ]->Fill( 1, usChan );
         else if( 0 < dDeltaT )
            fhStsChanHitDt[ uAsicIdx ]->Fill( dDeltaT, usChan );
         else fhStsChanHitDtNeg[ uAsicIdx ]->Fill( -dDeltaT, usChan );
   } // if( fbChanHitDtEna )
*/
   // Store new value of Hit time in ns
   fvdStsChanLastHitTime[ uAsicIdx ][ usChan ] = fvulStsChanLastHitTime[ uAsicIdx ][ usChan ] * stsxyter::kdClockCycleNs;
/*
   LOG(INFO) << " Asic " << std::setw( 2 ) << uAsicIdx
             << " Channel " << std::setw( 3 ) << usChan
             << " Diff to last hit " << std::setw( 12 ) << ( fvulStsChanLastHitTime[ uAsicIdx ][ usChan ] - ulOldHitTime)
             << " in s " << std::setw( 12 ) << ( fvdStsChanLastHitTime[ uAsicIdx ][ usChan ] - dOldHitTime) * 1e-9
             << FairLogger::endl;
*/
   // Pulser and MS
   fvmStsHitsInMs.push_back( stsxyter::FinalHit( fvulStsChanLastHitTime[ uAsicIdx ][ usChan ], usRawAdc, uAsicIdx, usChan ) );

/*
      LOG(INFO) << " TS " << std::setw( 12 ) << fulCurrentTsIdx
                << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                << " MsInTs " << std::setw( 3 ) << uMsIdx
                << " Asic " << std::setw( 2 ) << uAsicIdx
                << " Channel " << std::setw( 3 ) << usChan
                << " ADC " << std::setw( 3 ) << usRawAdc
                << " TS " << std::setw( 3 )  << usRawTs // 9 bits TS
                << " SX TsMsb " << std::setw( 2 ) << ( fvulStsCurrentTsMsb[fuCurrDpbIdx] & 0x1F ) // Total StsXyter TS = 14 bits => 9b Hit TS + lower 5b TS_MSB after DPB
                << " DPB TsMsb " << std::setw( 6 ) << ( fvulStsCurrentTsMsb[fuCurrDpbIdx] >> 5 ) // Total StsXyter TS = 14 bits => 9b Hit TS + lower 5b of TS_MSB after DPB
                << " TsMsb " << std::setw( 7 ) << fvulStsCurrentTsMsb[fuCurrDpbIdx]
                << " MsbCy " << std::setw( 4 ) << fvuStsCurrentTsMsbCycle[fuCurrDpbIdx]
                << " Time " << std::setw ( 12 ) << fvulStsChanLastHitTime[ uAsicIdx ][ usChan ]
                << FairLogger::endl;
*/
   // Check Starting point of histos with time as X axis
   if( -1 == fdStsStartTime )
      fdStsStartTime = fvdStsChanLastHitTime[ uAsicIdx ][ usChan ];

   // Fill histos with time as X axis
   Double_t dTimeSinceStartSec = (fvdStsChanLastHitTime[ uAsicIdx ][ usChan ] - fdStsStartTime)* 1e-9;
   Double_t dTimeSinceStartMin = dTimeSinceStartSec / 60.0;
   fhStsChanHitRateEvo[ uAsicIdx ]->Fill( dTimeSinceStartSec , usChan );
   fhStsFebRateEvo[ uAsicIdx ]->Fill(   dTimeSinceStartSec );
   fhStsChanHitRateEvoLong[ uAsicIdx ]->Fill( dTimeSinceStartMin, usChan, 1.0/60.0 );
   fhStsFebRateEvoLong[ uAsicIdx ]->Fill(   dTimeSinceStartMin, 1.0/60.0 );
   if( mess.IsHitMissedEvts() )
   {
      fhStsChanMissEvtEvo[ uAsicIdx ]->Fill( dTimeSinceStartSec , usChan );
      fhStsFebMissEvtEvo[ uAsicIdx ]->Fill( dTimeSinceStartSec );
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

void CbmMcbm2018MonitorMcbmSync::FillStsTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx, UInt_t uMsIdx )
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
   if( (uVal != fvulStsCurrentTsMsb[fuCurrDpbIdx] + 1) && 0 < uVal  &&
       !( 1 == uMessIdx && usVal == fvulStsCurrentTsMsb[fuCurrDpbIdx] ) ) // 1st TS_MSB in MS is always a repeat of the last one in previous MS!
   {
      LOG(INFO) << "TS MSB not increasing by 1!  TS " << std::setw( 12 ) << fulCurrentTsIdx
                << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                << " MsInTs " << std::setw( 3 ) << uMsIdx
                << " DPB " << std::setw( 2 ) << fuCurrDpbIdx
                << " Mess " << std::setw( 5 ) << uMessIdx
                << " Old TsMsb " << std::setw( 5 ) << fvulStsCurrentTsMsb[fuCurrDpbIdx]
                << " new TsMsb " << std::setw( 5 ) << uVal
                << " Diff " << std::setw( 5 ) << uVal - fvulStsCurrentTsMsb[fuCurrDpbIdx]
                << " Old MsbCy " << std::setw( 5 ) << fvuStsCurrentTsMsbCycle[fuCurrDpbIdx]
                << FairLogger::endl;
   } // if( (uVal != fvulStsCurrentTsMsb[fuCurrDpbIdx] + 1) && 0 < uVal )
*/

   // Update Status counters
   if( uVal < fvulStsCurrentTsMsb[fuCurrDpbIdx] )
   {

      LOG(INFO) << " TS " << std::setw( 12 ) << fulCurrentTsIdx
                << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                << " DPB " << std::setw( 2 ) << fuCurrDpbIdx
                << " Old TsMsb " << std::setw( 5 ) << fvulStsCurrentTsMsb[fuCurrDpbIdx]
                << " Old MsbCy " << std::setw( 5 ) << fvuStsCurrentTsMsbCycle[fuCurrDpbIdx]
                << " new TsMsb " << std::setw ( 5 ) << uVal
                << FairLogger::endl;

      fvuStsCurrentTsMsbCycle[fuCurrDpbIdx] ++;
   } // if( uVal < fvulStsCurrentTsMsb[fuCurrDpbIdx] )
   if( uVal != fvulStsCurrentTsMsb[fuCurrDpbIdx] + 1 &&
       0 != uVal && 4194303 != fvulStsCurrentTsMsb[fuCurrDpbIdx] &&
       1 != uMessIdx )
   {
      LOG(INFO) << "TS MSb Jump in "
                << " TS " << std::setw( 12 ) << fulCurrentTsIdx
                << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                << " MS Idx " << std::setw( 4 ) << uMsIdx
                << " Msg Idx " << std::setw( 5 ) << uMessIdx
                << " DPB " << std::setw( 2 ) << fuCurrDpbIdx
                << " => Old TsMsb " << std::setw( 5 ) << fvulStsCurrentTsMsb[fuCurrDpbIdx]
                << " new TsMsb " << std::setw ( 5 ) << uVal
                << FairLogger::endl;
   } // if( uVal + 1 != fvulStsCurrentTsMsb[fuCurrDpbIdx] && 4194303 != uVal && 0 != fvulStsCurrentTsMsb[fuCurrDpbIdx] )
   fvulStsCurrentTsMsb[fuCurrDpbIdx] = uVal;
/*
   if( 1 < uMessIdx )
   {
      fhStsDpbRawTsMsb->Fill( fuCurrDpbIdx,      fvulStsCurrentTsMsb[fuCurrDpbIdx] );
      fhStsDpbRawTsMsbSx->Fill( fuCurrDpbIdx,  ( fvulStsCurrentTsMsb[fuCurrDpbIdx] & 0x1F ) );
      fhStsDpbRawTsMsbDpb->Fill( fuCurrDpbIdx, ( fvulStsCurrentTsMsb[fuCurrDpbIdx] >> 5 ) );
   } // if( 0 < uMessIdx )
*/
//   fhStsAsicTsMsb->Fill( fvulStsCurrentTsMsb[fuCurrDpbIdx], uAsicIdx );

   ULong64_t ulNewTsMsbTime =  static_cast< ULong64_t >( stsxyter::kuHitNbTsBins )
                             * static_cast< ULong64_t >( fvulStsCurrentTsMsb[fuCurrDpbIdx])
                             + static_cast< ULong64_t >( stsxyter::kuTsCycleNbBins )
                             * static_cast< ULong64_t >( fvuStsCurrentTsMsbCycle[fuCurrDpbIdx] );
}

void CbmMcbm2018MonitorMcbmSync::FillStsEpochInfo( stsxyter::Message mess )
{
   UInt_t uVal    = mess.GetTsMsbVal();
}
/****************** STS Sync ******************************************/

/****************** TOF Sync ******************************************/

Bool_t CbmMcbm2018MonitorMcbmSync::ProcessTofMs( const fles::Timeslice& ts, size_t uMsComp, UInt_t uMsIdx )
{
   auto msDescriptor = ts.descriptor( uMsComp, uMsIdx );
   fuCurrentEquipmentId = msDescriptor.eq_id;
   fdTofMsIndex = static_cast<double>(msDescriptor.idx);
   const uint8_t* msContent = reinterpret_cast<const uint8_t*>( ts.content( uMsComp, uMsIdx ) );

   /*** TODO for TOF, should be changed ?!? ***/
   if( 0 == uMsIdx )
      fdTofTsStartTime = (1e-9) * fdTofMsIndex;
   /*** TODO for TOF, should be changed ?!? ***/

   uint32_t size = msDescriptor.size;
//    fulLastMsIdx = msDescriptor.idx;
   if (size > 0)
      LOG(DEBUG) << "Microslice: " << msDescriptor.idx << " has size: " << size
                 << FairLogger::endl;

   Int_t messageType = -111;

   // If not integer number of message in input buffer, print warning/error
   if (0 != (size % kuTofBytesPerMessage))
      LOG(ERROR) << "The input microslice buffer does NOT "
                 << "contain only complete nDPB messages!"
                 << FairLogger::endl;

   // Compute the number of complete messages in the input microslice buffer
   uint32_t uNbMessages = (size - (size % kuTofBytesPerMessage)) / kuTofBytesPerMessage;

   // Get the gDPB ID from the MS header
   fuTofGdpbId = fuCurrentEquipmentId;
   fuTofGdpbNr = fmTofGdpbIdIndexMap[fuTofGdpbId];

   // Prepare variables for the loop on contents
   const uint64_t* pInBuff = reinterpret_cast<const uint64_t*>(msContent);
   for( uint32_t uIdx = 0; uIdx < uNbMessages; uIdx++ )
   {
      // Fill message
      uint64_t ulData = static_cast<uint64_t>(pInBuff[uIdx]);
      gdpbv100::Message mess(ulData);

      if (gLogger->IsLogNeeded(DEBUG2))
      {
         mess.printDataCout();
      } // if (gLogger->IsLogNeeded(DEBUG2))


      // Increment counter for different message types
      // and fill the corresponding histogram
      messageType = mess.getMessageType();
      fviTofMsgCounter[messageType]++;
      fhTofMessType->Fill(messageType);

      fuTofGet4Id = mess.getGdpbGenChipId();
      fuTofGet4Nr = (fuTofGdpbNr * fuTofNrOfGet4PerGdpb) + fuTofGet4Id;

      if( fuTofNrOfGet4PerGdpb <= fuTofGet4Id &&
          !mess.isStarTrigger()  &&
          ( gdpbv100::kuChipIdMergedEpoch != fuTofGet4Id ) )
         LOG(WARNING) << "Message with Get4 ID too high: " << fuTofGet4Id
                      << " VS " << fuTofNrOfGet4PerGdpb << " set in parameters." << FairLogger::endl;

      switch (messageType)
      {
         case gdpbv100::MSG_HIT:
         {
            if( mess.getGdpbHitIs24b() )
            {
               fhTofGet4MessType->Fill(fuTofGet4Nr, 4 );
               PrintTofGenInfo(mess);
            } // if( getGdpbHitIs24b() )
               else
               {
                  fhTofGet4MessType->Fill(fuTofGet4Nr, 0 );
                  fvmTofEpSupprBuffer[fuTofGet4Nr].push_back( mess );
               } // else of if( getGdpbHitIs24b() )
            break;
         } // case gdpbv100::MSG_HIT:
         case gdpbv100::MSG_EPOCH:
         {
            if( gdpbv100::kuChipIdMergedEpoch == fuTofGet4Id )
            {
               for( uint32_t uGet4Index = 0; uGet4Index < fuTofNrOfGet4PerGdpb; uGet4Index ++ )
               {
                  fuTofGet4Id = uGet4Index;
                  fuTofGet4Nr = (fuTofGdpbNr * fuTofNrOfGet4PerGdpb) + fuTofGet4Id;
                  gdpbv100::Message tmpMess(mess);
                  tmpMess.setGdpbGenChipId( uGet4Index );

                  fhTofGet4MessType->Fill(fuTofGet4Nr, 1);
                  FillTofEpochInfo(tmpMess);
               } // for( uint32_t uGet4Index = 0; uGet4Index < fuTofNrOfGet4PerGdpb; uGetIndex ++ )
            } // if this epoch message is a merged one valiud for all chips
            else
            {
               fhTofGet4MessType->Fill(fuTofGet4Nr, 1);
               FillTofEpochInfo(mess);
            } // if single chip epoch message
            break;
         } // case gdpbv100::MSG_EPOCH:
         case gdpbv100::MSG_SLOWC:
         {
            fhTofGet4MessType->Fill(fuTofGet4Nr, 2);
            FillTofSlcInfo(mess);
            break;
         } // case gdpbv100::MSG_SLOWC:
         case gdpbv100::MSG_SYST:
         {
            fhTofSysMessType->Fill(mess.getGdpbSysSubType());
            if( gdpbv100::SYS_GET4_ERROR == mess.getGdpbSysSubType() )
            {
               fhTofGet4MessType->Fill(fuTofGet4Nr, 3);

               UInt_t uFeeNr   = (fuTofGet4Id / fuTofNrOfGet4PerFee);
               if (0 <= fdTofStartTime)
               {
                  fvhTofFeeErrorRate_gDPB[(fuTofGdpbNr * fuTofNrOfFeePerGdpb) + uFeeNr]->Fill(
                     1e-9 * (mess.getMsgFullTimeD(fvulTofCurrentEpoch[fuTofGet4Nr]) - fdTofStartTime));
                  fvhTofFeeErrorRatio_gDPB[(fuTofGdpbNr * fuTofNrOfFeePerGdpb) + uFeeNr]->Fill(
                     1e-9 * (mess.getMsgFullTimeD(fvulTofCurrentEpoch[fuTofGet4Nr]) - fdTofStartTime), 1, 1 );
               } // if (0 <= fdTofStartTime)
               if (0 <= fdTofStartTimeLong)
               {
                  fvhTofFeeErrorRateLong_gDPB[(fuTofGdpbNr * fuTofNrOfFeePerGdpb) + uFeeNr]->Fill(
                     1e-9 / 60.0 * (mess.getMsgFullTimeD(fvulTofCurrentEpoch[fuTofGet4Nr]) - fdTofStartTimeLong), 1 / 60.0);
                  fvhTofFeeErrorRatioLong_gDPB[(fuTofGdpbNr * fuTofNrOfFeePerGdpb) + uFeeNr]->Fill(
                     1e-9 / 60.0 * (mess.getMsgFullTimeD(fvulTofCurrentEpoch[fuTofGet4Nr]) - fdTofStartTimeLong), 1, 1 / 60.0);
               } // if (0 <= fdTofStartTime)

               Int_t dFullChId =  fuTofGet4Nr * fuTofNrOfChannelsPerGet4 + mess.getGdpbSysErrChanId();
               switch( mess.getGdpbSysErrData() )
               {
                  case gdpbv100::GET4_V2X_ERR_READ_INIT:
                     fhTofGet4ChanErrors->Fill(dFullChId, 0);
                     break;
                  case gdpbv100::GET4_V2X_ERR_SYNC:
                     fhTofGet4ChanErrors->Fill(dFullChId, 1);
                     break;
                  case gdpbv100::GET4_V2X_ERR_EP_CNT_SYNC:
                     fhTofGet4ChanErrors->Fill(dFullChId, 2);
                     break;
                  case gdpbv100::GET4_V2X_ERR_EP:
                     fhTofGet4ChanErrors->Fill(dFullChId, 3);
                     break;
                  case gdpbv100::GET4_V2X_ERR_FIFO_WRITE:
                     fhTofGet4ChanErrors->Fill(dFullChId, 4);
                     break;
                  case gdpbv100::GET4_V2X_ERR_LOST_EVT:
                     fhTofGet4ChanErrors->Fill(dFullChId, 5);
                     break;
                  case gdpbv100::GET4_V2X_ERR_CHAN_STATE:
                     fhTofGet4ChanErrors->Fill(dFullChId, 6);
                     break;
                  case gdpbv100::GET4_V2X_ERR_TOK_RING_ST:
                     fhTofGet4ChanErrors->Fill(dFullChId, 7);
                     break;
                  case gdpbv100::GET4_V2X_ERR_TOKEN:
                     fhTofGet4ChanErrors->Fill(dFullChId, 8);
                     break;
                  case gdpbv100::GET4_V2X_ERR_READOUT_ERR:
                     fhTofGet4ChanErrors->Fill(dFullChId, 9);
                     break;
                  case gdpbv100::GET4_V2X_ERR_SPI:
                     fhTofGet4ChanErrors->Fill(dFullChId, 10);
                     break;
                  case gdpbv100::GET4_V2X_ERR_DLL_LOCK:
                     fhTofGet4ChanErrors->Fill(dFullChId, 11);
                     break;
                  case gdpbv100::GET4_V2X_ERR_DLL_RESET:
                     fhTofGet4ChanErrors->Fill(dFullChId, 12);
                     break;
                  case gdpbv100::GET4_V2X_ERR_TOT_OVERWRT:
                     fhTofGet4ChanErrors->Fill(dFullChId, 13);
                     break;
                  case gdpbv100::GET4_V2X_ERR_TOT_RANGE:
                     fhTofGet4ChanErrors->Fill(dFullChId, 14);
                     break;
                  case gdpbv100::GET4_V2X_ERR_EVT_DISCARD:
                     fhTofGet4ChanErrors->Fill(dFullChId, 15);
                     break;
                  case gdpbv100::GET4_V2X_ERR_ADD_RIS_EDG:
                     fhTofGet4ChanErrors->Fill(dFullChId, 16);
                     break;
                  case gdpbv100::GET4_V2X_ERR_UNPAIR_FALL:
                     fhTofGet4ChanErrors->Fill(dFullChId, 17);
                     break;
                  case gdpbv100::GET4_V2X_ERR_SEQUENCE_ER:
                     fhTofGet4ChanErrors->Fill(dFullChId, 18);
                     break;
                  case gdpbv100::GET4_V2X_ERR_UNKNOWN:
                     fhTofGet4ChanErrors->Fill(dFullChId, 19);
                     break;
                  default: // Corrupt error or not yet supported error
                     fhTofGet4ChanErrors->Fill(dFullChId, 20);
                     break;
               } // Switch( mess.getGdpbSysErrData() )
            } // if( gdpbv100::SYSMSG_GET4_EVENT == mess.getGdpbSysSubType() )
            FillTofSysInfo(mess);
            break;
         } // case gdpbv100::MSG_SYST:
         case gdpbv100::MSG_STAR_TRI_A:
         case gdpbv100::MSG_STAR_TRI_B:
         case gdpbv100::MSG_STAR_TRI_C:
         case gdpbv100::MSG_STAR_TRI_D:
            fhTofGet4MessType->Fill(fuTofGet4Nr, 5);
            FillTofStarTrigInfo(mess);
            break;
         default:
            LOG(ERROR) << "Message type " << std::hex
                       << std::setw(2) << static_cast<uint16_t>(messageType)
                       << " not included in Get4 unpacker."
                       << FairLogger::endl;
      } // switch( mess.getMessageType() )
   } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)

   return kTRUE;
}

void CbmMcbm2018MonitorMcbmSync::FillTofHitInfo(gdpbv100::Message mess)
{
   UInt_t uChannel = mess.getGdpbHitChanId();
   UInt_t uTot     = mess.getGdpbHit32Tot();
   UInt_t uFts     = mess.getGdpbHitFineTs();

   ULong64_t ulCurEpochGdpbGet4 = fvulTofCurrentEpoch[ fuTofGet4Nr ];

   // In Ep. Suppr. Mode, receive following epoch instead of previous
   if( 0 < ulCurEpochGdpbGet4 )
      ulCurEpochGdpbGet4 --;
      else ulCurEpochGdpbGet4 = gdpbv100::kuEpochCounterSz; // Catch epoch cycle!

   UInt_t uChannelNr = fuTofGet4Id * fuTofNrOfChannelsPerGet4 + uChannel;
   UInt_t uChannelNrInFeet = (fuTofGet4Id % fuTofNrOfGet4PerFee) * fuTofNrOfChannelsPerGet4 + uChannel;
   UInt_t uFeeNr   = (fuTofGet4Id / fuTofNrOfGet4PerFee);
   UInt_t uFeeNrInSys = fuTofGdpbNr * fuTofNrOfFeePerGdpb + uFeeNr;
   UInt_t uRemappedChannelNr = uFeeNr * fuTofNrOfChannelsPerFee + fvuGet4ToPadi[ uChannelNrInFeet ];

   ULong_t  ulHitTime = mess.getMsgFullTime(ulCurEpochGdpbGet4);
   Double_t dHitTime  = mess.getMsgFullTimeD(ulCurEpochGdpbGet4);

   // In 32b mode the coarse counter is already computed back to 112 FTS bins
   // => need to hide its contribution from the Finetime
   // => FTS = Fullt TS modulo 112
   uFts = mess.getGdpbHitFullTs() % 112;

   fvhTofChCount_gDPB[fuTofGdpbNr]->Fill(uChannelNr);
   fvhTofRawFt_gDPB[fuTofGdpbNr]->Fill(uChannelNr, uFts);
//   fvhTofRawTot_gDPB[ fuTofGdpbNr * fuTofNbFeePlotsPerGdpb + uFeeNr/fuTofNbFeePlot ]->Fill(uChannelNr, uTot);

   /// Remapped for PADI to GET4
   fvhTofRemapChCount_gDPB[fuTofGdpbNr]->Fill( uRemappedChannelNr );
//   fvhTofRemapTot_gDPB[ fuTofGdpbNr * fuTofNbFeePlotsPerGdpb + uFeeNr/fuTofNbFeePlot ]->Fill(  uRemappedChannelNr , uTot);

   ///* Pulser monitoring *///
   /// Save last hist time if pulser channel
   /// Fill the corresponding histos if the time difference is reasonnable
   if( gdpbv100::kuFeePulserChannel == uChannelNrInFeet )
   {
      fvdTofTsLastPulserHit[ uFeeNrInSys ] = dHitTime;

      /// Update the difference to all other FEE with lower indices
      for( UInt_t uFeeB = 0; uFeeB < uFeeNrInSys; uFeeB++)
         if( nullptr != fvhTofTimeDiffPulser[uFeeB][uFeeNrInSys] )
         {
            Double_t dTimeDiff = 1e3 * ( fvdTofTsLastPulserHit[ uFeeNrInSys ] - fvdTofTsLastPulserHit[ uFeeB ] );
            if( TMath::Abs( dTimeDiff ) < kdTofMaxDtPulserPs )
               fvhTofTimeDiffPulser[uFeeB][uFeeNrInSys]->Fill( dTimeDiff );
         } // if( nullptr != fvhTofTimeDiffPulser[uFeeB][uFeeB] )

      /// Update the difference to all other FEE with higher indices
      for( UInt_t uFeeB = uFeeNrInSys + 1; uFeeB < fuTofNrOfFee; uFeeB++)
         if( nullptr != fvhTofTimeDiffPulser[uFeeNrInSys][uFeeB] )
         {
            Double_t dTimeDiff = 1e3 * ( fvdTofTsLastPulserHit[ uFeeB ] - fvdTofTsLastPulserHit[ uFeeNrInSys ] );
            if( TMath::Abs( dTimeDiff ) < kdTofMaxDtPulserPs )
               fvhTofTimeDiffPulser[uFeeNrInSys][uFeeB]->Fill( dTimeDiff );
         } // if( nullptr != fvhTofTimeDiffPulser[uFeeNrInSys][uFeeB] )
   } // if( gdpbv100::kuFeePulserChannel == uChannelNrInFeet )

   /// system sync check buffering
   fvmTofFeeHitsInMs[ uFeeNrInSys ].push_back( gdpbv100::FullMessage( mess, ulCurEpochGdpbGet4 ) );

   // In Run rate evolution
   if (fdTofStartTime < 0)
      fdTofStartTime = dHitTime;
/*
   // Reset the evolution Histogram and the start time when we reach the end of the range
   if( fuTofHistoryHistoSize < 1e-9 * (dHitTime - fdTofStartTime) )
   {
      ResetEvolutionHistograms();
      fdTofStartTime = dHitTime;
   } // if( fuTofHistoryHistoSize < 1e-9 * (dHitTime - fdTofStartTime) )
*/
   // In Run rate evolution
   if (fdTofStartTimeLong < 0)
      fdTofStartTimeLong = dHitTime;
/*
   // Reset the evolution Histogram and the start time when we reach the end of the range
   if( fuTofHistoryHistoSizeLong < 1e-9 * (dHitTime - fdTofStartTimeLong) / 60.0 )
   {
      ResetLongEvolutionHistograms();
      fdTofStartTimeLong = dHitTime;
   } // if( fuTofHistoryHistoSize < 1e-9 * (dHitTime - fdTofStartTime) / 60.0 )
*/
   if (0 <= fdTofStartTime)
   {
      fvhTofChannelRate_gDPB[ fuTofGdpbNr ]->Fill( 1e-9 * (dHitTime - fdTofStartTime), uChannelNr );
      fvhTofRemapChRate_gDPB[ fuTofGdpbNr ]->Fill( 1e-9 * (dHitTime - fdTofStartTime), uRemappedChannelNr );
      fvhTofFeeRate_gDPB[(fuTofGdpbNr * fuTofNrOfFeePerGdpb) + uFeeNr]->Fill( 1e-9 * (dHitTime - fdTofStartTime));
      fvhTofFeeErrorRatio_gDPB[(fuTofGdpbNr * fuTofNrOfFeePerGdpb) + uFeeNr]->Fill( 1e-9 * (dHitTime - fdTofStartTime), 0, 1);
   } // if (0 <= fdTofStartTime)

   if (0 <= fdTofStartTimeLong)
   {
      fvhTofFeeRateLong_gDPB[(fuTofGdpbNr * fuTofNrOfFeePerGdpb) + uFeeNr]->Fill(
            1e-9 / 60.0 * (dHitTime - fdTofStartTimeLong), 1 / 60.0 );
      fvhTofFeeErrorRatioLong_gDPB[(fuTofGdpbNr * fuTofNrOfFeePerGdpb) + uFeeNr]->Fill(
            1e-9 / 60.0 * (dHitTime - fdTofStartTimeLong), 0, 1 / 60.0 );
   } // if (0 <= fdTofStartTimeLong)
}

void CbmMcbm2018MonitorMcbmSync::FillTofEpochInfo(gdpbv100::Message mess)
{
   ULong64_t ulEpochNr = mess.getGdpbEpEpochNb();

   fvulTofCurrentEpoch[fuTofGet4Nr] = ulEpochNr;

   if (1 == mess.getGdpbEpSync())
      fhTofGet4EpochFlags->Fill(fuTofGet4Nr, 0);
   if (1 == mess.getGdpbEpDataLoss())
      fhTofGet4EpochFlags->Fill(fuTofGet4Nr, 1);
   if (1 == mess.getGdpbEpEpochLoss())
      fhTofGet4EpochFlags->Fill(fuTofGet4Nr, 2);
   if (1 == mess.getGdpbEpMissmatch())
      fhTofGet4EpochFlags->Fill(fuTofGet4Nr, 3);

   fulTofCurrentEpochTime = mess.getMsgFullTime(ulEpochNr);

   /// Re-align the epoch number of the message in case it will be used later:
   /// We received the epoch after the data instead of the one before!
   if( 0 < ulEpochNr )
      mess.setGdpbEpEpochNb( ulEpochNr - 1 );
      else mess.setGdpbEpEpochNb( gdpbv100::kuEpochCounterSz );

   Int_t iBufferSize = fvmTofEpSupprBuffer[fuTofGet4Nr].size();
   if( 0 < iBufferSize )
   {
      LOG(DEBUG) << "Now processing stored messages for for get4 " << fuTofGet4Nr << " with epoch number "
                 << (fvulTofCurrentEpoch[fuTofGet4Nr] - 1) << FairLogger::endl;

      /// Data are sorted between epochs, not inside => Epoch level ordering
      /// Sorting at lower bin precision level
      std::stable_sort( fvmTofEpSupprBuffer[fuTofGet4Nr].begin(), fvmTofEpSupprBuffer[fuTofGet4Nr].begin() );

      for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
      {
         FillTofHitInfo( fvmTofEpSupprBuffer[fuTofGet4Nr][ iMsgIdx ] );
      } // for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )

      fvmTofEpSupprBuffer[fuTofGet4Nr].clear();
   } // if( 0 < fvmTofEpSupprBuffer[fuTofGet4Nr] )
}

void CbmMcbm2018MonitorMcbmSync::FillTofSlcInfo(gdpbv100::Message mess)
{
   if (fmTofGdpbIdIndexMap.end() != fmTofGdpbIdIndexMap.find(fuTofGdpbId))
   {
      UInt_t uChip = mess.getGdpbGenChipId();
      UInt_t uChan = mess.getGdpbSlcChan();
      UInt_t uEdge = mess.getGdpbSlcEdge();
      UInt_t uData = mess.getGdpbSlcData();
      UInt_t uType = mess.getGdpbSlcType();
      Double_t dFullChId =  fuTofGet4Nr * fuTofNrOfChannelsPerGet4 + mess.getGdpbSlcChan() + 0.5 * mess.getGdpbSlcEdge();
      Double_t dMessTime = static_cast< Double_t>( fulTofCurrentEpochTime ) * 1.e-9;

      switch( uType )
      {
         case 0: // Scaler counter
         {
            fhTofGet4ChanScm->Fill(dFullChId, uType );
            fhTofScmScalerCounters->Fill( uData, dFullChId);
            break;
         }
         case 1: // Deadtime counter
         {
            fhTofGet4ChanScm->Fill(dFullChId, uType );
            fhTofScmDeadtimeCounters->Fill( uData, dFullChId);
            break;
         }
         case 2: // SPI message
         {
            fhTofGet4ChanScm->Fill(dFullChId, uType );
            break;
         }
         case 3: // Start message or SEU counter
         {
            if( 0 == mess.getGdpbSlcChan() && 0 == mess.getGdpbSlcEdge() ) // START message
            {
               fhTofGet4ChanScm->Fill(dFullChId, uType + 1);
            } // if( 0 == mess.getGdpbSlcChan() && 0 == mess.getGdpbSlcEdge() )
            else if( 0 == mess.getGdpbSlcChan() && 1 == mess.getGdpbSlcEdge() ) // SEU counter message
            {
               fhTofGet4ChanScm->Fill(dFullChId, uType );
               fhTofScmSeuCounters->Fill( uData, dFullChId);
               fhTofScmSeuCountersEvo->Fill( dMessTime - fdTofStartTime* 1.e-9, uData, dFullChId);
             } // else if( 0 == mess.getGdpbSlcChan() && 1 == mess.getGdpbSlcEdge() )
            break;
         }
         default: // Should never happen
            break;
      } // switch( mess.getGdpbSlcType() )
   }
}

void CbmMcbm2018MonitorMcbmSync::PrintTofGenInfo(gdpbv100::Message mess)
{
   Int_t mType = mess.getMessageType();
   Int_t channel = mess.getGdpbHitChanId();
   uint64_t uData = mess.getData();

   LOG(DEBUG) << "Get4 MSG type " << mType << " from gdpbId " << fuTofGdpbId
              << ", getId " << fuTofGet4Id << ", (hit channel) " << channel
              << " data " << std::hex << uData
              << FairLogger::endl;
}

void CbmMcbm2018MonitorMcbmSync::FillTofSysInfo(gdpbv100::Message mess)
{
   if (fmTofGdpbIdIndexMap.end() != fmTofGdpbIdIndexMap.find(fuTofGdpbId))
      LOG(DEBUG) << "GET4 System message,       epoch "
                 << static_cast<Int_t>(fvulTofCurrentEpoch[fuTofGet4Nr]) << ", time " << std::setprecision(9)
                 << std::fixed << Double_t(fulTofCurrentEpochTime) * 1.e-9 << " s "
                 << " for board ID " << std::hex << std::setw(4) << fuTofGdpbId
                 << std::dec << FairLogger::endl;

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
            LOG(DEBUG) << " +++++++ > gDPB: " << std::hex << std::setw(4) << fuTofGdpbId
                       << std::dec << ", Chip = " << std::setw(2)
                       << mess.getGdpbGenChipId() << ", Chan = " << std::setw(1)
                       << mess.getGdpbSysErrChanId() << ", Edge = "
                       << std::setw(1) << mess.getGdpbSysErrEdge() << ", Empt = "
                       << std::setw(1) << mess.getGdpbSysErrUnused()
                       << ", Data = " << std::hex << std::setw(2) << uData
                       << std::dec << " -- GET4 V1 Error Event"
                       << FairLogger::endl;
            else LOG(DEBUG) << " +++++++ >gDPB: " << std::hex << std::setw(4) << fuTofGdpbId
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
      case gdpbv100::SYS_SYNC_ERROR:
      {
         LOG(DEBUG) << "Closy synchronization error" << FairLogger::endl;
         break;
      } // case gdpbv100::SYS_SYNC_ERROR:
   } // switch( getGdpbSysSubType() )
}

void CbmMcbm2018MonitorMcbmSync::FillTofStarTrigInfo(gdpbv100::Message mess)
{
   Int_t iMsgIndex = mess.getStarTrigMsgIndex();

   switch( iMsgIndex )
   {
      case 0:
         fvhTofTokenMsgType[fuTofGdpbNr]->Fill(0);
         fvulTofGdpbTsMsb[fuTofGdpbNr] = mess.getGdpbTsMsbStarA();
         break;
      case 1:
         fvhTofTokenMsgType[fuTofGdpbNr]->Fill(1);
         fvulTofGdpbTsLsb[fuTofGdpbNr] = mess.getGdpbTsLsbStarB();
         fvulTofStarTsMsb[fuTofGdpbNr] = mess.getStarTsMsbStarB();
         break;
      case 2:
         fvhTofTokenMsgType[fuTofGdpbNr]->Fill(2);
         fvulTofStarTsMid[fuTofGdpbNr] = mess.getStarTsMidStarC();
         break;
      case 3:
      {
         fvhTofTokenMsgType[fuTofGdpbNr]->Fill(3);

         ULong64_t ulNewGdpbTsFull = ( fvulTofGdpbTsMsb[fuTofGdpbNr] << 24 )
                                   + ( fvulTofGdpbTsLsb[fuTofGdpbNr]       );
         ULong64_t ulNewStarTsFull = ( fvulTofStarTsMsb[fuTofGdpbNr] << 48 )
                                   + ( fvulTofStarTsMid[fuTofGdpbNr] <<  8 )
                                   + mess.getStarTsLsbStarD();
         UInt_t uNewToken  = mess.getStarTokenStarD();
         UInt_t uNewDaqCmd  = mess.getStarDaqCmdStarD();
         UInt_t uNewTrigCmd = mess.getStarTrigCmdStarD();

         if( ( uNewToken == fvuTofStarTokenLast[fuTofGdpbNr] ) && ( ulNewGdpbTsFull == fvulTofGdpbTsFullLast[fuTofGdpbNr] ) &&
             ( ulNewStarTsFull == fvulTofStarTsFullLast[fuTofGdpbNr] ) && ( uNewDaqCmd == fvuTofStarDaqCmdLast[fuTofGdpbNr] ) &&
             ( uNewTrigCmd == fvuTofStarTrigCmdLast[fuTofGdpbNr] ) )
         {
            UInt_t uTrigWord =  ( (fvuTofStarTrigCmdLast[fuTofGdpbNr] & 0x00F) << 16 )
                     + ( (fvuTofStarDaqCmdLast[fuTofGdpbNr]   & 0x00F) << 12 )
                     + ( (fvuTofStarTokenLast[fuTofGdpbNr]    & 0xFFF)       );
            LOG(WARNING) << "Possible error: identical STAR tokens found twice in a row => ignore 2nd! "
                         << " TS " << fulTofCurrentTsIndex
                         << " gDBB #" << fuTofGdpbNr << " "
                         << Form("token = %5u ", fvuTofStarTokenLast[fuTofGdpbNr] )
                         << Form("gDPB ts  = %12llu ", fvulTofGdpbTsFullLast[fuTofGdpbNr] )
                         << Form("STAR ts = %12llu ", fvulTofStarTsFullLast[fuTofGdpbNr] )
                         << Form("DAQ cmd = %2u ", fvuTofStarDaqCmdLast[fuTofGdpbNr] )
                         << Form("TRG cmd = %2u ", fvuTofStarTrigCmdLast[fuTofGdpbNr] )
                         << Form("TRG Wrd = %5x ", uTrigWord )
                         << FairLogger::endl;
            return;
         } // if exactly same message repeated

         // STAR TS counter reset detection
         if( ulNewStarTsFull < fvulTofStarTsFullLast[fuTofGdpbNr] )
            LOG(DEBUG) << "Probable reset of the STAR TS: old = " << Form("%16llu", fvulTofStarTsFullLast[fuTofGdpbNr])
                       << " new = " << Form("%16llu", ulNewStarTsFull)
                       << " Diff = -" << Form("%8llu", fvulTofStarTsFullLast[fuTofGdpbNr] - ulNewStarTsFull)
                       << FairLogger::endl;

         ULong64_t ulGdpbTsDiff = ulNewGdpbTsFull - fvulTofGdpbTsFullLast[fuTofGdpbNr];
         fvulTofGdpbTsFullLast[fuTofGdpbNr] = ulNewGdpbTsFull;
         fvulTofStarTsFullLast[fuTofGdpbNr] = ulNewStarTsFull;
         fvuTofStarTokenLast[fuTofGdpbNr]   = uNewToken;
         fvuTofStarDaqCmdLast[fuTofGdpbNr]  = uNewDaqCmd;
         fvuTofStarTrigCmdLast[fuTofGdpbNr] = uNewTrigCmd;


         /// In Run rate evolution
         if( 0 <= fdTofStartTime )
         {
/*
            /// Reset the evolution Histogram and the start time when we reach the end of the range
            if( fuTofHistoryHistoSize < 1e-9 * (fvulTofGdpbTsFullLast[fuTofGdpbNr] * gdpbv100::kdClockCycleSizeNs - fdTofStartTime) )
            {
               ResetEvolutionHistograms();
               fdTofStartTime = fvulTofGdpbTsFullLast[fuTofGdpbNr] * gdpbv100::kdClockCycleSizeNs;
            } // if( fuTofHistoryHistoSize < 1e-9 * (fulGdpbTsFullLast * gdpbv100::kdClockCycleSizeNs - fdTofStartTime) )
*/
            fvhTofTriggerRate[fuTofGdpbNr]->Fill( 1e-9 * ( fvulTofGdpbTsFullLast[fuTofGdpbNr] * gdpbv100::kdClockCycleSizeNs - fdTofStartTime ) );
            fvhTofStarTokenEvo[fuTofGdpbNr]->Fill( 1e-9 * ( fvulTofGdpbTsFullLast[fuTofGdpbNr] * gdpbv100::kdClockCycleSizeNs - fdTofStartTime ),
                                            fvuTofStarTokenLast[fuTofGdpbNr] );
            fvhTofStarTrigGdpbTsEvo[fuTofGdpbNr]->Fill( 1e-9 * ( fvulTofGdpbTsFullLast[fuTofGdpbNr] * gdpbv100::kdClockCycleSizeNs - fdTofStartTime ),
                                                  fvulTofGdpbTsFullLast[fuTofGdpbNr] );
            fvhTofStarTrigStarTsEvo[fuTofGdpbNr]->Fill( 1e-9 * ( fvulTofGdpbTsFullLast[fuTofGdpbNr] * gdpbv100::kdClockCycleSizeNs - fdTofStartTime ),
                                                  fvulTofStarTsFullLast[fuTofGdpbNr] );
         } // if( 0 < fdTofStartTime )
            else fdTofStartTime = fvulTofGdpbTsFullLast[fuTofGdpbNr] * gdpbv100::kdClockCycleSizeNs;
         fvhTofCmdDaqVsTrig[fuTofGdpbNr]->Fill( fvuTofStarDaqCmdLast[fuTofGdpbNr], fvuTofStarTrigCmdLast[fuTofGdpbNr] );

         break;
      } // case 3
      default:
         LOG(ERROR) << "Unknown Star Trigger messageindex: " << iMsgIndex << FairLogger::endl;
   } // switch( iMsgIndex )
}
/****************** TOF Sync ******************************************/

/****************** STS histos ****************************************/
/****************** STS histos ****************************************/

/****************** TOF histos ****************************************/
void CbmMcbm2018MonitorMcbmSync::UpdateZoomedFitTof()
{
   // Only do something is the user defined the width he want for the zoom
   if( 0.0 < fdTofFitZoomWidthPs )
   {
      // Reset summary histograms for safety
      fhTofTimeRmsZoomFitPuls->Reset();
      fhTofTimeResFitPuls->Reset();

      Double_t dRes = 0;
      TF1 *fitFuncPairs[ fuTofNrOfFee ][ fuTofNrOfFee ];

      for( UInt_t uFeeA = 0; uFeeA < fuTofNrOfFee; uFeeA++)
         for( UInt_t uFeeB = 0; uFeeB < fuTofNrOfFee; uFeeB++)
            if( NULL != fvhTofTimeDiffPulser[uFeeA][uFeeB] )
      {
         // Check that we have at least 1 entry
         if( 0 == fvhTofTimeDiffPulser[uFeeA][uFeeB]->GetEntries() )
         {
            fhTofTimeRmsZoomFitPuls->Fill( uFeeA, uFeeB, 0.0 );
            fhTofTimeResFitPuls->Fill( uFeeA, uFeeB, 0.0 );
            LOG(DEBUG) << "CbmMcbm2018MonitorMcbmSync::UpdateZoomedFitTof => Empty input "
                         << "for FEE pair " << uFeeA << " and " << uFeeB << " !!! "
                         << FairLogger::endl;
            continue;
         } // if( 0 == fvhTofTimeDiffPulser[uFeeA][uFeeB]->GetEntries() )

         // Read the peak position (bin with max counts) + total nb of entries
         Int_t    iBinWithMax = fvhTofTimeDiffPulser[uFeeA][uFeeB]->GetMaximumBin();
         Double_t dNbCounts   = fvhTofTimeDiffPulser[uFeeA][uFeeB]->Integral();

         // Zoom the X axis to +/- ZoomWidth around the peak position
         Double_t dPeakPos = fvhTofTimeDiffPulser[uFeeA][uFeeB]->GetXaxis()->GetBinCenter( iBinWithMax );
         fvhTofTimeDiffPulser[uFeeA][uFeeB]->GetXaxis()->SetRangeUser( dPeakPos - fdTofFitZoomWidthPs,
                                                                    dPeakPos + fdTofFitZoomWidthPs );

         // Read integral and check how much we lost due to the zoom (% loss allowed)
         Double_t dZoomCounts = fvhTofTimeDiffPulser[uFeeA][uFeeB]->Integral();
         if( ( dZoomCounts / dNbCounts ) < 0.99 )
         {
            fhTofTimeRmsZoomFitPuls->Fill( uFeeA, uFeeB, 0.0 );
            fhTofTimeResFitPuls->Fill( uFeeA, uFeeB, 0.0 );
            LOG(WARNING) << "CbmMcbm2018MonitorMcbmSync::UpdateZoomedFitTof => Zoom too strong, "
                         << "more than 1% loss for FEE pair " << uFeeA << " and " << uFeeB << " !!! "
                         << FairLogger::endl;
            continue;
         } // if( ( dZoomCounts / dNbCounts ) < 0.99 )

         // Fill new RMS after zoom into summary histo
         fhTofTimeRmsZoomFitPuls->Fill( uFeeA, uFeeB, fvhTofTimeDiffPulser[uFeeA][uFeeB]->GetRMS() );


         // Fit using zoomed boundaries + starting gaussian width, store into summary histo
         dRes = 0;
         fitFuncPairs[uFeeA][uFeeB] = new TF1( Form("fPair_%02d_%02d", uFeeA, uFeeB ), "gaus",
                                        dPeakPos - fdTofFitZoomWidthPs ,
                                        dPeakPos + fdTofFitZoomWidthPs);
         // Fix the Mean fit value around the Histogram Mean
         fitFuncPairs[uFeeA][uFeeB]->SetParameter( 0, dZoomCounts );
         fitFuncPairs[uFeeA][uFeeB]->SetParameter( 1, dPeakPos );
         fitFuncPairs[uFeeA][uFeeB]->SetParameter( 2, 200.0 ); // Hardcode start with ~4*BinWidth, do better later
         // Using integral instead of bin center seems to lead to unrealistic values => no "I"
         fvhTofTimeDiffPulser[uFeeA][uFeeB]->Fit( Form("fPair_%02d_%02d", uFeeA, uFeeB ), "QRM0");
         // Get Sigma
         dRes = fitFuncPairs[uFeeA][uFeeB]->GetParameter(2);
         // Cleanup memory
         delete fitFuncPairs[uFeeA][uFeeB];
         // Fill summary
         fhTofTimeResFitPuls->Fill( uFeeA, uFeeB,  dRes / TMath::Sqrt2() );


         LOG(INFO) << "CbmMcbm2018MonitorMcbmSync::UpdateZoomedFitTof => "
                      << "For FEE pair " << uFeeA << " and " << uFeeB
                      << " we have zoomed RMS = " << fvhTofTimeDiffPulser[uFeeA][uFeeB]->GetRMS()
                      << " and a resolution of " << dRes / TMath::Sqrt2()
                      << FairLogger::endl;

         // Restore original axis state?
         fvhTofTimeDiffPulser[uFeeA][uFeeB]->GetXaxis()->UnZoom();
      } // loop on uFeeA and uFeeB + check if corresponding fvhTofTimeDiffPulser exists
   } // if( 0.0 < fdTofFitZoomWidthPs )
      else
      {
         LOG(ERROR) << "CbmMcbm2018MonitorMcbmSync::UpdateZoomedFitTof => Zoom width not defined, "
                    << "please use SetTofFitZoomWidthPs, e.g. in macro, before trying this update !!!"
                    << FairLogger::endl;
      } // else of if( 0.0 < fdTofFitZoomWidthPs )
}
/****************** TOF Histos ****************************************/

void CbmMcbm2018MonitorMcbmSync::Reset()
{
}

void CbmMcbm2018MonitorMcbmSync::Finish()
{

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MonitorMcbmSync statistics are " << FairLogger::endl;
   LOG(INFO) << " Hit      messages: " << fmMsgCounter[ stsxyter::MessType::Hit   ] << FairLogger::endl
             << " Ts MSB   messages: " << fmMsgCounter[ stsxyter::MessType::TsMsb ] << FairLogger::endl
             << " Dummy    messages: " << fmMsgCounter[ stsxyter::MessType::Dummy ] << FairLogger::endl
             << " Epoch    messages: " << fmMsgCounter[ stsxyter::MessType::Epoch ] << FairLogger::endl
             << " Empty    messages: " << fmMsgCounter[ stsxyter::MessType::Empty ] << FairLogger::endl;

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;

   SaveAllHistos( fsHistoFileFullname );
   SaveAllHistos();

}


void CbmMcbm2018MonitorMcbmSync::FillOutput(CbmDigi* /*digi*/)
{
}

void CbmMcbm2018MonitorMcbmSync::SaveAllHistos( TString sFileName )
{
   TDirectory * oldDir = nullptr;
   TFile * histoFile = nullptr;
   if( "" != sFileName )
   {
      // Store current directory position to allow restore later
      oldDir = gDirectory;
      // open separate histo file in recreate mode
      histoFile = new TFile( sFileName , "RECREATE");
      histoFile->cd();
   } // if( "" != sFileName )

/****************** STS Sync ******************************************/
   gDirectory->mkdir("Sts_Raw");
   gDirectory->cd("Sts_Raw");

   fhStsMessType->Write();
   fhStsSysMessType->Write();
   fhStsMessTypePerDpb->Write();
   fhStsSysMessTypePerDpb->Write();
   fhStsMessTypePerElink->Write();
   fhStsSysMessTypePerElink->Write();

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
   {
      fhStsChanCntRaw[ uXyterIdx ]->Write();
      fhStsChanAdcRaw[ uXyterIdx ]->Write();
      fhStsChanAdcRawProf[ uXyterIdx ]->Write();
      fhStsChanRawTs[ uXyterIdx ]->Write();
      fhStsChanMissEvt[ uXyterIdx ]->Write();
      fhStsChanMissEvtEvo[ uXyterIdx ]->Write();
      fhStsFebMissEvtEvo[ uXyterIdx ]->Write();
      fhStsChanHitRateEvo[ uXyterIdx ]->Write();
      fhStsFebRateEvo[ uXyterIdx ]->Write();
      fhStsChanHitRateEvoLong[ uXyterIdx ]->Write();
      fhStsFebRateEvoLong[ uXyterIdx ]->Write();
/*
      if( kTRUE == fbLongHistoEnable )
      {
         fhFebRateEvoLong[ uXyterIdx ]->Write();
         fhFebChRateEvoLong[ uXyterIdx ]->Write();
      } // if( kTRUE == fbLongHistoEnable )
*/
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )

   gDirectory->cd("..");
/****************** STS Sync ******************************************/

/****************** TOF Sync ******************************************/
   gDirectory->mkdir("Tof_Raw_gDPB");
   gDirectory->cd("Tof_Raw_gDPB");

   fhTofMessType->Write();
   fhTofSysMessType->Write();
   fhTofGet4MessType->Write();
   fhTofGet4ChanScm->Write();
   fhTofGet4ChanErrors->Write();
   fhTofGet4EpochFlags->Write();
   fhTofScmScalerCounters->Write();
   fhTofScmDeadtimeCounters->Write();
   fhTofScmSeuCounters->Write();
   fhTofScmSeuCountersEvo->Write();

   for( UInt_t uTotPlot = 0; uTotPlot < fvhTofRawTot_gDPB.size(); ++uTotPlot )
      fvhTofRawTot_gDPB[ uTotPlot ]->Write();

   for( UInt_t uTotPlot = 0; uTotPlot < fvhTofRemapTot_gDPB.size(); ++uTotPlot )
      fvhTofRemapTot_gDPB[ uTotPlot ]->Write();

   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      fvhTofRawFt_gDPB[ uGdpb ]->Write();
      fvhTofChCount_gDPB[ uGdpb ]->Write();
      fvhTofChannelRate_gDPB[ uGdpb ]->Write();
      fvhTofRemapChCount_gDPB[ uGdpb ]->Write();
      fvhTofRemapChRate_gDPB[ uGdpb ]->Write();

      for (UInt_t uFeet = 0; uFeet < fuTofNrOfFeePerGdpb; ++ uFeet)
      {
         fvhTofFeeRate_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFeet ]->Write();
         fvhTofFeeErrorRate_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFeet ]->Write();
         fvhTofFeeErrorRatio_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFeet ]->Write();
         fvhTofFeeRateLong_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFeet ]->Write();
         fvhTofFeeErrorRateLong_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFeet ]->Write();
         fvhTofFeeErrorRatioLong_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFeet ]->Write();
      } // for (UInt_t uFeet = 0; uFeet < fuTofNrOfFeePerGdpb; ++ uFeet)

   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   fhTofTimeRmsPulser->Write();
   fhTofTimeRmsZoomFitPuls->Write();
   fhTofTimeResFitPuls->Write();

   gDirectory->cd("..");

   ///* STAR event building/cutting *///
   gDirectory->mkdir("Star_Raw");
   gDirectory->cd("Star_Raw");
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      fvhTofTokenMsgType[ uGdpb ]->Write();
      fvhTofTriggerRate[ uGdpb ]->Write();
      fvhTofCmdDaqVsTrig[ uGdpb ]->Write();
      fvhTofStarTokenEvo[ uGdpb ]->Write();
      fvhTofStarTrigGdpbTsEvo[ uGdpb ]->Write();
      fvhTofStarTrigStarTsEvo[ uGdpb ]->Write();
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   gDirectory->cd("..");

   ///* Pulser monitoring *///
   gDirectory->mkdir("TofDt");
   gDirectory->cd("TofDt");
   for( UInt_t uFeeA = 0; uFeeA < fuTofNrOfFee; uFeeA++)
      for( UInt_t uFeeB = 0; uFeeB < fuTofNrOfFee; uFeeB++)
         if( NULL != fvhTofTimeDiffPulser[uFeeA][uFeeB] )
            fvhTofTimeDiffPulser[uFeeA][uFeeB]->Write();
   gDirectory->cd("..");
/****************** TOF Sync ******************************************/

/****************** mCBM Sync *****************************************/
   gDirectory->mkdir("mCbmDt");
   gDirectory->cd("mCbmDt");

   fhMcbmTimeDiffStsTofMean->Write();
   fhMcbmTimeDiffStsTofRMS->Write();

   for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)
   {
      for( UInt_t uFee = 0; uFee < fuTofNrOfFee; uFee++)
      {
         fvhMcbmTimeDiffStsTof[uAsic][uFee]->Write();
         fvhMcbmTimeDiffStsTof[uAsic][uFee]->Write();
      } // for( UInt_t uFee = 0; uFee < fuTofNrOfFee; uFee++)
   } // for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)

   gDirectory->cd("..");
/****************** mCBM Sync *****************************************/

   /***************************/
   // Flib Histos
   gDirectory->mkdir("Flib_Raw");
   gDirectory->cd("Flib_Raw");

   TH1 * pMissedTsH1    = dynamic_cast< TH1 * >( gROOT->FindObjectAny( "Missed_TS" ) );
   if( nullptr != pMissedTsH1 )
      pMissedTsH1->Write();

   TProfile * pMissedTsEvoP = dynamic_cast< TProfile * >( gROOT->FindObjectAny( "Missed_TS_Evo" ) );
   if( nullptr != pMissedTsEvoP )
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
void CbmMcbm2018MonitorMcbmSync::SavePulserHistos( TString sFileName )
{
   TDirectory * oldDir = nullptr;
   TFile * histoFile = nullptr;
   if( "" != sFileName )
   {
      // Store current directory position to allow restore later
      oldDir = gDirectory;
      // open separate histo file in recreate mode
      histoFile = new TFile( sFileName , "RECREATE");
      histoFile->cd();
   } // if( "" != sFileName )

/****************** STS Sync ******************************************/
   gDirectory->mkdir("Sts_Pulser");
   gDirectory->cd("Sts_Pulser");

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
      {
         fhStsTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Write();
         fhStsRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Write();
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )

   gDirectory->cd("..");
/****************** STS Sync ******************************************/

   if( "" != sFileName )
   {
      // Restore original directory position
      histoFile->Close();
      oldDir->cd();
   } // if( "" != sFileName )

}
void CbmMcbm2018MonitorMcbmSync::ResetAllHistos()
{
/****************** STS Sync ******************************************/
   LOG(INFO) << "Reseting all STS histograms." << FairLogger::endl;

   fhStsMessType->Reset();
   fhStsSysMessType->Reset();
   fhStsMessTypePerDpb->Reset();
   fhStsSysMessTypePerDpb->Reset();
   fhStsMessTypePerElink->Reset();
   fhStsSysMessTypePerElink->Reset();

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
   {
      fhStsChanCntRaw[ uXyterIdx ]->Reset();
      fhStsChanAdcRaw[ uXyterIdx ]->Reset();
      fhStsChanAdcRawProf[ uXyterIdx ]->Reset();
      fhStsChanRawTs[ uXyterIdx ]->Reset();
      fhStsChanMissEvt[ uXyterIdx ]->Reset();
      fhStsChanMissEvtEvo[ uXyterIdx ]->Reset();
      fhStsFebMissEvtEvo[ uXyterIdx ]->Reset();
      fhStsChanHitRateEvo[ uXyterIdx ]->Reset();
      fhStsFebRateEvo[ uXyterIdx ]->Reset();
      fhStsChanHitRateEvoLong[ uXyterIdx ]->Reset();
      fhStsFebRateEvoLong[ uXyterIdx ]->Reset();
/*
      if( kTRUE == fbLongHistoEnable )
      {
         ftStartTimeUnix = std::chrono::steady_clock::now();
         fhFebRateEvoLong[ uXyterIdx ]->Reset();
         fhFebChRateEvoLong[ uXyterIdx ]->Reset();
      } // if( kTRUE == fbLongHistoEnable )
*/

      fhStsTimeDiffPerAsic[ uXyterIdx ]->Reset();
      for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
      {
         fhStsTimeDiffPerAsicPair[ uXyterIdx ][uXyterIdxB]->Reset();
         fhStsTimeDiffClkPerAsicPair[ uXyterIdx ][uXyterIdxB]->Reset();
         fhStsTimeDiffEvoPerAsicPair[ uXyterIdx ][uXyterIdxB]->Reset();
         fhStsTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Reset();
         fhStsRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Reset();
         fhStsTsLsbMatchPerAsicPair[ uXyterIdx ][uXyterIdxB]->Reset();
         fhStsTsMsbMatchPerAsicPair[ uXyterIdx ][uXyterIdxB]->Reset();
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
      fhStsIntervalAsic[ uXyterIdx ]->Reset();
      fhStsIntervalLongAsic[ uXyterIdx ]->Reset();
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )

   fdStsStartTime = -1;
   fdStsStartTimeMsSz = -1;
/****************** STS Sync ******************************************/

/****************** TOF Sync ******************************************/
   fhTofMessType->Reset();
   fhTofSysMessType->Reset();
   fhTofGet4MessType->Reset();
   fhTofGet4ChanScm->Reset();
   fhTofGet4ChanErrors->Reset();
   fhTofGet4EpochFlags->Reset();
   fhTofScmScalerCounters->Reset();
   fhTofScmDeadtimeCounters->Reset();
   fhTofScmSeuCounters->Reset();
   fhTofScmSeuCountersEvo->Reset();

   for( UInt_t uTotPlot = 0; uTotPlot < fvhTofRawTot_gDPB.size(); ++uTotPlot )
      fvhTofRawTot_gDPB[ uTotPlot ]->Reset();

   for( UInt_t uTotPlot = 0; uTotPlot < fvhTofRemapTot_gDPB.size(); ++uTotPlot )
      fvhTofRemapTot_gDPB[ uTotPlot ]->Reset();

   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      fvhTofRawFt_gDPB[ uGdpb ]->Reset();
      fvhTofChCount_gDPB[ uGdpb ]->Reset();
      fvhTofChannelRate_gDPB[ uGdpb ]->Reset();
      fvhTofRemapChCount_gDPB[ uGdpb ]->Reset();
      fvhTofRemapChRate_gDPB[ uGdpb ]->Reset();

      for (UInt_t uFeet = 0; uFeet < fuTofNrOfFeePerGdpb; ++ uFeet)
      {
         fvhTofFeeRate_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFeet ]->Reset();
         fvhTofFeeErrorRate_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFeet ]->Reset();
         fvhTofFeeErrorRatio_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFeet ]->Reset();
         fvhTofFeeRateLong_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFeet ]->Reset();
         fvhTofFeeErrorRateLong_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFeet ]->Reset();
         fvhTofFeeErrorRatioLong_gDPB[ uGdpb * fuTofNrOfFeePerGdpb + uFeet ]->Reset();
      } // for (UInt_t uFeet = 0; uFeet < fuTofNrOfFeePerGdpb; ++ uFeet)

   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   fhTofTimeRmsPulser->Reset();
   fhTofTimeRmsZoomFitPuls->Reset();
   fhTofTimeResFitPuls->Reset();

   ///* STAR event building/cutting *///
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfGdpbs; ++uGdpb )
   {
      fvhTofTokenMsgType[ uGdpb ]->Reset();
      fvhTofTriggerRate[ uGdpb ]->Reset();
      fvhTofCmdDaqVsTrig[ uGdpb ]->Reset();
      fvhTofStarTokenEvo[ uGdpb ]->Reset();
      fvhTofStarTrigGdpbTsEvo[ uGdpb ]->Reset();
      fvhTofStarTrigStarTsEvo[ uGdpb ]->Reset();
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

   ///* Pulser monitoring *///
   for( UInt_t uFeeA = 0; uFeeA < fuTofNrOfFee; uFeeA++)
      for( UInt_t uFeeB = 0; uFeeB < fuTofNrOfFee; uFeeB++)
         if( NULL != fvhTofTimeDiffPulser[uFeeA][uFeeB] )
            fvhTofTimeDiffPulser[uFeeA][uFeeB]->Reset();

   fdTofStartTime = -1;
   fdTofStartTimeLong = -1;
/****************** TOF Sync ******************************************/

/****************** mCBM Sync *****************************************/
   fhMcbmTimeDiffStsTofMean->Reset();
   fhMcbmTimeDiffStsTofRMS->Reset();

   for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)
   {
      for( UInt_t uFee = 0; uFee < fuTofNrOfFee; uFee++)
      {
         fvhMcbmTimeDiffStsTof[uAsic][uFee]->Reset();
         fvhMcbmTimeDiffStsTof[uAsic][uFee]->Reset();
      } // for( UInt_t uFee = 0; uFee < fuTofNrOfFee; uFee++)
   } // for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)
/****************** mCBM Sync *****************************************/
}

void CbmMcbm2018MonitorMcbmSync::SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize )
{
   TDatime * fRunStartDateTime     = new TDatime( dateIn, timeIn);
   fiRunStartDateTimeSec = fRunStartDateTime->Convert();
   fiBinSizeDatePlots    = iBinSize;

   LOG(INFO) << "Assigned new MUCH Run Start Date-Time: " << fRunStartDateTime->AsString() << FairLogger::endl;
}

void CbmMcbm2018MonitorMcbmSync::SetLongDurationLimits( UInt_t uDurationSeconds, UInt_t uBinSize )
{
   fbLongHistoEnable     = kTRUE;
   fuLongHistoNbSeconds  = uDurationSeconds;
   fuLongHistoBinSizeSec = uBinSize;
}

ClassImp(CbmMcbm2018MonitorMcbmSync)
