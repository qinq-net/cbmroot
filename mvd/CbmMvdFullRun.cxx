// -------------------------------------------------------------------------
// -----                    CbmMvdFullRun  source file                -----
// -------------------------------------------------------------------------

// Includes from MVD
#include "CbmMvdFullRun.h"


#include "CbmMvdGeoPar.h"
#include "CbmMvdHit.h"
#include "CbmMvdHitMatch.h"
#include "CbmMvdPileupManager.h"
#include "CbmMvdPoint.h"
#include "CbmMvdStation.h"
#include "CbmMvdSensor.h"
#include "SensorDataSheets/CbmMvdMimosa26AHR.h"



#include "plugins/buffers/CbmMvdSensorFrameBuffer.h"
#include "plugins/buffers/CbmMvdSensorTrackingBuffer.h"
#include "plugins/tasks/CbmMvdSensorDigitizerTask.h"
#include "plugins/tasks/CbmMvdSensorFindHitTask.h"
//#include "omp.h"

// Includes from base
#include "FairGeoNode.h"
#include "FairRootManager.h"
#include "FairRunSim.h"
#include "FairRuntimeDb.h"
#include "CbmMCTrack.h"

// Includes from ROOT
#include "TArrayD.h"
#include "TClonesArray.h"
#include "TGeoManager.h"
//#include "TGeoShape.h"
#include "TGeoTube.h"
#include "TObjArray.h"
#include "TRandom3.h"
#include "TString.h"
#include "TVector3.h"
#include "TMath.h"
#include "TH1.h"
#include "TH2.h"

// Includes from C++
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>

//#include "gsl/gsl_sf_erf.h"
//#include "CLHEP/Random/RandGauss.h"
//#include "CLHEP/Random/RandPoisson.h"
//#include "CLHEP/Random/RandFlat.h"


using std::cout;
using std::endl;
using std::map;
using std::setw;
using std::left;
using std::right;
using std::fixed;
using std::pair;
using std::setprecision;
using std::ios_base;
using std::vector;


// -----   Default constructor   ------------------------------------------
CbmMvdFullRun::CbmMvdFullRun()
    : FairTask("MVDSensorBuffer")
{
    bWriteOutput   = true;
    
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmMvdFullRun::CbmMvdFullRun(const char* name, Int_t iMode,
				 Int_t iVerbose)
: FairTask(name, iVerbose)
{
    cout << "Starting CbmMvdFullRun::CbmMvdFullRun::() "<< endl;

    fMode          = iMode;
    bWriteOutput = true; 
   
 }


// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmMvdFullRun::~CbmMvdFullRun() {


}




// -----   Virtual public method Exec   ------------------------------------
void CbmMvdFullRun::Exec(Option_t* opt) {

fHits->Clear("C");

if(fInputPoints->GetEntriesFast() > 0)
{
cout << endl << "Send Input" << endl;
fDetector->SendInput(fInputPoints);
cout << endl << "Execute Chain" << endl;
fDetector->ExecChain();
cout << endl << "End Chain" << endl;


if(bWriteOutput)
    {
    cout << "Start writing Hits" << endl;  
    fDigis->AbsorbObjects(fDetector->GetOutputDigis()); 
    fHits->AbsorbObjects(fDetector->GetOutputHits());
    fHits->Compress();
    cout << endl;
    }
}
}// end of exec

// -------------------------------------------------------------------------


			      
// -----    Virtual private method Init   ----------------------------------
InitStatus CbmMvdFullRun::Init() {

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
    fHits =  new TClonesArray("CbmMvdHit", 10000);
    fDigis = new TClonesArray("CbmMvdDigi", 10000);
    ioman->Register("MvdPluginHits", "MvdHit", fHits, kTRUE);
    ioman->Register("MvdPluginDigis", "MvdDigi", fDigis, kTRUE);

    // Screen output
    cout << GetName() << " initialised with parameters: " << endl;
    //PrintParameters();
    cout << "---------------------------------------------" << endl;

    fDetector = CbmMvdDetector::Instance();
    
     if(!fDetector)
    	{
	cout << endl << "No CbmMvdDetector detected!" << endl; 
        GetMvdGeometry(); 
	fDetector = CbmMvdDetector::Instance();
	}    

    CbmMvdSensorFrameBuffer* frameBuffer = new CbmMvdSensorFrameBuffer();
    CbmMvdSensorDigitizerTask* digiTask = new CbmMvdSensorDigitizerTask();
    CbmMvdSensorFindHitTask* findTask = new CbmMvdSensorFindHitTask();
  //  CbmMvdSensorTrackingBuffer* trackingBuffer = new CbmMvdSensorTrackingBuffer;
   

  fDetector->AddPlugin(frameBuffer);
  fDetector->AddPlugin(digiTask);
  fDetector->AddPlugin(findTask);
    //fDetector->AddPlugin(trackingBuffer);
    
    fDetector->Init();
    
    
    return kSUCCESS;

 

}
// -------------------------------------------------------------------------



// -----   Virtual public method Reinit   ----------------------------------
InitStatus CbmMvdFullRun::ReInit() {

    return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   Virtual method Finish   -----------------------------------------
void CbmMvdFullRun::Finish() {

    PrintParameters();

}					       
// -------------------------------------------------------------------------



// -----   Private method Reset   ------------------------------------------
void CbmMvdFullRun::Reset() {
    fHits->Delete();

}
// -------------------------------------------------------------------------  



// -----   Private method PrintParameters   --------------------------------
void CbmMvdFullRun::PrintParameters() {
    
    cout.setf(ios_base::fixed, ios_base::floatfield);
    cout << "============================================================" << endl;
    cout << "============== Parameters Buffer Task ======================" << endl;
    cout << "============================================================" << endl;
    
    
    cout << "=============== End Task ===================================" << endl;
 
}
// -------------------------------------------------------------------------  

// -----   Private method GetMvdGeometry   ---------------------------------
void CbmMvdFullRun::GetMvdGeometry() {
 
  Int_t iStation =  1;
  Int_t volId    = -1;
  Int_t chois = 0;
  CbmMvdDetector* Detector = new CbmMvdDetector("A");
  TString nodeName;

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
 				      nodeName = Form("cave_1/MVDo0123ohoFPCoextoHSoSo0123_0/MVDo0ohoFPCoHSoS_1/St0Q%iohoFPC_1/S0Q%iS%i_1/MVD-S0-Q%i-L%i-C%02i-P0oPartAss_1/MVD-S0-Q%i-L%i-C%02i-P0_1", QuadNr, QuadNr, SegmentNr, QuadNr, Layer, SensNr, QuadNr, Layer, SensNr);
					
				      break;
				    case 1:  
 				      nodeName = Form("cave_1/MVDo0123ohoFPCoextoHSoSo0123_0/MVDo1ohoFPCoextoHSoS_1/St1Q%iohoFPCoext_1/S1Q%iS%i_1/MVD-S1-Q%i-L%i-C%02i-P0oPartAss_1/MVD-S1-Q%i-L%i-C%02i-P0_1", QuadNr, QuadNr, SegmentNr, QuadNr, Layer, SensNr, QuadNr, Layer, SensNr);
				      break;
				    case 2:
 				      nodeName = Form("cave_1/MVDo0123ohoFPCoextoHSoSo0123_0/MVDo2ohoFPCoextoHSoS_1/St2Q%iohoFPCoext_1/S2Q%iS%i_1/MVD-S2-Q%i-L%i-C%02i-P0oPartAss_1/MVD-S2-Q%i-L%i-C%02i-P0_1", QuadNr, QuadNr, SegmentNr, QuadNr, Layer, SensNr, QuadNr, Layer, SensNr);
					break;
				    case 3:
					nodeName = Form("cave_1/MVDo0123ohoFPCoextoHSoSo0123_0/MVDo3ohoFPCoextoHSoS_1/St3Q%iohoFPCoext_1/S3Q%iS%i_1/MVD-S3-Q%i-L%i-C%02i-P0oPartAss_1/MVD-S3-Q%i-L%i-C%02i-P0_1", QuadNr, QuadNr, SegmentNr, QuadNr, Layer, SensNr, QuadNr, Layer, SensNr);
 				      break;   
				    default: 
					break;  
				    }
				Bool_t nodeFound = gGeoManager->CheckPath(nodeName.Data());
				if (  nodeFound ) 
				    {
				 // cout << endl << "found new sensor" << endl;
	 Detector->AddSensor(volName, volName, nodeName, new CbmMvdMimosa26AHR, iStation, volId, 0.0);     
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


ClassImp(CbmMvdFullRun);
