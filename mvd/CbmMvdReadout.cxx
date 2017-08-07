// -------------------------------------------------------------------------
// -----                    CbmMvdClusterfinder source file                -----
// -------------------------------------------------------------------------

// Includes from MVD
#include "CbmMvdReadout.h"
#include "CbmMvdDetector.h"
#include "CbmMvdSensorReadoutTask.h"

// Includes from FAIR
#include "FairRootManager.h"
#include "FairModule.h"

// Includes from ROOT
#include "TClonesArray.h"


// Includes from C++
#include <iomanip>
#include <iostream>


using std::cout;
using std::endl;

// -----   Default constructor   ------------------------------------------
CbmMvdReadout::CbmMvdReadout() 
    : CbmMvdReadout("MVDClusterfinder")
{
}
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
CbmMvdReadout::CbmMvdReadout(const char* name)
    : FairTask(name),
    fMode(-1), 
    fPluginNr(0),
    fShowDebugHistos(kFALSE),
    fDetector(nullptr),
    fInputDigis(nullptr)
{
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmMvdReadout::~CbmMvdReadout() {

}
// -----------------------------------------------------------------------------

// -----   Exec   --------------------------------------------------------------
void CbmMvdReadout::Exec(Option_t* /*opt*/){

if(fInputDigis && fInputDigis->GetEntriesFast() > 0)
   {
   if(fVerbose) cout << "//----------------------------------------//";
   if(fVerbose) cout << endl << "Send Input" << endl;
   fDetector->SendInputDigis(fInputDigis);
   if(fVerbose) cout << "Execute ReadoutPlugin Nr. "<< fPluginNr << endl;
   fDetector->Exec(fPluginNr);
   if(fVerbose) cout << "End Chain" << endl;
   }


}
// -----------------------------------------------------------------------------

// -----   Init   --------------------------------------------------------------
InitStatus CbmMvdReadout::Init() {
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
    fInputDigis = (TClonesArray*) ioman->GetObject("MvdDigi"); 

    if (! fInputDigis ) {
      LOG(ERROR) << "No MvdDigi branch found. There was no MVD in the simulation. Switch this task off" << FairLogger::endl;
      return kERROR;
    }

    fDetector = CbmMvdDetector::Instance();
    
	if(fDetector->GetSensorArraySize() > 1)
		{
		 if(fVerbose) cout << endl << "-I- succesfully loaded Geometry from file -I-" << endl;
		}
	else
		{
		LOG(FATAL) <<  "Geometry couldn't be loaded from file. No MVD digitizer available."
	        << FairLogger::endl;
		}

    CbmMvdSensorReadoutTask* readerTask = new CbmMvdSensorReadoutTask();
   
    fDetector->AddPlugin(readerTask);
    fPluginNr = (UInt_t) (fDetector->GetPluginArraySize());
    fDetector->Init();
   

    // Screen output
    cout << GetName() << " initialised with parameters: " << endl;
    //PrintParameters();
    cout << "---------------------------------------------" << endl;

       
    return kSUCCESS;
}

// -----   Virtual public method Reinit   ----------------------------------
InitStatus CbmMvdReadout::ReInit() {

    return kSUCCESS;
}
// -------------------------------------------------------------------------


// -----   Virtual method Finish   -----------------------------------------
void CbmMvdReadout::Finish() {
    fDetector->Finish();
    PrintParameters();

}					       
// -------------------------------------------------------------------------



// -----   Private method Reset   ------------------------------------------
void CbmMvdReadout::Reset() {
   // fCluster->Delete();

}
// -------------------------------------------------------------------------

// -----   Private method PrintParameters   --------------------------------
void CbmMvdReadout::PrintParameters() const {
    
    //cout.setf(ios_base::fixed, ios_base::floatfield);
    cout << "============================================================" << endl;
    cout << "============== Parameters Readout ===== ====================" << endl;
    cout << "============================================================" << endl;
    cout << "=============== End Task ===================================" << endl;
 
}
// -------------------------------------------------------------------------  



ClassImp(CbmMvdReadout);
