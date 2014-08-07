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
CbmMvdDigitizer::CbmMvdDigitizer(): FairTask("MVDDigitizer"){

}
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
CbmMvdDigitizer::CbmMvdDigitizer(const char* name, Int_t iMode, Int_t iVerbose) : FairTask(name, iVerbose){

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

if(fInputPoints->GetEntriesFast() > 0)
   {
   cout << endl << "//----------------------------------------//";
   cout << endl << "Send Input" << endl;
   fDetector->SendInput(fInputPoints);
   cout << "Execute DigitizerPlugin Nr. "<< fDigiPluginNr << endl;
   fDetector->Exec(fDigiPluginNr);
   cout << "End Chain" << endl;
   cout << "Start writing Digis" << endl;  
   fDigis->AbsorbObjects(fDetector->GetOutputDigis()); 
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
	cout << endl << "No CbmMvdDetector detected!" << endl; 
        GetMvdGeometry(); 
	fDetector = CbmMvdDetector::Instance();
	if(fDetector)
	cout << endl << "succesfull loaded Geometry from file" << endl;
	else
	{
	cout << endl << "Error now Detector found" << endl;
	exit(-1);
	}
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
void CbmMvdDigitizer::PrintParameters() {
    
    cout.setf(ios_base::fixed, ios_base::floatfield);
    cout << "============================================================" << endl;
    cout << "============== Parameters Digitizer ========================" << endl;
    cout << "============================================================" << endl;
    cout << "=============== End Task ===================================" << endl;
 
}
// -------------------------------------------------------------------------  

ClassImp(CbmMvdDigitizer);
