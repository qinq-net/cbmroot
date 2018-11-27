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
#include "CbmEvent.h"


#include <iostream>

using namespace std;



CbmRichHitProducer::CbmRichHitProducer():
FairTask("CbmRichHitProducer"),
fRichDigis(NULL),
fRichHits(NULL),
fEventNum(0),
fRotationNeeded(true),
fCbmEvents(NULL),
fHitError(0.6 / sqrt(12.))

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

    fCbmEvents = dynamic_cast<TClonesArray*>(manager->GetObject("Event"));
    if ( fCbmEvents == nullptr) {
        LOG(INFO) << ": CbmEvent NOT found \n \n \n" << FairLogger::endl;
    } else {
        LOG(INFO) << ": CbmEvent found \n \n \n" << FairLogger::endl;
    }

    fRichDigis = (TClonesArray*)manager->GetObject("RichDigi");
    if (nullptr == fRichDigis) { Fatal("CbmRichHitProducer::Init","No RichDigi array!"); }

    fRichHits = new TClonesArray("CbmRichHit");
    manager->Register("RichHit","RICH", fRichHits, IsOutputBranchPersistent("RichHit"));

    return kSUCCESS;
}

void CbmRichHitProducer::Exec(
        Option_t* /*option*/)
{
    fEventNum++;
    LOG(INFO) << "CbmRichHitProducer Event (or TimeSlice) " << fEventNum << FairLogger::endl;

    fRichHits->Delete();

    // if CbmEvent does not exist then process standard event.
    // if CbmEvent exists then proceed all events in time slice.
    Int_t nUnits = ( fCbmEvents != nullptr) ? fCbmEvents->GetEntriesFast() : 1;

    for (Int_t iUnit = 0; iUnit < nUnits; iUnit++) {
        CbmEvent* event = ( fCbmEvents != nullptr) ? static_cast<CbmEvent*>(fCbmEvents->At(iUnit)) : nullptr;
        ProcessData(event);
    }
}

void CbmRichHitProducer::ProcessData(
        CbmEvent* event)
{
    if (event != NULL) {
        LOG(INFO) << "CbmRichHitProducer CbmEvent mode. CbmEvent # " << event->GetNumber() << FairLogger::endl;
        Int_t nofDigis = event->GetNofData(kRichDigi);
        LOG(INFO) << "nofDigis: " << nofDigis << FairLogger::endl;

        for (Int_t iDigi = 0; iDigi < nofDigis; iDigi++) {
            Int_t digiIndex = event->GetIndex(kRichDigi, iDigi);
            ProcessDigi(event, iDigi);
        }

    } else {
        for(Int_t iDigi = 0; iDigi < fRichDigis->GetEntries(); iDigi++){
            ProcessDigi(event, iDigi);
        }
    }
}

void CbmRichHitProducer::ProcessDigi(
        CbmEvent* event,
        Int_t digiIndex)
{
    CbmRichDigi* digi = static_cast<CbmRichDigi*>(fRichDigis->At(digiIndex));
    if (digi == nullptr) return;
    if (digi->GetAddress() < 0) return;
    CbmRichPixelData* data =  CbmRichDigiMapManager::GetInstance().GetPixelDataByAddress(digi->GetAddress());
    TVector3 posPoint;
    posPoint.SetXYZ(data->fX, data->fY, data->fZ);
    TVector3 detPoint;

    CbmRichGeoManager::GetInstance().RotatePoint(&posPoint, &detPoint, !fRotationNeeded);
    AddHit(event, detPoint, digi->GetTime(), digiIndex);
}



void CbmRichHitProducer::AddHit(
        CbmEvent* event,
        TVector3 &posHit,
        Double_t time,
        Int_t index)
{
    Int_t nofHits = fRichHits->GetEntriesFast();
    new((*fRichHits)[nofHits]) CbmRichHit();
    CbmRichHit *hit = (CbmRichHit*)fRichHits->At(nofHits);
    hit->SetPosition(posHit);
    hit->SetDx(fHitError);
    hit->SetDy(fHitError);
    hit->SetRefId(index);
    hit->SetTime(time);

    if (event != NULL) {
        event->AddData(kRichHit, nofHits);
    }
}

void CbmRichHitProducer::Finish()
{
    fRichHits->Clear();
}



ClassImp(CbmRichHitProducer)
