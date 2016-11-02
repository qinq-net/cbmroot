// -------------------------------------------------------------------------
// -----                 CbmFHodoUnpackPar header file                 -----
// -----                 Created 02/11/16  by F. Uhlig                 -----
// -------------------------------------------------------------------------

#ifndef CBMFHODOUNPACKPAR_H
#define CBMFHODOUNPACKPAR_H

#include "FairParGenericSet.h"

#include "TArrayI.h"

class FairParIo;
class FairParamList;


class CbmFHodoUnpackPar : public FairParGenericSet 
{

 public:

  /** Standard constructor **/
  CbmFHodoUnpackPar(const char* name    = "CbmFHodoUnpackPar",
		    const char* title   = "FHodo unpacker parameters",
		    const char* context = "Default");


  /** Destructor **/
  virtual ~CbmFHodoUnpackPar();
  
  /** Reset all parameters **/
  virtual void clear();

  void putParams(FairParamList*);
  Bool_t getParams(FairParamList*);

  void SetNrOfModules(Int_t i) { fNrOfModules = i; }
  void SetModuleIdArray(TArrayI array) { fModuleIdArray = array; }

  Int_t GetNrOfModules() { return fNrOfModules; }
  Int_t GetModuleId(Int_t i) { return fModuleIdArray[i]; }
 private:

  TArrayI fModuleIdArray; // Array to hold the unique IDs for all FHodo modules
  Int_t fNrOfModules; // Total number of modules

  ClassDef(CbmFHodoUnpackPar,1);
};
#endif
