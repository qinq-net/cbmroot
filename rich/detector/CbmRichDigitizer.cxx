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

#include <iostream>

using namespace std;

CbmRichDigitizer::CbmRichDigitizer()
 : FairTask("CbmRichDigitizer"),
   fEventNum(0),
   fRichPoints(NULL),
   fRichDigis(NULL),
   fMcTracks(NULL),
   fPmt(),
   fCrossTalkProbability(0.02),
   fNofNoiseDigis(200),
   fDigisMap()
{

}

CbmRichDigitizer::~CbmRichDigitizer()
{

}


InitStatus CbmRichDigitizer::Init()
{
   FairRootManager* manager = FairRootManager::Instance();

   fRichPoints = (TClonesArray*)manager->GetObject("RichPoint");
   if (NULL == fRichPoints) { Fatal("CbmRichDigitizer::Init","No RichPoint array!"); }

   fMcTracks = (TClonesArray *)manager->GetObject("MCTrack");
   if (NULL == fMcTracks) { Fatal("CbmRichDigitizer::Init","No MCTrack array!"); }

   fRichDigis = new TClonesArray("CbmRichDigi");
   manager->Register("RichDigi","RICH", fRichDigis, IsOutputBranchPersistent("RichDigi"));

   return kSUCCESS;
}

void CbmRichDigitizer::Exec(
      Option_t* /*option*/)
{
	fEventNum++;
	LOG(INFO) << "CbmRichDigitizer event  " << fEventNum << FairLogger::endl;
	fRichDigis->Delete();
	fDigisMap.clear();

	Int_t nofRichPoints = fRichPoints->GetEntries();
	LOG(INFO) << "Number of RichPoints: " << nofRichPoints << FairLogger::endl;
	for(Int_t j = 0; j < nofRichPoints; j++){
		CbmRichPoint* point = (CbmRichPoint*) fRichPoints->At(j);
		ProcessPoint(point, j);
	}

	AddNoiseDigis();

	AddDigisToOutputArray();
}

void CbmRichDigitizer::ProcessPoint(CbmRichPoint* point, Int_t pointId)
{
   // LOG(INFO) << "ProcessPoint " << pointId << FairLogger::endl;
//	TGeoNode* node = gGeoManager->FindNode(point->GetX(), point->GetY(), point->GetZ());
	gGeoManager->FindNode(point->GetX(), point->GetY(), point->GetZ());
	string path(gGeoManager->GetPath());
	Int_t address = CbmRichDigiMapManager::GetInstance().GetAdressByPath(path);
   // cout << "address:" << address << " node:" << path << endl;

	Int_t trackId = point->GetTrackID();
	if (trackId < 0) return;
	CbmMCTrack* p = (CbmMCTrack*) fMcTracks->At(trackId);
	if (p == NULL) return;
	Int_t gcode = TMath::Abs(p->GetPdgCode());

	CbmLink link(1., pointId);

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

	if (isDetected) {
		AddDigi(address, link);
	}
}


void CbmRichDigitizer::Finish()
{
	fRichDigis->Delete();
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
	for(auto const &dm : fDigisMap) {
        //cout <<dm.second->GetAddress() << endl;
		new((*fRichDigis)[nofDigis]) CbmRichDigi();
		CbmRichDigi* digi = (CbmRichDigi*)fRichDigis->At(nofDigis);
		digi->SetAddress(dm.second->GetAddress());
		digi->SetMatch(dm.second->GetMatch());
		nofDigis++;
	}
	LOG(INFO) << "Number of RICH digis: " << nofDigis << FairLogger::endl;

}

void CbmRichDigitizer::AddNoiseDigis()
{
	for(Int_t j = 0; j < fNofNoiseDigis; j++) {
		Int_t address = CbmRichDigiMapManager::GetInstance().GetRandomAddress();
		CbmLink link(1., -1);
		AddDigi(address, link);
	}
}

void CbmRichDigitizer::AddDigi(Int_t address, const CbmLink& link)
{
	std::map<Int_t, CbmRichDigi*>::iterator it = fDigisMap.find(address);
	if (it == fDigisMap.end()) {
		CbmRichDigi* digi = new CbmRichDigi();
		digi->SetAddress(address);
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
