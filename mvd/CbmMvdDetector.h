// -------------------------------------------------------------------------
// -----                      CbmMvdDetector header file              -----
// -----                  Created 02/12/08  by M. Deveaux             -----
// -------------------------------------------------------------------------


/** CbmMvdDetector.h
 *@author M.Deveaux <deveaux@physik.uni-frankfurt.de>
 **
 ** Singleton holding information on all sensors of the MVD.
 ** User interface to the MVD-Software
 **
 **/


#ifndef CBMMVDDETECTOR_H
#define CBMMVDDETECTOR_H 1



#include <vector>
#include "TNamed.h"
#include "TString.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include "TGeoBBox.h"
#include "TClonesArray.h"

#include "CbmMvdSensor.h"


#include "plugins/CbmMvdSensorPlugin.h"
   #include "plugins/tasks/CbmMvdSensorTask.h"
      #include "plugins/tasks/CbmMvdSensorDigitizerTask.h"
      #include "plugins/tasks/CbmMvdSensorFindHitTask.h"
   #include "plugins/buffers/CbmMvdSensorBuffer.h"
      #include "plugins/buffers/CbmMvdSensorFrameBuffer.h"
      #include "plugins/buffers/CbmMvdSensorTrackingBuffer.h"

using std::vector;

// data classes
class CbmMvdPoint;
class CbmMvdSensorDataSheet;



class CbmMvdDetector : public TNamed
{

 public:

   CbmMvdDetector();  
   CbmMvdDetector(const char* name);
   
   /**
   * static instance
   */
    static CbmMvdDetector* Instance();


  /** Destructor **/
  virtual ~CbmMvdDetector();

  /** Data interface */
  void SendInput(TClonesArray* input);
  void SendClonesArray(TClonesArray* addedStructures, Int_t dataLevel=0){;}

  TClonesArray* GetOuput(){;}
  TClonesArray* GetOutputHits();
  TClonesArray* GetOutputHitMatchs();
  TClonesArray* GetOutputDigis();
  TClonesArray* GetOutputDigiMatchs();
  TClonesArray* GetOutputArray(Int_t nPlugin);
  TClonesArray* GetClonesArray(Int_t dataLevel){;}
  Int_t GetSensorArraySize(){return(fSensorArray->GetEntriesFast());} 
  UInt_t GetPluginArraySize(){return fPluginCount-1;} 
  
  
  TClonesArray* GetCurrentEvent	();
  /** Initialisation */
  
  void AddSensor(TString clearName, TString fullName, TString nodeName, 
		 CbmMvdSensorDataSheet* sensorData, Int_t stationNr, Int_t volumeId, Double_t sensorStartTime);
  void AddPlugin(CbmMvdSensorPlugin* plugin);
  void BuildDebugHistograms(){;};
  void Init();
  void SetMisalignment(Float_t misalignment[3]){for(Int_t i = 0; i<3; i++) fepsilon[i] = misalignment[i];}; 
  /** Data Processing */
  
  void ExecChain(); //Processes the full execution chain
  void Exec(UInt_t nLevel); //Processes Element nLevel of the chain
  void ExecTo(UInt_t nLevel){;}; // Processes Elements to a given Level of Plugins
  void ExecFrom(UInt_t nLevel); //Preocesses Elements from a given level till the end
  
  /** Finish */
  
  void Finish(){;}
  void StoreDebugHistograms(TString fileName){;}
  
  /** Accessors */
  
  CbmMvdSensor* GetSensor(UInt_t nSensor){return (CbmMvdSensor*)fSensorArray->At(nSensor);};
  
private:
  TClonesArray* fSensorArray;
  TClonesArray* fSensorIDArray; //Array of Sensor ID
  UInt_t fPluginCount;
  TClonesArray* foutput;
  TClonesArray* foutputHits;
  TClonesArray* foutputDigis;
  TClonesArray* foutputDigiMatchs;
  TClonesArray* foutputHitMatchs;
  TClonesArray* fcurrentEvent;
  Float_t fepsilon[3];
 /** Data members */
  
  Int_t fDigiPlugin; 
  Int_t fHitPlugin;
  
  
  static CbmMvdDetector* fInstance;
  Bool_t fSensorArrayFilled;
  Bool_t initialized;

  CbmMvdDetector(const CbmMvdDetector&);
  CbmMvdDetector operator=(const CbmMvdDetector&);
  
 ClassDef(CbmMvdDetector,1);
  

};



#endif
