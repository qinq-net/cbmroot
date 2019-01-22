// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmStar2019EventBuilderEtof                   -----
// -----               Created 14.11.2018 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmStar2019EventBuilderEtof.h"

#include "CbmStar2019EventBuilderEtofAlgo.h"
#include "CbmStar2019TofPar.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRunOnline.h"
#include "FairParGenericSet.h"

#include "THttpServer.h"
#include "TROOT.h"
#include "TString.h"

#include <iostream>
#include <stdint.h>
#include <iomanip>
#include <fstream>
#include <chrono>

Bool_t bStarEtof2019EventBuilderResetHistos = kFALSE;

CbmStar2019EventBuilderEtof::CbmStar2019EventBuilderEtof( UInt_t uNbGdpb )
  : CbmMcbmUnpack(),
    fbMonitorMode( kFALSE ),
    fbSandboxMode( kFALSE ),
    fbEventDumpEna( kFALSE ),
    fParCList( nullptr ),
    fulTsCounter( 0 ),
    fEventBuilderAlgo( nullptr ),
    fpBinDumpFile( nullptr )
{
   fEventBuilderAlgo = new CbmStar2019EventBuilderEtofAlgo();
}

CbmStar2019EventBuilderEtof::~CbmStar2019EventBuilderEtof()
{
   delete fEventBuilderAlgo;
}

Bool_t CbmStar2019EventBuilderEtof::Init()
{
   LOG(INFO) << "CbmStar2019EventBuilderEtof::Init" << FairLogger::endl;
   LOG(INFO) << "Initializing STAR eTOF 2018 Event Builder" << FairLogger::endl;

   FairRootManager* ioman = FairRootManager::Instance();
   if( NULL == ioman )
   {
      LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
   }

   return kTRUE;
}

void CbmStar2019EventBuilderEtof::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
             << FairLogger::endl;

   fParCList = fEventBuilderAlgo->GetParList();

   for( Int_t iparC = 0; iparC < fParCList->GetEntries(); ++iparC )
   {
      FairParGenericSet* tempObj = (FairParGenericSet*)(fParCList->At(iparC));
      fParCList->Remove(tempObj);

      std::string sParamName{ tempObj->GetName() };
      FairParGenericSet* newObj = dynamic_cast<FairParGenericSet*>( FairRun::Instance()->GetRuntimeDb()->getContainer( sParamName.data() ) );

      if( nullptr == newObj )
      {
         LOG(ERROR) << "Failed to obtain parameter container " << sParamName
                    << ", for parameter index " << iparC
                    << FairLogger::endl;
         return;
      } // if( nullptr == newObj )

      fParCList->AddAt(newObj, iparC);
//      delete tempObj;
   } // for( Int_t iparC = 0; iparC < fParCList->GetEntries(); ++iparC )

}

Bool_t CbmStar2019EventBuilderEtof::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
               << FairLogger::endl;

   /// Control flags
   CbmStar2019TofPar * pUnpackPar = dynamic_cast<CbmStar2019TofPar*>( FairRun::Instance()->GetRuntimeDb()->getContainer( "CbmStar2019TofPar" ) );
   if( nullptr == pUnpackPar )
   {
      LOG(ERROR) << "Failed to obtain parameter container CbmStar2019TofPar"
                 << FairLogger::endl;
      return kFALSE;
   } // if( nullptr == pUnpackPar )

   fbMonitorMode = pUnpackPar->GetMonitorMode();
   LOG(INFO) << "Monitor mode:       "
             << ( fbMonitorMode ? "ON" : "OFF" )
             << FairLogger::endl;

   fbDebugMonitorMode = pUnpackPar->GetDebugMonitorMode();
   LOG(INFO) << "Debug Monitor mode: "
             << ( fbDebugMonitorMode ? "ON" : "OFF" )
             << FairLogger::endl;

   Bool_t initOK = fEventBuilderAlgo->InitContainers();

   /// If monitor mode enabled, trigger histos creation, obtain pointer on them and add them to the HTTP server
   if( kTRUE == fbMonitorMode )
   {
      /// Trigger histo creation on all associated algos
      initOK &= fEventBuilderAlgo->CreateHistograms();

      /// Obtain vector of pointers on each histo from the algo (+ optionally desired folder)
      std::vector< std::pair< TNamed *, std::string > > vHistos = fEventBuilderAlgo->GetHistoVector();

      /// Register the histos in the HTTP server
      THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
      for( UInt_t uHisto = 0; uHisto < vHistos.size(); ++uHisto )
      {
         server->Register( Form( "/%s", vHistos[ uHisto ].second.data() ), vHistos[ uHisto ].first );
      } // for( UInt_t uHisto = 0; uHisto < vHistos.size(); ++uHisto )

      server->RegisterCommand("/Reset_EvtBuild_Hist", "bStarEtof2019EventBuilderResetHistos=kTRUE");
      server->Restrict("/Reset_EvtBuild_Hist", "allow=admin");

   } // if( kTRUE == fbMonitorMode )

   return initOK;
}

Bool_t CbmStar2019EventBuilderEtof::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
             << FairLogger::endl;
   Bool_t initOK = fEventBuilderAlgo->ReInitContainers();

   return initOK;
}

void CbmStar2019EventBuilderEtof::SetEventDumpEnable( Bool_t bDumpEna )
{
   if( fbEventDumpEna != bDumpEna )
   {
      if( bDumpEna )
      {
         LOG(INFO) << "Enabling event dump to binary file which was disabled. File will be opened."
                   << FairLogger::endl;

         std::time_t cTimeCurrent = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
         char tempBuff[80];
         std::strftime( tempBuff, 80, "%Y_%m_%d_%H_%M_%S", localtime (&cTimeCurrent) );
         TString sFileName = Form("event_dump_%s.bin", tempBuff);
         fpBinDumpFile = new std::fstream( sFileName, std::ios::out | std::ios::binary);

         if( NULL == fpBinDumpFile )
         {
            LOG(FATAL) << "Failed to open new binary file for event dump at "
                       << sFileName
                       << FairLogger::endl;
         } // if( NULL == fpBinDumpFile )
            else LOG(INFO) << "Opened binary dump file at "
                           << sFileName
                           << FairLogger::endl;
      } // if( bDumpEna )
         else
         {
            LOG(INFO) << "Disabling event dump to binary file which was enabled. File will be closed."
                      << FairLogger::endl;

            if( NULL != fpBinDumpFile )
               fpBinDumpFile->close();
         } // else of if( bDumpEna )
   } // if( fbEventDumpEna != bDumpEna )

   fbEventDumpEna = bDumpEna;
   if( fbEventDumpEna )
      LOG(INFO) << "Event dump to binary file is now ENABLED"
                << FairLogger::endl;
      else LOG(INFO) << "Event dump to binary file is now DISABLED"
                     << FairLogger::endl;
}

void CbmStar2019EventBuilderEtof::AddMsComponentToList( size_t component, UShort_t usDetectorId )
{
   fEventBuilderAlgo->AddMsComponentToList( component, usDetectorId );
}

static Double_t dctime=0.;

Bool_t CbmStar2019EventBuilderEtof::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   fTimer.Start();

   if( 0 == fulTsCounter )
   {
      LOG(INFO) << "FIXME ===> Jumping 1st TS as corrupted with current FW + FLESNET combination"
                << FairLogger::endl;
      fulTsCounter++;
      return kTRUE;
   } // if( 0 == fulTsCounter )

   if( fbMonitorMode && bStarEtof2019EventBuilderResetHistos )
   {
      LOG(INFO) << "Reset eTOF STAR histos " << FairLogger::endl;
      fEventBuilderAlgo->ResetHistograms();
      bStarEtof2019EventBuilderResetHistos = kFALSE;
   } // if( fbMonitorMode && bStarEtof2019EventBuilderResetHistos )

   if( kFALSE == fEventBuilderAlgo->ProcessTs( ts ) )
   {
      LOG(ERROR) << "Failed processing TS " << ts.index()
                 << " in event builder algorithm class"
                 << FairLogger::endl;
      return kTRUE;
   } // if( kFALSE == fEventBuilderAlgo->ProcessTs( ts ) )

   std::vector< CbmTofStarSubevent2019 > & eventBuffer = fEventBuilderAlgo->GetEventBuffer();

   for( UInt_t uEvent = 0; uEvent < eventBuffer.size(); ++uEvent )
   {
      /// Send the sub-event to the STAR systems
      Int_t  iBuffSzByte = 0;
      void * pDataBuff = eventBuffer[ uEvent ].BuildOutput( iBuffSzByte );
      if( NULL != pDataBuff )
      {
         /// Valid output, do stuff with it!
         Bool_t fbSendEventToStar = kFALSE;
         if( kFALSE == fbSandboxMode )
         {
            /*
             ** Function to send sub-event block to the STAR DAQ system
             *       trg_word received is packed as:
             *
             *       trg_cmd|daq_cmd|tkn_hi|tkn_mid|tkn_lo
             */
            star_rhicf_write( eventBuffer[ uEvent ].GetTrigger().GetStarTrigerWord(),
                              pDataBuff, iBuffSzByte );
         } // if( kFALSE == fbSandboxMode )

         LOG(DEBUG) << "Sent STAR event with size " << iBuffSzByte << " Bytes"
                   << " and token " << eventBuffer[ uEvent ].GetTrigger().GetStarToken()
                   << FairLogger::endl;

         if( kTRUE == fbEventDumpEna )
         {
            fpBinDumpFile->write( reinterpret_cast< const char * >( &kuBinDumpBegWord ), sizeof( UInt_t ) );
            fpBinDumpFile->write( reinterpret_cast< const char * >( &iBuffSzByte ), sizeof( Int_t ) );
            fpBinDumpFile->write( reinterpret_cast< const char * >( pDataBuff ), iBuffSzByte );
            fpBinDumpFile->write( reinterpret_cast< const char * >( &kuBinDumpEndWord ), sizeof( UInt_t ) );
         } // if( kTRUE == fbEventDumpEna )

      } // if( NULL != pDataBuff )
         else LOG(ERROR) << "Invalid STAR SubEvent Output, can only happen if trigger "
                         << " object was not set => Do Nothing more with it!!! "
                         << FairLogger::endl;
   } // for( UInt_t uEvent = 0; uEvent < eventBuffer.size(); ++uEvent )
   fTimer.Stop();
   dctime += fTimer.CpuTime();
   
   if( 0 == fulTsCounter % 10000 ) {
     LOG(INFO) << "Processed " << fulTsCounter << " TS,  CPUtime: "<< dctime/10. << " ms/TS"
                << FairLogger::endl;
     dctime=0.;
   }
   fulTsCounter++;

   return kTRUE;
}

void CbmStar2019EventBuilderEtof::Reset()
{
}

void CbmStar2019EventBuilderEtof::FillOutput(CbmDigi* /*digi*/)
{
}

void CbmStar2019EventBuilderEtof::Finish()
{
   if( NULL != fpBinDumpFile )
   {
      LOG(INFO) << "Closing binary file used for event dump."
                << FairLogger::endl;
      fpBinDumpFile->close();
   } // if( NULL != fpBinDumpFile )

   /// If monitor mode enabled, trigger histos creation, obtain pointer on them and add them to the HTTP server
   if( kTRUE == fbMonitorMode )
   {
      /// Obtain vector of pointers on each histo from the algo (+ optionally desired folder)
      std::vector< std::pair< TNamed *, std::string > > vHistos = fEventBuilderAlgo->GetHistoVector();

      /// (Re-)Create ROOT file to store the histos
      TDirectory * oldDir = NULL;
      TFile * histoFile = NULL;
      // Store current directory position to allow restore later
      oldDir = gDirectory;
      // open separate histo file in recreate mode
      histoFile = new TFile( "data/eventBuilderMonHist.root" , "RECREATE");
      histoFile->cd();

      /// Register the histos in the HTTP server
      for( UInt_t uHisto = 0; uHisto < vHistos.size(); ++uHisto )
      {
         /// Make sure we end up in chosen folder
         gDirectory->mkdir( vHistos[ uHisto ].second.data() );
         gDirectory->cd( vHistos[ uHisto ].second.data() );

         /// Write plot
         vHistos[ uHisto ].first->Write();

         histoFile->cd();
      } // for( UInt_t uHisto = 0; uHisto < vHistos.size(); ++uHisto )

     // Restore original directory position
     oldDir->cd();
     histoFile->Close();
   } // if( kTRUE == fbMonitorMode )
}

ClassImp(CbmStar2019EventBuilderEtof)
