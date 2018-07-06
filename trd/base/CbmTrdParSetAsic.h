#ifndef CBMTRDPARSETASIC_H
#define CBMTRDPARSETASIC_H

#include "CbmTrdParSet.h"
#include <vector>
#include <map>

class FairParamList;
class CbmTrdParAsic;
/**
  * \brief Describe TRD module ASIC settings (electronic gain, delays, etc)
  **/
class CbmTrdParSetAsic : public CbmTrdParSet 
{
public:

  /** Standard constructor **/
  CbmTrdParSetAsic(const char* name    = "CbmTrdParSetAsic",
    const char* title   = "TRD ASIC parameters",
    const char* context = "Default");

  /** \brief Destructor **/
  virtual ~CbmTrdParSetAsic();
  
  /** \brief Reset all parameters **/
  virtual void  clear() {;}
  virtual void                  GetAsicAddresses(std::vector<Int_t> *a) const;
  Bool_t        getParams(FairParamList*);
  /** \brief Look for the ASIC which operates on a specific channel
   *\param chAddress Address of the channel
   *\return id of the ASIC operating on the channel. -1 in case of failure
   */ 
  virtual Int_t                 GetAsicAddress(Int_t chAddress) const;
  virtual const CbmTrdParAsic*  GetAsicPar(Int_t address) const { return (CbmTrdParAsic*)GetModulePar(address);}
  virtual CbmTrdParAsic*        GetAsicPar(Int_t address)       { return (CbmTrdParAsic*)GetModulePar(address);}
  virtual const CbmTrdParSet*   GetModuleSet(Int_t detId) const;
  virtual Int_t                 GetNofAsics() const             { return GetNrOfModules();}
  virtual void  Print(Option_t *opt="") const;
  void          putParams(FairParamList*) {;}
  virtual void  SetAsicPar(Int_t address, CbmTrdParAsic *p);

private:  
  CbmTrdParSetAsic(const CbmTrdParSetAsic &ref);
  const CbmTrdParSetAsic& operator=(const CbmTrdParSetAsic &ref);

  std::map<Int_t, CbmTrdParSetAsic*> fModPar;     ///< module wise list of ASIC sets
  ClassDef(CbmTrdParSetAsic,1);   // The set of ASIC settings for all TRD modules
};
#endif
