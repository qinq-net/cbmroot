// -------------------------------------------------------------------------
// -----                CbmMCMatchSelectorTask source file             -----
// -----                  Created 18/07/08  by T.Stockmanns        -----
// -------------------------------------------------------------------------
#include "CbmMCMatchSelectorTask.h"

#include "CbmMCMatch.h"

// framework includes
#include "FairRootManager.h"
#include "FairLogger.h"

// -----   Default constructor   -------------------------------------------
CbmMCMatchSelectorTask::CbmMCMatchSelectorTask()
  : FairTask("Creates CbmMCMatch"), 
    fMCMatch(NULL),
    fStart(Cbm::kUnknown), 
    fStop(Cbm::kUnknown),
    fStageWeights(),
    fCommonWeight(0.)
{
}
// -------------------------------------------------------------------------

CbmMCMatchSelectorTask::CbmMCMatchSelectorTask(Cbm::DataType start, Cbm::DataType stop)
  : FairTask("Creates CbmMCMatch"), 
    fMCMatch(NULL),
    fStart(start), 
    fStop(stop),
    fStageWeights(),
    fCommonWeight(0.)
{
}

// -----   Destructor   ----------------------------------------------------
CbmMCMatchSelectorTask::~CbmMCMatchSelectorTask()
{
}

// -----   Public method Init   --------------------------------------------
InitStatus CbmMCMatchSelectorTask::Init()
{


//  fMCMatch->InitStage(kMCTrack, "", "MCTrack");



  FairRootManager* ioman = FairRootManager::Instance();
  	if (!ioman) {
  		LOG(FATAL) << "RootManager not instantiated!" << FairLogger::endl;
  	}

  	fMCMatch = (CbmMCMatch*)ioman->GetObject("MCMatch");

	LOG(INFO) << "CbmMCMatchSelectorTask::Init: Initialization successfull" << FairLogger::endl;


  return kSUCCESS;
}


// -------------------------------------------------------------------------
void CbmMCMatchSelectorTask::SetParContainers()
{
  // Get Base Container
//  FairRun* ana = FairRun::Instance();
//  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

}


// -----   Public method Exec   --------------------------------------------
void CbmMCMatchSelectorTask::Exec(Option_t* /*opt*/)
{
	LOG(INFO) << "Output Selector: " << FairLogger::endl;
	SetWeights();
	LOG(INFO) << fMCMatch->GetMCInfo(fStart, fStop);
}

void CbmMCMatchSelectorTask::SetWeights()
{
//	LOG(INFO) << "SetWeights: CommonWeight " << fCommonWeight << " NStageWeights " << fStageWeights.size() << FairLogger::endl;
	fMCMatch->SetCommonWeightStages(fCommonWeight);
	for (unsigned int i = 0; i < fStageWeights.size();i++){
		fMCMatch->GetMCStageType(fStageWeights[i].first)->SetWeight(fStageWeights[i].second);
	}
}

void CbmMCMatchSelectorTask::Finish()
{
}


ClassImp(CbmMCMatchSelectorTask);
