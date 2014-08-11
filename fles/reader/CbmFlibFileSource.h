// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmFlibFileSource                       -----
// -----                    Created 08.08.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMFLIBFILESOURCE_H
#define CBMFLIBFILESOURCE_H

#include "TimesliceSource.hpp"

#include "TObject.h"
#include "TString.h"

#include <memory>

class CbmFlibFileSource : public TObject
{
  public:
    CbmFlibFileSource();
    CbmFlibFileSource(const CbmFlibFileSource& source);
    virtual ~CbmFlibFileSource();

    Bool_t Init();
    Int_t ReadEvent();
    void Close();
    void Reset();

    void SetFileName(TString name) { fFileName = name; }

  public:
  
    TString fFileName;

    //std::unique_ptr<fles::TimesliceSource> fSource;
    fles::TimesliceSource* fSource;

    ClassDef(CbmFlibFileSource, 1)
};


#endif
