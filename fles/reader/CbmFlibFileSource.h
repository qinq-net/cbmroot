// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmFlibFileSource                       -----
// -----                    Created 08.08.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMFLIBFILESOURCE_H
#define CBMFLIBFILESOURCE_H

#ifndef __CINT__
  #include "TimesliceSource.hpp"
  #include "Timeslice.hpp"
  #include "MicrosliceDescriptor.hpp"
  #include "Message.hpp"
#endif

#include "FairSource.h"

#include "TString.h"
#include "TClonesArray.h"

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
    void SetHostName(TString name) { fFileName = name; }
    void SetPortNumber(Int_t port) { fPort = port; }

  private:
  
    TString fFileName;
    TString fHost;
    Int_t   fPort;
    
    // --- Output arrays
    TClonesArray* fSpadicRaw;         ///< Output array of CbmSpadicRawMessage

#ifndef __CINT__
    fles::TimesliceSource* fSource; //!
    Bool_t CheckTimeslice(const fles::Timeslice& ts);
    void UnpackSpadicCbmNetMessage(const fles::Timeslice& ts, size_t component);
    void print_message(const spadic::Message& m);
    void PrintMicroSliceDescriptor(const fles::MicrosliceDescriptor& mdsc);
#endif

    CbmFlibFileSource operator=(const CbmFlibFileSource&);

    ClassDef(CbmFlibFileSource, 1)
};


#endif
