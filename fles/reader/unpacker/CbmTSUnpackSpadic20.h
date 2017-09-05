// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmTSUnpackSpadic                       -----
// -----                    Created 07.11.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMTSUNPACKSPADIC20_H
#define CBMTSUNPACKSPADIC20_H

#include "Timeslice.hpp"
#include "Message.hpp"


#include "CbmTSUnpack.h"
#include "CbmBeamDefaults.h"
#include <map>

#include "TClonesArray.h"

class CbmTSUnpackSpadic20 : public CbmTSUnpack
{
 public:
  
  CbmTSUnpackSpadic20();
  virtual ~CbmTSUnpackSpadic20();
    
  virtual Bool_t Init();

  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
  void print_message(const spadic::Message& m);

  virtual void Reset();

  virtual void Finish();

  void SetParContainers() {;}

  // protected:
  //  virtual void Register();

  virtual void FillOutput(CbmDigi*){;}
 private:

  TClonesArray* fSpadicRaw;

  std::map<std::pair<int,int>,int> fEpochMarkerArray;
//Array to store the previous Epoch counter
  std::map<std::pair<int,int>,int>  fPreviousEpochMarkerArray;
//Suppress multiple Epoch Messages in duplicated Microslices. NOTE:Currently Buggy
  const Bool_t SuppressMultipliedEpochMessages = false;
  std::map<std::pair<int,int>,int>  fSuperEpochArray;

  Int_t fEpochMarker;
  Int_t fSuperEpoch;
  Int_t fNrExtraneousSamples;

  void GetEpochInfo(Int_t link, Int_t addr);
  void FillEpochInfo(Int_t link, Int_t addr, Int_t epoch_count);

  CbmTSUnpackSpadic20(const CbmTSUnpackSpadic20&);
  CbmTSUnpackSpadic20 operator=(const CbmTSUnpackSpadic20&);

  ClassDef(CbmTSUnpackSpadic20, 2)
};

#endif

