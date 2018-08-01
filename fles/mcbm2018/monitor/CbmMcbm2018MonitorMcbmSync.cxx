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

CbmMcbm2018MonitorMcbmSync::CbmMcbm2018MonitorMcbmSync() :
   CbmMcbmUnpack(),
   fvMsComponentsListSts(),
   fvMsComponentsListTof(),
   fuNbCoreMsPerTs(0),
   fuNbOverMsPerTs(0),
   fbIgnoreOverlapMs(kFALSE),
   fUnpackParHodo(NULL),
   fuStsNrOfDpbs(0),
   fmStsDpbIdIndexMap(),
   fuStsNbElinksPerDpb(0),
   fuStsNbStsXyters(0),
   fuStsNbChanPerAsic(0),
   fvuStsElinkToAsic(),
   fsHistoFileFullname( "data/SetupHistos.root" ),
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
   fdStartTime(-1.0),
   fdStartTimeMsSz(-1.0),
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
   fhStsMessType(NULL),
   fhStsSysMessType(NULL),
   fhStsMessTypePerDpb(NULL),
   fhStsSysMessTypePerDpb(NULL),
   fhStsMessTypePerElink(NULL),
   fhStsSysMessTypePerElink(NULL),
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
   fhStsIntervalLongAsic()
{
}

CbmMcbm2018MonitorMcbmSync::~CbmMcbm2018MonitorMcbmSync()
{
}

Bool_t CbmMcbm2018MonitorMcbmSync::Init()
{
  LOG(INFO) << "Initializing flib StsXyter unpacker for STS" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  return kTRUE;
}

void CbmMcbm2018MonitorMcbmSync::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
         << FairLogger::endl;
   fUnpackParHodo = (CbmCern2017UnpackParHodo*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmCern2017UnpackParHodo"));
}


Bool_t CbmMcbm2018MonitorMcbmSync::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
         << FairLogger::endl;

   Bool_t bReInit = ReInitContainers();
   CreateStsHistograms();

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
      title =  Form( "Time diff between consecutive hits in ASIC %03us; dt [ns];  Counts", uXyterIdx, uXyterIdx );
      fhStsIntervalAsic.push_back( new TH1I( sHistName, title, 200, 0, 200 * stsxyter::kdClockCycleNs ) );

      sHistName = Form( "fhStsIntervalLongAsic_%03u", uXyterIdx );
      title =  Form( "Time diff between consecutive hits in ASIC %03us; dt [ns];  Counts", uXyterIdx, uXyterIdx );
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
}

Bool_t CbmMcbm2018MonitorMcbmSync::DoUnpack(const fles::Timeslice& ts, size_t component)
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
      // Loop over registered STS components
      for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsListSts.size(); ++uMsCompIdx )
      {
         UInt_t uMsComp = fvMsComponentsListSts[ uMsCompIdx ];

         if( kFALSE == ProcessStsMs( ts, uMsComp, uMsIdx ) )
            return kFALSE;
      } // for( UInt_t uMsComp = 0; uMsComp < fvMsComponentsListSts.size(); ++uMsComp )
/*
      // Loop over registered TOF components
      for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsListTof.size(); ++uMsCompIdx )
      {
         UInt_t uMsComp = fvMsComponentsListTof[ uMsCompIdx ];

         if( kFALSE == ProcessTofMs( ts, uMsComp, uMsIdx ) )
            return kFALSE;
      } // for( UInt_t uMsComp = 0; uMsComp < fvMsComponentsListSts.size(); ++uMsComp )
*/
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

            Double_t dTimeSinceStartSec = (ulHitTs * stsxyter::kdClockCycleNs - fdStartTime)* 1e-9;

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

            /// Data in vector are not needed anymore as all possible matches are already checked
            fvmStsAsicHitsInMs[ uAsic ].clear();
         } // for( UInt_t uAsic = 0; uAsic < fuStsNbStsXyters; uAsic++)
      } // if( 0 < fvmStsHitsInMs.size() )
/****************** STS Sync ******************************************/
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


   if( fdStartTimeMsSz < 0 )
      fdStartTimeMsSz = dMsTime;

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
   if( -1 == fdStartTime )
      fdStartTime = fvdStsChanLastHitTime[ uAsicIdx ][ usChan ];

   // Fill histos with time as X axis
   Double_t dTimeSinceStartSec = (fvdStsChanLastHitTime[ uAsicIdx ][ usChan ] - fdStartTime)* 1e-9;
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
   /***************************/

   /***************************/
   // Flib Histos
   gDirectory->mkdir("Flib_Raw");
   gDirectory->cd("Flib_Raw");

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
void CbmMcbm2018MonitorMcbmSync::SavePulserHistos( TString sFileName )
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

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )
   {
      for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
      {
         fhStsTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Write();
         fhStsRawTimeDiffEvoPerAsicPairProf[ uXyterIdx ][uXyterIdxB]->Write();
      } // for( UInt_t uXyterIdxB = 0; uXyterIdxB < fuStsNbStsXyters; ++uXyterIdxB )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuStsNbStsXyters; ++uXyterIdx )

   gDirectory->cd("..");
   /***************************/

   if( "" != sFileName )
   {
      // Restore original directory position
      histoFile->Close();
      oldDir->cd();
   } // if( "" != sFileName )

}
void CbmMcbm2018MonitorMcbmSync::ResetAllHistos()
{
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

  fdStartTime = -1;
  fdStartTimeMsSz = -1;
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
