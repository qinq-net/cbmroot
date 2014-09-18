// ------------------------------------------------------------------------
// -----                  CbmMvdSensorTask header file              -----
// -----                   Created 02/02/12  by M. Deveaux            -----
// ------------------------------------------------------------------------

/**  CbmMvdSensorTask.h
 *@author M.Deveaux <deveaux@physik.uni-frankfurt.de>
 **
 ** Base class for the plugins of the MVD sensor 
 **
 **/

#ifndef CBMMVDSENSORTASK_H
#define CBMMVDSENSORTASK_H 1

#include "TObject.h"
#include "plugins/CbmMvdSensorPlugin.h"
#include <iostream>


class TClonesArray;
class CbmMvdSensor;

class CbmMvdSensorTask : public CbmMvdSensorPlugin 
{

 public:

  /** Default constructor **/
  CbmMvdSensorTask();

  /** Destructor **/
  virtual ~CbmMvdSensorTask();
  
  virtual void Init();
  virtual TClonesArray* GetInputArray();
  virtual TClonesArray* GetMatchArray();
  virtual TClonesArray* GetWriteArray();
  virtual void SetInputArray(TClonesArray* inputArray);
  virtual void SetInput(CbmMvdPoint* point);
  virtual void SetOutputArray(TClonesArray* outputArray);

  virtual void 		SendInputBuffer(TClonesArray* inputBuffer){fInputBuffer=inputBuffer;};
  virtual void          CallBufferForInputData(){;}; //See comment in ExecChain() in .cxx
  virtual TClonesArray* GetOutputArray(){return fOutputBuffer;};
  
  virtual void		Init(CbmMvdSensor* mySensor){fSensor=mySensor;};
  virtual void 		Exec(){;};
  virtual void 		Finish(){;};
  virtual void          ExecChain(){;};
  virtual bool	  PluginReady(){return (true);};
  /** Returns task type to a upper control unit **/
  const MvdSensorPluginType GetPluginType(){return task;};
  

  protected:

  TClonesArray* fInputBuffer;     // Buffer of background events
  TClonesArray* fOutputBuffer;
  CbmMvdSensor* fSensor;
  
  private:
    
    
  ClassDef(CbmMvdSensorTask,1);

};


#endif
