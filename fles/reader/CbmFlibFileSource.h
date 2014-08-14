// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmFlibFileSource                       -----
// -----                    Created 08.08.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMFLIBFILESOURCE_H
#define CBMFLIBFILESOURCE_H

#include "TimesliceSource.hpp"
#include "Timeslice.hpp"

#ifndef __CINT__
#include "Message.hpp"
#endif

#include "FairSource.h"

#include "TString.h"

#include <memory>


class CbmFlibFileSource : public FairSource
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

  private:
  
    TString fFileName;

    //#ifndef __CINT__
    //    std::unique_ptr<fles::TimesliceSource> fSource; //!
    //#else
    fles::TimesliceSource* fSource; //!
    //#endif

    Bool_t CheckTimeslice(const fles::Timeslice& ts);

    void UnpackSpadicCbmNetMessage(const fles::Timeslice& ts, size_t component);
#ifndef __CINT__
    void print_message(const spadic::Message& m);
#endif

    ClassDef(CbmFlibFileSource, 1)
};


#endif
