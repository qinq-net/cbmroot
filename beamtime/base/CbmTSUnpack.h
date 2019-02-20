// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                              CbmTSUnpack                          -----
// -----                    Created 07.11.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMTSUNPACK_H
#define CBMTSUNPACK_H

#include "Timeslice.hpp"

#include "TObject.h"

class CbmDigi;

class CbmTSUnpack : public TObject
{
 public:
  
  CbmTSUnpack();
  virtual ~CbmTSUnpack();
    
  virtual Bool_t Init() = 0;

  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component) = 0;

  virtual void Reset() = 0;

  virtual void Finish() = 0;
  // protected:
  //  virtual void Register() = 0;

  virtual void SetParContainers() = 0;

  virtual Bool_t InitContainers() {return kTRUE;}

  virtual Bool_t ReInitContainers() {return kTRUE;}

  virtual void FillOutput(CbmDigi*) = 0;
  
  ClassDef(CbmTSUnpack, 0)
};

#endif
