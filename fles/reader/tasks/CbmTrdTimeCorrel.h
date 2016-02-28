#ifndef CBMTRDTIMECORREL_H
#define CBMTRDTIMECORREL_H

#include "FairTask.h"

#include "CbmHistManager.h"

#include "TClonesArray.h"
#include "CbmSpadicRawMessage.h"

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

  void SetRun(Int_t run)               { fRun = run;}
  void SetRewriteSpadicName(Bool_t rewrite)  { fRewriteSpadicName = rewrite; }

 private:

  Int_t   fSpadics;
  Bool_t  fRewriteSpadicName;

  Int_t   GetSpadicID(Int_t sourceA);

  TString GetSpadicName(Int_t eqID,Int_t sourceA);
  TString RewriteSpadicName(TString spadicName);

  TString GetStopName(Int_t stopType);

  TClonesArray* fRawSpadic;

  CbmHistManager* fHM;

  Int_t fNrTimeSlices;
  
  Int_t fRun;

  Int_t fLastMessageTime[3][3];
/* Store the last message time.
Intended usage is for the first (outer) argument to be the SysCoreID 
and the second (inner) to be the SPADICID. 
Timestamps are only stored for a full spadic, not a half spadic.
*/

  void CreateHistograms();

  void ReLabelAxis(TAxis* axis, TString type, Bool_t underflow, Bool_t overflow);

  Int_t GetMessageType(CbmSpadicRawMessage* raw);

  Int_t GetChannelOnPadPlane(Int_t SpadicChannel);

  CbmTrdTimeCorrel(const CbmTrdTimeCorrel&);
  CbmTrdTimeCorrel operator=(const CbmTrdTimeCorrel&);

  ClassDef(CbmTrdTimeCorrel,1);
};

#endif


