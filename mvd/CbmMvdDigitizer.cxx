// -------------------------------------------------------------------------
// -----                    CbmMvdDigitizer source file                -----
// -------------------------------------------------------------------------

// Includes from MVD
#include "CbmMvdDigitizer.h"
#include "CbmMvdPoint.h"
#include "plugins/tasks/CbmMvdSensorDigitizerTask.h"
#include "SensorDataSheets/CbmMvdMimosa26AHR.h"

// Includes from FAIR
#include "FairRootManager.h"
#include "FairModule.h"

// Includes from ROOT
#include "TClonesArray.h"


// -----   Default constructor   ------------------------------------------
CbmMvdDigitizer::CbmMvdDigitizer() 
  : FairTask("MVDDigitizer"),
    fMode(0),
    fDetector(NULL),
    fInputPoints(NULL),
    fDigis(NULL),
    fDigiMatch(NULL),
    fDigiPluginNr(0),
    fFakeRate(-1.),
    fNPileup(-1),
    fNDeltaElect(-1),
    fDeltaBufferSize(-1),
    fBgBufferSize(-1),
    fBranchName(""),
    fBgFileName(""),
    fDeltaFileName(""),
    fRandGen(),
    fPileupManager(NULL),
    fDeltaManager(NULL)
{

}
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
CbmMvdDigitizer::CbmMvdDigitizer(const char* name, Int_t iMode, Int_t iVerbose) 
  : FairTask(name, iVerbose),
    fMode(iMode),
    fDetector(NULL),
    fInputPoints(NULL),
    fDigis(NULL),
    fDigiMatch(NULL),
    fDigiPluginNr(0),
    fFakeRate(-1.),
    fNPileup(0),
    fNDeltaElect(0),
    fDeltaBufferSize(-1),
    fBgBufferSize(-1),
    fBranchName("MvdPoint"),
    fBgFileName(""),
    fDeltaFileName(""),
    fRandGen(),
    fPileupManager(NULL),
    fDeltaManager(NULL)    
{
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmMvdDigitizer::~CbmMvdDigitizer() {
 
if ( fDigis) 
    {
    fDigis->Delete();
    delete fDigis;
    }
}
// -----------------------------------------------------------------------------

// -----   Exec   --------------------------------------------------------------
void CbmMvdDigitizer::Exec(Option_t* opt){

fDigis->Clear();
BuildEvent();
if(fInputPoints->GetEntriesFast() > 0)
   {
   cout << "//----------------------------------------//";
   cout << endl << "Send Input" << endl;
   fDetector->SendInput(fInputPoints);
   cout << "Execute DigitizerPlugin Nr. "<< fDigiPluginNr << endl;
   fDetector->Exec(fDigiPluginNr);
   cout << "End Chain" << endl;
   cout << "Start writing Digis" << endl;  
   fDigis->AbsorbObjects(fDetector->GetOutputDigis()); 
   cout << "Total of " << fDigis->GetEntriesFast() << " digis in this Event" << endl;
   cout << "Start writing DigiMatchs" << endl;  
   fDigiMatch->AbsorbObjects(fDetector->GetOutputDigiMatchs()); 
    cout  << "//----------------------------------------//" << endl ;
   }
}
// -----------------------------------------------------------------------------

// -----   Init   --------------------------------------------------------------
InitStatus CbmMvdDigitizer::Init() {
  cout << "-I- " << GetName() << ": Initialisation..." << endl;
  cout << endl;
  cout << "---------------------------------------------" << endl;
  cout << "-I- Initialising " << GetName() << " ...." << endl;

    // **********  RootManager
    FairRootManager* ioman = FairRootManager::Instance();
    if ( ! ioman ) 
	{
	cout << "-E- " << GetName() << "::Init: No FairRootManager!" << endl;
	return kFATAL;
	}

    // **********  Get input arrays
    fInputPoints = (TClonesArray*) ioman->GetObject("MvdPoint"); 
   
    // **********  Register output array
    fDigis = new TClonesArray("CbmMvdDigi", 10000);
    ioman->Register("MvdDigis", "MvdDigi", fDigis, kTRUE);

    fDigiMatch = new TClonesArray("CbmMvdDigiMatch", 100000);
    ioman->Register("MvdDigiMatchs", "MvdDigiMatch", fDigiMatch, kTRUE);

    fDetector = CbmMvdDetector::Instance();
    
    if(!fDetector)
    	{
	cout << endl << "-I- Try to load CbmMvdDetector -I- " << endl; 
        GetMvdGeometry(); 
	fDetector = CbmMvdDetector::Instance();
	if(fDetector->GetSensorArraySize() > 1)
		{
		cout << endl << "-I- succesfully loaded Geometry from file -I-" << endl;
		}
	else
		{
		cout << endl << "-E- Error no Detector found -E-" << endl;
		exit(-1);
		}
	} 


       // **********  Create pileup manager if necessary
    if (fNPileup >= 1 && !(fPileupManager) && fMode == 0 ) {
	if ( fBgFileName == "" ) {
	    cout << "-E- " << GetName() << "::Init: Pileup events needed, but no "
		<< " background file name given! " << endl;
	    return kERROR;
	}
	fPileupManager = new CbmMvdPileupManager(fBgFileName,
						 fBranchName, fBgBufferSize);
	if(fPileupManager->GetNEvents()< 2* fNPileup) {
	  cout <<"-E- "<< GetName() << ": The size of your BG-File is insufficient to perform the requested pileup" << endl;
	  cout <<"    You need at least events > 2* fNPileup." << endl;
	  cout <<"    Detected: fPileUp = " << fNPileup << ", events in file " << fPileupManager->GetNEvents() << endl; 
	  Fatal("","");  
	  return kERROR;}
    }

    // **********   Create delta electron manager if required
    if (fNDeltaElect >= 1 && !(fDeltaManager) && fMode == 0 ) {
	if ( fDeltaFileName == "" ) {
	    cout << "-E- " << GetName() << "::Init: Pileup events needed, but no "
		<< " background file name given! " << endl;
	    return kERROR;
	}
	fDeltaManager = new CbmMvdPileupManager(fDeltaFileName,
						fBranchName, fDeltaBufferSize);
	if(fDeltaManager->GetNEvents()< 2* fNDeltaElect ) {
	  cout <<"-E- "<< GetName() << ": The size of your Delta-File is insufficient to perform the requested pileup" << endl;
	  cout <<"    You need at least events > 2* fNDeltaElect." << endl;
	  cout <<"    Detected: fNDeltaElect = " << fNDeltaElect << ", events in file " << fDeltaManager->GetNEvents() << endl; 
	  Fatal("","");
	  return kERROR;}
	}
    CbmMvdSensorDigitizerTask* digiTask = new CbmMvdSensorDigitizerTask();
   
    fDetector->AddPlugin(digiTask);
    fDigiPluginNr = (UInt_t) (fDetector->GetPluginArraySize());
    fDetector->Init();
   

    // Screen output
    cout << GetName() << " initialised with parameters: " << endl;
    //PrintParameters();
    cout << "---------------------------------------------" << endl;

       
    return kSUCCESS;
}

// -----   Virtual public method Reinit   ----------------------------------
InitStatus CbmMvdDigitizer::ReInit() {

    return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   Virtual method Finish   -----------------------------------------
void CbmMvdDigitizer::Finish() {

    PrintParameters();

}					       
// -------------------------------------------------------------------------



// -----   Private method Reset   ------------------------------------------
void CbmMvdDigitizer::Reset() {
    fDigis->Delete();

}
// -------------------------------------------------------------------------  

// -----   Private method GetMvdGeometry   ---------------------------------
void CbmMvdDigitizer::GetMvdGeometry() {
 
  Int_t iStation =  0;
  Int_t volId    = -1;
  CbmMvdDetector* Detector = new CbmMvdDetector("A"); 
  Detector->SetMisalignment(epsilon);
  TString nodeName;
  TString mother;
  TString pipeName = "pipevac1";
  Int_t pipeID;
  TGeoNode* pipeNode;
  TString motherName; 
  mother = "cave1/pipevac1";

      if (!gGeoManager->CheckPath(mother.Data()))
         {
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

 cout << endl << "MotherNode is : " << mother << endl;
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
}
// -------------------------------------------------------------------------  



// -----   Private method PrintParameters   --------------------------------
void CbmMvdDigitizer::PrintParameters() {
    
    cout.setf(ios_base::fixed, ios_base::floatfield);
    cout << "============================================================" << endl;
    cout << "============== Parameters Digitizer ========================" << endl;
    cout << "============================================================" << endl;
    cout << "=============== End Task ===================================" << endl;
 
}
// -------------------------------------------------------------------------  

// ---------------  Build Event  ------------------------------------------------------
void CbmMvdDigitizer::BuildEvent() {

  // Some frequently used variables
  CbmMvdPoint*   point   = NULL;
  Int_t nOrig = 0;
  Int_t nPile = 0;
  Int_t nElec = 0;

  // ----- First treat standard input file
  for (Int_t i=0; i<fInputPoints->GetEntriesFast(); i++) {
    point = (CbmMvdPoint*) fInputPoints->At(i);
    point->SetPointId(i);
    nOrig++;
  }


  // ----- Then treat event pileup
  if (fNPileup>0) {
   
    // --- Vector of available background events from pile-up. 
    // --- Each event is used only once.
    Int_t nBuffer = fPileupManager->GetNEvents();
    vector<Int_t> freeEvents(nBuffer);
    for (Int_t i=0; i<nBuffer; i++) freeEvents[i] = i;

    // --- Loop over pile-up events
    for (Int_t iBg=0; iBg<fNPileup; iBg++) {

      // Select random event from vector and remove it after usage
      Int_t index = gRandom->Integer(freeEvents.size());
      Int_t iEvent = freeEvents[index];
      TClonesArray* points = fPileupManager->GetEvent(iEvent);
      freeEvents.erase(freeEvents.begin() + index);

      // Add points from this event to the input arrays
      for (Int_t iPoint=0; iPoint<points->GetEntriesFast(); iPoint++) {
	point = (CbmMvdPoint*) points->At(iPoint);
	point->SetTrackID(-2);
	point->SetPointId(-2);
	nPile++;
      }
      fInputPoints->AbsorbObjects(points);
	
    }   // Pileup event loop

  }    // Usage of pile-up

			   
  // ----- Finally, treat delta electrons
  if (fNDeltaElect>0) {
    
    // --- Vector of available delta events.
    // --- Each event is used only once.
    Int_t nDeltaBuffer = fDeltaManager->GetNEvents();
    vector<Int_t> freeDeltaEvents(nDeltaBuffer);
    for (Int_t i=0; i<nDeltaBuffer; i++) freeDeltaEvents[i] = i;

    // --- Loop over delta events
    for (Int_t it=0; it<fNDeltaElect; it++) {

      // Select random event from vector and remove it after usage
      Int_t indexD  = gRandom->Integer(freeDeltaEvents.size());
      Int_t iEventD = freeDeltaEvents[indexD];
      TClonesArray* pointsD = fDeltaManager->GetEvent(iEventD);
      freeDeltaEvents.erase(freeDeltaEvents.begin() + indexD);

      // Add points from this event to the input arrays
      for (Int_t iPoint=0; iPoint<pointsD->GetEntriesFast(); iPoint++) {
	point = (CbmMvdPoint*) pointsD->At(iPoint);
	point->SetTrackID(-3); // Mark the points as delta electron
	point->SetPointId(-3);
      nElec++;
      }
     fInputPoints->AbsorbObjects(pointsD);

    }  // Delta electron event loop

  }    // Usage of delta


  // ----- At last: Screen output
  cout << endl << "-I- " << GetName() << "::BuildEvent: original "
			   << nOrig << ", pileup " << nPile << ", delta "
			   << nElec << ", total " << nOrig+nPile+nElec
			   << " MvdPoints" << endl;


}
// -----------------------------------------------------------------------------


ClassImp(CbmMvdDigitizer);
