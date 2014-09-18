// ------------------------------------------------------------------------
// -----                  CbmMvdSensorPlugin header file              -----
// -----                   Created 02/02/12  by M. Deveaux            -----
// ------------------------------------------------------------------------

/**  CbmMvdSensorPlugin.h
 *@author M.Deveaux <deveaux@physik.uni-frankfurt.de>
 **
 ** Base class for the plugins of the MVD sensor 
 **
 **/

#ifndef CBMMVDSENSORPLUGIN_H
#define CBMMVDSENSORPLUGIN_H 1

#include <iostream>
#include "FairTask.h" //needed to get data type InitStatus
#include "CbmMvdPoint.h"

class TClonesArray;
class CbmMvdSensor;

using std::cout;
using std::endl;

enum MvdSensorPluginType {task, buffer};

class CbmMvdSensorPlugin : public TObject
{

 public:

  /** Default constructor **/
  CbmMvdSensorPlugin();

  /** Destructor **/
  virtual ~CbmMvdSensorPlugin();
  
  virtual const MvdSensorPluginType GetPluginType();
  virtual void Init();
  virtual void Exec();
  virtual void ExecChain();
    
  virtual TClonesArray* GetInputArray();
  virtual TClonesArray* GetOutputArray();
  virtual TClonesArray* GetMatchArray();
  virtual TClonesArray* GetWriteArray();
  
  virtual void SetInputArray(TClonesArray* inputArray);
  virtual void SetInput(CbmMvdPoint* point);
  virtual void SetOutputArray(TClonesArray* outputArray);
  
  virtual void SetNextPlugin(CbmMvdSensorPlugin* plugin) {fNextPlugin=plugin;}
  virtual void SetPreviousPlugin(CbmMvdSensorPlugin* plugin) {fPreviousPlugin=plugin;}
  
  virtual CbmMvdSensorPlugin* GetNextPlugin()   {return fNextPlugin;}
  virtual CbmMvdSensorPlugin* GetPrevousPlugin(){return fPreviousPlugin;}
  
  bool	  PluginReady(){return (bFlag);};
  void  SetPluginReady(bool flag) {bFlag = flag;}

  Bool_t IsInit(){return(initialized);}
  /** data members **/
  
  CbmMvdSensorPlugin* fNextPlugin; 
  CbmMvdSensorPlugin* fPreviousPlugin;

protected:
    bool 	bFlag;
    Bool_t initialized;
  
  ClassDef(CbmMvdSensorPlugin,1);

};




#endif
