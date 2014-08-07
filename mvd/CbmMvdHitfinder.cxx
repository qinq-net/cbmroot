// -------------------------------------------------------------------------
// -----                    CbmMvdHitfinder source file                -----
// -------------------------------------------------------------------------

// Includes from MVD
#include "CbmMvdHitfinder.h"
#include "CbmMvdPoint.h"
#include "plugins/tasks/CbmMvdSensorFindHitTask.h"
#include "SensorDataSheets/CbmMvdMimosa26AHR.h"

// Includes from FAIR
#include "FairRootManager.h"
#include "FairModule.h"

// Includes from ROOT
#include "TClonesArray.h"


// -----   Default constructor   ------------------------------------------
CbmMvdHitfinder::CbmMvdHitfinder(): FairTask("MVDHitfinder"){

}
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
CbmMvdHitfinder::CbmMvdHitfinder(const char* name, Int_t iMode, Int_t iVerbose) : FairTask(name, iVerbose){

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
   cout << endl << "//----------------------------------------//" << endl;
   cout << "Execute HitfinderPlugin Nr. "<< fHitfinderPluginNr << endl;
   fDetector->Exec(fHitfinderPluginNr);
   cout << "End Chain" << endl;
   cout << "Start writing Hits" << endl;  
   fHits->AbsorbObjects(fDetector->GetOutputHits()); 
   cout << "Finished writing Hits" << endl;
    cout << "//----------------------------------------//" << endl;
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
    ioman->Register("MvdHits", "MvdHit", fHits, kTRUE);

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
 
  Int_t iStation =  1;
  Int_t volId    = -1;
  Int_t chois = 0;
  CbmMvdDetector* Detector = new CbmMvdDetector("A");
  TString nodeName;
  TString mother = "cave1/pipevac1";
      if (gGeoManager->CheckPath(mother.Data()))
         {mother = "cave_1";}
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
void CbmMvdHitfinder::PrintParameters() {
    
    cout.setf(ios_base::fixed, ios_base::floatfield);
    cout << "============================================================" << endl;
    cout << "============== Parameters Hitfinder ========================" << endl;
    cout << "============================================================" << endl;
    cout << "=============== End Task ===================================" << endl;
 
}
// -------------------------------------------------------------------------  

ClassImp(CbmMvdHitfinder);
