// -------------------------------------------------------------------------
// -----                       CbmStack source file                    -----
// -----             Created 10/08/04  by D. Bertini / V. Friese       -----
// -------------------------------------------------------------------------
#include "CbmStack.h"

#include "FairDetector.h"
#include "FairMCPoint.h"
#include "CbmMCTrack.h"

#include "FairRootManager.h"

#include "TLorentzVector.h"
#include "TClonesArray.h"
#include "TParticle.h"
#include "TRefArray.h"
#include "CbmStackFilter.h"

#include <list>
#include <cassert>


using std::make_pair;
using std::pair;
using std::vector;


// -----   Default constructor   -------------------------------------------
CbmStack::CbmStack(Int_t size)
  : FairGenericStack(),
    fStack(),
    fFilter(new CbmStackFilter),
    fParticles(new TClonesArray("TParticle", size)),
    fTracks(new TClonesArray("CbmMCTrack", size)),
    fIndexMap(),
    fIndexIter(),
    fPointsMap(),
    fCurrentTrack(-1),
    fNPrimaries(0),
    fNParticles(0),
    fNTracks(0),
    fIndex(0),
    fStoreSecondaries(kTRUE),
    fMinPoints(1),
    fEnergyCut(0.),
    fStoreMothers(kTRUE),
    fdoTracking(kTRUE)
{

}

// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStack::~CbmStack()
{
  if (fParticles) {
    fParticles->Delete();
    delete fParticles;
  }
  if (fTracks) {
    fTracks->Delete();
    delete fTracks;
  }
}
// -------------------------------------------------------------------------

void CbmStack::PushTrack(Int_t toBeDone, Int_t parentId, Int_t pdgCode,
                         Double_t px, Double_t py, Double_t pz,
                         Double_t e, Double_t vx, Double_t vy, Double_t vz,
                         Double_t time, Double_t polx, Double_t poly,
                         Double_t polz, TMCProcess proc, Int_t& ntr,
                         Double_t weight, Int_t is)
{

  PushTrack( toBeDone, parentId, pdgCode,
             px,  py,  pz,
             e,  vx,  vy,  vz,
             time,  polx,  poly,
             polz, proc, ntr,
             weight, is, -1);
}


// -----   Virtual public method PushTrack   -------------------------------
void CbmStack::PushTrack(Int_t toBeDone, Int_t parentId, Int_t pdgCode,
                         Double_t px, Double_t py, Double_t pz,
                         Double_t e, Double_t vx, Double_t vy, Double_t vz,
                         Double_t time, Double_t polx, Double_t poly,
                         Double_t polz, TMCProcess proc, Int_t& ntr,
                         Double_t weight, Int_t /*is*/, Int_t secondparentID)
{

  // --> Get TParticle array
  TClonesArray& partArray = *fParticles;

  // change parentId (was forces as a dummyparent of -1)
  if(parentId==-1 && secondparentID<fNParticles) parentId = secondparentID;

  // --> Create new TParticle and add it to the TParticle array
  Int_t trackId = fNParticles;
  Int_t nPoints = 0;
  Int_t daughter1Id = -1;
  Int_t daughter2Id = -1;

  TParticle* particle =
    new(partArray[fNParticles++]) TParticle(pdgCode, trackId, parentId,
					    nPoints, daughter1Id,
					    daughter2Id, px, py, pz, e,
					    vx, vy, vz, time);
  particle->SetPolarisation(polx, poly, polz);
  particle->SetWeight(weight);
  particle->SetUniqueID(proc);

  // --> Increment counter
  if (parentId < 0) { fNPrimaries++; }

  // --> Set argument variable
  ntr = trackId;

  // --> Push particle on the stack if toBeDone is set
  if (fdoTracking && toBeDone) {
    fStack.push(particle);
  }

}
// -------------------------------------------------------------------------



// -----   Virtual method PopNextTrack   -----------------------------------
TParticle* CbmStack::PopNextTrack(Int_t& iTrack)
{

  // If end of stack: Return empty pointer
  if (fStack.empty()) {
    iTrack = -1;
    return NULL;
  }

  // If not, get next particle from stack
  TParticle* thisParticle = fStack.top();
  fStack.pop();

  if ( !thisParticle) {
    iTrack = 0;
    return NULL;
  }

  fCurrentTrack = thisParticle->GetStatusCode();
  iTrack = fCurrentTrack;

  return thisParticle;

}
// -------------------------------------------------------------------------



// -----   Virtual method PopPrimaryForTracking   --------------------------
TParticle* CbmStack::PopPrimaryForTracking(Int_t iPrim)
{

  // Get the iPrimth particle from the fStack TClonesArray. This
  // should be a primary (if the index is correct).

  // Test for index
  if (iPrim < 0 || iPrim >= fNPrimaries) {
    LOG(FATAL) << "Primary index out of range! " << iPrim << FairLogger::endl;
  }

  // Return the iPrim-th TParticle from the fParticle array. This should be
  // a primary.
  TParticle* part = (TParticle*)fParticles->At(iPrim);
  if ( ! (part->GetMother(0) < 0) ) {
    LOG(FATAL) << "Not a primary track! " << iPrim << FairLogger::endl;
  }

  return part;

}
// -------------------------------------------------------------------------



// -----   Virtual public method GetCurrentTrack   -------------------------
TParticle* CbmStack::GetCurrentTrack() const
{
  TParticle* currentPart = GetParticle(fCurrentTrack);
  if ( ! currentPart) {
    LOG(WARNING) << "Current track not found in stack!" << FairLogger::endl;
  }
  return currentPart;
}
// -------------------------------------------------------------------------



// -----   Public method AddParticle   -------------------------------------
void CbmStack::AddParticle(TParticle* oldPart)
{
  TClonesArray& array = *fParticles;
  TParticle* newPart = new(array[fIndex]) TParticle(*oldPart);
  newPart->SetWeight(oldPart->GetWeight());
  newPart->SetUniqueID(oldPart->GetUniqueID());
  fIndex++;
}
// -------------------------------------------------------------------------



// -----   Fill the output MCTrack array   ---------------------------------
void CbmStack::FillTrackArray() {

  // Call the stack filter
  assert(fFilter);
  vector<Bool_t> store = fFilter->Select(*fParticles, fPointsMap);
  assert( store.size() == fParticles->GetEntriesFast() );

  // Reset index map
  fIndexMap.clear();
  fIndexMap[-1] = -1; // Map index for primary mothers

  // Copy selected particles to the output
  for (Int_t indexP = 0; indexP < fParticles->GetEntriesFast(); indexP++) {

    if ( store[indexP]) {

      // Create a new MCTrack in the output from the particle
      Int_t indexT = fTracks->GetEntriesFast();
      CbmMCTrack* track =
        new( (*fTracks)[indexT] ) CbmMCTrack(GetParticle(indexP));

      // Map the particle index to the track index
      fIndexMap[indexP] = indexT;

      // Set the number of points in the detectors for this track
      for (Int_t detector = kRef; detector <= kPsd; detector++) {
        auto it = fPointsMap.find(make_pair(indexP, detector));
        if ( it != fPointsMap.end() ) track->SetNPoints(detector, it->second);
      } //# detectors

    } //? store particle

    // For particles discarded from storage, the index is set to -2.
    else {
      fIndexMap[indexP] = -2;
    } //? do not store particle

  } //# stack particles

  fNTracks = fTracks->GetEntriesFast();
  LOG(INFO) << "CbmStack: " << fParticles->GetEntriesFast()
      << " particles, " << fNTracks << " written to output."
      << FairLogger::endl;

}
// -------------------------------------------------------------------------



// -----   Public method UpdateTrackIndex   --------------------------------
void CbmStack::UpdateTrackIndex(TRefArray* detList)
{

  LOG(DEBUG) << "Updating track indizes..." << FairLogger::endl;
  Int_t nColl = 0;

  // First update mother ID in MCTracks
  for (Int_t i=0; i<fNTracks; i++) {
    CbmMCTrack* track = (CbmMCTrack*)fTracks->At(i);
    Int_t iMotherOld = track->GetMotherId();
    fIndexIter = fIndexMap.find(iMotherOld);
    if (fIndexIter == fIndexMap.end()) {
      LOG(FATAL) << "Particle index " << iMotherOld
                 << " not found in dex map! " << FairLogger::endl;
    }
    track->SetMotherId( (*fIndexIter).second );
  }

  // Now iterate through all active detectors
  TIterator* detIter = detList->MakeIterator();
  detIter->Reset();
  FairDetector* det = NULL;
  while( (det = (FairDetector*)detIter->Next() ) ) {


    // --> Get hit collections from detector
    Int_t iColl = 0;
    TClonesArray* hitArray;
    while ( (hitArray = det->GetCollection(iColl++)) ) {
      nColl++;
      Int_t nPoints = hitArray->GetEntriesFast();

      // --> Update track index for all MCPoints in the collection
      for (Int_t iPoint=0; iPoint<nPoints; iPoint++) {
        FairMCPoint* point = (FairMCPoint*)hitArray->At(iPoint);
        Int_t iTrack = point->GetTrackID();

        fIndexIter = fIndexMap.find(iTrack);
        if (fIndexIter == fIndexMap.end()) {
          LOG(FATAL) << "Particle index " << iTrack
                     << " not found in index map! " << FairLogger::endl;
        }
        point->SetTrackID((*fIndexIter).second);
        point->SetLink(FairLink("MCTrack", (*fIndexIter).second));
      }

    }   // Collections of this detector
  }     // List of active detectors

  delete detIter;
  LOG(DEBUG) << "...stack and " << nColl << " collections updated." << FairLogger::endl;

}
// -------------------------------------------------------------------------



// -----   Public method Reset   -------------------------------------------
void CbmStack::Reset()
{
  fIndex = 0;
  fCurrentTrack = -1;
  fNPrimaries = fNParticles = fNTracks = 0;
  while (! fStack.empty() ) { fStack.pop(); }
  fParticles->Clear();
  fTracks->Clear();
  fPointsMap.clear();
}
// -------------------------------------------------------------------------



// -----   Public method Register   ----------------------------------------
void CbmStack::Register()
{
  FairRootManager::Instance()->Register("MCTrack", "Stack", fTracks,kTRUE);
}
// -------------------------------------------------------------------------



// -----   Public method Print  --------------------------------------------
void CbmStack::Print(Option_t*) const
{
  LOG(DEBUG) << "Number of primaries        = "
            << fNPrimaries << FairLogger::endl;
  LOG(DEBUG) << "Total number of particles  = "
            << fNParticles << FairLogger::endl;
  LOG(DEBUG) << "Number of tracks in output = "
            << fNTracks << FairLogger::endl;
  if ( FairLogger::GetLogger()->IsLogNeeded(DEBUG1) ) {
  	for (Int_t iTrack=0; iTrack<fNTracks; iTrack++) {
  		LOG(DEBUG1) << "MCTrack " << iTrack
  				        << ((CbmMCTrack*) fTracks->At(iTrack))->ToString()
  				        << FairLogger::endl;
  	}
  }
}
// -------------------------------------------------------------------------



// -----   Public method AddPoint (for current track)   --------------------
void CbmStack::AddPoint(ECbmModuleId detId)
{
  Int_t iDet = detId;
  pair<Int_t, Int_t> a(fCurrentTrack, iDet);
  if ( fPointsMap.find(a) == fPointsMap.end() ) { fPointsMap[a] = 1; }
  else { fPointsMap[a]++; }
}
// -------------------------------------------------------------------------



// -----   Public method AddPoint (for arbitrary track)  -------------------
void CbmStack::AddPoint(ECbmModuleId detId, Int_t iTrack)
{
  if ( iTrack < 0 ) { return; }
  Int_t iDet = detId;
  pair<Int_t, Int_t> a(iTrack, iDet);
  if ( fPointsMap.find(a) == fPointsMap.end() ) { fPointsMap[a] = 1; }
  else { fPointsMap[a]++; }
}
// -------------------------------------------------------------------------




// -----   Virtual method GetCurrentParentTrackNumber   --------------------
Int_t CbmStack::GetCurrentParentTrackNumber() const
{
  TParticle* currentPart = GetCurrentTrack();
  if ( currentPart ) { return currentPart->GetFirstMother(); }
  else { return -1; }
}
// -------------------------------------------------------------------------



// -----   Public method GetParticle   -------------------------------------
TParticle* CbmStack::GetParticle(Int_t trackID) const
{
  if (trackID < 0 || trackID >= fNParticles) {
    LOG(FATAL) << "Particle index " << trackID
               << " out of range." << FairLogger::endl;
  }
  return (TParticle*)fParticles->At(trackID);
}
// -------------------------------------------------------------------------





ClassImp(CbmStack)
