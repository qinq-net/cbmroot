// ------------------------------------------------------------------
// -----                     TTofTriglogScalUnpacker                -----
// -----              Created 03/07/2013 by P.-A. Loizeau       -----
// -----              Adapted from TTriglogProc.cxx by S. Linev -----
// -----                 and TMbsCrateProc.cxx by P.-A. Loizeau ----
// ------------------------------------------------------------------

#include "TTofTriglogScalUnpacker.h"

// TOF headers
#include "TofDef.h"
#include "TMbsUnpackTofPar.h"
#include "TofTriglogDef.h"
#include "TofScalerDef.h"
#include "TTofTriglogBoard.h"
#include "TTofScalerBoard.h"

// FAIR headers
#include "FairRootManager.h"
#include "FairLogger.h"
#include "TString.h"

// ROOT headers
#include "TClonesArray.h"
#include "TH1.h"
#include "TROOT.h"
#include "TDirectory.h"
#include "TString.h"

TTofTriglogScalUnpacker::TTofTriglogScalUnpacker():
   fParUnpack(0),
   fScalerBoardCollection(NULL),
   fuNbTriglogScal( 0 ),
   fuTrigOff( 0 ),
   fuTotalTriggerCount(),
   fuFirstMbsTime(),
   fuLastMbsTime(),
   fuFirstScaler(),
   fuLastScaler(),
   fhScalers(),
   fvuFirstScalers(),
   fdFirstMbsTime(),
   fdPrevMbsTime(),
   fdCurrMbsTime(),
   fuLastRefClk(),
   fhRefClkRate(),
   fhRefClkRateEvo()
{
}
TTofTriglogScalUnpacker::TTofTriglogScalUnpacker( TMbsUnpackTofPar * parIn ):
   fParUnpack( parIn ),
   fScalerBoardCollection(NULL),
   fuNbTriglogScal( parIn->GetNbActiveBoards( tofMbs::triglogscal ) ),
   fuTrigOff( 0 ),
   fuTotalTriggerCount(),
   fuFirstMbsTime(),
   fuLastMbsTime(),
   fuFirstScaler(),
   fuLastScaler(),
   fhScalers(),
   fvuFirstScalers(),
   fdFirstMbsTime(),
   fdPrevMbsTime(),
   fdCurrMbsTime(),
   fuLastRefClk(),
   fhRefClkRate(),
   fhRefClkRateEvo()
{
   // Some triglog board can be used as scaler board instead of trigger source
   if( 0 < fuNbTriglogScal )
   {
      // Recover first the TRIGLOG and SCALER board objects created in general unpacker class
      FairRootManager* rootMgr = FairRootManager::Instance();

      fScalerBoardCollection = (TClonesArray*) rootMgr->GetObject("TofRawScalers");
      if(NULL == fScalerBoardCollection)
      {
         LOG(WARNING)<<"TTofTriglogScalUnpacker::TTofTriglogScalUnpacker : no Raw Scalers array! "<<FairLogger::endl;
      } // if(NULL == fScalerBoardCollection)
         else
         {
            // Initialize the ScalerBoard object with TRIGLOG characteristics
            TTofScalerBoard * fScalerBoard = NULL;

            // TRIGLOG is always considered as first scaler board!
            fuTrigOff = 0;
            if( kTRUE == fParUnpack->WithActiveTriglog() )
               fuTrigOff = 1;
            // There may be some simple scaler boards
            fuTrigOff += parIn->GetNbActiveBoards( tofMbs::scaler2014 );

            LOG(DEBUG)<<"TTofTriglogScalUnpacker::TTofTriglogScalUnpacker : Initialize objects for "
                  << fuNbTriglogScal << " Triglog boards used as scaler "<<FairLogger::endl;

            for( Int_t iScalIndex = 0; iScalIndex < fuNbTriglogScal; iScalIndex++)
            {
             // TRIGLOG always first scaler board!
             // SCALORMU are always next, followed by Triglog used as scalers!
               fScalerBoard  = (TTofScalerBoard*)  fScalerBoardCollection->ConstructedAt( iScalIndex + fuTrigOff);
               if( tofscaler::undef == fScalerBoard->GetScalerType() )
                  fScalerBoard->SetType( tofscaler::triglogscal );
            } // for( Int_t iScalIndex = 0; iScalIndex < fuNbTriglogScal; iScalIndex++)
         } // else of if(NULL == fScalerBoardCollection)

      fuTotalTriggerCount.resize(fuNbTriglogScal);
      fuFirstMbsTime.resize(fuNbTriglogScal);
      fuLastMbsTime.resize(fuNbTriglogScal);
      fuFirstScaler.resize(fuNbTriglogScal);
      fuLastScaler.resize(fuNbTriglogScal);
      fdFirstMbsTime.resize(fuNbTriglogScal);
      fdPrevMbsTime.resize(fuNbTriglogScal);
      fdCurrMbsTime.resize(fuNbTriglogScal);
      fuLastRefClk.resize(fuNbTriglogScal);
      for( Int_t iScalIndex = 0; iScalIndex < fuNbTriglogScal; iScalIndex++)
      {
         // Initialize variable used in filling
         fdFirstMbsTime[iScalIndex] = 0;
         fdPrevMbsTime[iScalIndex]  = -1;
         fdCurrMbsTime[iScalIndex]  = -1;
         fuLastRefClk[iScalIndex]   = 0;
         fuTotalTriggerCount[iScalIndex] = 0;
         fuFirstMbsTime[iScalIndex] = 0;
         fuLastMbsTime[iScalIndex]  = 0;

         fuFirstScaler[iScalIndex].resize(triglog::kuNbScalers);
         fuLastScaler[iScalIndex].resize(triglog::kuNbScalers);
         for( UInt_t uScaler = 0; uScaler < triglog::kuNbScalers; uScaler++)
         {
            fuFirstScaler[iScalIndex][uScaler].resize(triglog::kuNbChan);
            fuLastScaler[iScalIndex][uScaler].resize(triglog::kuNbChan);
         } // for( UInt_t uScaler = 0; uScaler <triglog::kuNbScalers; uScaler++)
      } // for( Int_t iScalIndex = 0; iScalIndex < fuNbTriglogScal; iScalIndex++)
   } // if(  0 < fuNbTriglogScal )
      else LOG(ERROR)<<"TTofTriglogScalUnpacker::TTofTriglogScalUnpacker => No TRIGLOG board active as scaler!!!!!"
                   <<" recheck your VME address matrix "<<FairLogger::endl;
}
TTofTriglogScalUnpacker::~TTofTriglogScalUnpacker()
{
   // Some triglog board can be used as scaler board instead of trigger source
   if(  0 < fuNbTriglogScal )
   {
      for( Int_t iScalIndex = 0; iScalIndex < fuNbTriglogScal; iScalIndex++)
      {
         for( UInt_t uScaler = 0; uScaler <triglog::kuNbScalers; uScaler++)
         {
            fuFirstScaler[iScalIndex][uScaler].clear();
            fuLastScaler[iScalIndex][uScaler].clear();
         } // for( UInt_t uScaler = 0; uScaler <triglog::kuNbScalers; uScaler++)
         fuFirstScaler[iScalIndex].clear();
         fuLastScaler[iScalIndex].clear();
      } // for( Int_t iScalIndex = 0; iScalIndex < fuNbTriglogScal; iScalIndex++)
      fuTotalTriggerCount.clear();
      fuFirstMbsTime.clear();
      fuLastMbsTime.clear();
      fuFirstScaler.clear();
      fuLastScaler.clear();
   } // if(  0 < fuNbTriglogScal )
      else LOG(ERROR)<<"TTofTriglogScalUnpacker::~TTofTriglogScalUnpacker => No TRIGLOG board active as scaler!!!!!"
                      <<" recheck your VME address matrix "<<FairLogger::endl;

//   DeleteHistos();
}

void TTofTriglogScalUnpacker::Clear(Option_t *option)
{
   fParUnpack = NULL;
   fuNbTriglogScal    = 0;
   fScalerBoardCollection = NULL;
}

void TTofTriglogScalUnpacker::ProcessTriglogScal( Int_t iTrigScalIndex, UInt_t* pMbsData, UInt_t uLength )
{
   if( (iTrigScalIndex<0) || (fuNbTriglogScal <= iTrigScalIndex) )
   {
      LOG(ERROR)<<"Error TriglogScal number "<<iTrigScalIndex<<" out of bound (max "<<fuNbTriglogScal<<") "<<FairLogger::endl;
      return;
   }

   // Getting output objects
   // TRIGLOG always first scaler board!
   TTofScalerBoard * fScalerBoard = NULL;
   if( kTRUE == fParUnpack->WithActiveTriglog() )
   {
      // First check if ScalerBoard object for Triglog was created already
      // and check if ScalerBoard object for ScalOrMu14 were created already
      if( 1 + fuTrigOff + iTrigScalIndex <= fScalerBoardCollection->GetEntriesFast() )
         fScalerBoard = (TTofScalerBoard*) fScalerBoardCollection->ConstructedAt( fuTrigOff + iTrigScalIndex, "C");
         else for( Int_t iScalBd = fScalerBoardCollection->GetEntriesFast(); iScalBd <= fuTrigOff + iTrigScalIndex; iScalBd++ )
            fScalerBoard = (TTofScalerBoard*) fScalerBoardCollection->ConstructedAt( iScalBd);
   } // if( kTRUE == fParUnpack->WithActiveTriglog() )
      else fScalerBoard = (TTofScalerBoard*) fScalerBoardCollection->ConstructedAt( fuTrigOff + iTrigScalIndex, "C");

   if( tofscaler::undef == fScalerBoard->GetScalerType() )
      fScalerBoard->SetType( tofscaler::triglogscal );

   UInt_t uIndx = 0;

   // Jump TRIGLOG not scaler data
   Int_t sync_num = pMbsData[uIndx];
//   fTriglogBoard->SetSyncNb( sync_num );
   uIndx++;
   LOG(DEBUG2)<<" ===================== TRIGLOG SCAL EVENT ============ " << sync_num % 0x1000000 <<FairLogger::endl;
//   fTriglogBoard->SetTriggPatt( (UInt_t) pMbsData[uIndx] );
   uIndx++;
//   fTriglogBoard->SetMbsTimeSec( (UInt_t) pMbsData[uIndx] );
   UInt_t uMbsTime = (UInt_t) pMbsData[uIndx];
   uIndx++;
   LOG(DEBUG2)<<" TRIGLOG SCAL UNPACK test 0 "<< iTrigScalIndex <<FairLogger::endl;
//   fTriglogBoard->SetMbsTimeMilliSec( (UInt_t) pMbsData[uIndx] );
   UInt_t uMbsTimeMs = (UInt_t) pMbsData[uIndx];
   LOG(DEBUG2)<<" TRIGLOG SCAL UNPACK test 0a "<< iTrigScalIndex <<FairLogger::endl;
   fdCurrMbsTime[iTrigScalIndex] = uMbsTime + uMbsTimeMs*1e-3; // BROKEN: Will not work as histo filling in different instance
   uIndx++;
   LOG(DEBUG2)<<" TRIGLOG SCAL UNPACK test 1"<<FairLogger::endl;

   for( UInt_t uScaler = 0; uScaler < triglog::kuNbScalers; uScaler++)
   {
      // SL 20.10.11 MBS readout was changed and two more scalers were included
      // to be able process old files, add this break condition;

      for( UInt_t uCh = 0; uCh < triglog::kuNbChan; uCh++)
      {
         // SL: put protection against different length of this subevent
         // also trigger-pattern was not correctly counted
         UInt_t uValue = (UInt_t)(uIndx < uLength ? pMbsData[uIndx] : 0);
         uIndx++;
         fScalerBoard->SetScalerValue( uCh, uValue, uScaler );
      } // for( UInt uCh = 0; uCh < triglog::kuNbChan; uCh++) {
   } // for( UInt uScaler = 0; uScaler < triglog::kuNbScalers; uScaler++)

   LOG(DEBUG2)<<" TRIGLOG SCAL UNPACK test 2"<<FairLogger::endl;
   if( uIndx < uLength)
   {
      // Jump TRIGLOG not scaler data
      // Input pattern is inside Tof MBS event only since August 2012
//      fTriglogBoard->SetInpPatt( pMbsData[uIndx] );
      uIndx++;
   } // if( uIndx < uLength)
   if( uIndx < uLength)
   {
      // Reference clock is inside Tof MBS event only since October 2012
      fScalerBoard->SetRefClk(  pMbsData[uIndx] );
      uIndx++;
   } // if( uIndx < uLength)
   LOG(DEBUG2)<<" TRIGLOG SCAL UNPACK finished"<<FairLogger::endl;

   fScalerBoard->SetPresentFlag( kTRUE );

   UpdateStats( iTrigScalIndex, uMbsTime );

   return;
}
void TTofTriglogScalUnpacker::UpdateStats( Int_t iTrigScalIndex, UInt_t uMbsTime )
{
   if( (iTrigScalIndex<0) || (fuNbTriglogScal <= iTrigScalIndex) )
   {
      LOG(ERROR)<<"Error TriglogScal number "<<iTrigScalIndex<<" out of bound (max "<<fuNbTriglogScal<<") "<<FairLogger::endl;
      return;
   }
   // Some triglog board can be used as scaler board instead of trigger source
   if(  0 < fuNbTriglogScal )
   {
      // Getting output objects
      // TRIGLOG always first scaler board! then ScalOrMu/Scal2014
      TTofScalerBoard  * fScalerBoard  = (TTofScalerBoard*)  fScalerBoardCollection->At(fuTrigOff + iTrigScalIndex);
      if(  NULL == fScalerBoard )
      {
         LOG(ERROR)<<"TTofTriglogScalUnpacker::UpdateStats => Board objects not existing!!!!"<<FairLogger::endl;
         LOG(ERROR)<<"TTofTriglogScalUnpacker::UpdateStats => TTofScalerBoard = "<<fScalerBoard<<FairLogger::endl;
         LOG(ERROR)<<"TTofTriglogScalUnpacker::UpdateStats => Probably the TRIGLOG SCALER sub-event is not there in this event!"<<FairLogger::endl;
         return;
      } // if( | NULL == fScalerBoard )

      // Preparing output at the end:
         // MBS time is time in s since LINUX epoch (circa 1970) => never 0 except on first event!
      for( UInt_t uScaler = 0; uScaler <triglog::kuNbScalers; uScaler++)
         for( UInt_t uCh = 0; uCh < triglog::kuNbChan; uCh++)
         {
            fuLastScaler[iTrigScalIndex][uScaler][uCh] = fScalerBoard->GetScalerValue( uCh, uScaler);
            if( fuFirstMbsTime[iTrigScalIndex] == 0 )
               fuFirstScaler[iTrigScalIndex][uScaler][uCh] = fuLastScaler[iTrigScalIndex][uScaler][uCh];
         } // for( UInt_t uCh = 0; uCh < triglog::kuNbChan; uCh++)
      fuLastMbsTime[iTrigScalIndex]     = uMbsTime;
      if( 0 == fuFirstMbsTime[iTrigScalIndex] )
         fuFirstMbsTime[iTrigScalIndex] = fuLastMbsTime[iTrigScalIndex];
   } // if(  0 < fuNbTriglogScal )
      else LOG(ERROR)<<"TTofTriglogScalUnpacker::UpdateStats => No TRIGLOG board active as scaler!!!!!"
                <<" recheck your VME address matrix "<<FairLogger::endl;
}

void TTofTriglogScalUnpacker::FinishTriglog( )
{
   // Some triglog board can be used as scaler board instead of trigger source
   if(  0 < fuNbTriglogScal )
   {
      for( Int_t iScalIndex = 0; iScalIndex < fuNbTriglogScal; iScalIndex++)
      {
         LOG(INFO)<<Form("-------- Triglog Scaler board %02d ----", iScalIndex)<<FairLogger::endl;
         Double_t dRuntime = fuLastMbsTime[iScalIndex] - fuFirstMbsTime[iScalIndex];
         LOG(INFO)<<Form("Total MBS run time %6.0f sec.", dRuntime)<<FairLogger::endl;

         if( 0 < dRuntime )
         {
            TString sLine = "";
            for( UInt_t uCh = 0; uCh < triglog::kuNbChan; uCh++)
            {
               sLine = Form("  Ch %2d", uCh );
               for( UInt_t uScaler = 0; uScaler <triglog::kuNbScalers; uScaler++)
                  sLine += Form("   %9u", fuLastScaler[iScalIndex][uScaler][uCh] - fuFirstScaler[iScalIndex][uScaler][uCh]);
               LOG(INFO)<<sLine<<FairLogger::endl;
            } // for( UInt_t uCh = 0; uCh < triglog::kuNbChan; uCh++)

            LOG(INFO)<<Form("---------------------------------------")<<FairLogger::endl;

            for( UInt_t uCh = 0; uCh < triglog::kuNbChan; uCh++)
            {
               sLine = Form("  Ch %2d", uCh );
               for( UInt_t uScaler = 0; uScaler <triglog::kuNbScalers; uScaler++)
                  sLine += Form("   %9.2f", (fuLastScaler[iScalIndex][uScaler][uCh] - fuFirstScaler[iScalIndex][uScaler][uCh])/dRuntime);
               LOG(INFO)<<sLine<<FairLogger::endl;
            } // for( UInt_t uCh = 0; uCh < triglog::kuNbChan; uCh++)
         } // if( 0 < dRuntime )
         LOG(INFO)<<Form("---------------------------------------")<<FairLogger::endl;
      } // for( Int_t iScalIndex = 0; iScalIndex < fuNbTriglogScal; iScalIndex++)
   } // if(  0 < fuNbTriglogScal )
      else LOG(ERROR)<<"TTofTriglogScalUnpacker::FinishTriglog => No TRIGLOG board active as scaler!!!!!"
             <<" recheck your VME address matrix "<<FairLogger::endl;
}

// Histogram management
// TODO: maybe add some options to control which histograms are
// created and filed (memory management)
void TTofTriglogScalUnpacker::CreateHistos()
{
   // Some triglog board can be used as scaler board instead of trigger source
   if(  0 < fuNbTriglogScal )
   {
      TDirectory * oldir = gDirectory; // <= To prevent histos from being sucked in by the param file of the TRootManager!
      gROOT->cd(); // <= To prevent histos from being sucked in by the param file of the TRootManager !

      fhRefClkRate.resize(fuNbTriglogScal);
      fhRefClkRateEvo.resize(fuNbTriglogScal);
      fhScalers.resize(fuNbTriglogScal);
      for( Int_t iScalIndex = 0; iScalIndex < fuNbTriglogScal; iScalIndex++)
      {
         fhRefClkRate[iScalIndex]    = new TH1I( Form("tof_%s_%02d_refclk", tofscaler::ksTdcHistName[ tofscaler::triglogscal ].Data(),
                                                                            iScalIndex ),
                              "Rate of the reference clock; Freq. [Hz]",
                              (Int_t)(2*triglog::kdRefClkFreq/1000), 0.0 , 2*triglog::kdRefClkFreq );
         fhRefClkRateEvo[iScalIndex] = new TH1I( Form("tof_%s_%02d_refclk_evo", tofscaler::ksTdcHistName[ tofscaler::triglogscal ].Data(),
               iScalIndex ),
                              "Reference clock counts per second; Time [s]; Counts []", 3600, 0.0, 3600 );

         fhScalers[iScalIndex].resize(triglog::kuNbScalers);
         for( UInt_t uScaler = 0; uScaler <triglog::kuNbScalers; uScaler++)
            fhScalers[iScalIndex][uScaler] = new TH1I( Form("tof_%s_%02d_scalers%02d", tofscaler::ksTdcHistName[ tofscaler::triglogscal ].Data(),
                                                                                       iScalIndex, uScaler ),
                                           Form("Counts per scaler channel in scaler %02d; Channel []; Total counts []", uScaler),
                                           triglog::kuNbChan, 0, triglog::kuNbChan);
      } // for( Int_t iScalIndex = 0; iScalIndex < fuNbTriglogScal; iScalIndex++)

      gDirectory->cd( oldir->GetPath() ); // <= To prevent histos from being sucked in by the param file of the TRootManager!
   } // if(  0 < fuNbTriglogScal )
      else LOG(ERROR)<<"TTofTriglogScalUnpacker::CreateHistos => No TRIGLOG board active as scaler!!!!!"
          <<" recheck your VME address matrix "<<FairLogger::endl;
}
void TTofTriglogScalUnpacker::FillHistos()
{
   // Some triglog board can be used as scaler board instead of trigger source
   for( Int_t iScalIndex = 0; iScalIndex < fuNbTriglogScal; iScalIndex++)
   {
      // Getting output objects
      // TRIGLOG always first scaler board! then ScalOrMu/Scal2014
      TTofScalerBoard  * fScalerBoard  = (TTofScalerBoard*)  fScalerBoardCollection->At(fuTrigOff + iScalIndex);

      if( NULL == fScalerBoard )
      {
         LOG(ERROR)<<"TTofTriglogScalUnpacker::FillHistos => Board objects not existing!!!!"<<FairLogger::endl;
         LOG(ERROR)<<"TTofTriglogScalUnpacker::FillHistos => TTofScalerBoard = "<<fScalerBoard<<FairLogger::endl;
         LOG(ERROR)<<"TTofTriglogScalUnpacker::FillHistos => Probably the TRIGLOG SCALER sub-event is not there in this event!"<<FairLogger::endl;
         return;
      } // if( NULL == fScalerBoard )

      LOG(DEBUG3)<<(fScalerBoardCollection->GetEntries())<<FairLogger::endl;

      // BROKEN: Will not work as data filling in different instance
      if( 0 == fdFirstMbsTime[iScalIndex] )
         // MBS time is time in s since LINUX epoch (circa 1970) => never 0 except on first event!
         fdFirstMbsTime[iScalIndex] = fdCurrMbsTime[iScalIndex];

      if( ( 0 == fdPrevMbsTime[iScalIndex] ) || ( fdPrevMbsTime[iScalIndex] + 0.05 < fdCurrMbsTime[iScalIndex] ) )
      {
         Double_t dTimeDiff = fdCurrMbsTime[iScalIndex] - fdPrevMbsTime[iScalIndex];
         if( 0 < fdPrevMbsTime[iScalIndex] && dTimeDiff < 0.3 )
         {
            fhRefClkRate[iScalIndex]->Fill( (Double_t)(fScalerBoard->GetRefClk() - fuLastRefClk[iScalIndex] ) /
                                dTimeDiff );
            fhRefClkRateEvo[iScalIndex]->Fill(fdCurrMbsTime[iScalIndex] - fdFirstMbsTime[iScalIndex],
                                              fScalerBoard->GetRefClk() - fuLastRefClk[iScalIndex]);
         } // if( 0 < fdPrevMbsTime )

         fdPrevMbsTime[iScalIndex] = fdCurrMbsTime[iScalIndex];
         fuLastRefClk[iScalIndex] = fScalerBoard->GetRefClk();
      } // if( ( 0 == fdPrevMbsTime ) || ( dMbsTime > fdPrevMbsTime + 0.1 ) )

      for( UInt_t uScaler = 0; uScaler <triglog::kuNbScalers; uScaler++)
         for( UInt_t uCh = 0; uCh < triglog::kuNbChan; uCh++)
            fhScalers[iScalIndex][uScaler]->SetBinContent( 1 + uCh, fScalerBoard->GetScalerValue( uCh, uScaler)
                                                      - fuFirstScaler[iScalIndex][uScaler][uCh] );

   } // for( Int_t iScalIndex = 0; iScalIndex < fuNbTriglogScal; iScalIndex++)
}
void TTofTriglogScalUnpacker::WriteHistos( TDirectory* inDir)
{
   // Some triglog board can be used as scaler board instead of trigger source
   for( Int_t iScalIndex = 0; iScalIndex < fuNbTriglogScal; iScalIndex++)
   {
      TDirectory * oldir = gDirectory;

      TDirectory *cdTriglog = inDir->mkdir( Form( "Unp_%s_%02d", tofscaler::ksTdcHistName[ tofscaler::triglogscal ].Data(), iScalIndex ) );
      cdTriglog->cd();    // make the "Unp_triglog" directory the current directory

      fhRefClkRate[iScalIndex]->Write();
      fhRefClkRateEvo[iScalIndex]->Write();
      for( UInt_t uScaler = 0; uScaler <triglog::kuNbScalers; uScaler++)
         fhScalers[iScalIndex][uScaler]->Write();

      gDirectory->cd( oldir->GetPath() );
   } // for( Int_t iScalIndex = 0; iScalIndex < fuNbTriglogScal; iScalIndex++)
}
void TTofTriglogScalUnpacker::DeleteHistos()
{
/*
   // Some triglog board can be used as scaler board instead of trigger source
   if(  0 < fuNbTriglogScal )
   {
   } // if(  0 < fuNbTriglogScal )
      else LOG(ERROR)<<"TTofTriglogScalUnpacker::DeleteHistos => No TRIGLOG board active as scaler!!!!!"
            <<" recheck your VME address matrix "<<FairLogger::endl;
*/
}
