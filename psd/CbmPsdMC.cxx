/** @file CbmPsdMC.cxx
 ** @since 04.02.2005
 ** @author Alla Maevskaya <a.maevskaya@gsi.de>
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 18.09.2017 Major revision: Use geometry file instead of hard-coded geometry.
 **
 ** Former CbmPsdV1.cxx
 **/


#include "CbmPsdMC.h"

#include <cassert>
#include "TGeoManager.h"
#include "TKey.h"
#include "TVirtualMC.h"
#include "CbmModuleList.h"
#include "CbmPsdPoint.h"
#include "CbmStack.h"



// -----   Default constructor   -------------------------------------------
CbmPsdMC::CbmPsdMC(Bool_t active, const char* name)
  : FairDetector(name, active, kPsd),
    fPosX(0.),
    fPosZ(0.),
    fRotY(0.),
    fUserPlacement(kFALSE),
    fTrackID(-3),       
    fAddress(-3),
    fPsdPoints(new TClonesArray("CbmPsdPoint")),
    fPos(),           
    fMom(),        
    fTime(-1.),         
    fLength(-1.),        
    fEloss(-1.)
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmPsdMC::~CbmPsdMC() {
 if (fPsdPoints) {
   fPsdPoints->Delete();
   delete fPsdPoints;
 }
}
// -------------------------------------------------------------------------



// -----   Construct the geometry from file   ------------------------------
void CbmPsdMC::ConstructGeometry() {

  LOG(INFO) << GetName() << ": Constructing geometry from file "
              << fgeoName << FairLogger::endl;

  // --- A TGeoManager should be present
  assert(gGeoManager);

  // --- Only ROOT geometries are supported
  if (  ! GetGeometryFileName().EndsWith(".root") ) {
    LOG(FATAL) <<  GetName() << ": Geometry format of file "
                   << GetGeometryFileName() << " not supported."
                   << FairLogger::endl;
    return;
  }

  // --- Import PSD volume
  TGeoVolume* psd = TGeoVolume::Import(fgeoName);
  if ( ! psd ) {
    LOG(FATAL) << GetName() << ": No TGeoVolume found in " << fgeoName
        << FairLogger::endl;
    return;
  }
  if ( ! TString(psd->GetName()).BeginsWith("psd", TString::kIgnoreCase)) {
    LOG(FATAL) << GetName() << ": Volume " << psd->GetName() << " in "
        << fgeoName << " is not a PSD volume!" << FairLogger::endl;
    return;
  }
  LOG(DEBUG) << GetName() << ": Found PSD volume " << psd->GetName()
      << FairLogger::endl;

  // Import the transformation matrix
  TGeoMatrix* transformation = nullptr;
  TFile* geoFile = new TFile(fgeoName);
  assert ( geoFile );
  TKey* key = nullptr;
  TIter keyIter(geoFile->GetListOfKeys());
  while ( (key = (TKey*)keyIter() ) ) {
    if ( key->ReadObj()->InheritsFrom("TGeoMatrix") ) {
      transformation = dynamic_cast<TGeoMatrix*>(key->ReadObj());
      break;
    }
  }
  if ( ! transformation ) {
    LOG(FATAL) << GetName() << ": No transformation matrix found in "
              << fgeoName << FairLogger::endl;
    return;
  }

  // Add PSD to the geometry
  gGeoManager->GetTopVolume()->AddNode(psd, 0, transformation);
  if ( gLogger->IsLogNeeded(DEBUG) ) {
    psd->Print();
    transformation->Print();
  }

  // Register all sensitive volumes
  RegisterSensitiveVolumes(psd->GetNode(0));
  LOG(DEBUG) << GetName() << ": " << fNbOfSensitiveVol
      << " sensitive volumes" << FairLogger::endl;

}
// -------------------------------------------------------------------------



// -----   End of event action   -------------------------------------------
void CbmPsdMC::EndOfEvent() {
    Print();                 // Status output
    fPsdPoints->Delete();
}
// -------------------------------------------------------------------------



// -----   Print   ---------------------------------------------------------
void CbmPsdMC::Print(Option_t*) const {
  LOG(INFO) << fName << ": " << fPsdPoints->GetEntriesFast()
            << " points registered in this event." << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   Public method ProcessHits  --------------------------------------
Bool_t CbmPsdMC::ProcessHits(FairVolume*) {

  // No action for neutral particles
  if (TMath::Abs(gMC->TrackCharge()) <= 0) return kFALSE;

  Int_t layer  = -1;
  Int_t module = -1;

  // --- If this is the first step for the track in the volume:
  //     Reset energy loss and store track parameters
  if ( gMC->IsTrackEntering() ) {
    fTrackID = gMC->GetStack()->GetCurrentTrackNumber();
    gMC->CurrentVolOffID(1, layer);   // ID of scintillator layer
    gMC->CurrentVolOffID(3, module);  // ID of module
    fAddress = (layer << 16) | (module << 4) | kPsd; // Encode address
    gMC->TrackPosition(fPos);
    gMC->TrackMomentum(fMom);
    fTime    = gMC->TrackTime() * 1.0e09;
    fLength  = gMC->TrackLength();
    fEloss   = 0.;
  } //? track entering

  // --- For all steps within active volume: sum up differential energy loss
  fEloss += gMC->Edep();
  
  // --- If track is leaving: get track parameters and create CbmstsPoint
  if ( gMC->IsTrackExiting()    ||
       gMC->IsTrackStop()       ||
       gMC->IsTrackDisappeared()   ) {
    
    // Create CbmPsdPoint
    Int_t size = fPsdPoints->GetEntriesFast();
    CbmPsdPoint* point = new((*fPsdPoints)[size]) CbmPsdPoint(fTrackID,
                                                              fAddress,
                                                              fPos.Vect(),
                                                              fMom.Vect(),
                                                              fTime,
                                                              fLength,
                                                              fEloss);

    // --- Increment number of PsdPoints for this track in the stack
    CbmStack* stack = dynamic_cast<CbmStack*>(gMC->GetStack());
    assert(stack);
    stack->AddPoint(kPsd);
 
  }  //? track is exiting or stopped

  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Register the sensitive volumes   --------------------------------
void CbmPsdMC::RegisterSensitiveVolumes(TGeoNode* node) {

  TObjArray* daughters = node->GetVolume()->GetNodes();
  for (Int_t iDaughter = 0; iDaughter < daughters->GetEntriesFast();
       iDaughter++) {
    TGeoNode* daughter = dynamic_cast<TGeoNode*>(daughters->At(iDaughter));
    assert(daughter);
    if(daughter->GetNdaughters() > 0 ) RegisterSensitiveVolumes(daughter);
    TGeoVolume* daughterVolume = daughter->GetVolume();
    if ( CheckIfSensitive(daughterVolume->GetName()) ) {
      AddSensitiveVolume(daughterVolume);
    } //? Sensitive volume
  } //# Daughter nodes

}
// -------------------------------------------------------------------------


ClassImp(CbmPsdMC)
