// -------------------------------------------------------------------------
// -----                        CbmMvd source file                     -----
// -----                  Created 26/07/04  by V. Friese               -----
// -------------------------------------------------------------------------

#include "CbmMvd.h"

#include "CbmMvdGeo.h"
#include "CbmMvdGeoPar.h"
#include "CbmMvdPoint.h"

#include "CbmDetectorList.h"
#include "FairGeoInterface.h"
#include "FairGeoLoader.h"
#include "FairGeoNode.h"
#include "FairGeoVolume.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "CbmStack.h"
#include "FairVolume.h"

#include "TClonesArray.h"
#include "TGeoManager.h"
#include "TList.h"
#include "TObjArray.h"
#include "TParticle.h"
#include "TString.h"
#include "TVirtualMC.h"
#include "TVirtualMCStack.h"

#include <iostream>
using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
CbmMvd::CbmMvd() 
  : FairDetector("MVD", kTRUE, kMVD), 
    fTrackID(0),
    fPdg(0),
    fVolumeID(0),
    fPosIn(0.0, 0.0, 0.0, 0.0),
    fPosOut(0.0, 0.0, 0.0, 0.0),
    fMomIn(0.0, 0.0, 0.0, 0.0),
    fMomOut(0.0, 0.0, 0.0, 0.0),
    fTime(0.),
    fLength(0.),
    fELoss(0.),
    fPosIndex(0),
    fCollection(new TClonesArray("CbmMvdPoint")),
    kGeoSaved(kFALSE),
    fGeoPar(new TList()),
    fStationMap()
{
  ResetParameters();
  fGeoPar->SetName( GetName());
  fVerboseLevel = 1;

}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmMvd::CbmMvd(const char* name, Bool_t active) 
  : FairDetector(name, active, kMVD),
    fTrackID(0),
    fPdg(0),
    fVolumeID(0),
    fPosIn(0.0, 0.0, 0.0, 0.0),
    fPosOut(0.0, 0.0, 0.0, 0.0),
    fMomIn(0.0, 0.0, 0.0, 0.0),
    fMomOut(0.0, 0.0, 0.0, 0.0),
    fTime(0.),
    fLength(0.),
    fELoss(0.),
    fPosIndex(0),
    fCollection(new TClonesArray("CbmMvdPoint")),
    kGeoSaved(kFALSE),
    fGeoPar(new TList()),
    fStationMap()
 {
  fGeoPar->SetName( GetName());
  fVerboseLevel = 1;
 
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmMvd::~CbmMvd() {
  if ( fGeoPar ) {
    fGeoPar->Delete();
    delete fGeoPar;
  }
  if (fCollection) {
    fCollection->Delete();
    delete fCollection;
  }
}
// -------------------------------------------------------------------------



// -----   Virtual public method ProcessHits  ------------------------------
Bool_t CbmMvd::ProcessHits(FairVolume* vol) {


  // Store track parameters at entrance of sensitive volume
  if ( gMC->IsTrackEntering() ) {
    fPdg    = gMC->TrackPid();
    fELoss  = 0.;
    fTime   = gMC->TrackTime() * 1.0e09;
    fLength = gMC->TrackLength();
    gMC->TrackPosition(fPosIn);
    gMC->TrackMomentum(fMomIn);
  }

  // Sum energy loss for all steps in the active volume
  fELoss += gMC->Edep();

  // Set additional parameters at exit of active volume. Create CbmMvdPoint.
  if ( gMC->IsTrackExiting()    ||
       gMC->IsTrackStop()       ||
       gMC->IsTrackDisappeared()   ) {
    fTrackID  = gMC->GetStack()->GetCurrentTrackNumber();
    fVolumeID = vol->getMCid();
    gMC->TrackPosition(fPosOut);
    gMC->TrackMomentum(fMomOut);
    if (fELoss == 0. ) return kFALSE;
    AddHit(fTrackID, fPdg, fStationMap[fVolumeID],
	   TVector3(fPosIn.X(),   fPosIn.Y(),   fPosIn.Z()),
	   TVector3(fPosOut.X(),  fPosOut.Y(),  fPosOut.Z()),
	   TVector3(fMomIn.Px(),  fMomIn.Py(),  fMomIn.Pz()),
	   TVector3(fMomOut.Px(), fMomOut.Py(), fMomOut.Pz()),
	   fTime, fLength, fELoss);

    // Increment number of MvdPoints for this track
    CbmStack* stack = (CbmStack*) gMC->GetStack();
    stack->AddPoint(kMVD);
    
    ResetParameters();
  }

  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Public method BeginEvent   --------------------------------------
void CbmMvd::BeginEvent() { }
// -------------------------------------------------------------------------



// -----   Virtual public method EndOfEvent   ------------------------------
void CbmMvd::EndOfEvent() {
  if (fVerboseLevel) Print();
  //  fCollection->Clear();
  fCollection->Delete();
  ResetParameters();
}
// -------------------------------------------------------------------------



// -----   Virtual public method Register   --------------------------------
void CbmMvd::Register() {
  FairRootManager::Instance()->Register("MvdPoint", GetName(), 
				       fCollection, kTRUE);
}
// -------------------------------------------------------------------------



// -----   Virtual public method GetCollection   ---------------------------
TClonesArray* CbmMvd::GetCollection(Int_t iColl) const {
  if (iColl == 0) return fCollection;
  else return NULL;
}
// -------------------------------------------------------------------------



// -----   Virtual public method Print   -----------------------------------
void CbmMvd::Print() const {
  Int_t nHits = fCollection->GetEntriesFast();
  cout << "-I- CbmMvd: " << nHits << " points registered in this event." 
       << endl;
}
// -------------------------------------------------------------------------



// -----   Virtual public method Reset   -----------------------------------
void CbmMvd::Reset() {
  //  fCollection->Clear();
  fCollection->Delete();
  ResetParameters();
}
// -------------------------------------------------------------------------



// -----   Virtual public method CopyClones   ------------------------------
void CbmMvd::CopyClones(TClonesArray* cl1, TClonesArray* cl2, Int_t offset) {
  Int_t nEntries = cl1->GetEntriesFast();
  cout << "-I- CbmMvd: " << nEntries << " entries to add." << endl;
  TClonesArray& clref = *cl2;
  CbmMvdPoint* oldpoint = NULL;
   for (Int_t i=0; i<nEntries; i++) {
   oldpoint = (CbmMvdPoint*) cl1->At(i);
    Int_t index = oldpoint->GetTrackID() + offset;
    oldpoint->SetTrackID(index);
    new (clref[fPosIndex]) CbmMvdPoint(*oldpoint);
    fPosIndex++;
  }
   cout << " -I- CbmMvd: " << cl2->GetEntriesFast() << " merged entries."
       << endl;
}
// -------------------------------------------------------------------------

// --------Pulic method ConstructGeometry()-----------------------------------------------------------------
void CbmMvd::ConstructGeometry()
{
  TString fileName = GetGeometryFileName();
  if ( fileName.EndsWith(".root") ) 
	{
	LOG(INFO) << "Constructing MVD  geometry from ROOT  file "
	<< fileName.Data() << FairLogger::endl;
    	ConstructRootGeometry();
  	}
  else if ( fileName.EndsWith(".geo") ) 
	{
    	LOG(INFO) <<  "Constructing MVD  geometry from ASCII file "
	<< fileName.Data() << FairLogger::endl;
    	ConstructAsciiGeometry();
 	}
  else
    LOG(FATAL) <<  "Geometry format of MVD file " << fileName.Data()
		 << " not supported." << FairLogger::endl;
}


// -----   Virtual public method ConstructAsciiGeometry   -----------------------
void CbmMvd::ConstructAsciiGeometry() {
  
  FairGeoLoader*    geoLoad = FairGeoLoader::Instance();
  FairGeoInterface* geoFace = geoLoad->getGeoInterface();
  CbmMvdGeo*       mvdGeo  = new CbmMvdGeo();
  mvdGeo->setGeomFile(GetGeometryFileName());
  geoFace->addGeoModule(mvdGeo);

  Bool_t rc = geoFace->readSet(mvdGeo);
  if (rc) mvdGeo->create(geoLoad->getGeoBuilder());
  TList* volList = mvdGeo->getListOfVolumes();
  // store geo parameter
  FairRun *fRun = FairRun::Instance();
  FairRuntimeDb *rtdb= FairRun::Instance()->GetRuntimeDb();
  CbmMvdGeoPar* geoPar = (CbmMvdGeoPar*)(rtdb->getContainer("CbmMvdGeoPar"));
  TObjArray *fSensNodes = geoPar->GetGeoSensitiveNodes();
  TObjArray *fPassNodes = geoPar->GetGeoPassiveNodes();

  TListIter iter(volList);
  FairGeoNode*   node = NULL;
  FairGeoVolume* vol  = NULL;

  while( (node = (FairGeoNode*)iter.Next()) ) {
    vol = dynamic_cast<FairGeoVolume*> ( node );
    if ( node->isSensitive()  ) {
	   LOG(DEBUG) << "Add: "<< vol->GetName() << FairLogger::endl;
           fSensNodes->AddLast( vol );
       }else{
           fPassNodes->AddLast( vol );
       }
  }
  geoPar->setChanged();
  geoPar->setInputVersion(fRun->GetRunId(),1);
  ProcessNodes( volList );
  
  // Fill map of station numbers
  Int_t iStation =  1;
  Int_t volId    = -1;
  do {
    TString volName = Form("mvdstation%02i", iStation);
    volId = gGeoManager->GetUID(volName);
    if (volId > -1 ) {
      fStationMap[volId] = iStation;
      LOG(INFO) << GetName() << "::ConstructAsciiGeometry: "
           << "Station No. " << iStation << ", volume ID " << volId 
	   << ", volume name " << volName << FairLogger::endl;
      iStation++;
    }
  } while ( volId > -1 );

 
}
// -------------------------------------------------------------------------

// --------    Public method ConstructRootGeometry     ---------------------
void CbmMvd::ConstructRootGeometry() // added 05.05.14 by P. Sitzmann
{
  /** Construct the detector geometry from ROOT files, possible inputs are:
   * 1. A TGeoVolume as a mother (master) volume containing the detector geometry
   * 2. A TGeoManager with the detector geometry
   * 3. A TGeoVolume as a mother or Master volume which is the output of the CAD2ROOT geometry, in this case
   *    the materials are not proprely defined and had to be reset
   *  In all cases we have to check that the material properties are the same or is the materials defined in
   *  the current simulation session
   */

  TGeoManager* OldGeo=gGeoManager;
  TGeoManager* NewGeo=0;
  TGeoVolume* volume=0;;
  TFile* f=new TFile(GetGeometryFileName().Data());
  TList* l= f->GetListOfKeys();
  TKey* key;
  TIter next( l);
  TGeoNode* n=0;
  TGeoNode* myNode=0;
  TGeoVolume* v1=0;
  TGeoVolume* myMotherVolume = 0;
  
  cout << endl << "Start ConstructRootGeometry in CbmMvd" << endl;
  while ((key = (TKey*)next())) {
    /**loop inside the delivered root file and try to fine a TGeoManager object
     * the first TGeoManager found will be read
     */
    if (strcmp(key->GetClassName(),"TGeoManager") != 0) { continue; }
    gGeoManager=0;
    NewGeo = (TGeoManager*)key->ReadObj();
    break;
  }
  if (NewGeo!=0) {
    /** in case a TGeoManager was found get the top most volume and the node
     */

    NewGeo->cd();
    volume=(TGeoVolume*)NewGeo->GetNode(0)->GetDaughter(0)->GetVolume();
    v1=volume->MakeCopyVolume(volume->GetShape());
    //n=NewGeo->GetTopNode();
    n=v1->GetNode(0);
    
    //  NewGeo=0;
    delete NewGeo;

  } else {
    /** The file does not contain any TGeoManager, so we assume to have a file with a TGeoVolume
     * try to look for a TGeoVolume inside the file
     */

    key=(TKey*) l->At(0);  //Get the first key in the list
    volume=dynamic_cast<TGeoVolume*> (key->ReadObj());
    if(volume!=0) { n=volume->GetNode(0); }
    if(n!=0) { 
      v1=n->GetVolume(); 
      n->Print();
    }
  }
  if(v1==0) {
    fLogger->Info(MESSAGE_ORIGIN, "\033[5m\033[31mFairModule::ConstructRootGeometry(): could not find any geometry in File!!  \033[0m", GetGeometryFileName().Data());
    exit(0);
  }
  gGeoManager=OldGeo;
  gGeoManager->cd();
  // If AddToVolume is empty add the volume to the top volume Cave
  // If it is defined check i´f the volume exists and if it exists add the volume from the root file
  // to the already existing volume
  TGeoVolume* Cave=NULL;
  if ( 0 == fMotherVolumeName.Length() ) {
    Cave= gGeoManager->GetTopVolume();
  } else {
    Cave = gGeoManager->GetVolume(fMotherVolumeName);
    if ( NULL == Cave ) {
      fLogger->Error(MESSAGE_ORIGIN,"\033[5m\033[31mFairModule::ConstructRootGeometry(): could not find the given mother volume \033[0m   %s \033[5m\033[31m where the geomanger should be added. \033[0m", fMotherVolumeName.Data());
      exit(0);
    }
  }
  /**Every thing is OK, we have a TGeoVolume and now we add it to the simulation TGeoManager  */
  gGeoManager->AddVolume(v1);
  /** force rebuilding of voxels */
  TGeoVoxelFinder* voxels = v1->GetVoxels();
  if (voxels) { voxels->SetNeedRebuild(); }
  /**To avoid having different names of the default matrices because we could have get the volume from another
   * TGeoManager, we reset the default matrix name
   */
  TGeoMatrix* M = n->GetMatrix();
  SetDefaultMatrixName(M);

  /** NOw we can remove the matrix so that the new geomanager will rebuild it properly*/
  gGeoManager->GetListOfMatrices()->Remove(M);
  TGeoHMatrix* global = gGeoManager->GetHMatrix();
  gGeoManager->GetListOfMatrices()->Remove(global); //Remove the Identity matrix
  /**Now we can add the node to the existing cave */
  Cave->AddNode(v1,0, M);
  /** correction from O. Merle: in case of a TGeoVolume (v1) set the material properly */
  AssignMediumAtImport(v1);
  /** now go through the herachy and set the materials properly, this is important becase the CAD converter
   *  produce TGeoVolumes with materials that have only names and no properties
   */
 
   ExpandNode(n);
  // Fill map of station numbers

  
  
  Int_t iStation =  1;
  Int_t volId    = -1;
  Int_t chois = 0;
  CbmMvdDetector* Detector = new CbmMvdDetector("A");
  TString nodeName; 
TString mother = "cave_1/" + fMotherVolumeName + "_0";
      if (!mother)
         {mother = "cave_1";}
cout << endl << "Try to find Geometry in " << mother << endl;

if ( gGeoManager->CheckPath(mother + "/Beamtimeosetupoobgnum_0"))
	{
	cout << endl << "Found Beamtimesetup" << endl;
	chois = 1;
	}
else if (gGeoManager->CheckPath(mother + "/MVDoMistraloquero012oStationo150umodigi_0"))
	{
	cout << endl << "Found MVD with 3 Stations" << endl;
	chois = 2;
	}
else if (gGeoManager->CheckPath(mother + "/MVDo0123ohoFPCoextoHSoSo0123_0"))
	{
	cout << endl << "Found MVD with 4 Stations" << endl;
	chois = 3;
	}
else 
	{cout << endl << "Try standart Geometry" << endl;}
         
switch (chois)
	{
case 1:
	cout << endl << "Start Beamtime" << endl;
	
  	cout << endl << "detected segmented geometry, start searching for Sensors" << endl; 
  	do {
    		TString volName = Form("MVD-S%i-A", iStation);
    		volId = gGeoManager->GetUID(volName);
    		if (volId > -1 ) 
			{
      			nodeName = Form("cave_1/Beamtimeosetupoobgnum_0/MVD-S%i-AoPartAss_1/MVD-S%i-A_1", iStation, iStation );
     			fStationMap[volId] = iStation;
      			cout << "-I- " << GetName() << "::ConstructGeometry: "
          		 << "Station No. " << iStation << ", volume ID " << volId 
	   		<< ", volume name " << volName << endl;
      			Detector->AddSensor (volName, volName, nodeName, new CbmMvdMimosa26AHR, iStation, volId, 				0.0);     
       			iStation++;
    			}
 	 } while ( volId > -1 );

	break;
case 2:
	cout << endl << "Start Mvd with 3 Stations" << endl;
 
  for(Int_t StatNr = 0; StatNr < 3; StatNr++)
      {
	for(Int_t QuadNr = 0; QuadNr < 4; QuadNr++)
	    {
	    
	      for(Int_t Layer = 0; Layer < 2; Layer++)
		  {
		  
		      for(Int_t SensNr = 0; SensNr < 100; SensNr++)
			  {
			    TString volName = Form("MVD-S%i-Q%i-L%i-C%02i-P0", StatNr, QuadNr, Layer, SensNr);
			    //cout << endl << "Trying to find " << volName << endl;
			    volId = gGeoManager->GetUID(volName);
			    //cout << endl << "VolId for this volume is " << volId << endl;
			
			    if (volId > -1 ) 
				{
			    for(Int_t SegmentNr = 0; SegmentNr < 100; SegmentNr++)
			       {
			    
				  switch(StatNr)
				    {
				    case 0:
 				      nodeName = Form("cave_1/MVDoMistraloquero012oStationo150umodigi_0/ersteoStationoMistraloquer_1/St0Q%i_1/S0Q%iS%i_1/MVD-S0-Q%i-L%i-C%02i-P0oPartAss_1/MVD-S0-Q%i-L%i-C%02i-P0_1", QuadNr, QuadNr, SegmentNr, QuadNr, Layer, SensNr, QuadNr, Layer, SensNr);
				      break;
				    case 1:  
 				      nodeName = Form("cave_1/MVDoMistraloquero012oStationo150umodigi_0/zweiteoStationoMistraloquer_1/St1Q%i_1/S1Q%iS%i_1/MVD-S1-Q%i-L%i-C%02i-P0oPartAss_1/MVD-S1-Q%i-L%i-C%02i-P0_1", QuadNr, QuadNr, SegmentNr, QuadNr, Layer, SensNr, QuadNr, Layer, SensNr);
				      break;
				    case 2:
 				      nodeName = Form("cave_1/MVDoMistraloquero012oStationo150umodigi_0/dritteoStationoMistraloquer_1/St2Q%i_1/S2Q%iS%i_1/MVD-S2-Q%i-L%i-C%02i-P0oPartAss_1/MVD-S2-Q%i-L%i-C%02i-P0_1", QuadNr, QuadNr, SegmentNr, QuadNr, Layer, SensNr, QuadNr, Layer, SensNr);
 				      break;     
				    }
				Bool_t nodeFound = gGeoManager->CheckPath(nodeName.Data());
				if (  nodeFound ) 
				    {
				    fStationMap[volId] = iStation;
				    cout << "-I- " << GetName() << "::ConstructGeometry: "
				    << "Station No. " << iStation << ", volume ID " << volId 
				    << ", volume name " << volName << endl;
				    Detector->AddSensor (volName, volName, nodeName, new CbmMvdMimosa26AHR, iStation, volId, 0.0);     
				    iStation++;
				    break;
				    }
				    
				}
			    
			  }
			
			
		      }  
		 }
	    }
      }
	break;
case 3:
	cout << endl << "Start Mvd with 4 Stations" << endl;

  for(Int_t StatNr = 0; StatNr < 4; StatNr++)
      {
	for(Int_t QuadNr = 0; QuadNr < 4; QuadNr++)
	    {
	    
	      for(Int_t Layer = 0; Layer < 2; Layer++)
		  {
		  
		      for(Int_t SensNr = 0; SensNr < 100; SensNr++)
			  {
			    
			    TString volName = Form("MVD-S%i-Q%i-L%i-C%02i-P0", StatNr, QuadNr, Layer, SensNr);
			    
			    volId = gGeoManager->GetUID(volName);
			   
			
			    if (volId > -1 ) 
				{
			    for(Int_t SegmentNr = 0; SegmentNr < 100; SegmentNr++)
			       {
			    
				  switch(StatNr)
				    {
				    case 0:
 				      nodeName = mother; nodeName += Form("/MVDo0123ohoFPCoextoHSoSo0123_0/MVDo0ohoFPCoHSoS_1/St0Q%iohoFPC_1/S0Q%iS%i_1/MVD-S0-Q%i-L%i-C%02i-P0oPartAss_1/MVD-S0-Q%i-L%i-C%02i-P0_1", QuadNr, QuadNr, SegmentNr, QuadNr, Layer, SensNr, QuadNr, Layer, SensNr);
					
				      break;
				    case 1:  
 				       nodeName = mother; nodeName += Form("/MVDo0123ohoFPCoextoHSoSo0123_0/MVDo1ohoFPCoextoHSoS_1/St1Q%iohoFPCoext_1/S1Q%iS%i_1/MVD-S1-Q%i-L%i-C%02i-P0oPartAss_1/MVD-S1-Q%i-L%i-C%02i-P0_1", QuadNr, QuadNr, SegmentNr, QuadNr, Layer, SensNr, QuadNr, Layer, SensNr);
				      break;
				    case 2:
 				       nodeName = mother; nodeName += Form("/MVDo0123ohoFPCoextoHSoSo0123_0/MVDo2ohoFPCoextoHSoS_1/St2Q%iohoFPCoext_1/S2Q%iS%i_1/MVD-S2-Q%i-L%i-C%02i-P0oPartAss_1/MVD-S2-Q%i-L%i-C%02i-P0_1", QuadNr, QuadNr, SegmentNr, QuadNr, Layer, SensNr, QuadNr, Layer, SensNr);
					break;
				    case 3:
					 nodeName = mother; nodeName += Form("/MVDo0123ohoFPCoextoHSoSo0123_0/MVDo3ohoFPCoextoHSoS_1/St3Q%iohoFPCoext_1/S3Q%iS%i_1/MVD-S3-Q%i-L%i-C%02i-P0oPartAss_1/MVD-S3-Q%i-L%i-C%02i-P0_1", QuadNr, QuadNr, SegmentNr, QuadNr, Layer, SensNr, QuadNr, Layer, SensNr);
 				      break;   
				    default: 
					break;  
				    }
				Bool_t nodeFound = gGeoManager->CheckPath(nodeName.Data());
				if (  nodeFound ) 
				    {
				    fStationMap[volId] = iStation;
				    //cout << "-I- " << GetName() << "::ConstructGeometry: "
				    //<< "Station No. " << iStation << ", volume ID " << volId 
				    //<< ", volume name " << volName << endl;
				    Detector->AddSensor (volName, volName, nodeName, new CbmMvdMimosa26AHR, iStation, volId, 0.0);     
				    iStation++;
				    break;
				    
				    } 
				    
				}
			    
			  }
			
			
		      }  
		 }
	    }
      }
	break;
default:
	cout << endl << "Start old Geometry" << endl;
	do {
      	 TString volName = Form("mvdstation%02i", iStation);
      	 volId = gGeoManager->GetUID(volName);
      	 if (volId > -1 ) 
  	  	{
          	fStationMap[volId] = iStation;
          	LOG(INFO) << GetName() << "::ConstructRootGeometry: "
          	    << "Station No. " << iStation << ", volume ID " << volId 
	  	    << ", volume name " << volName << FairLogger::endl;
         	 iStation++;
         	 } else{;}
       	 } while ( volId > -1 );
	break;
	}
}
// -------------------------------------------------------------------------


// -----   Private method AddHit   --------------------------------------------
CbmMvdPoint* CbmMvd::AddHit(Int_t trackID, Int_t pdg, Int_t stationNr, 
			    TVector3 posIn, TVector3 posOut, 
			    TVector3 momIn, TVector3 momOut, Double_t time, 
			    Double_t length, Double_t eLoss) {
  TClonesArray& clref = *fCollection;
  Int_t size = clref.GetEntriesFast();
  if (fVerboseLevel>1) 
    cout << "-I- CbmMvd: Adding Point at (" << posIn.X() << ", " << posIn.Y() 
	 << ", " << posIn.Z() << ") cm,  station " << stationNr << ", track "
	 << trackID << ", energy loss " << eLoss*1e06 << " keV" << endl;
  return new(clref[size]) CbmMvdPoint(trackID, pdg, stationNr, posIn, posOut,
				      momIn, momOut, time, length, eLoss);
}
// ----------------------------------------------------------------------------

Bool_t CbmMvd::CheckIfSensitive(std::string name)
{
  TString tsname = name; 
  if (tsname.Contains("MimosaActive") || tsname.Contains("mvdstation") && !(tsname.Contains("PartAss")) ){
    //cout<<"*** Register "<<tsname<<" as active volume."<<endl;
    return kTRUE;
  }
  else if(tsname.EndsWith("-P0"))    
      {
      std::cout<<"*** Register "<<tsname<<" as active volume."<<endl;
      
      return kTRUE;
      }
  return kFALSE;
}
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

void CbmMvd::SetDefaultMatrixName(TGeoMatrix* matrix) // added 05.05.14 by P. Sitzmann, needed by ConstructRootGeometry
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
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
void CbmMvd::AssignMediumAtImport(TGeoVolume* v) // added 05.05.14 by P. Sitzmann, needed by ConstructRootGeometry
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
    TGeoMaterial* mat1=v->GetMaterial();
    TGeoMaterial* newMat = gGeoManager->GetMaterial(mat1->GetName());
    if( newMat==0) {
      /**The Material is not defined in the TGeoManager, we try to create one if we have enough information about it*/
      FairGeoMedium* FairMedium=Media->getMedium(mat1->GetName());
      if (!FairMedium) {
        fLogger->Debug(MESSAGE_ORIGIN,"Material %s is not defined in ASCII file nor in Root file we try to create one", mat1->GetName());
        FairMedium=new FairGeoMedium(mat1->GetName());
        Media->addMedium(FairMedium);
      }

      Int_t nmed=geobuild->createMedium(FairMedium);
      v->SetMedium(gGeoManager->GetMedium(nmed));
      gGeoManager->SetAllIndex();
    } else {
      /**Material is already available in the TGeoManager and we can set it */
      TGeoMedium* med2= gGeoManager->GetMedium(mat1->GetName());
      v->SetMedium(med2);
    }
  } else {
    if (strcmp(v->ClassName(),"TGeoVolumeAssembly") != 0) {
      //[R.K.-3.3.08]  // When there is NO material defined, set it to avoid conflicts in Geant
      fLogger->Error(MESSAGE_ORIGIN,"The volume  %s  Has no medium information and not an Assembly so we have to quit", v->GetName());
      abort();
    }
  }
}
// ----------------------------------------------------------------------------

ClassImp(CbmMvd)
