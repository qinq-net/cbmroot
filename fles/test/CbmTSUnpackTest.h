// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSUnpackTest                            -----
// -----                 Created 20.06.2016 by F. Uhlig                    -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMTSUNPACKTEST_H
#define CBMTSUNPACKTEST_H

#ifndef __CINT__
  #include "Timeslice.hpp"
#endif

#include "CbmTSUnpack.h"

#include "TClonesArray.h"

class CbmTSUnpackTest : public CbmTSUnpack
{
public:
  
  CbmTSUnpackTest();
  virtual ~CbmTSUnpackTest();
    
  virtual Bool_t Init();
#ifndef __CINT__
  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
#endif
  virtual void Reset();

  virtual void Finish() {;}

private:

  CbmTSUnpackTest(const CbmTSUnpackTest&);
  CbmTSUnpackTest operator=(const CbmTSUnpackTest&);

  ClassDef(CbmTSUnpackTest, 1)
};

#endif
