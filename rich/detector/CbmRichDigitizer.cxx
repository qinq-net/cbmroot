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
#include "CbmRichGeoManager.h"
#include "TClonesArray.h"
#include "CbmRichPoint.h"
#include "TRandom.h"
#include "TStopwatch.h"
#include "CbmLink.h"
#include "CbmMCTrack.h"
#include "CbmMatch.h"
#include "FairLogger.h"
#include "FairRunAna.h"
#include "FairRunSim.h"
#include "FairEventHeader.h"
#include <cassert>
#include <iomanip>
#include <iostream>
#include "../../base/CbmDaqBuffer.h"

using namespace std;

CbmRichDigitizer::CbmRichDigitizer()
 : CbmDigitize("RichDigitizer"),
   fEventNum(0),
   fDetectorType(CbmRichPmtTypeCosy17NoWls),
   fRichPoints(NULL),
   fRichDigis(NULL),
   fMcTracks(NULL),
   fNofPoints(0.),
   fNofDigis(0.),
   fTimeTot(0.),
   fPmt(),
   fCrossTalkProbability(0.02),
   fNoiseDigiRate(5.),
   fDigisMap(),
   fTimeResolution(1.),
   fDarkRatePerPixel(1000),
   fPixelDeadTime(30.),
   fFiredPixelsMap(),
   fMaxNofHitsPerPmtCut(65)
{

}

CbmRichDigitizer::~CbmRichDigitizer()
{

}


InitStatus CbmRichDigitizer::Init()
{

  // Screen output
  std::cout << std::endl;
  LOG(INFO) << "==========================================================" << FairLogger::endl;
  LOG(INFO) << GetName() << ": Initialisation" << FairLogger::endl << FairLogger::endl;

   //FairTask* daq = FairRun::Instance()->GetTask("Daq");
   if ( ! fEventMode ) {
      LOG(INFO) << "CbmRichDigitizer uses TimeBased mode." << FairLogger::endl;
   } else {
      LOG(INFO) << "CbmRichDigitizer uses Events mode." << FairLogger::endl;
   }

   FairRootManager* manager = FairRootManager::Instance();

   // --- Initialise helper singletons in order not to do it in event
   // --- processing (spoils timing measurement)
   CbmRichDigiMapManager::GetInstance();
   CbmRichGeoManager::GetInstance();

   fRichPoints = (TClonesArray*)manager->GetObject("RichPoint");
   if (NULL == fRichPoints) { Fatal("CbmRichDigitizer::Init","No RichPoint array!"); }

   fMcTracks = (TClonesArray *)manager->GetObject("MCTrack");
   if (NULL == fMcTracks) { Fatal("CbmRichDigitizer::Init","No MCTrack array!"); }

   fRichDigis = new TClonesArray("CbmRichDigi");
   manager->Register("RichDigi","RICH", fRichDigis, IsOutputBranchPersistent("RichDigi"));

   LOG(INFO) << GetName() << ": Initialisation successful" << FairLogger::endl;

   fFiredPixelsMap.clear();

   return kSUCCESS;
}

void CbmRichDigitizer::Exec(
      Option_t* /*option*/)
{
    TStopwatch timer;
    timer.Start();

	fEventNum++;
	LOG(DEBUG) << fName << ": Event  " << fEventNum << FairLogger::endl;

	for(auto itr = fDigisMap.begin(); itr !=fDigisMap.end(); itr++) {
	  delete itr->second;
	}
	fDigisMap.clear();

	Double_t oldEventTime = fCurrentEventTime;
	GetEventInfo();

	LOG(DEBUG) << fName << ": EventNumber: " << fCurrentEvent << ", EventTime: " << fCurrentEventTime << FairLogger::endl;

	GenerateNoiseBetweenEvents(oldEventTime, fCurrentEventTime);

	Int_t nPoints = ProcessMcEvent();

	Int_t nDigis = AddDigisToOutputArray();


	// --- Statistics
    timer.Stop();
	fNofPoints += nPoints;
	fNofDigis  += nDigis;
	fTimeTot += timer.RealTime();

	// --- Event log
	LOG(INFO) << "+ " << setw(15) << GetName() << ": Event " << setw(6)
	          << right << fCurrentEvent << " at " << fixed << setprecision(3)
	          << fCurrentEventTime << " ns, points: " << nPoints
	          << ", digis: " << nDigis << ". Exec time " << setprecision(6)
	          << timer.RealTime() << " s."
	          << FairLogger::endl;
}

Int_t CbmRichDigitizer::ProcessMcEvent()
{
   Int_t nofRichPoints = fRichPoints->GetEntries();
   LOG(DEBUG) << fName << ": EventNum:" << fCurrentEvent << " InputNum:" << fCurrentInput << " EventTime:" << fCurrentEventTime
                 << " nofRichPoints:" << nofRichPoints << FairLogger::endl;

   for(Int_t j = 0; j < nofRichPoints; j++){
      CbmRichPoint* point = (CbmRichPoint*) fRichPoints->At(j);
      ProcessPoint(point, j, fCurrentMCEntry, fCurrentInput);
   }
  // cout << "nofDigis:" << fRichDigis->GetEntries() << endl;

   AddNoiseDigis(fCurrentMCEntry, fCurrentInput);

   return nofRichPoints;
}

void CbmRichDigitizer::GenerateNoiseBetweenEvents(Double_t oldEventTime, Double_t newEventTime)
{
    Int_t nofPixels = CbmRichDigiMapManager::GetInstance().GetPixelAddresses().size();
    Double_t dT = newEventTime - oldEventTime;
    Double_t nofNoisePixels = nofPixels * dT * (fDarkRatePerPixel / 1.e9);

    LOG(DEBUG) << "CbmRichDigitizer::GenerateNoiseBetweenEvents deltaTime:" << dT << " nofPixels:" << nofPixels
            << " nofNoisePixels:" <<nofNoisePixels << FairLogger::endl;

    for(Int_t j = 0; j < nofNoisePixels; j++) {
        Int_t address = CbmRichDigiMapManager::GetInstance().GetRandomPixelAddress();
        CbmLink link(1., -1, -1, -1);
        Double_t time = gRandom->Uniform(oldEventTime, newEventTime);
        AddDigi(address, time, link);
    }
}

void CbmRichDigitizer::ProcessPoint(CbmRichPoint* point, Int_t pointId, Int_t eventNum, Int_t inputNum)
{
   // LOG(INFO) << "ProcessPoint " << pointId << FairLogger::endl;
//	TGeoNode* node = gGeoManager->FindNode(point->GetX(), point->GetY(), point->GetZ());
	gGeoManager->FindNode(point->GetX(), point->GetY(), point->GetZ());
	string path(gGeoManager->GetPath());
	Int_t address = CbmRichDigiMapManager::GetInstance().GetPixelAddressByPath(path);

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
		isDetected = fPmt.isPhotonDetected(fDetectorType, momTotal);
	} else { // if not photon
		// worst case: assume that all charged particles crossing
		// the PMTplane leave Cherenkov light in the PMTglass which will be detected
		isDetected = true;
	}

	Double_t time = fCurrentEventTime + point->GetTime();
	Double_t  deltaT = gRandom->Gaus(0., fTimeResolution);
	time += deltaT;
	if (isDetected) {
		AddDigi(address, time, link);
	}
}


void CbmRichDigitizer::Finish()
{
	  std::cout << std::endl;
	  LOG(INFO) << "=====================================" << FairLogger::endl;
	  LOG(INFO) << GetName() << ": Run summary" << FairLogger::endl;
	  LOG(INFO) << "Events processed    : " << fEventNum << FairLogger::endl;
	  LOG(INFO) << "RichPoint / event   : " << setprecision(1)
	                              << fNofPoints / Double_t(fEventNum)
	                              << FairLogger::endl;
	  LOG(INFO) << "RichDigi / event    : "
	      << fNofDigis  / Double_t(fEventNum) << FairLogger::endl;
	  LOG(INFO) << "Digis per point     : " << setprecision(6)
	                              << fNofDigis / fNofPoints << FairLogger::endl;
	  LOG(INFO) << "Real time per event : " << fTimeTot / Double_t(fEventNum)
	                              << " s" << FairLogger::endl;
	  LOG(INFO) << "=====================================" << FairLogger::endl;
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

Int_t CbmRichDigitizer::AddDigisToOutputArray()
{
    Int_t nofDigis = 0;

    // fMaxNofHitsPerPmtCut is a maximum number of hits which can be registered per PMT per event.
    // If more then the whole PMT is skipped
    map<Int_t, Int_t> digisPerPmtMap;
    for(auto const &dm : fDigisMap) {
        CbmRichPixelData* pixelData = CbmRichDigiMapManager::GetInstance().GetPixelDataByAddress(dm.second->GetAddress());
        if (nullptr == pixelData) continue;
        Int_t pmtId = pixelData->fPmtId;
        digisPerPmtMap[pmtId]++;
    }

    Int_t nofSkippedPmts = 0;
    for(auto const &dm : digisPerPmtMap) {
        if (dm.second > fMaxNofHitsPerPmtCut) nofSkippedPmts++;
    }

    Int_t nofSkippedDigis = 0;
    for(auto const &dm : fDigisMap) {
        CbmRichPixelData* pixelData = CbmRichDigiMapManager::GetInstance().GetPixelDataByAddress(dm.second->GetAddress());
        if (nullptr == pixelData) continue;
        if (digisPerPmtMap[pixelData->fPmtId] > fMaxNofHitsPerPmtCut){
            nofSkippedDigis++;
            continue;
        }

        CbmRichDigi* digi = new CbmRichDigi();
        digi->SetAddress(dm.second->GetAddress());
        CbmMatch* digiMatch = new CbmMatch(*dm.second->GetMatch());
        digi->SetMatch(digiMatch);
        digi->SetTime(dm.second->GetTime());
        SendDigi(digi);
        nofDigis++;
    } //# digis in map

    LOG(DEBUG) << "Number of RICH digis before skip: " << fDigisMap.size() << FairLogger::endl;
    LOG(DEBUG) << "Nof skipped PMTs:" << nofSkippedPmts << FairLogger::endl;
    LOG(DEBUG) << "Nof skipped digis:" << nofSkippedDigis << FairLogger::endl;
    LOG(DEBUG) << "Number of RICH digis: " << nofDigis << FairLogger::endl;
    return nofDigis;
}

void CbmRichDigitizer::AddNoiseDigis(
        Int_t eventNum,
        Int_t inputNum)
{
    Int_t nofPixels = CbmRichDigiMapManager::GetInstance().GetPixelAddresses().size();
    Double_t dT = 50.; //ns
    Double_t nofRichPoints = fRichPoints->GetEntries();
    Double_t nofNoiseDigis = nofRichPoints * nofPixels * dT * (fNoiseDigiRate / 1.e9);

    LOG(DEBUG) << "CbmRichDigitizer::AddNoiseDigis NofAllPixels:" << nofPixels << " nofNoiseDigis:" << nofNoiseDigis << FairLogger::endl;
    for(Int_t j = 0; j < nofNoiseDigis; j++) {
        Int_t address = CbmRichDigiMapManager::GetInstance().GetRandomPixelAddress();
        CbmLink link(1., -1, eventNum, inputNum);
        Double_t time = fCurrentEventTime + gRandom->Uniform(0, dT);
        AddDigi(address, time, link);
    }
}

void CbmRichDigitizer::AddDigi(Int_t address, Double_t time, const CbmLink& link)
{
    Bool_t wasFired = fFiredPixelsMap.count(address) > 0;
    Bool_t isDetected = true;
    if (!fEventMode && wasFired) {
        Double_t lastFiredTime = fFiredPixelsMap[address];
        Double_t dt = std::fabs(time - lastFiredTime);
        if (dt < fPixelDeadTime) {
            isDetected = false;
//            LOG(DEBUG) << "CbmRichDigitizer::AddDigi pixel NOT registered: address:" << address << " cur-last=dT: "<< time << "-"
//                                        << lastFiredTime << "=" <<  dt << " fPixelDeadTime:" << fPixelDeadTime
//                                        << " fFiredPixelsMap.size():" << fFiredPixelsMap.size() << " link: " << link.GetIndex()<< FairLogger::endl;
        } else {
            isDetected = true;
//            LOG(DEBUG) << "CbmRichDigitizer::AddDigi pixel registered: address:" << address << " cur-last=dT: "<< time << "-"
//                                        << lastFiredTime << "=" <<  dt << " fPixelDeadTime:" << fPixelDeadTime
//                                        << " fFiredPixelsMap.size():" << fFiredPixelsMap.size() << " link: " << link.GetIndex() << FairLogger::endl;
        }
    } else {
        isDetected = true;
//        LOG(DEBUG) << "CbmRichDigitizer::AddDigi pixel was not fired before: address:" << address << " time:" << time
//                << " fFiredPixelsMap.size():" << fFiredPixelsMap.size() << " link: " << link.GetIndex() << FairLogger::endl;

    }

    if (isDetected) {
        std::map<Int_t, CbmRichDigi*>::iterator it = fDigisMap.find(address);
        if (it == fDigisMap.end()) {
            CbmRichDigi* digi = new CbmRichDigi();
            digi->SetAddress(address);
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

	fFiredPixelsMap[address] = time;
}


void CbmRichDigitizer::WriteDigi(CbmDigi* digi) {
  CbmRichDigi* thisDigi = dynamic_cast<CbmRichDigi*>(digi);
  assert (thisDigi);
  Int_t nDigis = fRichDigis->GetEntriesFast();
  new ((*fRichDigis)[nDigis]) CbmRichDigi(*thisDigi);
}


void CbmRichDigitizer::ResetArrays() {
  LOG(DEBUG) << fName << ": Resetting output arrays with size " << fRichDigis->GetEntriesFast();
  if ( fRichDigis ) fRichDigis->Delete();
}


ClassImp(CbmRichDigitizer)
