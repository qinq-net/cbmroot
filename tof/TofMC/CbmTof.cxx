// -------------------------------------------------------------------------
// -----                       CbmTof source file                      -----
// -----                  Created 28/07/04  by V. Friese               -----
// -------------------------------------------------------------------------

#include "CbmTof.h"

#include "CbmTofPoint.h"
#include "CbmTofGeoHandler.h"
#include "CbmGeoTofPar.h"
#include "CbmGeoTof.h"
#include "CbmStack.h"

#include "FairGeoBuilder.h"
#include "FairGeoMedia.h"
#include "FairGeoInterface.h"
#include "FairGeoLoader.h"
#include "FairGeoNode.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"
#include "FairRunSim.h"
#include "FairVolume.h"
#include "FairMCEventHeader.h"

#include "TClonesArray.h"
#include "TParticle.h"
#include "TVirtualMC.h"
#include "TObjArray.h"
#include "TGeoVolume.h"
#include "TGeoNode.h"
#include "TGeoManager.h"
#include "TKey.h"
#include "TGeoPhysicalNode.h"

#include <iostream>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

using std::cout;
using std::endl;

// -----   Default constructor   -------------------------------------------
CbmTof::CbmTof()
  : FairDetector("TOF", kTRUE, kTof),
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
    fbIsNewTrack(kFALSE),
    fTofNodePath(""),
    fCurrentNodePath(""),
    fCurrentModuleType(0),
    fCurrentModuleIndex(0),
    fCurrentCounterIndex(0),
    fInactiveCounters(),
    fCountersInBeam(),
    fNodesInBeam()
{
  fVerboseLevel = 1;
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmTof::CbmTof(const char* name, Bool_t active)
  : FairDetector(name, active, kTof),
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
    fbIsNewTrack(kFALSE),
    fTofNodePath(""),
    fCurrentNodePath(""),
    fCurrentModuleType(0),
    fCurrentModuleIndex(0),
    fCurrentCounterIndex(0),
    fInactiveCounters(),
    fCountersInBeam(),
    fNodesInBeam()
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

  for(auto &itNode : fNodesInBeam)
  {
    if(itNode)
    {
      delete itNode;
    }
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

  // Extract collision vertex information from the MC event header
  FairMCEventHeader* tEventHeader = FairRunSim::Instance()->GetMCEventHeader();

  Double_t dTargetVertexT = tEventHeader->GetT(); // [s]
  // Define a beam line parallel to Z through the collision vertex and another point shifted along Z
  Double_t dGlobalTargetCoordinates[3] = {tEventHeader->GetX(), tEventHeader->GetY(), tEventHeader->GetZ()};
  Double_t dGlobalTargetCoordinates1[3] = {tEventHeader->GetX(), tEventHeader->GetY(), tEventHeader->GetZ() + 1.};
  Double_t dGlobalCounterCoordinates[3] = {0., 0., 0.};
  Double_t dLocalTargetCoordinates[3] = {0., 0., 0.};
  Double_t dLocalTargetCoordinates1[3] = {0., 0., 0.};
  Double_t dLocalCounterCoordinates[3] = {0., 0., 0.};

  Int_t iCounter(0);
  TGeoPhysicalNode* tCurrentNode(NULL);
  Int_t iModuleType(0);
  Int_t iModuleIndex(0);
  Int_t iCounterIndex(0);
  Int_t iUniqueCounterId(0);
  CbmTofPoint* tBeamPoint(NULL);

  // Loop over all counters that are eligible for beam points
  for(auto const & itCounterInBeam : fCountersInBeam)
  {
    iModuleType   = std::get<0>(itCounterInBeam.first);
    iModuleIndex  = std::get<1>(itCounterInBeam.first);
    iCounterIndex = std::get<2>(itCounterInBeam.first);

    // Create the unique counter ID
    CbmTofDetectorInfo tCounterInfo(kTof, iModuleType, iModuleIndex, iCounterIndex, 0, 0);
    iUniqueCounterId = fGeoHandler->GetDetIdPointer()->SetDetectorInfo(tCounterInfo);

    tCurrentNode = fNodesInBeam.at(iCounter);

    // Transform the two points defining the beam line into the local coordinate system of the counter
    tCurrentNode->GetMatrix()->MasterToLocal(dGlobalTargetCoordinates, dLocalTargetCoordinates);
    tCurrentNode->GetMatrix()->MasterToLocal(dGlobalTargetCoordinates1, dLocalTargetCoordinates1);

    // Calculate the intersection point between the beam line and the counter plane in the local c.s.
    dLocalCounterCoordinates[0] = dLocalTargetCoordinates[0] - dLocalTargetCoordinates[2]
                                                              *(dLocalTargetCoordinates1[0] - dLocalTargetCoordinates[0])
                                                              /(dLocalTargetCoordinates1[2] - dLocalTargetCoordinates[2]);
    dLocalCounterCoordinates[1] = dLocalTargetCoordinates[1] - dLocalTargetCoordinates[2]
                                                              *(dLocalTargetCoordinates1[1] - dLocalTargetCoordinates[1])
                                                              /(dLocalTargetCoordinates1[2] - dLocalTargetCoordinates[2]);

    // Check if the intersection point is contained in the actual counter volume (i.e. glass plate/gap)
    // If yes, create a CbmTofPoint for the beam traversing the counter
    if(tCurrentNode->GetShape()->Contains(dLocalCounterCoordinates))
    {
      // Transform the local beam point into the global coordinate system
      tCurrentNode->GetMatrix()->LocalToMaster(dLocalCounterCoordinates, dGlobalCounterCoordinates);

      // Retrieve the beam momentum in laboratory to calculate the time of flight between the
      // collision vertex in the target and the (global) beam point in the counter
      Double_t dBeamMomentumLab = FairRunSim::Instance()->GetBeamMom();
      Double_t dCounterBeamTime = dTargetVertexT;
      // Distance between the collision vertex point and the beam point in the counter
      // N.B.: The beam line is assumed to be parallel to Z
      Double_t dCounterTargetDistance = dGlobalCounterCoordinates[2] - dGlobalTargetCoordinates[2];

      if(0. < dBeamMomentumLab)
      {
        Double_t dBeamVelocityLab = dBeamMomentumLab/TMath::Sqrt(TMath::Power(dBeamMomentumLab, 2.) + TMath::Power(0.938271998, 2.))*TMath::Ccgs(); // [cm/s]
        dCounterBeamTime += dCounterTargetDistance/dBeamVelocityLab;
      }

      // Create the beam point in the CbmTofPoint collection
      tBeamPoint = new( (*fTofCollection)[fTofCollection->GetEntriesFast()] ) CbmTofPoint();
      tBeamPoint->SetDetectorID(iUniqueCounterId);
      tBeamPoint->SetTime(dCounterBeamTime);
      tBeamPoint->SetLength(dCounterTargetDistance);
      tBeamPoint->SetX(dGlobalCounterCoordinates[0]);
      tBeamPoint->SetY(dGlobalCounterCoordinates[1]);
      tBeamPoint->SetZ(dGlobalCounterCoordinates[2]);
      tBeamPoint->SetPz(dBeamMomentumLab);
      tBeamPoint->SetNCells(1);
    }

    iCounter++;
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
          stack->AddPoint(kTof);
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
      stack->AddPoint(kTof);

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
    // Create TGeoPhysicalNode objects for those counters eligible for beam points
    CreateInBeamNodes();
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


// -----   Public method SetCounterInactive   ------------------------------
void CbmTof::SetCounterInactive(Int_t iModuleType, Int_t iModuleIndex, Int_t iCounterIndex)
{
  fInactiveCounters.push_back( std::tuple<Int_t, Int_t, Int_t>(iModuleType, iModuleIndex, iCounterIndex) );
}
// -------------------------------------------------------------------------


// -----   Public method SetCounterInBeam   --------------------------------
void CbmTof::SetCounterInBeam(Int_t iModuleType, Int_t iModuleIndex, Int_t iCounterIndex)
{
  fCountersInBeam.push_back( std::pair<std::tuple<Int_t, Int_t, Int_t>, TString>(std::tuple<Int_t, Int_t, Int_t>(iModuleType, iModuleIndex, iCounterIndex), "") );
}
// -------------------------------------------------------------------------


// -----   Private method CreateInBeamNodes   ------------------------------
void CbmTof::CreateInBeamNodes()
{
  for(auto const & itCounterInBeam : fCountersInBeam)
  {
    fNodesInBeam.push_back(new TGeoPhysicalNode((itCounterInBeam.second).Data()));
  }
}
// -------------------------------------------------------------------------


// -----   Public method CreateInBeamNodes   -------------------------------
void CbmTof::ExpandNode(TGeoNode* fN)
{
  TGeoMatrix* Matrix =fN->GetMatrix();
  if(gGeoManager->GetListOfMatrices()->FindObject(Matrix)) { gGeoManager->GetListOfMatrices()->Remove(Matrix); }
  TGeoVolume* v1=fN->GetVolume();

  // Save the path to the physical ToF node
  if(TString(fN->GetName()).Contains("tof"))
  {
    gGeoManager->CdDown(gGeoManager->GetTopNode()->GetNdaughters() - 1);
    fTofNodePath = gGeoManager->GetPath();
    gGeoManager->CdTop();
  }

  TObjArray* NodeList=v1->GetNodes();
  for (Int_t Nod=0; Nod<NodeList->GetEntriesFast(); Nod++) {
    TGeoNode* fNode =static_cast<TGeoNode*>(NodeList->At(Nod));
    TGeoMatrix* M =fNode->GetMatrix();
    SetDefaultMatrixName(M);

    // Extract the current module type and module index from the module node
    if(TString(fNode->GetName()).Contains("module"))
    {
      fCurrentNodePath = fTofNodePath + "/" + (TString)fNode->GetName();

      boost::regex rgx(".*_(\\d+)_.*");
      boost::cmatch match;
      if( boost::regex_search(fNode->GetName(), match, rgx) )
      {
        fCurrentModuleType = boost::lexical_cast<Int_t>(match[1]);
      }

      fCurrentModuleIndex = fNode->GetNumber();
    }
    // Extract the current counter index from the counter node and find the
    // central node in the counter volume (a glass plate OR a gap) to serve as
    // reference coordinate system during (optional) beam point creation
    else if(TString(fNode->GetName()).Contains("counter"))
    {
      fCurrentNodePath += "/" + (TString)fNode->GetName();

      fCurrentCounterIndex = fNode->GetNumber();

      TString tCentralNodePath = fCurrentNodePath;

      Int_t iNGaps(0);

      for(Int_t iDaughter = 0; iDaughter < fNode->GetNdaughters(); iDaughter++)
      {
        if(TString(fNode->GetDaughter(iDaughter)->GetName()).Contains("Gap"))
        {
          iNGaps++;
        }
      }

      if(0 == iNGaps%2)
      {
        tCentralNodePath += TString::Format("/tof_glass_%d",iNGaps/2);
      }
      else
      {
        tCentralNodePath += TString::Format("/Gap_%d",(iNGaps-1)/2);
      }

      for(auto &itCounterInBeam : fCountersInBeam)
      {
        if(std::get<0>(itCounterInBeam.first) == fCurrentModuleType &&
           std::get<1>(itCounterInBeam.first) == fCurrentModuleIndex &&
           std::get<2>(itCounterInBeam.first) == fCurrentCounterIndex)
        {
          itCounterInBeam.second = tCentralNodePath;
        }
      }
    }
    else
    {
      fCurrentNodePath += "/" + (TString)fNode->GetName();
    }

    // Expand nodes recursively
    if(fNode->GetNdaughters()>0) { ExpandNode(fNode); }

    // Remove a node's name from the current node path upon completing a
    // recursion step
    fCurrentNodePath.ReplaceAll("/"+(TString)fNode->GetName(), "");

    TGeoVolume* v= fNode->GetVolume();
    AssignMediumAtImport(v);
    if (!gGeoManager->FindVolumeFast(v->GetName())) {
      LOG(DEBUG2)<<"Register Volume " << v->GetName()<<FairLogger::endl;
      v->RegisterYourself();
    }

    // Check if the current volume is eligible for CbmTof::ProcessHits treatment
    if ( CheckIfSensitive(v->GetName()) ) {
      LOG(DEBUG2)<<"Sensitive Volume "<< v->GetName() << FairLogger::endl;
      AddSensitiveVolume(v);
    }
  }
}
// -------------------------------------------------------------------------


// -----   Private method CheckIfSensitive   -------------------------------
Bool_t CbmTof::CheckIfSensitive(std::string name)
{
  // If the current Cell volume belongs to a counter declared inactive w.r.t.
  // Monte Carlo point creation, it is not declared sensitive
  TString tsname = name;
  if (tsname.Contains("Cell"))
  {
    for(auto const& itInactiveCounter : fInactiveCounters)
    {
      if(std::get<0>(itInactiveCounter) == fCurrentModuleType &&
         std::get<1>(itInactiveCounter) == fCurrentModuleIndex &&
         std::get<2>(itInactiveCounter) == fCurrentCounterIndex)
      {
        return kFALSE;
      }
    }

    return kTRUE;
  }
  return kFALSE;
}
// -------------------------------------------------------------------------


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

Bool_t CbmTof::IsNewGeometryFile(TString /*filename*/)
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


//__________________________________________________________________________
void CbmTof::SetDefaultMatrixName(TGeoMatrix* matrix)
{
  // Copied from root TGeoMatrix::SetDefaultName() and modified (memory leak)
  // If no name was supplied in the ctor, the type of transformation is checked.
  // A letter will be prepended to the name :
  //   t - translation
  //   r - rotation
  //   s - scale
  //   c - combi (translation + rotation)
  //   g - general (tr+rot+scale)
  // The index of the transformation in gGeoManager list of transformations will
  // be appended.
  if (!gGeoManager) { return; }
  if (strlen(matrix->GetName())) { return; }
  char type = 'n';
  if (matrix->IsTranslation()) { type = 't'; }
  if (matrix->IsRotation()) { type = 'r'; }
  if (matrix->IsScale()) { type = 's'; }
  if (matrix->IsCombi()) { type = 'c'; }
  if (matrix->IsGeneral()) { type = 'g'; }
  TObjArray* matrices = gGeoManager->GetListOfMatrices();
  Int_t index = 0;
  if (matrices) { index =matrices->GetEntriesFast() - 1; }
  matrix->SetName(Form("%c%i", type, index));
}
// -------------------------------------------------------------------------

//__________________________________________________________________________
void CbmTof::AssignMediumAtImport(TGeoVolume* v)
{

  /**
   * Assign medium to the the volume v, this has to be done in all cases:
   * case 1: For CAD converted volumes they have no mediums (only names)
   * case 2: TGeoVolumes, we need to be sure that the material is defined in this session
   */
  FairGeoMedia* Media       = FairGeoLoader::Instance()->getGeoInterface()->getMedia();
  FairGeoBuilder* geobuild  = FairGeoLoader::Instance()->getGeoBuilder();

  TGeoMedium* med1=v->GetMedium();


  if(med1) {
    // In newer ROOT version also a TGeoVolumeAssembly has a material and medium.
    // This medium is called dummy and is automatically set when the geometry is constructed.
    // Since this material and medium is neither in the TGeoManager (at this point) nor in our
    // ASCII file we have to create it the same way it is done in TGeoVolume::CreateDummyMedium()
    // In the end the new medium and material has to be added to the TGeomanager, because this is
    // not done automatically when using the default constructor. For all other constructors the
    // newly created medium or material is added to the TGeomanger.
    // Create the medium and material only the first time.
    TString medName = static_cast<TString>(med1->GetName());
    if ( medName.EqualTo("dummy") && NULL == gGeoManager->GetMedium(medName) ) {

      TGeoMaterial *dummyMaterial = new TGeoMaterial();
      dummyMaterial->SetName("dummy");

      TGeoMedium* dummyMedium = new TGeoMedium();
      dummyMedium->SetName("dummy");
      dummyMedium->SetMaterial(dummyMaterial);

      gGeoManager->GetListOfMedia()->Add(dummyMedium);
      gGeoManager->AddMaterial(dummyMaterial);
    }

    TGeoMaterial* mat1=v->GetMaterial();
    TGeoMaterial* newMat = gGeoManager->GetMaterial(mat1->GetName());
    if( newMat==0) {
      /**The Material is not defined in the TGeoManager, we try to create one if we have enough information about it*/
      FairGeoMedium* FairMedium=Media->getMedium(mat1->GetName());
      if (!FairMedium) {
        LOG(FATAL)<<"Material "<< mat1->GetName() << "is not defined in ASCII file nor in Root file." << FairLogger::endl;
        //     FairMedium=new FairGeoMedium(mat1->GetName());
        //      Media->addMedium(FairMedium);
      } else {

        Int_t nmed=geobuild->createMedium(FairMedium);
        v->SetMedium(gGeoManager->GetMedium(nmed));
        gGeoManager->SetAllIndex();
      }
    } else {
      /**Material is already available in the TGeoManager and we can set it */
      TGeoMedium* med2= gGeoManager->GetMedium(mat1->GetName());
      v->SetMedium(med2);
    }
  } else {
    if (strcmp(v->ClassName(),"TGeoVolumeAssembly") != 0) {
      //[R.K.-3.3.08]  // When there is NO material defined, set it to avoid conflicts in Geant
      LOG(FATAL)<<"The volume "<< v->GetName() << "has no medium information and not an Assembly so we have to quit"<<FairLogger::endl;
    }
  }
}
// -------------------------------------------------------------------------

ClassImp(CbmTof)
