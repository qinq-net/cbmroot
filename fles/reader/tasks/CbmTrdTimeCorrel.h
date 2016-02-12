#ifndef CBMTRDTIMECORREL_H
#define CBMTRDTIMECORREL_H

#include "FairTask.h"

#include "CbmHistManager.h"

#include "TClonesArray.h"

class CbmTrdTimeCorrel : public FairTask
{
 public:

  CbmTrdTimeCorrel();
  ~CbmTrdTimeCorrel();

  virtual InitStatus Init();

  virtual InitStatus ReInit();

  virtual void Exec(Option_t* opt);

  virtual void SetParContainers();

  virtual void Finish();

  virtual void FinishEvent();

 private:

  TString GetSysCore(Int_t eqID);
  Int_t   GetSysCoreID(Int_t eqID);

  TString GetSpadic(Int_t sourceA);
  Int_t   GetSpadicID(Int_t sourceA);

  TString GetStopName(Int_t stopType);

  TClonesArray* fRawSpadic;

  CbmHistManager* fHM;

  Int_t fNrTimeSlices;

  void CreateHistograms();

  Int_t GetChannelOnPadPlane(Int_t SpadicChannel);

  CbmTrdTimeCorrel(const CbmTrdTimeCorrel&);
  CbmTrdTimeCorrel operator=(const CbmTrdTimeCorrel&);

  ClassDef(CbmTrdTimeCorrel,1);
};

#endif


