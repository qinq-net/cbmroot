/** @file CbmStackFilter.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 12.02.2019
 **/


#include "CbmStackFilter.h"

#include <cassert>
#include "TClonesArray.h"
#include "TMCProcess.h"
#include "TParticle.h"
#include "FairLogger.h"


using std::make_pair;
using std::vector;


// -----   Constructor   ----------------------------------------------------
CbmStackFilter::CbmStackFilter() :
        fStoreAllPrimaries(kTRUE),
        fStoreAllMothers(kTRUE),
        fMinNofPointsGlobal(1),
        fMinNofPoints(),
        fMinEkin(0.),
        fStore()
{
}
// --------------------------------------------------------------------------



// -----   Destructor   -----------------------------------------------------
CbmStackFilter::~CbmStackFilter() {
}
// --------------------------------------------------------------------------



// -----   Selection procedure   --------------------------------------------
const vector<Bool_t>& CbmStackFilter::Select(const TClonesArray& particles,
                                             const PointMap& points) {

  // Adjust size of output vector
  Int_t nParticles = particles.GetEntriesFast();
  fStore.resize(nParticles);
  Int_t nSelected = 0;

  // Loop over particles in array
  for (Int_t index = 0; index < nParticles; index++) {

    // Get particle object
    TParticle* particle = dynamic_cast<TParticle*>(particles.At(index));
    assert(particle);

    // Check for being a primary
    if ( fStoreAllPrimaries ) {
      Int_t iMother = particle->GetMother(0);
      if (iMother < 0 || particle->GetUniqueID()==kPPrimary) {
        fStore[index] = kTRUE;
        nSelected++;
        continue;
      } //? is a primary
    } //? store all primaries

    // Check cuts on number of points in detectors
    Bool_t store = kTRUE;
    for (auto& cut : fMinNofPoints) {
      ECbmModuleId detector = cut.first;
      UInt_t minimum = cut.second;
      auto it = points.find(make_pair(index, detector));
      UInt_t nPoints = ( it == points.end() ? 0 : it->second);
      if ( nPoints < minimum ) store = kFALSE;
    } //# cuts on number of points in detectors
    if ( ! store ) {
      fStore[index] = kFALSE;
      continue;
    } //? not passed the nPoints cuts

    // Check cut on total number of points
    Int_t nPoints = 0;
    for (Int_t detector=kMvd; detector<kNofSystems; detector++) {
      auto it = points.find(make_pair(index, detector));
      if ( it != points.end() ) nPoints += it->second;
    } //# detectors
    if ( nPoints < fMinNofPointsGlobal ) {
      fStore[index] = kFALSE;
      continue;
    } //? not passed cut on global number of points

    // Check cut on kinetic energy
    if ( particle->Ek() < fMinEkin ) {
      fStore[index] = kFALSE;
      continue;
    } //? not passed cut on kinetic energy

    // Arriving here after all cuts, the particle is flagged for storage
    fStore[index] = kTRUE;
    nSelected++;

  } //# particles


  // Mark recursively all mothers of already selected tracks for storage
  if ( fStoreAllMothers ) {
    for (UInt_t index = 0; index < particles.GetEntriesFast(); index++) {
      if ( ! fStore[index] ) continue;
      Int_t iMother = dynamic_cast<TParticle*>(particles.At(index))->GetMother(0);
      while ( iMother >= 0 ) {
        fStore[iMother] = kTRUE;
        iMother = dynamic_cast<TParticle*>(particles.At(iMother))->GetMother(0);
      } //? not a primary
    } //# particles
  } //? store all mothers

  return fStore;
}
// --------------------------------------------------------------------------


ClassImp(CbmStackFilter)

