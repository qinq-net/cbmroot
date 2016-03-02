// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmTSUnpackSpadic                       -----
// -----                    Created 07.11.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMTSUNPACKSPADICLEGACY_H
#define CBMTSUNPACKSPADICLEGACY_H

#include "CbmTSUnpack.h"
#include "CbmBeamDefaults.h"

#include "TClonesArray.h"

#ifndef __CINT__
  #include "Timeslice.hpp"
  #include "Message.hpp"
#endif

class CbmTSUnpackSpadicLegacy : public CbmTSUnpack
{
 public:
  
  CbmTSUnpackSpadicLegacy();
  virtual ~CbmTSUnpackSpadicLegacy();
    
  virtual Bool_t Init();
#ifndef __CINT__
  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
  void print_message(const spadic::Message& m);
#endif
  virtual void Reset();

  virtual void Finish();
  // protected:
  //  virtual void Register();
  static const Int_t NrOfSyscores2 = 3;
  static const Int_t NrOfSpadics2 = 3;
  static const Int_t NrOfHalfSpadics2 = NrOfSpadics2*2;


 private:
 
  TClonesArray* fSpadicRaw;

  Int_t fEpochMarkerArray[NrOfSyscores2][NrOfHalfSpadics2];
//Array to store the previous Epoch counter
  Int_t fPreviousEpochMarkerArray[NrOfSyscores2][NrOfHalfSpadics2];
  Int_t fSuperEpochArray[NrOfSyscores2][NrOfHalfSpadics2];

  Int_t fEpochMarker;
  Int_t fSuperEpoch;

  void GetEpochInfo(Int_t link, Int_t addr);
  void FillEpochInfo(Int_t link, Int_t addr, Int_t epoch_count);

  CbmTSUnpackSpadicLegacy(const CbmTSUnpackSpadicLegacy&);
  CbmTSUnpackSpadicLegacy operator=(const CbmTSUnpackSpadicLegacy&);

  ClassDef(CbmTSUnpackSpadicLegacy, 2)
};

#endif

