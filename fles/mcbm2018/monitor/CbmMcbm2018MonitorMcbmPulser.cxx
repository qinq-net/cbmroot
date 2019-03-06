// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                   CbmMcbm2018MonitorMcbmPulser                    -----
// -----                Created 20/02/19  by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018MonitorMcbmPulser.h"

// Data

// CbmRoot
#include "CbmMcbm2018StsPar.h"
#include "CbmMcbm2018TofPar.h"

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

Bool_t bMcbm2018ResetPulser = kFALSE;
Bool_t bMcbm2018WritePulser = kFALSE;
/*
Bool_t bMcbm2018ResetPulser = kFALSE;
Bool_t bMcbm2018WritePulser = kFALSE;
*/
/*
Bool_t bMcbm2018SyncResetHistosTof = kFALSE;
Bool_t bMcbm2018SyncSaveHistosTof  = kFALSE;
Bool_t bMcbm2018SyncUpdateZoomedFit = kFALSE;
*/

CbmMcbm2018MonitorMcbmPulser::CbmMcbm2018MonitorMcbmPulser() :
   CbmMcbmUnpack(),
   fvMsComponentsListSts(),
   fvMsComponentsListTof(),
   fuNbCoreMsPerTs(0),
   fuNbOverMsPerTs(0),
   fbIgnoreOverlapMs(kFALSE),
   fUnpackParSts(nullptr),
   fuStsNrOfDpbs(0),
   fmStsDpbIdIndexMap(),
   fuMuchDpbIdx(1),
   fUnpackParTof(nullptr),
   fuTofNrOfDpbs(0),
   fmTofDpbIdIndexMap(),
   fuDiamondDpbIdx(2),
   fuTotalNrOfDpb(0),
   fdStsTofOffsetNs(0.0),
   fdMuchTofOffsetNs(0.0),
   fbUseBestPair( kFALSE ),
   fbTsLevelAna( kFALSE ),
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
   fvmStsSdpbHitsInMs(),
   fvmStsSdpbHitsInTs(),
   fulTofCurrentTsIndex( 0 ),
   fuTofCurrentMs( 0 ),
   fdTofMsIndex( 0 ),
   fuTofGdpbId( 0 ),
   fuTofGdpbNr( 0 ),
   fiTofEquipmentId( 0 ),
   fviTofMsgCounter( 1 + gdpbv100::MSG_STAR_TRI_D, 0),
   fvulTofCurrentEpoch(),
   fvulTofCurrentEpochCycle(),
   fvulTofCurrentEpochFull(),
   fulTofCurrentEpochTime( 0 ),
   fvmTofEpSupprBuffer(),
   fvmTofGdpbHitsInMs(),
   fvmTofGdpbHitsInTs(),
   fhMcbmHitsNbPerMs(),
	fvhMcbmTimeDiffToSelDpb(),
	fvhMcbmTimeDiffToSelDpbWide(),
	fvhMcbmTimeDiffToSelDpbTs(),
   fvhMcbmTimeDiffToDiamondEvoDpb(),
   fvhMcbmTimeDiffToDiamondWideEvoDpb(),
   fvhMcbmTimeDiffToDiamondTsEvoDpb(),
	fvhMcbmTimeDiffToTofEvoDpb(),
	fvhMcbmTimeDiffToTofWideEvoDpb(),
	fvhMcbmTimeDiffToTofTsEvoDpb()
{
}

CbmMcbm2018MonitorMcbmPulser::~CbmMcbm2018MonitorMcbmPulser()
{
}

Bool_t CbmMcbm2018MonitorMcbmPulser::Init()
{
  LOG(INFO) << "Initializing flib StsXyter unpacker for STS" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == nullptr) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  return kTRUE;
}

void CbmMcbm2018MonitorMcbmPulser::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
         << FairLogger::endl;
   fUnpackParSts = (CbmMcbm2018StsPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmMcbm2018StsPar"));
   fUnpackParTof = (CbmMcbm2018TofPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmMcbm2018TofPar") );
}


Bool_t CbmMcbm2018MonitorMcbmPulser::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
         << FairLogger::endl;

   Bool_t bReInit = ReInitContainers();
   CreateStsHistograms();
   CreateTofHistograms();
   CreateMcbmHistograms();

   return bReInit;
}

Bool_t CbmMcbm2018MonitorMcbmPulser::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
         << FairLogger::endl;

/***************** STS parameters *************************************/
   fuStsNrOfDpbs = fUnpackParSts->GetNrOfDpbs();
   LOG(INFO) << "Nr. of STS DPBs:       " << fuStsNrOfDpbs
             << FairLogger::endl;

   fmStsDpbIdIndexMap.clear();
   for( UInt_t uDpb = 0; uDpb < fuStsNrOfDpbs; ++uDpb )
   {
      fmStsDpbIdIndexMap[ fUnpackParSts->GetDpbId( uDpb )  ] = uDpb;
      LOG(INFO) << "Eq. ID for DPB #" << std::setw(2) << uDpb << " = 0x"
                << std::setw(4) << std::hex << fUnpackParSts->GetDpbId( uDpb )
                << std::dec
                << " => " << fmStsDpbIdIndexMap[ fUnpackParSts->GetDpbId( uDpb )  ]
                << FairLogger::endl;
   } // for( UInt_t uDpb = 0; uDpb < fuStsNrOfDpbs; ++uDpb )

   fvulStsCurrentTsMsb.resize( fuStsNrOfDpbs, 0 );
   fvuStsCurrentTsMsbCycle.resize( fuStsNrOfDpbs, 0 );
   fvmStsSdpbHitsInMs.resize( fuStsNrOfDpbs );
   fvmStsSdpbHitsInTs.resize( fuStsNrOfDpbs );
/***************** STS parameters *************************************/

/***************** TOF parameters *************************************/
   fuTofNrOfDpbs = fUnpackParTof->GetNrOfGdpbs();
   LOG(INFO) << "Nr. of Tof GDPBs: " << fuTofNrOfDpbs << FairLogger::endl;

   fmTofDpbIdIndexMap.clear();
   for( UInt_t i = 0; i < fuTofNrOfDpbs; ++i )
   {
      fmTofDpbIdIndexMap[fUnpackParTof->GetGdpbId(i)] = i;
      LOG(INFO) << "GDPB Id of TOF  " << i << " : " << std::hex << fUnpackParTof->GetGdpbId(i)
                 << std::dec << FairLogger::endl;
   } // for( UInt_t i = 0; i < fuTofNrOfDpbs; ++i )

   /// System sync monitoring
   fvulTofCurrentEpoch.resize( fuTofNrOfDpbs, 0 );
   fvulTofCurrentEpochCycle.resize( fuTofNrOfDpbs, 0 );
   fvulTofCurrentEpochFull.resize( fuTofNrOfDpbs, 0 );
   fvmTofEpSupprBuffer.resize( fuTofNrOfDpbs );
   fvmTofGdpbHitsInMs.resize( fuTofNrOfDpbs );
   fvmTofGdpbHitsInTs.resize( fuTofNrOfDpbs );
/***************** TOF parameters *************************************/

   return kTRUE;
}

void CbmMcbm2018MonitorMcbmPulser::AddMsComponentToList( size_t component, UShort_t usDetectorId )
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
void CbmMcbm2018MonitorMcbmPulser::AddMsComponentToListSts( size_t component )
{
   /// Check for duplicates and ignore if it is the case
   for( UInt_t uCompIdx = 0; uCompIdx < fvMsComponentsListSts.size(); ++uCompIdx )
      if( component == fvMsComponentsListSts[ uCompIdx ] )
         return;

   /// Add to list
   fvMsComponentsListSts.push_back( component );
}
void CbmMcbm2018MonitorMcbmPulser::AddMsComponentToListTof( size_t component )
{
   /// Check for duplicates and ignore if it is the case
   for( UInt_t uCompIdx = 0; uCompIdx < fvMsComponentsListTof.size(); ++uCompIdx )
      if( component == fvMsComponentsListTof[ uCompIdx ] )
         return;

   /// Add to list
   fvMsComponentsListTof.push_back( component );
}
void CbmMcbm2018MonitorMcbmPulser::SetNbMsInTs( size_t uCoreMsNb, size_t uOverlapMsNb )
{
   fuNbCoreMsPerTs = uCoreMsNb;
   fuNbOverMsPerTs = uOverlapMsNb;

   UInt_t uNbMsTotal = fuNbCoreMsPerTs + fuNbOverMsPerTs;
}

/***************** STS Histograms *************************************/
void CbmMcbm2018MonitorMcbmPulser::CreateStsHistograms()
{
   TString sHistName{""};
   TString title{""};
/*
   sHistName = "hPulserMessageType";
   title = "Nb of message for each type; Type";
   fhStsMessType = new TH1I(sHistName, title, 5, 0., 5.);
   fhStsMessType->GetXaxis()->SetBinLabel( 1, "Dummy");
   fhStsMessType->GetXaxis()->SetBinLabel( 2, "Hit");
   fhStsMessType->GetXaxis()->SetBinLabel( 3, "TsMsb");
   fhStsMessType->GetXaxis()->SetBinLabel( 4, "Epoch");
   fhStsMessType->GetXaxis()->SetBinLabel( 5, "Empty");

   // Online histo browser commands
#ifdef USE_HTTP_SERVER
   THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
   if( server )
   {
      server->Register("/StsRaw", fhStsMessType );

      server->RegisterCommand("/Reset_All_Pulser", "bMcbm2018ResetPulser=kTRUE");
      server->RegisterCommand("/Write_All_Pulser", "bMcbm2018WritePulser=kTRUE");

      server->Restrict("/Reset_All_Pulser", "allow=admin");
      server->Restrict("/Write_All_Pulser", "allow=admin");
   } // if( server )
#endif
*/
   /** Create summary Canvases for CERN 2017 **/
/*
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
*/
  /*****************************/
  LOG(INFO) << "Done Creating STS Histograms" << FairLogger::endl;
}
/***************** STS Histograms *************************************/

/***************** TOF Histograms *************************************/
void CbmMcbm2018MonitorMcbmPulser::CreateTofHistograms()
{
   TString sHistName{""};
   TString title{""};

   /*******************************************************************/
/*

   sHistName = "hMessageType";
   title = "Nb of message for each type; Type";
   // Test Big Data readout with plotting
   fhTofMessType = new TH1I(sHistName, title, 1 + gdpbv100::MSG_STAR_TRI_A, 0., 1 + gdpbv100::MSG_STAR_TRI_A);
   fhTofMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_HIT,        "HIT");
   fhTofMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_EPOCH,      "EPOCH");
   fhTofMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_SLOWC,      "SLOWC");
   fhTofMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_SYST,       "SYST");
   fhTofMessType->GetXaxis()->SetBinLabel(1 + gdpbv100::MSG_STAR_TRI_A, "MSG_STAR_TRI");
*/
   /*******************************************************************/
/*
#ifdef USE_HTTP_SERVER
   THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
   if( server )
   {
      server->Register("/TofRaw", fhTofMessType );

      server->RegisterCommand("/Reset_All_TOF",  "bMcbm2018SyncResetHistosTof=kTRUE");
      server->RegisterCommand("/Save_All_Tof",   "bMcbm2018SyncSaveHistosTof=kTRUE");
      server->RegisterCommand("/Update_PulsFit", "bMcbm2018SyncUpdateZoomedFit=kTRUE");

      server->Restrict("/Reset_All_Tof", "allow=admin");
      server->Restrict("/Save_All_Tof",  "allow=admin");
      server->Restrict("/Update_PulsFit", "allow=admin");
   } // if( server )
#endif
*/
   /** Create summary Canvases for STAR 2018 **/
/*
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
*/
   /*****************************/

  LOG(INFO) << "Done Creating TOF Histograms" << FairLogger::endl;
}
/***************** TOF Histograms *************************************/

/***************** mCBM Histograms ************************************/
void CbmMcbm2018MonitorMcbmPulser::CreateMcbmHistograms()
{
   TString sHistName{""};
   TString sHistTitle{""};

   fuTotalNrOfDpb = fuStsNrOfDpbs + fuTofNrOfDpbs;

   sHistName  = "hMcbmHitsNbPerMs";
   sHistTitle = "Nb of hits per DPB; Nb of hits []; DPB []";
   fhMcbmHitsNbPerMs = new TH2D( sHistName, sHistTitle,
                                  1000.0, 0., 1000.,
                                  fuTotalNrOfDpb, 0., fuTotalNrOfDpb);

   for( UInt_t uDpb = 0; uDpb < fuTotalNrOfDpb; ++uDpb )
   {
		sHistName  = Form( "hMcbmTimeDiffToDpb%02u", uDpb);
		sHistTitle = Form( "Time difference for STS and TOF hits, per DPB, against any hits in DPB %02u; <tn - tRef> [ns]; DPB []", uDpb);
		fvhMcbmTimeDiffToSelDpb.push_back( new TH2D( sHistName, sHistTitle,
														 1001, -500.5 * stsxyter::kdClockCycleNs, 500.5 * stsxyter::kdClockCycleNs,
														 fuTotalNrOfDpb, 0., fuTotalNrOfDpb) );
		sHistName  = Form( "hMcbmTimeDiffToDpb%02uWide", uDpb);
		sHistTitle = Form( "Time difference for STS and TOF hits, per DPB, against any hits in DPB %02u, wide range; <tn - tRef> [us]; DPB []", uDpb);
		fvhMcbmTimeDiffToSelDpbWide.push_back( new TH2D( sHistName, sHistTitle,
														 6000.0, -300., 300.,
														 fuTotalNrOfDpb, 0., fuTotalNrOfDpb) );
		sHistName  = Form( "hMcbmTimeDiffToDpb%02uTs", uDpb);
		sHistTitle = Form( "Time difference for STS and TOF hits, per DPB, against any hits in DPB %02u, TS range; <tn - tRef> [ms]; DPB []", uDpb);
		fvhMcbmTimeDiffToSelDpbTs.push_back( new TH2D( sHistName, sHistTitle,
														 2000.0, -10., 10.,
														 fuTotalNrOfDpb, 0., fuTotalNrOfDpb) );

      sHistName  = Form( "hMcbmTimeDiffToDiamondEvoDpb%02u", uDpb );
      sHistTitle = Form( "Evolution of time difference for STS or TOF hits from DPB %02u against any Diamond hit; TS []; <tn - tDia> [ns]", uDpb );
      fvhMcbmTimeDiffToDiamondEvoDpb.push_back( new TH2D( sHistName, sHistTitle,
                                                         2000.0, 0., 200000.,
                                                         1001, -500.5 * stsxyter::kdClockCycleNs, 500.5 * stsxyter::kdClockCycleNs) );

      sHistName  = Form( "hMcbmTimeDiffToDiamondWideEvoDpb%02u", uDpb );
      sHistTitle = Form( "Evolution of time difference for STS or TOF hits from DPB %02u against any Diamond hit, wide range; TS []; <tn - tDia> [us]", uDpb );
      fvhMcbmTimeDiffToDiamondWideEvoDpb.push_back( new TH2D( sHistName, sHistTitle,
                                                         2000.0, 0., 200000.,
                                                         4000.0, -200., 200.) );

      sHistName  = Form( "hMcbmTimeDiffToDiamondTsEvoDpb%02u", uDpb );
      sHistTitle = Form( "Evolution of time difference for STS or TOF hits from DPB %02u against any Diamond hit, TS range; TS []; <tn - tDia> [ms]", uDpb );
      fvhMcbmTimeDiffToDiamondTsEvoDpb.push_back( new TH2D( sHistName, sHistTitle,
                                                         2000.0, 0., 200000.,
                                                         200.0, -10., 10.) );

      sHistName  = Form( "hMcbmTimeDiffToTofEvoDpb%02u", uDpb );
      sHistTitle = Form( "Evolution of time difference for STS or TOF hits from DPB %02u against any Tof hit; TS []; <tn - tTof> [ns]", uDpb );
      fvhMcbmTimeDiffToTofEvoDpb.push_back( new TH2D( sHistName, sHistTitle,
                                                         2000.0, 0., 200000.,
                                                         1001, -500.5 * stsxyter::kdClockCycleNs, 500.5 * stsxyter::kdClockCycleNs) );

      sHistName  = Form( "hMcbmTimeDiffToTofWideEvoDpb%02u", uDpb );
      sHistTitle = Form( "Evolution of time difference for STS or TOF hits from DPB %02u against any Tof hit, wide range; TS []; <tn - tTof> [us]", uDpb );
      fvhMcbmTimeDiffToTofWideEvoDpb.push_back( new TH2D( sHistName, sHistTitle,
                                                         2000.0, 0., 200000.,
                                                         4000.0, -200., 200.) );

      sHistName  = Form( "hMcbmTimeDiffToTofTsEvoDpb%02u", uDpb );
      sHistTitle = Form( "Evolution of time difference for STS or TOF hits from DPB %02u against any Tof hit, TS range; TS []; <tn - tTof> [ms]", uDpb );
      fvhMcbmTimeDiffToTofTsEvoDpb.push_back( new TH2D( sHistName, sHistTitle,
                                                         2000.0, 0., 200000.,
                                                         200.0, -10., 10.) );
   } // for( UInt_t uDpb = 0; uDpb < fuTotalNrOfDpb; ++uDpb )
#ifdef USE_HTTP_SERVER
   THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
   if( server )
   {
		server->Register("/mCbmDt", fhMcbmHitsNbPerMs );

      for( UInt_t uDpb = 0; uDpb < fuTotalNrOfDpb; ++uDpb )
      {
         server->Register("/mCbmDt", fvhMcbmTimeDiffToSelDpb[uDpb] );
         server->Register("/mCbmDt", fvhMcbmTimeDiffToSelDpbWide[uDpb] );
         server->Register("/mCbmDt", fvhMcbmTimeDiffToSelDpbTs[uDpb] );

         server->Register("/mCbmDt", fvhMcbmTimeDiffToDiamondEvoDpb[uDpb] );
         server->Register("/mCbmDt", fvhMcbmTimeDiffToDiamondWideEvoDpb[uDpb] );
         server->Register("/mCbmDt", fvhMcbmTimeDiffToDiamondTsEvoDpb[uDpb] );

         server->Register("/mCbmDt", fvhMcbmTimeDiffToTofEvoDpb[uDpb] );
         server->Register("/mCbmDt", fvhMcbmTimeDiffToTofWideEvoDpb[uDpb] );
         server->Register("/mCbmDt", fvhMcbmTimeDiffToTofTsEvoDpb[uDpb] );
      } // for( UInt_t uDpb = 0; uDpb < fuTotalNrOfDpb; ++uDpb )

      server->RegisterCommand("/Reset_All",  "bMcbm2018ResetPulser=kTRUE");
      server->RegisterCommand("/Save_All",   "bMcbm2018WritePulser=kTRUE");

      server->Restrict("/Reset_All", "allow=admin");
      server->Restrict("/Save_All",  "allow=admin");
   } // if( server )
#endif

   Double_t w = 10;
   Double_t h = 10;
   /** Create Pulser check Canvas for All vs selected DPB **/
   for( UInt_t uDpb = 0; uDpb < fuTotalNrOfDpb; ++uDpb )
   {
		TCanvas* cSyncMcbmDpb = new TCanvas( Form("cSyncMcbmDpb%02u", uDpb ),
		                                     Form("Time difference for STS and TOF hits, per DPB, against any DPB %02u hit", uDpb ), w, h);
		cSyncMcbmDpb->Divide( 3 );

		cSyncMcbmDpb->cd(1);
		gPad->SetGridx();
		gPad->SetGridy();
		gPad->SetLogz();
		fvhMcbmTimeDiffToSelDpb[uDpb]->Draw( "colz" );

		cSyncMcbmDpb->cd(2);
		gPad->SetGridx();
		gPad->SetGridy();
		gPad->SetLogz();
		fvhMcbmTimeDiffToSelDpbWide[uDpb]->Draw( "colz" );

		cSyncMcbmDpb->cd(3);
		gPad->SetGridx();
		gPad->SetGridy();
		gPad->SetLogz();
		fvhMcbmTimeDiffToSelDpbTs[uDpb]->Draw( "colz" );
	} // for( UInt_t uDpb = 0; uDpb < fuTotalNrOfDpb; ++uDpb )
   /*****************************/

  LOG(INFO) << "Done Creating mCBM Histograms" << FairLogger::endl;
}
/***************** mCBM Histograms ************************************/

Bool_t CbmMcbm2018MonitorMcbmPulser::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   /// General commands
   if( bMcbm2018ResetPulser )
   {
      ResetAllHistos();
      bMcbm2018ResetPulser = kFALSE;
   } // if( bMcbm2018ResetPulser )
   if( bMcbm2018WritePulser )
   {
      SaveAllHistos( fsHistoFileFullname );
      bMcbm2018WritePulser = kFALSE;
   } // if( bMcbm2018WritePulser )

   LOG(DEBUG) << "Timeslice contains " << ts.num_microslices(component)
              << "microslices." << FairLogger::endl;
   fulCurrentTsIdx = ts.index();

   /// Ignore first TS as often data corruption there
   if( 0 == fulCurrentTsIdx )
      return kTRUE;

   if( fulCurrentTsIdx < 30 )
      LOG(INFO) << Form( "TS %2llu", fulCurrentTsIdx )
                << FairLogger::endl;

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
      /// Sort the buffers of hits
      for( UInt_t uSdpb = 0; uSdpb < fuStsNrOfDpbs; ++uSdpb )
         std::sort( fvmStsSdpbHitsInMs[ uSdpb ].begin(), fvmStsSdpbHitsInMs[ uSdpb ].end() );
/****************** STS Sync ******************************************/

/****************** TOF Sync ******************************************/
      /// Sort the buffers of hits
      for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfDpbs; ++uGdpb )
         std::sort( fvmTofGdpbHitsInMs[ uGdpb ].begin(), fvmTofGdpbHitsInMs[ uGdpb ].end() );
/****************** TOF Sync ******************************************/

/****************** mCBM Sync *****************************************/
      /// Build time differences for each DPB hit against each sDPB hit
		for( UInt_t uRefSdpb = 0; uRefSdpb < fuStsNrOfDpbs; ++uRefSdpb )
		{
			UInt_t uNbRefHits = fvmStsSdpbHitsInMs[ uRefSdpb ].size();
			for( UInt_t uHitRef = 0; uHitRef < uNbRefHits; uHitRef++)
			{
				Double_t dRefTime = stsxyter::kdClockCycleNs * fvmStsSdpbHitsInMs[ uRefSdpb ][ uHitRef ].GetTs();
				if( fuMuchDpbIdx == uRefSdpb )
					dRefTime -= fdMuchTofOffsetNs;
					else dRefTime -= fdStsTofOffsetNs;

				for( UInt_t uSdpb = 0; uSdpb < fuStsNrOfDpbs; ++uSdpb )
				{
					UInt_t uNbHits = fvmStsSdpbHitsInMs[ uSdpb ].size();
					Double_t dBestDt  = 1e9;
					UInt_t   uBestAdc = 0;
					UInt_t   uNbIncrDt = 0;

					for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )
					{
						if( uRefSdpb == uSdpb && uHitRef == uHit )
							continue;

						Double_t dHitTime = stsxyter::kdClockCycleNs * fvmStsSdpbHitsInMs[ uSdpb ][ uHit ].GetTs();
						if( fuMuchDpbIdx == uSdpb )
							dHitTime -= fdMuchTofOffsetNs;
							else dHitTime -= fdStsTofOffsetNs;

						Double_t dDt = dHitTime - dRefTime;

						if( kTRUE == fbUseBestPair )
						{
							/// Check if this hits is better than the previous ones
							if( TMath::Abs( dDt ) < TMath::Abs( dBestDt ) )
							{
								dBestDt = dDt;
								uBestAdc = fvmStsSdpbHitsInMs[ uSdpb ][ uHit ].GetAdc();
							} // if( TMath::Abs( dDt ) < TMath::Abs( dBestDt ) )
								else if( dBestDt < dDt ) /// Count increasing dt to detect minimum
									uNbIncrDt++;

							/// Stop after 5 increasing dt (hits are time ordered)
							if( 5 == dBestDt )
								break;
						} // if( kTRUE == fbUseBestPair )
							else
							{
								fvhMcbmTimeDiffToSelDpb[ uRefSdpb ]->Fill( dDt, uSdpb );
								fvhMcbmTimeDiffToSelDpbWide[ uRefSdpb ]->Fill( dDt / 1000.0, uSdpb );
							} // else of if( kTRUE == fbUseBestPair )
					} // for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )

					if( kTRUE == fbUseBestPair )
					{
						fvhMcbmTimeDiffToSelDpb[ uRefSdpb ]->Fill( dBestDt, uSdpb );
						fvhMcbmTimeDiffToSelDpbWide[ uRefSdpb ]->Fill( dBestDt / 1000.0, uSdpb );
					} // if( kTRUE == fbUseBestPair )
				} // for( UInt_t uSdpb = 0; uSdpb < fuStsNrOfDpbs; ++uSdpb )

				for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfDpbs; ++uGdpb )
				{
					UInt_t uNbHits = fvmTofGdpbHitsInMs[ uGdpb ].size();
					Double_t dBestDt = 1e9;
					UInt_t   uNbIncrDt = 0;

					for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )
					{
						Double_t dHitTime = fvmTofGdpbHitsInMs[ uGdpb ][ uHit ].GetFullTimeNs();

						Double_t dDt = dHitTime - dRefTime;

						if( kTRUE == fbUseBestPair )
						{
							/// Check if this hits is better than the previous ones
							if( TMath::Abs( dDt ) < TMath::Abs( dBestDt ) )
								dBestDt = dDt;
								else if( dBestDt < dDt ) /// Count increasing dt to detect minimum
									uNbIncrDt++;

							/// Stop after 5 increasing dt (hits are time ordered)
							if( 5 == dBestDt )
								break;
						} // if( kTRUE == fbUseBestPair )
							else
							{
								fvhMcbmTimeDiffToSelDpb[ uRefSdpb ]->Fill( dDt, uGdpb + fuStsNrOfDpbs );
								fvhMcbmTimeDiffToSelDpbWide[ uRefSdpb ]->Fill( dDt / 1000.0, uGdpb + fuStsNrOfDpbs );
							} // else of if( kTRUE == fbUseBestPair )
					} // for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )

					if( kTRUE == fbUseBestPair )
					{
						fvhMcbmTimeDiffToSelDpb[ uRefSdpb ]->Fill( dBestDt, uGdpb + fuStsNrOfDpbs );
						fvhMcbmTimeDiffToSelDpbWide[ uRefSdpb ]->Fill( dBestDt / 1000.0, uGdpb + fuStsNrOfDpbs );
					} // if( kTRUE == fbUseBestPair )
				} // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfDpbs; ++uGdpb )
			} // for( UInt_t uHitRef = 0; uHitRef < uNbRefHits; uHitRef++)
		} // for( UInt_t uRefSdpb = 0; uRefSdpb < fuStsNrOfDpbs; ++uRefSdpb )

      /// Build time differences for each DPB hit against each gDPB hit
		for( UInt_t uRefGdpb = 0; uRefGdpb < fuTofNrOfDpbs; ++uRefGdpb )
		{
			UInt_t uNbRefHits = fvmTofGdpbHitsInMs[ uRefGdpb ].size();
			for( UInt_t uHitRef = 0; uHitRef < uNbRefHits; uHitRef++)
			{
				Double_t dRefTime = fvmTofGdpbHitsInMs[ uRefGdpb ][ uHitRef ].GetFullTimeNs();

				for( UInt_t uSdpb = 0; uSdpb < fuStsNrOfDpbs; ++uSdpb )
				{
					UInt_t uNbHits = fvmStsSdpbHitsInMs[ uSdpb ].size();
					Double_t dBestDt = 1e9;
					UInt_t   uNbIncrDt = 0;

					for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )
					{
						Double_t dHitTime = stsxyter::kdClockCycleNs * fvmStsSdpbHitsInMs[ uSdpb ][ uHit ].GetTs();
						if( fuMuchDpbIdx == uSdpb )
							dHitTime -= fdMuchTofOffsetNs;
							else dHitTime -= fdStsTofOffsetNs;

						Double_t dDt = dHitTime - dRefTime;

						if( kTRUE == fbUseBestPair )
						{
							/// Check if this hits is better than the previous ones
							if( TMath::Abs( dDt ) < TMath::Abs( dBestDt ) )
								dBestDt = dDt;
								else if( dBestDt < dDt ) /// Count increasing dt to detect minimum
									uNbIncrDt++;

							/// Stop after 5 increasing dt (hits are time ordered)
							if( 5 == dBestDt )
								break;
						} // if( kTRUE == fbUseBestPair )
							else
							{
								fvhMcbmTimeDiffToSelDpb[ uRefGdpb + fuStsNrOfDpbs ]->Fill( dDt, uSdpb );
								fvhMcbmTimeDiffToSelDpbWide[ uRefGdpb + fuStsNrOfDpbs ]->Fill( dDt / 1000.0, uSdpb );

								if( fuDiamondDpbIdx == uRefGdpb )
								{
									fvhMcbmTimeDiffToDiamondEvoDpb[ uSdpb ]->Fill( fulCurrentTsIdx, dDt );
									fvhMcbmTimeDiffToDiamondWideEvoDpb[ uSdpb ]->Fill( fulCurrentTsIdx, dDt / 1000.0 );
								} // if( fuDiamondDpbIdx == uRefGdpb )
									else
									{
										fvhMcbmTimeDiffToTofEvoDpb[ uSdpb ]->Fill( fulCurrentTsIdx, dDt );
										fvhMcbmTimeDiffToTofWideEvoDpb[ uSdpb ]->Fill( fulCurrentTsIdx, dDt / 1000.0 );
									} // else of if( fuDiamondDpbIdx == uRefGdpb )

							} // else of if( kTRUE == fbUseBestPair )
					} // for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )

					if( kTRUE == fbUseBestPair )
					{
						fvhMcbmTimeDiffToSelDpb[ uRefGdpb + fuStsNrOfDpbs ]->Fill( dBestDt, uSdpb );
						fvhMcbmTimeDiffToSelDpbWide[ uRefGdpb + fuStsNrOfDpbs ]->Fill( dBestDt / 1000.0, uSdpb );

						if( fuDiamondDpbIdx == uRefGdpb )
						{
							fvhMcbmTimeDiffToDiamondEvoDpb[ uSdpb ]->Fill( fulCurrentTsIdx, dBestDt );
							fvhMcbmTimeDiffToDiamondWideEvoDpb[ uSdpb ]->Fill( fulCurrentTsIdx, dBestDt / 1000.0 );
						} // if( fuDiamondDpbIdx == uRefGdpb )
							else
							{
								fvhMcbmTimeDiffToTofEvoDpb[ uSdpb ]->Fill( fulCurrentTsIdx, dBestDt );
								fvhMcbmTimeDiffToTofWideEvoDpb[ uSdpb ]->Fill( fulCurrentTsIdx, dBestDt / 1000.0 );
							} // else of if( fuDiamondDpbIdx == uRefGdpb )
					} // if( kTRUE == fbUseBestPair )
				} // for( UInt_t uSdpb = 0; uSdpb < fuStsNrOfDpbs; ++uSdpb )

				for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfDpbs; ++uGdpb )
				{
					UInt_t uNbHits = fvmTofGdpbHitsInMs[ uGdpb ].size();
					Double_t dBestDt = 1e9;
					UInt_t   uNbIncrDt = 0;

					for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )
					{

						if( uRefGdpb == uGdpb && uHitRef == uHit )
							continue;

						Double_t dHitTime = fvmTofGdpbHitsInMs[ uGdpb ][ uHit ].GetFullTimeNs();

						Double_t dDt = dHitTime - dRefTime;

						if( kTRUE == fbUseBestPair )
						{
							/// Check if this hits is better than the previous ones
							if( TMath::Abs( dDt ) < TMath::Abs( dBestDt ) )
								dBestDt = dDt;
								else if( dBestDt < dDt ) /// Count increasing dt to detect minimum
									uNbIncrDt++;

							/// Stop after 5 increasing dt (hits are time ordered)
							if( 5 == dBestDt )
								break;
						} // if( kTRUE == fbUseBestPair )
							else
							{
								fvhMcbmTimeDiffToSelDpb[ uRefGdpb + fuStsNrOfDpbs ]->Fill( dDt, uGdpb + fuStsNrOfDpbs );
								fvhMcbmTimeDiffToSelDpbWide[ uRefGdpb + fuStsNrOfDpbs ]->Fill( dDt / 1000.0, uGdpb + fuStsNrOfDpbs );

								if( fuDiamondDpbIdx == uRefGdpb )
								{
									fvhMcbmTimeDiffToDiamondEvoDpb[ uGdpb + fuStsNrOfDpbs ]->Fill( fulCurrentTsIdx, dDt );
									fvhMcbmTimeDiffToDiamondWideEvoDpb[ uGdpb + fuStsNrOfDpbs ]->Fill( fulCurrentTsIdx, dDt / 1000.0 );
								} // if( fuDiamondDpbIdx == uRefGdpb )
									else
									{
										fvhMcbmTimeDiffToTofEvoDpb[ uGdpb + fuStsNrOfDpbs ]->Fill( fulCurrentTsIdx, dDt );
										fvhMcbmTimeDiffToTofWideEvoDpb[ uGdpb + fuStsNrOfDpbs ]->Fill( fulCurrentTsIdx, dDt / 1000.0 );
									} // else of if( fuDiamondDpbIdx == uRefGdpb )
							} // else of if( kTRUE == fbUseBestPair )
					} // for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )

					if( kTRUE == fbUseBestPair )
					{
						fvhMcbmTimeDiffToSelDpb[ uRefGdpb + fuStsNrOfDpbs ]->Fill( dBestDt, uGdpb + fuStsNrOfDpbs );
						fvhMcbmTimeDiffToSelDpbWide[ uRefGdpb + fuStsNrOfDpbs ]->Fill( dBestDt / 1000.0, uGdpb + fuStsNrOfDpbs );

						if( fuDiamondDpbIdx == uRefGdpb )
						{
							fvhMcbmTimeDiffToDiamondEvoDpb[ uGdpb + fuStsNrOfDpbs ]->Fill( fulCurrentTsIdx, dBestDt );
							fvhMcbmTimeDiffToDiamondWideEvoDpb[ uGdpb + fuStsNrOfDpbs ]->Fill( fulCurrentTsIdx, dBestDt / 1000.0 );
						} // if( fuDiamondDpbIdx == uRefGdpb )
							else
							{
								fvhMcbmTimeDiffToTofEvoDpb[ uGdpb + fuStsNrOfDpbs ]->Fill( fulCurrentTsIdx, dBestDt );
								fvhMcbmTimeDiffToTofWideEvoDpb[ uGdpb + fuStsNrOfDpbs ]->Fill( fulCurrentTsIdx, dBestDt / 1000.0 );
							} // else of if( fuDiamondDpbIdx == uRefGdpb )
					} // if( kTRUE == fbUseBestPair )
				} // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfDpbs; ++uGdpb )
			} // for( UInt_t uHitDia = 0; uHitDia < uNbRefHits; uHitDia++)
		} // for( UInt_t uRefGdpb = 0; uRefGdpb < fuTofNrOfDpbs; ++uRefGdpb )

      /// Clear buffers
      for( UInt_t uSdpb = 0; uSdpb < fuStsNrOfDpbs; ++uSdpb )
      {
         fhMcbmHitsNbPerMs->Fill( fvmStsSdpbHitsInMs[ uSdpb ].size(), uSdpb );

         if( fbTsLevelAna )
            fvmStsSdpbHitsInTs[ uSdpb ].insert( fvmStsSdpbHitsInTs[ uSdpb ].end(),
                                                fvmStsSdpbHitsInMs[ uSdpb ].begin(),
                                                fvmStsSdpbHitsInMs[ uSdpb ].end() );

         fvmStsSdpbHitsInMs[ uSdpb ].clear();
      } // for( UInt_t uSdpb = 0; uSdpb < fuStsNrOfDpbs; ++uSdpb )
      for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfDpbs; ++uGdpb )
      {
         fhMcbmHitsNbPerMs->Fill( fvmTofGdpbHitsInMs[ uGdpb ].size(), uGdpb + fuStsNrOfDpbs );

         if( fbTsLevelAna )
            fvmTofGdpbHitsInTs[ uGdpb ].insert( fvmTofGdpbHitsInTs[ uGdpb ].end(),
                                                fvmTofGdpbHitsInMs[ uGdpb ].begin(),
                                                fvmTofGdpbHitsInMs[ uGdpb ].end() );

         fvmTofGdpbHitsInMs[ uGdpb ].clear();
      } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfDpbs; ++uGdpb )
/****************** mCBM Sync *****************************************/

   } // for( UInt_t uMsIdx = 0; uMsIdx < uNbMsLoop; uMsIdx ++ )

/****************** mCBM Sync *****************************************/
   /// Build time differences for each DPB hit against each sDPB hit
	for( UInt_t uRefSdpb = 0; uRefSdpb < fuStsNrOfDpbs; ++uRefSdpb )
	{
		UInt_t uNbRefHits = fvmStsSdpbHitsInTs[ uRefSdpb ].size();
		for( UInt_t uHitRef = 0; uHitRef < uNbRefHits; uHitRef++)
		{
			Double_t dRefTime = stsxyter::kdClockCycleNs * fvmStsSdpbHitsInTs[ fuMuchDpbIdx ][ uHitRef ].GetTs();
			if( fuMuchDpbIdx == uRefSdpb )
				dRefTime -= fdMuchTofOffsetNs;
				else dRefTime -= fdStsTofOffsetNs;

			for( UInt_t uSdpb = 0; uSdpb < fuStsNrOfDpbs; ++uSdpb )
			{
				UInt_t uNbHits = fvmStsSdpbHitsInTs[ uSdpb ].size();
				Double_t dBestDt  = 1e9;
				UInt_t   uBestAdc = 0;
				UInt_t   uNbIncrDt = 0;

				for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )
				{
					if( uRefSdpb == uSdpb && uHitRef == uHit )
						continue;

					Double_t dHitTime = stsxyter::kdClockCycleNs * fvmStsSdpbHitsInTs[ uSdpb ][ uHit ].GetTs();
					if( fuMuchDpbIdx == uSdpb )
						dHitTime -= fdMuchTofOffsetNs;
						else dHitTime -= fdStsTofOffsetNs;

					Double_t dDt = dHitTime - dRefTime;

					/// Limit scan to "reasonnable range" of 300 us
					if( 300e3 < dDt )
						break;

					fvhMcbmTimeDiffToSelDpbTs[ uRefSdpb ]->Fill( dDt / 1e6, uSdpb );
				} // for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )
			} // for( UInt_t uSdpb = 0; uSdpb < fuStsNrOfDpbs; ++uSdpb )

			for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfDpbs; ++uGdpb )
			{
				UInt_t uNbHits = fvmTofGdpbHitsInTs[ uGdpb ].size();
				Double_t dBestDt = 1e9;
				UInt_t   uNbIncrDt = 0;

				for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )
				{
					Double_t dHitTime = fvmTofGdpbHitsInTs[ uGdpb ][ uHit ].GetFullTimeNs();

					Double_t dDt = dHitTime - dRefTime;

					/// Limit scan to "reasonnable range" of 300 us
					if( 300e3 < dDt )
						break;

					fvhMcbmTimeDiffToSelDpbTs[ uRefSdpb ]->Fill( dDt / 1e6, uGdpb + fuStsNrOfDpbs );
				} // for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )
			} // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfDpbs; ++uGdpb )
		} // for( UInt_t uHitRef = 0; uHitRef < uNbRefHits; uHitRef++)
	} // for( UInt_t uRefSdpb = 0; uRefSdpb < fuStsNrOfDpbs; ++uRefSdpb )

   /// Build time differences for each DPB hit against each gDPB hit
	for( UInt_t uRefGdpb = 0; uRefGdpb < fuTofNrOfDpbs; ++uRefGdpb )
	{
		UInt_t uNbRefHits = fvmTofGdpbHitsInTs[ uRefGdpb ].size();
		for( UInt_t uHitRef = 0; uHitRef < uNbRefHits; uHitRef++)
		{
			Double_t dRefTime = fvmTofGdpbHitsInTs[ uRefGdpb ][ uHitRef ].GetFullTimeNs();

			for( UInt_t uSdpb = 0; uSdpb < fuStsNrOfDpbs; ++uSdpb )
			{
				UInt_t uNbHits = fvmStsSdpbHitsInTs[ uSdpb ].size();
				for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )
				{
					Double_t dHitTime = stsxyter::kdClockCycleNs * fvmStsSdpbHitsInTs[ uSdpb ][ uHit ].GetTs();

					Double_t dDt = dHitTime - dRefTime;

					/// Limit scan to "reasonnable range" of 300 us
					if( 300e3 < dDt )
						break;

					fvhMcbmTimeDiffToSelDpbTs[ uRefGdpb + fuStsNrOfDpbs ]->Fill( dDt / 1e6, uSdpb );

					if( fuDiamondDpbIdx == uRefGdpb )
						fvhMcbmTimeDiffToDiamondTsEvoDpb[ uSdpb ]->Fill( fulCurrentTsIdx, dDt / 1e6 );
						else fvhMcbmTimeDiffToTofTsEvoDpb[ uSdpb ]->Fill( fulCurrentTsIdx, dDt / 1e6 );
				} // for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )
			} // for( UInt_t uSdpb = 0; uSdpb < fuStsNrOfDpbs; ++uSdpb )

			for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfDpbs; ++uGdpb )
			{
				UInt_t uNbHits = fvmTofGdpbHitsInTs[ uGdpb ].size();
				for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )
				{
					if( uRefGdpb == uGdpb && uHitRef == uHit )
						continue;

					Double_t dHitTime = fvmTofGdpbHitsInTs[ uGdpb ][ uHit ].GetFullTimeNs();

					Double_t dDt = dHitTime - dRefTime;

					/// Limit scan to "reasonnable range" of 300 us
					if( 300e3 < dDt )
						break;

					fvhMcbmTimeDiffToSelDpbTs[ uRefGdpb + fuStsNrOfDpbs ]->Fill( dDt / 1e6, uGdpb + fuStsNrOfDpbs );

					if( fuDiamondDpbIdx == uRefGdpb )
						fvhMcbmTimeDiffToDiamondTsEvoDpb[ uGdpb + fuStsNrOfDpbs ]->Fill( fulCurrentTsIdx, dDt / 1e6 );
						else fvhMcbmTimeDiffToTofTsEvoDpb[ uGdpb + fuStsNrOfDpbs ]->Fill( fulCurrentTsIdx, dDt / 1e6 );
				} // for( UInt_t uHit = 0; uHit < uNbHits; ++uHit )
			} // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfDpbs; ++uGdpb )
		} // for( UInt_t uHitRef = 0; uHitRef < uNbRefHits; uHitRef++)
	} // for( UInt_t uRefGdpb = 0; uRefGdpb < fuTofNrOfDpbs; ++uRefGdpb )

   /// Clear buffers
   for( UInt_t uSdpb = 0; uSdpb < fuStsNrOfDpbs; ++uSdpb )
   {
      fvmStsSdpbHitsInTs[ uSdpb ].clear();
   } // for( UInt_t uSdpb = 0; uSdpb < fuStsNrOfDpbs; ++uSdpb )
   for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfDpbs; ++uGdpb )
   {
      fvmTofGdpbHitsInTs[ uGdpb ].clear();
   } // for( UInt_t uGdpb = 0; uGdpb < fuTofNrOfDpbs; ++uGdpb )

/****************** mCBM Sync *****************************************/

   if( 0 == ts.index() % 1000 )
   {
      LOG(INFO) << "End of TS " << std::setw(7) << ts.index()
                << FairLogger::endl;
   } // if( 0 == ts.index() % 1000 )

  return kTRUE;
}

/****************** STS Sync ******************************************/
Bool_t CbmMcbm2018MonitorMcbmPulser::ProcessStsMs(const fles::Timeslice& ts, size_t uMsComp, UInt_t uMsIdx)
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

   /** Check the current TS_MSb cycle and correct it if wrong **/
   UInt_t uTsMsbCycleHeader = std::floor( fulCurrentMsIdx /
                                          ( stsxyter::kuTsCycleNbBins * stsxyter::kdClockCycleNs ) );

   if( 0 == uMsIdx )
   {
      fvuStsCurrentTsMsbCycle[ fuCurrDpbIdx ] = uTsMsbCycleHeader;
      fvulStsCurrentTsMsb[fuCurrDpbIdx] = 0;
   } // if( 0 == uMsIdx )
   else if( uTsMsbCycleHeader != fvuStsCurrentTsMsbCycle[ fuCurrDpbIdx ] &&
            4194303 != fvulStsCurrentTsMsb[fuCurrDpbIdx] )
   {
      LOG(WARNING) << "TS MSB cycle from MS header does not match current cycle from data "
                    << "for TS " << std::setw( 12 ) << fulCurrentTsIdx
                    << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                    << " MsInTs " << std::setw( 3 ) << uMsIdx
                    << " ====> " << fvuStsCurrentTsMsbCycle[ fuCurrDpbIdx ]
                    << " VS " << uTsMsbCycleHeader
                    << FairLogger::endl;
      fvuStsCurrentTsMsbCycle[ fuCurrDpbIdx ] = uTsMsbCycleHeader;
   }

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

      switch( typeMess )
      {
         case stsxyter::MessType::Hit :
         {

            FillStsHitInfo( mess, uMsIdx );
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
               LOG(INFO) << "CbmMcbm2018MonitorMcbmPulser::DoUnpack => "
                         << "EPOCH message at unexpected position in MS: message "
                         << uIdx << " VS message 0 expected!"
                         << FairLogger::endl;
            break;
         } // case stsxyter::MessType::TsMsb :
         case stsxyter::MessType::Status :
         case stsxyter::MessType::Empty :
         case stsxyter::MessType::Dummy :
         {
            break;
         } // case stsxyter::MessType::Dummy / ReadDataAck / Ack :
         default:
         {
            LOG(FATAL) << "CbmMcbm2018MonitorMcbmPulser::DoUnpack => "
                       << "Unknown message type, should never happen, stopping here!"
                       << FairLogger::endl;
         }
      } // switch( mess.GetMessType() )
   } // for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )
   return kTRUE;
}

void CbmMcbm2018MonitorMcbmPulser::FillStsHitInfo( stsxyter::Message mess, const UInt_t & uMsIdx )
{
   UShort_t usChan   = mess.GetHitChannel();
   UShort_t usRawAdc = mess.GetHitAdc();
//   UShort_t usFullTs = mess.GetHitTimeFull();
   UShort_t usTsOver = mess.GetHitTimeOver();
   UShort_t usRawTs  = mess.GetHitTime();

   UInt_t uAsicIdx = 0; /// Not used here, otherwise should be extracted from eLink mapping

   if( fuCurrDpbIdx == fuMuchDpbIdx )
   {
      /// MUCH bad channels
      switch( usChan )
      {
         case 101:
         case  99:
         case  91:
         case  89:
         case  88:
         case  86:
         case  84:
         case  83:
         case  80:
         case  78:
         case  76:
         case  50:
         case  39:
         case  37:
         case  35:
         case  20:
         {
            return;
            break;
         } // if bad channel
         default:
            break;
      } // switch( usChan )
   } // if( fuCurrDpbIdx == fuMuchDpbIdx )
      else
      {
         /// Ignore low ADC hits
         if( usRawAdc < 15 )
            return;

         /// STS bad channels
         uAsicIdx = fUnpackParSts->ElinkIdxToAsicIdx( kFALSE, mess.GetLinkIndex() );
         UInt_t uChanIdx = usChan + fUnpackParSts->GetNbChanPerAsic() * uAsicIdx;
         switch( uChanIdx )
         {
            case 781:
            case 270:
            case 411:
            case 518:
            {
               return;
               break;
            } // if bad channel
            default:
               break;
         } // switch( mess.GetLinkIndex() )
         if( ( 0 == uChanIdx % 2 ) && (543 < uChanIdx ) && ( uChanIdx < 633 ) )
         {
            return;
         } // if bad channel
      } // else of if( fuCurrDpbIdx == fuMuchDpbIdx )

   // Compute the Full time stamp
      // Use TS w/o overlap bits as they will anyway come from the TS_MSB
   Long64_t ulStsHitTime = usRawTs;

   ulStsHitTime +=
               static_cast<ULong64_t>( stsxyter::kuHitNbTsBins )
             * static_cast<ULong64_t>( fvulStsCurrentTsMsb[fuCurrDpbIdx])
             + static_cast<ULong64_t>( stsxyter::kuTsCycleNbBins )
             * static_cast<ULong64_t>( fvuStsCurrentTsMsbCycle[fuCurrDpbIdx] )
             ;

   // Convert the Hit time in bins to Hit time in ns
   Double_t dHitTimeNs = ulStsHitTime * stsxyter::kdClockCycleNs;


   // Pulser and MS
   fvmStsSdpbHitsInMs[fuCurrDpbIdx].push_back( stsxyter::FinalHit( ulStsHitTime, usRawAdc, uAsicIdx, usChan ) );
}

void CbmMcbm2018MonitorMcbmPulser::FillStsTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx, UInt_t uMsIdx )
{
   UInt_t uVal    = mess.GetTsMsbVal();

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

   ULong64_t ulNewTsMsbTime =  static_cast< ULong64_t >( stsxyter::kuHitNbTsBins )
                             * static_cast< ULong64_t >( fvulStsCurrentTsMsb[fuCurrDpbIdx])
                             + static_cast< ULong64_t >( stsxyter::kuTsCycleNbBins )
                             * static_cast< ULong64_t >( fvuStsCurrentTsMsbCycle[fuCurrDpbIdx] );
}

void CbmMcbm2018MonitorMcbmPulser::FillStsEpochInfo( stsxyter::Message mess )
{
   UInt_t uVal    = mess.GetTsMsbVal();
}
/****************** STS Sync ******************************************/

/****************** TOF Sync ******************************************/

Bool_t CbmMcbm2018MonitorMcbmPulser::ProcessTofMs( const fles::Timeslice& ts, size_t uMsComp, UInt_t uMsIdx )
{
   auto msDescriptor = ts.descriptor( uMsComp, uMsIdx );
   fuCurrentEquipmentId = msDescriptor.eq_id;
   fdTofMsIndex = static_cast<double>(msDescriptor.idx);
   const uint8_t* msContent = reinterpret_cast<const uint8_t*>( ts.content( uMsComp, uMsIdx ) );

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
   fuTofGdpbNr = fmTofDpbIdIndexMap[fuTofGdpbId];

   // Prepare variables for the loop on contents
   const uint64_t* pInBuff = reinterpret_cast<const uint64_t*>(msContent);
   for( uint32_t uIdx = 0; uIdx < uNbMessages; uIdx++ )
   {
      // Fill message
      uint64_t ulData = static_cast<uint64_t>(pInBuff[uIdx]);
      gdpbv100::Message mess(ulData);

      /// Catch the Epoch cycle block which is always the first 64b of the MS
      if( 0 == uIdx )
      {
         FillTofEpochCycle( ulData );
         continue;
      } // if( 0 == uIdx )

      if (gLogger->IsLogNeeded(DEBUG2))
      {
         mess.printDataCout();
      } // if (gLogger->IsLogNeeded(DEBUG2))

      // Increment counter for different message types
      // and fill the corresponding histogram
      messageType = mess.getMessageType();
      fviTofMsgCounter[messageType]++;

      switch (messageType)
      {
         case gdpbv100::MSG_HIT:
         {
            if( !mess.getGdpbHitIs24b() )
            {
               fvmTofEpSupprBuffer[ fuTofGdpbNr ].push_back( mess );
            } // if( !getGdpbHitIs24b() )
            break;
         } // case gdpbv100::MSG_HIT:
         case gdpbv100::MSG_EPOCH:
         {
            if( gdpbv100::kuChipIdMergedEpoch == mess.getGdpbGenChipId() )
            {
               FillTofEpochInfo( mess );
            } // if this epoch message is a merged one valiud for all chips
               else
            LOG(FATAL) << "Bad epoch: " << mess.getGdpbGenChipId()
                       << FairLogger::endl;
            break;
         } // case gdpbv100::MSG_EPOCH:
         case gdpbv100::MSG_SLOWC:
         case gdpbv100::MSG_SYST:
         case gdpbv100::MSG_STAR_TRI_A:
         case gdpbv100::MSG_STAR_TRI_B:
         case gdpbv100::MSG_STAR_TRI_C:
         case gdpbv100::MSG_STAR_TRI_D:
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

void CbmMcbm2018MonitorMcbmPulser::FillTofEpochCycle( uint64_t ulCycleData )
{
   uint64_t ulEpochCycleVal = ulCycleData & gdpbv100::kulEpochCycleFieldSz;
   fvulTofCurrentEpochCycle[ fuTofGdpbNr ] = ulEpochCycleVal;

   return;
}

void CbmMcbm2018MonitorMcbmPulser::FillTofHitInfo(gdpbv100::Message mess)
{
   UInt_t uChannel = mess.getGdpbHitChanId();
   UInt_t uTot     = mess.getGdpbHit32Tot();
   UInt_t uFts     = mess.getGdpbHitFineTs();

   ULong64_t ulCurEpochGdpbGet4 = fvulTofCurrentEpochFull[ fuTofGdpbNr ];

   // In Ep. Suppr. Mode, receive following epoch instead of previous
   if( 0 < ulCurEpochGdpbGet4 )
      ulCurEpochGdpbGet4 --;
      else ulCurEpochGdpbGet4 = gdpbv100::kuEpochCounterSz; // Catch epoch cycle!

   ULong_t  ulHitTime = mess.getMsgFullTime(ulCurEpochGdpbGet4);
   Double_t dHitTime  = mess.getMsgFullTimeD(ulCurEpochGdpbGet4);

   // In 32b mode the coarse counter is already computed back to 112 FTS bins
   // => need to hide its contribution from the Finetime
   // => FTS = Fullt TS modulo 112
   uFts = mess.getGdpbHitFullTs() % 112;

   /// system sync check buffering
   fvmTofGdpbHitsInMs[ fuTofGdpbNr ].push_back( gdpbv100::FullMessage( mess, ulCurEpochGdpbGet4 ) );
}

void CbmMcbm2018MonitorMcbmPulser::FillTofEpochInfo(gdpbv100::Message mess)
{
   ULong64_t ulEpochNr = mess.getGdpbEpEpochNb();

   fvulTofCurrentEpoch[fuTofGdpbNr] = ulEpochNr;
   fvulTofCurrentEpochFull[fuTofGdpbNr] = ulEpochNr + gdpbv100::kulEpochCycleBins * fvulTofCurrentEpochCycle[fuTofGdpbNr];

   fulTofCurrentEpochTime = mess.getMsgFullTime(ulEpochNr);

   /// Re-align the epoch number of the message in case it will be used later:
   /// We received the epoch after the data instead of the one before!
   if( 0 < ulEpochNr )
      mess.setGdpbEpEpochNb( ulEpochNr - 1 );
      else mess.setGdpbEpEpochNb( gdpbv100::kuEpochCounterSz );

   Int_t iBufferSize = fvmTofEpSupprBuffer[fuTofGdpbNr].size();
   if( 0 < iBufferSize )
   {
      LOG(DEBUG) << "Now processing stored messages for for gDPB " << fuTofGdpbNr << " with epoch number "
                 << (fvulTofCurrentEpoch[fuTofGdpbNr] - 1) << FairLogger::endl;

      /// Data are sorted between epochs, not inside => Epoch level ordering
      /// Sorting at lower bin precision level
      std::stable_sort( fvmTofEpSupprBuffer[fuTofGdpbNr].begin(), fvmTofEpSupprBuffer[fuTofGdpbNr].begin() );

      for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
      {
         FillTofHitInfo( fvmTofEpSupprBuffer[fuTofGdpbNr][ iMsgIdx ] );
      } // for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )

      fvmTofEpSupprBuffer[fuTofGdpbNr].clear();
   } // if( 0 < fvmTofEpSupprBuffer[fuTofGdpbNr] )
}
/****************** TOF Sync ******************************************/

/****************** STS histos ****************************************/
/****************** STS histos ****************************************/

/****************** TOF histos ****************************************/
/****************** TOF Histos ****************************************/

void CbmMcbm2018MonitorMcbmPulser::Reset()
{
}

void CbmMcbm2018MonitorMcbmPulser::Finish()
{
/*
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MonitorMcbmPulser statistics are " << FairLogger::endl;
   LOG(INFO) << " Hit      messages: " << fmMsgCounter[ stsxyter::MessType::Hit   ] << FairLogger::endl
             << " Ts MSB   messages: " << fmMsgCounter[ stsxyter::MessType::TsMsb ] << FairLogger::endl
             << " Dummy    messages: " << fmMsgCounter[ stsxyter::MessType::Dummy ] << FairLogger::endl
             << " Epoch    messages: " << fmMsgCounter[ stsxyter::MessType::Epoch ] << FairLogger::endl
             << " Empty    messages: " << fmMsgCounter[ stsxyter::MessType::Empty ] << FairLogger::endl;
*/
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;

   SaveAllHistos( fsHistoFileFullname );
   SaveAllHistos();

}


void CbmMcbm2018MonitorMcbmPulser::FillOutput(CbmDigi* /*digi*/)
{
}

void CbmMcbm2018MonitorMcbmPulser::SaveAllHistos( TString sFileName )
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
/****************** STS Sync ******************************************/

/****************** MUCH Sync *****************************************/
/****************** MUCH Sync *****************************************/

/****************** TOF Sync ******************************************/
/****************** TOF Sync ******************************************/

/****************** mCBM Sync *****************************************/
   gDirectory->mkdir("mcbmDt");
   gDirectory->cd("mcbmDt");

   for( UInt_t uDpb = 0; uDpb < fuTotalNrOfDpb; ++uDpb )
   {
		fvhMcbmTimeDiffToSelDpb[uDpb]->Reset();
		fvhMcbmTimeDiffToSelDpbWide[uDpb]->Reset();
		fvhMcbmTimeDiffToSelDpbTs[uDpb]->Reset();

      fvhMcbmTimeDiffToDiamondEvoDpb[uDpb]->Write();
      fvhMcbmTimeDiffToDiamondWideEvoDpb[uDpb]->Write();
      fvhMcbmTimeDiffToDiamondTsEvoDpb[uDpb]->Write();

		fvhMcbmTimeDiffToTofEvoDpb[uDpb]->Write();
		fvhMcbmTimeDiffToTofWideEvoDpb[uDpb]->Write();
		fvhMcbmTimeDiffToTofTsEvoDpb[uDpb]->Write();
   } // for( UInt_t uDpb = 0; uDpb < fuTotalNrOfDpb; ++uDpb )

   gDirectory->cd("..");
/****************** mCBM Sync *****************************************/

   if( "" != sFileName )
   {
      // Restore original directory position
      histoFile->Close();
      oldDir->cd();
   } // if( "" != sFileName )

}
void CbmMcbm2018MonitorMcbmPulser::ResetAllHistos()
{
/****************** STS Sync ******************************************/
/****************** STS Sync ******************************************/

/****************** MUCH Sync *****************************************/
/****************** MUCH Sync *****************************************/

/****************** TOF Sync ******************************************/
/****************** TOF Sync ******************************************/


/****************** mCBM Sync *****************************************/
   for( UInt_t uDpb = 0; uDpb < fuTotalNrOfDpb; ++uDpb )
   {
		fvhMcbmTimeDiffToSelDpb[uDpb]->Reset();
		fvhMcbmTimeDiffToSelDpbWide[uDpb]->Reset();
		fvhMcbmTimeDiffToSelDpbTs[uDpb]->Reset();

      fvhMcbmTimeDiffToDiamondEvoDpb[uDpb]->Reset();
      fvhMcbmTimeDiffToDiamondWideEvoDpb[uDpb]->Reset();
      fvhMcbmTimeDiffToDiamondTsEvoDpb[uDpb]->Reset();

		fvhMcbmTimeDiffToTofEvoDpb[uDpb]->Reset();
		fvhMcbmTimeDiffToTofWideEvoDpb[uDpb]->Reset();
		fvhMcbmTimeDiffToTofTsEvoDpb[uDpb]->Reset();
   } // for( UInt_t uDpb = 0; uDpb < fuTotalNrOfDpb; ++uDpb )
/****************** mCBM Sync *****************************************/
}

ClassImp(CbmMcbm2018MonitorMcbmPulser)
