// ------------------------------------------------------------------
// -----                     TTofScal2014Unpacker                   -----
// -----              Created 19/04/2014 by P.-A. Loizeau       -----
// ------------------------------------------------------------------ 

#include "TTofScal2014Unpacker.h"

// TOF headers
#include "TofDef.h"
#include "TMbsUnpackTofPar.h"
#include "TofScal2014Def.h"
#include "TofScalerDef.h"
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

TTofScal2014Unpacker::TTofScal2014Unpacker():
   fParUnpack(0),
   fuNbScal2014(0),
   fScalerBoardCollection(NULL),
   fhScalers(),
   fhScalersAnd(),
   bFirstEvent(kTRUE),
   fvuFirstScalers(),
   fvuFirstScalersAnd()
{
}
TTofScal2014Unpacker::TTofScal2014Unpacker( TMbsUnpackTofPar * parIn ):
   fParUnpack( parIn ),
   fuNbScal2014( parIn->GetNbActiveBoards( tofMbs::scaler2014 ) ),
   fScalerBoardCollection(NULL),
   fhScalers(),
   fhScalersAnd(),
   bFirstEvent(kTRUE),
   fvuFirstScalers(),
   fvuFirstScalersAnd()
{
   // Recover first the ScalerBoard objects created in general unpacker class
   FairRootManager* rootMgr = FairRootManager::Instance();
   
   fScalerBoardCollection = (TClonesArray*) rootMgr->GetObject("TofRawScalers");
   if(NULL == fScalerBoardCollection) 
   {
      LOG(WARNING)<<"TTofScal2014Unpacker::TTofScal2014Unpacker : no Raw Scalers array! "<<FairLogger::endl;
      fuNbScal2014 = 0;
   } // if(NULL == fScalerBoardCollection) 
      else
      {
         // Initialize the ScalerBoard object with each Scal2014 characteristics
         TTofScalerBoard * fScalerBoard = NULL;
         
         // TRIGLOG is always considered as first scaler board!
         UInt_t uTrigOff = 0;
         if( kTRUE == fParUnpack->WithActiveTriglog() )
            uTrigOff = 1;
         
         LOG(DEBUG)<<"TTofScal2014Unpacker::TTofScal2014Unpacker : Initialize objects for  "
                  <<fuNbScal2014<<" boards "<<FairLogger::endl;

         for( Int_t iScalIndex = 0; iScalIndex < fuNbScal2014; iScalIndex++)
         {
        	 // TRIGLOG always first scaler board!
            fScalerBoard  = (TTofScalerBoard*)  fScalerBoardCollection->ConstructedAt( iScalIndex + uTrigOff);
            if( tofscaler::undef == fScalerBoard->GetScalerType() )
               fScalerBoard->SetType( tofscaler::scaler2014 );
         } // for( Int_t iScalIndex = 0; iScalIndex < fuNbScal2014; iScalIndex++)
      } // else of if(NULL == fScalerBoardCollection) 
   bFirstEvent = kTRUE;
}
TTofScal2014Unpacker::~TTofScal2014Unpacker()
{
//   DeleteHistos();
}

void TTofScal2014Unpacker::Clear(Option_t *option)
{
   fParUnpack = NULL;
   fuNbScal2014    = 0;
   fScalerBoardCollection = NULL;
}

void TTofScal2014Unpacker::ProcessScal2014( Int_t iScalIndex, UInt_t* pMbsData, UInt_t uLength )
{
   if( (iScalIndex<0) || (fuNbScal2014 <= iScalIndex) )
   {
      LOG(ERROR)<<"Error Scaler2014 number "<<iScalIndex<<" out of bound (max "<<fuNbScal2014<<") "<<FairLogger::endl;
      return;
   }
   
   // TRIGLOG is always considered as first scaler board!
   TTofScalerBoard * fScalerBoard = NULL;
   if( kTRUE == fParUnpack->WithActiveTriglog() )
   {
      // First check if ScalerBoard object for Triglog was created already
      if( 2 + iScalIndex <= fScalerBoardCollection->GetEntriesFast() )
         fScalerBoard = (TTofScalerBoard*) fScalerBoardCollection->ConstructedAt( 1 + iScalIndex, "C");
         else for( Int_t iScalBd = fScalerBoardCollection->GetEntriesFast(); iScalBd <= 1 + iScalIndex; iScalBd++ )
            fScalerBoard = (TTofScalerBoard*) fScalerBoardCollection->ConstructedAt( iScalBd); 
   } // if( kTRUE == fParUnpack->WithActiveTriglog() )
      else fScalerBoard = (TTofScalerBoard*) fScalerBoardCollection->ConstructedAt( iScalIndex, "C");

   UInt_t uIndx = 0;
   
   UInt_t uNbScalCh = scaler2014::kuNbChan;

   if( tofscaler::undef == fScalerBoard->GetScalerType() )
	   fScalerBoard->SetType( tofscaler::scaler2014 );
   
   // First readout scalers for the input channels
   for( Int_t iScalerInd=0; iScalerInd < scaler2014::kuNbChanIn; iScalerInd++)
   {
      UInt_t uValue = (UInt_t)( uIndx < uLength ? pMbsData[uIndx] : 0 );
      uIndx++;
      fScalerBoard->SetScalerValue(iScalerInd, uValue);
   } // for( Int_t iScalerInd=0; iScalerInd < uNbScalCh; iScalerInd++)

   // Then readout the reference clock scaler
   fScalerBoard->SetRefClk( pMbsData[uIndx] );
   uIndx++;

   // Finally readout the scalers for the AND of the input signal pairs
   for( Int_t iScalerInd=0; iScalerInd < scaler2014::kuNbChanAnd; iScalerInd++)
   {
      UInt_t uValue = (UInt_t)( uIndx < uLength ? pMbsData[uIndx] : 0 );
      uIndx++;
	   fScalerBoard->SetScalerValue( scaler2014::kuNbChanIn + iScalerInd, uValue);
   } // for( Int_t iScalerInd=0; iScalerInd < uNbScalCh; iScalerInd++)

   if( uLength != uIndx )
      LOG(ERROR)<<" TTofScal2014Unpacker::ProcessScal2014 => Scaler 2014 #"
               <<fParUnpack->GetActiveToAllTypeInd(iScalIndex, tofMbs::scaler2014)
               <<" unpacking length missmatch: length "
               <<uLength<<" VS index "<<uIndx
               <<FairLogger::endl;

   fScalerBoard->SetPresentFlag( kTRUE );

   LOG(DEBUG2)<<" Scaler 2014 #"<<fParUnpack->GetActiveToAllTypeInd(iScalIndex, tofMbs::scaler2014)
		      <<" finished"<<FairLogger::endl;
   return;
} 

// Histogram management
// TODO: maybe add some options to control which histograms are 
// created and filed (memory management)
void TTofScal2014Unpacker::CreateHistos()
{
   LOG(DEBUG)<<"TTofScal2014Unpacker::CreateHistos : Create histos for  "
            <<fuNbScal2014<<" boards "<<FairLogger::endl;

   TDirectory * oldir = gDirectory; // <= To prevent histos from being sucked in by the param file of the TRootManager!
   gROOT->cd(); // <= To prevent histos from being su

   fhScalers.resize(fuNbScal2014, NULL);
   fvuFirstScalers.resize(fuNbScal2014);

   fhScalersAnd.resize(fuNbScal2014, NULL);
   fvuFirstScalersAnd.resize(fuNbScal2014);

   for( Int_t iScalIndex = 0; iScalIndex < fuNbScal2014; iScalIndex++)
   {
      fhScalers[iScalIndex] = new TH1I(
            Form("tof_%s_%02d_scalersIn", tofscaler::ksTdcHistName[ tofscaler::scaler2014 ].Data(), iScalIndex ),
            "Counts per scaler channel; Channel []; Total counts []", scaler2014::kuNbChanIn, 0., scaler2014::kuNbChanIn);

      fvuFirstScalers[iScalIndex].resize( scaler2014::kuNbChanIn );
      for( Int_t iScalerInd=0; iScalerInd < scaler2014::kuNbChanIn; iScalerInd++)
         fvuFirstScalers[iScalIndex][iScalerInd] = 0;

      fhScalersAnd[iScalIndex] = new TH1I(
            Form("tof_%s_%02d_scalersAnd", tofscaler::ksTdcHistName[ tofscaler::scaler2014 ].Data(), iScalIndex ),
            "Counts per scaler channel; Channel []; Total counts []", scaler2014::kuNbChanAnd, 0., scaler2014::kuNbChanAnd);

      fvuFirstScalersAnd[iScalIndex].resize( scaler2014::kuNbChanAnd );
      for( Int_t iScalerInd=0; iScalerInd < scaler2014::kuNbChanAnd; iScalerInd++)
         fvuFirstScalersAnd[iScalIndex][iScalerInd] = 0;

   } // for( Int_t iScalIndex = 0; iScalIndex < fuNbScal2014; iScalIndex++)
   
   gDirectory->cd( oldir->GetPath() ); // <= To prevent histos from being sucked in by the param file of the TRootManager!

   LOG(DEBUG)<<"TTofScal2014Unpacker::CreateHistos : Created histos for  "
            <<fuNbScal2014<<" boards "<<FairLogger::endl;
}
void TTofScal2014Unpacker::FillHistos()
{
   LOG(DEBUG)<<"TTofScal2014Unpacker::FillHistos : Fill histos for  "
            <<fuNbScal2014<<" boards "<<FairLogger::endl;

   for( Int_t iScalIndex = 0; iScalIndex < fuNbScal2014; iScalIndex++)
   {

      // TRIGLOG is always considered as first scaler board!
      TTofScalerBoard * fScalerBoard;
      if( kTRUE == fParUnpack->WithActiveTriglog() )
         fScalerBoard = (TTofScalerBoard*) fScalerBoardCollection->ConstructedAt( 1 + iScalIndex );
         else fScalerBoard = (TTofScalerBoard*) fScalerBoardCollection->ConstructedAt( 0 + iScalIndex );

      if( kFALSE == fScalerBoard->IsUpdated() )
         continue;

      for( Int_t iScalerInd=0; iScalerInd < scaler2014::kuNbChanIn; iScalerInd++)
      {
         if( kTRUE == bFirstEvent )
            fvuFirstScalers[iScalIndex][iScalerInd] = fScalerBoard->GetScalerValue( iScalerInd );
            else fhScalers[iScalIndex]->SetBinContent( 1 + iScalerInd, fScalerBoard->GetScalerValue( iScalerInd )
                                                             - fvuFirstScalers[iScalIndex][iScalerInd] );
      } // for( Int_t iScalerInd=0; iScalerInd < scaler2014::kuNbChanIn; iScalerInd++)
      for( Int_t iScalerInd=0; iScalerInd < scaler2014::kuNbChanAnd; iScalerInd++)
      {
         if( kTRUE == bFirstEvent )
            fvuFirstScalersAnd[iScalIndex][iScalerInd] = fScalerBoard->GetScalerValue(
                  scaler2014::kuNbChanIn + iScalerInd );
            else fhScalersAnd[iScalIndex]->SetBinContent( 1 + iScalerInd,
                     fScalerBoard->GetScalerValue( scaler2014::kuNbChanIn +iScalerInd )
                     - fvuFirstScalersAnd[iScalIndex][iScalerInd] );
      } // for( Int_t iScalerInd=0; iScalerInd < scaler2014::kuNbChanAnd; iScalerInd++)
   } // for( Int_t iScalIndex = 0; iScalIndex < fuNbScal2014; iScalIndex++)
   if( kTRUE == bFirstEvent )
      bFirstEvent = kFALSE;

   LOG(DEBUG)<<"TTofScal2014Unpacker::FillHistos : Filled histos for  "
            <<fuNbScal2014<<" boards "<<FairLogger::endl;
}
void TTofScal2014Unpacker::WriteHistos( TDirectory* inDir)
{
   TDirectory * oldir = gDirectory;

   LOG(DEBUG)<<"TTofScal2014Unpacker::WriteHistos : Write histos for  "
            <<fuNbScal2014<<" boards "<<FairLogger::endl;
   for( Int_t iScalIndex = 0; iScalIndex < fuNbScal2014; iScalIndex++)
   {
      TDirectory *cdScal2014 = inDir->mkdir(
            Form( "Unp_%s_%02d", tofscaler::ksTdcHistName[ tofscaler::scaler2014 ].Data(), iScalIndex ) );
      cdScal2014->cd();    // make the "Unp_Scal2014" directory the current directory
      fhScalers[iScalIndex]->Write();
      fhScalersAnd[iScalIndex]->Write();
   } // for( Int_t iScalIndex = 0; iScalIndex < fuNbScal2014; iScalIndex++)
   
   gDirectory->cd( oldir->GetPath() );
}
void TTofScal2014Unpacker::DeleteHistos()
{
   // not sure if it will not make problems for seeing them
   // => for now just delete histograming variables

   for( Int_t iScalIndex = 0; iScalIndex < fuNbScal2014; iScalIndex++)
   {
      fvuFirstScalers[iScalIndex].clear();
      fvuFirstScalersAnd[iScalIndex].clear();
   } // for( Int_t iScalIndex = 0; iScalIndex < fuNbScal2014; iScalIndex++)
   fvuFirstScalers.clear();
   fvuFirstScalersAnd.clear();
} 
