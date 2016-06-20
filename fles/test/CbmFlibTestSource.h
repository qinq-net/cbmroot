// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmFlibTestSource                       -----
// -----                    Created 20.06.2016 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMFLIBTESTSOURCE_H
#define CBMFLIBTESTSOURCE_H

#ifndef __CINT__
  #include "TimesliceSource.hpp"
  #include "Timeslice.hpp"
  #include "MicrosliceDescriptor.hpp"
//  #include "Message.hpp"
#endif

#include "FairSource.h"
#include "CbmTSUnpack.h"

#include "TString.h"
#include "TClonesArray.h"
#include "TList.h"
#include "TObjString.h"
#include "TStopwatch.h"

#include <memory>
#include <map>

class CbmDaqBuffer;

class CbmFlibTestSource : public FairSource
{
  public:
    CbmFlibTestSource();
    CbmFlibTestSource(const CbmFlibTestSource& source);
    virtual ~CbmFlibTestSource();

    Bool_t Init();
#ifdef _NewFairSource
    Int_t ReadEvent(UInt_t);
#else                
    Int_t ReadEvent();      
#endif    

#ifdef VERSION_LESS_151102
    enum Source_Type {kONLINE, kFILE};
#endif            

    Source_Type GetSourceType() {return kONLINE;}
  
    virtual void SetParUnpackers() {;}

    virtual Bool_t InitUnpackers() {return kTRUE;}

    virtual Bool_t ReInitUnpackers() {return kTRUE;}

    void Close();
    void Reset();

    void SetFileName(TString name) { fFileName = name; fInputFileList.Add(new TObjString(name));}
    void SetHostName(TString name) { fHost = name; }
    void SetPortNumber(Int_t port) { fPort = port; }

    void AddUnpacker(CbmTSUnpack* unpacker, Int_t id)
    { fUnpackers.insert ( std::pair<Int_t,CbmTSUnpack*>(id,unpacker) ); }    

    void AddFile(const char * name) {       
      fInputFileList.Add(new TObjString(name));
    }

  private:
  
    TString fFileName;
    TList   fInputFileList;    ///< List of input files
    Int_t   fFileCounter;
    TString fHost;
    Int_t   fPort;

    std::map<Int_t, CbmTSUnpack*> fUnpackers;

    //    CbmDaqBuffer* fBuffer;
 
    UInt_t fTSNumber;
    UInt_t fTSCounter;
    TStopwatch fTimer;

#ifndef __CINT__
    fles::TimesliceSource* fSource; //!
    Bool_t CheckTimeslice(const fles::Timeslice& ts);
    void PrintMicroSliceDescriptor(const fles::MicrosliceDescriptor& mdsc);
#endif

    CbmFlibTestSource operator=(const CbmFlibTestSource&);

    ClassDef(CbmFlibTestSource, 1)
};


#endif
