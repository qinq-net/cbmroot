/** CbmRunOnline.cxx
 *@author Volker Friese v.friese@gsi.de
 *@since 10.12.2009
 *@version 1.0
 **/


#include "CbmRunOnline.h"

#include "FairEventHeader.h"
#include "FairTask.h"
#include "FairSource.h"

#include "TROOT.h"
#include "TSystem.h"

#include <signal.h>
#include <stdlib.h>
#include <iostream>
#include <list>

using std::cout;
using std::endl;
using std::list;

/*
#include "FairRunOnline.h"
#include "FairRootManager.h"
#include "FairTask.h"
#include "FairBaseParSet.h"
#include "FairEventHeader.h"
#include "FairFieldFactory.h"
#include "FairRuntimeDb.h"
#include "FairRunIdGenerator.h"
#include "FairLogger.h"
#include "FairFileHeader.h"
#include "FairParIo.h"
#include "FairField.h"
#include "FairGeoInterface.h"
#include "FairGeoLoader.h"
#include "FairGeoParSet.h"

#include "TTree.h"
#include "TSeqCollection.h"
#include "TGeoManager.h"
#include "TKey.h"
#include "TF1.h"
#include "TSystem.h"
#include "TFolder.h"
#include "TH1F.h"
#include "TH2F.h"

#include <iostream>

using std::cout;
using std::endl;
*/

//_____________________________________________________________________________
//  From base/steer/FairRunOnline.cxx, special functions
Bool_t gIsInterrupted;
void handler_ctrlc(int s)
{
   gIsInterrupted = kTRUE;
}
//_____________________________________________________________________________

// -----   Constructor   -----------------------------------------------------
CbmRunOnline::CbmRunOnline()
  : FairRunOnline(),
    fAsync(kFALSE),
    fMarkFill(kFALSE)
{
}
// ---------------------------------------------------------------------------



// -----   Destructor   ------------------------------------------------------
CbmRunOnline::~CbmRunOnline() {
}
// ---------------------------------------------------------------------------

// -----   Even tLoop   ---------------------------------------------------
Int_t CbmRunOnline::EventLoop()
{
   gSystem->IgnoreInterrupt();
   gIsInterrupted = kFALSE;
   signal(SIGINT, handler_ctrlc);
   fSource->Reset();

   Int_t status = fSource->ReadEvent();
   if(1 == status || 2 == status)
   {
      return status;
   } // if(1 == status || 2 == status)

   Int_t tmpId = GetEventHeader()->GetRunId();
   if ( tmpId != fRunId )
   {
   // LOG(INFO) << "Call Reinit due to changed RunID" << FairLogger::endl;
   // fRunId = tmpId;
   // Reinit( fRunId );
   // fTask->ReInitTask();
   } // if ( tmpId != fRunId )

   // To be done in one of the tasks!
//   fRootManager->StoreWriteoutBufferData(fRootManager->GetEventTime());

   fTask->ExecuteTask("");

   // To be done in one of the tasks!
//   fRootManager->Fill();
//   fRootManager->DeleteOldWriteoutBufferData();

   // In normal mode: mark output tree for filling
   if ( ! fAsync ) MarkFill();

   // If output marked for filling: do so
   if ( fMarkFill ) {
     fRootManager->Fill();
     fTask->FinishEvent();
   }

   fNevents += 1;
   if(fGenerateHtml && 0 == (fNevents%fRefreshRate))
   {
      WriteObjects();
      GenerateHtml();
   } // if(fGenerateHtml && 0 == (fNevents%fRefreshRate))
   if(gIsInterrupted)
   {
      return 1;
   } // if(gIsInterrupted)

   return 0;
}
// ---------------------------------------------------------------------------

// -----   Run execution   ---------------------------------------------------
void CbmRunOnline::ExecRun(Int_t iNbEvt ) {

   fOutFile->cd();
   fNevents = 0;
   Int_t status;
   if(iNbEvt < 0)
   {
      while(kTRUE)
      {
         status = EventLoop();
         if(1 == status)
         {
            break;
         } // if(1 == status)
         else if(2 == status)
         {
            continue;
         } // else if(2 == status)
         if(gIsInterrupted)
         {
            break;
         } // if(gIsInterrupted)
      } // while(kTRUE)
   } // if(iNbEvt < 0)
      else
      {
         for (Int_t i = 0; i < iNbEvt; i++)
         {
            status = EventLoop();
            if(1 == status)
            {
               break;
            } // if(1 == status)
            else if(2 == status)
            {
               i -= 1;
               continue;
            } // else if(2 == status)
            if(gIsInterrupted)
            {
               break;
            } // if(gIsInterrupted)
         } // for (Int_t i = 0; i < iNbEvt; i++)
      } // else of if(iNbEvt < 0)

   // In normal mode: mark output tree for filling
   if ( ! fAsync ) MarkFill();

   // If output marked for filling: do so
   if ( fMarkFill )
   {
      fRootManager->StoreAllWriteoutBufferData();
   } // if ( fMarkFill )

   /*
    * Not possible to use it due to private member in FairRunOnline & not getter!!
   if (fAutomaticFinish)
   {
      Finish();
   } // if (fAutomaticFinish)
   */
}
// ---------------------------------------------------------------------------



ClassImp(CbmRunOnline)
