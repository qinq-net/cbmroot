// -------------------------------------------------------------------------
// -----                    CbmMvdClusterfinderTB source file                -----
// -------------------------------------------------------------------------

// Includes from MVD
#include "CbmMvdClusterfinderTB.h"
#include "CbmMvdPoint.h"
#include "CbmEvent.h"
#include "plugins/tasks/CbmMvdSensorClusterfinderTask.h"
#include "tools/CbmMvdGeoHandler.h"
#include "CbmMvdDetector.h"

// Includes from FAIR
#include "FairRootManager.h"
#include "FairModule.h"


// Includes from ROOT
#include "TClonesArray.h"
#include "TGeoManager.h"

#include "TString.h"
#include "TMath.h"



// Includes from C++
#include <iomanip>
#include <iostream>

using std::cout;
using std::endl;
using std::setw;
using std::setprecision;
using std::fixed;

// -----   Default constructor   ------------------------------------------
CbmMvdClusterfinderTB::CbmMvdClusterfinderTB() 
  :CbmMvdClusterfinderTB("MVDCLusterfinderTB", 0, 0)  
{
}
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
CbmMvdClusterfinderTB::CbmMvdClusterfinderTB(const char* name, Int_t iMode, Int_t iVerbose) 
  : FairTask(name, iVerbose),
    fMode(iMode),
    fShowDebugHistos(kFALSE),
    fDetector(NULL),
    fInputDigis(NULL),
    fCluster(NULL),
    fClusterPluginNr(0),
    fBranchName("MvdDigi"),
    fTimer()
{
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmMvdClusterfinderTB::~CbmMvdClusterfinderTB() {
 
if ( fCluster) 
    {
    fCluster->Delete();
    delete fCluster;
    }
}
// -----------------------------------------------------------------------------

// -----   Exec   --------------------------------------------------------------
void CbmMvdClusterfinderTB::Exec(Option_t* /*opt*/){
// --- Start timer
fTimer.Start();
	
fCluster->Delete();

Int_t nEvents = fEvents->GetEntriesFast();
for(Int_t iEv = 0; iEv < nEvents; ++iEv)
{
    LOG(DEBUG) << "Getting data from CbmEvent" <<  FairLogger:: endl;
    CbmEvent* event = dynamic_cast<CbmEvent*>(fEvents->At(iEv));
    Int_t eventNr = event->GetNumber();
    Int_t nrOfDigis = event->GetNofData(kMvdDigi);
    fEventDigis->Delete();
    for(Int_t nDigi = 0; nDigi < nrOfDigis; ++nDigi)
    {
     Int_t iDigi = event->GetIndex(kMvdDigi, nDigi);
     fEventDigis->AddLast((CbmMvdDigi*)fInputDigis->At(iDigi));
    }
   LOG(DEBUG) << "//----------------------------------------//";
   LOG(DEBUG) << endl << "Send Input" <<  FairLogger::endl;
   fDetector->SendInputDigis(fEventDigis);
   LOG(DEBUG) << "Execute ClusterPlugin Nr. "<< fClusterPluginNr << FairLogger::endl;
   fDetector->Exec(fClusterPluginNr);
   LOG(DEBUG) << "End Chain" <<  FairLogger::endl;
   LOG(DEBUG) << "Start writing Cluster" <<  FairLogger::endl;
   fCluster->AbsorbObjects(fDetector->GetOutputCluster(),0,fDetector->GetOutputCluster()->GetEntriesFast()-1); 
   LOG(DEBUG) << "Total of " << fCluster->GetEntriesFast() << " Cluster in this Event" << FairLogger:: endl;
   LOG(DEBUG) << "//----------------------------------------//" <<  FairLogger::endl ;
   LOG(INFO) << "+ " << setw(20) << GetName() << ": Created: " 
        << fCluster->GetEntriesFast() << " cluster in " 
        << fixed << setprecision(6) << fTimer.RealTime() << " s" << FairLogger::endl;
}
fTimer.Stop();
}
// -----------------------------------------------------------------------------

// -----   Init   --------------------------------------------------------------
InitStatus CbmMvdClusterfinderTB::Init() {
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
    fEvents =  (TClonesArray*)ioman->GetObject("Event");

    fInputDigis = (TClonesArray*) ioman->GetObject("MvdDigi"); 
    fEventDigis = new TClonesArray("CbmMvdDigi", 10000);
    if (! fInputDigis ) {
      LOG(ERROR) << "No MvdDigi branch found. There was no MVD in the simulation. Switch this task off" << FairLogger::endl;
      return kERROR;
    }
   
    // **********  Register output array
    fCluster = new TClonesArray("CbmMvdCluster", 10000);
    ioman->Register("MvdCluster", "Mvd Clusters", fCluster, IsOutputBranchPersistent("MvdCluster"));

    fDetector = CbmMvdDetector::Instance();
    
	if(fDetector->GetSensorArraySize() > 1)
		{
		    LOG(DEBUG) << "-I- succesfully loaded Geometry from file -I-" << FairLogger::endl;
		}
	else
		{
		LOG(FATAL) <<  "Geometry couldn't be loaded from file. No MVD digitizer available."
	        << FairLogger::endl;
		}

    CbmMvdSensorClusterfinderTask* clusterTask = new CbmMvdSensorClusterfinderTask();
   
    fDetector->AddPlugin(clusterTask);
    fClusterPluginNr = (UInt_t) (fDetector->GetPluginArraySize());
    if(fShowDebugHistos)fDetector->ShowDebugHistos();
    fDetector->Init();
   

    // Screen output
    LOG(INFO) << GetName() << " initialised" << FairLogger::endl;

    return kSUCCESS;
}

// -----   Virtual public method Reinit   ----------------------------------
InitStatus CbmMvdClusterfinderTB::ReInit() {

    return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   Virtual method Finish   -----------------------------------------
void CbmMvdClusterfinderTB::Finish() {
    fDetector->Finish();
    PrintParameters();

}					       
// -------------------------------------------------------------------------



// -----   Private method Reset   ------------------------------------------
void CbmMvdClusterfinderTB::Reset() {
    fCluster->Delete();

}
// -------------------------------------------------------------------------  

// -----   Private method GetMvdGeometry   ---------------------------------
void CbmMvdClusterfinderTB::GetMvdGeometry() {

}
// -------------------------------------------------------------------------  



// -----   Private method PrintParameters   --------------------------------
void CbmMvdClusterfinderTB::PrintParameters() {
    
    cout << "============================================================" << endl;
    cout << "============== Parameters Clusterfinder ====================" << endl;
    cout << "============================================================" << endl;
    cout << "=============== End Task ===================================" << endl;
 
}
// -------------------------------------------------------------------------  



ClassImp(CbmMvdClusterfinderTB);
