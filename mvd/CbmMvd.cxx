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

FairDetector::ConstructRootGeometry();

Int_t chois = 0;
  Int_t iStation =  0;
  Int_t volId    = -1;
  CbmMvdDetector* Detector = new CbmMvdDetector("A"); 
  TString nodeName;
  TString mother;
  TString pipeName = "pipevac1";
  Int_t pipeID;
  TGeoNode* pipeNode;
  TString motherName; 
  mother = "cave_1/pipevac1_0";

      if (!gGeoManager->CheckPath(mother.Data()))
         {
        if(fVerboseLevel) cout << endl << "pipevac1 not found in cave. Looking for Pipe..." << endl;
	pipeID = gGeoManager->GetUID(pipeName);
 	pipeNode = gGeoManager->GetNode(pipeID);
        gGeoManager->CdTop();
	gGeoManager->CdDown(0);
	motherName=gGeoManager->GetPath();
	mother = motherName;
	mother += "/";
	mother += pipeName;
	mother += "_0";
	gGeoManager->CdTop();
	}
      else
	mother = "cave_1/pipevac1_0";
cout << endl << "-I- Try to find Geometry in " << mother << endl;

if ( gGeoManager->CheckPath(mother + "/Beamtimeosetupoobgnum_0"))
	{
	cout << endl << "Found Beamtimesetup" << endl;
	chois = 1;
	}
else if (gGeoManager->CheckPath(mother + "/MVDoMistraloquero012oStationo150umodigi_0"))
	{
	cout << "Found MVD with 3 Stations" << endl;
	chois = 2;
	}
else if (gGeoManager->CheckPath(mother + "/MVDo0123ohoFPCoextoHSoSo0123_0"))
	{
	cout << "-I- Found MVD with 4 Stations" << endl;
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
	if (iStation > 1)
		cout << "-I- Finished building MVD Geometry" << endl << endl;
	break;
default:
	cout << endl << "Start old Geometry" << endl;
	iStation = 1;
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
	if(fVerboseLevel>1)
      std::cout<<"*** Register "<<tsname<<" as active volume."<<endl;
      
      return kTRUE;
      }
  return kFALSE;
}
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------



ClassImp(CbmMvd)
