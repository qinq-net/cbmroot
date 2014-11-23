// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                      CbmTSUnpackNxyter                            -----
// -----                    Created 07.11.2014 by                          -----
// -----                   E. Ovcharenko, F. Uhlig                         -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMTSUNPACKNXYTER_H
#define CBMTSUNPACKNXYTER_H

#ifndef __CINT__
  #include "Timeslice.hpp"
#endif


#include "CbmTSUnpack.h"

#include "TClonesArray.h"

class CbmTSUnpackNxyter : public CbmTSUnpack
{
 public:
  
  CbmTSUnpackNxyter();
  virtual ~CbmTSUnpackNxyter();
    
  virtual Bool_t Init();
#ifndef __CINT__
  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
#endif
  virtual void Reset();

  virtual void Finish() {;}

  // protected:
  //  virtual void Register();

 private:
  Int_t fCurrEpoch; // Current epoch (first epoch in the stream initialises the 

  TClonesArray* fNxyterRaw;
  
  ClassDef(CbmTSUnpackNxyter, 1)
};

#endif
