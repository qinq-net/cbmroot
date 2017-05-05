// -------------------------------------------------------------------------
// -----                       CbmTof source file                      -----
// -----                  Created 28/07/04  by V. Friese               -----
// -------------------------------------------------------------------------

#include "CbmTof.h"

#include "CbmTofPoint.h"
#include "CbmTofGeoHandler.h"
#include "CbmGeoTofPar.h"
#include "CbmGeoTof.h"
#include "CbmDetectorList.h"
#include "CbmStack.h"

#include "FairGeoInterface.h"
#include "FairGeoLoader.h"
#include "FairGeoNode.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"
#include "FairRun.h"
#include "FairVolume.h"

#include "TClonesArray.h"
#include "TParticle.h"
#include "TVirtualMC.h"
#include "TObjArray.h"
#include "TGeoVolume.h"
#include "TGeoNode.h"
#include "TGeoManager.h"
#include "TKey.h"

#include <iostream>

using std::cout;
using std::endl;

// -----   Default constructor   -------------------------------------------
CbmTof::CbmTof() 
  : FairDetector("TOF", kTRUE, kTOF),
    fTrackID(-1),
    fVolumeID(-1),
    fPos(),
    fMom(),
    fTime(-1.),
    fLength(-1.),
    fELoss(-1.),
    fPosIndex(0),
    fTofCollection(new TClonesArray("CbmTofPoint")),
    fGeoHandler(new CbmTofGeoHandler()),
    fCombiTrans(),
    fVolumeName(""),
    fbOnePointPerTrack(kFALSE),
    fbIsNewTrack(kFALSE)
{
  fVerboseLevel = 1;
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmTof::CbmTof(const char* name, Bool_t active)
  : FairDetector(name, active, kTOF),
    fTrackID(-1),
    fVolumeID(-1),
    fPos(),
    fMom(),
    fTime(-1.),
    fLength(-1.),
    fELoss(-1.),
    fPosIndex(0),
    fTofCollection(new TClonesArray("CbmTofPoint")),
    fGeoHandler(new CbmTofGeoHandler()),
    fCombiTrans(),
    fVolumeName(""),
    fbOnePointPerTrack(kFALSE),
    fbIsNewTrack(kFALSE)
{
  fVerboseLevel = 1;
}
// -------------------------------------------------------------------------




// -----   Destructor   ----------------------------------------------------
CbmTof::~CbmTof() {
 if (fTofCollection) {
   fTofCollection->Delete(); 
   delete fTofCollection;
 }
 if (fGeoHandler) {
   delete fGeoHandler;
 }
}
// -------------------------------------------------------------------------

void CbmTof::Initialize()
{
  FairDetector::Initialize();

  // Initialize the CbmTofGeoHandler helper class from the
  // TVirtualMC interface
  Bool_t isSimulation=kTRUE;
  /*Int_t bla =*/ fGeoHandler->Init(isSimulation);

}

void CbmTof::PreTrack()
{
  if( fbOnePointPerTrack )
  {
    fbIsNewTrack = kTRUE;
  }
}

void CbmTof::FinishEvent()
{
  // method is called right before FairRootManager::Fill, once per FairDetector per event

  if( fbOnePointPerTrack )
  {
    // loop over all MC points (1 point per counter) created by all tracks in the event
    for( Int_t iPoint = 0; iPoint < fTofCollection->GetEntriesFast(); iPoint++ )
    {
      CbmTofPoint* tCurrentPoint = dynamic_cast<CbmTofPoint*>(fTofCollection->At(iPoint));

      // average all point properties except for the energy loss in the gaps
      Int_t iNCells = tCurrentPoint->GetNCells();
      tCurrentPoint->SetTime( tCurrentPoint->GetTime() / iNCells );
      tCurrentPoint->SetLength( tCurrentPoint->GetLength() / iNCells );
      tCurrentPoint->SetX( tCurrentPoint->GetX() / iNCells );
      tCurrentPoint->SetY( tCurrentPoint->GetY() / iNCells );
      tCurrentPoint->SetZ( tCurrentPoint->GetZ() / iNCells );
      tCurrentPoint->SetPx( tCurrentPoint->GetPx() / iNCells );
      tCurrentPoint->SetPy( tCurrentPoint->GetPy() / iNCells );
      tCurrentPoint->SetPz( tCurrentPoint->GetPz() / iNCells );
    }
  }
}

// -----   Public method ProcessHits  --------------------------------------
Bool_t  CbmTof::ProcessHits(FairVolume* /*vol*/)
{
  if( fbOnePointPerTrack )
  {
    // create/update CbmTofPoint objects for any charged particle or geantinos/rootinos
    if( 0 != gMC->TrackCharge() || 0 == gMC->TrackPid() )
    {
      Int_t iCounterID = fGeoHandler->GetUniqueCounterId();
      Int_t iTrackID = gMC->GetStack()->GetCurrentTrackNumber();

      Double_t dTrackEnergyDeposit = gMC->Edep();

      CbmTofPoint* tCounterPoint(0);
      Bool_t bCounterPointExists = kFALSE;

      // scan the MC point array only if an existing point may be found
      if( !fbIsNewTrack )
      {
        // loop over all MC points (1 point per counter) created by all tracks in the event so far
        // in reverse order to find the proper MC point immediately
        for( Int_t iPoint = fTofCollection->GetEntriesFast() - 1; iPoint >= 0; iPoint-- )
        {
          tCounterPoint = dynamic_cast<CbmTofPoint*>(fTofCollection->At(iPoint));

          if( tCounterPoint->GetDetectorID() == iCounterID && tCounterPoint->GetTrackID() == iTrackID )
          {
            bCounterPointExists = kTRUE;
            break;
          }
        }
      }
      else
      {
        fbIsNewTrack = kFALSE;
      }

      // first step of the track in the current gas gap (cell)
      if( gMC->IsTrackEntering() )
      {
        Double_t dTrackTime = gMC->TrackTime() * 1.0e09;
        Double_t dTrackLength = gMC->TrackLength();
        Double_t dTrackPositionX(0.);
        Double_t dTrackPositionY(0.);
        Double_t dTrackPositionZ(0.);
        gMC->TrackPosition(dTrackPositionX, dTrackPositionY, dTrackPositionZ);
        Double_t dTrackMomentumX(0.);
        Double_t dTrackMomentumY(0.);
        Double_t dTrackMomentumZ(0.);
        Double_t dTrackEnergy(0.);
        gMC->TrackMomentum(dTrackMomentumX, dTrackMomentumY, dTrackMomentumZ, dTrackEnergy);

        if( bCounterPointExists )
        {
          tCounterPoint->SetTime( tCounterPoint->GetTime() + dTrackTime );
          tCounterPoint->SetLength( tCounterPoint->GetLength() + dTrackLength );
          tCounterPoint->SetEnergyLoss( tCounterPoint->GetEnergyLoss() + dTrackEnergyDeposit );
          tCounterPoint->SetX( tCounterPoint->GetX() + dTrackPositionX );
          tCounterPoint->SetY( tCounterPoint->GetY() + dTrackPositionY );
          tCounterPoint->SetZ( tCounterPoint->GetZ() + dTrackPositionZ );
          tCounterPoint->SetPx( tCounterPoint->GetPx() + dTrackMomentumX );
          tCounterPoint->SetPy( tCounterPoint->GetPy() + dTrackMomentumY );
          tCounterPoint->SetPz( tCounterPoint->GetPz() + dTrackMomentumZ );
          tCounterPoint->SetNCells( tCounterPoint->GetNCells() + 1 );
        }
        else
        {
          tCounterPoint = new( (*fTofCollection)[fTofCollection->GetEntriesFast()] ) CbmTofPoint();
          tCounterPoint->SetTrackID( iTrackID );
          tCounterPoint->SetDetectorID( iCounterID );

          tCounterPoint->SetTime( dTrackTime );
          tCounterPoint->SetLength( dTrackLength );
          tCounterPoint->SetEnergyLoss( dTrackEnergyDeposit );
          tCounterPoint->SetX( dTrackPositionX );
          tCounterPoint->SetY( dTrackPositionY );
          tCounterPoint->SetZ( dTrackPositionZ );
          tCounterPoint->SetPx( dTrackMomentumX );
          tCounterPoint->SetPy( dTrackMomentumY );
          tCounterPoint->SetPz( dTrackMomentumZ );
          tCounterPoint->SetNCells( 1 );

          // Increment number of tof points for TParticle
          CbmStack* stack = dynamic_cast<CbmStack*>(gMC->GetStack());
          stack->AddPoint(kTOF);
        }

        tCounterPoint->SetGap( fGeoHandler->GetGap( iCounterID ) );
      }
      else
      {
        tCounterPoint->SetEnergyLoss( tCounterPoint->GetEnergyLoss() + dTrackEnergyDeposit );
      }

    }
  }
  else
  {
    // Set parameters at entrance of volume. Reset ELoss.
    if ( gMC->IsTrackEntering() ) {
      fELoss  = 0.;
      fTime   = gMC->TrackTime() * 1.0e09;
      fLength = gMC->TrackLength();
      gMC->TrackPosition(fPos);
      gMC->TrackMomentum(fMom);
    }

    // Sum energy loss for all steps in the active volume
    fELoss += gMC->Edep();

    // Create CbmTofPoint at exit of active volume
    if (((0 == gMC->GetStack()->GetCurrentTrack()->GetPdgCode()) || // Add geantinos/rootinos
         (gMC->TrackCharge()!=0) )&&
        (gMC->IsTrackExiting()    ||
         gMC->IsTrackStop()       ||
         gMC->IsTrackDisappeared()) 
         ) {

      fTrackID  = gMC->GetStack()->GetCurrentTrackNumber();

      fVolumeID = fGeoHandler->GetUniqueDetectorId();

      LOG(DEBUG2)<<"CbmTof::TID: "<<fTrackID;
      LOG(DEBUG2)<<" TofVol: "<<fVolumeID;
      LOG(DEBUG2)<<" DetSys: "<<fGeoHandler->GetDetSystemId(fVolumeID);
      LOG(DEBUG2)<<" SMtype: "<<fGeoHandler->GetSMType(fVolumeID);
      LOG(DEBUG2)<<" SModule: "<<fGeoHandler->GetSModule(fVolumeID);
      LOG(DEBUG2)<<" Counter: "<<fGeoHandler->GetCounter(fVolumeID);
      LOG(DEBUG2)<<" Gap: "<<fGeoHandler->GetGap(fVolumeID);
      LOG(DEBUG2)<<" Cell: "<<fGeoHandler->GetCell(fVolumeID);
      LOG(DEBUG2)<<Form(" x: %6.2f",fPos.X());
      LOG(DEBUG2)<<Form(" y: %6.2f",fPos.Y());
      LOG(DEBUG2)<<Form(" z: %6.2f",fPos.Z())<<FairLogger::endl;
      //   LOG(DEBUG2)<<"Region: "<<fGeoHandler->GetRegion(fVolumeID)<<FairLogger::endl;
      //   LOG(DEBUG2)<<"*************"<<FairLogger::endl;

      //fVolumeID = ((region-1)<<24) + ((module-1)<<14) + ((cell-1)<<4) + (gap-1);

      AddHit(fTrackID, fVolumeID, TVector3(fPos.X(),  fPos.Y(),  fPos.Z()),
	     TVector3(fMom.Px(), fMom.Py(), fMom.Pz()), fTime, fLength, 
	     fELoss);

      // Increment number of tof points for TParticle
      CbmStack* stack = (CbmStack*) gMC->GetStack();
      stack->AddPoint(kTOF);

      ResetParameters();
    }

  }

  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Public method EndOfEvent   --------------------------------------
void CbmTof::EndOfEvent() {
  if (fVerboseLevel) Print();
  fTofCollection->Delete();
  fPosIndex = 0;
}
// -------------------------------------------------------------------------



// -----   Public method Register   ----------------------------------------
void CbmTof::Register() {
  FairRootManager::Instance()->Register("TofPoint", "Tof", fTofCollection, kTRUE);
}
// -------------------------------------------------------------------------



// -----   Public method GetCollection   -----------------------------------
TClonesArray* CbmTof::GetCollection(Int_t iColl) const {
  if (iColl == 0) return fTofCollection;
  else return NULL;
}
// -------------------------------------------------------------------------



// -----   Public method Print   -------------------------------------------
void CbmTof::Print(Option_t*) const {
  Int_t nHits = fTofCollection->GetEntriesFast();
  LOG(INFO) << fName << ": " << nHits << " points registered in this event." << FairLogger::endl;

  if (fVerboseLevel>1)
    for (Int_t i=0; i<nHits; i++) 
      (*fTofCollection)[i]->Print();
}
// -------------------------------------------------------------------------



// -----   Public method Reset   -------------------------------------------
void CbmTof::Reset() {
  fTofCollection->Delete();
  ResetParameters();
}
// -------------------------------------------------------------------------



// -----   Public method CopyClones   --------------------------------------
void CbmTof::CopyClones(TClonesArray* cl1, TClonesArray* cl2, Int_t offset){
  Int_t nEntries = cl1->GetEntriesFast();
  LOG(INFO) << "CbmTof: " << nEntries << " entries to add." << FairLogger::endl;
  TClonesArray& clref = *cl2;
  CbmTofPoint* oldpoint = NULL;
   for (Int_t i=0; i<nEntries; i++) {
   oldpoint = (CbmTofPoint*) cl1->At(i);
    Int_t index = oldpoint->GetTrackID() + offset;
    oldpoint->SetTrackID(index);
    new (clref[fPosIndex]) CbmTofPoint(*oldpoint);
    fPosIndex++;
  }
  LOG(INFO) << "CbmTof: " << cl2->GetEntriesFast() << " merged entries." << FairLogger::endl;
}
// -------------------------------------------------------------------------
void CbmTof::ConstructGeometry()
{
  TString fileName=GetGeometryFileName();
  if (fileName.EndsWith(".geo")) {
    LOG(INFO)<<"Constructing TOF  geometry from ASCII file "<<fileName<<FairLogger::endl;
    ConstructASCIIGeometry();
  } else if (fileName.EndsWith(".root")) {
    LOG(INFO)<<"Constructing TOF  geometry from ROOT  file "<<fileName<<FairLogger::endl;
    ConstructRootGeometry();
  } else {
    std::cout << "Geometry format not supported." << std::endl;
  }
}
 
// -----   Public method ConstructGeometry   -------------------------------
void CbmTof::ConstructASCIIGeometry() {
  
  Int_t count=0;
  Int_t count_tot=0;

  FairGeoLoader*    geoLoad = FairGeoLoader::Instance();
  FairGeoInterface* geoFace = geoLoad->getGeoInterface();
  CbmGeoTof*       tofGeo  = new CbmGeoTof();
  tofGeo->setGeomFile(GetGeometryFileName());
  geoFace->addGeoModule(tofGeo);

  Bool_t rc = geoFace->readSet(tofGeo);
  if (rc) tofGeo->create(geoLoad->getGeoBuilder());
  TList* volList = tofGeo->getListOfVolumes();

  // store geo parameter
  FairRun *fRun = FairRun::Instance();
  FairRuntimeDb *rtdb= FairRun::Instance()->GetRuntimeDb();
  CbmGeoTofPar* par=(CbmGeoTofPar*)(rtdb->getContainer("CbmGeoTofPar"));
  TObjArray *fSensNodes = par->GetGeoSensitiveNodes();
  TObjArray *fPassNodes = par->GetGeoPassiveNodes();

  TListIter iter(volList);
  FairGeoNode* node   = NULL;
  FairGeoVolume *aVol=NULL;

  while( (node = (FairGeoNode*)iter.Next()) ) {
      aVol = dynamic_cast<FairGeoVolume*> ( node );
       if ( node->isSensitive()  ) {
           fSensNodes->AddLast( aVol );
	   count++;
       }else{
           fPassNodes->AddLast( aVol );
       }
       count_tot++;
  }
  par->setChanged();
  par->setInputVersion(fRun->GetRunId(),1);  
  ProcessNodes ( volList );
}
// -------------------------------------------------------------------------
Bool_t CbmTof::CheckIfSensitive(std::string name)
{
  TString tsname = name; 
  if (tsname.Contains("Cell")){
    return kTRUE;
  }
  return kFALSE;
}


// -----   Private method AddHit   -----------------------------------------
CbmTofPoint* CbmTof::AddHit(Int_t trackID, Int_t detID, TVector3 pos,
			    TVector3 mom, Double_t time, Double_t length, 
			    Double_t eLoss) {
  TClonesArray& clref = *fTofCollection;
  Int_t size = clref.GetEntriesFast();
  return new(clref[size]) CbmTofPoint(trackID, detID, pos, mom,
				      time, length, eLoss);
}
// -------------------------------------------------------------------------

//__________________________________________________________________________
void CbmTof::ConstructRootGeometry()
{   
  if( IsNewGeometryFile(fgeoName) ) {

    TGeoVolume *module1 = TGeoVolume::Import(fgeoName, fVolumeName.c_str());
    gGeoManager->GetTopVolume()->AddNode(module1, 0, fCombiTrans);

    TObjArray* nodeList = gGeoManager->GetTopVolume()->GetNodes();
    TGeoNode* node = NULL;
    for (Int_t iNode = 0; iNode < nodeList->GetEntriesFast(); iNode++) {
      node = (TGeoNode*) nodeList->At(iNode);
      if (TString(node->GetName()).Contains(fVolumeName.c_str())) {
        break;
      }
    }
    if (NULL == node) {
      LOG(FATAL) << "Node " << fVolumeName.c_str() 
                 << " not found." << FairLogger::endl;
    }
    ExpandTofNodes(node);
  } else {
    FairModule::ConstructRootGeometry();
  } 
}   

void CbmTof::ExpandTofNodes(TGeoNode* fN)
{   
  TGeoVolume* v1=fN->GetVolume();
  TObjArray* NodeList=v1->GetNodes();
  for (Int_t Nod=0; Nod<NodeList->GetEntriesFast(); Nod++) {
    TGeoNode* fNode =(TGeoNode*)NodeList->At(Nod);
    
    if(fNode->GetNdaughters()>0) { ExpandTofNodes(fNode); }
    TGeoVolume* v= fNode->GetVolume();
    if ( (this->InheritsFrom("FairDetector")) && CheckIfSensitive(v->GetName())) {
      AddSensitiveVolume(v);
    }
  } 

}   

Bool_t CbmTof::IsNewGeometryFile(TString filename)
{

  TFile* f=new TFile(fgeoName);
  TList* l = f->GetListOfKeys();
  Int_t numKeys = l->GetSize();
  if ( 2 != numKeys) {
    LOG(INFO) << "Not exactly two keys in the file. File is not of new type."
              << FairLogger::endl;
    return kFALSE;
  }
  TKey* key;
  TIter next( l);
  Bool_t foundGeoVolume = kFALSE;
  Bool_t foundGeoMatrix = kFALSE;
  TGeoTranslation* trans = NULL;
  TGeoRotation* rot = NULL;
  while ((key = (TKey*)next())) {
    if (strcmp(key->GetClassName(),"TGeoVolume") == 0) {
      LOG(INFO) << "Found TGeoVolume in geometry file." << FairLogger::endl;
      LOG(INFO) << "Name: " << key->GetName() << FairLogger::endl;      
      foundGeoVolume =  kTRUE;
      fVolumeName = key->GetName();
      continue;
    }
    if (strcmp(key->GetClassName(),"TGeoVolumeAssembly") == 0) {
      LOG(INFO) << "Found TGeoVolumeAssembly in geometry file." << FairLogger::endl;
      LOG(INFO) << "Name: " << key->GetName() << FairLogger::endl;
      foundGeoVolume =  kTRUE;
      fVolumeName = key->GetName();
      continue;
    }
    if (strcmp(key->GetClassName(),"TGeoTranslation") == 0) {
      LOG(DEBUG) << "Found TGeoTranslation in geometry file." << FairLogger::endl;
      foundGeoMatrix =  kTRUE;
      trans = static_cast<TGeoTranslation*>(key->ReadObj());
      rot = new TGeoRotation();
      fCombiTrans = new TGeoCombiTrans(*trans, *rot);
      continue;
    }
    if (strcmp(key->GetClassName(),"TGeoRotation") == 0) {
      LOG(DEBUG) << "Found TGeoRotation in geometry file." << FairLogger::endl;
      foundGeoMatrix =  kTRUE;
      trans = new TGeoTranslation();
      rot = static_cast<TGeoRotation*>(key->ReadObj());
      fCombiTrans = new TGeoCombiTrans(*trans, *rot);
      continue;
    }
    if (strcmp(key->GetClassName(),"TGeoCombiTrans") == 0) {
      LOG(DEBUG) << "Found TGeoCombiTrans in geometry file." << FairLogger::endl;
      foundGeoMatrix =  kTRUE;
      fCombiTrans = static_cast<TGeoCombiTrans*>(key->ReadObj());
      continue;
    }
  }
  if ( foundGeoVolume && foundGeoMatrix ) {
    LOG(INFO) << "Geometry file is of new type." << FairLogger::endl;
    return kTRUE;
  } else {
    if ( !foundGeoVolume) {
      LOG(INFO) << "No TGeoVolume found in geometry file. File is not of new type."
              << FairLogger::endl;
    }
    if ( !foundGeoMatrix) {
      LOG(INFO) << "Not TGeoMatrix derived object found in geometry file. File is not of new type."
                << FairLogger::endl;
    }
    return kFALSE;
  }
}



ClassImp(CbmTof)
