// -------------------------------------------------------------------------
// -----                    CbmMvdHitfinder source file                -----
// -------------------------------------------------------------------------

// Includes from MVD
#include "CbmMvdHitfinder.h"
#include "CbmMvdPoint.h"
#include "plugins/tasks/CbmMvdSensorFindHitTask.h"
#include "SensorDataSheets/CbmMvdMimosa26AHR.h"
#include "tools/CbmMvdGeoHandler.h"


// Includes from FAIR
#include "FairRootManager.h"
#include "FairModule.h"

// Includes from ROOT
#include "TClonesArray.h"


// -----   Default constructor   ------------------------------------------
CbmMvdHitfinder::CbmMvdHitfinder()
  : FairTask("MVDHitfinder"),
    fDetector(NULL),
    fInputDigis(NULL),
    fHits(NULL),
    fHitMatch(NULL),
    fHitfinderPluginNr(0)
{
}
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
CbmMvdHitfinder::CbmMvdHitfinder(const char* name, Int_t iMode, Int_t iVerbose) 
  : FairTask(name, iVerbose),
    fDetector(NULL),
    fInputDigis(NULL),
    fHits(NULL),
    fHitMatch(NULL),
    fHitfinderPluginNr(0)
{
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmMvdHitfinder::~CbmMvdHitfinder() {
 
if ( fHits) 
    {
    fHits->Delete();
    delete fHits;
    }
}
// -----------------------------------------------------------------------------

// -----   Exec   --------------------------------------------------------------
void CbmMvdHitfinder::Exec(Option_t* opt){

fHits->Clear();

if(fInputDigis)
   {
   if(fVerbose)cout << endl << "//----------------------------------------//" << endl;
   if(fVerbose)cout << "Execute HitfinderPlugin Nr. "<< fHitfinderPluginNr << endl;
   fDetector->SendInputDigis(fInputDigis);
   fDetector->Exec(fHitfinderPluginNr);
   if(fVerbose)cout << "End Chain" << endl;
   if(fVerbose)cout << "Start writing Hits" << endl;  
   fHits->AbsorbObjects(fDetector->GetOutputHits()); 
   if(fVerbose)cout << "Total of " << fHits->GetEntriesFast() << " hits found" << endl;
   if(fVerbose)cout << "Finished writing Hits" << endl;
   if(fVerbose)cout << "//----------------------------------------//" << endl << endl;
   }
   
}
// -----------------------------------------------------------------------------

// -----   Init   --------------------------------------------------------------
InitStatus CbmMvdHitfinder::Init() {
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
    fInputDigis = (TClonesArray*) ioman->GetObject("MvdDigis"); 
   
    // **********  Register output array
    fHits = new TClonesArray("CbmMvdHit", 10000);
    ioman->Register("MvdHit", "Mvd Hits", fHits, kTRUE);

    fDetector = CbmMvdDetector::Instance();
    if(!fDetector)
    	{
	cout << endl << "No CbmMvdDetector detected!" << endl; 
        GetMvdGeometry(); 
	fDetector = CbmMvdDetector::Instance();
	}

    CbmMvdSensorFindHitTask* HitfinderTask = new CbmMvdSensorFindHitTask();
    fDetector->AddPlugin(HitfinderTask);
    fHitfinderPluginNr = (UInt_t) (fDetector->GetPluginArraySize());
    fDetector->Init(); 



    // Screen output
    cout << GetName() << " initialised with parameters: " << endl;
    //PrintParameters();
    cout << "---------------------------------------------" << endl;

       
    return kSUCCESS;
}

// -----   Virtual public method Reinit   ----------------------------------
InitStatus CbmMvdHitfinder::ReInit() {

    return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   Virtual method Finish   -----------------------------------------
void CbmMvdHitfinder::Finish() {

    PrintParameters();

}					       
// -------------------------------------------------------------------------



// -----   Private method Reset   ------------------------------------------
void CbmMvdHitfinder::Reset() {
    fHits->Delete();

}
// -------------------------------------------------------------------------  

// -----   Private method GetMvdGeometry   ---------------------------------
void CbmMvdHitfinder::GetMvdGeometry() {
 
CbmMvdDetector* Detector = new CbmMvdDetector("A");
CbmMvdGeoHandler* mvdHandler = new CbmMvdGeoHandler();
mvdHandler->Init();
mvdHandler->Fill();
}
// -------------------------------------------------------------------------  


// -----   Private method PrintParameters   --------------------------------
void CbmMvdHitfinder::PrintParameters() {
    
    cout.setf(ios_base::fixed, ios_base::floatfield);
    cout << "============================================================" << endl;
    cout << "============== Parameters Hitfinder ========================" << endl;
    cout << "============================================================" << endl;
    cout << "=============== End Task ===================================" << endl;
 
}
// -------------------------------------------------------------------------  

ClassImp(CbmMvdHitfinder);
