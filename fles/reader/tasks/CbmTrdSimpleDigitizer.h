#ifndef CBMTRDSIMPLEDIGITIZER_H
#define CBMTRDSIMPLEDIGITIZER_H


#include "CbmTrdQABase.h"
class CbmTrdSimpleDigitizer : public CbmTrdQABase
{
  void StatelessDigitizer();
 protected:
  TClonesArray* fDigis;
 public:
  CbmTrdSimpleDigitizer();
  //  ~CbmTrdSimpleDigitizer();
  InitStatus Init();
  void Exec(Option_t*);
  void CreateHistograms();
    
ClassDef(CbmTrdSimpleDigitizer,1);
};

#endif

