// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmMcbm2018UnpackerTaskTof                    -----
// -----               Created 10.02.2019 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018UnpackerTaskTof.h"

#include "CbmMcbm2018UnpackerAlgoTof.h"
#include "CbmMcbm2018TofPar.h"
#include "CbmTbDaqBuffer.h"
#include "CbmTofDigiExp.h"

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

Bool_t bMcbm2018UnpackerTaskTofResetHistos = kFALSE;

CbmMcbm2018UnpackerTaskTof::CbmMcbm2018UnpackerTaskTof( UInt_t uNbGdpb )
  : CbmMcbmUnpack(),
    fbMonitorMode( kFALSE ),
    fbDebugMonitorMode( kFALSE ),
    fbSeparateArrayT0( kFALSE ),
    fUseDaqBuffer( kTRUE ),
    fParCList( nullptr ),
    fulTsCounter( 0 ),
    fBuffer( CbmTbDaqBuffer::Instance() ),
    fTofDigiCloneArray(),
    fUnpackerAlgo( nullptr ),
    fT0DigiCloneArray( nullptr )
{
   fUnpackerAlgo = new CbmMcbm2018UnpackerAlgoTof();
}

CbmMcbm2018UnpackerTaskTof::~CbmMcbm2018UnpackerTaskTof()
{
   delete fUnpackerAlgo;
}

Bool_t CbmMcbm2018UnpackerTaskTof::Init()
{
   LOG(INFO) << "CbmMcbm2018UnpackerTaskTof::Init" << FairLogger::endl;
   LOG(INFO) << "Initializing mCBM TOF 2018 Unpacker" << FairLogger::endl;

   FairRootManager* ioman = FairRootManager::Instance();
   if( NULL == ioman )
   {
      LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
   }

   fTofDigiCloneArray= new TClonesArray("CbmTofDigiExp", 10);
   if( NULL == fTofDigiCloneArray )
   {
      LOG(FATAL) << "Failed creating the TOF Digi TClonesarray " << FairLogger::endl;
   }
   ioman->Register("CbmTofDigi", "Tof raw Digi", fTofDigiCloneArray, kTRUE);

   if( kTRUE == fbSeparateArrayT0 )
   {
      fT0DigiCloneArray= new TClonesArray("CbmTofDigiExp", 10);
      if( NULL == fT0DigiCloneArray )
      {
         LOG(FATAL) << "Failed creating the T0 Digi TClonesarray " << FairLogger::endl;
      }
      ioman->Register("CbmT0Digi", "T0 raw Digi", fT0DigiCloneArray, kTRUE);
   } // if( kTRUE == fbSeparateArrayT0 )

   return kTRUE;
}

void CbmMcbm2018UnpackerTaskTof::SetParContainers()
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

Bool_t CbmMcbm2018UnpackerTaskTof::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
               << FairLogger::endl;

   /// Control flags
   CbmMcbm2018TofPar * pUnpackPar = dynamic_cast<CbmMcbm2018TofPar*>( FairRun::Instance()->GetRuntimeDb()->getContainer( "CbmMcbm2018TofPar" ) );
   if( nullptr == pUnpackPar )
   {
      LOG(ERROR) << "Failed to obtain parameter container CbmMcbm2018TofPar"
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
      if( nullptr != server )
      {
         for( UInt_t uHisto = 0; uHisto < vHistos.size(); ++uHisto )
         {
            server->Register( Form( "/%s", vHistos[ uHisto ].second.data() ), vHistos[ uHisto ].first );
         } // for( UInt_t uHisto = 0; uHisto < vHistos.size(); ++uHisto )

      server->RegisterCommand("/Reset_UnpTof_Hist", "bMcbm2018UnpackerTaskTofResetHistos=kTRUE");
      server->Restrict("/Reset_UnpTof_Hist", "allow=admin");
      } // if( nullptr != server )

   } // if( kTRUE == fbMonitorMode )

   fUnpackerAlgo->SetMonitorMode( fbMonitorMode );

   return initOK;
}

Bool_t CbmMcbm2018UnpackerTaskTof::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
             << FairLogger::endl;
   Bool_t initOK = fUnpackerAlgo->ReInitContainers();

   return initOK;
}

void CbmMcbm2018UnpackerTaskTof::AddMsComponentToList( size_t component, UShort_t usDetectorId )
{
   fUnpackerAlgo->AddMsComponentToList( component, usDetectorId );
}

Bool_t CbmMcbm2018UnpackerTaskTof::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   if( fbMonitorMode && bMcbm2018UnpackerTaskTofResetHistos )
   {
      LOG(INFO) << "Reset TOF unpacker histos " << FairLogger::endl;
      fUnpackerAlgo->ResetHistograms();
      bMcbm2018UnpackerTaskTofResetHistos = kFALSE;
   } // if( fbMonitorMode && bMcbm2018UnpackerTaskTofResetHistos )

   if( kFALSE == fUnpackerAlgo->ProcessTs( ts ) )
   {
      LOG(ERROR) << "Failed processing TS " << ts.index()
                 << " in unpacker algorithm class"
                 << FairLogger::endl;
      return kTRUE;
   } // if( kFALSE == fUnpackerAlgo->ProcessTs( ts ) )

   /// Copy the digis in the DaqBuffer
   std::vector< CbmTofDigiExp > vDigi = fUnpackerAlgo->GetVector();

   if ( fUseDaqBuffer ) {
     for( UInt_t uDigi = 0; uDigi < vDigi.size(); ++uDigi )
     {
        LOG(DEBUG) << Form("Insert 0x%08x digi with time ", vDigi[ uDigi ].GetAddress() ) << vDigi[ uDigi ].GetTime()
                   << Form(", Tot %4.0f", vDigi[ uDigi ].GetCharge())
                   << " into buffer with " << fBuffer->GetSize() << " data from "
                   << Form("%11.1f to %11.1f ", fBuffer->GetTimeFirst(), fBuffer->GetTimeLast())
                   << FairLogger::endl;
        fBuffer->InsertData( new CbmTofDigiExp( vDigi[ uDigi ] ) );
     }
   } else {
     for( auto digi: vDigi) {
       /// FIXME: remove T0 address hardcoding!!!
       if( kTRUE == fbSeparateArrayT0 && 0x00000066 == ( digi.GetAddress() & 0x00000FFF ) )
       {
         /// Insert data in TOF output container
         LOG(DEBUG) << "Fill digi TClonesarray with "
                    << Form("0x%08x", digi.GetAddress())
                    << " at " << static_cast<Int_t>( fT0DigiCloneArray->GetEntriesFast() )
                    << FairLogger::endl;

         new( (*fT0DigiCloneArray)[ fT0DigiCloneArray->GetEntriesFast() ] )
            CbmTofDigiExp( digi ) ;
       } // if( kTRUE == fbSeparateArrayT0 && 0x00000066 == ( digi.GetAddress() & 0x00000FFF ) )
       else
       {    
         /// Insert data in TOF output container
         LOG(DEBUG) << "Fill digi TClonesarray with "
                    << Form("0x%08x", digi.GetAddress())
                    << " at " << static_cast<Int_t>( fTofDigiCloneArray->GetEntriesFast() )
                    << FairLogger::endl;

         new( (*fTofDigiCloneArray)[ fTofDigiCloneArray->GetEntriesFast() ] )
            CbmTofDigiExp( digi ) ;
       } // else of if( kTRUE == fbSeparateArrayT0 && 0x00000066 == ( digi->GetAddress() & 0x00000FFF ) )
     }      
     vDigi.clear();
   }
   fUnpackerAlgo->ClearVector();

   if( 0 == fulTsCounter % 10000 )
      LOG(INFO) << "Processed " << fulTsCounter << "TS"
                << FairLogger::endl;
   fulTsCounter++;

   return kTRUE;
}

void CbmMcbm2018UnpackerTaskTof::Reset()
{
   fTofDigiCloneArray->Clear();

   if( kTRUE == fbSeparateArrayT0 )
   {
      fT0DigiCloneArray->Clear();
   } // if( kTRUE == fbSeparateArrayT0 )
}

void CbmMcbm2018UnpackerTaskTof::FillOutput(CbmDigi* digi)
{
   /// FIXME: remove T0 address hardcoding!!!
   if( kTRUE == fbSeparateArrayT0 && 0x00000066 == ( digi->GetAddress() & 0x00000FFF ) )
   {
      /// Insert data in TOF output container
      LOG(DEBUG) << "Fill digi TClonesarray with "
                 << Form("0x%08x", digi->GetAddress())
                 << " at " << static_cast<Int_t>( fT0DigiCloneArray->GetEntriesFast() )
                 << FairLogger::endl;

      new( (*fT0DigiCloneArray)[ fT0DigiCloneArray->GetEntriesFast() ] )
         CbmTofDigiExp( *( dynamic_cast<CbmTofDigiExp*>(digi) ) );
   } // if( kTRUE == fbSeparateArrayT0 && 0x00000066 == ( digi->GetAddress() & 0x00000FFF ) )
      else
      {
         /// Insert data in TOF output container
         LOG(DEBUG) << "Fill digi TClonesarray with "
                    << Form("0x%08x", digi->GetAddress())
                    << " at " << static_cast<Int_t>( fTofDigiCloneArray->GetEntriesFast() )
                    << FairLogger::endl;

         new( (*fTofDigiCloneArray)[ fTofDigiCloneArray->GetEntriesFast() ] )
            CbmTofDigiExp( *( dynamic_cast<CbmTofDigiExp*>(digi) ) );
      } // else of if( kTRUE == fbSeparateArrayT0 && 0x00000066 == ( digi->GetAddress() & 0x00000FFF ) )

/*
   if( 1 == fTofDigiCloneArray->GetEntriesFast())
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

void CbmMcbm2018UnpackerTaskTof::Finish()
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
      histoFile = new TFile( "data/HistosUnpackerTof.root" , "RECREATE");
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

void CbmMcbm2018UnpackerTaskTof::SetIgnoreOverlapMs( Bool_t bFlagIn )
{
   fUnpackerAlgo->SetIgnoreOverlapMs( bFlagIn );
}

void CbmMcbm2018UnpackerTaskTof::SetTimeOffsetNs( Double_t dOffsetIn )
{
   fUnpackerAlgo->SetTimeOffsetNs( dOffsetIn );
}
void CbmMcbm2018UnpackerTaskTof::SetDiamondDpbIdx( UInt_t uIdx )
{
   fUnpackerAlgo->SetDiamondDpbIdx( uIdx );
}

ClassImp(CbmMcbm2018UnpackerTaskTof)
