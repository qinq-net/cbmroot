#include "CbmTutorial2Det.h"

#include "CbmTutorial2DetPoint.h"
#include "CbmTutorial2DetGeo.h"
#include "CbmTutorial2DetGeoPar.h"

#include "FairVolume.h"
#include "FairGeoVolume.h"
#include "FairGeoNode.h"
#include "FairRootManager.h"
#include "FairGeoLoader.h"
#include "FairGeoInterface.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "CbmDetectorList.h"
#include "CbmStack.h"

#include "TClonesArray.h"
#include "TVirtualMC.h"

#include <iostream>
using std::cout;
using std::endl;

CbmTutorial2Det::CbmTutorial2Det() :
  FairDetector("Tutorial2Det", kTRUE, kTutDet) {
  /** create your collection for data points */
  fCbmTutorial2DetPointCollection = new TClonesArray("CbmTutorial2DetPoint");
  
}

CbmTutorial2Det::CbmTutorial2Det(const char* name, Bool_t active)
  : FairDetector(name, active, kTutDet) {
  fCbmTutorial2DetPointCollection = new TClonesArray("CbmTutorial2DetPoint");
  
}
 
CbmTutorial2Det::~CbmTutorial2Det() {
   if (fCbmTutorial2DetPointCollection) {
     fCbmTutorial2DetPointCollection->Delete();
     delete fCbmTutorial2DetPointCollection;
   }
}

void CbmTutorial2Det::Initialize()
{
  FairDetector::Initialize();
  FairRuntimeDb *rtdb= FairRun::Instance()->GetRuntimeDb();
  CbmTutorial2DetGeoPar* par=(CbmTutorial2DetGeoPar*)(rtdb->getContainer("CbmTutorial2DetGeoPar"));
}
 
Bool_t  CbmTutorial2Det::ProcessHits(FairVolume* vol)
{ 
  /** This method is called from the MC stepping */
  
  //Set parameters at entrance of volume. Reset ELoss.
  if ( gMC->IsTrackEntering() ) {
    fELoss  = 0.;
    fTime   = gMC->TrackTime() * 1.0e09;
    fLength = gMC->TrackLength();
    gMC->TrackPosition(fPos);
    gMC->TrackMomentum(fMom);
  }

  // Sum energy loss for all steps in the active volume
  fELoss += gMC->Edep();
  
  // Create CbmTutorial2DetPoint at exit of active volume
  if ( gMC->IsTrackExiting()    ||
       gMC->IsTrackStop()       ||
       gMC->IsTrackDisappeared()   ) {
    fTrackID  = gMC->GetStack()->GetCurrentTrackNumber();
    fVolumeID = vol->getMCid();
    if (fELoss == 0. ) return kFALSE;
    AddHit(fTrackID, fVolumeID, TVector3(fPos.X(),  fPos.Y(),  fPos.Z()),
	   TVector3(fMom.Px(), fMom.Py(), fMom.Pz()), fTime, fLength,
	   fELoss);
    
    // Increment number of tutorial det points in TParticle
    CbmStack* stack = (CbmStack*) gMC->GetStack();
    stack->AddPoint(kTutDet);
  }
  
  return kTRUE;
}
 
void CbmTutorial2Det::EndOfEvent() {
  
  fCbmTutorial2DetPointCollection->Clear();
  
}



void CbmTutorial2Det::Register() {

/** This will create a branch in the output tree called  
    CbmTutorial2DetPoint, setting the last parameter to kFALSE means:
    this collection will not be written to the file, it will exist 
    only during the simulation. 
*/
 
  FairRootManager::Instance()->Register("Tutorial2DetPoint", "Tutorial2Det",
				       fCbmTutorial2DetPointCollection, kTRUE);
 
}


TClonesArray* CbmTutorial2Det::GetCollection(Int_t iColl) const {
  if (iColl == 0) return fCbmTutorial2DetPointCollection;
  else return NULL;
}

void CbmTutorial2Det::Reset() {
  fCbmTutorial2DetPointCollection->Clear();
}

void CbmTutorial2Det::ConstructGeometry() {
  /** If you are using the standard ASCII input for the geometry 
      just copy this and use it for your detector, otherwise you can
      implement here you own way of constructing the geometry. */
  
  FairGeoLoader*    geoLoad = FairGeoLoader::Instance();
  FairGeoInterface* geoFace = geoLoad->getGeoInterface();
  CbmTutorial2DetGeo*  Geo  = new CbmTutorial2DetGeo();
  Geo->setGeomFile(GetGeometryFileName());
  geoFace->addGeoModule(Geo);
  
  Bool_t rc = geoFace->readSet(Geo);
  if (rc) Geo->create(geoLoad->getGeoBuilder());
  TList* volList = Geo->getListOfVolumes();
  
  // store geo parameter
  FairRun *fRun = FairRun::Instance();
  FairRuntimeDb *rtdb= FairRun::Instance()->GetRuntimeDb();
  CbmTutorial2DetGeoPar* par=(CbmTutorial2DetGeoPar*)(rtdb->getContainer("CbmTutorial2DetGeoPar"));
  TObjArray *fSensNodes = par->GetGeoSensitiveNodes();
  TObjArray *fPassNodes = par->GetGeoPassiveNodes();
  
  TListIter iter(volList);
  FairGeoNode* node   = NULL;
  FairGeoVolume *aVol=NULL;
  
  while( (node = (FairGeoNode*)iter.Next()) ) {
    aVol = dynamic_cast<FairGeoVolume*> ( node );
    if ( node->isSensitive()  ) {
      fSensNodes->AddLast( aVol );
    }else{
      fPassNodes->AddLast( aVol );
    }
  }
  par->setChanged();
  par->setInputVersion(fRun->GetRunId(),1);
  
  ProcessNodes ( volList );
}

CbmTutorial2DetPoint* CbmTutorial2Det::AddHit(Int_t trackID, Int_t detID, 
					    TVector3 pos, TVector3 mom, 
					    Double_t time, Double_t length,
					    Double_t eLoss) {
  TClonesArray& clref = *fCbmTutorial2DetPointCollection;
  Int_t size = clref.GetEntriesFast();
  return new(clref[size]) CbmTutorial2DetPoint(trackID, detID, pos, mom,
					      time, length, eLoss);
}

ClassImp(CbmTutorial2Det)
