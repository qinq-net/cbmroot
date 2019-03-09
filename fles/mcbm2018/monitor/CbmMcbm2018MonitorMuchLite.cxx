// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                   CbmMcbm2018MonitorMuchLite                        -----
// -----                Created 11/05/18  by P.-A. Loizeau                 -----
// -----                Modified 11/05/18  by Ajit kumar                  -----
// -----                Modified 05/03/19  by Vikas Singhal                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018MonitorMuchLite.h"

// Data

// CbmRoot
#include "CbmMcbm2018MuchPar.h"
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

Bool_t bMcbm2018ResetMuchLite = kFALSE;
Bool_t bMcbm2018WriteMuchLite = kFALSE;
Bool_t bMcbm2018ScanNoisyMuchLite = kFALSE;


CbmMcbm2018MonitorMuchLite::CbmMcbm2018MonitorMuchLite() :
	CbmMcbmUnpack(),
	fbMuchMode(kFALSE),
   fvbMaskedComponents(),
   fvMsComponentsList(),
   fuNbCoreMsPerTs(0),
   fuNbOverMsPerTs(0),
   //uTimeBin(1e-9),
   fbIgnoreOverlapMs(kFALSE),
   fUnpackParMuch(NULL),
   fuNrOfDpbs(0),
   fDpbIdIndexMap(),
   fvbCrobActiveFlag(),
   fuNbFebs(0),
   fuNbStsXyters(0),
   //fvdFebAdcGain(),
   //fvdFebAdcOffs(),
   /*
   fuNrOfDpbs(0),
   fDpbIdIndexMap(),
   fuNbStsXyters(0),
   fUnpackParMuch->GetNbChanPerAsic()(0),
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
   fvdPrevMsTime(),
   fvdMsTime(),
   fvuChanNbHitsInMs(),
   fvdChanLastHitTimeInMs(),
   fvusChanLastHitAdcInMs(),
   //   fvmChanHitsInTs(),
   prevTime(0.0),
   prevChan(0.0),
   prevAsic(0.0),
   prevtime_new(0.0),
   fdStartTime(-1.0),
   fdStartTimeMsSz(-1.0),
   ftStartTimeUnix( std::chrono::steady_clock::now() ),
   fvmHitsInMs(),
   fvmAsicHitsInMs(),
   fvmFebHitsInMs(),
   fuMaxNbMicroslices(100),
   fiTimeIntervalRateUpdate( 10 ),
   fviFebTimeSecLastRateUpdate(),
   fviFebCountsSinceLastRateUpdate(),
   fvdFebChanCountsSinceLastRateUpdate(),
   /*fbLongHistoEnable( kFALSE ),
   fuLongHistoNbSeconds( 0 ),
   fuLongHistoBinSizeSec( 0 ),
   fuLongHistoBinNb( 0 ),*/
   Counter( 0 ),
   Counter1( 0 ),
   fHM(new CbmHistManager()),
   fhRate(NULL),
   fhRateAdcCut(NULL),
   fhMuchMessType(NULL),
   fhMuchSysMessType(NULL),
   fhMuchMessTypePerDpb(NULL),
   fhMuchFebChanAdcRaw_combined(NULL),
   fhMuchSysMessTypePerDpb(NULL),
   fhStatusMessType(NULL),
   fhMsStatusFieldType(NULL),
   fhMuchHitsElinkPerDpb(NULL),
   fhMuchFebChanCntRaw(),
   //fhMuchFebChanCntRawGood(),
   fhMuchFebChanAdcRaw(),
   fhMuchFebChanAdcRawProf(),
   fhMuchFebChanRawTs(),
   fHistPadDistr(),
   fRealHistPadDistr(),
   fhMuchFebChanHitRateEvo(),
   fhMuchFebChanHitRateProf(),
   //fhMuchFebAsicHitRateEvo(),
   fhMuchFebHitRateEvo(),
   fhMuchFebHitRateEvo_mskch(),
   fhMuchFebHitRateEvo_mskch_adccut(),
   fhMuchFebHitRateEvo_WithoutDupli(),
   fdMuchFebChanLastTimeForDist(),
   fhMuchFebChanDistT(),
   fhMuchFebDuplicateHitProf(),
   fcMsSizeAll(NULL)
{
}



CbmMcbm2018MonitorMuchLite::~CbmMcbm2018MonitorMuchLite()
{
}


Bool_t CbmMcbm2018MonitorMuchLite::Init()
{
  LOG(INFO) << "Initializing flib StsXyter unpacker for MUCH" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  return kTRUE;
}

void CbmMcbm2018MonitorMuchLite::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
         << FairLogger::endl;
   fUnpackParMuch = (CbmMcbm2018MuchPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmMcbm2018MuchPar"));
}

Bool_t CbmMcbm2018MonitorMuchLite::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
         << FairLogger::endl;

   Bool_t bInit = InitMuchParameters();
   if( kTRUE == bInit )
      CreateHistograms();

   return bInit;
}

Bool_t CbmMcbm2018MonitorMuchLite::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
             << FairLogger::endl;

   return InitMuchParameters();
}


Bool_t CbmMcbm2018MonitorMuchLite::InitMuchParameters()
{

   fuNrOfDpbs = fUnpackParMuch->GetNrOfDpbs();
   LOG(INFO) << "Nr. of MUCH DPBs:       " << fuNrOfDpbs
             << FairLogger::endl;

   fDpbIdIndexMap.clear();
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fDpbIdIndexMap[ fUnpackParMuch->GetDpbId( uDpb )  ] = uDpb;
      LOG(INFO) << "Eq. ID for DPB #" << std::setw(2) << uDpb << " = 0x"
                << std::setw(4) << std::hex << fUnpackParMuch->GetDpbId( uDpb )
                << std::dec
                << " => " << fDpbIdIndexMap[ fUnpackParMuch->GetDpbId( uDpb )  ]
                << FairLogger::endl;
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   fuNbFebs      = fUnpackParMuch->GetNrOfFebs();
   LOG(INFO) << "Nr. of FEBs:           " << fuNbFebs
             << FairLogger::endl;

   fuNbStsXyters = fUnpackParMuch->GetNrOfAsics();
   LOG(INFO) << "Nr. of StsXyter ASICs: " << fuNbStsXyters
             << FairLogger::endl;

   fvbCrobActiveFlag.resize( fuNrOfDpbs );
   //fvdFebAdcGain.resize(     fuNrOfDpbs );
   //fvdFebAdcOffs.resize(     fuNrOfDpbs );
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fvbCrobActiveFlag[ uDpb ].resize( fUnpackParMuch->GetNbCrobsPerDpb() );
      //fvdFebAdcGain[ uDpb ].resize(        fUnpackParMuch->GetNbCrobsPerDpb() );
      //fvdFebAdcOffs[ uDpb ].resize(        fUnpackParMuch->GetNbCrobsPerDpb() );
      for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackParMuch->GetNbCrobsPerDpb(); ++uCrobIdx )
      {
         fvbCrobActiveFlag[ uDpb ][ uCrobIdx ] = fUnpackParMuch->IsCrobActive( uDpb, uCrobIdx );
         // fvdFebAdcGain[ uDpb ][ uCrobIdx ].resize(     fUnpackParMuch->GetNbFebsPerCrob(), 0.0 );
         //fvdFebAdcOffs[ uDpb ][ uCrobIdx ].resize(     fUnpackParMuch->GetNbFebsPerCrob(), 0.0 );
      } // for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackParMuch->GetNbCrobsPerDpb(); ++uCrobIdx )
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      //LOG(INFO) << Form( "DPB #%02u CROB Active ?:       ", uDpb);
      for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackParMuch->GetNbCrobsPerDpb(); ++uCrobIdx )
      {
         LOG(INFO) << Form("DPB #%02u CROB #%02u Active:  ", uDpb, uCrobIdx) <<  fvbCrobActiveFlag[ uDpb ][ uCrobIdx ] << " ";
      } // for( UInt_t uCrobIdx = 0; uCrobIdx < fUnpackParMuch->GetNbCrobsPerDpb(); ++uCrobIdx )
      LOG(INFO) << FairLogger::endl;
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )

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

   fvdPrevMsTime.resize( kiMaxNbFlibLinks );
   fvulChanLastHitTime.resize( fuNbStsXyters );
   fvdChanLastHitTime.resize( fuNbStsXyters );
   fvdMsTime.resize( fuMaxNbMicroslices );
   fvuChanNbHitsInMs.resize( fuNbFebs );
   fvdChanLastHitTimeInMs.resize( fuNbFebs );
   fvusChanLastHitAdcInMs.resize( fuNbFebs );
   fvmAsicHitsInMs.resize( fuNbFebs );

   //fvdMsTime.resize( fuMaxNbMicroslices );
   //fvuChanNbHitsInMs.resize( fuNbStsXyters );
   //fvdChanLastHitTimeInMs.resize( fuNbStsXyters );
   //fvusChanLastHitAdcInMs.resize( fuNbStsXyters );
   //fvmAsicHitsInMs.resize( fuNbStsXyters );

   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbFebs; ++uXyterIdx )
   {
      fvulChanLastHitTime[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
      fvdChanLastHitTime[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
      fvuChanNbHitsInMs[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
      fvdChanLastHitTimeInMs[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
      fvusChanLastHitAdcInMs[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
      fvmAsicHitsInMs[ uXyterIdx ].clear();

      for( UInt_t uChan = 0; uChan < fUnpackParMuch->GetNbChanPerAsic(); ++uChan )
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
      } // for( UInt_t uChan = 0; uChan < fUnpackParMuch->GetNbChanPerAsic(); ++uChan )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )

   LOG(INFO) << "CbmMcbm2018MonitorMuchLite::ReInitContainers => Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs.size() << " VS " << fuNbFebs
                << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MonitorMuchLite::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fUnpackParMuch->GetNbChanPerAsic()
                << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MonitorMuchLite::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuMaxNbMicroslices
                << FairLogger::endl;

   fvmFebHitsInMs.resize( fuNbFebs );
   fviFebTimeSecLastRateUpdate.resize( fuNbFebs, -1 );
   fviFebCountsSinceLastRateUpdate.resize( fuNbFebs, -1 );
   fvdFebChanCountsSinceLastRateUpdate.resize( fuNbFebs );
   fdMuchFebChanLastTimeForDist.resize( fuNbFebs );
   for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
   {
      fvmFebHitsInMs[ uFebIdx ].clear();
      fvdFebChanCountsSinceLastRateUpdate[ uFebIdx ].resize( fUnpackParMuch->GetNbChanPerFeb(), 0.0 );
      fdMuchFebChanLastTimeForDist[ uFebIdx ].resize( fUnpackParMuch->GetNbChanPerFeb(), -1.0 );
   } // for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )

///----------------- SXM 2.0 Logic Error Tagging --------------------///
//   SmxErrInitializeVariables();
///------------------------------------------------------------------///

   return kTRUE;
}

void CbmMcbm2018MonitorMuchLite::AddMsComponentToList( size_t component, UShort_t usDetectorId )
{
   /// Check for duplicates and ignore if it is the case
   for( UInt_t uCompIdx = 0; uCompIdx < fvMsComponentsList.size(); ++uCompIdx )
      if( component == fvMsComponentsList[ uCompIdx ] )
         return;

   /// Check if this does not go above hardcoded limits
   if( kiMaxNbFlibLinks <= component  )
   {
      LOG(ERROR) << "CbmMcbm2018MonitorMuchLite::AddMsComponentToList => "
                 << "Ignored the addition of component " << component
                 << " as it is above the hadcoded limit of " << static_cast<const Int_t>(kiMaxNbFlibLinks)
                 << " !!!!!!!!! "
                 << FairLogger::endl
                 << "         To change this behavior check kiMaxNbFlibLinks in CbmMcbm2018MonitorMuchLite.cxx"
                 << FairLogger::endl;
      return;
   } // if( kiMaxNbFlibLinks <= component  )


   /// Add to list
   fvMsComponentsList.push_back( component );
   LOG(INFO) << "CbmMcbm2018MonitorMuchLite::AddMsComponentToList => Added component: " << component
             << FairLogger::endl;

   /// Create MS size monitoring histos
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

      THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
      if( server )
      {
         server->Register("/FlibRaw", fhMsSz[ component ] );
         server->Register("/FlibRaw", fhMsSzTime[ component ] );
      } // if( server )
   } // if( NULL == fhMsSz[ component ] )
}

void CbmMcbm2018MonitorMuchLite::SetNbMsInTs( size_t uCoreMsNb, size_t uOverlapMsNb )
{
   fuNbCoreMsPerTs = uCoreMsNb;
   fuNbOverMsPerTs = uOverlapMsNb;
   //LOG(INFO) <<" fuNbCoreMsPerTs "<<fuNbCoreMsPerTs<<" fuNbOverMsPerTs "<<fuNbOverMsPerTs<<FairLogger::endl;
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
         fvuChanNbHitsInMs[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
         fvdChanLastHitTimeInMs[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
         fvusChanLastHitAdcInMs[ uXyterIdx ].resize( fUnpackParMuch->GetNbChanPerAsic() );
         for( UInt_t uChan = 0; uChan < fUnpackParMuch->GetNbChanPerAsic(); ++uChan )
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
         } // for( UInt_t uChan = 0; uChan < fUnpackParMuch->GetNbChanPerAsic(); ++uChan )
      } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      LOG(INFO) << "CbmMcbm2018MonitorMuchLite::DoUnpack => Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                   << FairLogger::endl;
      LOG(INFO) << "CbmMcbm2018MonitorMuchLite::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fUnpackParMuch->GetNbChanPerAsic()
                   << FairLogger::endl;
      LOG(INFO) << "CbmMcbm2018MonitorMuchLite::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuMaxNbMicroslices
                   << FairLogger::endl;
   } // if( fuMaxNbMicroslices < uNbMsTotal )
}

void CbmMcbm2018MonitorMuchLite::CreateHistograms()
{
   TString sHistName{""};
   TString title{""};

   sHistName = "hMessageType";
   title = "Nb of message for each type; Type";
   fhMuchMessType = new TH1I(sHistName, title, 6, 0., 6.);
   fhMuchMessType->GetXaxis()->SetBinLabel( 1, "Dummy");
   fhMuchMessType->GetXaxis()->SetBinLabel( 2, "Hit");
   fhMuchMessType->GetXaxis()->SetBinLabel( 3, "TsMsb");
   fhMuchMessType->GetXaxis()->SetBinLabel( 4, "Epoch");
   fhMuchMessType->GetXaxis()->SetBinLabel( 5, "Status");
   fhMuchMessType->GetXaxis()->SetBinLabel( 6, "Empty");


   sHistName = "hSysMessType";
   title = "Nb of system message for each type; System Type";
   fhMuchSysMessType = new TH1I(sHistName, title, 17, 0., 17.);

   sHistName = "hMuchFebChanAdcRaw_combined";
   title = "ADC hist combined";
   fhMuchFebChanAdcRaw_combined = new TH1I(sHistName, title, stsxyter::kuHitNbAdcBins, -0.5, stsxyter::kuHitNbAdcBins -0.5);

   LOG(DEBUG)<< "Initialized 1st Histo" << FairLogger::endl;
   sHistName = "hMessageTypePerDpb";
   title = "Nb of message of each type for each DPB; DPB; Type";
   fhMuchMessTypePerDpb = new TH2I(sHistName, title, fuNrOfDpbs, 0, fuNrOfDpbs, 6, 0., 6.);
   fhMuchMessTypePerDpb->GetYaxis()->SetBinLabel( 1, "Dummy");
   fhMuchMessTypePerDpb->GetYaxis()->SetBinLabel( 2, "Hit");
   fhMuchMessTypePerDpb->GetYaxis()->SetBinLabel( 3, "TsMsb");
   fhMuchMessTypePerDpb->GetYaxis()->SetBinLabel( 4, "Epoch");
   fhMuchMessTypePerDpb->GetYaxis()->SetBinLabel( 5, "Status");
   fhMuchMessTypePerDpb->GetYaxis()->SetBinLabel( 6, "Empty");

   for( UInt_t uModuleId = 0; uModuleId < 2; ++uModuleId )
   {
      /// Raw Ts Distribution
      sHistName = Form( "HistPadDistr_Module_%01u", uModuleId );
      title = Form( "Pad distribution for, Module #%01u; ", uModuleId );

      //Below for Rectangular Module shape VS
      fHistPadDistr.push_back( new TH2I(sHistName, title,
                                 23, -0.5, 22.5,
                                 97, -0.5, 96.5 ) );

      sHistName = Form( "RealHistPadDistr_Module_%01u", uModuleId );
      title = Form( "Progressive Pad distribution for, Module #%01u; ", uModuleId );
      //Below for Progressive Geometry Module shape VS
      fRealHistPadDistr.push_back( new TH2D(sHistName, title,
                                500,-0.5,499.5,
			        1000, -0.5, 999.5) );


      /// FEB wise Duplicate Hit profile for each Module (If same hit at same time, same channel and same FEB)
      sHistName = Form( "hMuchFebDuplicateHitProf_%01u", uModuleId );
      title = Form( "FEB wise Duplicate Hit for Module #%01u; FEB []; Hit []", uModuleId );
      if(uModuleId==0)
      //fhMuchFebDuplicateHitProf.push_back( new TProfile(sHistName, title,fUnpackParMuch->GetNrOfFebsInGemA(), -0.5, fUnpackParMuch->GetNrOfFebsInGemA() - 0.5 ) );
      	fhMuchFebDuplicateHitProf.push_back( new TProfile(sHistName, title,18, -0.5, 18 - 0.5 ) );
      if(uModuleId==1)
      //fhMuchFebDuplicateHitProf.push_back( new TProfile(sHistName, title,fUnpackParMuch->GetNrOfFebsInGemB(), -0.5, fUnpackParMuch->GetNrOfFebsInGemB() - 0.5 ) );
     	 fhMuchFebDuplicateHitProf.push_back( new TProfile(sHistName, title,18, -0.5, 18 - 0.5 ) );

   }

   sHistName = "hRate";
   title = "Rate in kHz";
   fhRate = new TH1I(sHistName, title, 10000, -0.5, 9999.5);

   sHistName = "hRateAdcCut";
   title = "Rate in kHz with Adc cut";
   fhRateAdcCut = new TH1I(sHistName, title, 10000, -0.5, 9999.5);

   sHistName = "hSysMessTypePerDpb";
   title = "Nb of system message of each type for each DPB; DPB; System Type";
   fhMuchSysMessTypePerDpb = new TH2I(sHistName, title, fuNrOfDpbs, 0, fuNrOfDpbs, 17, 0., 17.);

   sHistName = "hStatusMessType";
   title = "Nb of status message of each type for each DPB; ASIC; Status Type";
   fhStatusMessType = new TH2I(sHistName, title, fuNbStsXyters, 0, fuNbStsXyters,
                                       16, 0., 16.);


   sHistName = "hMsStatusFieldType";
   title = "For each flag in the MS header, ON/OFF counts; Flag bit []; ON/OFF; MS []";
   fhMsStatusFieldType = new TH2I(sHistName, title, 16, -0.5, 15.5, 2, -0.5, 1.5);

   //For mCBM March 2019 data taking we will have only one eLink enable for each FEB
   sHistName = "hMuchHitsElinkPerDpb";
   title = "Nb of hit messages per eLink for each DPB; DPB; eLink; Hits nb []";
   fhMuchHitsElinkPerDpb = new TH2I(sHistName, title, fuNrOfDpbs, 0, fuNrOfDpbs, 42, 0., 42.);

   LOG(DEBUG)<< "Initialized 2nd Histo" << FairLogger::endl;

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

  LOG(DEBUG)<< "Initialized 3rd Histo" << FairLogger::endl;
  ///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++///
   //UInt_t uAlignedLimit = fuLongHistoNbSeconds - (fuLongHistoNbSeconds % fuLongHistoBinSizeSec);
   UInt_t uAlignedLimit = 0;

   UInt_t uNbBinEvo = (32768 + 1) * 2;
   Double_t dMaxEdgeEvo = stsxyter::kdClockCycleNs
                         * static_cast< Double_t >( uNbBinEvo ) / 2.0;
   Double_t dMinEdgeEvo = dMaxEdgeEvo * -1.0;

   //UInt_t uNbBinDt     = static_cast<UInt_t>( (fdCoincMax - fdCoincMin )/stsxyter::kdClockCycleNs );

   // Miscroslice properties histos
   for( Int_t component = 0; component < kiMaxNbFlibLinks; component ++ )
   {
      fhMsSz[ component ] = NULL;
      fhMsSzTime[ component ] = NULL;
   } // for( Int_t component = 0; component < kiMaxNbFlibLinks; component ++ )

   /// All histos per FEB: with channels or ASIC as axis!!
   // fhMuchFebChanDtCoinc.resize( fuNbFebs );
   // fhMuchFebChanCoinc.resize( fuNbFebs );
   for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
   {
      /// Channel counts
      sHistName = Form( "hMuchFebChanCntRaw_%03u", uFebIdx );
      title = Form( "Hits Count per channel, FEB #%03u; Channel; Hits []", uFebIdx );
      fhMuchFebChanCntRaw.push_back( new TH1I(sHistName, title,
                                  fUnpackParMuch->GetNbChanPerFeb(), -0.5, fUnpackParMuch->GetNbChanPerFeb() - 0.5 ) );

      //sHistName = Form( "hMuchFebChanCntRawGood_%03u", uFebIdx );
      //title = Form( "Hits Count per channel in good MS (SX2 bug flag off), FEB #%03u; Channel; Hits []", uFebIdx );
      //fhMuchFebChanCntRawGood.push_back( new TH1I(sHistName, title,
      //                          fUnpackParMuch->GetNbChanPerFeb(), -0.5, fUnpackParMuch->GetNbChanPerFeb() - 0.5 ) );

      /// Raw Adc Distribution
      sHistName = Form( "hMuchFebChanAdcRaw_%03u", uFebIdx );
      title = Form( "Raw Adc distribution per channel, FEB #%03u; Channel []; Adc []; Hits []", uFebIdx );
      fhMuchFebChanAdcRaw.push_back( new TH2I(sHistName, title,
                                 fUnpackParMuch->GetNbChanPerFeb(), -0.5, fUnpackParMuch->GetNbChanPerFeb() - 0.5,
                                 stsxyter::kuHitNbAdcBins, -0.5, stsxyter::kuHitNbAdcBins -0.5 ) );

      /// Raw Adc Distribution profile
      sHistName = Form( "hMuchFebChanAdcRawProfc_%03u", uFebIdx );
      title = Form( "Raw Adc prodile per channel, FEB #%03u; Channel []; Adc []", uFebIdx );
      fhMuchFebChanAdcRawProf.push_back( new TProfile(sHistName, title,
                                 fUnpackParMuch->GetNbChanPerFeb(), -0.5, fUnpackParMuch->GetNbChanPerFeb() - 0.5 ) );

      /// Cal Adc Distribution
      //sHistName = Form( "hMuchFebChanAdcCal_%03u", uFebIdx );
      //title = Form( "Cal. Adc distribution per channel, FEB #%03u; Channel []; Adc [e-]; Hits []", uFebIdx );
      //fhMuchFebChanAdcCal.push_back( new TH2I(sHistName, title,
      //                           fUnpackParMuch->GetNbChanPerFeb(), -0.5, fUnpackParMuch->GetNbChanPerFeb() - 0.5,
      //                            50, 0., 100000. ) );

      /// Cal Adc Distribution profile
      //sHistName = Form( "hMuchFebChanAdcCalProfc_%03u", uFebIdx );
      //title = Form( "Cal. Adc prodile per channel, FEB #%03u; Channel []; Adc [e-]", uFebIdx );
      //fhMuchFebChanAdcCalProf.push_back( new TProfile(sHistName, title,
      //                           fUnpackParMuch->GetNbChanPerFeb(), -0.5, fUnpackParMuch->GetNbChanPerFeb() - 0.5 ) );

      /// Raw Ts Distribution
      sHistName = Form( "hMuchFebChanRawTs_%03u", uFebIdx );
      title = Form( "Raw Timestamp distribution per channel, FEB #%03u; Channel []; Ts []; Hits []", uFebIdx );
      fhMuchFebChanRawTs.push_back( new TH2I(sHistName, title,
                                 fUnpackParMuch->GetNbChanPerFeb(), -0.5, fUnpackParMuch->GetNbChanPerFeb() - 0.5,
                                 stsxyter::kuHitNbTsBins, -0.5, stsxyter::kuHitNbTsBins -0.5 ) );

      /// Hit rates evo per channel
      sHistName = Form( "hMuchFebChanRateEvo_%03u", uFebIdx );
      title = Form( "Hits per second & channel in FEB #%03u; Time [s]; Channel []; Hits []", uFebIdx );
      fhMuchFebChanHitRateEvo.push_back( new TH2I( sHistName, title,
                                                1800, 0, 1800,
                                                fUnpackParMuch->GetNbChanPerFeb(), -0.5, fUnpackParMuch->GetNbChanPerFeb() - 0.5 ) );

      /// Hit rates profile per channel
      sHistName = Form( "hMuchFebChanRateProf_%03u", uFebIdx );
      title = Form( "Hits per second for each channel in FEB #%03u; Channel []; Hits/s []", uFebIdx );
      fhMuchFebChanHitRateProf.push_back( new TProfile( sHistName, title,
                                                fUnpackParMuch->GetNbChanPerFeb(), -0.5, fUnpackParMuch->GetNbChanPerFeb() - 0.5 ) );

      /// Hit rates evo per StsXyter
      // sHistName = Form( "hMuchFebAsicRateEvo_%03u", uFebIdx );
      // title = Form( "Hits per second & StsXyter in FEB #%03u; Time [s]; Asic []; Hits []", uFebIdx );
      // fhMuchFebAsicHitRateEvo.push_back( new TH2I( sHistName, title, 1800, 0, 1800,
      //                                         fUnpackParMuch->GetNbAsicsPerFeb(), -0.5, fUnpackParMuch->GetNbAsicsPerFeb() - 0.5  ) );

      /// Hit rates evo per FEB
      sHistName = Form( "hMuchFebRateEvo_%03u", uFebIdx );
      title = Form( "Hits per second in FEB #%03u; Time [s]; Hits []", uFebIdx );
      fhMuchFebHitRateEvo.push_back( new TH1I(sHistName, title, 1800, 0, 1800 ) );


      /// Hit rates evo per FEB for Mask Channel
      sHistName = Form( "hMuchFebRateEvo_mskch_%03u", uFebIdx );
      title = Form( "Hits per second in FEB #%03u; Time [s]; Hits []", uFebIdx );
      fhMuchFebHitRateEvo_mskch.push_back( new TH1I(sHistName, title, 1800, 0, 1800 ) );

      /// Hit rates evo per FEB for Mask Channel with ADC Cut
      sHistName = Form( "hMuchFebRateEvo_mskch_adcut_%03u", uFebIdx );
      title = Form( "Hits per second in FEB #%03u; Time [s]; Hits []", uFebIdx );
      fhMuchFebHitRateEvo_mskch_adccut.push_back( new TH1I(sHistName, title, 1800, 0, 1800 ) );

      /// Hit rates evo per FEB
      sHistName = Form( "hMuchFebRateEvo_WithoutDupli_%03u", uFebIdx );
      title = Form( "Hits per second in FEB #%03u; Time [s]; Hits []", uFebIdx );
      fhMuchFebHitRateEvo_WithoutDupli.push_back( new TH1I(sHistName, title, 50000, 0, 5000 ) );

      /// Hit rates evo per channel, 1 minute bins, 24h
      //sHistName = Form( "hMuchFebChanRateEvoLong_%03u", uFebIdx );
      //title = Form( "Hits per second & channel in FEB #%03u; Time [min]; Channel []; Hits []", uFebIdx );
      //fhMuchFebChanHitRateEvoLong.push_back( new TH2D( sHistName, title,
      //                                          fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5,
      //                                          fUnpackParMuch->GetNbChanPerFeb(), -0.5, fUnpackParMuch->GetNbChanPerFeb() - 0.5 ) );

      /// Hit rates evo per channel, 1 minute bins, 24h
      //sHistName = Form( "hMuchFebAsicRateEvoLong_%03u", uFebIdx );
      //title = Form( "Hits per second & StsXyter in FEB #%03u; Time [min]; Asic []; Hits []", uFebIdx );
      //fhMuchFebAsicHitRateEvoLong.push_back( new TH2D( sHistName, title,
      //                                          fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5,
      //                                          fUnpackParMuch->GetNbAsicsPerFeb(), -0.5, fUnpackParMuch->GetNbAsicsPerFeb() - 0.5 ) );

      /// Hit rates evo per FEB, 1 minute bins, 24h
      //sHistName = Form( "hMuchFebRateEvoLong_%03u", uFebIdx );
      //title = Form( "Hits per second in FEB #%03u; Time [min]; Hits []", uFebIdx );
      //fhMuchFebHitRateEvoLong.push_back( new TH1D(sHistName, title,
      //                                          fuLongHistoBinNb, -0.5, uAlignedLimit - 0.5 ) );

      /// Distance between hits on same channel
      sHistName = Form( "hMuchFebChanDistT_%03u", uFebIdx );
      title = Form( "Time distance between hits on same channel in between FEB #%03u; Time difference [ns]; Channel []; ",
                     uFebIdx );
      fhMuchFebChanDistT.push_back( new TH2I( sHistName, title,
                                             1000, -0.5, 6250.0 - 0.5,
                                             fUnpackParMuch->GetNbChanPerFeb(), -0.5, fUnpackParMuch->GetNbChanPerFeb() - 0.5) );

   } // for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++///

  // Miscroslice properties histos
   for( Int_t component = 0; component < kiMaxNbFlibLinks; component ++ )
   {
      fhMsSz[ component ] = NULL;
      fhMsSzTime[ component ] = NULL;
   } // for( Int_t component = 0; component < kiMaxNbFlibLinks; component ++ )

   LOG(DEBUG)<< "Initialized 6th Histo before FairRunOnlne Instance" << FairLogger::endl;
   THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
   if( server )
   {
     for( UInt_t uModuleId = 0; uModuleId < 2; ++uModuleId )
     {
      server->Register("/MuchRaw", fHistPadDistr[uModuleId] );
      server->Register("/MuchRaw", fRealHistPadDistr[uModuleId] );
      server->Register("/MuchFeb", fhMuchFebDuplicateHitProf[uModuleId] );
     }

      server->Register("/MuchRaw", fhRate );
      server->Register("/MuchRaw", fhRateAdcCut );
      server->Register("/MuchRaw", fhMuchMessType );
      server->Register("/MuchRaw", fhMuchSysMessType );
      server->Register("/MuchRaw", fhMuchMessTypePerDpb );
      server->Register("/MuchRaw", fhMuchSysMessTypePerDpb );
      server->Register("/MuchRaw", fhStatusMessType );
      server->Register("/MuchRaw", fhMsStatusFieldType );
      server->Register("/MuchRaw", fhMuchHitsElinkPerDpb );
      server->Register("/MuchRaw", fhMuchFebChanAdcRaw_combined );
      for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
      {
         if( kTRUE == fUnpackParMuch->IsFebActive( uFebIdx ) )
         {
            server->Register("/MuchFeb", fhMuchFebChanCntRaw[ uFebIdx ] );
            //server->Register("/MuchFeb", fhMuchFebChanCntRawGood[ uFebIdx ] );
            server->Register("/MuchFeb", fhMuchFebChanAdcRaw[ uFebIdx ] );
            server->Register("/MuchFeb", fhMuchFebChanAdcRawProf[ uFebIdx ] );
            //server->Register("/MuchFeb", fhMuchFebChanAdcCal[ uFebIdx ] );
            //server->Register("/MuchFeb", fhMuchFebChanAdcCalProf[ uFebIdx ] );
            server->Register("/MuchFeb", fhMuchFebChanRawTs[ uFebIdx ] );
            server->Register("/MuchFeb", fhMuchFebChanHitRateEvo[ uFebIdx ] );
            server->Register("/MuchFeb", fhMuchFebChanHitRateProf[ uFebIdx ] );
            //server->Register("/MuchFeb", fhMuchFebAsicHitRateEvo[ uFebIdx ] );
            server->Register("/MuchFeb", fhMuchFebHitRateEvo[ uFebIdx ] );
            server->Register("/MuchFeb", fhMuchFebHitRateEvo_mskch[ uFebIdx ] );
            server->Register("/MuchFeb", fhMuchFebHitRateEvo_mskch_adccut[ uFebIdx ] );
            server->Register("/MuchFeb", fhMuchFebHitRateEvo_WithoutDupli[ uFebIdx ] );
   	    LOG(DEBUG)<< "Initialized fhMuchFebHitRateEvo_WithoutDupli number "<< uFebIdx << FairLogger::endl;
            /*server->Register("/MuchFeb", fhMuchFebChanHitRateEvoLong[ uFebIdx ] );
            server->Register("/MuchFeb", fhMuchFebAsicHitRateEvoLong[ uFebIdx ] );
            server->Register("/MuchFeb", fhMuchFebHitRateEvoLong[ uFebIdx ] );
            server->Register("/MuchFeb", fhMuchFebChanDistT[ uFebIdx ] );*/

	 } // if( kTRUE == fUnpackParMuch->IsFebActive( uFebIdx ) )
      } // for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )

      LOG(DEBUG)<< "Initialized FEB  8th Histo" << FairLogger::endl;
      server->RegisterCommand("/Reset_All", "bMcbm2018ResetMuchLite=kTRUE");
      server->RegisterCommand("/Write_All", "bMcbm2018WriteMuchLite=kTRUE");
      server->RegisterCommand("/ScanNoisyCh", "bMcbm2018ScanNoisyMuchLite=kTRUE");
      server->Restrict("/Reset_All", "allow=admin");
      server->Restrict("/Write_All", "allow=admin");
      server->Restrict("/ScanNoisyCh", "allow=admin");
   } // if( server )

   LOG(DEBUG)<< "Initialized All Histos  8th Histo" << FairLogger::endl;
  /** Create summary Canvases for mCBM 2019 **/
   Double_t w = 10;
   Double_t h = 10;
   LOG(DEBUG)<< "Initialized 7th Histo before Summary per FEB" << FairLogger::endl;
      // Summary per FEB
   for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
   {
      if( kTRUE == fUnpackParMuch->IsFebActive( uFebIdx ) )
      {
         TCanvas* cMuchSumm = new TCanvas( Form("cMuchSum_%03u", uFebIdx ),
                                          Form("Summary plots for FEB %03u", uFebIdx ),
                                          w, h);
         cMuchSumm->Divide( 2, 2 );

         cMuchSumm->cd(1);
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogy();
         fhMuchFebChanCntRaw[ uFebIdx ]->Draw();

         cMuchSumm->cd(2);
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogy();
         fhMuchFebChanHitRateProf[ uFebIdx ]->Draw( "e0" );

         cMuchSumm->cd(3);
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();
         fhMuchFebChanAdcRaw[ uFebIdx ]->Draw( "colz" );

         cMuchSumm->cd(4);
         gPad->SetGridx();
         gPad->SetGridy();
	 //      gPad->SetLogy();
         fhMuchFebChanAdcRawProf[ uFebIdx ]->Draw();

         /*cMuchSumm->cd(5);
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetLogz();
         //fhMuchFebChanAdcCal[ uFebIdx ]->Draw( "colz" );

         cMuchSumm->cd(6);
         gPad->SetGridx();
         gPad->SetGridy();
	 //      gPad->SetLogy();
         //fhMuchFebChanAdcCalProf[ uFebIdx ]->Draw();*/

      } // if( kTRUE == fUnpackParMuch->IsFebActive( uFebIdx ) )
   } // for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )


  //====================================================================//
     LOG(DEBUG)<< "Initialized Last Histo before exiting CreateHistograms" << FairLogger::endl;
  //====================================================================//
  /** Recovers/Create Ms Size Canvase for CERN 2016 **/
  // Try to recover canvas in case it was created already by another monitor
  // If not existing, create it
  fcMsSizeAll = dynamic_cast<TCanvas *>( gROOT->FindObject( "cMsSizeAll" ) );
  if( NULL == fcMsSizeAll )
  {
     fcMsSizeAll = new TCanvas("cMsSizeAll", "Evolution of MS size in last 300 s", w, h);
     fcMsSizeAll->Divide( 4, 4 );
      LOG(INFO) << "Created MS size canvas in Much monitor" << FairLogger::endl;
  } // if( NULL == fcMsSizeAll )
      else LOG(INFO) << "Recovered MS size canvas in Much monitor" << FairLogger::endl;
//====================================================================//

  /*****************************/
}

Bool_t CbmMcbm2018MonitorMuchLite::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   if( bMcbm2018ResetMuchLite )
   {
      ResetAllHistos();
      bMcbm2018ResetMuchLite = kFALSE;
   } // if( bMcbm2018ResetMuchLite )
   if( bMcbm2018WriteMuchLite )
   {
      SaveAllHistos( fsHistoFileFullname );
      bMcbm2018WriteMuchLite = kFALSE;
   } // if( bMcbm2018WriteMuchLite )
   if( bMcbm2018ScanNoisyMuchLite )
   {
      ScanForNoisyChannels( );
      bMcbm2018ScanNoisyMuchLite = kFALSE;
   } // if( bMcbm2018WriteMuchLite )

   LOG(DEBUG) << "Timeslice contains " << ts.num_microslices(component)
              << " microslices." << FairLogger::endl;
   fulCurrentTsIdx = ts.index();

   // Ignore overlap ms if flag set by user
   UInt_t uNbMsLoop = fuNbCoreMsPerTs;
   if( kFALSE == fbIgnoreOverlapMs )
      uNbMsLoop += fuNbOverMsPerTs;

   //LOG(INFO) <<" uNbMsLoop "<<uNbMsLoop<<FairLogger::endl;
   // Loop over core microslices (and overlap ones if chosen)
   for( UInt_t uMsIdx = 0; uMsIdx < uNbMsLoop; uMsIdx ++ )
   {
      Double_t dMsTime = (1e-9) * static_cast<double>( ts.descriptor( fvMsComponentsList[ 0 ], uMsIdx ).idx );

      if( 0 == fulCurrentTsIdx && 0 == uMsIdx )
      {
         for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsList.size(); ++uMsCompIdx )
         {
            UInt_t uMsComp = fvMsComponentsList[ uMsCompIdx ];
            auto msDescriptor = ts.descriptor( uMsComp, uMsIdx );
            LOG(INFO) << "hi hv eqid flag si sv idx/start        crc      size     offset"
                      << FairLogger::endl;
            LOG(INFO) << Form( "%02x %02x %04x %04x %02x %02x %016lx %08x %08x %016lx",
                            static_cast<unsigned int>(msDescriptor.hdr_id),
                            static_cast<unsigned int>(msDescriptor.hdr_ver), msDescriptor.eq_id, msDescriptor.flags,
                            static_cast<unsigned int>(msDescriptor.sys_id),
                            static_cast<unsigned int>(msDescriptor.sys_ver), msDescriptor.idx, msDescriptor.crc,
                            msDescriptor.size, msDescriptor.offset )
                      << FairLogger::endl;
            uint32_t uEqId  = static_cast< uint32_t >( msDescriptor.eq_id & 0xFFFF );
            auto it = fDpbIdIndexMap.find( uEqId );
            if( fDpbIdIndexMap.end() == it )
            {
               LOG(WARNING) << "Could not find the sDPB index for AFCK id 0x"
                         << std::hex << uEqId << std::dec
                         << " component " << uMsCompIdx
                         << "\n"
                         << "If valid this index has to be added in the TOF parameter file in the RocIdArray field"
                         << "\n"
                         << "For now we remove it from the list of components analyzed"
                         << FairLogger::endl;
            } // if( fDpbIdIndexMap.end() == it )
         } // for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsList.size(); ++uMsCompIdx )
      } // if( 0 == fulCurrentTsIndex && 0 == uMsIdx )


      // Loop over registered components
      for( UInt_t uMsCompIdx = 0; uMsCompIdx < fvMsComponentsList.size(); ++uMsCompIdx )
      {
         UInt_t uMsComp = fvMsComponentsList[ uMsCompIdx ];

         if( kFALSE == ProcessMuchMs( ts, uMsComp, uMsIdx ) )
            return kFALSE;

      } // for( UInt_t uMsComp = 0; uMsComp < fvMsComponentsList.size(); ++uMsComp )

      /// Pulses time difference calculation and plotting
      // Sort the buffer of hits
      std::sort( fvmHitsInMs.begin(), fvmHitsInMs.end() );

      // Time differences plotting using the fully time sorted hits
      if( 0 < fvmHitsInMs.size() )
      {
         ULong64_t ulLastHitTime = ( *( fvmHitsInMs.rbegin() ) ).GetTs();
         std::vector< stsxyter::FinalHit >::iterator itA;
         std::vector< stsxyter::FinalHit >::iterator itB;

         std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();
         Double_t dUnixTimeInRun = std::chrono::duration_cast< std::chrono::seconds >(tNow - ftStartTimeUnix).count();
         //LOG(INFO) <<" ulLastHitTime "<<ulLastHitTime<<" dUnixTimeInRun "<<dUnixTimeInRun<<FairLogger::endl;
         for( itA  = fvmHitsInMs.begin();
              itA != fvmHitsInMs.end();
//              itA != fvmHitsInMs.end() && (*itA).GetTs() < ulLastHitTime - 320; // 320 * 3.125 ns = 1000 ns
              ++itA )
         {
            UShort_t  usAsicIdx = (*itA).GetAsic();
            UShort_t  usChanIdx = (*itA).GetChan();
            ULong64_t ulHitTs   = (*itA).GetTs();
            UShort_t  usHitAdc  = (*itA).GetAdc();
            UShort_t  usFebIdx    = usAsicIdx / fUnpackParMuch->GetNbAsicsPerFeb();
            UShort_t  usAsicInFeb = usAsicIdx % fUnpackParMuch->GetNbAsicsPerFeb();
	//LOG(INFO) <<" usAsicIdx "<<usAsicIdx<<" usChanIdx "<<usChanIdx<<" ulHitTs "<<ulHitTs<<" usHitAdc "<<usHitAdc<<" usFebIdx "<<usFebIdx<<" usAsicInFeb "<<usAsicInFeb<<FairLogger::endl;
            Double_t dTimeSinceStartSec = (ulHitTs * stsxyter::kdClockCycleNs - fdStartTime)* 1e-9;
            //LOG(INFO) <<" dTimeSinceStartSec "<<dTimeSinceStartSec<<FairLogger::endl;
            fvmAsicHitsInMs[ usAsicIdx ].push_back( (*itA) );
            fvmFebHitsInMs[ usFebIdx ].push_back( (*itA) );
         } // loop on time sorted hits and split per asic/feb

         // Remove all hits which were already used
         fvmHitsInMs.erase( fvmHitsInMs.begin(), itA );
           /// Data in vector are not needed anymore as all possible matches are already checked
         // fvmFebHitsInMs[ uFebIdx ].clear();
         //} // for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
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
   //If Needed store Histos after 10000 TS.
   //if( 0 == ts.index() % 10000 )
      //SaveAllHistos( "data/PeriodicHistosSave.root");

  return kTRUE;
}

Bool_t CbmMcbm2018MonitorMuchLite::ProcessMuchMs( const fles::Timeslice& ts, size_t uMsComp, UInt_t uMsIdx )
{
   auto msDescriptor = ts.descriptor( uMsComp, uMsIdx );
   fuCurrentEquipmentId = msDescriptor.eq_id;
   const uint8_t* msContent = reinterpret_cast<const uint8_t*>( ts.content( uMsComp, uMsIdx ) );

   fulCurrentTsIdx = ts.index();
   if( 0 == fvbMaskedComponents.size() )
      fvbMaskedComponents.resize( ts.num_components(), kFALSE );

   if( 0 == fulCurrentTsIdx && 0 == uMsIdx )
   {
      LOG(INFO) << "hi hv eqid flag si sv idx/start        crc      size     offset"
                << FairLogger::endl;
      LOG(INFO) << Form( "%02x %02x %04x %04x %02x %02x %016lx %08x %08x %016lx",
                      static_cast<unsigned int>(msDescriptor.hdr_id),
                      static_cast<unsigned int>(msDescriptor.hdr_ver), msDescriptor.eq_id, msDescriptor.flags,
                      static_cast<unsigned int>(msDescriptor.sys_id),
                      static_cast<unsigned int>(msDescriptor.sys_ver), msDescriptor.idx, msDescriptor.crc,
                      msDescriptor.size, msDescriptor.offset )
                << FairLogger::endl;
   } // if( 0 == fulCurrentTsIndex && 0 == uMsIdx )
   if( kFALSE == fvbMaskedComponents[ uMsComp ] && 0 == uMsIdx )
   {
      auto it = fDpbIdIndexMap.find( fuCurrentEquipmentId );
      if( fDpbIdIndexMap.end() == it )
      {
         LOG(WARNING) << "Could not find the sDPB index for AFCK id 0x"
                   << std::hex << fuCurrentEquipmentId << std::dec
                   << " component " << uMsComp
                   << "\n"
                   << "If valid this index has to be added in the TOF parameter file in the RocIdArray field"
                   << "\n"
                   << "For now we remove it from the list of components analyzed"
                   << FairLogger::endl;
         fvbMaskedComponents[ uMsComp ] = kTRUE;
      } // if( fDpbIdIndexMap.end() == it )

   } // if( kFALSE == fvbMaskedComponents[ uMsComp ] && 0 == uMsIdx )

   if( kTRUE == fvbMaskedComponents[ uMsComp ] )
      return kTRUE;

   uint32_t uSize  = msDescriptor.size;
   fulCurrentMsIdx = msDescriptor.idx;
   Double_t dMsTime = (1e-9) * static_cast<double>(fulCurrentMsIdx);
   LOG(DEBUG) << "Microslice: " << fulCurrentMsIdx
              << " from EqId " << std::hex << fuCurrentEquipmentId << std::dec
              << " has size: " << uSize << FairLogger::endl;

   fuCurrDpbId  = static_cast< uint32_t >( fuCurrentEquipmentId & 0xFFFF );
   fuCurrDpbIdx = fDpbIdIndexMap[ fuCurrDpbId ];
   //LOG(INFO) <<" fuCurrDpbIdx "<<fuCurrDpbIdx<<" fuCurrDpbId "<<fuCurrDpbId<<FairLogger::endl;

   if( uMsComp < kiMaxNbFlibLinks )
   {
      if( fdStartTimeMsSz < 0 )
         fdStartTimeMsSz = dMsTime;
      fhMsSz[ uMsComp ]->Fill( uSize );
      fhMsSzTime[ uMsComp ]->Fill( dMsTime - fdStartTimeMsSz, uSize);
   } // if( uMsComp < kiMaxNbFlibLinks )

   /// Plots in [X/s] update
   if( static_cast<Int_t>( fvdPrevMsTime[ uMsComp ] ) < static_cast<Int_t>( dMsTime )  )
   {
      /// "new second"
      UInt_t uFebIdxOffset = fUnpackParMuch->GetNbFebsPerDpb() * fuCurrDpbIdx;
      for( UInt_t uFebIdx = 0; uFebIdx < fUnpackParMuch->GetNbFebsPerDpb(); ++uFebIdx )
      {
         UInt_t uFebIdxInSyst = uFebIdxOffset + uFebIdx;

         /// Ignore first interval is not clue how late the data taking was started
         if( 0 == fviFebTimeSecLastRateUpdate[uFebIdxInSyst] )
         {
            fviFebTimeSecLastRateUpdate[uFebIdxInSyst] = static_cast<Int_t>( dMsTime );
            fviFebCountsSinceLastRateUpdate[uFebIdxInSyst] = 0;
            for( UInt_t uChan = 0; uChan < fUnpackParMuch->GetNbChanPerFeb(); ++uChan )
               fvdFebChanCountsSinceLastRateUpdate[uFebIdxInSyst][uChan] = 0.0;
            continue;
         } // if( 0 == fviFebTimeSecLastRateUpdate[uFebIdxInSyst] )

         Int_t iTimeInt = static_cast<Int_t>( dMsTime ) - fviFebTimeSecLastRateUpdate[uFebIdxInSyst];
         if( fiTimeIntervalRateUpdate <= iTimeInt )
         {
            /// Jump empty FEBs without looping over channels
            if( 0 == fviFebCountsSinceLastRateUpdate[uFebIdxInSyst] )
            {
               fviFebTimeSecLastRateUpdate[uFebIdxInSyst] = static_cast<Int_t>( dMsTime );
               continue;
            } // if( 0 == fviFebCountsSinceLastRateUpdate[uFebIdxInSyst] )

            for( UInt_t uChan = 0; uChan < fUnpackParMuch->GetNbChanPerFeb(); ++uChan )
            {
               fhMuchFebChanHitRateProf[uFebIdxInSyst]->Fill( uChan,
                                                       fvdFebChanCountsSinceLastRateUpdate[uFebIdxInSyst][uChan] / iTimeInt );
               fvdFebChanCountsSinceLastRateUpdate[uFebIdxInSyst][uChan] = 0.0;
            } // for( UInt_t uChan = 0; uChan < fUnpackParMuch->GetNbChanPerFeb(); ++uChan )

            fviFebTimeSecLastRateUpdate[uFebIdxInSyst] = static_cast<Int_t>( dMsTime );
            fviFebCountsSinceLastRateUpdate[uFebIdxInSyst] = 0;
         } // if( fiTimeIntervalRateUpdate <= iTimeInt )
      } // for( UInt_t uFebIdx = 0; uFebIdx < fUnpackParMuch->GetNbFebsPerDpb(); ++uFebIdx )
   } // if( static_cast<Int_t>( fvdMsTime[ uMsCompIdx ] ) < static_cast<Int_t>( dMsTime )  )

   // Store MS time for coincidence plots
   fvdPrevMsTime[ uMsComp ] = dMsTime;

   /// Check Flags field of MS header
   uint16_t uMsHeaderFlags = msDescriptor.flags;
   for( UInt_t uBit = 0; uBit < 16; ++uBit )
      fhMsStatusFieldType->Fill( uBit, ( uMsHeaderFlags >> uBit ) & 0x1 );

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
   {/*
      LOG(WARNING) << "TS MSB cycle from MS header does not match current cycle from data "
                    << "for TS " << std::setw( 12 ) << fulCurrentTsIdx
                    << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                    << " MsInTs " << std::setw( 3 ) << uMsIdx
                    << " ====> " << fvuCurrentTsMsbCycle[ fuCurrDpbIdx ]
                    << " VS " << uTsMsbCycleHeader
                    << FairLogger::endl;*/
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
/*
      if( 1000 == fulCurrentTsIdx )
      {
         mess.PrintMess( std::cout, fPrintMessCtrl );
      } // if( 0 == fulCurrentTsIdx )
*/
      stsxyter::MessType typeMess = mess.GetMessType();
      fmMsgCounter[ typeMess ] ++;
      fhMuchMessType->Fill( static_cast< uint16_t > (typeMess) );
      fhMuchMessTypePerDpb->Fill( fuCurrDpbIdx, static_cast< uint16_t > (typeMess) );

      switch( typeMess )
      {
         case stsxyter::MessType::Hit :
         {
            // Extract the eLink and Asic indices => Should GO IN the fill method now that obly hits are link/asic specific!
            UShort_t usElinkIdx = mess.GetLinkIndex();
            UInt_t   uCrobIdx   = usElinkIdx / fUnpackParMuch->GetNbElinkPerCrob();
            Int_t   uFebIdx    = fUnpackParMuch->ElinkIdxToFebIdx( usElinkIdx );
//            if(usElinkIdx!=0)
			//LOG(INFO) <<" usElinkIdx "<<usElinkIdx<<" uCrobIdx "<<uCrobIdx<<" uFebIdx "<<uFebIdx<<FairLogger::endl;
            if( kTRUE == fbMuchMode )
               uFebIdx = usElinkIdx;
            fhMuchHitsElinkPerDpb->Fill( fuCurrDpbIdx, usElinkIdx );
            if( -1 == uFebIdx )
            {
               LOG(WARNING) << "CbmMcbm2018MonitorMuchLite::DoUnpack => "
                         << "Wrong elink Idx! Elink raw "
                         << Form("%d remap %d", usElinkIdx, uFebIdx )
                         << FairLogger::endl;
               continue;
            } // if( -1 == uFebIdx )
            //LOG(INFO) << " uCrobIdx "<<uCrobIdx<<" fUnpackParMuch->ElinkIdxToAsicIdx( usElinkIdx ) "<<fUnpackParMuch->ElinkIdxToAsicIdx( usElinkIdx )<<" usElinkIdx "<<usElinkIdx<<FairLogger::endl;
            UInt_t   uAsicIdx   = ( fuCurrDpbIdx * fUnpackParMuch->GetNbCrobsPerDpb() + uCrobIdx
                                  ) * fUnpackParMuch->GetNbAsicsPerCrob()
                                 + fUnpackParMuch->ElinkIdxToAsicIdx( usElinkIdx );

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
               LOG(INFO) << "CbmMcbm2018MonitorMuchLite::DoUnpack => "
                         << "EPOCH message at unexpected position in MS: message "
                         << uIdx << " VS message 0 expected!"
                         << FairLogger::endl;
            break;
         } // case stsxyter::MessType::TsMsb :
         case stsxyter::MessType::Status :
         {
            UShort_t usElinkIdx    = mess.GetStatusLink();
            UInt_t   uCrobIdx   = usElinkIdx / fUnpackParMuch->GetNbElinkPerCrob();
            Int_t   uFebIdx    = fUnpackParMuch->ElinkIdxToFebIdx( usElinkIdx );
            UInt_t   uAsicIdx   = ( fuCurrDpbIdx * fUnpackParMuch->GetNbCrobsPerDpb() + uCrobIdx
                                  ) * fUnpackParMuch->GetNbAsicsPerCrob()
                                 + fUnpackParMuch->ElinkIdxToAsicIdx( usElinkIdx );

            UShort_t usStatusField = mess.GetStatusStatus();

            fhStatusMessType->Fill( uAsicIdx, usStatusField );
            /// Always print status messages... or not?
            if( fbPrintMessages )
            {
               std::cout << Form("DPB %2u TS %12u MS %12u mess %5u ", fuCurrDpbIdx, fulCurrentTsIdx, fulCurrentMsIdx, uIdx );
               mess.PrintMess( std::cout, fPrintMessCtrl );
            } // if( fbPrintMessages )
//                   FillTsMsbInfo( mess );
            break;
         } // case stsxyter::MessType::Status
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
            LOG(FATAL) << "CbmMcbm2018MonitorMuchLite::DoUnpack => "
                       << "Unknown message type, should never happen, stopping here! Type found was: "
                       << static_cast< int >( typeMess )
                       << FairLogger::endl;
         }
      } // switch( mess.GetMessType() )
   } // for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )

  return kTRUE;
}


void CbmMcbm2018MonitorMuchLite::FillHitInfo( stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx )
{
   UShort_t usChan   = mess.GetHitChannel();
   UShort_t usRawAdc = mess.GetHitAdc();
//   UShort_t usFullTs = mess.GetHitTimeFull();
   UShort_t usTsOver = mess.GetHitTimeOver();
   UShort_t usRawTs  = mess.GetHitTime();
   //Below FebID is according to FEB Position in Module GEM A or Module GEM B (Carefully write MUCH Par file)
   Int_t FebId = fUnpackParMuch->GetFebId(uAsicIdx);

//   UInt_t uFebIdx    = uAsicIdx / fUnpackParMuch->GetNbAsicsPerFeb();
	//For MUCH each FEB has one StsXyter
   UInt_t uFebIdx    = uAsicIdx;
   UInt_t uCrobIdx   = usElinkIdx / fUnpackParMuch->GetNbElinkPerCrob();
   UInt_t uAsicInFeb = uAsicIdx % fUnpackParMuch->GetNbAsicsPerFeb();
   UInt_t uChanInFeb = usChan + fUnpackParMuch->GetNbChanPerAsic() * (uAsicIdx % fUnpackParMuch->GetNbAsicsPerFeb());
   Int_t sector  = fUnpackParMuch->GetPadX(FebId, usChan);
   Int_t channel = fUnpackParMuch->GetPadY(FebId, usChan);

   //Convert into Real X Y Position
//   Double_t ActualX = fUnpackParMuch->GetRealX(channel+97*sector);
//   Double_t ActualY = fUnpackParMuch->GetRealPadSize(channel+97*sector);
   Double_t ActualX = fUnpackParMuch->GetRealX( channel, sector );
   Double_t ActualY = fUnpackParMuch->GetRealPadSize( channel, sector );

   //Converting Module (Small side up)
	ActualX = 1000-ActualX;
        channel = 96 - channel;

   Int_t ModuleNr = fUnpackParMuch->GetModule(uAsicIdx);

   LOG(DEBUG)<< "Module Nr " << ModuleNr << " Sector Nr "<< sector <<" Channel Nr "<<channel << "Actual X "<< ActualX << "Actual Y " <<ActualY << "uAsicIdx "<<uAsicIdx << FairLogger::endl;


   fHistPadDistr[ModuleNr]->Fill(sector, channel);
   fRealHistPadDistr[ModuleNr]->Fill(ActualY, ActualX);

   //Double_t dCalAdc = fvdFebAdcOffs[ fuCurrDpbIdx ][ uCrobIdx ][ uFebIdx ]
   //                  + (usRawAdc - 1)* fvdFebAdcGain[ fuCurrDpbIdx ][ uCrobIdx ][ uFebIdx ];

   fhMuchFebChanCntRaw[  uFebIdx ]->Fill( uChanInFeb );
   fhMuchFebChanAdcRaw[  uFebIdx ]->Fill( uChanInFeb, usRawAdc );
   fhMuchFebChanAdcRawProf[  uFebIdx ]->Fill( uChanInFeb, usRawAdc );
   //fhMuchFebChanAdcCal[  uFebIdx ]->Fill(     uChanInFeb, dCalAdc );
   //fhMuchFebChanAdcCalProf[  uFebIdx ]->Fill( uChanInFeb, dCalAdc );
   fhMuchFebChanRawTs[   uFebIdx ]->Fill( usChan, usRawTs );
   //fhMuchFebChanMissEvt[ uFebIdx ]->Fill( usChan, mess.IsHitMissedEvts() );
   fhMuchFebChanAdcRaw_combined->Fill(usRawAdc);

   // Compute the Full time stamp
   Long64_t ulOldHitTime = fvulChanLastHitTime[ uAsicIdx ][ usChan ];
   Long64_t dOldHitTime  = fvdChanLastHitTime[ uAsicIdx ][ usChan ];

      // Use TS w/o overlap bits as they will anyway come from the TS_MSB
   fvulChanLastHitTime[ uAsicIdx ][ usChan ] = usRawTs;

   fvulChanLastHitTime[ uAsicIdx ][ usChan ] +=
               static_cast<ULong64_t>( stsxyter::kuHitNbTsBins )
             * static_cast<ULong64_t>( fvulCurrentTsMsb[fuCurrDpbIdx])
             + static_cast<ULong64_t>( stsxyter::kuTsCycleNbBins )
             * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx] )
             ;


   // Convert the Hit time in bins to Hit time in ns
   Long64_t dHitTimeNs = fvulChanLastHitTime[ uAsicIdx ][ usChan ] * stsxyter::kdClockCycleNs;

   // Store new value of Hit time in ns
   fvdChanLastHitTime[ uAsicIdx ][ usChan ] = fvulChanLastHitTime[ uAsicIdx ][ usChan ] * stsxyter::kdClockCycleNs;
   // For StsXyter2.0 Duplicate Hit Error
   //Int_t ModuleNr = fUnpackParMuch->GetModule(uAsicIdx);
   fhMuchFebDuplicateHitProf[ModuleNr]->Fill(FebId,0);
   if( ulOldHitTime == fvulChanLastHitTime[uAsicIdx][usChan] )
  		     fhMuchFebDuplicateHitProf[ModuleNr]->Fill(FebId,1);


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

   // Check Starting point of histos with time as X axis
   if( -1 == fdStartTime )
      fdStartTime = fvdChanLastHitTime[ uAsicIdx ][ usChan ];

	Int_t constime = (fvdChanLastHitTime[ uAsicIdx ][ usChan ] - prevtime_new) ;

	if(constime<10000000)
	{
		if(usRawAdc>1)
		{
			Counter1++;
		}
		Counter++;
	}
	else
	{
		fhRate->Fill(Counter);
                fhRateAdcCut->Fill(Counter1);
		Counter=0;
                Counter1=0;
	        prevtime_new = fvdChanLastHitTime[ uAsicIdx ][ usChan ];
	}


   // Fill histos with time as X axis
   Double_t dTimeSinceStartSec = (fvdChanLastHitTime[ uAsicIdx ][ usChan ] - fdStartTime)* 1e-9;    //uTimeBin
   Double_t dTimeSinceStartMin = dTimeSinceStartSec / 60.0;

   fviFebCountsSinceLastRateUpdate[uFebIdx]++;
   fvdFebChanCountsSinceLastRateUpdate[uFebIdx][uChanInFeb] += 1;

   fhMuchFebChanHitRateEvo[ uFebIdx ]->Fill( dTimeSinceStartSec, uChanInFeb );
   //fhMuchFebAsicHitRateEvo[ uFebIdx ]->Fill( dTimeSinceStartSec, uAsicInFeb );
   fhMuchFebHitRateEvo[ uFebIdx ]->Fill( dTimeSinceStartSec );
   fhMuchFebHitRateEvo_mskch[ uFebIdx ]->Fill( dTimeSinceStartSec );
   if(usRawAdc>1)fhMuchFebHitRateEvo_mskch_adccut[ uFebIdx ]->Fill(     dTimeSinceStartSec );
   //fhMuchFebChanHitRateEvoLong[ uFebIdx ]->Fill( dTimeSinceStartMin, uChanInFeb, 1.0/60.0 );
   //fhMuchFebAsicHitRateEvoLong[ uFebIdx ]->Fill( dTimeSinceStartMin, uAsicInFeb,   1.0/60.0 );
   //fhMuchFebHitRateEvoLong[ uFebIdx ]->Fill(     dTimeSinceStartMin,             1.0/60.0 );

   /*
   if( mess.IsHitMissedEvts() )
   {
      fhMuchFebChanMissEvtEvo[ uFebIdx ]->Fill( dTimeSinceStartSec, uChanInFeb );
      fhMuchFebAsicMissEvtEvo[ uFebIdx ]->Fill( dTimeSinceStartSec, uAsicInFeb );
      fhMuchFebMissEvtEvo[ uFebIdx ]->Fill(     dTimeSinceStartSec );
   } // if( mess.IsHitMissedEvts() )
   //if(fvdChanLastHitTime[ uAsicIdx ][ usChan ] == prevTime && uAsicIdx == prevAsic && usChan == prevChan)
   */
   if(fvdChanLastHitTime[ uAsicIdx ][ usChan ] == prevTime && usChan == prevChan)
   {
      //fDupliCount++;
   }
   else
   {
     fhMuchFebHitRateEvo_WithoutDupli[ uFebIdx ]->Fill( dTimeSinceStartSec );
   }
   prevTime = fvdChanLastHitTime[ uAsicIdx ][ usChan ];
   prevChan = usChan;
   prevAsic = uAsicIdx;
}

void CbmMcbm2018MonitorMuchLite::FillTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx, UInt_t uMsIdx )
{
   UInt_t uVal    = mess.GetTsMsbVal();
   // Update Status counters
   if( uVal < fvulCurrentTsMsb[fuCurrDpbIdx] )
   {

      fvuCurrentTsMsbCycle[fuCurrDpbIdx] ++;
   } // if( uVal < fvulCurrentTsMsb[fuCurrDpbIdx] )
   fvulCurrentTsMsb[fuCurrDpbIdx] = uVal;

   ULong64_t ulNewTsMsbTime =  static_cast< ULong64_t >( stsxyter::kuHitNbTsBins )
                             * static_cast< ULong64_t >( fvulCurrentTsMsb[fuCurrDpbIdx])
                             + static_cast< ULong64_t >( stsxyter::kuTsCycleNbBins )
                             * static_cast< ULong64_t >( fvuCurrentTsMsbCycle[fuCurrDpbIdx] );
}

void CbmMcbm2018MonitorMuchLite::FillEpochInfo( stsxyter::Message mess )
{
   UInt_t uVal    = mess.GetEpochVal();
   UInt_t uCurrentCycle = uVal % stsxyter::kuTsCycleNbBins;
}

void CbmMcbm2018MonitorMuchLite::Reset()
{
}

void CbmMcbm2018MonitorMuchLite::Finish()
{

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MonitorMuchLite statistics are " << FairLogger::endl;
   LOG(INFO) << " Hit      messages: " << fmMsgCounter[ stsxyter::MessType::Hit   ] << FairLogger::endl
             << " Ts MSB   messages: " << fmMsgCounter[ stsxyter::MessType::TsMsb ] << FairLogger::endl
             << " Dummy    messages: " << fmMsgCounter[ stsxyter::MessType::Dummy ] << FairLogger::endl
             << " Epoch    messages: " << fmMsgCounter[ stsxyter::MessType::Epoch ] << FairLogger::endl
             << " Empty    messages: " << fmMsgCounter[ stsxyter::MessType::Empty ] << FairLogger::endl;

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;

   SaveAllHistos( fsHistoFileFullname );
   //SaveAllHistos();

}


void CbmMcbm2018MonitorMuchLite::FillOutput(CbmDigi* /*digi*/)
{
}

void CbmMcbm2018MonitorMuchLite::SaveAllHistos( TString sFileName )
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
   gDirectory->mkdir("Much_Raw");
   gDirectory->cd("Much_Raw");

   for( UInt_t uModuleId = 0; uModuleId < 2; ++uModuleId )
   {
	   fHistPadDistr[uModuleId]->Write();
	   fRealHistPadDistr[uModuleId]->Write();
	   fhMuchFebDuplicateHitProf[uModuleId]->Write();
   }
   fhRate->Write();
   fhRateAdcCut->Write();
   fhMuchMessType->Write();
   fhMuchSysMessType->Write();
   fhMuchMessTypePerDpb->Write();
   fhMuchSysMessTypePerDpb->Write();
   fhStatusMessType->Write();
   fhMsStatusFieldType->Write();
   fhMuchHitsElinkPerDpb->Write();
   fhMuchFebChanAdcRaw_combined->Write();
   gDirectory->cd("..");
   /***************************/

   /***************************/
   gDirectory->mkdir("Much_Feb");
   gDirectory->cd("Much_Feb");
   for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
   {
      if( kTRUE == fUnpackParMuch->IsFebActive( uFebIdx ) )
      {
         fhMuchFebChanCntRaw[ uFebIdx ]->Write();
         //fhMuchFebChanCntRawGood[ uFebIdx ]->Write();
         fhMuchFebChanAdcRaw[ uFebIdx ]->Write();
         fhMuchFebChanAdcRawProf[ uFebIdx ]->Write();
         //fhMuchFebChanAdcCal[ uFebIdx ]->Write();
         //fhMuchFebChanAdcCalProf[ uFebIdx ]->Write();
         fhMuchFebChanRawTs[ uFebIdx ]->Write();
         fhMuchFebChanHitRateProf[ uFebIdx ]->Write();
         //fhMuchFebAsicHitRateEvo[ uFebIdx ]->Write();
         fhMuchFebHitRateEvo[ uFebIdx ]->Write();
         fhMuchFebHitRateEvo_mskch[ uFebIdx ]->Write();
         fhMuchFebHitRateEvo_mskch_adccut[ uFebIdx ]->Write();
         fhMuchFebHitRateEvo_WithoutDupli[ uFebIdx ]->Write();
         /*fhMuchFebChanHitRateEvoLong[ uFebIdx ]->Write();
         fhMuchFebAsicHitRateEvoLong[ uFebIdx ]->Write();
         fhMuchFebHitRateEvoLong[ uFebIdx ]->Write();
         fhMuchFebChanDistT[ uFebIdx ]->Write();
         for( UInt_t uFebIdxB = uFebIdx; uFebIdxB < fuNbFebs; ++uFebIdxB )
         {
            fhMuchFebChanDtCoinc[ uFebIdx ][ uFebIdxB ]->Write();
            fhMuchFebChanCoinc[ uFebIdx ][ uFebIdxB ]->Write();
         } // for( UInt_t uFebIdxB = uFebIdx; uFebIdxB < fuNbFebs; ++uFebIdxB )*/
      } // if( kTRUE == fUnpackParMuch->IsFebActive( uFebIdx ) )
   } // for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
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

   /***************************/

   if( "" != sFileName )
   {
      // Restore original directory position
      histoFile->Close();
      oldDir->cd();
   } // if( "" != sFileName )

}

void CbmMcbm2018MonitorMuchLite::ResetAllHistos()
{
   LOG(INFO) << "Reseting all Much histograms." << FairLogger::endl;

   for( UInt_t uModuleId = 0; uModuleId < 2; ++uModuleId )
   {
   fHistPadDistr[uModuleId]->Reset();
   fRealHistPadDistr[uModuleId]->Reset();
   fhMuchFebDuplicateHitProf[uModuleId]->Reset();
   }
   fhRate->Reset();
   fhRateAdcCut->Reset();
   fhMuchMessType->Reset();
   fhMuchSysMessType->Reset();
   fhMuchMessTypePerDpb->Reset();
   fhMuchSysMessTypePerDpb->Reset();
   fhStatusMessType->Reset();
   fhMsStatusFieldType->Reset();
   fhMuchHitsElinkPerDpb->Reset();
   fhMuchFebChanAdcRaw_combined->Reset();

   for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
   {
      if( kTRUE == fUnpackParMuch->IsFebActive( uFebIdx ) )
      {
         fhMuchFebChanCntRaw[ uFebIdx ]->Reset();
         //fhMuchFebChanCntRawGood[ uFebIdx ]->Reset();
         fhMuchFebChanAdcRaw[ uFebIdx ]->Reset();
         fhMuchFebChanAdcRawProf[ uFebIdx ]->Reset();
         //fhMuchFebChanAdcCal[ uFebIdx ]->Reset();
         //fhMuchFebChanAdcCalProf[ uFebIdx ]->Reset();
         fhMuchFebChanRawTs[ uFebIdx ]->Reset();
         fhMuchFebChanHitRateEvo[ uFebIdx ]->Reset();
         fhMuchFebChanHitRateProf[ uFebIdx ]->Reset();
         //fhMuchFebAsicHitRateEvo[ uFebIdx ]->Reset();
         fhMuchFebHitRateEvo[ uFebIdx ]->Reset();
         fhMuchFebHitRateEvo_mskch[ uFebIdx ]->Reset();
         fhMuchFebHitRateEvo_mskch_adccut[ uFebIdx ]->Reset();
         fhMuchFebHitRateEvo_WithoutDupli[ uFebIdx ]->Reset();
         /*fhMuchFebChanHitRateEvoLong[ uFebIdx ]->Reset();
         fhMuchFebAsicHitRateEvoLong[ uFebIdx ]->Reset();
         fhMuchFebHitRateEvoLong[ uFebIdx ]->Reset();
         fhMuchFebChanDistT[ uFebIdx ]->Reset();
         for( UInt_t uFebIdxB = uFebIdx; uFebIdxB < fuNbFebs; ++uFebIdxB )
         {
            fhMuchFebChanDtCoinc[ uFebIdx ][ uFebIdxB ]->Reset();
            fhMuchFebChanCoinc[ uFebIdx ][ uFebIdxB ]->Reset();
         } // for( UInt_t uFebIdxB = uFebIdx; uFebIdxB < fuNbFebs; ++uFebIdxB )*/
      } // if( kTRUE == fUnpackParMuch->IsFebActive( uFebIdx ) )
   } // for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )

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

void CbmMcbm2018MonitorMuchLite::SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize )
{
   TDatime * fRunStartDateTime     = new TDatime( dateIn, timeIn);
   fiRunStartDateTimeSec = fRunStartDateTime->Convert();
   fiBinSizeDatePlots    = iBinSize;

   LOG(INFO) << "Assigned new MUCH Run Start Date-Time: " << fRunStartDateTime->AsString() << FairLogger::endl;
}

///------------------------------------------------------------------///
Bool_t CbmMcbm2018MonitorMuchLite::ScanForNoisyChannels( Double_t dNoiseThreshold )
{
   for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
   {
      if( kTRUE == fUnpackParMuch->IsFebActive( uFebIdx ) )
      {
         LOG(INFO) << Form( " ------------------ Noisy channels scan for FEB %2d ------------", uFebIdx )
                   << FairLogger::endl;
         for( UInt_t uAsicIdx = 0; uAsicIdx < fUnpackParMuch->GetNbAsicsPerFeb(); ++uAsicIdx )
            for( UInt_t uChanIdx = 0; uChanIdx < fUnpackParMuch->GetNbChanPerAsic(); ++uChanIdx )
            {
               UInt_t uChanInFeb = uAsicIdx * fUnpackParMuch->GetNbChanPerAsic() + uChanIdx;
               if( dNoiseThreshold < fhMuchFebChanHitRateProf[ uFebIdx ]->GetBinContent( 1 + uChanInFeb ) )
                  LOG(INFO) << Form( "Noisy Channel ASIC %d channel %3d (%4d) level %6.0f",
                                     uAsicIdx, uChanIdx, uChanInFeb,
                                     fhMuchFebChanHitRateProf[ uFebIdx ]->GetBinContent( 1 + uChanInFeb ) )
                            << FairLogger::endl;

            } // for( UInt_t uChanIdx = 0; uChanIdx < fUnpackParMuch->GetNbChanPerAsic(); ++uChanIdx )

         LOG(INFO) << " ---------------------------------------------------------------"
                   << FairLogger::endl;
      } // if( kTRUE == fUnpackParMuch->IsFebActive( uFebIdx ) )
   } // for( UInt_t uFebIdx = 0; uFebIdx < fuNbFebs; ++uFebIdx )
   return kTRUE;
}
///------------------------------------------------------------------///

ClassImp(CbmMcbm2018MonitorMuchLite)
