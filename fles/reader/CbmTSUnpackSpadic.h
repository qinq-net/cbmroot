// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmTSUnpackSpadic                       -----
// -----                    Created 07.11.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMTSUNPACKSPADIC_H
#define CBMTSUNPACKSPADIC_H

#ifndef __CINT__
  #include "Timeslice.hpp"
#endif


#include "CbmTSUnpack.h"

class CbmTSUnpackSpadic : public CbmTSUnpack
{
 public:
  
  CbmTSUnpackSpadic();
  virtual ~CbmTSUnpackSpadic();
    
  virtual Bool_t Init();
#ifndef __CINT__
  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
#endif
  virtual void Reset();

  // protected:
  //  virtual void Register();

  ClassDef(CbmTSUnpackSpadic, 1)
};

#endif
