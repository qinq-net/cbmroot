// ------------------------------------------------------------------
// -----                    TMbsUnpackTof                       -----
// -----              Created 05/07/2013 by P.-A. Loizeau       -----
// ------------------------------------------------------------------
#include "TMbsUnpackTof.h"

// SubEvent ProcId from ROC library
#include "commons.h"

// General MBS headers
#include "TofDef.h"
#include "TMbsUnpackTofPar.h"

// ToF specific headers
#include "TTofScal2014Unpacker.h"
#include "TTofScomUnpacker.h"
#include "TTofTriglogScalUnpacker.h"
#include "TTofScalerBoard.h"
#include "TTofVftxUnpacker.h"
#include "TTofVftxBoard.h"
#include "TTofTdcBoard.h"

// ROOT headers
#include "TClonesArray.h"
#include "TFile.h"
#include "TDatime.h"
#include "TROOT.h"
#include "TDirectory.h"

// FAIR headers
#include "FairLogger.h"
//#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

TMbsUnpackTof::TMbsUnpackTof() : 
   FairUnpack( 10, 1, roc3587::proc_COSY_Nov11, 1, 9  ),
   fiVerbosity(0),
   fMbsUnpackPar(0),
   fiNbEvents(0),
   fiFirstEventNumber(0),
   fiLastEventNumber(0),
   fLastCheck(),
   fCheck(),
   fScal2014Unp(NULL),
   fScomUnp(NULL),
   fVftxUnp(NULL),
   fTrloScalUnp(NULL),
   fScalerBoardCollection(NULL),
   fVftxBoardCollection(NULL),
   fbSaveScalers(kFALSE),
   fbSaveRawVftx(kFALSE)
{
}

TMbsUnpackTof::TMbsUnpackTof( Int_t verbose ) : 
   FairUnpack( 10, 1, roc3587::proc_COSY_Nov11, 1, 9 ),
   fiVerbosity(verbose),
   fMbsUnpackPar(0),
   fiNbEvents(0),
   fiFirstEventNumber(0),
   fiLastEventNumber(0),
   fLastCheck(),
   fCheck(),
   fScal2014Unp(NULL),
   fScomUnp(NULL),
   fVftxUnp(NULL),
   fTrloScalUnp(NULL),
   fScalerBoardCollection(NULL),
   fVftxBoardCollection(NULL),
   fbSaveScalers(kFALSE),
   fbSaveRawVftx(kFALSE)
{
}

TMbsUnpackTof::TMbsUnpackTof( Int_t type, Int_t subType, Short_t procId, Int_t verbose,
                                Short_t subCrate, Short_t control) :
   FairUnpack( type, subType, procId, subCrate, control ),
   fiVerbosity(verbose),
   fMbsUnpackPar(0),
   fiNbEvents(0),
   fiFirstEventNumber(0),
   fiLastEventNumber(0),
   fLastCheck(),
   fCheck(),
   fScal2014Unp(NULL),
   fScomUnp(NULL),
   fVftxUnp(NULL),
   fTrloScalUnp(NULL),
   fScalerBoardCollection(NULL),
   fVftxBoardCollection(NULL),
   fbSaveScalers(kFALSE),
   fbSaveRawVftx(kFALSE)
{
}
      
TMbsUnpackTof::~TMbsUnpackTof()
{      
   LOG(INFO)<<"**** TMbsUnpackTof: Delete instance "<<FairLogger::endl;

   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::scaler2014 ) )
      delete fScal2014Unp;
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::scalormu ) )
      delete fScomUnp;
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::triglogscal ) )
      delete fTrloScalUnp;
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::vftx ) )
      delete fVftxUnp;
   LOG(INFO)<<"**** TMbsUnpackTof: instance deleted "<<FairLogger::endl;
}

// --------------------------------------------------
// FairUnpack specific functions
Bool_t TMbsUnpackTof::Init()
{
   LOG(INFO)<<"**** TMbsUnpackTof: Init Params "<<FairLogger::endl;
   if( kFALSE == InitParameters() )
      LOG(ERROR)<<"**** TMbsUnpackTof: Failed to Initialize params"<<FairLogger::endl;
//      return kFALSE;
   LOG(INFO)<<"**** TMbsUnpackTof: Params Initialized "<<FairLogger::endl;
   
   // Create the proper Unpackers and number of output objects for each boards type
//   if( kFALSE == RegisterOutput() )
//      return kFALSE;
   Register(); // FairUnpack Style, I feel unsafe as no check on output Array validity!
   if( kFALSE == CreateUnpackers() )
      LOG(ERROR)<<"**** TMbsUnpackTof: Failed to create unpackers "<<FairLogger::endl;
//      return kFALSE;
   
   return kTRUE;
}
Bool_t TMbsUnpackTof::DoUnpack(Int_t* data, Int_t size)
{   
   Int_t  * pData   = data;
//   UInt_t  uNbWords = size/2 - 1; // <= Somehow FAIRROOT transfers the size in words from the subevt header and not the size in longwords reads by the f_evt_get_subevent function
// Changed in git commit 0c0bd037c201d3496f9d5d7c133874382e885677 to fairroot LMD source
// TODO: Make sure the same change is applied to FairMbsStreamSource !!!!
   UInt_t  uNbWords = size;

   
   if( 3 < fiVerbosity )
   {
      TString sPrintEvent = "";
      LOG(INFO)<<"----------------------------------------------------------------"<<FairLogger::endl;
      LOG(INFO)<<"Size: "<<uNbWords<<FairLogger::endl;
      for( UInt_t uWdInd = 0; uWdInd < uNbWords; uWdInd++ )
      {
         if( 0 < uWdInd && 0 == uWdInd%8 )
         {
            LOG(INFO)<<sPrintEvent<<FairLogger::endl;
            sPrintEvent = "";
         } // if( 0 < uWdInd && 0 == uWdInd%8 )
         sPrintEvent += Form("%08x ", pData[uWdInd]);
      } // for( UInt_t uWdInd = 0; uWdInd < uNbWords; uWdInd++ )
      LOG(INFO)<<sPrintEvent<<FairLogger::endl;
      LOG(INFO)<<"----------------------------------------------------------------"<<FairLogger::endl;
   } // if( 3 < fiVerbosity )

   Int_t* iTagPos[ fMbsUnpackPar->GetBoardsNumber() ];
   Int_t  iTagLen[ fMbsUnpackPar->GetBoardsNumber() ];

   for(UInt_t uBoard = 0; uBoard < fMbsUnpackPar->GetBoardsNumber(); uBoard++) 
   {
      iTagPos[uBoard] = NULL;
      iTagLen[uBoard] = 0;
   } // for(UInt_t uBoard = 0; uBoard < fMbsUnpackPar->GetBoardsNumber(); uBoard++) 

   Int_t iLastTag = -1;

//   if( 400 < fiNbEvents )
//         LOG(ERROR)<<"TMbsUnpackTof::ProcessSubevent => Event "<<fiNbEvents<< "size "<<size<<" nbWords "<<uNbWords<<FairLogger::endl;
   for (UInt_t uCurrWord = 0; uCurrWord < uNbWords; ++uCurrWord) 
   {
      ULong64_t* plTag = (ULong64_t*) pData;
      Bool_t bFound = kFALSE;

      // loop over boards until finding a matching one0
      for( UInt_t uBoard=0;uBoard< fMbsUnpackPar->GetBoardsNumber(); uBoard++ )
         if( kTRUE == fMbsUnpackPar->IsActive( uBoard ) )
            if ((*plTag == fMbsUnpackPar->GetBoardTag( uBoard )) && (fMbsUnpackPar->GetBoardTag( uBoard )!=0)) 
            {
               if (iLastTag>=0) 
               {
                  // If a tag was already found:
                  // length of corresponding data is nb words since it was found
                  iTagLen[iLastTag] = pData - iTagPos[iLastTag];
                  iLastTag = -1;
               } // if (iLastTag>=0) 

               if (iTagPos[uBoard]==0) 
               {
//                  if( 411 == fiNbEvents )
//                     cout<<"Found board "<<uBoard<<" Tag "
//                         <<Form("%08llX %08llX", fMbsUnpackPar->GetBoardTag( uBoard )>>32, (fMbsUnpackPar->GetBoardTag( uBoard )&0xFFFFFFFF) )<<" at word "<<uCurrWord<<endl;
                  // if Tag of this board was never found
                  pData+=2; 
                  uCurrWord+=1;
                  iTagPos[uBoard] = pData;
                  iLastTag = uBoard;
               } // if (iTagPos[uBoard]==0) 
                  else 
                  {
                     LOG(ERROR)<<"----------------------------------------------------------------"<<FairLogger::endl;
                     LOG(ERROR)<<"TMbsUnpackTof::ProcessSubevent => size "<<size<<" nbWords "<<uNbWords<<FairLogger::endl;
                     LOG(ERROR)<<"TMbsUnpackTof::ProcessSubevent => FORMAT ERROR found tag "
                               <<Form("%08llX %08llX", fMbsUnpackPar->GetBoardTag( uBoard )>>32, 
                                      fMbsUnpackPar->GetBoardTag( uBoard ) )
                               <<" twice in event "<<fiNbEvents<<FairLogger::endl;
                     LOG(ERROR)<<" First: "<<iTagPos[uBoard]<<" Second "<<pData<<" Origin "<<data
                               <<" Last Board "<<iLastTag<<" This Board "<<uBoard<<FairLogger::endl;
                     LOG(ERROR)<<" Current word: "<<uCurrWord<<FairLogger::endl;
                     LOG(ERROR)<<" Buffer: "<<Form("%08llX %08llX", (*plTag)>>32, (*plTag)&0xFFFFFFFF )<<FairLogger::endl;
                     // Display full sub-event for debug
                     TString sPrintEvent = "";
                     for( UInt_t uWdInd = 0; uWdInd < uNbWords; uWdInd++ )
                     {
                        if( 0 < uWdInd && 0 == uWdInd%8 )
                        {
                           LOG(ERROR)<<sPrintEvent<<FairLogger::endl;
                           sPrintEvent = "";
                        } // if( 0 < uWdInd && 0 == uWdInd%8 )
                        sPrintEvent += Form("%08x ", data[uWdInd]);
                     } // for( UInt_t uWdInd = 0; uWdInd < uNbWords; uWdInd++ )
                     LOG(ERROR)<<sPrintEvent<<FairLogger::endl;
                     LOG(ERROR)<<"----------------------------------------------------------------"<<FairLogger::endl;
                     // jump this tag
                     pData+=2; 
                     uCurrWord+=1;
                  } // else of if (iTagPos[uBoard]==0) 

               bFound = kTRUE;
               break;
            } // if ((*plTag == fMbsUnpackPar->GetBoardTag( uBoard )) && (fMbsUnpackPar->GetBoardTag( uBoard )!=0))

      if( kFALSE == bFound ) 
         pData++;
   } // for (UInt_t uCurrWord = 0; uCurrWord < uNbWords; ++uCurrWord) 

   // if t east one TAG found: end of corresponding data is end of subevent
   if (iLastTag>=0)
      iTagLen[iLastTag] = pData - iTagPos[iLastTag];

   // Actually process data in different Unpackers:
   
   for( UInt_t uBoard=0; uBoard < fMbsUnpackPar->GetBoardsNumber(); uBoard++ )
      if( NULL != iTagPos[uBoard] )
      {
         LOG(DEBUG)<<"TMbsUnpackTof::ProcessSubevent => TAG "<<uBoard
                   <<" POS "<<iTagPos[uBoard] - data
                   <<" LEN "<<iTagLen[uBoard]
                   <<" TYP "<<fMbsUnpackPar->GetBoardType( uBoard )<<FairLogger::endl;
         if( 0 < iTagLen[uBoard])
            switch( fMbsUnpackPar->GetBoardType( uBoard ) )
            {
               case tofMbs::caenV1290:
                  break;
               case tofMbs::vftx:
                  fVftxUnp->ProcessVFTX( fMbsUnpackPar->GetGlobalToActiveInd( uBoard ), 
                                         (UInt_t*) iTagPos[uBoard], iTagLen[uBoard]); 
                  break;
               case tofMbs::get4:
                  break;
               case tofMbs::triglog:
                  break;
               case tofMbs::scalormu:
                  fScomUnp->ProcessScom( fMbsUnpackPar->GetGlobalToActiveInd( uBoard ), 
                                         (UInt_t*) iTagPos[uBoard], iTagLen[uBoard]);  // Scaler Or Multiplicity
                  break;
               case tofMbs::scaler2014:
                  fScal2014Unp->ProcessScal2014( fMbsUnpackPar->GetGlobalToActiveInd( uBoard ),
                                         (UInt_t*) iTagPos[uBoard], iTagLen[uBoard]);  // Scalers 2014
                  break;
               case tofMbs::triglogscal:
                  fTrloScalUnp->ProcessTriglogScal( fMbsUnpackPar->GetGlobalToActiveInd( uBoard ),
                                         (UInt_t*) iTagPos[uBoard], iTagLen[uBoard]);  // Triglog used as scalers
                  break;
               case tofMbs::undef:
               default:
                  LOG(WARNING)<<"TMbsUnpackTof::ProcessSubevent => Unknown board type "
                              <<fMbsUnpackPar->GetBoardType( uBoard )
                              <<"!!!!" <<FairLogger::endl;
            } // switch( fMbsUnpackPar->GetBoardType( uBoard ) )
      } // if( NULL != iTagPos[uBoard] )
/*
   if (iTagPos[VME__ID_1182] && iTagLen[VME__ID_1182])
      Process1182(0, iTagPos[VME__ID_1182], iTagLen[VME__ID_1182]);  // 1182
   if (iTagPos[VME__ID_1183] && iTagLen[VME__ID_1183])
      Process1182(1, iTagPos[VME__ID_1183], iTagLen[VME__ID_1183]);  // 1183
   for (int tdc=VME__ID_TDC1;tdc<=VME__ID_TDC8;tdc++)
      if (iTagPos[tdc] && iTagLen[tdc])
         Process1290(tdc-VME__ID_TDC1, iTagPos[tdc], iTagLen[tdc]);  // all TDCs
   for (int qdc=VME__ID_QDC1;qdc<=VME__ID_QDC2;qdc++)
      if (iTagPos[qdc] && iTagLen[qdc])
         Process965(qdc-VME__ID_QDC1, iTagPos[qdc], iTagLen[qdc]);  // all QDCs
   if (iTagPos[VME__ID_QFW] && iTagLen[VME__ID_QFW])
      ProcessQFW((uint32_t*)iTagPos[VME__ID_QFW], iTagLen[VME__ID_QFW]);  // QFW
*/
   fiNbEvents++;
   LOG(DEBUG)<<"MBS unpacked!"<<FairLogger::endl;
   return kTRUE;
}
void TMbsUnpackTof::Reset()
{
   ClearOutput();
}
void TMbsUnpackTof::Register()
{
   RegisterOutput();
}
Bool_t TMbsUnpackTof::Finish()
{   
//   fScomUnp->FinishScom();
   return kTRUE;
}

// ------------------------------------------------------------------
Bool_t TMbsUnpackTof::InitParameters()
{
   // Get Base Container
   FairRun* ana = FairRun::Instance();
   FairRuntimeDb* rtdb=ana->GetRuntimeDb();

   // Unpacker parameter
   fMbsUnpackPar = (TMbsUnpackTofPar*) (rtdb->getContainer("TMbsUnpackTofPar"));
   if( 0 == fMbsUnpackPar )
      return kFALSE;

//   rtdb->initContainers(  ana->GetRunId() );
   fMbsUnpackPar->printParams();
     
   return kTRUE;
} 
// ------------------------------------------------------------------
Bool_t TMbsUnpackTof::CreateHistogramms()
{
   TDirectory * oldir = gDirectory; // <= To prevent histos from being sucked in by the param file of the TRootManager!
   gROOT->cd(); // <= To prevent histos from being sucked in by the param file of the TRootManager!

   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::scaler2014 ) )
      fScal2014Unp->CreateHistos();
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::scalormu ) )
      fScomUnp->CreateHistos();
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::triglogscal ) )
      fTrloScalUnp->CreateHistos();
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::vftx ) )
      fVftxUnp->CreateHistos();
   
   gDirectory->cd( oldir->GetPath() ); // <= To prevent histos from being sucked in by the param file of the TRootManager!
   
   return kTRUE;
}
Bool_t TMbsUnpackTof::FillHistograms()
{
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::scaler2014 ) )
      fScal2014Unp->FillHistos();
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::scalormu ) )
      fScomUnp->FillHistos();
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::triglogscal ) )
      fTrloScalUnp->FillHistos();
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::vftx ) )
      fVftxUnp->FillHistos();
   
   return kTRUE;
}
void TMbsUnpackTof::WriteHistogramms()
{
   TDirectory * oldir = gDirectory;
   TFile *fHist;
   
   if( 1 == fMbsUnpackPar->GetNbActiveBoards( tofMbs::triglog ) )
      fHist = new TFile("./tofMbsUnp.hst.root","UPDATE");
      else fHist = new TFile("./tofMbsUnp.hst.root","RECREATE");

   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::scaler2014 ) )
      fScal2014Unp->WriteHistos( fHist );
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::scalormu ) )
      fScomUnp->WriteHistos( fHist );
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::triglogscal ) )
      fTrloScalUnp->WriteHistos( fHist );
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::vftx ) )
      fVftxUnp->WriteHistos( fHist );
      
   gDirectory->cd( oldir->GetPath() );
   fHist->Close();
}
void TMbsUnpackTof::DeleteHistograms()
{
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::scaler2014 ) )
      fScal2014Unp->DeleteHistos();
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::scalormu ) )
      fScomUnp->DeleteHistos();
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::triglogscal ) )
      fTrloScalUnp->DeleteHistos();
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::vftx ) )
      fVftxUnp->DeleteHistos();
}
// ------------------------------------------------------------------
Bool_t TMbsUnpackTof::CreateUnpackers()
{
   if( 0 == fMbsUnpackPar->GetNbActiveBoards() )
   {
      LOG(ERROR)<<"TMbsUnpackTof::CreateUnpackers => No active boards => No unpacker !?!?! "<<FairLogger::endl;
      return kFALSE;
   }

   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::scaler2014 ) )
      fScal2014Unp = new TTofScal2014Unpacker( fMbsUnpackPar );
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::scalormu ) )
      fScomUnp     = new TTofScomUnpacker( fMbsUnpackPar );
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::triglogscal ) )
      fTrloScalUnp = new TTofTriglogScalUnpacker( fMbsUnpackPar );
/*
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::caenV1290 ) )
      fV1290Unp = new TTofV1290Unpacker( fMbsUnpackPar );
*/
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::vftx ) )
      fVftxUnp = new TTofVftxUnpacker( fMbsUnpackPar );
/*
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::get4 ) )
      fGet4Unp = new TTofGet4Unpacker( fMbsUnpackPar );
*/      
   return kTRUE;
}
Bool_t TMbsUnpackTof::RegisterOutput()
{
   if( 0 == fMbsUnpackPar->GetNbActiveBoards() )
   {
      LOG(ERROR)<<"TMbsUnpackTof::RegisterOutput => No active boards => No output objects !?!?! "<<FairLogger::endl;
      return kFALSE;
   }
   // Not sure here what is the best way to group the TDC objects......
   // The vector of data in the board objects kill probably the TClonesArray purpose?
   // Maybe better to have big fixed size array and TTofVftxData::Clear calls in TTofVftxBoard combined
   // with ConstructedAt access in TTofVftxUnpacker
   FairRootManager *fManager = FairRootManager::Instance();
      // Scalers
         // TRIGLOG + Scalormu + Scaler2014
   if( 0 < fMbsUnpackPar->GetNbActiveScalersB() && 1 != fMbsUnpackPar->GetNbActiveBoards( tofMbs::triglog ) ) 
   {
      fScalerBoardCollection = new TClonesArray( "TTofScalerBoard", 
                                                  fMbsUnpackPar->GetNbActiveScalersB());
//      fManager->Register("TofRawScalers","TofUnpack",fScalerBoardCollection, kTRUE);
      fManager->Register( "TofRawScalers","TofUnpack",fScalerBoardCollection,
                          fMbsUnpackPar->WriteDataInCbmOut() || fbSaveScalers );
   } // if( 0 < fMbsUnpackPar->GetNbActiveScalersB() )
      //TDC
         // VFTX
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::vftx ) )
   {
      fVftxBoardCollection = new TClonesArray( "TTofVftxBoard", 
                                               fMbsUnpackPar->GetNbActiveBoards( tofMbs::vftx ));
//      fManager->Register("TofVftxTdc","TofUnpack",fVftxBoardCollection, kTRUE);
      fManager->Register( "TofVftxTdc","TofUnpack",fVftxBoardCollection,
                          fMbsUnpackPar->WriteDataInCbmOut() || fbSaveRawVftx );
   } // if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::vftx ) )
         // Caen v1290 TDC
         // VFTX
         // TRB3

   return kTRUE;
}
Bool_t TMbsUnpackTof::ClearOutput()
{
   LOG(DEBUG)<<"Clear MBS"<<FairLogger::endl;
   if( 0 < fMbsUnpackPar->GetNbActiveScalersB() && 1 != fMbsUnpackPar->GetNbActiveBoards( tofMbs::triglog ) )
      fScalerBoardCollection->Clear("C");
   if( 0 < fMbsUnpackPar->GetNbActiveBoards( tofMbs::vftx ) ) 
      fVftxBoardCollection->Clear("C");
   return kTRUE;
}
void TMbsUnpackTof::SetSaveScalers( Bool_t bSaveScal )
{
   fbSaveScalers = bSaveScal;
   LOG(INFO)<<"TMbsUnpackTof => Enable the saving of raw scaler data in analysis output file"
            <<FairLogger::endl;

}
void TMbsUnpackTof::SetSaveRawVftx( Bool_t bSaveVftx )
{
   fbSaveRawVftx = bSaveVftx;
   LOG(INFO)<<"TMbsUnpackTof => Enable the saving of raw VFTX data in analysis output file"
            <<FairLogger::endl;

}
// ------------------------------------------------------------------
