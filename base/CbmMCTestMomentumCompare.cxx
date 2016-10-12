// -------------------------------------------------------------------------
// -----                CbmMCTestMomentumCompare source file             -----
// -----                  Created 18/07/08  by T.Stockmanns        -----
// -------------------------------------------------------------------------
// libc includes

// Root includes
#include "TROOT.h"
#include "TClonesArray.h"

// framework includes
#include "FairRootManager.h"
#include "CbmMCTestMomentumCompare.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairHit.h"
#include "FairMultiLinkedData.h"
#include "FairLogger.h"
#include "CbmMCTrack.h"
#include "CbmTrack.h"
#include "CbmMCEntry.h"


// -----   Default constructor   -------------------------------------------
CbmMCTestMomentumCompare::CbmMCTestMomentumCompare()
	: FairTask("Creates CbmMC test")
{
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmMCTestMomentumCompare::~CbmMCTestMomentumCompare()
{
}

// -----   Public method Init   --------------------------------------------
InitStatus CbmMCTestMomentumCompare::Init()
{


//  fMCMatch->InitStage(kMCTrack, "", "MCTrack");



  FairRootManager* ioman = FairRootManager::Instance();
  	if (!ioman) {
  	  LOG(FATAL) << "RootManager not instantiated!" << FairLogger::endl;
  	}

  	fMCMatch = (CbmMCMatch*)ioman->GetObject("MCMatch");

  	fTrack = (TClonesArray*)ioman->GetObject("LheGenTrack");
  	fMCTrack = (TClonesArray*)ioman->GetObject("MCTrack");

	LOG(INFO)<< "CbmMCTestMomentumCompare: Initialization successfull" << FairLogger::endl;


  return kSUCCESS;
}


// -------------------------------------------------------------------------
void CbmMCTestMomentumCompare::SetParContainers()
{
  // Get Base Container
//  FairRun* ana = FairRun::Instance();
//  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

}


// -----   Public method Exec   --------------------------------------------
void CbmMCTestMomentumCompare::Exec(Option_t* /*opt*/)
{
// 	//fMCMatch->CreateArtificialStage(kMCTrack, "", "");
// 
// 	CbmMCResult myResult = fMCMatch->GetMCInfo(kTrack, kMCTrack);
// 	LOG(INFO) << myResult;
// 	for (int i = 0; i < myResult.GetNEntries(); i++){
// 		CbmMCEntry myLinks = myResult.GetMCLink(i);
// 		CbmTrack* myTrack = (CbmTrack*)fTrack->At(i);
// 		LOG(INFO) << "TrackMatch for Track " << i << FairLogger::endl;
// 		//myTrack->Print();
// 		LOG(INFO) << "P: " << myTrack->GetParamFirst().GetSDMomentum().Mag() << FairLogger::endl;
// 		LOG(INFO) << "Belongs to: " << FairLogger::endl;
// 		for (int j = 0; j < myLinks.GetNLinks(); j++){
// 			if (myLinks.GetLink(j).GetType() == kMCTrack){
// 				LOG(INFO) << "MCTrack " << myLinks.GetLink(j).GetIndex() << FairLogger::endl;
// 				CbmMCTrack* myMCTrack = (CbmMCTrack*)fMCTrack->At(myLinks.GetLink(j).GetIndex());
// 				//myMCTrack->Print(myLinks.GetFairLink(j).GetIndex());
// 				LOG(INFO) << "P: " << myMCTrack->GetMomentum().Mag() << " PID: " << myMCTrack->GetPdgCode() << FairLogger::endl;
// 				LOG(INFO) << "--------------------------------" << FairLogger::endl;
// 			}
// 		}
// 		LOG(INFO) << FairLogger::endl;
// 	}
}

void CbmMCTestMomentumCompare::Finish()
{
}


ClassImp(CbmMCTestMomentumCompare);
