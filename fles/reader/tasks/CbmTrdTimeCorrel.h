#ifndef CBMTRDTIMECORREL_H
#define CBMTRDTIMECORREL_H

#include "FairTask.h"

#include "CbmHistManager.h"

#include "TClonesArray.h"
#include "CbmSpadicRawMessage.h"
typedef std::map<Int_t, std::map<ULong_t, CbmSpadicRawMessage* > > EpochMap;
typedef std::map<Int_t, std::map<Int_t,std::map<ULong_t, Long_t> > > OffsetMap;

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
  std::map<TString, std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> > > fMessageBuffer;
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

  ULong_t fLastMessageTime[3][3][32];
/* Store the last message time.
Intended usage is for the first (outer) argument to be the SysCoreID 
and the second (inner) to be the SPADICID of a full SPADIC.
Third argument is the ChannelID.
Timestamps are stored for a full spadic.
*/

  void CreateHistograms();

  void ReLabelAxis(TAxis* axis, TString type, Bool_t underflow, Bool_t overflow);

  Int_t GetMessageType(CbmSpadicRawMessage* raw);

  Int_t GetChannelOnPadPlane(Int_t SpadicChannel);

  CbmTrdTimeCorrel(const CbmTrdTimeCorrel&);
  CbmTrdTimeCorrel operator=(const CbmTrdTimeCorrel&);
  std::map<Int_t, std::map<Int_t,std::map<ULong_t, Long_t> > > CalcutlateTimestampOffsets(const EpochMap &epochBuffer);

  ClassDef(CbmTrdTimeCorrel,1);
};

#endif


