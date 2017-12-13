/**
* \file CbmRichDigitizer.cxx
*
* \author S.Lebedev
* \date 2015
**/

#include "CbmRichDigitizer.h"
#include "CbmRichDigi.h"
#include "TGeoManager.h"
#include "CbmRichDigiMapManager.h"
#include "TClonesArray.h"
#include "CbmRichPoint.h"
#include "TRandom.h"
#include "CbmLink.h"
#include "CbmMCTrack.h"
#include "CbmMatch.h"
#include "FairLogger.h"
#include "FairRunAna.h"
#include "FairRunSim.h"
#include "FairEventHeader.h"
#include "CbmDaqBuffer.h"

#include <iostream>

using namespace std;

CbmRichDigitizer::CbmRichDigitizer()
 : FairTask("CbmRichDigitizer"),
   fEventNum(0),
   fMode(CbmRichDigitizerModeEvents),
   fRichPoints(NULL),
   fRichDigis(NULL),
   fMcTracks(NULL),
   fPmt(),
   fCrossTalkProbability(0.02),
   fNofNoiseDigis(200),
   fDigisMap(),
   fEventTime(0.),
   fTimeResolution(2.)
{

}

CbmRichDigitizer::~CbmRichDigitizer()
{

}


InitStatus CbmRichDigitizer::Init()
{
   FairTask* daq = FairRun::Instance()->GetTask("Daq");
   if ( daq != NULL ) {
      LOG(INFO) << "CbmRichDigitizer uses TimeBased mode." << FairLogger::endl;
      fMode = CbmRichDigitizerModeTimeBased;
   } else {
      LOG(INFO) << "CbmRichDigitizer uses Events mode." << FairLogger::endl;
      fMode = CbmRichDigitizerModeEvents;
   }

   FairRootManager* manager = FairRootManager::Instance();

   fRichPoints = (TClonesArray*)manager->GetObject("RichPoint");
   if (NULL == fRichPoints) { Fatal("CbmRichDigitizer::Init","No RichPoint array!"); }

   fMcTracks = (TClonesArray *)manager->GetObject("MCTrack");
   if (NULL == fMcTracks) { Fatal("CbmRichDigitizer::Init","No MCTrack array!"); }

   if (fMode == CbmRichDigitizerModeEvents) {
      fRichDigis = new TClonesArray("CbmRichDigi");
      manager->Register("RichDigi","RICH", fRichDigis, IsOutputBranchPersistent("RichDigi"));
   }

   return kSUCCESS;
}

void CbmRichDigitizer::Exec(
      Option_t* /*option*/)
{
	fEventNum++;
	LOG(INFO) << "CbmRichDigitizer event  " << fEventNum << FairLogger::endl;
	if (fRichDigis != NULL) {
	   fRichDigis->Delete();
	}
	fDigisMap.clear();

	Int_t eventNumber = FairRootManager::Instance()->GetEntryNr();
	Double_t oldEventTime = fEventTime;
	fEventTime = FairRun::Instance()->GetEventHeader()->GetEventTime();

	LOG(INFO) << "EventNumber:" << eventNumber << " fEventTime:" << fEventTime << FairLogger::endl;

	GenerateNoiseBetweenEvents(oldEventTime, fEventTime);

	ProcessMcEvent();

	AddDigisToOutputArray();
}

void CbmRichDigitizer::ProcessMcEvent()
{
   Int_t eventNum = FairRootManager::Instance()->GetEntryNr();
   Int_t inputNum = 0;
   Double_t eventTime = 0.;
   if ( FairRunAna::Instance() != NULL && FairRunAna::Instance()->GetEventHeader() != NULL) {
      inputNum   = FairRunAna::Instance()->GetEventHeader()->GetInputFileId();
      eventTime = FairRunAna::Instance()->GetEventHeader()->GetEventTime();
   }
   if (  FairRunAna::Instance() == NULL && FairRunSim::Instance() == NULL){
      LOG(FATAL)  << "CbmRichDigitizer: neither SIM nor ANA run." << FairLogger::endl;
   }

   Int_t nofRichPoints = fRichPoints->GetEntries();
   LOG(INFO) << "CbmRichDigitizer: EventNum:" << eventNum << " InputNum:" << inputNum << " EventTime:" << eventTime
                 << " nofRichPoints:" << nofRichPoints << FairLogger::endl;

   for(Int_t j = 0; j < nofRichPoints; j++){
      CbmRichPoint* point = (CbmRichPoint*) fRichPoints->At(j);
      ProcessPoint(point, j, eventNum, inputNum);
   }

   AddNoiseDigis(eventNum, inputNum);
}

void CbmRichDigitizer::GenerateNoiseBetweenEvents(Double_t oldEventTime, Double_t newEventTime)
{
   // TODO: Implement this method
}

void CbmRichDigitizer::ProcessPoint(CbmRichPoint* point, Int_t pointId, Int_t eventNum, Int_t inputNum)
{
   // LOG(INFO) << "ProcessPoint " << pointId << FairLogger::endl;
//	TGeoNode* node = gGeoManager->FindNode(point->GetX(), point->GetY(), point->GetZ());
	gGeoManager->FindNode(point->GetX(), point->GetY(), point->GetZ());
	string path(gGeoManager->GetPath());
	Int_t address = CbmRichDigiMapManager::GetInstance().GetAdressByPath(path);

	Int_t trackId = point->GetTrackID();
	if (trackId < 0) return;
	CbmMCTrack* p = (CbmMCTrack*) fMcTracks->At(trackId);
	if (p == NULL) return;
	Int_t gcode = TMath::Abs(p->GetPdgCode());

	CbmLink link(1., pointId, eventNum, inputNum);

	Bool_t isDetected = false;
	// for photons weight with efficiency of PMT
	if (gcode == 50000050) {
		TVector3 mom;
		point->Momentum(mom);
		Double_t momTotal = sqrt(mom.Px()*mom.Px() + mom.Py()*mom.Py() + mom.Pz()*mom.Pz());
		isDetected = fPmt.isPhotonDetected(momTotal);
	} else { // if not photon
		// worst case: assume that all charged particles crossing
		// the PMTplane leave Cherenkov light in the PMTglass which will be detected
		isDetected = true;
	}

	Double_t time = fEventTime + point->GetTime();
	Double_t  deltaT = gRandom->Gaus(0., fTimeResolution);
	time += deltaT;
	if (isDetected) {
		AddDigi(address, time, link);
	}
}


void CbmRichDigitizer::Finish()
{
	if (fRichDigis != NULL) fRichDigis->Delete();
}

void CbmRichDigitizer::AddCrossTalkDigis(
      CbmRichDigi* digi)
{
	Bool_t crosstalkDigiDetected = false;
	vector<Int_t> directPixels = CbmRichDigiMapManager::GetInstance().GetDirectNeighbourPixels(digi->GetAddress());
	vector<Int_t> diagonalPixels = CbmRichDigiMapManager::GetInstance().GetDiagonalNeighbourPixels(digi->GetAddress());

	for (UInt_t i = 0; i < directPixels.size(); i++) {
		if (gRandom->Rndm() < fCrossTalkProbability && !crosstalkDigiDetected) {
			//FindRichHitPositionMAPMT(0., x + r, y, xHit, yHit, pmtID);
			crosstalkDigiDetected = true;
			break;
		}
	}

	if (!crosstalkDigiDetected) {
		for (UInt_t i = 0; i < diagonalPixels.size(); i++) {
			if (gRandom->Rndm() < fCrossTalkProbability / 4. && !crosstalkDigiDetected) {
				//FindRichHitPositionMAPMT(0., x + r, y, xHit, yHit, pmtID);
				crosstalkDigiDetected = true;
				break;
			}
		}
	}

   if (crosstalkDigiDetected) {
	   //AddDigi(posHit, posHitErr, RichDetID, pmtID, ampl, pointInd);
	   //fNofCrossTalkDigis++;
   }
}

void CbmRichDigitizer::AddDigisToOutputArray()
{
   Int_t nofDigis = 0;
   if ( fMode == CbmRichDigitizerModeTimeBased ) {
      for(auto const &dm : fDigisMap) {
         CbmRichDigi* digi = new CbmRichDigi();
         digi->SetAddress(dm.second->GetAddress());
         digi->SetMatch(dm.second->GetMatch());
         digi->SetTime(dm.second->GetTime());
         CbmDaqBuffer::Instance()->InsertData(digi);
         nofDigis++;
      }
   } else if ( fMode == CbmRichDigitizerModeEvents ){
      for(auto const &dm : fDigisMap) {
         new((*fRichDigis)[nofDigis]) CbmRichDigi();
         CbmRichDigi* digi = (CbmRichDigi*)fRichDigis->At(nofDigis);
         digi->SetAddress(dm.second->GetAddress());
         digi->SetMatch(dm.second->GetMatch());
         digi->SetTime(dm.second->GetTime());
         nofDigis++;
      }
   }
   //CbmDaqBuffer::Instance()->PrintStatus();

	LOG(INFO) << "Number of RICH digis: " << nofDigis << FairLogger::endl;
}

void CbmRichDigitizer::AddNoiseDigis(
        Int_t eventNum,
        Int_t inputNum)
{
	for(Int_t j = 0; j < fNofNoiseDigis; j++) {
		Int_t address = CbmRichDigiMapManager::GetInstance().GetRandomAddress();
		CbmLink link(1., -1, eventNum, inputNum);
		// TODO: what time to assign for noise hits
		Double_t time = fEventTime + gRandom->Gaus(20., 2.);
		AddDigi(address, time, link);
	}
}

void CbmRichDigitizer::AddDigi(Int_t address, Double_t time, const CbmLink& link)
{
	std::map<Int_t, CbmRichDigi*>::iterator it = fDigisMap.find(address);
	if (it == fDigisMap.end()) {
		CbmRichDigi* digi = new CbmRichDigi();
		digi->SetAddress(address);
		// TODO: what time to assign if one has several MCPoints in digi
		digi->SetTime(time);
		CbmMatch* match = new CbmMatch();
		match->AddLink(link);
		digi->SetMatch(match);
		fDigisMap.insert(pair<Int_t, CbmRichDigi*>(address, digi));
	} else {
		CbmRichDigi* digi = it->second;
		CbmMatch* match = digi->GetMatch();
		match->AddLink(link);
	}

}

ClassImp(CbmRichDigitizer)
