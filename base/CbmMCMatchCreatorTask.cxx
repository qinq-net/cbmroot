// -------------------------------------------------------------------------
// -----                CbmMCMatchCreatorTask source file             -----
// -----                  Created 18/07/08  by T.Stockmanns        -----
// -------------------------------------------------------------------------
#include "CbmMCMatchCreatorTask.h"

#include "CbmDetectorList.h"
#include "CbmMCEntry.h"
#include "CbmMCMatch.h"

// framework includes
#include "FairRootManager.h"
#include "FairMultiLinkedData.h"
#include "FairLogger.h"

// Root includes
#include "TClonesArray.h"

// -----   Default constructor   -------------------------------------------
CbmMCMatchCreatorTask::CbmMCMatchCreatorTask() 
  : FairTask("Creates CbmMCMatch"), 
    fBranches(),
    fMCLink(NULL),
    fEventNr(0),
    fMCMatch(NULL)
{
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmMCMatchCreatorTask::~CbmMCMatchCreatorTask()
{
}

// -----   Public method Init   --------------------------------------------
InitStatus CbmMCMatchCreatorTask::Init()
{


//  fMCMatch->InitStage(kMCTrack, "", "MCTrack");

	fMCMatch = new CbmMCMatch("CbmMCMatch", "CbmMCMatch");

  fMCMatch->InitStage(Cbm::kStsPoint, "", "StsPoint");
  fMCMatch->InitStage(Cbm::kStsDigi, "", "StsDigi");
  fMCMatch->InitStage(Cbm::kStsCluster, "", "StsCluster");
  fMCMatch->InitStage(Cbm::kStsHit, "", "StsHit");

  InitStatus status = InitBranches();


  FairRootManager* ioman = FairRootManager::Instance();
  	if (!ioman) {
  		LOG(FATAL) << "RootManager not instantiated!" << FairLogger::endl;
  	}

	fMCLink = new TClonesArray("CbmMCEntry");
	ioman->Register("MCLink", "MCInfo", fMCLink, IsOutputBranchPersistent("MCLink"));

	ioman->Register("MCMatch", "MCMatch", fMCMatch, kFALSE);

	LOG(INFO) << "CbmMCMatchCreatorTask::Init: Initialization successfull" << FairLogger::endl;


  return status;
}

InitStatus CbmMCMatchCreatorTask::InitBranches()
{

	 // Get RootManager
	FairRootManager* ioman = FairRootManager::Instance();
	if (!ioman) {
		LOG(FATAL) << "RootManager not instantiated!" << FairLogger::endl;
	}

	int NStages = fMCMatch->GetNMCStages();
	for (int i = NStages-1; i > -1; i--){
		TClonesArray* myBranch = (TClonesArray*)ioman->GetObject(fMCMatch->GetMCStage(i)->GetBranchName().c_str());
		if (!myBranch)	{
			//LOG(INFO) << "NMCStages: " << fMCMatch->GetNMCStages() << FairLogger::endl;
			LOG(WARNING) << "CbmMCMatchCreatorTask::Init: "<< "No "<<fMCMatch->GetMCStage(i)->GetBranchName() << " array!" << FairLogger::endl;
			fMCMatch->GetMCStage(i)->SetFill(kFALSE); //RemoveStage(fMCMatch->GetMCStage(i)->GetStageId());

			continue;
		}
		else fMCMatch->GetMCStage(i)->SetFill(kTRUE);
		fBranches[fMCMatch->GetMCStage(i)->GetBranchName()] = myBranch;
	}
	return kSUCCESS;
}
// -------------------------------------------------------------------------
void CbmMCMatchCreatorTask::SetParContainers()
{
  // Get Base Container
//  FairRun* ana = FairRun::Instance();
//  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

}


// -----   Public method Exec   --------------------------------------------
void CbmMCMatchCreatorTask::Exec(Option_t* /*opt*/)
{

	if (!fMCLink) Fatal("Exec", "No fMCLinkDet");
	fMCLink->Delete();
	fMCMatch->ClearMCList();

	fMCMatch->LoadInMCLists(fMCLink);

	LOG(INFO) << "NMCStages: " << fMCMatch->GetNMCStages() << FairLogger::endl;
	for (int i = 0; i < fMCMatch->GetNMCStages(); i++){
		if (fMCMatch->GetMCStage(i)->GetFill() == kTRUE && fMCMatch->GetMCStage(i)->GetLoaded() == kFALSE){
			LOG(INFO) << i << ": ";
			LOG(INFO) << "BranchName: " << fMCMatch->GetMCStage(i)->GetBranchName() << FairLogger::endl;
			TClonesArray* clArray = fBranches[fMCMatch->GetMCStage(i)->GetBranchName()];
			for (int j = 0; j < clArray->GetEntries(); j++){
				FairMultiLinkedData* myData = (FairMultiLinkedData*)clArray->At(j);
				fMCMatch->SetElements(fMCMatch->GetMCStage(i)->GetStageId(), j, myData);
			}
			if (fMCMatch->GetMCStage(i)->GetNEntries() > 0)
				fMCMatch->GetMCStage(i)->SetLoaded(kTRUE);
		}
	}

	int i = 0;
	for (int index = 0; index < fMCMatch->GetNMCStages(); index++){
		CbmMCStage myStage(*(fMCMatch->GetMCStage(index)));

		for (int indStage = 0; indStage < myStage.GetNEntries(); indStage++){

			CbmMCEntry myLink(myStage.GetMCLink(indStage));
			//LOG(INFO) << "myLink: " << myStage.GetMCLink(indStage).GetSource() << "/" << myStage.GetMCLink(indStage).GetPos() << FairLogger::endl;
			new((*fMCLink)[i]) CbmMCEntry(myLink.GetLinks(), myLink.GetSource(), myLink.GetPos());
			i++;
		}
	}

	if (fVerbose > 0){
		fMCMatch->Print();
		LOG(INFO) << FairLogger::endl;
	}
}

void CbmMCMatchCreatorTask::Finish()
{
}


ClassImp(CbmMCMatchCreatorTask);
