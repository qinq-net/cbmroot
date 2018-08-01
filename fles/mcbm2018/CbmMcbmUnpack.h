// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                            CbmMcbmUnpack                          -----
// -----               Created 09.07.2018 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMMCBMUNPACK_H
#define CBMMCBMUNPACK_H

#include "CbmTSUnpack.h"

class CbmDigi;

class CbmMcbmUnpack : public CbmTSUnpack
{
 public:

  CbmMcbmUnpack();
  virtual ~CbmMcbmUnpack();

  virtual Bool_t Init() = 0;
#ifndef __CINT__
  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component) = 0;
#endif
  virtual void Reset() = 0;

  virtual void Finish() = 0;
  // protected:
  //  virtual void Register() = 0;

  virtual void SetParContainers() = 0;

  virtual Bool_t InitContainers() {return kTRUE;}

  virtual Bool_t ReInitContainers() {return kTRUE;}

  virtual void FillOutput(CbmDigi*) = 0;

  virtual void AddMsComponentToList( size_t component, UShort_t usDetectorId ) = 0;
  virtual void SetNbMsInTs( size_t uCoreMsNb, size_t uOverlapMsNb ) = 0;

  ClassDef(CbmMcbmUnpack, 0)
};

#endif
