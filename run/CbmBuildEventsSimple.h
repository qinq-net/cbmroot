/** @file CbmBuildEventsSimple.h
 ** @ athor //Dr.Sys (Mikhail Prokudin) prokudin@cern.ch
 ** @ date 14.06.06
 **/

#ifndef CBMBUILDEVENTSSIMPLE_H
#define CBMBUILDEVENTSSIMPLE_H

#include "FairTask.h"

class CbmTimeSlice;
class TClonesArray;

class CbmBuildEventsSimple : public FairTask
{
public:
  CbmBuildEventsSimple();
  CbmBuildEventsSimple(const char* name, Int_t iVerbose=1);

  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  virtual void Finish();

  // Setters. Time in [ns], events in counts :) Only STS information is used.
  void SetWindDur         (Double_t winddur=2)           {fWindDur=winddur;}
  void SetMinusDeltaT     (Double_t minusdeltat=2)       {fMinusDeltaT=minusdeltat;}
  void SetPlusDeltaT      (Double_t plusdeltat=5)        {fPlusDeltaT=plusdeltat;}
  void SetMinHitStations  (Int_t minhitstations=8)       {fMinHitStations=minhitstations;}
  void SetMinDigis        (Int_t mindigis=5000)            {fMinDigis=mindigis;}
  void SetDeadT           (Double_t deadt=10)            {fDeadT=deadt;}

  ~CbmBuildEventsSimple();

private:
  /** Fills Stsdigis array. STS separate, because start and end end digi is known. **/
  void FillEvent(Int_t st, Int_t end);

  CbmTimeSlice* fSlice;	// Pointer to current timeslce
  TClonesArray* fSTSDigi;	// Array of STS digis
  Int_t fNSTSDigis;

  Int_t fEv;		// Number of current event **/
  Int_t fSliceN;	// Number of current time slice **/
  Int_t fNDigis;	// Number of digis in current time window
  Int_t fSN;		// N of start of time window
  Double_t fST;		// Time of start of time window
  Int_t fNStsDigis[16];	// Number of STS digis per station
  Double_t fWindDur;	// Duration of time window [ns]
  Double_t fMinusDeltaT;// [ns] before start of time window for event construction
  Double_t fPlusDeltaT;	// [ns] after end of time window for event construction
  Double_t fDeadT;	// [ns] after last accepted digi of blindless
  Int_t fMinHitStations;// Minimum number of hit STS stations for event formation
  Int_t fMinDigis;	// Minimum number of digis in time window for event formation

  /** Prevent usage of copy constructor and assignment operator **/
  CbmBuildEventsSimple(const CbmBuildEventsSimple&);
  CbmBuildEventsSimple& operator =(const CbmBuildEventsSimple&);


  ClassDef(CbmBuildEventsSimple, 0)
};

#endif

