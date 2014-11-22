// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                              CbmTSUnpackTrb                       -----
// -----                    Created 22.11.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMTSUNPACKTRB_H
#define CBMTSUNPACKTRB_H

#ifndef __CINT__
  #include "Timeslice.hpp"
  #include "Message.hpp"
  #include "TrbBridge.hpp"
#endif


#include "CbmTSUnpack.h"
#include "CbmBeamDefaults.h"

#include "TClonesArray.h"

class CbmTSUnpackTrb : public CbmTSUnpack
{
 public:
  
  CbmTSUnpackTrb();
  virtual ~CbmTSUnpackTrb();
    
  virtual Bool_t Init();
#ifndef __CINT__
  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
#endif
  virtual void Reset();

  virtual void Finish();

 private:

  //  TClonesArray* fTrbRaw;
#ifndef __CINT__
  TrbBridge* fTrbBridge;
  std::list<std::vector<uint32_t>> fTrbEventList;
#endif

  ClassDef(CbmTSUnpackTrb, 1)
};

#endif
