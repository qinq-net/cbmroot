#include "CbmRichTrbRecoQa.h"
#include "FairRootManager.h"
#include "FairLogger.h"

#include "TClonesArray.h"

#include "TH1D.h"
#include "TCanvas.h"

#include <iostream>
#include <vector>

CbmRichTrbRecoQa::CbmRichTrbRecoQa():
	fEventNum(0)
{
	;
}

CbmRichTrbRecoQa::~CbmRichTrbRecoQa()
{
	;
}


InitStatus CbmRichTrbRecoQa::Init()
{
	FairRootManager* manager = FairRootManager::Instance();
	fRichHits = (TClonesArray*)manager->GetObject("RichHit");
	if (NULL == fRichHits) { Fatal("CbmRichTrbRawQa::Init","No RichHit array!"); }
}

void CbmRichTrbRecoQa::Exec(
		Option_t* option)
{
	fEventNum++;
	LOG(INFO) << "Event #" << fEventNum << FairLogger::endl;

	LOG(INFO) << "nof hits in event = " << fRichHits->GetEntries() << FairLogger::endl;

}

void CbmRichTrbRecoQa::Finish()
{

}

ClassImp(CbmRichTrbRecoQa)
