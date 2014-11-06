// -------------------------------------------------------------------------
// -----                     CbmMvdDetector source file                  -----
// -----                  Created 31/01/11  by M. Deveaux              -----
// -------------------------------------------------------------------------

#include "CbmMvdDetector.h"

#include <iostream>


using std::cout;
using std::endl;

//_____________________________________________________________________________
CbmMvdDetector* CbmMvdDetector::fInstance= 0;
//_____________________________________________________________________________
CbmMvdDetector* CbmMvdDetector::Instance()
{
  return fInstance;
}

// -----   Default constructor   -------------------------------------------
CbmMvdDetector::CbmMvdDetector() 
  : TNamed(),
    fSensorArray(NULL),
    fSensorIDArray(NULL),
    fPluginCount(0),
    foutput(NULL),
    foutputHits(NULL),
    foutputDigis(NULL),
    foutputDigiMatchs(NULL),
    foutputHitMatchs(NULL),
    fcurrentEvent(NULL),
    fDigiPlugin(-1),
    fHitPlugin(-1),
    fSensorArrayFilled(kFALSE),
    initialized(kFALSE),
    fepsilon(),
    fParameter()
{

  Fatal (GetName(), " - Do not use standard constructor");

}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmMvdDetector::CbmMvdDetector(const char* name)
  : TNamed(),
    fSensorArray(new TClonesArray("CbmMvdSensor", 10)),
    fSensorIDArray(NULL),
    fPluginCount(0),
    foutput(NULL),
    foutputHits(NULL),
    foutputDigis(NULL),
    foutputDigiMatchs(NULL),
    foutputHitMatchs(NULL),
    fcurrentEvent(NULL),
    fDigiPlugin(-1),
    fHitPlugin(-1),
    fSensorArrayFilled(kFALSE),
    initialized(kFALSE),
    fepsilon(),
    fParameter()
{
 
  if(fInstance) {Fatal(GetName(), " - Error, singleton does already exist.");}
  else {fInstance=this;};
  fepsilon[0]=fepsilon[1]=fepsilon[2]=0;
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmMvdDetector::~CbmMvdDetector() {
  
}
//-----------------------------------------------------------------------

// -------Setters -----------------------------------------------------

void CbmMvdDetector::AddSensor(TString clearName, TString fullName, TString nodeName, 
			       CbmMvdSensorDataSheet* sensorData, Int_t sensorNr, Int_t volumeId, Double_t sensorStartTime, Int_t stationNr) {
 
  /**
   * 
   * new sensor is registered in sensor array 
   * 
   * **/
  
  if (fSensorArrayFilled) {Fatal (GetName(), " - Error, must add all sensors before adding plugins.");}
  
  
  Int_t nSensors=fSensorArray->GetEntriesFast();
 
  new ((*fSensorArray)[nSensors]) CbmMvdSensor(clearName, sensorData, fullName, nodeName,sensorNr,volumeId,sensorStartTime);
  //CbmMvdSensor(const char* name, CbmMvdSensorDataSheet* dataSheet, TString volName,
  //TString nodeName, Int_t stationNr, Int_t volumeId, Double_t sensorStartTime);
 
  CbmMvdSensor* sensor = (CbmMvdSensor*)fSensorArray->At(nSensors);
  sensor->SetDataSheet(sensorData);
  sensor->SetStation(stationNr);

    Float_t misalignment[3], randArray[3];
    TRandom3* rand = new TRandom3(0);
    rand->RndmArray(3,randArray);
    misalignment[0] = ((2*randArray[0])-1) * fepsilon[0]; //cout << endl << "calculated misalignment from: " << fepsilon[0] << " und " << randArray[0] << " to " << misalignment[0] << endl;
    misalignment[1] = ((2*randArray[0])-1) * fepsilon[1]; //cout << endl << misalignment[1] << endl;
    misalignment[2] = ((2*randArray[0])-1) * fepsilon[2]; //cout << endl << misalignment[2] << endl;
    sensor->SetMisalignment(misalignment);
  //cout << endl << "new sensor " << fullName << " to detector added" << endl;
} 
 
// ----------------------------------------------------------------------


//-----------------------------------------------------------------------
void CbmMvdDetector::AddPlugin(CbmMvdSensorPlugin* plugin) {
  
/**
 * if there is a new buffer or task typ you have to insert it here 
 * or you can't use it.
 */
  fSensorArrayFilled=kTRUE;
  fPluginCount++;
  CbmMvdSensor* sensor;
  Int_t nSensors=fSensorArray->GetEntriesFast();
  const TString digitizername = "CbmMvdSensorDigitizerTask";
  const TString findername = "CbmMvdSensorFindHitTask";
  const TString framename = "CbmMvdSensorFrameBuffer";
  const TString trackingname = "CbmMvdSensorTrackingBuffer";
  


  for (Int_t i=0;i<nSensors; i++){
   
    
    if(plugin->GetPluginType() == task)
      {
      
      if (plugin->ClassName() == digitizername)
	  {
	  CbmMvdSensorDigitizerTask* digiTask = new CbmMvdSensorDigitizerTask();
	  sensor=(CbmMvdSensor*)fSensorArray->At(i);
	  sensor->AddPlugin(digiTask);
	 // cout <<  "Adding Task CbmMvdSensorDigitizerTask at Sensor " << sensor->GetName() << endl;
	  }
      else if (plugin->ClassName() == findername)
	  {
	  CbmMvdSensorFindHitTask* findTask = new CbmMvdSensorFindHitTask();
	  sensor=(CbmMvdSensor*)fSensorArray->At(i);
	  sensor->AddPlugin(findTask);
	 // cout <<  "Adding Task CbmMvdSensorFindHitTask at Sensor " << sensor->GetName() << endl;
	  }
      
      
      else 
	  {
	   cout << endl << "task not included yet, adding standart task." << endl;
	   CbmMvdSensorTask* task = new CbmMvdSensorTask();
	   sensor=(CbmMvdSensor*)fSensorArray->At(i);
	   sensor->AddPlugin(task);
	  }
      //data parallelizm requires that each sensor get its own task object
    
    }
    
    else if( plugin->GetPluginType() == buffer)
    {
     // cout << endl << "Type is Buffer" << endl;
     // cout << endl <<  plugin->ClassName() << endl;
      
     
      if ( plugin->ClassName() == framename)
	  {
	  CbmMvdSensorFrameBuffer* frameBuffer = new CbmMvdSensorFrameBuffer();
	  sensor=(CbmMvdSensor*)fSensorArray->At(i);
	  sensor->AddPlugin(frameBuffer);
         // cout << "Adding Buffer CbmMvdSensorFrameBuffer at Sensor " << sensor->GetName() << endl;
	  }
      else if ( plugin->ClassName() == trackingname)
	  {
	  CbmMvdSensorTrackingBuffer* trackingBuffer = new CbmMvdSensorTrackingBuffer();
	  sensor=(CbmMvdSensor*)fSensorArray->At(i);
	  sensor->AddPlugin(trackingBuffer);
         // cout << "Adding Buffer CbmMvdSensorTrackingBuffer at Sensor " << sensor->GetName() << endl;
	  }
	else  
	  {
	   cout << endl << "buffer not included yet, adding standart buffer." << endl;
	   CbmMvdSensorBuffer* buffer = new CbmMvdSensorBuffer();
	   sensor=(CbmMvdSensor*)fSensorArray->At(i);
	   sensor->AddPlugin(buffer);
	  }
      //data parallelizm requires that each sensor get its own buffer object
    
    }
    else {cout << "Invalide" << endl;}
  };
  
};

//----------------------------------------------------------------------


//-----------------------------------------------------------------------
void CbmMvdDetector::Init(){ 

  /**
   * 
   * Initialisation method
   * 
   * **/
  
  Int_t nSensors=fSensorArray->GetEntriesFast();
  CbmMvdSensor* sensor;
  
if(!initialized)
  {
  foutput = new TClonesArray("CbmMvdPoint",1000);
  fcurrentEvent = new TClonesArray("CbmMvdPoint",1000);
  foutputDigis = new TClonesArray("CbmMvdDigi",1000);
  foutputDigiMatchs = new TClonesArray("CbmMvdDigiMatch", 1000);
  foutputHits = new TClonesArray("CbmMvdHit",1000);
  foutputHitMatchs = new TClonesArray("CbmMvdHitMatch", 1000);
  }
  for(Int_t j = 0; j < nSensors; j++)
    {
    
    sensor=(CbmMvdSensor*)fSensorArray->At(j);
    //cout << "Init Sensor " << sensor->GetName() << endl;
    sensor->Init();
    }
  initialized = kTRUE;
 
}
//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
void CbmMvdDetector::SendInput(TClonesArray* input){

  /**
   * 
   * Sending event to sensors, 
   * each sensor gets only his own points
   * 
   * TODO: find faster way to re-sort points to sensors
   * 
   * **/
  

  CbmMvdPoint* point;
  Int_t nEntries = input->GetEntriesFast();
  Int_t nSensors=fSensorArray->GetEntriesFast();
  CbmMvdSensor* sensor;
  for(Int_t k = 0; k < nSensors; k++)
      {
      sensor=(CbmMvdSensor*)fSensorArray->At(k);
  	 for (Int_t i = 0; i < nEntries ; i++ )   
		{
		point= (CbmMvdPoint*) input->At(i); 
		point->SetPointId(i);
	        if (point->GetDetectorID() == sensor->GetDetectorID())
	           {
                   sensor->SendInput(point);
                   }
                }
        }
}
//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
void CbmMvdDetector::ExecChain(){
  
/**
   * 
   * method to execute plugin chain on sensors
   * 
   * **/
  
 foutput->Clear();
  fcurrentEvent->Clear();
  foutputDigis->Clear();
  foutputDigiMatchs->Clear();
  foutputHits->Clear();

  Int_t nSensors=fSensorArray->GetEntriesFast();
  CbmMvdSensor* sensor;
  for(Int_t i=0; i<nSensors; i++){
    sensor=(CbmMvdSensor*)fSensorArray->At(i);
    //cout << "I------ Send Chain to " << sensor->GetName() << endl;
    sensor->ExecChain();
    //cout << "I------ finished Chain at "<< sensor->GetName() <<endl<< endl;
  };
  
 
}

//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
void CbmMvdDetector::Exec(UInt_t nLevel){

  /**
   * 
   * execute spezific plugin on all sensors
   * 
   * **/
  
  Int_t nSensors=fSensorArray->GetEntriesFast();
  CbmMvdSensor* sensor;
  
  for(Int_t i=0; i<nSensors; i++){
    sensor=(CbmMvdSensor*)fSensorArray->At(i);
    sensor->Exec(nLevel);
  }
}

//-----------------------------------------------------------------------
 
//-----------------------------------------------------------------------

void CbmMvdDetector::ExecFrom(UInt_t nLevel) {


}
 //-----------------------------------------------------------------------

//-----------------------------------------------------------------------  
TClonesArray* CbmMvdDetector::GetCurrentEvent(){
  
  /**
   * Method used for debugging, Plugins have to hold there output until next call
   */
  
  Int_t nSensors = fSensorArray->GetEntriesFast();
  CbmMvdSensor* sensor;
  for(Int_t i=0; i<nSensors; i++){
    sensor=(CbmMvdSensor*)fSensorArray->At(i);
   // foutput = sensor->GetOutputArray(0);
    fcurrentEvent->AbsorbObjects(sensor->GetOutputArray(0));
       
  }

return(fcurrentEvent);
}   
//-----------------------------------------------------------------------    
 
//-----------------------------------------------------------------------
    
TClonesArray* CbmMvdDetector::GetOutputHits(){
  
   /**
   * method used to write hits to hd
   */
   
  Int_t nSensors = fSensorArray->GetEntriesFast();
  CbmMvdSensor* sensor;
  for(Int_t i=0; i<nSensors; i++){
    sensor=(CbmMvdSensor*)fSensorArray->At(i);
    foutputHits->AbsorbObjects(sensor->GetOutputBuffer());
  }

return(foutputHits);
} 
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
TClonesArray* CbmMvdDetector::GetOutputDigis(){
  
   /**
   * method used to write digis to hd
   */
   
  Int_t nSensors = fSensorArray->GetEntriesFast();
  CbmMvdSensor* sensor;
  
  for(Int_t i=0; i<nSensors; i++){
    sensor=(CbmMvdSensor*)fSensorArray->At(i);
    fDigiPlugin = sensor->GetDigiPlugin();
    foutputDigis->AbsorbObjects(sensor->GetOutputArray(fDigiPlugin));
  }

return(foutputDigis);
}
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
TClonesArray* CbmMvdDetector::GetOutputDigiMatchs(){
  
   /**
   * method used to write digiMatches to hd
   */
   
  Int_t nSensors = fSensorArray->GetEntriesFast();
  CbmMvdSensor* sensor;
  
  for(Int_t i=0; i<nSensors; i++){
    sensor=(CbmMvdSensor*)fSensorArray->At(i);
    fDigiPlugin = sensor->GetDigiPlugin();
    foutputDigiMatchs->AbsorbObjects(sensor->GetOutputMatch(fDigiPlugin));
  }

return(foutputDigiMatchs);
}
//-----------------------------------------------------------------------  

//-----------------------------------------------------------------------
TClonesArray* CbmMvdDetector::GetOutputHitMatchs(){
  
   /**
   * method used to write digiMatches to hd
   */
   
  Int_t nSensors = fSensorArray->GetEntriesFast();
  CbmMvdSensor* sensor;
  
  for(Int_t i=0; i<nSensors; i++){
    sensor=(CbmMvdSensor*)fSensorArray->At(i);
    fHitPlugin = sensor->GetHitPlugin();
    foutputHitMatchs->AbsorbObjects(sensor->GetOutputMatch(fHitPlugin));
  }

return(foutputHitMatchs);
}
//----------------------------------------------------------------------- 

//-----------------------------------------------------------------------

TClonesArray* CbmMvdDetector::GetOutputArray(Int_t nPlugin){
  
   /**
   * method used to write processed events to hd
   */
   
  Int_t nSensors = fSensorArray->GetEntriesFast();
  CbmMvdSensor* sensor;
  for(Int_t i=0; i<nSensors; i++){
    sensor=(CbmMvdSensor*)fSensorArray->At(i);
    foutputDigis->AbsorbObjects(sensor->GetOutputArray(nPlugin));
  }

return(foutputDigis);
} 
    
ClassImp(CbmMvdDetector)
