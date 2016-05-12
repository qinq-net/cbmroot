#ifndef TTRBHEADER_H_
#define TTRBHEADER_H_

#include "TNamed.h"

class TTrbHeader : public TNamed
{
 public:

  TTrbHeader();

  virtual ~TTrbHeader();

  inline void SetTriggerPattern(UInt_t iVal) { fiTriggerPattern = iVal;}
  inline void SetTriggerType(UInt_t iVal)    { fiTriggerType = iVal;}
  inline void SetTimeInSpill(Double_t dVal)    { fdTimeInSpill = dVal;}
  void SetTimeInRun(Double_t dVal) {fdTimeInRun = dVal;}
  void SetSpillIndex(Int_t iVal) {fiSpillIndex = iVal;}

  inline UInt_t GetTriggerPattern() { return fiTriggerPattern;}
  inline UInt_t GetTriggerType()    { return fiTriggerType;}
  inline Double_t GetTimeInSpill()    { return fdTimeInSpill;}
  Double_t GetTimeInRun() {return fdTimeInRun;}
  Int_t GetSpillIndex() {return fiSpillIndex;}

  Bool_t TriggerFired(Int_t iTrg);

 private:
  UInt_t fiTriggerPattern; 
  UInt_t fiTriggerType;
  Double_t fdTimeInSpill; // in [s]
  Double_t fdTimeInRun; // in [s]
  Int_t fiSpillIndex;

  ClassDef(TTrbHeader, 2)
};
#endif
