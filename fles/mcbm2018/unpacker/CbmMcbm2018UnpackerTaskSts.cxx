// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmMcbm2018UnpackerTaskSts                    -----
// -----               Created 26.01.2019 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018UnpackerTaskSts.h"

#include "CbmMcbm2018UnpackerAlgoSts.h"
#include "CbmMcbm2018StsPar.h"
#include "CbmTbDaqBuffer.h"
#include "CbmStsDigi.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRunOnline.h"
#include "FairParGenericSet.h"

#include "THttpServer.h"
#include "TROOT.h"
#include "TString.h"
#include "TClonesArray.h"

#include <iostream>
#include <stdint.h>
#include <iomanip>
#include <fstream>
#include <chrono>

Bool_t bMcbm2018UnpackerTaskStsResetHistos = kFALSE;

CbmMcbm2018UnpackerTaskSts::CbmMcbm2018UnpackerTaskSts( UInt_t uNbGdpb )
  : CbmMcbmUnpack(),
    fbMonitorMode( kFALSE ),
    fParCList( nullptr ),
    fulTsCounter( 0 ),
    fBuffer( CbmTbDaqBuffer::Instance() ),
    fStsDigiCloneArray(),
    fUnpackerAlgo( nullptr )
{
   fUnpackerAlgo = new CbmMcbm2018UnpackerAlgoSts();
}

CbmMcbm2018UnpackerTaskSts::~CbmMcbm2018UnpackerTaskSts()
{
   delete fUnpackerAlgo;
}

Bool_t CbmMcbm2018UnpackerTaskSts::Init()
{
   LOG(INFO) << "CbmMcbm2018UnpackerTaskSts::Init" << FairLogger::endl;
   LOG(INFO) << "Initializing mCBM STS 2018 Unpacker" << FairLogger::endl;

   FairRootManager* ioman = FairRootManager::Instance();
   if( NULL == ioman )
   {
      LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
   }

   fStsDigiCloneArray= new TClonesArray("CbmStsDigi", 10);
   if( NULL == fStsDigiCloneArray )
   {
      LOG(FATAL) << "Failed creating the STS Digi TClonesarray " << FairLogger::endl;
   }
   ioman->Register("CbmStsDigi", "STS raw Digi", fStsDigiCloneArray, kTRUE);

   return kTRUE;
}

void CbmMcbm2018UnpackerTaskSts::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
             << FairLogger::endl;

   fParCList = fUnpackerAlgo->GetParList();

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

Bool_t CbmMcbm2018UnpackerTaskSts::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
               << FairLogger::endl;

   /// Control flags
   CbmMcbm2018StsPar * pUnpackPar = dynamic_cast<CbmMcbm2018StsPar*>( FairRun::Instance()->GetRuntimeDb()->getContainer( "CbmMcbm2018StsPar" ) );
   if( nullptr == pUnpackPar )
   {
      LOG(ERROR) << "Failed to obtain parameter container CbmMcbm2018StsPar"
                 << FairLogger::endl;
      return kFALSE;
   } // if( nullptr == pUnpackPar )
/*
   fbMonitorMode = pUnpackPar->GetMonitorMode();
   LOG(INFO) << "Monitor mode:       "
             << ( fbMonitorMode ? "ON" : "OFF" )
             << FairLogger::endl;

   fbDebugMonitorMode = pUnpackPar->GetDebugMonitorMode();
   LOG(INFO) << "Debug Monitor mode: "
             << ( fbDebugMonitorMode ? "ON" : "OFF" )
             << FairLogger::endl;
*/
   Bool_t initOK = fUnpackerAlgo->InitContainers();

   /// If monitor mode enabled, trigger histos creation, obtain pointer on them and add them to the HTTP server
   if( kTRUE == fbMonitorMode )
   {
      /// Trigger histo creation on all associated algos
      initOK &= fUnpackerAlgo->CreateHistograms();

      /// Obtain vector of pointers on each histo from the algo (+ optionally desired folder)
      std::vector< std::pair< TNamed *, std::string > > vHistos = fUnpackerAlgo->GetHistoVector();

      /// Register the histos in the HTTP server
      THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
      for( UInt_t uHisto = 0; uHisto < vHistos.size(); ++uHisto )
      {
         server->Register( Form( "/%s", vHistos[ uHisto ].second.data() ), vHistos[ uHisto ].first );
      } // for( UInt_t uHisto = 0; uHisto < vHistos.size(); ++uHisto )

      server->RegisterCommand("/Reset_UnpSts_Hist", "bMcbm2018UnpackerTaskStsResetHistos=kTRUE");
      server->Restrict("/Reset_UnpSts_Hist", "allow=admin");

   } // if( kTRUE == fbMonitorMode )

   return initOK;
}

Bool_t CbmMcbm2018UnpackerTaskSts::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
             << FairLogger::endl;
   Bool_t initOK = fUnpackerAlgo->ReInitContainers();

   return initOK;
}

void CbmMcbm2018UnpackerTaskSts::AddMsComponentToList( size_t component, UShort_t usDetectorId )
{
   fUnpackerAlgo->AddMsComponentToList( component, usDetectorId );
}

Bool_t CbmMcbm2018UnpackerTaskSts::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   if( fbMonitorMode && bMcbm2018UnpackerTaskStsResetHistos )
   {
      LOG(INFO) << "Reset STS unpacker histos " << FairLogger::endl;
      fUnpackerAlgo->ResetHistograms();
      bMcbm2018UnpackerTaskStsResetHistos = kFALSE;
   } // if( fbMonitorMode && bMcbm2018UnpackerTaskStsResetHistos )

   if( kFALSE == fUnpackerAlgo->ProcessTs( ts ) )
   {
      LOG(ERROR) << "Failed processing TS " << ts.index()
                 << " in unpacker algorithm class"
                 << FairLogger::endl;
      return kTRUE;
   } // if( kFALSE == fUnpackerAlgo->ProcessTs( ts ) )

   /// Copy the digis in the DaqBuffer
   std::vector< CbmStsDigi > vDigi = fUnpackerAlgo->GetVector();
   for( UInt_t uDigi = 0; uDigi < vDigi.size(); ++uDigi )
      fBuffer->InsertData( new CbmStsDigi( vDigi[ uDigi ] ) );
   fUnpackerAlgo->ClearVector();

   if( 0 == fulTsCounter % 10000 )
      LOG(INFO) << "Processed " << fulTsCounter << "TS"
                << FairLogger::endl;
   fulTsCounter++;

   return kTRUE;
}

void CbmMcbm2018UnpackerTaskSts::Reset()
{
   fStsDigiCloneArray->Clear();
}

void CbmMcbm2018UnpackerTaskSts::FillOutput(CbmDigi* digi)
{
   /// Insert data in output container
   LOG(DEBUG) << "Fill digi TClonesarray with "
              << Form("0x%08x", digi->GetAddress())
              << " at " << static_cast<Int_t>( fStsDigiCloneArray->GetEntriesFast() )
              << FairLogger::endl;


   new( (*fStsDigiCloneArray)[ fStsDigiCloneArray->GetEntriesFast() ] )
      CbmStsDigi( *( dynamic_cast<CbmStsDigi*>(digi) ) );
/*
   if( 1 == fStsDigiCloneArray->GetEntriesFast())
      fdEvTime0=digi->GetTime();
      else fhRawTDigEvT0->Fill( digi->GetTime() - fdEvTime0 );

   if( (digi->GetAddress() & DetMask) != 0x00005006 )
   {
      fhRawTDigRef0->Fill( digi->GetTime() - fdRefTime);
      fhRawTDigRef->Fill( digi->GetTime() - fdRefTime);

      Double_t dDigiTime = digi->GetTime();
      fhRawDigiLastDigi->Fill( dDigiTime - dLastDigiTime );
      dLastDigiTime = dDigiTime;
   } // if( (digi->GetAddress() & DetMask) != 0x00005006 )
      else  fdRefTime = digi->GetTime();
*/
   digi->Delete();
}

void CbmMcbm2018UnpackerTaskSts::Finish()
{
   /// If monitor mode enabled, trigger histos creation, obtain pointer on them and add them to the HTTP server
   if( kTRUE == fbMonitorMode )
   {
      /// Obtain vector of pointers on each histo from the algo (+ optionally desired folder)
      std::vector< std::pair< TNamed *, std::string > > vHistos = fUnpackerAlgo->GetHistoVector();

      /// (Re-)Create ROOT file to store the histos
      TDirectory * oldDir = NULL;
      TFile * histoFile = NULL;
      // Store current directory position to allow restore later
      oldDir = gDirectory;
      // open separate histo file in recreate mode
      histoFile = new TFile( "data/HistosUnpackerSts.root" , "RECREATE");
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

void CbmMcbm2018UnpackerTaskSts::SetIgnoreOverlapMs( Bool_t bFlagIn )
{
   fUnpackerAlgo->SetIgnoreOverlapMs( bFlagIn );
}

void CbmMcbm2018UnpackerTaskSts::SetTimeOffsetNs( Double_t dOffsetIn )
{
   fUnpackerAlgo->SetTimeOffsetNs( dOffsetIn );
}

ClassImp(CbmMcbm2018UnpackerTaskSts)
