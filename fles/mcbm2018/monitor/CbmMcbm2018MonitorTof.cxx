// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                    CbmMcbm2018MonitorTof                          -----
// -----               Created 10.07.2018 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018MonitorTof.h"
#include "CbmMcbm2018TofPar.h"

#include "CbmHistManager.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRunOnline.h"

#include "TROOT.h"
#include "TClonesArray.h"
#include "TString.h"
#include "THttpServer.h"
#include "Rtypes.h"
#include "TProfile2D.h"
#include "TProfile.h"
#include "TH2.h"
#include "TH1.h"
#include "TF1.h"
#include "TCanvas.h"
#include "THStack.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPaveStats.h"
#include "TMath.h"

#include <algorithm>
#include <iostream>
#include <stdint.h>
#include <iomanip>
#include <ctime>

Bool_t bMcbmMoniTofResetHistos = kFALSE;
Bool_t bMcbmMoniTofSaveHistos  = kFALSE;
Bool_t bMcbmMoniTofUpdateZoomedFit = kFALSE;
Bool_t bMcbmMoniTofRawDataPrint     = kFALSE;
Bool_t bMcbmMoniTofPrintAllHitsEna  = kFALSE;
Bool_t bMcbmMoniTofPrintAllEpochsEna = kFALSE;

CbmMcbm2018MonitorTof::CbmMcbm2018MonitorTof() :
    CbmMcbmUnpack(),
    fvMsComponentsList(),
    fuNbCoreMsPerTs(0),
    fuNbOverMsPerTs(0),
    fbIgnoreOverlapMs(kFALSE),
    fuMsAcceptsPercent(100),
    fuTotalMsNb(0),
    fuOverlapMsNb(0),
    fuCoreMs(0),
    fdMsSizeInNs(0.0),
    fdTsCoreSizeInNs(0.0),
    fuMinNbGdpb( 0 ),
    fuCurrNbGdpb( 0 ),
    fUnpackPar(),
    fuNrOfGdpbs(0),
    fuNrOfFeePerGdpb(0),
    fuNrOfGet4PerFee(0),
    fuNrOfChannelsPerGet4(0),
    fuNrOfChannelsPerFee(0),
    fuNrOfGet4(0),
    fuNrOfGet4PerGdpb(0),
    fuNrOfChannelsPerGdpb(0),
    fuRawDataPrintMsgNb(100),
    fuRawDataPrintMsgIdx(fuRawDataPrintMsgNb),
    fbPrintAllHitsEnable(kFALSE),
    fbPrintAllEpochsEnable(kFALSE),
    fbPulserModeEnable(kFALSE),
    fbCoincMapsEnable(kFALSE),
    fulCurrentTsIndex(0),
    fuCurrentMs(0),
    fdMsIndex(0),
    fuGdpbId(0),
    fuGdpbNr(0),
    fuGet4Id(0),
    fuGet4Nr(0),
    fiEquipmentId(0),
    fviMsgCounter(11, 0), // length of enum MessageTypes initialized with 0
    fvulGdpbTsMsb(),
    fvulGdpbTsLsb(),
    fvulStarTsMsb(),
    fvulStarTsMid(),
    fvulGdpbTsFullLast(),
    fvulStarTsFullLast(),
    fvuStarTokenLast(),
    fvuStarDaqCmdLast(),
    fvuStarTrigCmdLast(),
    fvulCurrentEpoch(),
    fvbFirstEpochSeen(),
    fulCurrentEpochTime(0),
    fGdpbIdIndexMap(),
    fvmEpSupprBuffer(),
    fvuFeeNbHitsLastMs(),
    fdTsLastPulserHit(),
    fvuCoincNbHitsLastMs(),
    fvdCoincTsLastHit(),
    dMinDt(-1.*(kuNbBinsDt*gdpbv100::kdBinSize/2.) - gdpbv100::kdBinSize/2.),
    dMaxDt(1.*(kuNbBinsDt*gdpbv100::kdBinSize/2.) + gdpbv100::kdBinSize/2.),
    fuNbFeePlot(2),
    fuNbFeePlotsPerGdpb(0),
    fdStartTime(-1.),
    fdStartTimeLong(-1.),
    fdStartTimeMsSz(-1.),
    fuHistoryHistoSize( 1800 ),
    fuHistoryHistoSizeLong( 600 ),
    fdLastRmsUpdateTime(0.0),
    fdFitZoomWidthPs(0.0),
    fcMsSizeAll(NULL),
    fvhMsSzPerLink(12, NULL),
    fvhMsSzTimePerLink(12, NULL),
    fhMessType(NULL),
    fhSysMessType(NULL),
    fhGdpbMessType(NULL),
    fhGdpbSysMessType(NULL),
    fhGet4MessType(NULL),
    fhGet4ChanScm(NULL),
    fhGet4ChanErrors(NULL),
    fhGet4EpochFlags(NULL),
    fhScmScalerCounters(NULL),
    fhScmDeadtimeCounters(NULL),
    fhScmSeuCounters(NULL),
    fhScmSeuCountersEvo(NULL),
    fvhRawFt_gDPB(),
    fvhRawTot_gDPB(),
    fvhChCount_gDPB(),
    fvhChannelRate_gDPB(),
    fvhRemapTot_gDPB(),
    fvhRemapChCount_gDPB(),
    fvhRemapChRate_gDPB(),
    fvhFeeRate_gDPB(),
    fvhFeeErrorRate_gDPB(),
    fvhFeeErrorRatio_gDPB(),
    fvhFeeRateLong_gDPB(),
    fvhFeeErrorRateLong_gDPB(),
    fvhFeeErrorRatioLong_gDPB(),
    fvhRemapTotSideA_mod(),
    fvhRemapTotSideB_mod(),
    fvhTokenMsgType(),
    fvhTriggerRate(),
    fvhCmdDaqVsTrig(),
    fvhStarTokenEvo(),
    fvhStarTrigGdpbTsEvo(),
    fvhStarTrigStarTsEvo(),
    fvhTimeDiffPulser(),
    fhTimeMeanPulser(NULL),
    fhTimeRmsPulser(NULL),
    fhTimeRmsZoomFitPuls(NULL),
    fhTimeResFitPuls(NULL),
    fvhPulserTimeDiffEvoGbtxGbtx(),
    fvvhPulserTimeDiffEvoGdpbGdpb(),
    fvuPadiToGet4(),
    fvuGet4ToPadi(),
    fvuElinkToGet4(),
    fvuGet4ToElink(),
    fTimeLastHistoSaving()
{
}

CbmMcbm2018MonitorTof::~CbmMcbm2018MonitorTof()
{
}

Bool_t CbmMcbm2018MonitorTof::Init()
{
   LOG(INFO) << "Initializing Get4 monitor" << FairLogger::endl;

   FairRootManager* ioman = FairRootManager::Instance();
   if( ioman == NULL )
   {
      LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
   } // if( ioman == NULL )

   return kTRUE;
}

void CbmMcbm2018MonitorTof::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
             << FairLogger::endl;
   fUnpackPar = (CbmMcbm2018TofPar*) (FairRun::Instance()->GetRuntimeDb()->getContainer(
          "CbmMcbm2018TofPar") );

}

Bool_t CbmMcbm2018MonitorTof::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
             << FairLogger::endl;
   Bool_t initOK = ReInitContainers();

   CreateHistograms();

   fvulCurrentEpoch.resize( fuNrOfGdpbs * fuNrOfGet4PerGdpb );
   fvbFirstEpochSeen.resize( fuNrOfGdpbs * fuNrOfGet4PerGdpb );
   for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )
   {
      for( UInt_t j = 0; j < fuNrOfGet4PerGdpb; ++j )
      {
         fvulCurrentEpoch[GetArrayIndex(i, j)] = 0;
      } // for( UInt_t j = 0; j < fuNrOfGet4PerGdpb; ++j )
   } // for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )

  return initOK;
}

Bool_t CbmMcbm2018MonitorTof::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
             << FairLogger::endl;

   fuNrOfGdpbs = fUnpackPar->GetNrOfGdpbs();
   LOG(INFO) << "Nr. of Tof GDPBs: " << fuNrOfGdpbs << FairLogger::endl;
   fuMinNbGdpb = fuNrOfGdpbs;

   fuNrOfFeePerGdpb = fUnpackPar->GetNrOfFeesPerGdpb();
   LOG(INFO) << "Nr. of FEEs per Tof GDPB: " << fuNrOfFeePerGdpb
               << FairLogger::endl;

   fuNrOfGet4PerFee = fUnpackPar->GetNrOfGet4PerFee();
   LOG(INFO) << "Nr. of GET4 per Tof FEE: " << fuNrOfGet4PerFee
               << FairLogger::endl;

   fuNrOfChannelsPerGet4 = fUnpackPar->GetNrOfChannelsPerGet4();
   LOG(INFO) << "Nr. of channels per GET4: " << fuNrOfChannelsPerGet4
               << FairLogger::endl;

   fuNrOfChannelsPerFee = fuNrOfGet4PerFee * fuNrOfChannelsPerGet4;
   LOG(INFO) << "Nr. of channels per FEE: " << fuNrOfChannelsPerFee
               << FairLogger::endl;

   fuNrOfGet4 = fuNrOfGdpbs * fuNrOfFeePerGdpb * fuNrOfGet4PerFee;
   LOG(INFO) << "Nr. of GET4s: " << fuNrOfGet4 << FairLogger::endl;

   fuNrOfGet4PerGdpb = fuNrOfFeePerGdpb * fuNrOfGet4PerFee;
   LOG(INFO) << "Nr. of GET4s per GDPB: " << fuNrOfGet4PerGdpb
               << FairLogger::endl;

   fuNrOfChannelsPerGdpb = fuNrOfGet4PerGdpb * fuNrOfChannelsPerGet4;
   LOG(INFO) << "Nr. of channels per GDPB: " << fuNrOfChannelsPerGdpb
               << FairLogger::endl;

   fGdpbIdIndexMap.clear();
   for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )
   {
      fGdpbIdIndexMap[fUnpackPar->GetGdpbId(i)] = i;
      LOG(INFO) << "GDPB Id of TOF  " << i << " : " << std::hex << fUnpackPar->GetGdpbId(i)
                 << std::dec << FairLogger::endl;
   } // for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )

   fuNrOfGbtx  =  fUnpackPar->GetNrOfGbtx();
   LOG(INFO) << "Nr. of GBTx: " << fuNrOfGbtx << FairLogger::endl;

   fuNrOfModules  =  fUnpackPar->GetNrOfModules();
   LOG(INFO) << "Nr. of GBTx: " << fuNrOfModules << FairLogger::endl;

   fviRpcType.resize(  fuNrOfGbtx );
   fviModuleId.resize( fuNrOfGbtx );
   fviNrOfRpc.resize(  fuNrOfGbtx );
   fviRpcSide.resize(  fuNrOfGbtx );
   for( UInt_t uGbtx = 0; uGbtx < fuNrOfGbtx; ++uGbtx)
   {
      fviNrOfRpc[ uGbtx ]  = fUnpackPar->GetNrOfRpc( uGbtx );
      fviRpcType[ uGbtx ]  = fUnpackPar->GetRpcType( uGbtx );
      fviRpcSide[ uGbtx ]  = fUnpackPar->GetRpcSide( uGbtx );
      fviModuleId[ uGbtx ] = fUnpackPar->GetModuleId( uGbtx );
   } // for( UInt_t uGbtx = 0; uGbtx < uNrOfGbtx; ++uGbtx)

   LOG(INFO) << "Nr. of RPCs per GBTx: ";
   for( UInt_t uGbtx = 0; uGbtx < fuNrOfGbtx; ++uGbtx)
      LOG(INFO) << Form(" %2d", fviNrOfRpc[ uGbtx ] );
   LOG(INFO) << FairLogger::endl;

   LOG(INFO) << "RPC type per GBTx:    ";
   for( UInt_t uGbtx = 0; uGbtx < fuNrOfGbtx; ++uGbtx)
      LOG(INFO) << Form(" %2d", fviRpcType[ uGbtx ] );
   LOG(INFO) << FairLogger::endl;

   LOG(INFO) << "RPC side per GBTx:    ";
   for( UInt_t uGbtx = 0; uGbtx < fuNrOfGbtx; ++uGbtx)
      LOG(INFO) << Form(" %2d", fviRpcSide[ uGbtx ] );
   LOG(INFO) << FairLogger::endl;

   LOG(INFO) << "Module ID per GBTx:   ";
   for( UInt_t uGbtx = 0; uGbtx < fuNrOfGbtx; ++uGbtx)
      LOG(INFO) << Form(" %2d", fviModuleId[ uGbtx ] );
   LOG(INFO) << FairLogger::endl;

   fuTotalMsNb   = fUnpackPar->GetNbMsTot();
   fuOverlapMsNb = fUnpackPar->GetNbMsOverlap();
   fuCoreMs      = fuTotalMsNb - fuOverlapMsNb;
   fdMsSizeInNs  = fUnpackPar->GetSizeMsInNs();
   fdTsCoreSizeInNs = fdMsSizeInNs * fuCoreMs;
   LOG(INFO) << "Timeslice parameters: "
             << fuTotalMsNb << " MS per link, of which "
             << fuOverlapMsNb << " overlap MS, each MS is "
             << fdMsSizeInNs << " ns"
             << FairLogger::endl;

   /// STAR Trigger decoding and monitoring
   fvulGdpbTsMsb.resize(  fuNrOfGdpbs );
   fvulGdpbTsLsb.resize(  fuNrOfGdpbs );
   fvulStarTsMsb.resize(  fuNrOfGdpbs );
   fvulStarTsMid.resize(  fuNrOfGdpbs );
   fvulGdpbTsFullLast.resize(  fuNrOfGdpbs );
   fvulStarTsFullLast.resize(  fuNrOfGdpbs );
   fvuStarTokenLast.resize(  fuNrOfGdpbs );
   fvuStarDaqCmdLast.resize(  fuNrOfGdpbs );
   fvuStarTrigCmdLast.resize(  fuNrOfGdpbs );
   for (UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb)
   {
      fvulGdpbTsMsb[ uGdpb ] = 0;
      fvulGdpbTsLsb[ uGdpb ] = 0;
      fvulStarTsMsb[ uGdpb ] = 0;
      fvulStarTsMid[ uGdpb ] = 0;
      fvulGdpbTsFullLast[ uGdpb ] = 0;
      fvulStarTsFullLast[ uGdpb ] = 0;
      fvuStarTokenLast[ uGdpb ]   = 0;
      fvuStarDaqCmdLast[ uGdpb ]  = 0;
      fvuStarTrigCmdLast[ uGdpb ] = 0;
   } // for (Int_t iGdpb = 0; iGdpb < fuNrOfGdpbs; ++iGdpb)

   fvmEpSupprBuffer.resize( fuNrOfGet4 );

   ///* Pulser monitoring *///
   if( kTRUE == fbPulserModeEnable )
   {
      fvuFeeNbHitsLastMs.resize( fuNrOfFeePerGdpb * fuNrOfGdpbs, 0 );
      fdTsLastPulserHit.resize( fuNrOfFeePerGdpb * fuNrOfGdpbs, 0.0 );
   } // if( kTRUE == fbPulserModeEnable )

   ///* coincidence maps *///
   if( kTRUE == fbCoincMapsEnable )
   {
      fvuCoincNbHitsLastMs.resize( fuNrOfGdpbs );
      fvdCoincTsLastHit.resize( fuNrOfGdpbs );
      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      {
         fvuCoincNbHitsLastMs[ uGdpb ].resize( fuNrOfChannelsPerGdpb, 0 );
         fvdCoincTsLastHit[ uGdpb ].resize( fuNrOfChannelsPerGdpb, 0.0 );
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   } // if( kTRUE == fbCoincMapsEnable )

/// TODO: move these constants somewhere shared, e.g the parameter file
   fvuPadiToGet4.resize( fuNrOfChannelsPerFee );
   fvuGet4ToPadi.resize( fuNrOfChannelsPerFee );
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

   for( UInt_t uChan = 0; uChan < fuNrOfChannelsPerFee; ++uChan )
   {
      fvuPadiToGet4[ uChan ] = uPaditoget4[ uChan ] - 1;
      fvuGet4ToPadi[ uChan ] = uGet4topadi[ uChan ] - 1;
   } // for( UInt_t uChan = 0; uChan < fuNrOfChannelsPerFee; ++uChan )


/// TODO: move these constants somewhere shared, e.g the parameter file
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
   } // for( UInt_t uChan = 0; uChan < fuNrOfChannelsPerFee; ++uChan )

	return kTRUE;
}


void CbmMcbm2018MonitorTof::AddMsComponentToList( size_t component, UShort_t usDetectorId )
{
   /// Check for duplicates and ignore if it is the case
   for( UInt_t uCompIdx = 0; uCompIdx < fvMsComponentsList.size(); ++uCompIdx )
      if( component == fvMsComponentsList[ uCompIdx ] )
         return;

   /// Add to list
   fvMsComponentsList.push_back( component );

   /// Create MS size monitoring histos
   if( NULL == fvhMsSzPerLink[ component ] )
   {
      TString sMsSzName  = Form( "MsSz_link_%02lu", component );
      TString sMsSzTitle = Form( "Size of MS from link %02lu; Ms Size [bytes]", component );
      fvhMsSzPerLink[ component ] = new TH1F(sMsSzName.Data(), sMsSzTitle.Data(), 160000, 0., 20000. );

      sMsSzName = Form("MsSzTime_link_%02lu", component);
      sMsSzTitle = Form( "Size of MS vs time for gDPB of link %02lu; Time[s] ; Ms Size [bytes]", component);
      fvhMsSzTimePerLink[ component ] =  new TProfile( sMsSzName.Data(), sMsSzTitle.Data(),
                                                       100 * fuHistoryHistoSize, 0., 2 * fuHistoryHistoSize );
#ifdef USE_HTTP_SERVER
      THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
      if( server )
      {
         server->Register("/FlibRaw", fvhMsSzPerLink[ component ]);
         server->Register("/FlibRaw", fvhMsSzTimePerLink[ component ]);
      } // if( server )
#endif
      if( NULL != fcMsSizeAll )
      {
         fcMsSizeAll->cd( 1 + component );
         gPad->SetLogy();
         fvhMsSzTimePerLink[ component ]->Draw("hist le0");
      } // if( NULL != fcMsSizeAll )
      LOG(INFO) << "Added MS size histo for component (link): " << component
                << FairLogger::endl;
   } // if( NULL == fvhMsSzPerLink[ component ] )
}
void CbmMcbm2018MonitorTof::SetNbMsInTs( size_t uCoreMsNb, size_t uOverlapMsNb )
{
   fuNbCoreMsPerTs = uCoreMsNb;
   fuNbOverMsPerTs = uOverlapMsNb;

//   UInt_t uNbMsTotal = fuNbCoreMsPerTs + fuNbOverMsPerTs;
}

void CbmMcbm2018MonitorTof::CreateHistograms()
{
   LOG(INFO) << "create Histos for " << fuNrOfGdpbs <<" gDPBs "
	          << FairLogger::endl;

#ifdef USE_HTTP_SERVER
   THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
#endif

   TString name { "" };
   TString title { "" };

   // Full Fee time difference test
   UInt_t uNbBinsDt = kuNbBinsDt + 1; // To account for extra bin due to shift by 1/2 bin of both ranges

   fuNbFeePlotsPerGdpb = fuNrOfFeePerGdpb/fuNbFeePlot + ( 0 != fuNrOfFeePerGdpb%fuNbFeePlot ? 1 : 0 );
   Double_t dBinSzG4v2 = (6250. / 112.);
   dMinDt     = -1.*(kuNbBinsDt*dBinSzG4v2/2.) - dBinSzG4v2/2.;
   dMaxDt     =  1.*(kuNbBinsDt*dBinSzG4v2/2.) + dBinSzG4v2/2.;

   /*******************************************************************/
   name = "hMessageType";
   title = "Nb of message for each type; Type";
   // Test Big Data readout with plotting
   fhMessType = new TH1I(name, title, 1 + gdpbv100::MSG_STAR_TRI_D, 0., 1 + gdpbv100::MSG_STAR_TRI_D);
   fhMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_HIT,        "HIT");
   fhMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_EPOCH,      "EPOCH");
   fhMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_SLOWC,      "SLOWC");
   fhMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_SYST,       "SYST");
   fhMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_STAR_TRI_A, "TRI_A");
   fhMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_STAR_TRI_B, "TRI_B");
   fhMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_STAR_TRI_C, "TRI_C");
   fhMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_STAR_TRI_D, "TRI_D");

   /*******************************************************************/
   name = "hSysMessType";
   title = "Nb of system message for each type; System Type";
   fhSysMessType = new TH1I(name, title, 1 + gdpbv100::SYS_SYNC_ERROR, 0., 1 + gdpbv100::SYS_SYNC_ERROR);
   fhSysMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::SYS_GET4_ERROR,     "GET4 ERROR");
   fhSysMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::SYS_GDPB_UNKWN,     "UNKW GET4 MSG");
   fhSysMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::SYS_GET4_SYNC_MISS, "SYS_GET4_SYNC_MISS");
   fhSysMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::SYS_SYNC_ERROR,     "SYNC ERROR");

   /*******************************************************************/
   name = "hGdpbMessageType";
   title = "Nb of message for each type per Gdpb; Type; Gdpb Idx []";
   // Test Big Data readout with plotting
   fhGdpbMessType = new TH2I(name, title, 1 + gdpbv100::MSG_STAR_TRI_D, 0., 1 + gdpbv100::MSG_STAR_TRI_D,
                                          fuNrOfGdpbs, -0.5, fuNrOfGdpbs - 0.5 );
   fhGdpbMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_HIT,        "HIT");
   fhGdpbMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_EPOCH,      "EPOCH");
   fhGdpbMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_SLOWC,      "SLOWC");
   fhGdpbMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_SYST,       "SYST");
   fhGdpbMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_STAR_TRI_A, "TRI_A");
   fhGdpbMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_STAR_TRI_B, "TRI_B");
   fhGdpbMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_STAR_TRI_C, "TRI_C");
   fhGdpbMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_STAR_TRI_D, "TRI_D");

   /*******************************************************************/
   name = "hGdpbSysMessType";
   title = "Nb of system message for each type per Gdpb; System Type; Gdpb Idx []";
   fhGdpbSysMessType = new TH2I(name, title, 1 + gdpbv100::SYS_SYNC_ERROR, 0., 1 + gdpbv100::SYS_SYNC_ERROR,
                                          fuNrOfGdpbs, -0.5, fuNrOfGdpbs - 0.5 );
   fhGdpbSysMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::SYS_GET4_ERROR,     "GET4 ERROR");
   fhGdpbSysMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::SYS_GDPB_UNKWN,     "UNKW GET4 MSG");
   fhGdpbSysMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::SYS_GET4_SYNC_MISS, "SYS_GET4_SYNC_MISS");
   fhGdpbSysMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::SYS_SYNC_ERROR,     "SYNC ERROR");


   /*******************************************************************/
   name = "hGet4MessType";
   title = "Nb of message for each type per GET4; GET4 chip # ; Type";
   fhGet4MessType = new TH2I(name, title, fuNrOfGet4, 0., fuNrOfGet4, 4, 0., 4.);
   fhGet4MessType->GetYaxis()->SetBinLabel( 1, "DATA 32b");
   fhGet4MessType->GetYaxis()->SetBinLabel( 2, "EPOCH");
   fhGet4MessType->GetYaxis()->SetBinLabel( 3, "S.C. M");
   fhGet4MessType->GetYaxis()->SetBinLabel( 4, "ERROR");
//   fhGet4MessType->GetYaxis()->SetBinLabel( 5, "DATA 24b");
//   fhGet4MessType->GetYaxis()->SetBinLabel( 6, "STAR Trigger");

   /*******************************************************************/
   name = "hGet4ChanScm";
   title = "SC messages per GET4 channel; GET4 channel # ; SC type";
   fhGet4ChanScm =  new TH2I(name, title,
         2 * fuNrOfGet4 * fuNrOfChannelsPerGet4, 0., fuNrOfGet4 * fuNrOfChannelsPerGet4,
         5, 0., 5.);
   fhGet4ChanScm->GetYaxis()->SetBinLabel( 1, "Hit Scal" );
   fhGet4ChanScm->GetYaxis()->SetBinLabel( 2, "Deadtime" );
   fhGet4ChanScm->GetYaxis()->SetBinLabel( 3, "SPI" );
   fhGet4ChanScm->GetYaxis()->SetBinLabel( 4, "SEU Scal" );
   fhGet4ChanScm->GetYaxis()->SetBinLabel( 5, "START" );

   /*******************************************************************/
   name = "hGet4ChanErrors";
   title = "Error messages per GET4 channel; GET4 channel # ; Error";
   fhGet4ChanErrors = new TH2I(name, title,
            fuNrOfGet4 * fuNrOfChannelsPerGet4, 0., fuNrOfGet4 * fuNrOfChannelsPerGet4,
            21, 0., 21.);
   fhGet4ChanErrors->GetYaxis()->SetBinLabel( 1, "0x00: Readout Init    ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel( 2, "0x01: Sync            ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel( 3, "0x02: Epoch count sync");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel( 4, "0x03: Epoch           ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel( 5, "0x04: FIFO Write      ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel( 6, "0x05: Lost event      ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel( 7, "0x06: Channel state   ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel( 8, "0x07: Token Ring state");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel( 9, "0x08: Token           ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(10, "0x09: Error Readout   ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(11, "0x0a: SPI             ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(12, "0x0b: DLL Lock error  "); // <- From GET4 v1.2
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(13, "0x0c: DLL Reset invoc."); // <- From GET4 v1.2
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(14, "0x11: Overwrite       ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(15, "0x12: ToT out of range");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(16, "0x13: Event Discarded ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(17, "0x14: Add. Rising edge"); // <- From GET4 v1.3
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(18, "0x15: Unpaired Falling"); // <- From GET4 v1.3
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(19, "0x16: Sequence error  "); // <- From GET4 v1.3
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(20, "0x7f: Unknown         ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(21, "Corrupt/unsuprtd error");

   /*******************************************************************/
   name = "hGet4EpochFlags";
   title = "Epoch flags per GET4; GET4 chip # ; Type";
   fhGet4EpochFlags = new TH2I(name, title, fuNrOfGet4, 0., fuNrOfGet4, 4, 0., 4.);
   fhGet4EpochFlags->GetYaxis()->SetBinLabel(1, "SYNC");
   fhGet4EpochFlags->GetYaxis()->SetBinLabel(2, "Ep LOSS");
   fhGet4EpochFlags->GetYaxis()->SetBinLabel(3, "Da LOSS");
   fhGet4EpochFlags->GetYaxis()->SetBinLabel(4, "MISSMAT");

   /*******************************************************************/
      // Slow control messages analysis
   name = "hScmScalerCounters";
   title = "Content of Scaler counter SC messages; Scaler counter [hit]; Channel []";
   fhScmScalerCounters = new TH2I(name, title, fuNrOfGet4 * fuNrOfChannelsPerGet4 * 2, 0., fuNrOfGet4 * fuNrOfChannelsPerGet4,
                                               8192, 0., 8192.);

   name = "hScmDeadtimeCounters";
   title = "Content of Deadtime counter SC messages; Deadtime [Clk Cycles]; Channel []";
   fhScmDeadtimeCounters = new TH2I(name, title, fuNrOfGet4 * fuNrOfChannelsPerGet4 * 2, 0., fuNrOfGet4 * fuNrOfChannelsPerGet4,
                                               8192, 0., 8192.);

   name = "hScmSeuCounters";
   title = "Content of SEU counter SC messages; SEU []; Channel []";
   fhScmSeuCounters = new TH2I(name, title, fuNrOfGet4 * fuNrOfChannelsPerGet4 * 2, 0., fuNrOfGet4 * fuNrOfChannelsPerGet4,
                                               8192, 0., 8192.);

   name = "hScmSeuCountersEvo";
   title = "SEU counter rate from SC messages; Time in Run [s]; Channel []; SEU []";
   fhScmSeuCountersEvo = new TH2I(name, title, fuNrOfGet4 * fuNrOfChannelsPerGet4 * 2, 0., fuNrOfGet4 * fuNrOfChannelsPerGet4,
                                               fuHistoryHistoSize, 0., fuHistoryHistoSize);

   /*******************************************************************/
  for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
  {
      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
      name = Form("RawFt_gDPB_%02u", uGdpb);
      title = Form("Raw FineTime gDPB %02u Plot 0; channel; FineTime [bin]", uGdpb);
      fvhRawFt_gDPB.push_back(
         new TH2F(name.Data(), title.Data(),
            fuNrOfChannelsPerGdpb, 0, fuNrOfChannelsPerGdpb,
            128, 0, 128 ) );

      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
      name = Form("RawTot_gDPB_%02u", uGdpb);
      title = Form("Raw TOT gDPB %02u; channel; TOT [bin]", uGdpb);
      fvhRawTot_gDPB.push_back(
         new TH2F(name.Data(), title.Data(),
            fuNrOfChannelsPerGdpb, 0, fuNrOfChannelsPerGdpb,
            256, 0, 256 ) );

      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
       name = Form("ChCount_gDPB_%02u", uGdpb);
       title = Form("Channel counts gDPB %02u; channel; Hits", uGdpb);
       fvhChCount_gDPB.push_back( new TH1I(name.Data(), title.Data(),
                fuNrOfFeePerGdpb * fuNrOfChannelsPerFee, 0, fuNrOfFeePerGdpb * fuNrOfChannelsPerFee) );

      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
       name = Form("ChRate_gDPB_%02u", uGdpb);
       title = Form("Channel rate gDPB %02u; Time in run [s]; channel; Rate [1/s]", uGdpb);
       fvhChannelRate_gDPB.push_back( new TH2D(name.Data(), title.Data(),
                fuHistoryHistoSize, 0, fuHistoryHistoSize,
                fuNrOfFeePerGdpb * fuNrOfChannelsPerFee, 0, fuNrOfFeePerGdpb * fuNrOfChannelsPerFee ) );

      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
      name = Form("RemapTot_gDPB_%02u", uGdpb);
      title = Form("Raw TOT gDPB %02u remapped; PADI channel; TOT [bin]", uGdpb);
      fvhRemapTot_gDPB.push_back(
         new TH2F(name.Data(), title.Data(),
            fuNrOfChannelsPerGdpb, 0, fuNrOfChannelsPerGdpb,
            256, 0, 256 ) );

      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
       name = Form("RemapChCount_gDPB_%02u", uGdpb);
       title = Form("Channel counts gDPB %02u remapped; PADI channel; Hits", uGdpb);
       fvhRemapChCount_gDPB.push_back( new TH1I(name.Data(), title.Data(),
                fuNrOfFeePerGdpb * fuNrOfChannelsPerFee, 0, fuNrOfFeePerGdpb * fuNrOfChannelsPerFee) );

      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
       name = Form("RemapChRate_gDPB_%02u", uGdpb);
       title = Form("PADI channel rate gDPB %02u; Time in run [s]; PADI channel; Rate [1/s]", uGdpb);
       fvhRemapChRate_gDPB.push_back( new TH2D(name.Data(), title.Data(),
                fuHistoryHistoSize, 0, fuHistoryHistoSize,
                fuNrOfFeePerGdpb * fuNrOfChannelsPerFee, 0, fuNrOfFeePerGdpb * fuNrOfChannelsPerFee ) );

      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
      for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++ uFee)
      {
         name = Form("FeeRate_gDPB_g%02u_f%1u", uGdpb, uFee);
         title = Form(
             "Counts per second in Fee %1u of gDPB %02u; Time[s] ; Counts", uFee,
             uGdpb);
         fvhFeeRate_gDPB.push_back( new TH1D(name.Data(), title.Data(), fuHistoryHistoSize, 0, fuHistoryHistoSize) );

         name = Form("FeeErrorRate_gDPB_g%02u_f%1u", uGdpb, uFee);
         title = Form(
             "Error Counts per second in Fee %1u of gDPB %02u; Time[s] ; Error Counts", uFee,
             uGdpb);
         fvhFeeErrorRate_gDPB.push_back( new TH1D(name.Data(), title.Data(), fuHistoryHistoSize, 0, fuHistoryHistoSize) );

         name = Form("FeeErrorRatio_gDPB_g%02u_f%1u", uGdpb, uFee);
         title = Form(
             "Error to data ratio per second in Fee %1u of gDPB %02u; Time[s] ; Error ratio[]", uFee,
             uGdpb);
         fvhFeeErrorRatio_gDPB.push_back( new TProfile(name.Data(), title.Data(), fuHistoryHistoSize, 0, fuHistoryHistoSize) );

         name = Form("FeeRateLong_gDPB_g%02u_f%1u", uGdpb, uFee);
         title = Form(
             "Counts per minutes in Fee %1u of gDPB %02u; Time[min] ; Counts", uFee,
             uGdpb);
         fvhFeeRateLong_gDPB.push_back( new TH1D(name.Data(), title.Data(), fuHistoryHistoSizeLong, 0, fuHistoryHistoSizeLong) );

         name = Form("FeeErrorRateLong_gDPB_g%02u_f%1u", uGdpb, uFee);
         title = Form(
             "Error Counts per minutes in Fee %1u of gDPB %02u; Time[min] ; Error Counts", uFee,
             uGdpb);
         fvhFeeErrorRateLong_gDPB.push_back( new TH1D(name.Data(), title.Data(), fuHistoryHistoSizeLong, 0, fuHistoryHistoSizeLong) );

         name = Form("FeeErrorRatioLong_gDPB_g%02u_f%1u", uGdpb, uFee);
         title = Form(
             "Error to data ratio per minutes in Fee %1u of gDPB %02u; Time[min] ; Error ratio[]", uFee,
             uGdpb);
         fvhFeeErrorRatioLong_gDPB.push_back( new TProfile(name.Data(), title.Data(), fuHistoryHistoSizeLong, 0, fuHistoryHistoSizeLong) );
      } // for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; uFee++)

      /**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
      /// STAR Trigger decoding and monitoring
      name = Form( "hTokenMsgType_gDPB_%02u", uGdpb);
      title = Form( "STAR trigger Messages type gDPB %02u; Type ; Counts", uGdpb);
      fvhTokenMsgType.push_back(  new TH1F(name, title, 4, 0, 4) );
      fvhTokenMsgType[ uGdpb ]->GetXaxis()->SetBinLabel( 1, "A"); // gDPB TS high
      fvhTokenMsgType[ uGdpb ]->GetXaxis()->SetBinLabel( 2, "B"); // gDPB TS low, STAR TS high
      fvhTokenMsgType[ uGdpb ]->GetXaxis()->SetBinLabel( 3, "C"); // STAR TS mid
      fvhTokenMsgType[ uGdpb ]->GetXaxis()->SetBinLabel( 4, "D"); // STAR TS low, token, CMDs
#ifdef USE_HTTP_SERVER
      if (server)
         server->Register("/StarRaw", fvhTokenMsgType[ uGdpb ] );
#endif

      name = Form( "hTriggerRate_gDPB_%02u", uGdpb);
      title = Form( "STAR trigger signals per second gDPB %02u; Time[s] ; Counts", uGdpb);
      fvhTriggerRate.push_back(  new TH1F(name, title, fuHistoryHistoSize, 0, fuHistoryHistoSize) );
#ifdef USE_HTTP_SERVER
      if (server)
         server->Register("/StarRaw", fvhTriggerRate[ uGdpb ] );
#endif

      name = Form( "hCmdDaqVsTrig_gDPB_%02u", uGdpb);
      title = Form( "STAR daq command VS STAR trigger command gDPB %02u; DAQ ; TRIGGER", uGdpb);
      fvhCmdDaqVsTrig.push_back( new TH2I(name, title, 16, 0, 16, 16, 0, 16 ) );
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 1, "0x0: no-trig "); // idle link
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 2, "0x1: clear   "); // clears redundancy counters on the readout boards
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 3, "0x2: mast-rst"); // general reset of the whole front-end logic
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 4, "0x3: spare   "); // reserved
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 5, "0x4: trigg. 0"); // Default physics readout, all det support required
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 6, "0x5: trigg. 1"); //
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 7, "0x6: trigg. 2"); //
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 8, "0x7: trigg. 3"); //
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 9, "0x8: puls.  0"); //
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(10, "0x9: puls.  1"); //
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(11, "0xA: puls.  2"); //
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(12, "0xB: puls.  3"); //
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(13, "0xC: config  "); // housekeeping trigger: return geographic info of FE
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(14, "0xD: abort   "); // aborts and clears an active event
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(15, "0xE: L1accept"); //
      fvhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(16, "0xF: L2accept"); //
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 1, "0x0:  0"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 2, "0x1:  1"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 3, "0x2:  2"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 4, "0x3:  3"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 5, "0x4:  4"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 6, "0x5:  5"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 7, "0x6:  6"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 8, "0x7:  7"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 9, "0x8:  8"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(10, "0x9:  9"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(11, "0xA: 10"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(12, "0xB: 11"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(13, "0xC: 12"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(14, "0xD: 13"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(15, "0xE: 14"); // To be filled at STAR
      fvhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(16, "0xF: 15"); // To be filled at STAR
#ifdef USE_HTTP_SERVER
      if (server)
         server->Register("/StarRaw", fvhCmdDaqVsTrig[ uGdpb ] );
#endif

      name = Form( "hStarTokenEvo_gDPB_%02u", uGdpb);
      title = Form( "STAR token value VS time gDPB %02u; Time in Run [s] ; STAR Token; Counts", uGdpb);
      fvhStarTokenEvo.push_back( new TH2I(name, title, fuHistoryHistoSize, 0, fuHistoryHistoSize, 410, 0, 4100 ) );


      name = Form( "hStarTrigGdpbTsEvo_gDPB_%02u", uGdpb);
      title = Form( "gDPB TS in STAR triger tokens for gDPB %02u; Time in Run [s] ; gDPB TS;", uGdpb);
      fvhStarTrigGdpbTsEvo.push_back( new TProfile(name, title, fuHistoryHistoSize, 0, fuHistoryHistoSize ) );

      name = Form( "hStarTrigStarTsEvo_gDPB_%02u", uGdpb);
      title = Form( "STAR TS in STAR triger tokens for gDPB %02u; Time in Run [s] ; STAR TS;", uGdpb);
      fvhStarTrigStarTsEvo.push_back( new TProfile(name, title, fuHistoryHistoSize, 0, fuHistoryHistoSize ) );
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

   /*******************************************************************/
   /// Module TOT map
   for( UInt_t uMod = 0; uMod < fuNrOfModules; uMod ++ )
   {
      name = Form("RemapTotSideA_mod_%02u", uMod);
      title = Form("Raw TOT module %02u Side A; PADI channel; TOT [bin]", uMod);
      fvhRemapTotSideA_mod.push_back(
         new TH2F(name.Data(), title.Data(),
            kuNbFeeSide * fuNrOfChannelsPerFee, 0, kuNbFeeSide * fuNrOfChannelsPerFee,
            256, 0, 256 ) );
      name = Form("RemapTotSideB_mod_%02u", uMod);
      title = Form("Raw TOT module %02u Side B; PADI channel; TOT [bin]", uMod);
      fvhRemapTotSideB_mod.push_back(
         new TH2F(name.Data(), title.Data(),
            kuNbFeeSide * fuNrOfChannelsPerFee, 0, kuNbFeeSide * fuNrOfChannelsPerFee,
            256, 0, 256 ) );
   } // for( UInt_t uMod = 0; uMod < fuNrOfModules; uMod ++ )

   /*******************************************************************/
   /// FEE pulser test channels
   if( kTRUE == fbPulserModeEnable )
   {
      fvhTimeDiffPulser.resize( fuNrOfFeePerGdpb * fuNrOfGdpbs );
      for( UInt_t uFeeA = 0; uFeeA < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeA++)
      {
         fvhTimeDiffPulser[uFeeA].resize( fuNrOfFeePerGdpb * fuNrOfGdpbs );
         for( UInt_t uFeeB = 0; uFeeB < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeB++)
         {
            if( uFeeA < uFeeB )
            {
               UInt_t uGdpbA = uFeeA / ( fuNrOfFeePerGdpb );
               UInt_t uFeeIdA = uFeeA - ( fuNrOfFeePerGdpb * uGdpbA );
               UInt_t uGdpbB = uFeeB / ( fuNrOfFeePerGdpb );
               UInt_t uFeeIdB = uFeeB - ( fuNrOfFeePerGdpb * uGdpbB );
               fvhTimeDiffPulser[uFeeA][uFeeB] = new TH1I(
                  Form("hTimeDiffPulser_g%02u_f%1u_g%02u_f%1u", uGdpbA, uFeeIdA, uGdpbB, uFeeIdB),
                  Form("Time difference for pulser on gDPB %02u FEE %1u and gDPB %02u FEE %1u; DeltaT [ps]; Counts",
                        uGdpbA, uFeeIdA, uGdpbB, uFeeIdB ),
                  uNbBinsDt, dMinDt, dMaxDt);
            } // if( uFeeA < uFeeB )
               else fvhTimeDiffPulser[uFeeA][uFeeB] = NULL;
         } // for( UInt_t uFeeB = uFeeA; uFeeB < fuNrOfFeePerGdpb * fuNrOfGdpbs - 1; uFeeB++)
      } // for( UInt_t uFeeA = 0; uFeeA < kuNbChanTest - 1; uFeeA++)

      name = "hTimeMeanPulser";
      fhTimeMeanPulser = new TH2D( name.Data(),
            "Time difference Mean for each FEE pairs; FEE A; FEE B ; Mean [ps]",
            fuNrOfFeePerGdpb * fuNrOfGdpbs - 1, -0.5, fuNrOfFeePerGdpb * fuNrOfGdpbs - 1.5,
            fuNrOfFeePerGdpb * fuNrOfGdpbs - 1,  0.5, fuNrOfFeePerGdpb * fuNrOfGdpbs - 0.5 );

      name = "hTimeRmsPulser";
      fhTimeRmsPulser = new TH2D( name.Data(),
            "Time difference RMS for each FEE pairs; FEE A; FEE B ; RMS [ps]",
            fuNrOfFeePerGdpb * fuNrOfGdpbs - 1, -0.5, fuNrOfFeePerGdpb * fuNrOfGdpbs - 1.5,
            fuNrOfFeePerGdpb * fuNrOfGdpbs - 1,  0.5, fuNrOfFeePerGdpb * fuNrOfGdpbs - 0.5 );

      name = "hTimeRmsZoomFitPuls";
      fhTimeRmsZoomFitPuls = new TH2D( name.Data(),
            "Time difference RMS after zoom for each FEE pairs; FEE A; FEE B ; RMS [ps]",
            fuNrOfFeePerGdpb * fuNrOfGdpbs - 1, -0.5, fuNrOfFeePerGdpb * fuNrOfGdpbs - 1.5,
            fuNrOfFeePerGdpb * fuNrOfGdpbs - 1,  0.5, fuNrOfFeePerGdpb * fuNrOfGdpbs - 0.5 );

      name = "hTimeResFitPuls";
      fhTimeResFitPuls = new TH2D( name.Data(),
            "Time difference Res from fit for each FEE pairs; FEE A; FEE B ; Sigma [ps]",
            fuNrOfFeePerGdpb * fuNrOfGdpbs - 1, -0.5, fuNrOfFeePerGdpb * fuNrOfGdpbs - 1.5,
            fuNrOfFeePerGdpb * fuNrOfGdpbs - 1,  0.5, fuNrOfFeePerGdpb * fuNrOfGdpbs - 0.5 );

      fvhPulserTimeDiffEvoGbtxGbtx.resize( fuNrOfGdpbs * (kuNbGbtxPerGdpb - 1) );
      fvvhPulserTimeDiffEvoGdpbGdpb.resize( fuNrOfGdpbs );
      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      {
         for( UInt_t uGbtx = 0; uGbtx < kuNbGbtxPerGdpb - 1; ++uGbtx )
         {
            name = Form("hPulserTimeDiffEvoGdpb%02uGbtx00Gbtx%02u", uGdpb, uGbtx + 1 );
            fvhPulserTimeDiffEvoGbtxGbtx[ uGdpb * (kuNbGbtxPerGdpb - 1)  + uGbtx ] = new TProfile( name.Data(),
               Form( "Time difference of the 1st FEE in the 1st GBTx of gDPB %02u vs GBTx %02u; time in run [min]; dt [ps]",
                     uGdpb, uGbtx + 1 ),
               fuHistoryHistoSizeLong, 0, fuHistoryHistoSizeLong );
         } // for( UInt_t uGbtx = 0; uGbtx < kuNbGbtxPerGdpb; ++uGbtx )

         fvvhPulserTimeDiffEvoGdpbGdpb[ uGdpb ].resize( fuNrOfGdpbs, NULL );
         for( UInt_t uGdpbB = uGdpb + 1; uGdpbB < fuNrOfGdpbs; ++uGdpbB )
         {
            name = Form("hPulserTimeDiffEvoGdpb%02uGdpb%02u", uGdpb, uGdpbB );
            fvvhPulserTimeDiffEvoGdpbGdpb[ uGdpb ][ uGdpbB ] = new TProfile( name.Data(),
               Form( "Time difference of the 1st FEE in the 1st GBTx of gDPB %02u vs %02u; time in run [min]; dt [ps]",
                     uGdpb, uGdpbB ),
               fuHistoryHistoSizeLong, 0, fuHistoryHistoSizeLong );
         } // for( UInt_t uGdpbB = uGdpb + 1; uGdpbB < fuNrOfGdpbs; ++uGdpbB )
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   } // if( kTRUE == fbPulserModeEnable )

   /// Coincidence maps
   if( kTRUE == fbCoincMapsEnable )
   {
      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      {
         name = Form("hCoincMapAllChanGdpb%02u", uGdpb );
         fvhCoincMapAllChanGdpb.push_back( new TH2D( name.Data(),
               Form( "Coincidence map of all channels of gDPB %02u; Chan A []; Chan B[]; Coinc. []", uGdpb ),
               fuNrOfChannelsPerGdpb, -0.5, fuNrOfChannelsPerGdpb - 0.5,
               fuNrOfChannelsPerGdpb, -0.5, fuNrOfChannelsPerGdpb - 0.5 ) );

         name = Form("hCoincMeanAllChanGdpb%02u", uGdpb );
         fvhCoincMeanAllChanGdpb.push_back( new TProfile2D( name.Data(),
               Form( "Coincidence mean of all channels of gDPB %02u; Chan A []; Chan B[]; Mean dt [ps]", uGdpb ),
               fuNrOfChannelsPerGdpb, -0.5, fuNrOfChannelsPerGdpb - 0.5,
               fuNrOfChannelsPerGdpb, -0.5, fuNrOfChannelsPerGdpb - 0.5) );
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   } // if( kTRUE == fbCoincMapsEnable )

#ifdef USE_HTTP_SERVER
   if( server )
   {
      server->Register("/TofRaw", fhMessType );
      server->Register("/TofRaw", fhSysMessType );
      server->Register("/TofRaw", fhGdpbMessType );
      server->Register("/TofRaw", fhGdpbSysMessType );
      server->Register("/TofRaw", fhGet4MessType );
      server->Register("/TofRaw", fhGet4ChanScm );
      server->Register("/TofRaw", fhGet4ChanErrors );
      server->Register("/TofRaw", fhGet4EpochFlags );
      server->Register("/TofRaw", fhScmScalerCounters );
      server->Register("/TofRaw", fhScmDeadtimeCounters );
      server->Register("/TofRaw", fhScmSeuCounters );
      server->Register("/TofRaw", fhScmSeuCountersEvo );

      for( UInt_t uTotPlot = 0; uTotPlot < fvhRawTot_gDPB.size(); ++uTotPlot )
         server->Register("/TofRaw", fvhRawTot_gDPB[ uTotPlot ] );

      for( UInt_t uTotPlot = 0; uTotPlot < fvhRemapTot_gDPB.size(); ++uTotPlot )
         server->Register("/TofRaw", fvhRemapTot_gDPB[ uTotPlot ] );

      for( UInt_t uMod = 0; uMod < fuNrOfModules; uMod ++ )
      {
         server->Register("/TofRaw", fvhRemapTotSideA_mod[ uMod ] );
         server->Register("/TofRaw", fvhRemapTotSideB_mod[ uMod ] );
	   } // for( UInt_t uMod = 0; uMod < fuNrOfModules; uMod ++ )

      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      {
         server->Register("/TofRaw", fvhRawFt_gDPB[ uGdpb ] );
         server->Register("/TofRaw", fvhChCount_gDPB[ uGdpb ] );
         server->Register("/TofRates", fvhChannelRate_gDPB[ uGdpb ] );
         server->Register("/TofRaw", fvhRemapChCount_gDPB[ uGdpb ] );
         server->Register("/TofRates", fvhRemapChRate_gDPB[ uGdpb ] );

         for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++ uFee)
         {
            server->Register("/TofRates", fvhFeeRate_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ] );
            server->Register("/TofRates", fvhFeeErrorRate_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ] );
            server->Register("/TofRates", fvhFeeErrorRatio_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ] );
            server->Register("/TofRates", fvhFeeRateLong_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ] );
            server->Register("/TofRates", fvhFeeErrorRateLong_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ] );
            server->Register("/TofRates", fvhFeeErrorRatioLong_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ] );
         } // for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++ uFee)

         server->Register("/StarRaw", fvhTokenMsgType[ uGdpb ] );
         server->Register("/StarRaw", fvhTriggerRate[ uGdpb ] );
         server->Register("/StarRaw", fvhCmdDaqVsTrig[ uGdpb ] );
         server->Register("/StarRaw", fvhStarTokenEvo[ uGdpb ] );
         server->Register("/StarRaw", fvhStarTrigGdpbTsEvo[ uGdpb ] );
         server->Register("/StarRaw", fvhStarTrigStarTsEvo[ uGdpb ] );
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

      if( kTRUE == fbPulserModeEnable )
      {
         for( UInt_t uFeeA = 0; uFeeA < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeA++)
            for( UInt_t uFeeB = 0; uFeeB < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeB++)
               if( NULL != fvhTimeDiffPulser[uFeeA][uFeeB] )
                  server->Register("/TofDt", fvhTimeDiffPulser[uFeeA][uFeeB] );

         server->Register("/TofRaw", fhTimeMeanPulser );
         server->Register("/TofRaw", fhTimeRmsPulser );
         server->Register("/TofRaw", fhTimeRmsZoomFitPuls );
         server->Register("/TofRaw", fhTimeResFitPuls );

         for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
         {
            for( UInt_t uGbtx = 0; uGbtx < kuNbGbtxPerGdpb - 1; ++uGbtx )
               if( NULL != fvhPulserTimeDiffEvoGbtxGbtx[ uGdpb * (kuNbGbtxPerGdpb - 1)  + uGbtx ] )
                  server->Register("/TofDtEvo", fvhPulserTimeDiffEvoGbtxGbtx[ uGdpb * (kuNbGbtxPerGdpb - 1)  + uGbtx ] );

            for( UInt_t uGdpbB = uGdpb + 1; uGdpbB < fuNrOfGdpbs; ++uGdpbB )
               if( NULL != fvvhPulserTimeDiffEvoGdpbGdpb[uGdpb ][ uGdpbB] )
                  server->Register("/TofDtEvo", fvvhPulserTimeDiffEvoGdpbGdpb[uGdpb ][ uGdpbB] );
         } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      } // if( kTRUE == fbPulserModeEnable )
      if( kTRUE == fbCoincMapsEnable )
      {
         for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
         {
            server->Register("/TofCoinc", fvhCoincMapAllChanGdpb[ uGdpb ] );
            server->Register("/TofCoinc", fvhCoincMeanAllChanGdpb[ uGdpb ] );
         } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      } // if( kTRUE == fbCoincMapsEnable )

      server->RegisterCommand("/Reset_All_eTOF", "bMcbmMoniTofResetHistos=kTRUE");
      server->RegisterCommand("/Save_All_eTof",  "bMcbmMoniTofSaveHistos=kTRUE");
      server->RegisterCommand("/Update_PulsFit", "bMcbmMoniTofUpdateZoomedFit=kTRUE");
      server->RegisterCommand("/Print_Raw_Data", "bMcbmMoniTofRawDataPrint=kTRUE");
      server->RegisterCommand("/Print_AllHits",  "bMcbmMoniTofPrintAllHitsEna=kTRUE");
      server->RegisterCommand("/Print_AllEps",   "bMcbmMoniTofPrintAllEpochsEna=kTRUE");

      server->Restrict("/Reset_All_eTof", "allow=admin");
      server->Restrict("/Save_All_eTof",  "allow=admin");
      server->Restrict("/Update_PulsFit", "allow=admin");
      server->Restrict("/Print_Raw_Data", "allow=admin");
      server->Restrict("/Print_AllHits",  "allow=admin");
      server->Restrict("/Print_AllEps",   "allow=admin");
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
   fhMessType->Draw();

   cSummary->cd(2);
   gPad->SetLogy();
   fhSysMessType->Draw();

   cSummary->cd(3);
   gPad->SetLogz();
   fhGet4MessType->Draw("colz");

   // 2nd Column: GET4 Errors + Epoch flags + SCm
   cSummary->cd(4);
   gPad->SetLogz();
   fhGet4ChanErrors->Draw("colz");

   cSummary->cd(5);
   gPad->SetLogz();
   fhGet4EpochFlags->Draw("colz");

   cSummary->cd(6);
   fhGet4ChanScm->Draw("colz");
   /*****************************/

   /** Create FEE rates Canvas for STAR 2018 **/
   TCanvas* cFeeRates = new TCanvas("cFeeRates", "gDPB Monitoring FEE rates", w, h);
//   cFeeRates->Divide(fuNrOfFeePerGdpb, fuNrOfGdpbs );
   cFeeRates->Divide(kuNbFeePerGbtx, kuNbGbtxPerGdpb * fuNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++uFee )
      {
         cFeeRates->cd( 1 + uGdpb * fuNrOfFeePerGdpb + uFee );
         gPad->SetLogy();
         fvhFeeRate_gDPB[uGdpb * fuNrOfFeePerGdpb + uFee]->Draw("hist");

         fvhFeeErrorRate_gDPB[uGdpb * fuNrOfFeePerGdpb + uFee]->SetLineColor( kRed );
         fvhFeeErrorRate_gDPB[uGdpb * fuNrOfFeePerGdpb + uFee]->Draw("same hist");
      } // for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++uFee )
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   /*****************************/

   /** Create FEE error ratio Canvas for STAR 2018 **/
   TCanvas* cFeeErrRatio = new TCanvas("cFeeErrRatio", "gDPB Monitoring FEE error ratios", w, h);
//   cFeeErrRatio->Divide(fuNrOfFeePerGdpb, fuNrOfGdpbs );
   cFeeErrRatio->Divide(kuNbFeePerGbtx, kuNbGbtxPerGdpb * fuNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++uFee )
      {
         cFeeErrRatio->cd( 1 + uGdpb * fuNrOfFeePerGdpb + uFee );
         gPad->SetLogy();
         fvhFeeErrorRatio_gDPB[uGdpb * fuNrOfFeePerGdpb + uFee]->Draw( "hist le0");
      } // for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++uFee )
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   /*****************************/


   /** Create FEE rates long Canvas for STAR 2018 **/
   TCanvas* cFeeRatesLong = new TCanvas("cFeeRatesLong", "gDPB Monitoring FEE rates", w, h);
//   cFeeRatesLong->Divide(fuNrOfFeePerGdpb, fuNrOfGdpbs );
   cFeeRatesLong->Divide(kuNbFeePerGbtx, kuNbGbtxPerGdpb * fuNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++uFee )
      {
         cFeeRatesLong->cd( 1 + uGdpb * fuNrOfFeePerGdpb + uFee );
         gPad->SetLogy();
         fvhFeeRateLong_gDPB[uGdpb]->Draw( "hist" );

         fvhFeeErrorRateLong_gDPB[uGdpb * fuNrOfFeePerGdpb + uFee]->SetLineColor( kRed );
         fvhFeeErrorRateLong_gDPB[uGdpb * fuNrOfFeePerGdpb + uFee]->Draw("same hist");
      } // for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++uFee )
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   /*****************************/

   /** Create FEE error ratio long Canvas for STAR 2018 **/
   TCanvas* cFeeErrRatioLong = new TCanvas("cFeeErrRatioLong", "gDPB Monitoring FEE error ratios", w, h);
//   cFeeErrRatioLong->Divide(fuNrOfFeePerGdpb, fuNrOfGdpbs );
   cFeeErrRatioLong->Divide(kuNbFeePerGbtx, kuNbGbtxPerGdpb * fuNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++uFee )
      {
         cFeeErrRatioLong->cd( 1 + uGdpb * fuNrOfFeePerGdpb + uFee );
         gPad->SetLogy();
         fvhFeeErrorRatioLong_gDPB[uGdpb * fuNrOfFeePerGdpb + uFee]->Draw( "hist le0");
      } // for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++uFee )
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   /*****************************/

   /** Create channel count Canvas for STAR 2018 **/
   TCanvas* cGdpbChannelCount = new TCanvas("cGdpbChannelCount", "Integrated Get4 channel counts per gDPB", w, h);
   cGdpbChannelCount->Divide( 1, fuNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      cGdpbChannelCount->cd( 1 + uGdpb );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogy();
      fvhChCount_gDPB[ uGdpb ]->Draw();
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   /*****************************/

   /** Create remapped channel count Canvas for STAR 2018 **/
   TCanvas* cGdpbRemapChCount = new TCanvas("cGdpbRemapChCount", "Integrated PADI channel counts per gDPB", w, h);
   cGdpbRemapChCount->Divide( 1, fuNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      cGdpbRemapChCount->cd( 1 + uGdpb );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogy();
      fvhRemapChCount_gDPB[ uGdpb ]->Draw();
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   /*****************************/

   /** Create channel rate Canvas for STAR 2018 **/
   TCanvas* cGdpbChannelRate = new TCanvas("cGdpbChannelRate", "Get4 channel rate per gDPB", w, h);
   cGdpbChannelRate->Divide( 1, fuNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      cGdpbChannelRate->cd( 1 + uGdpb );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogz();
      fvhChannelRate_gDPB[ uGdpb ]->Draw( "colz" );
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   /*****************************/

   /** Create remapped rate count Canvas for STAR 2018 **/
   TCanvas* cGdpbRemapChRate = new TCanvas("cGdpbRemapChRate", "PADI channel rate per gDPB", w, h);
   cGdpbRemapChRate->Divide( 1, fuNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      cGdpbRemapChRate->cd( 1 + uGdpb );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogz();
      fvhRemapChRate_gDPB[ uGdpb ]->Draw( "colz" );
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   /*****************************/

   /** Create TOT Canvas(es) for STAR 2018 **/
   TCanvas* cTotPnt = NULL;
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      cTotPnt = new TCanvas( Form("cTot_g%02u", uGdpb),
                             Form("gDPB %02u TOT distributions", uGdpb),
                             w, h);

      cTotPnt->cd( );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogz();

      fvhRawTot_gDPB[ uGdpb ]->Draw( "colz" );
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   cTotPnt  = new TCanvas( "cTot_all", "TOT distributions", w, h);
   cTotPnt->Divide( fuNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      cTotPnt->cd( 1 + uGdpb );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogz();

      fvhRawTot_gDPB[ uGdpb]->Draw( "colz" );
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

   /**************************************************/

   /** Create PADI TOT Canvas(es) for STAR 2018 **/
   cTotPnt = NULL;
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      cTotPnt = new TCanvas( Form("cTotRemap_g%02u", uGdpb),
                             Form("PADI ch gDPB %02u TOT distributions", uGdpb),
                             w, h);

      cTotPnt->cd( );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogz();

      fvhRemapTot_gDPB[ uGdpb ]->Draw( "colz" );
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   cTotPnt  = new TCanvas( "cTotRemap_all", "TOT distributions", w, h);
   cTotPnt->Divide( fuNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      cTotPnt->cd( 1 + uGdpb );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogz();

      fvhRemapTot_gDPB[ uGdpb ]->Draw( "colz" );
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

   /**************************************************/

   /** Create Side TOT Canvas(es) for STAR 2018 **/
   cTotPnt = NULL;
   for( UInt_t uMod = 0; uMod < fuNrOfModules; uMod ++ )
   {
      cTotPnt = new TCanvas( Form("cTotRemapSides_m%02u", uMod),
                             Form("Sides ch module %02u TOT distributions", uMod),
                             w, h);
      cTotPnt->Divide( 1, 2 );

      cTotPnt->cd( 1 );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogz();

      fvhRemapTotSideA_mod[ uMod ]->Draw( "colz" );

      cTotPnt->cd( 2 );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogz();
      fvhRemapTotSideB_mod[ uMod ]->Draw( "colz" );
   } // for( UInt_t uMod = 0; uMod < fuNrOfModules; uMod ++ )

   /**************************************************/

   /** Create STAR token Canvas for STAR 2018 **/
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
   {
      TCanvas* cStarToken = new TCanvas( Form("cStarToken_g%02u", uGdpb),
                                           Form("STAR token detection info for gDPB %02u", uGdpb),
                                           w, h);
      cStarToken->Divide( 2, 2 );

      cStarToken->cd(1);
      fvhTriggerRate[uGdpb]->Draw();

      cStarToken->cd(2);
      fvhCmdDaqVsTrig[uGdpb]->Draw( "colz" );

      cStarToken->cd(3);
      fvhStarTokenEvo[uGdpb]->Draw();

      cStarToken->cd(4);
      fvhStarTrigGdpbTsEvo[uGdpb]->Draw( "hist le0" );
      fvhStarTrigStarTsEvo[uGdpb]->SetLineColor( kRed );
      fvhStarTrigStarTsEvo[uGdpb]->Draw( "same hist le0" );
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
   /*****************************/

   if( kTRUE == fbPulserModeEnable )
   {
      /** Create Pulser check Canvas for STAR 2018 **/
      TCanvas* cPulser = new TCanvas("cPulser", "Time difference RMS for pulser channels when FEE pulser mode is ON", w, h);
      cPulser->Divide( 2, 2 );

      cPulser->cd(1);
      gPad->SetGridx();
      gPad->SetGridy();
      fhTimeRmsPulser->Draw( "colz" );

      cPulser->cd(2);
      gPad->SetGridx();
      gPad->SetGridy();
      fhTimeMeanPulser->Draw( "colz" );

      cPulser->cd(3);
      gPad->SetGridx();
      gPad->SetGridy();
      fhTimeRmsZoomFitPuls->Draw( "colz" );

      cPulser->cd(4);
      gPad->SetGridx();
      gPad->SetGridy();
      fhTimeResFitPuls->Draw( "colz" );
      /*****************************/

      /** Create Pulser evo Canvas for gDPB to gDPB **/
      TCanvas* cPulserEvo = new TCanvas("cPulserEvo", "Time difference evolution between 1st FEE of 1st GBTx of gDPB pairs", w, h);
      cPulserEvo->Divide( 1, fuNrOfGdpbs - 1 );
      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs - 1; uGdpb ++)
      {
         cPulserEvo->cd( 1 + uGdpb );
         gPad->SetGridx();
         gPad->SetGridy();
         if( NULL != fvvhPulserTimeDiffEvoGdpbGdpb[uGdpb ][ uGdpb + 1] )
            fvvhPulserTimeDiffEvoGdpbGdpb[uGdpb ][ uGdpb + 1]->Draw( );

      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs - 1; uGdpb ++)
      /*****************************/
      /** Create Pulser evo Canvas within gDPB **/
      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
      {
         TCanvas* cPulserEvoGbtx = new TCanvas( Form("cPulserEvoGbtx%02u", uGdpb ),
                                                Form("Time difference evolution between 1st FEE of GBTx pairs in gDPB %02u", uGdpb),
                                                w, h);
         cPulserEvoGbtx->Divide( 1, kuNbGbtxPerGdpb - 1 );

         for( UInt_t uGbtx = 0; uGbtx < kuNbGbtxPerGdpb - 1; ++uGbtx )
         {
            cPulserEvoGbtx->cd( 1 + uGbtx );
            gPad->SetGridx();
            gPad->SetGridy();

            if( NULL != fvhPulserTimeDiffEvoGbtxGbtx[ uGdpb * (kuNbGbtxPerGdpb - 1)  + uGbtx ] )
               fvhPulserTimeDiffEvoGbtxGbtx[ uGdpb * (kuNbGbtxPerGdpb - 1)  + uGbtx ]->Draw( );
         } // for( UInt_t uGbtx = 0; uGbtx < kuNbGbtxPerGdpb - 1; ++uGbtx )
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
      /*****************************/
   } // if( kTRUE == fbPulserModeEnable )

   /** Recovers/Create Ms Size Canvas for STAR 2018 **/
   // Try to recover canvas in case it was created already by another monitor
   // If not existing, create it
   fcMsSizeAll = dynamic_cast<TCanvas *>( gROOT->FindObject( "cMsSizeAll" ) );
   if( NULL == fcMsSizeAll )
   {
      fcMsSizeAll = new TCanvas("cMsSizeAll", "Evolution of MS size in last 300 s", w, h);
      fcMsSizeAll->Divide( 4, 3 );
      LOG(INFO) << "Created MS size canvas in TOF monitor" << FairLogger::endl;
   } // if( NULL == fcMsSizeAll )
      else LOG(INFO) << "Recovered MS size canvas in TOF monitor" << FairLogger::endl;

  LOG(INFO) << "Leaving CreateHistograms" << FairLogger::endl;
}

Bool_t CbmMcbm2018MonitorTof::DoUnpack(const fles::Timeslice& ts,
    size_t component)
{
   if( bMcbmMoniTofResetHistos )
   {
      LOG(INFO) << "Reset eTOF STAR histos " << FairLogger::endl;
      ResetAllHistos();
      bMcbmMoniTofResetHistos = kFALSE;
   } // if( bMcbmMoniTofResetHistos )
   if( bMcbmMoniTofSaveHistos )
   {
      LOG(INFO) << "Start saving eTOF STAR histos " << FairLogger::endl;
      SaveAllHistos( "data/histos_Shift_StarTof.root" );
      bMcbmMoniTofSaveHistos = kFALSE;
   } // if( bSaveStsHistos )
   if( bMcbmMoniTofUpdateZoomedFit )
   {
      UpdateZoomedFit();
      bMcbmMoniTofUpdateZoomedFit = kFALSE;
   } // if (bMcbmMoniTofUpdateZoomedFit)
   if( bMcbmMoniTofRawDataPrint )
   {
      fuRawDataPrintMsgIdx = 0;
      bMcbmMoniTofRawDataPrint = kFALSE;
   } // if( bMcbmMoniTofRawDataPrint )
   if( bMcbmMoniTofPrintAllHitsEna )
   {
      fbPrintAllHitsEnable = !fbPrintAllHitsEnable;
      bMcbmMoniTofPrintAllHitsEna = kFALSE;
   } // if( bMcbmMoniTofPrintAllHitsEna )
   if( bMcbmMoniTofPrintAllEpochsEna )
   {
      fbPrintAllEpochsEnable = !fbPrintAllEpochsEnable;
      bMcbmMoniTofPrintAllHitsEna = kFALSE;
   } // if( bMcbmMoniTofPrintAllEpochsEna )

   /// Periodically save the histograms
   std::chrono::time_point<std::chrono::system_clock> timeCurrent = std::chrono::system_clock::now();
   std::chrono::duration<double> elapsed_seconds = timeCurrent - fTimeLastHistoSaving;
   if( 0 == fTimeLastHistoSaving.time_since_epoch().count() )
   {
      fTimeLastHistoSaving = timeCurrent;
//      fulNbBuiltSubEventLastPrintout = fulNbBuiltSubEvent;
//      fulNbStarSubEventLastPrintout  = fulNbStarSubEvent;
   } // if( 0 == fTimeLastHistoSaving.time_since_epoch().count() )
   else if( 300 < elapsed_seconds.count() )
   {
      std::time_t cTimeCurrent = std::chrono::system_clock::to_time_t( timeCurrent );
      char tempBuff[80];
      std::strftime( tempBuff, 80, "%F %T", localtime (&cTimeCurrent) );
/*
      LOG(INFO) << "CbmTofStarEventBuilder2018::DoUnpack => " << tempBuff
               << " Total number of Built events: " << std::setw(9) << fulNbBuiltSubEvent
               << ", " << std::setw(9) << (fulNbBuiltSubEvent - fulNbBuiltSubEventLastPrintout)
               << " events in last " << std::setw(4) << elapsed_seconds.count() << " s"
               << FairLogger::endl;
      fTimeLastPrintoutNbStarEvent = timeCurrent;
      fulNbBuiltSubEventLastPrintout   = fulNbBuiltSubEvent;
*/
      fTimeLastHistoSaving = timeCurrent;
      SaveAllHistos( "data/histos_shift.root" );
   } // else if( 300 < elapsed_seconds.count() )

   LOG(DEBUG1) << "Timeslice contains " << ts.num_microslices(component)
                 << "microslices." << FairLogger::endl;

   /// Ignore overlap ms if flag set by user
   UInt_t uNbMsLoop = fuNbCoreMsPerTs;
   if( kFALSE == fbIgnoreOverlapMs )
      uNbMsLoop += fuNbOverMsPerTs;

   Int_t messageType = -111;
   Double_t dTsStartTime = -1;

   /// Loop over core microslices (and overlap ones if chosen)
   for( UInt_t uMsIdx = 0; uMsIdx < uNbMsLoop; uMsIdx ++ )
   {
      if (fuMsAcceptsPercent < uMsIdx)
         continue;

      /// Loop over registered components
      for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsList.size(); ++uMsCompIdx )
      {
         constexpr uint32_t kuBytesPerMessage = 8;

         UInt_t uMsComp = fvMsComponentsList[ uMsCompIdx ];
         auto msDescriptor = ts.descriptor( uMsComp, uMsIdx );
         fiEquipmentId = msDescriptor.eq_id;
         fdMsIndex = static_cast<double>(msDescriptor.idx);
         const uint8_t* msContent = reinterpret_cast<const uint8_t*>( ts.content( uMsComp, uMsIdx ) );

         uint32_t size = msDescriptor.size;
//    fulLastMsIdx = msDescriptor.idx;
         if (size > 0)
            LOG(DEBUG) << "Microslice: " << msDescriptor.idx << " has size: " << size
                       << FairLogger::endl;
/*
         if( numCompMsInTs - fuOverlapMsNb <= m )
         {
//         LOG(INFO) << "Ignore overlap Microslice: " << msDescriptor.idx << FairLogger::endl;
            continue;
         } // if( numCompMsInTs - fuOverlapMsNb <= m )
*/
         if( 0 == uMsIdx && 0 == uMsCompIdx )
            dTsStartTime = (1e-9) * fdMsIndex;

         if( fdStartTimeMsSz < 0 )
            fdStartTimeMsSz = (1e-9) * fdMsIndex;
         fvhMsSzPerLink[ uMsComp ]->Fill(size);
         if( 2 * fuHistoryHistoSize < (1e-9) * fdMsIndex - fdStartTimeMsSz )
         {
            // Reset the evolution Histogram and the start time when we reach the end of the range
            fvhMsSzTimePerLink[ uMsComp ]->Reset();
            fdStartTimeMsSz = (1e-9) * fdMsIndex;
         } // if( 2 * fuHistoryHistoSize < (1e-9) * fdMsIndex - fdStartTimeMsSz )
         fvhMsSzTimePerLink[ uMsComp ]->Fill((1e-9) * fdMsIndex - fdStartTimeMsSz, size);

         // If not integer number of message in input buffer, print warning/error
         if (0 != (size % kuBytesPerMessage))
            LOG(ERROR) << "The input microslice buffer does NOT "
                       << "contain only complete nDPB messages!"
                       << FairLogger::endl;

         // Compute the number of complete messages in the input microslice buffer
         uint32_t uNbMessages = (size - (size % kuBytesPerMessage)) / kuBytesPerMessage;

////////////////////////////////////////////////////////////////////////
//                   FINAL SOLUTION                                   //
////////////////////////////////////////////////////////////////////////

         // Get the gDPB ID from the MS header
         fuGdpbId = fiEquipmentId;

         /// Check if this gDPB ID was declared in parameter file and stop there if not
         auto it = fGdpbIdIndexMap.find( fuGdpbId );
         if( it == fGdpbIdIndexMap.end() )
         {
            LOG(FATAL) << "Could not find the gDPB index for AFCK id 0x"
                      << std::hex << fuGdpbId << std::dec
                      << " in microslice " << fdMsIndex
                      << FairLogger::endl
                      << "If valid this index has to be added in the TOF parameter file in the RocIdArray field"
                      << FairLogger::endl;
            continue;
         } // if( it == fGdpbIdIndexMap.end() )
            else fuGdpbNr = fGdpbIdIndexMap[ fuGdpbId ];

////////////////////////////////////////////////////////////////////////
//                   FINAL SOLUTION                                   //
////////////////////////////////////////////////////////////////////////

         // Prepare variables for the loop on contents
         const uint64_t* pInBuff = reinterpret_cast<const uint64_t*>(msContent);
         for( uint32_t uIdx = 0; uIdx < uNbMessages; uIdx++ )
         {
            // Fill message
            uint64_t ulData = static_cast<uint64_t>(pInBuff[uIdx]);
            gdpbv100::Message mess(ulData);

            if( fuRawDataPrintMsgIdx < fuRawDataPrintMsgNb || gLogger->IsLogNeeded(DEBUG2) )
            {
               mess.printDataCout();
               fuRawDataPrintMsgIdx ++;
            } // if( fuRawDataPrintMsgIdx < fuRawDataPrintMsgNb || gLogger->IsLogNeeded(DEBUG2) )

////////////////////////////////////////////////////////////////////////
//                   TEMP SOLUTION                                    //
////////////////////////////////////////////////////////////////////////
/*
         fuGdpbId = mess.getGdpbGenGdpbId();

         /// Check if this gDPB ID was declared in parameter file and stop there if not
         auto it = fGdpbIdIndexMap.find( fuGdpbId );
         if( it == fGdpbIdIndexMap.end() )
         {
            LOG(FATAL) << "Could not find the gDPB index for AFCK id 0x"
                      << std::hex << fuGdpbId << std::dec
                      << " in microslice " << fdMsIndex
                      << FairLogger::endl
                      << "If valid this index has to be added in the TOF parameter file in the RocIdArray field"
                      << FairLogger::endl;
            continue;
         } // if( it == fGdpbIdIndexMap.end() )
            else fuGdpbNr = fGdpbIdIndexMap[ fuGdpbId ];
*/
////////////////////////////////////////////////////////////////////////
//                   TEMP SOLUTION                                    //
////////////////////////////////////////////////////////////////////////

            // Increment counter for different message types
            // and fill the corresponding histogram
            messageType = mess.getMessageType();
            fviMsgCounter[messageType]++;
            fhMessType->Fill(messageType);
            fhGdpbMessType->Fill(messageType, fuGdpbNr );

   ///         fuGet4Id = mess.getGdpbGenChipId();
            fuGet4Id = ConvertElinkToGet4( mess.getGdpbGenChipId() );
            fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + fuGet4Id;

            if( fuNrOfGet4PerGdpb <= fuGet4Id &&
                !mess.isStarTrigger()  &&
                ( gdpbv100::kuChipIdMergedEpoch != fuGet4Id ) )
               LOG(WARNING) << "Message with Get4 ID too high: " << fuGet4Id
                            << " VS " << fuNrOfGet4PerGdpb << " set in parameters." << FairLogger::endl;

            switch (messageType)
            {
               case gdpbv100::MSG_HIT:
               {
                  if( mess.getGdpbHitIs24b() )
                  {
                     fhGet4MessType->Fill(fuGet4Nr, 4 );
                     PrintGenInfo(mess);
                  } // if( getGdpbHitIs24b() )
                     else
                     {
                        fhGet4MessType->Fill(fuGet4Nr, 0 );
                        fvmEpSupprBuffer[fuGet4Nr].push_back( mess );
                     } // else of if( getGdpbHitIs24b() )
                  break;
               } // case gdpbv100::MSG_HIT:
               case gdpbv100::MSG_EPOCH:
               {
                  if( gdpbv100::kuChipIdMergedEpoch == fuGet4Id )
                  {
                     for( uint32_t uGet4Index = 0; uGet4Index < fuNrOfGet4PerGdpb; uGet4Index ++ )
                     {
                        fuGet4Id = uGet4Index;
                        fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + fuGet4Id;
                        gdpbv100::Message tmpMess(mess);
                        tmpMess.setGdpbGenChipId( uGet4Index );

                        fhGet4MessType->Fill(fuGet4Nr, 1);
                        FillEpochInfo(tmpMess);
                     } // for( uint32_t uGet4Index = 0; uGet4Index < fuNrOfGet4PerGdpb; uGetIndex ++ )

                     if( kTRUE == fbPrintAllEpochsEnable )
                     {
                        LOG(INFO) << "Epoch: " << Form("0x%08x ", fuGdpbId)
                                  << ", Merg"
                                  << ", Link " << std::setw(1) << mess.getGdpbEpLinkId()
                                  << ", epoch " << std::setw(8) << mess.getGdpbEpEpochNb()
                                  << ", Sync " << std::setw(1) << mess.getGdpbEpSync()
                                  << ", Data loss " << std::setw(1) << mess.getGdpbEpDataLoss()
                                  << ", Epoch loss " << std::setw(1) << mess.getGdpbEpEpochLoss()
                                  << ", Epoch miss " << std::setw(1) << mess.getGdpbEpMissmatch()
                                  << FairLogger::endl;
                     } // if( kTRUE == fbPrintAllEpochsEnable )
                  } // if this epoch message is a merged one valid for all chips
                  else
                  {
                     fhGet4MessType->Fill(fuGet4Nr, 1);
                     FillEpochInfo(mess);

                     if( kTRUE == fbPrintAllEpochsEnable )
                     {
                        LOG(INFO) << "Epoch: " << Form("0x%08x ", fuGdpbId)
                                  << ", " << std::setw(4) << fuGet4Nr
                                  << ", Link " << std::setw(1) << mess.getGdpbEpLinkId()
                                  << ", epoch " << std::setw(8) << mess.getGdpbEpEpochNb()
                                  << ", Sync " << std::setw(1) << mess.getGdpbEpSync()
                                  << ", Data loss " << std::setw(1) << mess.getGdpbEpDataLoss()
                                  << ", Epoch loss " << std::setw(1) << mess.getGdpbEpEpochLoss()
                                  << ", Epoch miss " << std::setw(1) << mess.getGdpbEpMissmatch()
                                  << FairLogger::endl;
                     } // if( kTRUE == fbPrintAllEpochsEnable )
                  } // if single chip epoch message
                  break;
               } // case gdpbv100::MSG_EPOCH:
               case gdpbv100::MSG_SLOWC:
               {
                  fhGet4MessType->Fill(fuGet4Nr, 2);
                  PrintSlcInfo(mess);
                  break;
               } // case gdpbv100::MSG_SLOWC:
               case gdpbv100::MSG_SYST:
               {
                  fhSysMessType->Fill(mess.getGdpbSysSubType());
                  fhGdpbSysMessType->Fill(mess.getGdpbSysSubType(), fuGdpbNr );
                  if( gdpbv100::SYS_GET4_ERROR == mess.getGdpbSysSubType() )
                  {
                     fhGet4MessType->Fill(fuGet4Nr, 3);

                     UInt_t uFeeNr   = (fuGet4Id / fuNrOfGet4PerFee);
                     if (0 <= fdStartTime)
                     {
                        fvhFeeErrorRate_gDPB[(fuGdpbNr * fuNrOfFeePerGdpb) + uFeeNr]->Fill(
                           1e-9 * (mess.getMsgFullTimeD(fvulCurrentEpoch[fuGet4Nr]) - fdStartTime));
                        fvhFeeErrorRatio_gDPB[(fuGdpbNr * fuNrOfFeePerGdpb) + uFeeNr]->Fill(
                           1e-9 * (mess.getMsgFullTimeD(fvulCurrentEpoch[fuGet4Nr]) - fdStartTime), 1, 1 );
                     } // if (0 <= fdStartTime)
                     if (0 <= fdStartTimeLong)
                     {
                        fvhFeeErrorRateLong_gDPB[(fuGdpbNr * fuNrOfFeePerGdpb) + uFeeNr]->Fill(
                           1e-9 / 60.0 * (mess.getMsgFullTimeD(fvulCurrentEpoch[fuGet4Nr]) - fdStartTimeLong), 1 / 60.0);
                        fvhFeeErrorRatioLong_gDPB[(fuGdpbNr * fuNrOfFeePerGdpb) + uFeeNr]->Fill(
                           1e-9 / 60.0 * (mess.getMsgFullTimeD(fvulCurrentEpoch[fuGet4Nr]) - fdStartTimeLong), 1, 1 / 60.0);
                     } // if (0 <= fdStartTime)

                     Int_t dFullChId =  fuGet4Nr * fuNrOfChannelsPerGet4 + mess.getGdpbSysErrChanId();
                     switch( mess.getGdpbSysErrData() )
                     {
                        case gdpbv100::GET4_V2X_ERR_READ_INIT:
                           fhGet4ChanErrors->Fill(dFullChId, 0);
                           break;
                        case gdpbv100::GET4_V2X_ERR_SYNC:
                           fhGet4ChanErrors->Fill(dFullChId, 1);
                           break;
                        case gdpbv100::GET4_V2X_ERR_EP_CNT_SYNC:
                           fhGet4ChanErrors->Fill(dFullChId, 2);
                           break;
                        case gdpbv100::GET4_V2X_ERR_EP:
                           fhGet4ChanErrors->Fill(dFullChId, 3);
                           break;
                        case gdpbv100::GET4_V2X_ERR_FIFO_WRITE:
                           fhGet4ChanErrors->Fill(dFullChId, 4);
                           break;
                        case gdpbv100::GET4_V2X_ERR_LOST_EVT:
                           fhGet4ChanErrors->Fill(dFullChId, 5);
                           break;
                        case gdpbv100::GET4_V2X_ERR_CHAN_STATE:
                           fhGet4ChanErrors->Fill(dFullChId, 6);
                           break;
                        case gdpbv100::GET4_V2X_ERR_TOK_RING_ST:
                           fhGet4ChanErrors->Fill(dFullChId, 7);
                           break;
                        case gdpbv100::GET4_V2X_ERR_TOKEN:
                           fhGet4ChanErrors->Fill(dFullChId, 8);
                           break;
                        case gdpbv100::GET4_V2X_ERR_READOUT_ERR:
                           fhGet4ChanErrors->Fill(dFullChId, 9);
                           break;
                        case gdpbv100::GET4_V2X_ERR_SPI:
                           fhGet4ChanErrors->Fill(dFullChId, 10);
                           break;
                        case gdpbv100::GET4_V2X_ERR_DLL_LOCK:
                           fhGet4ChanErrors->Fill(dFullChId, 11);
                           break;
                        case gdpbv100::GET4_V2X_ERR_DLL_RESET:
                           fhGet4ChanErrors->Fill(dFullChId, 12);
                           break;
                        case gdpbv100::GET4_V2X_ERR_TOT_OVERWRT:
                           fhGet4ChanErrors->Fill(dFullChId, 13);
                           break;
                        case gdpbv100::GET4_V2X_ERR_TOT_RANGE:
                           fhGet4ChanErrors->Fill(dFullChId, 14);
                           break;
                        case gdpbv100::GET4_V2X_ERR_EVT_DISCARD:
                           fhGet4ChanErrors->Fill(dFullChId, 15);
                           break;
                        case gdpbv100::GET4_V2X_ERR_ADD_RIS_EDG:
                           fhGet4ChanErrors->Fill(dFullChId, 16);
                           break;
                        case gdpbv100::GET4_V2X_ERR_UNPAIR_FALL:
                           fhGet4ChanErrors->Fill(dFullChId, 17);
                           break;
                        case gdpbv100::GET4_V2X_ERR_SEQUENCE_ER:
                           fhGet4ChanErrors->Fill(dFullChId, 18);
                           break;
                        case gdpbv100::GET4_V2X_ERR_UNKNOWN:
                           fhGet4ChanErrors->Fill(dFullChId, 19);
                           break;
                        default: // Corrupt error or not yet supported error
                           fhGet4ChanErrors->Fill(dFullChId, 20);
                           break;
                     } // Switch( mess.getGdpbSysErrData() )
                  } // if( gdpbv100::SYSMSG_GET4_EVENT == mess.getGdpbSysSubType() )
                  PrintSysInfo(mess);
                  break;
               } // case gdpbv100::MSG_SYST:
               case gdpbv100::MSG_STAR_TRI_A:
               case gdpbv100::MSG_STAR_TRI_B:
               case gdpbv100::MSG_STAR_TRI_C:
               case gdpbv100::MSG_STAR_TRI_D:
//                  fhGet4MessType->Fill(fuGet4Nr, 5);
                  FillStarTrigInfo(mess);
                  break;
               default:
                  LOG(ERROR) << "Message type " << std::hex
                             << std::setw(2) << static_cast<uint16_t>(messageType)
                             << " not included in Get4 unpacker."
                             << FairLogger::endl;
            } // switch( mess.getMessageType() )
         } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)
      } // for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsList.size(); ++uMsCompIdx )

      ///* Pulser monitoring *///
      if( kTRUE == fbPulserModeEnable )
      {
         /// Fill the corresponding histos if the time difference is reasonnable
         for( UInt_t uFeeA = 0; uFeeA < fuNrOfFeePerGdpb * fuNrOfGdpbs; ++uFeeA)
         {
            if( 1 != fvuFeeNbHitsLastMs[ uFeeA ] )
            {
               /// Pulser should lead to single hit in a MS, so ignore this time
               fvuFeeNbHitsLastMs[ uFeeA ] = 0;
               continue;
            } // if( 1 != fvuFeeNbHitsLastMs[ uFeeA ] )

            UInt_t uGdpbNr = uFeeA / fuNrOfFeePerGdpb;
            UInt_t uGbtxNr = uFeeA / kuNbFeePerGbtx;
            /// Update the difference to all other FEE with higher indices
            for( UInt_t uFeeB = uFeeA + 1; uFeeB < fuNrOfFeePerGdpb * fuNrOfGdpbs; ++uFeeB)
            {
               if( 1 != fvuFeeNbHitsLastMs[ uFeeB ] )
                  continue;

               if( NULL != fvhTimeDiffPulser[uFeeA][uFeeB] )
               {
                  Double_t dTimeDiff = 1e3 * ( fdTsLastPulserHit[ uFeeB ] - fdTsLastPulserHit[ uFeeA ] );
                  if( TMath::Abs( dTimeDiff ) < kdMaxDtPulserPs )
                  {
                     fvhTimeDiffPulser[uFeeA][uFeeB]->Fill( dTimeDiff );

                     /// Dt Evo plots only for first FEE in each GBTx
                     if( 0 == uFeeA % kuNbFeePerGbtx && 0 == uFeeB % kuNbFeePerGbtx )
                     {
                        /// Evo of GBTx inside same DPB
                        if( uGdpbNr == uFeeB / fuNrOfFeePerGdpb )
                        {
                           if( 0 == uGbtxNr  )
                           {
                              UInt_t uPlotIdx = uGdpbNr * ( kuNbGbtxPerGdpb - 1)
                                               + ( uFeeB - uGdpbNr * fuNrOfFeePerGdpb) / kuNbFeePerGbtx - 1;
                              fvhPulserTimeDiffEvoGbtxGbtx[ uPlotIdx ]->Fill(
                                    1e-9 / 60.0 * (fdTsLastPulserHit[ uFeeA ] - fdStartTime), dTimeDiff );
                           } // if( 0 == uGbtxNr  )
                        } // if( uGdpbNr == uFeeB / fuNrOfFeePerGdpb )
                           else // if( NULL != fvvhPulserTimeDiffEvoGdpbGdpb[ uFeeB / fuNrOfFeePerGdpb ][ uGdpbNr ] )
                           {
                              /// Evo of DPBs if both first FEE in First GBTx
                              if( 0 == uGbtxNr && 0 == uFeeB / kuNbFeePerGbtx )
                                 fvvhPulserTimeDiffEvoGdpbGdpb[ uGdpbNr ][ uFeeB / fuNrOfFeePerGdpb ]->Fill(
                                    1e-9 / 60.0 * (fdTsLastPulserHit[ uFeeA ] - fdStartTime), dTimeDiff );
                           } // else of if( uGdpbNr == uFeeB / fuNrOfFeePerGdpb )
                     } // if( 0 == uFeeA % kuNbFeePerGbtx && 0 == uFeeB % kuNbFeePerGbtx )
                  } // if( TMath::Abs( dTimeDiff ) < kdMaxDtPulserPs )
      //               else if( 10 == uFeeA && 20 == uFeeB )
      //                  LOG( INFO ) << "new in 10 " << dTimeDiff
      //                              << FairLogger::endl;
               } // if( NULL != fvhTimeDiffPulser[uFeeA][uFeeB] )
            } // for( UInt_t uFeeB = uFeeA + 1; uFeeB < fuNrOfFeePerGdpb * fuNrOfGdpbs; ++uFeeB)

            /// Done with this FEE, we can reset the hit counter
            fvuFeeNbHitsLastMs[ uFeeA ] = 0;
         } // for( UInt_t uFeeA = 0; uFeeA < fuNrOfFeePerGdpb * fuNrOfGdpbs; ++uFeeA)
      } // if( kTRUE == fbPulserModeEnable )

      if( kTRUE == fbCoincMapsEnable )
      {
         for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
         {
            for( UInt_t uChanA = 0; uChanA < fuNrOfChannelsPerGdpb; ++uChanA )
            {
               if( 0 == fvuCoincNbHitsLastMs[ uGdpb ][ uChanA ] )
               {
                  /// Coinc need a hit in a MS, so ignore this time
                  continue;
               } // if( 0 == fvuCoincNbHitsLastMs[ uGdpb ][ uChanA ] )

               for( UInt_t uChanB = uChanA + 1; uChanB < fuNrOfChannelsPerGdpb; ++uChanB )
               {
                  if( 0 == fvuCoincNbHitsLastMs[ uGdpb ][ uChanB ] )
                  {
                     /// Coinc need a hit in a MS, so ignore this time
                     continue;
                  } // if( 0 == fvuCoincNbHitsLastMs[ uGdpb ][ uChanB ] )

                  Double_t dTimeDiff = 1e3 * ( fvdCoincTsLastHit[ uGdpb ][ uChanB ] - fvdCoincTsLastHit[ uGdpb ][ uChanA ] );

                  fvhCoincMeanAllChanGdpb[ uGdpb ]->Fill( uChanA, uChanB, dTimeDiff );

                  if( TMath::Abs( dTimeDiff ) < kdMaxDtPulserPs )
                  {
                     fvhCoincMapAllChanGdpb[ uGdpb ]->Fill( uChanA, uChanB );
                  } // if( TMath::Abs( dTimeDiff ) < kdMaxDtPulserPs )
               } // for( UInt_t uChanA = 0; uChanA < fuNrOfChannelsPerGdpb; ++uChan A )

               /// Done with this channel, we can reset the hit counter
               fvuCoincNbHitsLastMs[ uGdpb ][ uChanA ] = 0;
            } // for( UInt_t uChanA = 0; uChanA < fuNrOfChannelsPerGdpb; ++uChan A )
         } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      } // if( kTRUE == fbCoincMapsEnable )
   } // for( UInt_t uMsIdx = 0; uMsIdx < uNbMsLoop; uMsIdx ++ )

   // Update RMS plots only every 10s in data
   if( kTRUE == fbPulserModeEnable )
   {
      if( 10.0 < dTsStartTime - fdLastRmsUpdateTime )
      {
         // Reset summary histograms for safety
         fhTimeMeanPulser->Reset();
         fhTimeRmsPulser->Reset();

         for( UInt_t uFeeA = 0; uFeeA < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeA++)
            for( UInt_t uFeeB = 0; uFeeB < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeB++)
               if( NULL != fvhTimeDiffPulser[uFeeA][uFeeB] )
               {
                  fhTimeMeanPulser->Fill( uFeeA, uFeeB, fvhTimeDiffPulser[uFeeA][uFeeB]->GetMean() );
                  fhTimeRmsPulser->Fill( uFeeA, uFeeB, fvhTimeDiffPulser[uFeeA][uFeeB]->GetRMS() );
               } // for( UInt_t uChan = 0; uChan < kuNbChanTest - 1; uChan++)
         fdLastRmsUpdateTime = dTsStartTime;
      } // if( 10.0 < dTsStartTime - fdLastRmsUpdateTime )
   } // if( kTRUE == fbPulserModeEnable )

  return kTRUE;
}

void CbmMcbm2018MonitorTof::FillHitInfo(gdpbv100::Message mess)
{
   UInt_t uChannel = mess.getGdpbHitChanId();
   UInt_t uTot     = mess.getGdpbHit32Tot();
   UInt_t uFts     = mess.getGdpbHitFineTs();

   ULong64_t ulCurEpochGdpbGet4 = fvulCurrentEpoch[ fuGet4Nr ];

   // In Ep. Suppr. Mode, receive following epoch instead of previous
   if( 0 < ulCurEpochGdpbGet4 )
      ulCurEpochGdpbGet4 --;
      else ulCurEpochGdpbGet4 = gdpbv100::kuEpochCounterSz; // Catch epoch cycle!

   UInt_t uChannelNr   = fuGet4Id * fuNrOfChannelsPerGet4 + uChannel;
   UInt_t uChannelNrInFee = (fuGet4Id % fuNrOfGet4PerFee) * fuNrOfChannelsPerGet4 + uChannel;
   UInt_t uFeeNr      = (fuGet4Id / fuNrOfGet4PerFee);
   UInt_t uFeeNrInSys = fuGdpbNr * fuNrOfFeePerGdpb + uFeeNr;
   UInt_t uRemappedChannelNr = uFeeNr * fuNrOfChannelsPerFee + fvuGet4ToPadi[ uChannelNrInFee ];
   UInt_t uGbtxNr      = (uFeeNr / kuNbFeePerGbtx);
   UInt_t uFeeInGbtx  = (uFeeNr % kuNbFeePerGbtx);
   UInt_t uGbtxNrInSys = fuGdpbNr * kuNbGbtxPerGdpb + uGbtxNr;

   ULong_t  ulHitTime = mess.getMsgFullTime(ulCurEpochGdpbGet4);
   Double_t dHitTime  = mess.getMsgFullTimeD(ulCurEpochGdpbGet4);

   // In 32b mode the coarse counter is already computed back to 112 FTS bins
   // => need to hide its contribution from the Finetime
   // => FTS = Fullt TS modulo 112
   uFts = mess.getGdpbHitFullTs() % 112;

   fvhChCount_gDPB[fuGdpbNr]->Fill(uChannelNr);
   fvhRawFt_gDPB[fuGdpbNr]->Fill(uChannelNr, uFts);
   fvhRawTot_gDPB[ fuGdpbNr ]->Fill(uChannelNr, uTot);

   /// Remapped for PADI to GET4
   fvhRemapChCount_gDPB[fuGdpbNr]->Fill( uRemappedChannelNr );
   fvhRemapTot_gDPB[ fuGdpbNr ]->Fill(  uRemappedChannelNr , uTot);

   if( uGbtxNrInSys < fuNrOfGbtx )
   {
      UInt_t uOffset = uGbtxNrInSys * kuNbFeeSide * fuNrOfChannelsPerFee;
      if( fviRpcSide[ uGbtxNrInSys ] )
         fvhRemapTotSideB_mod[ fviModuleId[ uGbtxNrInSys ] ]->Fill( uRemappedChannelNr - uOffset, uTot);
         else fvhRemapTotSideA_mod[ fviModuleId[ uGbtxNrInSys ] ]->Fill( uRemappedChannelNr - uOffset, uTot);
   } // if( uGbtxNrInSys < fuNrOfGbtx )
/*
   switch( ( uRemappedChannelNr / fuNrOfChannelsPerFee ) / kuNbFeeSide )
   {
      case 0: // Module 1 Side A
         fvhRemapTotSideA_mod[ fuGdpbNr ]->Fill(  uRemappedChannelNr , uTot);
      case 1: // Module 1 Side B
         fvhRemapTotSideB_mod[ fuGdpbNr ]->Fill(  uRemappedChannelNr - 160 , uTot);
      case 2: // Module 2 Side A
         fvhRemapTotSideA_mod[ fuGdpbNr ]->Fill(  uRemappedChannelNr - 160 , uTot);
      case 3: // Module 2 Side B
         fvhRemapTotSideB_mod[ fuGdpbNr ]->Fill(  uRemappedChannelNr - 320, uTot);
   } // switch( ( uRemappedChannelNr / fuNrOfChannelsPerFee ) % kuNbFeeSide )
*/
   // In Run rate evolution
   if (fdStartTime < 0)
      fdStartTime = dHitTime;

   // Reset the evolution Histogram and the start time when we reach the end of the range
   if( fuHistoryHistoSize < 1e-9 * (dHitTime - fdStartTime) )
   {
      ResetEvolutionHistograms();
      fdStartTime = dHitTime;
   } // if( fuHistoryHistoSize < 1e-9 * (dHitTime - fdStartTime) )

   // In Run rate evolution
   if (fdStartTimeLong < 0)
      fdStartTimeLong = dHitTime;

   // Reset the evolution Histogram and the start time when we reach the end of the range
   if( fuHistoryHistoSizeLong < 1e-9 * (dHitTime - fdStartTimeLong) / 60.0 )
   {
      ResetLongEvolutionHistograms();
      fdStartTimeLong = dHitTime;
   } // if( fuHistoryHistoSize < 1e-9 * (dHitTime - fdStartTime) / 60.0 )

   ///* Pulser monitoring *///
   if( kTRUE == fbPulserModeEnable )
   {
      /// Save last hist time if pulser channel
      /// Fill the corresponding histos if the time difference is reasonnable
      if( gdpbv100::kuFeePulserChannel == uChannelNrInFee )
      {
         fdTsLastPulserHit[ uFeeNrInSys ] = dHitTime;
         fvuFeeNbHitsLastMs[ uFeeNrInSys ]++;
/*
         /// Update the difference to all other FEE with lower indices
         for( UInt_t uFeeB = 0; uFeeB < uFeeNrInSys; uFeeB++)
            if( NULL != fvhTimeDiffPulser[uFeeB][uFeeNrInSys] )
            {
               Double_t dTimeDiff = 1e3 * ( fdTsLastPulserHit[ uFeeNrInSys ] - fdTsLastPulserHit[ uFeeB ] );
               if( TMath::Abs( dTimeDiff ) < kdMaxDtPulserPs )
               {
                  fvhTimeDiffPulser[uFeeB][uFeeNrInSys]->Fill( dTimeDiff );

                  /// Dt Evo plots only for first FEE in each GBTx
                  if( 0 == uFeeInGbtx && 0 == uFeeB % kuNbFeePerGbtx )
                  {
                     /// Evo of GBTx inside same DPB
                     if( fuGdpbNr == uFeeB / fuNrOfFeePerGdpb )
                     {
                        if( 0 == uFeeB / kuNbFeePerGbtx )
                        {
                           UInt_t uPlotIdx = fuGdpbNr * ( kuNbGbtxPerGdpb - 1) + uGbtxNr - 1;
                           fvhPulserTimeDiffEvoGbtxGbtx[ uPlotIdx ]->Fill( 1e-9 / 60.0 * (dHitTime - fdStartTime), dTimeDiff );
                        }
                     } // if( fuGdpbNr == uFeeB / fuNrOfFeePerGdpb )
                        else // if( NULL != fvvhPulserTimeDiffEvoGdpbGdpb[ uFeeB / fuNrOfFeePerGdpb ][ fuGdpbNr ] )
                        {
                           /// Evo of DPBs if both first FEE in First GBTx
                           if( 0 == uGbtxNr && 0 == uFeeB / kuNbFeePerGbtx )
                              fvvhPulserTimeDiffEvoGdpbGdpb[ uFeeB / fuNrOfFeePerGdpb ][ fuGdpbNr ]->Fill(
                                 1e-9 / 60.0 * (dHitTime - fdStartTime), dTimeDiff );
                        } // else of if( fuGdpbNr == uFeeB / fuNrOfFeePerGdpb )
                  } // if( 0 == uFeeInGbtx && 0 == uFeeB % kuNbFeePerGbtx )
               } // if( TMath::Abs( dTimeDiff ) < kdMaxDtPulserPs )
   //               else if( 10 == uFeeB && 20 == uFeeNrInSys )
   //                  LOG( INFO ) << "new in 20 " << dTimeDiff
   //                              << FairLogger::endl;
            } // if( NULL != fvhTimeDiffPulser[uFeeB][uFeeB] )

         /// Update the difference to all other FEE with higher indices
         for( UInt_t uFeeB = uFeeNrInSys + 1; uFeeB < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeB++)
            if( NULL != fvhTimeDiffPulser[uFeeNrInSys][uFeeB] )
            {
               Double_t dTimeDiff = 1e3 * ( fdTsLastPulserHit[ uFeeB ] - fdTsLastPulserHit[ uFeeNrInSys ] );
               if( TMath::Abs( dTimeDiff ) < kdMaxDtPulserPs )
               {
                  fvhTimeDiffPulser[uFeeNrInSys][uFeeB]->Fill( dTimeDiff );

                  /// Dt Evo plots only for first FEE in each GBTx
                  if( 0 == uFeeInGbtx && 0 == uFeeB % kuNbFeePerGbtx )
                  {
                     /// Evo of GBTx inside same DPB
                     if( fuGdpbNr == uFeeB / fuNrOfFeePerGdpb )
                     {
                        if( 0 == uGbtxNr  )
                        {
                           UInt_t uPlotIdx = fuGdpbNr * ( kuNbGbtxPerGdpb - 1)
                                            + ( uFeeB - fuGdpbNr * fuNrOfFeePerGdpb) / kuNbFeePerGbtx - 1;
                           fvhPulserTimeDiffEvoGbtxGbtx[ uPlotIdx ]->Fill( 1e-9 / 60.0 * (dHitTime - fdStartTime), dTimeDiff );
                        } // if( 0 == uGbtxNr  )
                     } // if( fuGdpbNr == uFeeB / fuNrOfFeePerGdpb )
                        else // if( NULL != fvvhPulserTimeDiffEvoGdpbGdpb[ uFeeB / fuNrOfFeePerGdpb ][ fuGdpbNr ] )
                        {
                           /// Evo of DPBs if both first FEE in First GBTx
                           if( 0 == uGbtxNr && 0 == uFeeB / kuNbFeePerGbtx )
                              fvvhPulserTimeDiffEvoGdpbGdpb[ fuGdpbNr ][ uFeeB / fuNrOfFeePerGdpb ]->Fill(
                                 1e-9 / 60.0 * (dHitTime - fdStartTime), dTimeDiff );
                        } // else of if( fuGdpbNr == uFeeB / fuNrOfFeePerGdpb )
                  } // if( 0 == uFeeInGbtx && 0 == uFeeB % kuNbFeePerGbtx )
               } // if( TMath::Abs( dTimeDiff ) < kdMaxDtPulserPs )
   //               else if( 10 == uFeeNrInSys && 20 == uFeeB )
   //                  LOG( INFO ) << "new in 10 " << dTimeDiff
   //                              << FairLogger::endl;
            } // if( NULL != fvhTimeDiffPulser[uFeeNrInSys][uFeeB] )
*/
      } // if( gdpbv100::kuFeePulserChannel == uChannelNrInFee )
   } // if( kTRUE == fbPulserModeEnable )

   /// Coincidence maps
   if( kTRUE == fbCoincMapsEnable )
   {
      fvdCoincTsLastHit[ fuGdpbNr ][ uRemappedChannelNr ] = dHitTime;
      fvuCoincNbHitsLastMs[ fuGdpbNr ][ uRemappedChannelNr ]++;
   } // if( kTRUE == fbCoincMapsEnable )

   if (0 <= fdStartTime)
   {
      fvhChannelRate_gDPB[ fuGdpbNr ]->Fill( 1e-9 * (dHitTime - fdStartTime), uChannelNr );
      fvhRemapChRate_gDPB[ fuGdpbNr ]->Fill( 1e-9 * (dHitTime - fdStartTime), uRemappedChannelNr );
      fvhFeeRate_gDPB[(fuGdpbNr * fuNrOfFeePerGdpb) + uFeeNr]->Fill( 1e-9 * (dHitTime - fdStartTime));
      fvhFeeErrorRatio_gDPB[(fuGdpbNr * fuNrOfFeePerGdpb) + uFeeNr]->Fill( 1e-9 * (dHitTime - fdStartTime), 0, 1);
   } // if (0 <= fdStartTime)

   if (0 <= fdStartTimeLong)
   {
      fvhFeeRateLong_gDPB[(fuGdpbNr * fuNrOfFeePerGdpb) + uFeeNr]->Fill(
            1e-9 / 60.0 * (dHitTime - fdStartTimeLong), 1 / 60.0 );
      fvhFeeErrorRatioLong_gDPB[(fuGdpbNr * fuNrOfFeePerGdpb) + uFeeNr]->Fill(
            1e-9 / 60.0 * (dHitTime - fdStartTimeLong), 0, 1 / 60.0 );
   } // if (0 <= fdStartTimeLong)

   if( kTRUE == fbPrintAllHitsEnable )
   {
      LOG(INFO) << "Hit: " << Form("0x%08x ", fuGdpbId)
                << ", " << std::setw(2) << fuGet4Nr
                << ", " << std::setw(3) << uChannel
                << ", " << std::setw(3) << uTot
                << ", epoch " << std::setw(3) << ulCurEpochGdpbGet4
                << ", FullTime Clk " << Form("%12lu ", ulHitTime )
                << ", FullTime s "  << Form("%12.9f ", dHitTime / 1e9 )
                << ", FineTime " << uFts
                << FairLogger::endl;
   } // if( kTRUE == fbPrintAllHitsEnable )
}

void CbmMcbm2018MonitorTof::FillEpochInfo(gdpbv100::Message mess)
{
   ULong64_t ulEpochNr = mess.getGdpbEpEpochNb();

   fvulCurrentEpoch[fuGet4Nr] = ulEpochNr;

   if (1 == mess.getGdpbEpSync())
      fhGet4EpochFlags->Fill(fuGet4Nr, 0);
   if (1 == mess.getGdpbEpDataLoss())
      fhGet4EpochFlags->Fill(fuGet4Nr, 1);
   if (1 == mess.getGdpbEpEpochLoss())
      fhGet4EpochFlags->Fill(fuGet4Nr, 2);
   if (1 == mess.getGdpbEpMissmatch())
      fhGet4EpochFlags->Fill(fuGet4Nr, 3);

   fulCurrentEpochTime = mess.getMsgFullTime(ulEpochNr);

   /// Re-align the epoch number of the message in case it will be used later:
   /// We received the epoch after the data instead of the one before!
   if( 0 < ulEpochNr )
      mess.setGdpbEpEpochNb( ulEpochNr - 1 );
      else mess.setGdpbEpEpochNb( gdpbv100::kuEpochCounterSz );

   Int_t iBufferSize = fvmEpSupprBuffer[fuGet4Nr].size();
   if( 0 < iBufferSize )
   {
      LOG(DEBUG) << "Now processing stored messages for for get4 " << fuGet4Nr << " with epoch number "
                 << (fvulCurrentEpoch[fuGet4Nr] - 1) << FairLogger::endl;

      /// Data are sorted between epochs, not inside => Epoch level ordering
      /// Sorting at lower bin precision level
      std::stable_sort( fvmEpSupprBuffer[fuGet4Nr].begin(), fvmEpSupprBuffer[fuGet4Nr].begin() );

      for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
      {
         FillHitInfo( fvmEpSupprBuffer[fuGet4Nr][ iMsgIdx ] );
      } // for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )

      fvmEpSupprBuffer[fuGet4Nr].clear();
   } // if( 0 < fvmEpSupprBuffer[fuGet4Nr] )
}

void CbmMcbm2018MonitorTof::PrintSlcInfo(gdpbv100::Message mess)
{
   if (fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find(fuGdpbId))
   {
      UInt_t uChip = mess.getGdpbGenChipId();
      UInt_t uChan = mess.getGdpbSlcChan();
      UInt_t uEdge = mess.getGdpbSlcEdge();
      UInt_t uData = mess.getGdpbSlcData();
      UInt_t uType = mess.getGdpbSlcType();
      Double_t dFullChId =  fuGet4Nr * fuNrOfChannelsPerGet4 + mess.getGdpbSlcChan() + 0.5 * mess.getGdpbSlcEdge();
      Double_t dMessTime = static_cast< Double_t>( fulCurrentEpochTime ) * 1.e-9;

      switch( uType )
      {
         case 0: // Scaler counter
         {
            fhGet4ChanScm->Fill(dFullChId, uType );
            fhScmScalerCounters->Fill( uData, dFullChId);
            break;
         }
         case 1: // Deadtime counter
         {
            fhGet4ChanScm->Fill(dFullChId, uType );
            fhScmDeadtimeCounters->Fill( uData, dFullChId);
            break;
         }
         case 2: // SPI message
         {
/*
            LOG(INFO) << "GET4 Slow Control message, epoch "
                       << static_cast<Int_t>(fvulCurrentEpoch[fuGet4Nr]) << ", time " << std::setprecision(9)
                       << std::fixed << dMessTime << " s "
                       << " for board ID " << std::hex << std::setw(4) << fuGdpbId
                       << std::dec << FairLogger::endl << " +++++++ > Chip = "
                       << std::setw(2) << uChip << ", Chan = "
                       << std::setw(1) << uChan << ", Edge = "
                       << std::setw(1) << uEdge << ", Type = "
                       << std::setw(1) << mess.getGdpbSlcType() << ", Data = 0x"
//                 << std::hex << std::setw(6) << mess.getGdpbSlcData()
                       << Form( "%06x", uData )
                       << std::dec << ", CRC = " << uCRC
//                 << " RAW: " << Form( "%08x", mess.getGdpbSlcMess() )
                       << FairLogger::endl;
*/
            fhGet4ChanScm->Fill(dFullChId, uType );
            break;
         }
         case 3: // Start message or SEU counter
         {
            if( 0 == mess.getGdpbSlcChan() && 0 == mess.getGdpbSlcEdge() ) // START message
            {
/*
               LOG(INFO) << std::setprecision(9)
                             << std::fixed << dMessTime << " s "
                             << FairLogger::endl;
            LOG(INFO) << "GET4 Slow Control message, epoch "
                    << static_cast<Int_t>(fvulCurrentEpoch[fuGet4Nr]) << ", time " << std::setprecision(9)
                    << std::fixed << dMessTime << " s "
                    << " for board ID " << std::hex << std::setw(4) << fuGdpbId
                    << std::dec << FairLogger::endl << " +++++++ > Chip = "
                    << std::setw(2) << mess.getGdpbGenChipId() << ", Chan = "
                    << std::setw(1) << mess.getGdpbSlcChan() << ", Edge = "
                    << std::setw(1) << mess.getGdpbSlcEdge() << ", Type = "
                    << std::setw(1) << mess.getGdpbSlcType() << ", Data = 0x"
   //                 << std::hex << std::setw(6) << mess.getGdpbSlcData()
                    << Form( "%06x", mess.getGdpbSlcData() )
                    << std::dec << ", CRC = " << mess.getGdpbSlcCrc()
                    << FairLogger::endl;
*/
               fhGet4ChanScm->Fill(dFullChId, uType + 1);
            } // if( 0 == mess.getGdpbSlcChan() && 0 == mess.getGdpbSlcEdge() )
            else if( 0 == mess.getGdpbSlcChan() && 1 == mess.getGdpbSlcEdge() ) // SEU counter message
            {
/*
         LOG(INFO) << "GET4 Slow Control message, epoch "
                 << static_cast<Int_t>(fvulCurrentEpoch[fuGet4Nr]) << ", time " << std::setprecision(9)
                 << std::fixed << dMessTime << " s "
                 << " for board ID " << std::hex << std::setw(4) << fuGdpbId
                 << std::dec << FairLogger::endl << " +++++++ > Chip = "
                 << std::setw(2) << mess.getGdpbGenChipId() << ", Chan = "
                 << std::setw(1) << mess.getGdpbSlcChan() << ", Edge = "
                 << std::setw(1) << mess.getGdpbSlcEdge() << ", Type = "
                 << std::setw(1) << mess.getGdpbSlcType() << ", Data = 0x"
//                 << std::hex << std::setw(6) << mess.getGdpbSlcData()
                 << Form( "%06x", mess.getGdpbSlcData() )
                 << std::dec << ", CRC = " << mess.getGdpbSlcCrc()
                 << FairLogger::endl;
*/
               fhGet4ChanScm->Fill(dFullChId, uType );
               fhScmSeuCounters->Fill( uData, dFullChId);
               fhScmSeuCountersEvo->Fill( dMessTime - fdStartTime* 1.e-9, uData, dFullChId);
             } // else if( 0 == mess.getGdpbSlcChan() && 1 == mess.getGdpbSlcEdge() )
            break;
         }
         default: // Should never happen
            break;
      } // switch( mess.getGdpbSlcType() )
   }
}

void CbmMcbm2018MonitorTof::PrintGenInfo(gdpbv100::Message mess)
{
   Int_t mType = mess.getMessageType();
   Int_t channel = mess.getGdpbHitChanId();
   uint64_t uData = mess.getData();

   LOG(DEBUG) << "Get4 MSG type " << mType << " from gdpbId " << fuGdpbId
              << ", getId " << fuGet4Id << ", (hit channel) " << channel
              << " data " << std::hex << uData
              << FairLogger::endl;
}

void CbmMcbm2018MonitorTof::PrintSysInfo(gdpbv100::Message mess)
{
   if (fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find(fuGdpbId))
      LOG(DEBUG) << "GET4 System message,       epoch "
                 << static_cast<Int_t>(fvulCurrentEpoch[fuGet4Nr]) << ", time " << std::setprecision(9)
                 << std::fixed << Double_t(fulCurrentEpochTime) * 1.e-9 << " s "
                 << " for board ID " << std::hex << std::setw(4) << fuGdpbId
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
            LOG(DEBUG) << " +++++++ > gDPB: " << std::hex << std::setw(4) << fuGdpbId
                       << std::dec << ", Chip = " << std::setw(2)
                       << mess.getGdpbGenChipId() << ", Chan = " << std::setw(1)
                       << mess.getGdpbSysErrChanId() << ", Edge = "
                       << std::setw(1) << mess.getGdpbSysErrEdge() << ", Empt = "
                       << std::setw(1) << mess.getGdpbSysErrUnused()
                       << ", Data = " << std::hex << std::setw(2) << uData
                       << std::dec << " -- GET4 V1 Error Event"
                       << FairLogger::endl;
            else LOG(DEBUG) << " +++++++ >gDPB: " << std::hex << std::setw(4) << fuGdpbId
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
         LOG(DEBUG) << "GET$ synchronization pulse missing" << FairLogger::endl;
         break;
      } // case gdpbv100::SYS_GET4_SYNC_MISS:
      case gdpbv100::SYS_SYNC_ERROR:
      {
         LOG(DEBUG) << "gDPB synchronization pulse error" << FairLogger::endl;
         break;
      } // case gdpbv100::SYS_SYNC_ERROR:
      default:
      {
         LOG(DEBUG) << "Crazy system message, subtype " << mess.getGdpbSysSubType() << FairLogger::endl;
         break;
      } // case gdpbv100::SYS_SYNC_ERROR:

   } // switch( getGdpbSysSubType() )
}

void CbmMcbm2018MonitorTof::FillStarTrigInfo(gdpbv100::Message mess)
{
   Int_t iMsgIndex = mess.getStarTrigMsgIndex();

   switch( iMsgIndex )
   {
      case 0:
         fvhTokenMsgType[fuGdpbNr]->Fill(0);
         fvulGdpbTsMsb[fuGdpbNr] = mess.getGdpbTsMsbStarA();
         break;
      case 1:
         fvhTokenMsgType[fuGdpbNr]->Fill(1);
         fvulGdpbTsLsb[fuGdpbNr] = mess.getGdpbTsLsbStarB();
         fvulStarTsMsb[fuGdpbNr] = mess.getStarTsMsbStarB();
         break;
      case 2:
         fvhTokenMsgType[fuGdpbNr]->Fill(2);
         fvulStarTsMid[fuGdpbNr] = mess.getStarTsMidStarC();
         break;
      case 3:
      {
         fvhTokenMsgType[fuGdpbNr]->Fill(3);

         ULong64_t ulNewGdpbTsFull = ( fvulGdpbTsMsb[fuGdpbNr] << 24 )
                                   + ( fvulGdpbTsLsb[fuGdpbNr]       );
         ULong64_t ulNewStarTsFull = ( fvulStarTsMsb[fuGdpbNr] << 48 )
                                   + ( fvulStarTsMid[fuGdpbNr] <<  8 )
                                   + mess.getStarTsLsbStarD();
         UInt_t uNewToken  = mess.getStarTokenStarD();
         UInt_t uNewDaqCmd  = mess.getStarDaqCmdStarD();
         UInt_t uNewTrigCmd = mess.getStarTrigCmdStarD();

         if( ( uNewToken == fvuStarTokenLast[fuGdpbNr] ) && ( ulNewGdpbTsFull == fvulGdpbTsFullLast[fuGdpbNr] ) &&
             ( ulNewStarTsFull == fvulStarTsFullLast[fuGdpbNr] ) && ( uNewDaqCmd == fvuStarDaqCmdLast[fuGdpbNr] ) &&
             ( uNewTrigCmd == fvuStarTrigCmdLast[fuGdpbNr] ) )
         {
            UInt_t uTrigWord =  ( (fvuStarTrigCmdLast[fuGdpbNr] & 0x00F) << 16 )
                     + ( (fvuStarDaqCmdLast[fuGdpbNr]   & 0x00F) << 12 )
                     + ( (fvuStarTokenLast[fuGdpbNr]    & 0xFFF)       );
            LOG(WARNING) << "Possible error: identical STAR tokens found twice in a row => ignore 2nd! "
                         << " TS " << fulCurrentTsIndex
                         << " gDBB #" << fuGdpbNr << " "
                         << Form("token = %5u ", fvuStarTokenLast[fuGdpbNr] )
                         << Form("gDPB ts  = %12llu ", fvulGdpbTsFullLast[fuGdpbNr] )
                         << Form("STAR ts = %12llu ", fvulStarTsFullLast[fuGdpbNr] )
                         << Form("DAQ cmd = %2u ", fvuStarDaqCmdLast[fuGdpbNr] )
                         << Form("TRG cmd = %2u ", fvuStarTrigCmdLast[fuGdpbNr] )
                         << Form("TRG Wrd = %5x ", uTrigWord )
                         << FairLogger::endl;
            return;
         } // if exactly same message repeated
/*
         if( (uNewToken != fuStarTokenLast[fuGdpbNr] + 1) &&
             0 < fvulGdpbTsFullLast[fuGdpbNr] && 0 < fvulStarTsFullLast[fuGdpbNr] &&
             ( 4095 != fvuStarTokenLast[fuGdpbNr] || 1 != uNewToken)  )
            LOG(WARNING) << "Possible error: STAR token did not increase by exactly 1! "
                         << " gDBB #" << fuGdpbNr << " "
                         << Form("old = %5u vs new = %5u ", fvuStarTokenLast[fuGdpbNr],   uNewToken)
                         << Form("old = %12llu vs new = %12llu ", fvulGdpbTsFullLast[fuGdpbNr], ulNewGdpbTsFull)
                         << Form("old = %12llu vs new = %12llu ", fvulStarTsFullLast[fuGdpbNr], ulNewStarTsFull)
                         << Form("old = %2u vs new = %2u ", fvuStarDaqCmdLast[fuGdpbNr],  uNewDaqCmd)
                         << Form("old = %2u vs new = %2u ", fvuStarTrigCmdLast[fuGdpbNr], uNewTrigCmd)
                         << FairLogger::endl;
*/
         // STAR TS counter reset detection
         if( ulNewStarTsFull < fvulStarTsFullLast[fuGdpbNr] )
            LOG(DEBUG) << "Probable reset of the STAR TS: old = " << Form("%16llu", fvulStarTsFullLast[fuGdpbNr])
                       << " new = " << Form("%16llu", ulNewStarTsFull)
                       << " Diff = -" << Form("%8llu", fvulStarTsFullLast[fuGdpbNr] - ulNewStarTsFull)
                       << FairLogger::endl;

/*
         LOG(INFO) << "Updating  trigger token for " << fuGdpbNr
                   << " " << fuStarTokenLast[fuGdpbNr] << " " << uNewToken
                   << FairLogger::endl;
*/
         ULong64_t ulGdpbTsDiff = ulNewGdpbTsFull - fvulGdpbTsFullLast[fuGdpbNr];
         fvulGdpbTsFullLast[fuGdpbNr] = ulNewGdpbTsFull;
         fvulStarTsFullLast[fuGdpbNr] = ulNewStarTsFull;
         fvuStarTokenLast[fuGdpbNr]   = uNewToken;
         fvuStarDaqCmdLast[fuGdpbNr]  = uNewDaqCmd;
         fvuStarTrigCmdLast[fuGdpbNr] = uNewTrigCmd;

         /// Histograms filling only in core MS
         if( fuCurrentMs < fuCoreMs  )
         {
            /// In Run rate evolution
            if( 0 <= fdStartTime )
            {
               /// Reset the evolution Histogram and the start time when we reach the end of the range
               if( fuHistoryHistoSize < 1e-9 * (fvulGdpbTsFullLast[fuGdpbNr] * gdpbv100::kdClockCycleSizeNs - fdStartTime) )
               {
                  ResetEvolutionHistograms();
                  fdStartTime = fvulGdpbTsFullLast[fuGdpbNr] * gdpbv100::kdClockCycleSizeNs;
               } // if( fuHistoryHistoSize < 1e-9 * (fulGdpbTsFullLast * gdpbv100::kdClockCycleSizeNs - fdStartTime) )

               fvhTriggerRate[fuGdpbNr]->Fill( 1e-9 * ( fvulGdpbTsFullLast[fuGdpbNr] * gdpbv100::kdClockCycleSizeNs - fdStartTime ) );
               fvhStarTokenEvo[fuGdpbNr]->Fill( 1e-9 * ( fvulGdpbTsFullLast[fuGdpbNr] * gdpbv100::kdClockCycleSizeNs - fdStartTime ),
                                               fvuStarTokenLast[fuGdpbNr] );
               fvhStarTrigGdpbTsEvo[fuGdpbNr]->Fill( 1e-9 * ( fvulGdpbTsFullLast[fuGdpbNr] * gdpbv100::kdClockCycleSizeNs - fdStartTime ),
                                                     fvulGdpbTsFullLast[fuGdpbNr] );
               fvhStarTrigStarTsEvo[fuGdpbNr]->Fill( 1e-9 * ( fvulGdpbTsFullLast[fuGdpbNr] * gdpbv100::kdClockCycleSizeNs - fdStartTime ),
                                                     fvulStarTsFullLast[fuGdpbNr] );
            } // if( 0 < fdStartTime )
               else fdStartTime = fvulGdpbTsFullLast[fuGdpbNr] * gdpbv100::kdClockCycleSizeNs;
            fvhCmdDaqVsTrig[fuGdpbNr]->Fill( fvuStarDaqCmdLast[fuGdpbNr], fvuStarTrigCmdLast[fuGdpbNr] );
         } // if( fuCurrentMs < fuCoreMs  )

         break;
      } // case 3
      default:
         LOG(ERROR) << "Unknown Star Trigger messageindex: " << iMsgIndex << FairLogger::endl;
   } // switch( iMsgIndex )
}

void CbmMcbm2018MonitorTof::Reset()
{
}

void CbmMcbm2018MonitorTof::Finish()
{
   // Printout some stats on what was unpacked
   TString message_type;
   for( unsigned int i = 0; i < fviMsgCounter.size(); ++i)
   {
      switch (i)
      {
         case 0:
            message_type = "NOP";
            break;
         case 1:
            message_type = "HIT";
            break;
         case 2:
            message_type = "EPOCH";
            break;
         case 3:
            message_type = "SYNC";
            break;
         case 4:
            message_type = "AUX";
            break;
         case 5:
            message_type = "EPOCH2";
            break;
         case 6:
            message_type = "GET4";
            break;
         case 7:
            message_type = "SYS";
            break;
         case 8:
            message_type = "GET4_SLC";
            break;
         case 9:
            message_type = "GET4_32B";
            break;
         case 10:
            message_type = "GET4_SYS";
            break;
         default:
            message_type = "UNKNOWN";
            break;
      } // switch(i)
      LOG(INFO) << message_type << " messages: " << fviMsgCounter[i]
                << FairLogger::endl;
   } // for (unsigned int i=0; i< fviMsgCounter.size(); ++i)

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   for (UInt_t i = 0; i < fuNrOfGdpbs; ++i)
   {
      for (UInt_t j = 0; j < fuNrOfGet4PerGdpb; ++j)
      {
         LOG(INFO) << "Last epoch for gDPB: " << std::hex << std::setw(4) << i
                   << std::dec << " , GET4  " << std::setw(4) << j << " => "
                   << fvulCurrentEpoch[GetArrayIndex(i, j)] << FairLogger::endl;
      } // for (UInt_t j = 0; j < fuNrOfGet4PerGdpb; ++j)
   } // for (UInt_t i = 0; i < fuNrOfGdpbs; ++i)
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;


   /// Update RMS plots
   if( kTRUE == fbPulserModeEnable )
   {
      /// Reset summary histograms for safety
      fhTimeMeanPulser->Reset();
      fhTimeRmsPulser->Reset();

      for( UInt_t uFeeA = 0; uFeeA < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeA++)
         for( UInt_t uFeeB = 0; uFeeB < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeB++)
            if( NULL != fvhTimeDiffPulser[uFeeA][uFeeB] )
            {
               fhTimeMeanPulser->Fill( uFeeA, uFeeB, fvhTimeDiffPulser[uFeeA][uFeeB]->GetMean() );
               fhTimeRmsPulser->Fill( uFeeA, uFeeB, fvhTimeDiffPulser[uFeeA][uFeeB]->GetRMS() );
            } // for( UInt_t uChan = 0; uChan < kuNbChanTest - 1; uChan++)

      /// Update zoomed RMS and pulser fit plots
      UpdateZoomedFit();
   } // if( kTRUE == fbPulserModeEnable )

   SaveAllHistos();
}

void CbmMcbm2018MonitorTof::FillOutput(CbmDigi* /*digi*/)
{
}

void CbmMcbm2018MonitorTof::SaveAllHistos( TString sFileName )
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

   gDirectory->mkdir("Tof_Raw_gDPB");
   gDirectory->cd("Tof_Raw_gDPB");

   fhMessType->Write();
   fhSysMessType->Write();
   fhGdpbMessType->Write();
   fhGdpbSysMessType->Write();
   fhGet4MessType->Write();
   fhGet4ChanScm->Write();
   fhGet4ChanErrors->Write();
   fhGet4EpochFlags->Write();
   fhScmScalerCounters->Write();
   fhScmDeadtimeCounters->Write();
   fhScmSeuCounters->Write();
   fhScmSeuCountersEvo->Write();

   for( UInt_t uTotPlot = 0; uTotPlot < fvhRawTot_gDPB.size(); ++uTotPlot )
      fvhRawTot_gDPB[ uTotPlot ]->Write();

   for( UInt_t uTotPlot = 0; uTotPlot < fvhRemapTot_gDPB.size(); ++uTotPlot )
      fvhRemapTot_gDPB[ uTotPlot ]->Write();

   for( UInt_t uMod = 0; uMod < fuNrOfModules; uMod ++ )
   {
      fvhRemapTotSideA_mod[ uMod ]->Write();
      fvhRemapTotSideB_mod[ uMod ]->Write();
   } // for( UInt_t uMod = 0; uMod < fuNrOfModules; uMod ++ )

   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      fvhRawFt_gDPB[ uGdpb ]->Write();
      fvhChCount_gDPB[ uGdpb ]->Write();
      fvhChannelRate_gDPB[ uGdpb ]->Write();
      fvhRemapChCount_gDPB[ uGdpb ]->Write();
      fvhRemapChRate_gDPB[ uGdpb ]->Write();

      for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++ uFee)
      {
         fvhFeeRate_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ]->Write();
         fvhFeeErrorRate_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ]->Write();
         fvhFeeErrorRatio_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ]->Write();
         fvhFeeRateLong_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ]->Write();
         fvhFeeErrorRateLong_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ]->Write();
         fvhFeeErrorRatioLong_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ]->Write();
      } // for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++ uFee)

   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   if( kTRUE == fbPulserModeEnable )
   {
      fhTimeMeanPulser->Write();
      fhTimeRmsPulser->Write();
      fhTimeRmsZoomFitPuls->Write();
      fhTimeResFitPuls->Write();
   } // if( kTRUE == fbPulserModeEnable )
   gDirectory->cd("..");

   ///* STAR event building/cutting *///
   gDirectory->mkdir("Star_Raw");
   gDirectory->cd("Star_Raw");
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      fvhTokenMsgType[ uGdpb ]->Write();
      fvhTriggerRate[ uGdpb ]->Write();
      fvhCmdDaqVsTrig[ uGdpb ]->Write();
      fvhStarTokenEvo[ uGdpb ]->Write();
      fvhStarTrigGdpbTsEvo[ uGdpb ]->Write();
      fvhStarTrigStarTsEvo[ uGdpb ]->Write();
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   gDirectory->cd("..");

   ///* Pulser monitoring *///
   if( kTRUE == fbPulserModeEnable )
   {
      gDirectory->mkdir("TofDt");
      gDirectory->cd("TofDt");
      for( UInt_t uFeeA = 0; uFeeA < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeA++)
         for( UInt_t uFeeB = 0; uFeeB < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeB++)
            if( NULL != fvhTimeDiffPulser[uFeeA][uFeeB] )
               fvhTimeDiffPulser[uFeeA][uFeeB]->Write();
      gDirectory->cd("..");

      ///* Pulser evolution monitoring *///
      gDirectory->mkdir("TofDtEvo");
      gDirectory->cd("TofDtEvo");
      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      {
         for( UInt_t uGbtx = 0; uGbtx < kuNbGbtxPerGdpb - 1; ++uGbtx )
            fvhPulserTimeDiffEvoGbtxGbtx[ uGdpb * (kuNbGbtxPerGdpb - 1)  + uGbtx ]->Write();

         for( UInt_t uGdpbB = uGdpb + 1; uGdpbB < fuNrOfGdpbs; ++uGdpbB )
            fvvhPulserTimeDiffEvoGdpbGdpb[ uGdpb ][ uGdpbB ]->Write();
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      gDirectory->cd("..");
   } // if( kTRUE == fbPulserModeEnable )

   ///* Coincidence maps *///
   if( kTRUE == fbCoincMapsEnable )
   {
      gDirectory->mkdir("TofCoinc");
      gDirectory->cd("TofCoinc");
      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      {
         fvhCoincMapAllChanGdpb[ uGdpb ]->Write();
         fvhCoincMeanAllChanGdpb[ uGdpb ]->Write();
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      gDirectory->cd("..");
   } // if( kTRUE == fbCoincMapsEnable )

   gDirectory->mkdir("Flib_Raw");
   gDirectory->cd("Flib_Raw");
   for( UInt_t uLinks = 0; uLinks < fvhMsSzPerLink.size(); uLinks++ )
   {
      if( NULL == fvhMsSzPerLink[ uLinks ] )
         continue;

      fvhMsSzPerLink[ uLinks ]->Write();
      fvhMsSzTimePerLink[ uLinks ]->Write();
   } // for( UInt_t uLinks = 0; uLinks < fvhMsSzPerLink.size(); uLinks++ )

   TH1 * pMissedTsH1    = dynamic_cast< TH1 * >( gROOT->FindObjectAny( "Missed_TS" ) );
   if( NULL != pMissedTsH1 )
      pMissedTsH1->Write();

   TProfile * pMissedTsEvoP = dynamic_cast< TProfile * >( gROOT->FindObjectAny( "Missed_TS_Evo" ) );
   if( NULL != pMissedTsEvoP )
      pMissedTsEvoP->Write();

   gDirectory->cd("..");


  if( "" != sFileName )
  {
     // Restore original directory position
     histoFile->Close();
     oldDir->cd();
  } // if( "" != sFileName )
  if( "" != sFileName )
  {
     // Restore original directory position
     histoFile->Close();
     oldDir->cd();
  } // if( "" != sFileName )

}

void CbmMcbm2018MonitorTof::ResetAllHistos()
{
   LOG(INFO) << "Reseting all TOF histograms." << FairLogger::endl;

   fhMessType->Reset();
   fhSysMessType->Reset();
   fhGdpbMessType->Reset();
   fhGdpbSysMessType->Reset();
   fhGet4MessType->Reset();
   fhGet4ChanScm->Reset();
   fhGet4ChanErrors->Reset();
   fhGet4EpochFlags->Reset();
   fhScmScalerCounters->Reset();
   fhScmDeadtimeCounters->Reset();
   fhScmSeuCounters->Reset();
   fhScmSeuCountersEvo->Reset();

   for( UInt_t uTotPlot = 0; uTotPlot < fvhRawTot_gDPB.size(); ++uTotPlot )
      fvhRawTot_gDPB[ uTotPlot ]->Reset();

   for( UInt_t uTotPlot = 0; uTotPlot < fvhRemapTot_gDPB.size(); ++uTotPlot )
      fvhRemapTot_gDPB[ uTotPlot ]->Reset();

   for( UInt_t uMod = 0; uMod < fuNrOfModules; uMod ++ )
   {
      fvhRemapTotSideA_mod[ uMod ]->Reset();
      fvhRemapTotSideB_mod[ uMod ]->Reset();
   } // for( UInt_t uMod = 0; uMod < fuNrOfModules; uMod ++ )

   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      fvhRawFt_gDPB[ uGdpb ]->Reset();
      fvhChCount_gDPB[ uGdpb ]->Reset();
      fvhChannelRate_gDPB[ uGdpb ]->Reset();
      fvhRemapChCount_gDPB[ uGdpb ]->Reset();
      fvhRemapChRate_gDPB[ uGdpb ]->Reset();

      for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++ uFee)
      {
         fvhFeeRate_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ]->Reset();
         fvhFeeErrorRate_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ]->Reset();
         fvhFeeErrorRatio_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ]->Reset();
         fvhFeeRateLong_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ]->Reset();
         fvhFeeErrorRateLong_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ]->Reset();
         fvhFeeErrorRatioLong_gDPB[ uGdpb * fuNrOfFeePerGdpb + uFee ]->Reset();
      } // for (UInt_t uFee = 0; uFee < fuNrOfFeePerGdpb; ++ uFee)

   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

   ///* STAR event building/cutting *///
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   {
      fvhTokenMsgType[ uGdpb ]->Reset();
      fvhTriggerRate[ uGdpb ]->Reset();
      fvhCmdDaqVsTrig[ uGdpb ]->Reset();
      fvhStarTokenEvo[ uGdpb ]->Reset();
      fvhStarTrigGdpbTsEvo[ uGdpb ]->Reset();
      fvhStarTrigStarTsEvo[ uGdpb ]->Reset();
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )

   ///* Pulser monitoring *///
   if( kTRUE == fbPulserModeEnable )
   {
      for( UInt_t uFeeA = 0; uFeeA < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeA++)
         for( UInt_t uFeeB = 0; uFeeB < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeB++)
            if( NULL != fvhTimeDiffPulser[uFeeA][uFeeB] )
               fvhTimeDiffPulser[uFeeA][uFeeB]->Reset();

      fhTimeMeanPulser->Reset();
      fhTimeRmsPulser->Reset();
      fhTimeRmsZoomFitPuls->Reset();
      fhTimeResFitPuls->Reset();

      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      {
         for( UInt_t uGbtx = 0; uGbtx < kuNbGbtxPerGdpb - 1; ++uGbtx )
            fvhPulserTimeDiffEvoGbtxGbtx[ uGdpb * (kuNbGbtxPerGdpb - 1)  + uGbtx ]->Reset();

         for( UInt_t uGdpbB = uGdpb + 1; uGdpbB < fuNrOfGdpbs; ++uGdpbB )
            fvvhPulserTimeDiffEvoGdpbGdpb[ uGdpb ][ uGdpbB ]->Reset();
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   } // if( kTRUE == fbPulserModeEnable )

   ///* Coincidence maps *///
   if( kTRUE == fbCoincMapsEnable )
   {
      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      {
         fvhCoincMapAllChanGdpb[ uGdpb ]->Reset();
         fvhCoincMeanAllChanGdpb[ uGdpb ]->Reset();
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
   } // if( kTRUE == fbCoincMapsEnable )

   for( UInt_t uLinks = 0; uLinks < fvhMsSzPerLink.size(); uLinks++ )
   {
      if( NULL == fvhMsSzPerLink[ uLinks ] )
         continue;

      fvhMsSzPerLink[ uLinks ]->Reset();
      fvhMsSzTimePerLink[ uLinks ]->Reset();
   } // for( UInt_t uLinks = 0; uLinks < fvhMsSzPerLink.size(); uLinks++ )

   fdStartTime = -1;
   fdStartTimeLong = -1;
   fdStartTimeMsSz = -1;
}
void CbmMcbm2018MonitorTof::ResetEvolutionHistograms()
{
   for( UInt_t uGdpbLoop = 0; uGdpbLoop < fuNrOfGdpbs; ++uGdpbLoop )
   {
      fvhChannelRate_gDPB[ uGdpbLoop ]->Reset();
      fvhRemapChRate_gDPB[ uGdpbLoop ]->Reset();
      for( UInt_t uFeeLoop = 0; uFeeLoop < fuNrOfFeePerGdpb; ++uFeeLoop )
      {
         fvhFeeRate_gDPB[(uGdpbLoop * fuNrOfFeePerGdpb) + uFeeLoop]->Reset();
         fvhFeeErrorRate_gDPB[(uGdpbLoop * fuNrOfFeePerGdpb) + uFeeLoop]->Reset();
         fvhFeeErrorRatio_gDPB[(uGdpbLoop * fuNrOfFeePerGdpb) + uFeeLoop]->Reset();
      } // for( UInt_t uFeeLoop = 0; uFeeLoop < fuNrOfFeePerGdpb; ++uFeeLoop )
      fvhTriggerRate[ uGdpbLoop ]->Reset();
      fvhStarTokenEvo[ uGdpbLoop ]->Reset();
      fvhStarTrigGdpbTsEvo[ uGdpbLoop ]->Reset();
      fvhStarTrigStarTsEvo[ uGdpbLoop ]->Reset();
   } // for( UInt_t uGdpbLoop = 0; uGdpbLoop < fuNrOfGdpbs; ++uGdpbLoop )

   fdStartTime = -1;
}
void CbmMcbm2018MonitorTof::ResetLongEvolutionHistograms()
{
   for (UInt_t uGdpbLoop = 0; uGdpbLoop < fuNrOfGdpbs; uGdpbLoop++)
   {
      for (UInt_t uFeeLoop = 0; uFeeLoop < fuNrOfFeePerGdpb; uFeeLoop++)
      {
         fvhFeeRateLong_gDPB[(uGdpbLoop * fuNrOfFeePerGdpb) + uFeeLoop]->Reset();
         fvhFeeErrorRateLong_gDPB[(uGdpbLoop * fuNrOfFeePerGdpb) + uFeeLoop]->Reset();
         fvhFeeErrorRatioLong_gDPB[(uGdpbLoop * fuNrOfFeePerGdpb) + uFeeLoop]->Reset();
      } // for (UInt_t uFeeLoop = 0; uFeeLoop < fuNrOfFeePerGdpb; uFeeLoop++)
   } // for (UInt_t uFeeLoop = 0; uFeeLoop < fuNrOfFeePerGdpb; uFeeLoop++)

   fdStartTimeLong = -1;
}

void CbmMcbm2018MonitorTof::UpdateZoomedFit()
{
   if( kFALSE == fbPulserModeEnable )
   {
      LOG(ERROR) << "CbmMcbm2018MonitorTof::UpdateZoomedFit => "
                 << "Pulser mode not enabled in root macro, doinb nothing !!! "
                 << FairLogger::endl;
      return;
   } // if( kFALSE == fbPulserModeEnable )

   // Only do something is the user defined the width he want for the zoom
   if( 0.0 < fdFitZoomWidthPs )
   {
      // Reset summary histograms for safety
      fhTimeRmsZoomFitPuls->Reset();
      fhTimeResFitPuls->Reset();

      Double_t dRes = 0;
      TF1 *fitFuncPairs[ fuNrOfFeePerGdpb * fuNrOfGdpbs ][ fuNrOfFeePerGdpb * fuNrOfGdpbs ];

      for( UInt_t uFeeA = 0; uFeeA < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeA++)
         for( UInt_t uFeeB = 0; uFeeB < fuNrOfFeePerGdpb * fuNrOfGdpbs; uFeeB++)
            if( NULL != fvhTimeDiffPulser[uFeeA][uFeeB] )
      {
         // Check that we have at least 1 entry
         if( 0 == fvhTimeDiffPulser[uFeeA][uFeeB]->GetEntries() )
         {
            fhTimeRmsZoomFitPuls->Fill( uFeeA, uFeeB, 0.0 );
            fhTimeResFitPuls->Fill( uFeeA, uFeeB, 0.0 );
            LOG(INFO) << "CbmMcbm2018MonitorTof::UpdateZoomedFit => Empty input "
                         << "for FEE pair " << uFeeA << " and " << uFeeB << " !!! "
                         << FairLogger::endl;
            continue;
         } // if( 0 == fvhTimeDiffPulser[uFeeA][uFeeB]->GetEntries() )

         // Read the peak position (bin with max counts) + total nb of entries
         Int_t    iBinWithMax = fvhTimeDiffPulser[uFeeA][uFeeB]->GetMaximumBin();
         Double_t dNbCounts   = fvhTimeDiffPulser[uFeeA][uFeeB]->Integral();

         // Zoom the X axis to +/- ZoomWidth around the peak position
         Double_t dPeakPos = fvhTimeDiffPulser[uFeeA][uFeeB]->GetXaxis()->GetBinCenter( iBinWithMax );
         fvhTimeDiffPulser[uFeeA][uFeeB]->GetXaxis()->SetRangeUser( dPeakPos - fdFitZoomWidthPs,
                                                                    dPeakPos + fdFitZoomWidthPs );

         // Read integral and check how much we lost due to the zoom (% loss allowed)
         Double_t dZoomCounts = fvhTimeDiffPulser[uFeeA][uFeeB]->Integral();
         if( ( dZoomCounts / dNbCounts ) < 0.99 )
         {
            fhTimeRmsZoomFitPuls->Fill( uFeeA, uFeeB, 0.0 );
            fhTimeResFitPuls->Fill( uFeeA, uFeeB, 0.0 );
            LOG(WARNING) << "CbmMcbm2018MonitorTof::UpdateZoomedFit => Zoom too strong, "
                         << "more than 1% loss for FEE pair " << uFeeA << " and " << uFeeB << " !!! "
                         << FairLogger::endl;
            continue;
         } // if( ( dZoomCounts / dNbCounts ) < 0.99 )

         // Fill new RMS after zoom into summary histo
         fhTimeRmsZoomFitPuls->Fill( uFeeA, uFeeB, fvhTimeDiffPulser[uFeeA][uFeeB]->GetRMS() );


         // Fit using zoomed boundaries + starting gaussian width, store into summary histo
         dRes = 0;
         fitFuncPairs[uFeeA][uFeeB] = new TF1( Form("fPair_%02d_%02d", uFeeA, uFeeB ), "gaus",
                                        dPeakPos - fdFitZoomWidthPs ,
                                        dPeakPos + fdFitZoomWidthPs);
         // Fix the Mean fit value around the Histogram Mean
         fitFuncPairs[uFeeA][uFeeB]->SetParameter( 0, dZoomCounts );
         fitFuncPairs[uFeeA][uFeeB]->SetParameter( 1, dPeakPos );
         fitFuncPairs[uFeeA][uFeeB]->SetParameter( 2, 200.0 ); // Hardcode start with ~4*BinWidth, do better later
         // Using integral instead of bin center seems to lead to unrealistic values => no "I"
         fvhTimeDiffPulser[uFeeA][uFeeB]->Fit( Form("fPair_%02d_%02d", uFeeA, uFeeB ), "QRM0");
         // Get Sigma
         dRes = fitFuncPairs[uFeeA][uFeeB]->GetParameter(2);
         // Cleanup memory
         delete fitFuncPairs[uFeeA][uFeeB];
         // Fill summary
         fhTimeResFitPuls->Fill( uFeeA, uFeeB,  dRes / TMath::Sqrt2() );


         LOG(INFO) << "CbmMcbm2018MonitorTof::UpdateZoomedFit => "
                      << "For FEE pair " << uFeeA << " and " << uFeeB
                      << " we have zoomed RMS = " << fvhTimeDiffPulser[uFeeA][uFeeB]->GetRMS()
                      << " and a resolution of " << dRes / TMath::Sqrt2()
                      << FairLogger::endl;

         // Restore original axis state?
         fvhTimeDiffPulser[uFeeA][uFeeB]->GetXaxis()->UnZoom();
      } // loop on uFeeA and uFeeB + check if corresponding fvhTimeDiffPulser exists
   } // if( 0.0 < fdFitZoomWidthPs )
      else
      {
         LOG(ERROR) << "CbmMcbm2018MonitorTof::UpdateZoomedFit => Zoom width not defined, "
                    << "please use SetFitZoomWidthPs, e.g. in macro, before trying this update !!!"
                    << FairLogger::endl;
      } // else of if( 0.0 < fdFitZoomWidthPs )
}

ClassImp(CbmMcbm2018MonitorTof)
