// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmFlibCern2016Source                       -----
// -----                    Created 20.06.2016 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMFLIBCERN2016SOURCE_H
#define CBMFLIBCERN2016SOURCE_H

#ifndef __CINT__
  #include "TimesliceSource.hpp"
  #include "Timeslice.hpp"
  #include "MicrosliceDescriptor.hpp"
//  #include "Message.hpp"
#endif

#include "FairSource.h"
#include "CbmTSUnpack.h"
#include "CbmTbDaqBuffer.h"

#include "TString.h"
#include "TClonesArray.h"
#include "TList.h"
#include "TObjString.h"
#include "TStopwatch.h"

#include <memory>
#include <map>

class TH1I;

class CbmFlibCern2016Source : public FairSource
{
  public:
    CbmFlibCern2016Source();
    CbmFlibCern2016Source(const CbmFlibCern2016Source& source);
    virtual ~CbmFlibCern2016Source();

    Bool_t Init();
    Int_t ReadEvent(UInt_t);

    Source_Type GetSourceType() {return kONLINE;}
  
    virtual void SetParUnpackers();

    virtual Bool_t InitUnpackers();

    virtual Bool_t ReInitUnpackers();

    void Close();
    void Reset();

    void SetFileName(TString name) { fFileName = name; fInputFileList.Add(new TObjString(name));}
    void SetHostName(TString name) { fHost = name; }
    void SetPortNumber(Int_t port) { fPort = port; }

    void AddUnpacker(CbmTSUnpack* unpacker, Int_t flibId, Int_t detId)
    { 
      fUnpackers.insert ( std::pair<Int_t,CbmTSUnpack*>(flibId,unpacker) ); 
      fDetectorSystemMap.insert ( std::pair<Int_t,Int_t>(detId,flibId) ); 
    }    

    void AddFile(const char * name) {       
      fInputFileList.Add(new TObjString(name));
    }
    
    UInt_t GetTsCount() { return fTSCounter; }
    UInt_t GetNofTSSinceLastTS() { return fNofTSSinceLastTS; }

    void SetDataReductionMuch( UInt_t uTsReduction = 15 ) { fuTsReduction = uTsReduction; }

  private:
  
    TString fFileName;
    TList   fInputFileList;    ///< List of input files
    Int_t   fFileCounter;
    TString fHost;
    Int_t   fPort;

    std::map<Int_t, CbmTSUnpack*> fUnpackers;
    std::map<Int_t, Int_t> fDetectorSystemMap; //! Map detector system id to flib system id 

    CbmTbDaqBuffer* fBuffer;
 
    UInt_t fTSNumber;
    UInt_t fTSCounter;
    TStopwatch fTimer;

    Bool_t fBufferFillNeeded; /** True if the input buffer has to be filled again **/  

    TH1I* fHistoMissedTS;
    Int_t fNofTSSinceLastTS;
    
    UInt_t fuTsReduction;

#ifndef __CINT__
    fles::TimesliceSource* fSource; //!
    Bool_t CheckTimeslice(const fles::Timeslice& ts);
    void PrintMicroSliceDescriptor(const fles::MicrosliceDescriptor& mdsc);
#endif

    Int_t FillBuffer();
    Int_t GetNextEvent();
    Bool_t OpenNextFile();

    CbmFlibCern2016Source operator=(const CbmFlibCern2016Source&);

    ClassDef(CbmFlibCern2016Source, 1)
};


#endif
