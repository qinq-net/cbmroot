/** CbmMCStreamerQa.h
 *@author E.Kryshen <e.kryshen@gsi.de>
 *@since 2010-10-06
 **
 **/

#ifndef CbmMCStreamerQa_H_
#define CbmMCStreamerQa_H_ 1

#include "FairTask.h"
#include "TString.h"
#include "TClonesArray.h"
#include "CbmDefs.h"
#include <map>
class CbmMCEvent;
class CbmMCEpoch;
class TChain;
class TH3D;
class TH1D;

class CbmMCStreamerQa : public FairTask{
public:
  /** Default constructor **/
  CbmMCStreamerQa();

  /** Standard constructor
  *@param name   Name of class
  *@param title  Task title
  **/
  CbmMCStreamerQa(const char* name, TChain* mcChain);

  /** Destructor **/
  virtual ~CbmMCStreamerQa();

  /** Initialisation **/
  virtual InitStatus Init();

  /** Task execution **/
  virtual void Exec(Option_t* opt);

  /** Finish at the end **/
  virtual void Finish();

  /** SetParContainers **/
  virtual void SetParContainers();

private:
  Int_t         fNepoch;  //! MC epoch counter
  CbmMCEpoch*   fMcEpoch; //! Pointer to the current MC epoch
  TChain*       fMcChain; //! tree of MC events
  TClonesArray* fPointArrays[kNofSystems];       //! Array of MC points per detector
  std::map<int,int>  fMapPointsInEvents[kNofSystems]; //! map of point counters for each event per detector

  CbmMCStreamerQa(const CbmMCStreamerQa&);
  CbmMCStreamerQa& operator=(const CbmMCStreamerQa&);

  ClassDef(CbmMCStreamerQa,1);
};

#endif
