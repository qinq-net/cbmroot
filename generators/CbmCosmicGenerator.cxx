// -------------------------------------------------------------------------
// -----             CbmCosmicGenerator header file                    -----
// -----              Created 05/09/18  by F.Uhlig                     -----
// -------------------------------------------------------------------------

#include "CbmCosmicGenerator.h"

#include "FairPrimaryGenerator.h"
#include "FairLogger.h"

#include "TRandom.h"
#include "TParticlePDG.h"
#include "TDatabasePDG.h"
#include "TMath.h"

// ------------------------------------------------------------------------
CbmCosmicGenerator::CbmCosmicGenerator() 
  : CbmCosmicGenerator(13,1)
{
}

// ------------------------------------------------------------------------
CbmCosmicGenerator::CbmCosmicGenerator(Int_t pdgid, Int_t mult) 
  : FairGenerator()
  , fPDGType(13)
  , fMult(1)
  , fX1(0.)
  , fX2(0.)
  , fY(0.)
  , fZ1(0.)
  , fZ2(0.)
{
}

// ------------------------------------------------------------------------
Bool_t  CbmCosmicGenerator::Init()
{
  // Initialize generator

  // Check for particle type
  TDatabasePDG* pdgBase = TDatabasePDG::Instance();
  TParticlePDG* particle = pdgBase->GetParticle(fPDGType);

  if (! particle) {
    LOG(FATAL) << "PDG code " << fPDGType << " not defined." << FairLogger::endl;
  } 

  return kTRUE;
}

// ------------------------------------------------------------------------
Bool_t CbmCosmicGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{
  // Generate one event with one particle only: 
  // produce a primary particles emitted from a plane.
  // The vertex of the primary particle is distributed uniformly inside the plane
  // The momentum is according to the distribution of cosmic muons

  Double_t x = gRandom->Uniform(fX1,fX2);
  Double_t z = gRandom->Uniform(fZ1,fZ2);

  Double_t px = -1.; 
  Double_t py = -1.; 
  Double_t pz = -1.;

  LOG(DEBUG) << "CbmCosmicGenerator: id=" << fPDGType 
             << " p=("<< px << "," << py << "," << pz <<") GeV"
             << " x=("<< x  << "," << fY << "," << z  <<") cm"
             << FairLogger::endl;

  primGen->AddTrack(fPDGType, px, py, pz, x, fY, z);
  
  return kTRUE;

}

// ------------------------------------------------------------------------
/*
FairGenerator* CbmCosmicGenerator::CloneGenerator() const
{
  // Clone for worker (used in MT mode only)

  return new CbmCosmicGenerator(*this);
}
*/

// ------------------------------------------------------------------------


ClassImp(CbmCosmicGenerator)
