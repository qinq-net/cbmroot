// -------------------------------------------------------------------------
// -----                    CbmMvdHitfinderTB source file                -----
// -------------------------------------------------------------------------

// Includes from MVD
#include "CbmMvdHitfinderTB.h"
#include "CbmMvdPoint.h"
#include "plugins/tasks/CbmMvdSensorFindHitTask.h"
#include "plugins/tasks/CbmMvdSensorHitfinderTask.h"
#include "SensorDataSheets/CbmMvdMimosa26AHR.h"
#include "tools/CbmMvdGeoHandler.h"


// Includes from FAIR
#include "FairRootManager.h"
#include "FairModule.h"

// Includes from ROOT
#include "TClonesArray.h"


// Includes from C++
#include <iomanip>
#include <iostream>

using std::setw;
using std::fixed;
using std::setprecision;
using std::ios_base;
using std::cout;
using std::endl;

// -----   Default constructor   ------------------------------------------
CbmMvdHitfinderTB::CbmMvdHitfinderTB()
  : FairTask("MVDHitfinder"),
    fDetector(NULL),
    fInputDigis(NULL),
    fInputCluster(NULL),
    fHits(NULL),
    fHitfinderPluginNr(0),
    useClusterfinder(kFALSE),
    fShowDebugHistos(kFALSE),
    fTimer(),
    fmode(-1)
{
}
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
CbmMvdHitfinderTB::CbmMvdHitfinderTB(const char* name, Int_t iVerbose)
  : FairTask(name, iVerbose),
    fDetector(NULL),
    fInputDigis(NULL),
    fInputCluster(NULL),
    fHits(NULL),
    fHitfinderPluginNr(0),
    useClusterfinder(kFALSE),
    fShowDebugHistos(kFALSE),
    fTimer(),
    fmode(-1)
{
}
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
CbmMvdHitfinderTB::CbmMvdHitfinderTB(const char* name, Int_t mode, Int_t iVerbose)
  : FairTask(name, iVerbose),
    fDetector(NULL),
    fInputDigis(NULL),
    fInputCluster(NULL),
    fHits(NULL),
    fHitfinderPluginNr(0),
    useClusterfinder(kFALSE),
    fShowDebugHistos(kFALSE),
    fTimer(),
    fmode(mode)
{
//    fmode = mode;
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmMvdHitfinderTB::~CbmMvdHitfinderTB() {
 
if ( fHits) 
    {
    fHits->Delete();
    delete fHits;
    }
}
// -----------------------------------------------------------------------------

// -----   Exec   --------------------------------------------------------------
void CbmMvdHitfinderTB::Exec(Option_t* /*opt*/){

using namespace std;

fHits->Clear();
fTimer.Start();

   LOG(DEBUG) << endl << "//----------------------------------------//" <<  FairLogger::endl;
   fDetector->SendInputCluster(fInputCluster);
   LOG(DEBUG) << "Execute HitfinderPlugin Nr. "<< fHitfinderPluginNr <<  FairLogger::endl;
   fDetector->Exec(fHitfinderPluginNr);
   LOG(DEBUG) << "End Chain" << endl;
   LOG(DEBUG) << "Start writing Hits" << endl;
   fHits->AbsorbObjects(fDetector->GetOutputHits(),0,fDetector->GetOutputHits()->GetEntriesFast()-1); 
   LOG(DEBUG) << "Total of " << fHits->GetEntriesFast() << " hits found" <<  FairLogger::endl;
   LOG(DEBUG) << "Finished writing Hits" << endl;
   LOG(DEBUG) << "//----------------------------------------//" << endl <<  FairLogger::endl;
   LOG(INFO) << "+ " << setw(20) << GetName() << ": Created: " 
        << fHits->GetEntriesFast() << " hits in " 
        << fixed << setprecision(6) << fTimer.RealTime() << " s" << FairLogger::endl;
fTimer.Stop();
}
// -----------------------------------------------------------------------------

// -----   Init   --------------------------------------------------------------
InitStatus CbmMvdHitfinderTB::Init() {

    using namespace std;

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

    fInputCluster = (TClonesArray*) ioman->GetObject("MvdCluster");
    if (! fInputCluster )
       {
        LOG(ERROR) << "No MvdCluster branch found. There was no MVD in the simulation. Switch this task off" << FairLogger::endl;
        return kERROR;
       }

    // **********  Register output array
    fHits = new TClonesArray("CbmMvdHit", 10000);
    ioman->Register("MvdHit", "Mvd Hits", fHits, IsOutputBranchPersistent("MvdHit"));

    fDetector = CbmMvdDetector::Instance();

    CbmMvdSensorHitfinderTask* HitfinderTask = new CbmMvdSensorHitfinderTask();
    fDetector->AddPlugin(HitfinderTask);

    fHitfinderPluginNr = (UInt_t) (fDetector->GetPluginArraySize());
    if(fShowDebugHistos) fDetector->ShowDebugHistos();
    fDetector->Init();

    // Screen output
    cout << GetName() << " initialised with parameters: " << endl;
    //PrintParameters();
    cout << "---------------------------------------------" << endl;

       
    return kSUCCESS;
}

// -----   Virtual public method Reinit   ----------------------------------
InitStatus CbmMvdHitfinderTB::ReInit() {

    return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   Virtual method Finish   -----------------------------------------
void CbmMvdHitfinderTB::Finish() {

    PrintParameters();

}					       
// -------------------------------------------------------------------------



// -----   Private method Reset   ------------------------------------------
void CbmMvdHitfinderTB::Reset() {
    fHits->Delete();

}
// -------------------------------------------------------------------------  

// -----   Private method GetMvdGeometry   ---------------------------------
void CbmMvdHitfinderTB::GetMvdGeometry() {
 
}
// -------------------------------------------------------------------------  


// -----   Private method PrintParameters   --------------------------------
void CbmMvdHitfinderTB::PrintParameters() {

using namespace std;

    cout.setf(ios_base::fixed, ios_base::floatfield);
    cout << "============================================================" << endl;
    cout << "============== Parameters MvdHitfinder =====================" << endl;
    cout << "============================================================" << endl;
    cout << "=============== End Task ===================================" << endl;
 
}
// -------------------------------------------------------------------------  

ClassImp(CbmMvdHitfinderTB);
