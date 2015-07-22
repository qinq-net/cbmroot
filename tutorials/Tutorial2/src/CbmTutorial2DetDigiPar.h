// -------------------------------------------------------------------------
// -----                    CbmTrdDigiPar header file                  -----
// -----                 Created 15/05/08  by F. Uhlig                 -----
// -------------------------------------------------------------------------

#ifndef CBMTUTORIAL2DETDIGIPAR_H
#define CBMTUTORIAL2DETDIGIPAR_H


#include "FairParGenericSet.h"

#include "TObject.h"
#include "TObjArray.h"
#include "TArrayF.h"
#include "TArrayI.h"

class FairParIo;
class FairParamList;


class CbmTutorial2DetDigiPar : public FairParGenericSet 
{

 public:

  /** Standard constructor **/
  CbmTutorial2DetDigiPar(const char* name    = "CbmTutorial2DetDigiPar",
			const char* title   = "Tutorial2 Det digi parameters",
			const char* context = "Default");


  /** Destructor **/
  virtual ~CbmTutorial2DetDigiPar();


  /** Initialisation from input device**/
  //virtual Bool_t init(FairParIo* input);


  /** Output to file **/
  //  virtual Int_t write(FairParIo* output);

  //  virtual void print(); 
  virtual void printparams();

  
  /** Reset all parameters **/
  virtual void clear();

  void putParams(FairParamList*);
  Bool_t getParams(FairParamList*);

 private:

  TArrayF* ftutdetdigipar;  //
  TArrayI ftutdetdigiparsector; //
  Int_t ftutdetdigiparstation; //

  CbmTutorial2DetDigiPar (const CbmTutorial2DetDigiPar&);
  CbmTutorial2DetDigiPar& operator=(const CbmTutorial2DetDigiPar&);

  ClassDef(CbmTutorial2DetDigiPar,1);


};


#endif





