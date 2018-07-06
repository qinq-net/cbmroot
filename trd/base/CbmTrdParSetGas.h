#ifndef CBMTRDPARSETGAS_H
#define CBMTRDPARSETGAS_H

#include "CbmTrdParSet.h"
#include <map>

class FairParamList;
class TH2F;
/**
  * \brief Describe TRD module working settings (HV, etc)
  **/
class CbmTrdParSetGas : public CbmTrdParSet 
{
public:

  /** Standard constructor **/
  CbmTrdParSetGas(const char* name    = "CbmTrdParSetGas",
    const char* title   = "TRD chamber parameters",
    const char* context = "Default");

  /** \brief Destructor **/
  virtual ~CbmTrdParSetGas() {;}
  
  /** \brief Reset all parameters **/
  virtual void  clear() {;}

  void          putParams(FairParamList*) {;}
  Bool_t        getParams(FairParamList*);
private:
  TH2F*   ScanDriftMap();
  
  ClassDef(CbmTrdParSetGas,1) // Container of the chamber parameters for the TRD detector
};
#endif
