/** CbmRunAna.cxx
 *@author Volker Friese v.friese@gsi.de
 *@since 10.12.2009
 *@version 1.0
 **/


#include "CbmRunAna.h"

#include "FairEventHeader.h"
#include "FairLogger.h"
#include "FairMCEventHeader.h"
#include "FairTask.h"
#include "FairTrajFilter.h"

#include <iostream>

using std::cout;
using std::endl;



// -----   Constructor   -----------------------------------------------------
CbmRunAna::CbmRunAna() 
  : FairRunAna(),
    fAsync(kFALSE),
    fMarkFill(kFALSE),
    fRunInfo()
{
}
// ---------------------------------------------------------------------------



// -----   Destructor   ------------------------------------------------------
CbmRunAna::~CbmRunAna() {
}
// ---------------------------------------------------------------------------




// -----   Run execution   ---------------------------------------------------
void CbmRunAna::ExecRun(Int_t iStart, Int_t iStop) {

	// This is basically a copy of FairRunAna::Run(Int_t, Int_t), with the
	// difference that in asynchronous mode, FairRootManager::Fill() is not
	// called at the end of each input event, but only if fMarkFill is set.
	// Not a good solution for the asynchronous I/O but, hey, what can a poor
	// programmer do?

  // --- Check and, if necessary, adjust event range
  Int_t nEvents = fRootManager->GetInChain()->GetEntries();

  if ( iStart >= nEvents ) {
  	LOG(ERROR) << "CbmRunAna: First event requested is " << iStart
  			       << ", but last event in input is " << nEvents - 1
  			       << "; no run execution." << FairLogger::endl;
    return;
  }

  if ( iStop > nEvents - 1 ) {
  	LOG(WARNING) << "CbmRunAna: Last event requested is " << iStop
  			         << ", but last event in input is " << nEvents - 1
  			         << FairLogger::endl;
    iStop = nEvents - 1;
    LOG(INFO) << "CbmRunAna: Last event is set to " << iStop << FairLogger::endl;
   }

  if ( iStart < 0 ) iStart = 0;
  LOG(INFO) << "CbmRunAna: Processing events " << iStart << " to " << iStop
  		      << FairLogger::endl;


  // --- Bail out if input file is not open
  if ( ! fInFileIsOpen ) {
  	LOG(ERROR) << "CbmRunAna: Input file is not open; no run execution."
  			       << FairLogger::endl;
  	return;
  }


  // --- Reset run info if the latter is requested
  if ( fGenerateRunInfo ) fRunInfo.Reset();


  // --- Event loop
  for (Int_t iEvent = iStart; iEvent <= iStop; iEvent++) {

  	// Read in one event
  	fRootManager->ReadEvent(iEvent);

  	// Get the run ID from the event header
  	UInt_t runId = 0;
  	if ( fRootManager->IsEvtHeaderNew() ) runId = fMCHeader->GetRunID();
  	else                                  runId = fEvtHeader->GetRunId();

  	// Re-initialise if run ID has changed
  	if ( runId != fRunId ) {
  		fRunId = runId;
  		if ( ! fStatic ) {
  			Reinit( fRunId );
  			fTask->ReInitTask();
  		}
  	}

  	// Store write-out buffer data
  	fRootManager->StoreWriteoutBufferData(fRootManager->GetEventTime());

    // Execute master task
    fTask->ExecuteTask("");

    // In normal mode: mark output tree for filling
    if ( ! fAsync ) MarkFill();

    // Fill data tree
    Fill();

    // Delete write-out buffer data
    fRootManager->DeleteOldWriteoutBufferData();

    // Call finish event methods of the tasks
    fTask->FinishEvent();

    // Generate run info if requested
    if ( fGenerateRunInfo ) fRunInfo.StoreInfo();

    // Reset trajectory filter if present
    if ( FairTrajFilter::Instance() ) FairTrajFilter::Instance()->Reset();

  } //# event loop

  // --- End of run action
  fRootManager->StoreAllWriteoutBufferData();
  fTask->FinishTask();
  if ( fGenerateRunInfo ) fRunInfo.WriteInfo();
  fRootManager->LastFill();
  fRootManager->Write();

}
// ---------------------------------------------------------------------------



// -----   Fill output tree   ------------------------------------------------
void CbmRunAna::Fill() {

	if ( fAsync && ! fMarkFill ) return;
	fRootManager->Fill();

}
// ---------------------------------------------------------------------------


    
ClassImp(CbmRunAna)
