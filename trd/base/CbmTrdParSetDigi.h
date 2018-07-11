#ifndef CBMTRDPARSETDIGI_H
#define CBMTRDPARSETDIGI_H

#include "CbmTrdParSet.h"
#include <map>

/**
  * \brief Describe TRD module working settings (HV, etc)
  **/
class FairParamList;
class CbmTrdParSetDigi : public CbmTrdParSet 
{
public:

  /** Standard constructor **/
  CbmTrdParSetDigi(const char* name    = "CbmTrdParSetDigi",
    const char* title   = "TRD chamber parameters",
    const char* context = "Default");

  /** \brief Destructor **/
  virtual ~CbmTrdParSetDigi() {;}
  
  /** \brief Reset all parameters **/
  virtual void  clear() {;}

  void          putParams(FairParamList*);
  Bool_t        getParams(FairParamList*);
private:

  ClassDef(CbmTrdParSetDigi,1) // Container of the chamber parameters for the TRD detector
};
#endif
