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
  #include "Message.hpp"
#endif


#include "CbmTSUnpack.h"
#include "CbmBeamDefaults.h"

#include "TClonesArray.h"

class CbmTSUnpackSpadic : public CbmTSUnpack
{
 public:
  
  CbmTSUnpackSpadic();
  virtual ~CbmTSUnpackSpadic();
    
  virtual Bool_t Init();
#ifndef __CINT__
  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
  void print_message(const spadic::Message& m);
#endif
  virtual void Reset();

  virtual void Finish();
  // protected:
  //  virtual void Register();

 private:

  TClonesArray* fSpadicRaw;

  Int_t fEpochMarkerArray[NrOfSyscores][NrOfHalfSpadics];
  Int_t fSuperEpochCounter[NrOfSyscores][NrOfHalfSpadics];

  ClassDef(CbmTSUnpackSpadic, 1)
};

#endif
