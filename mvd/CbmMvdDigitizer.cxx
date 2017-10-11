// -------------------------------------------------------------------------
// -----                    CbmMvdDigitizer source file                -----
// -------------------------------------------------------------------------

// Includes from MVD
#include "CbmMvdDigitizer.h"
#include "CbmMvdPoint.h"
#include "plugins/tasks/CbmMvdSensorDigitizerTask.h"
#include "SensorDataSheets/CbmMvdMimosa26AHR.h"
#include "tools/CbmMvdGeoHandler.h"
#include "CbmMvdPileupManager.h"
#include "CbmMvdDetector.h"


// Includes from FAIR
#include "FairRootManager.h"
#include "FairModule.h"
#include "FairLogger.h"

// Includes from ROOT
#include "TClonesArray.h"
#include "TStopwatch.h"


// Includes from C++
#include <iomanip>
#include <iostream>
#include <vector>


using std::cout;
using std::endl;
using std::vector;

// -----   Default constructor   ------------------------------------------
CbmMvdDigitizer::CbmMvdDigitizer() 
  : FairTask("MVDDigitizer"),
    fMode(0),
    fShowDebugHistos(kFALSE),
    fNoiseSensors(kFALSE),
    fDetector(NULL),
    fSensorTyp(CbmMvdSensorTyp::MIMOSA26),
    fInputPoints(NULL),
    fDigis(NULL),
    fDigiMatch(NULL),
    fMcPileUp(NULL),
    fPerformanceDigi(),
    fDigiPluginNr(0),
    fFakeRate(-1.),
    fNPileup(-1),
    fNDeltaElect(-1),
    fDeltaBufferSize(-1),
    fBgBufferSize(-1),
    epsilon(),
    fBranchName(""),
    fBgFileName(""),
    fDeltaFileName(""),
    fTimer(),
    fPileupManager(NULL),
    fDeltaManager(NULL)
{

}
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
CbmMvdDigitizer::CbmMvdDigitizer(const char* name, Int_t iMode, Int_t iVerbose) 
  : FairTask(name, iVerbose),
    fMode(iMode),
    fShowDebugHistos(kFALSE),
    fNoiseSensors(kFALSE),
    fDetector(NULL),
    fSensorTyp(CbmMvdSensorTyp::MIMOSA26),
    fInputPoints(NULL),
    fDigis(NULL),
    fDigiMatch(NULL),
    fMcPileUp(NULL),
    fPerformanceDigi(),
    fDigiPluginNr(0),
    fFakeRate(-1.),
    fNPileup(0),
    fNDeltaElect(0),
    fDeltaBufferSize(-1),
    fBgBufferSize(-1),
    epsilon(),
    fBranchName("MvdPoint"),
    fBgFileName(""),
    fDeltaFileName(""),
    fTimer(),
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
void CbmMvdDigitizer::Exec(Option_t* /*opt*/){
    // --- Start timer
using namespace std;

fTimer.Start();
	
fDigis->Delete();
fDigiMatch->Delete();
fMcPileUp->Delete();

BuildEvent();
if(fInputPoints->GetEntriesFast() > 0)
   {
   if(fVerbose) cout << "//----------------------------------------//";
   if(fVerbose) cout << endl << "Send Input" << endl;
   fDetector->SendInput(fInputPoints);
   LOG(DEBUG) << "Execute DigitizerPlugin Nr. "<< fDigiPluginNr << FairLogger::endl;
   fDetector->Exec(fDigiPluginNr);
   if(fVerbose) cout << "End Chain" << endl;
   if(fVerbose) cout << "Start writing Digis" << endl;  
   fDigis->AbsorbObjects(fDetector->GetOutputDigis(),0,fDetector->GetOutputArray(fDigiPluginNr)->GetEntriesFast()-1); 
   LOG(DEBUG) << "Total of " << fDigis->GetEntriesFast() << " digis in this Event" << FairLogger::endl;
   if(fVerbose) cout << "Start writing DigiMatchs" << endl;  
   fDigiMatch->AbsorbObjects(fDetector->GetOutputDigiMatchs(),0,fDetector->GetOutputDigiMatchs()->GetEntriesFast()-1);
   if(fVerbose) cout << "Total of " << fDigiMatch->GetEntriesFast() << " digisMatch in this Event" << endl; 
   if(fVerbose) cout  << "//----------------------------------------//" << endl ;
   LOG(INFO) << "+ " << setw(20) << GetName() << ": Created: " 
        << fDigis->GetEntriesFast() << " digis in " 
        << fixed << setprecision(6) << fTimer.RealTime() << " s" << FairLogger::endl;
   }

fTimer.Stop();
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

    if (! fInputPoints ) {
      LOG(ERROR) << "No MvdPoint branch found. There was no MVD in the simulation. Switch this task off" << FairLogger::endl;
      return kERROR;
    }

   
    // **********  Register output array
    fDigis = new TClonesArray("CbmMvdDigi", 10000);
    ioman->Register("MvdDigi", "Mvd Digis", fDigis, IsOutputBranchPersistent("MvdDigi"));

    fDigiMatch = new TClonesArray("CbmMatch", 100000);
    ioman->Register("MvdDigiMatch", "Mvd DigiMatches", fDigiMatch, IsOutputBranchPersistent("MvdDigiMatch"));

    fMcPileUp = new TClonesArray("CbmMvdPoint", 10000);
    ioman->Register("MvdPileUpMC", "Mvd MC Points after Pile Up", fMcPileUp, IsOutputBranchPersistent("MvdPileUpMC"));

    CbmMvdDetector::SetSensorTyp(fSensorTyp);
    fDetector = CbmMvdDetector::Instance();

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
	  Fatal(GetName(), "The size of your BG-File is insufficient");  
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
	  Fatal(GetName(), "The size of your Delta-File is insufficient");  
	  return kERROR;}
	}
    CbmMvdSensorDigitizerTask* digiTask = new CbmMvdSensorDigitizerTask();
   
    fDetector->AddPlugin(digiTask);
    fDigiPluginNr = (UInt_t) (fDetector->GetPluginArraySize());
    if(fShowDebugHistos) fDetector->ShowDebugHistos();
    fDetector->Init();

    if(fNoiseSensors) fDetector->SetProduceNoise();

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
   // cout<< endl << "finishing" << endl;
    fDetector->Finish();
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

}
// -------------------------------------------------------------------------  



// -----   Private method PrintParameters   --------------------------------
void CbmMvdDigitizer::PrintParameters() {

using namespace std;

    cout.setf(ios_base::fixed, ios_base::floatfield);
    cout << "============================================================" << endl;
    cout << "============== Parameters MvdDigitizer =====================" << endl;
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
    nOrig = fInputPoints->GetEntriesFast();


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
	nPile++;
	new((*fInputPoints)[fInputPoints->GetEntriesFast()]) CbmMvdPoint(*((CbmMvdPoint*)points->At(iPoint)));
      }

	
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
	new((*fInputPoints)[fInputPoints->GetEntriesFast()]) CbmMvdPoint(*((CbmMvdPoint*)pointsD->At(iPoint)));
      nElec++;
      }


    }  // Delta electron event loop

  }    // Usage of delta

  for(Int_t i = 0; i < fInputPoints->GetEntriesFast(); i++)
      new((*fMcPileUp)[i]) CbmMvdPoint(*((CbmMvdPoint*)fInputPoints->At(i)));


  // ----- At last: Screen output
  LOG(INFO) << "+ " << GetName() << "::BuildEvent: original "
			   << nOrig << ", pileup " << nPile << ", delta "
			   << nElec << ", total " << nOrig+nPile+nElec
			   << " MvdPoints" << FairLogger::endl;


}
// -----------------------------------------------------------------------------


ClassImp(CbmMvdDigitizer);
