/**
* \file CbmRichHitProducer.cxx
*
* \author B. Polichtchouk
* \date 2004
**/

#include "CbmRichHitProducer.h"

#include "CbmRichPoint.h"
#include "CbmRichHit.h"
#include "CbmRichDigi.h"
#include "TClonesArray.h"
#include "CbmRichGeoManager.h"
#include "CbmRichDigiMapManager.h"
#include "FairLogger.h"


#include <iostream>

using namespace std;



CbmRichHitProducer::CbmRichHitProducer():
   FairTask("CbmRichHitProducer"),
   fRichDigis(NULL),
   fRichHits(NULL),
   fEventNum(0)
{

}

CbmRichHitProducer::~CbmRichHitProducer()
{
   FairRootManager *manager =FairRootManager::Instance();
   manager->Write();
}

void CbmRichHitProducer::SetParContainers()
{
}

InitStatus CbmRichHitProducer::Init()
{
   FairRootManager* manager = FairRootManager::Instance();

   fRichDigis = (TClonesArray*)manager->GetObject("RichDigi");
   if (NULL == fRichDigis) { Fatal("CbmRichHitProducer::Init","No RichDigi array!"); }

   fRichHits = new TClonesArray("CbmRichHit");
   manager->Register("RichHit","RICH", fRichHits, kTRUE);

   return kSUCCESS;
}

void CbmRichHitProducer::Exec(
      Option_t* option)
{
	fEventNum++;
	LOG(INFO) << "CbmRichHitProducer event " << fEventNum << FairLogger::endl;

	fRichHits->Delete();

	for(Int_t j = 0; j < fRichDigis->GetEntries(); j++){
		CbmRichDigi* digi = (CbmRichDigi*) fRichDigis->At(j);
        if (digi == NULL) continue;
        if (digi->GetAddress() < 0) continue;
		CbmRichMapData* data =  CbmRichDigiMapManager::GetInstance().GetDataByAddress(digi->GetAddress());
		TVector3 posPoint;
		posPoint.SetXYZ(data->fX, data->fY, data->fZ);
		TVector3 detPoint;

		CbmRichGeoManager::GetInstance().RotatePoint(&posPoint, &detPoint);
        
        AddHit(detPoint, j);
	}
}



void CbmRichHitProducer::AddHit(
      TVector3 &posHit,
		Int_t index)
{
    Int_t nofHits = fRichHits->GetEntriesFast();
    new((*fRichHits)[nofHits]) CbmRichHit();
    CbmRichHit *hit = (CbmRichHit*)fRichHits->At(nofHits);
    hit->SetPosition(posHit);
    //hit->SetPositionError(posHitErr);
    //hit->SetAddress(address);
    //hit->SetPmtId(0);
    //hit->SetNPhotons(0);
    //hit->SetAmplitude(0);
    hit->SetRefId(index);
}

void CbmRichHitProducer::Finish()
{
  fRichHits->Clear();
}



ClassImp(CbmRichHitProducer)
