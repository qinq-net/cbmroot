/** @file CbmStackFilter.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 16.02.2019
 ** @date 05.03.2019
 **
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
   fStoreAllDecays(kFALSE),
   fMinNofPoints(kNofSystems),
   fMinEkin(0.),
   fStore() {

  // Initialise NofPoints cuts
  for (Int_t iDet = 0; iDet < kNofSystems; iDet++) fMinNofPoints[iDet] = 1;
  fMinNofPoints[kPsd] = 5;  // A hard-coded number. I'll rot in hell for that.

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
      if (particle->GetUniqueID()==kPPrimary) {
        fStore[index] = kTRUE;
        nSelected++;
        continue;
      } //? is a primary
    } //? store all primaries

    // Check cuts on number of points in detectors
    fStore[index] = kFALSE;
    for (Int_t system = 0; system < kNofSystems; system++) {
      auto it = points.find(make_pair(index, system));
      UInt_t nPoints = ( it == points.end() ? 0 : it->second);
      if ( nPoints >= fMinNofPoints[system] ) {
        fStore[index] = kTRUE;
        continue;
      } //? Number cut satisfied
    } //# detector systems

    // Check cut on kinetic energy
    if ( particle->Ek() < fMinEkin ) fStore[index] = kFALSE;

  } //# particles


  // Mark all decay daughters of primaries for storage (if chosen such)
  TParticle* particle = nullptr;
  if ( fStoreAllDecays) {
    for (UInt_t index = 0; index < particles.GetEntriesFast(); index++) {
      if ( fStore[index] ) continue;  // already selected
      particle = dynamic_cast<TParticle*>(particles.At(index));
      assert(particle);

      // Follow the mother chain up to the primary.
      Bool_t store = kTRUE;
      UInt_t process = particle->GetUniqueID();
      while ( process != kPPrimary ) {
        if ( process != kPDecay ) {
          store = kFALSE;
          break;  // not a decay
        } //? not a decay
        Int_t iMother = particle->GetMother(0);  // mother index
        particle = dynamic_cast<TParticle*>(particles.At(iMother));
        assert(particle);
        process = particle->GetUniqueID();
      } //? not a primary

      fStore[index] = store;

    } //# particles
  } //? store all decays


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

