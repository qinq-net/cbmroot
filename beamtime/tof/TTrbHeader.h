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
  inline void SetTimeInSpill(UInt_t iVal)    { fiTimeInSpill = iVal;}

  inline UInt_t GetTriggerPattern() { return fiTriggerPattern;}
  inline UInt_t GetTriggerType()    { return fiTriggerType;}
  inline UInt_t GetTimeInSpill()    { return fiTimeInSpill;}

  Bool_t TriggerFired(Int_t iTrg);

 private:
  UInt_t fiTriggerPattern; 
  UInt_t fiTriggerType;
  UInt_t fiTimeInSpill;

  ClassDef(TTrbHeader, 1)
};
#endif
