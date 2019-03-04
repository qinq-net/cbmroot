// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmMcbm2018MonitorTaskT0                      -----
// -----               Created 10.02.2019 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018MonitorTaskT0.h"

#include "CbmMcbm2018MonitorAlgoT0.h"
#include "CbmMcbm2018TofPar.h"

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

Bool_t bMcbm2018MonitorTaskT0ResetHistos = kFALSE;

CbmMcbm2018MonitorTaskT0::CbmMcbm2018MonitorTaskT0()
  : CbmMcbmUnpack(),
    fbMonitorMode( kTRUE ),
    fbDebugMonitorMode( kFALSE ),
    fuHistoryHistoSize( 3600 ),
    fParCList( nullptr ),
    fulTsCounter( 0 ),
    fMonitorAlgo( nullptr )
{
   fMonitorAlgo = new CbmMcbm2018MonitorAlgoT0();
}

CbmMcbm2018MonitorTaskT0::~CbmMcbm2018MonitorTaskT0()
{
   delete fMonitorAlgo;
}

Bool_t CbmMcbm2018MonitorTaskT0::Init()
{
   LOG(INFO) << "CbmMcbm2018MonitorTaskT0::Init" << FairLogger::endl;
   LOG(INFO) << "Initializing mCBM T0 2019 Monitor" << FairLogger::endl;

   return kTRUE;
}

void CbmMcbm2018MonitorTaskT0::SetParContainers()
{
   LOG(INFO) << "Setting parameter containers for " << GetName()
             << FairLogger::endl;

   fParCList = fMonitorAlgo->GetParList();

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

Bool_t CbmMcbm2018MonitorTaskT0::InitContainers()
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
   Bool_t initOK = fMonitorAlgo->InitContainers();

   /// Histos creation, obtain pointer on them and add them to the HTTP server
      /// Trigger histo creation on all associated algos
   initOK &= fMonitorAlgo->CreateHistograms();

      /// Obtain vector of pointers on each histo from the algo (+ optionally desired folder)
   std::vector< std::pair< TNamed *, std::string > > vHistos = fMonitorAlgo->GetHistoVector();

      /// Register the histos in the HTTP server
   THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
   if( nullptr != server )
   {
      for( UInt_t uHisto = 0; uHisto < vHistos.size(); ++uHisto )
      {
         server->Register( Form( "/%s", vHistos[ uHisto ].second.data() ), vHistos[ uHisto ].first );
      } // for( UInt_t uHisto = 0; uHisto < vHistos.size(); ++uHisto )

   server->RegisterCommand("/Reset_MoniT0_Hist", "bMcbm2018MonitorTaskT0ResetHistos=kTRUE");
   server->Restrict("/Reset_MoniT0_Hist", "allow=admin");
   } // if( nullptr != server )

   fMonitorAlgo->SetMonitorMode( fbMonitorMode );
   fMonitorAlgo->SetHistoryHistoSize( fuHistoryHistoSize );

   return initOK;
}

Bool_t CbmMcbm2018MonitorTaskT0::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
             << FairLogger::endl;
   Bool_t initOK = fMonitorAlgo->ReInitContainers();

   return initOK;
}

void CbmMcbm2018MonitorTaskT0::AddMsComponentToList( size_t component, UShort_t usDetectorId )
{
   fMonitorAlgo->AddMsComponentToList( component, usDetectorId );
}

Bool_t CbmMcbm2018MonitorTaskT0::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   if( fbMonitorMode && bMcbm2018MonitorTaskT0ResetHistos )
   {
      LOG(INFO) << "Reset T0 Monitor histos " << FairLogger::endl;
      fMonitorAlgo->ResetHistograms();
      bMcbm2018MonitorTaskT0ResetHistos = kFALSE;
   } // if( fbMonitorMode && bMcbm2018MonitorTaskT0ResetHistos )

   if( kFALSE == fMonitorAlgo->ProcessTs( ts ) )
   {
      LOG(ERROR) << "Failed processing TS " << ts.index()
                 << " in unpacker algorithm class"
                 << FairLogger::endl;
      return kTRUE;
   } // if( kFALSE == fMonitorAlgo->ProcessTs( ts ) )

   /// Cleqr the digis vector in case it was filled
   std::vector< CbmTofDigiExp > vDigi = fMonitorAlgo->GetVector();
   fMonitorAlgo->ClearVector();

   if( 0 == fulTsCounter % 10000 )
      LOG(INFO) << "Processed " << fulTsCounter << "TS"
                << FairLogger::endl;
   fulTsCounter++;

   return kTRUE;
}

void CbmMcbm2018MonitorTaskT0::Reset()
{
}

void CbmMcbm2018MonitorTaskT0::FillOutput(CbmDigi* /*digi*/)
{
}

void CbmMcbm2018MonitorTaskT0::Finish()
{
   /// Obtain vector of pointers on each histo from the algo (+ optionally desired folder)
   std::vector< std::pair< TNamed *, std::string > > vHistos = fMonitorAlgo->GetHistoVector();

   /// (Re-)Create ROOT file to store the histos
   TDirectory * oldDir = NULL;
   TFile * histoFile = NULL;
   // Store current directory position to allow restore later
   oldDir = gDirectory;
   // open separate histo file in recreate mode
   histoFile = new TFile( "data/HistosMonitorT0.root" , "RECREATE");
   histoFile->cd();

   /// Register the histos in the HTTP server
   for( UInt_t uHisto = 0; uHisto < vHistos.size(); ++uHisto )
   {
      /// Make sure we end up in chosen folder
      TString sFolder = vHistos[ uHisto ].second.data();
      if( nullptr == gDirectory->Get( sFolder ) )
         gDirectory->mkdir( sFolder );
      gDirectory->cd( sFolder );

      /// Write plot
      vHistos[ uHisto ].first->Write();

      histoFile->cd();
   } // for( UInt_t uHisto = 0; uHisto < vHistos.size(); ++uHisto )

  // Restore original directory position
  oldDir->cd();
  histoFile->Close();
}

void CbmMcbm2018MonitorTaskT0::SetIgnoreOverlapMs( Bool_t bFlagIn )
{
   fMonitorAlgo->SetIgnoreOverlapMs( bFlagIn );
}

ClassImp(CbmMcbm2018MonitorTaskT0)
