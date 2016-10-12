// -------------------------------------------------------------------------
// -----                CbmMCMatchLoaderTask source file             -----
// -----                  Created 18/07/08  by T.Stockmanns        -----
// -------------------------------------------------------------------------
#include "CbmMCMatchLoaderTask.h"

#include "CbmMCMatch.h"
#include "CbmDetectorList.h"

// framework includes
#include "FairRootManager.h"
#include "FairLogger.h"

// Root includes
#include "TClonesArray.h"

// -----   Default constructor   -------------------------------------------
CbmMCMatchLoaderTask::CbmMCMatchLoaderTask() 
  : FairTask("Creates CbmMCMatch"), 
    fMCLink(NULL),
    fEventNr(0),
    fMCMatch(NULL)
{
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmMCMatchLoaderTask::~CbmMCMatchLoaderTask()
{
}

// -----   Public method Init   --------------------------------------------
InitStatus CbmMCMatchLoaderTask::Init()
{
	fMCMatch = new CbmMCMatch("CbmMCMatch", "CbmMCMatch");

//  fMCMatch->InitStage(kMCTrack, "", "MCTrack");
  fMCMatch->InitStage(Cbm::kStsPoint, "", "StsPoint");
  fMCMatch->InitStage(Cbm::kStsDigi, "", "StsDigi");
  fMCMatch->InitStage(Cbm::kStsCluster, "", "StsCluster");
  fMCMatch->InitStage(Cbm::kStsHit, "", "StsHit");


  FairRootManager* ioman = FairRootManager::Instance();
  	if (!ioman) {
  		LOG(FATAL) << "RootManager not instantiated!" << FairLogger::endl;
  	}

	fMCLink = (TClonesArray*)ioman->GetObject("MCLink");
	ioman->Register("MCMatch", "MCMatch", fMCMatch, kFALSE);

  return kSUCCESS;
}


// -------------------------------------------------------------------------
void CbmMCMatchLoaderTask::SetParContainers()
{
  // Get Base Container
//  FairRun* ana = FairRun::Instance();
//  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

}


// -----   Public method Exec   --------------------------------------------
void CbmMCMatchLoaderTask::Exec(Option_t* /*opt*/)
{
/*	for (int trackIndex = 0; trackIndex < fTrack->GetEntriesFast(); trackIndex++){
		CbmTrack* myTrack = (CbmTrack*)fTrack->At(trackIndex);
		fMCMatch->AddElement(kTrack, trackIndex, kTrackCand, myTrack->GetRefIndex());
	}
*/

	if (!fMCLink) Fatal("Exec", "No fMCLink");
//	fMCLinkDet->Delete();
//	fMCLinkHit->Delete();
	fMCMatch->ClearMCList();

	fMCMatch->LoadInMCLists(fMCLink);
	fMCMatch->CreateArtificialStage(Cbm::kMCTrack, "", "");

	fMCMatch->Print();
	LOG(INFO) << FairLogger::endl;
}

void CbmMCMatchLoaderTask::Finish()
{
}


ClassImp(CbmMCMatchLoaderTask);
