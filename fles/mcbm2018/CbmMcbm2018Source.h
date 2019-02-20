// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                          CbmMcbm2018Source                     -----
// -----                    Created 19.01.2018 by P.-A. Loizeau            -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMMCBM2018SOURCE_H
#define CBMMCBM2018SOURCE_H

#include "TimesliceSource.hpp"
#include "Timeslice.hpp"
#include "MicrosliceDescriptor.hpp"

#include "FairSource.h"
#include "CbmMcbmUnpack.h"
#include "CbmTbDaqBuffer.h"

#include "TString.h"
#include "TClonesArray.h"
#include "TList.h"
#include "TObjString.h"
#include "TStopwatch.h"

#include <memory>
#include <map>
#include <unordered_set>

class TH1I;
class TProfile;

class CbmMcbm2018Source : public FairSource
{
  public:
    CbmMcbm2018Source();

    CbmMcbm2018Source(const CbmMcbm2018Source& source) = delete;
    CbmMcbm2018Source operator=(const CbmMcbm2018Source&) = delete;

    virtual ~CbmMcbm2018Source();

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

    void AddUnpacker(CbmMcbmUnpack* unpacker, Int_t flibId, Int_t detId)
    {
      fUnpackers.insert( std::pair<Int_t,CbmMcbmUnpack*>(flibId, unpacker) );
      fDetectorSystemMap.insert( std::pair<Int_t,CbmMcbmUnpack*>(detId, unpacker) );
    }

    void AddFile(const char * name) {
      fInputFileList.Add(new TObjString(name));
    }

    UInt_t GetTsCount() { return fTSCounter; }
    UInt_t GetNofTSSinceLastTS() { return fNofTSSinceLastTS; }

    void SetDataReductionMuch( UInt_t uTsReduction = 15 ) { fuTsReduction = uTsReduction; }

    void EnableDataOutput( Bool_t bFlagIn = kTRUE ){ fbOutputData = bFlagIn; }

  private:

    TString fFileName;
    TList   fInputFileList;    ///< List of input files
    Int_t   fFileCounter;
    TString fHost;
    Int_t   fPort;
    Int_t   fbOutputData;

    std::multimap<Int_t, CbmMcbmUnpack*> fUnpackers; //! List pairs of system ID and unpacker pointer (unpacker can appear multiple times)
    std::map<Int_t, CbmMcbmUnpack*> fDetectorSystemMap; //! Map detector system id to flib system id
    std::unordered_set<CbmMcbmUnpack*> fUnpackersToRun; //! List of all unpackers for which at least one matching container was found

    CbmTbDaqBuffer* fBuffer;

    UInt_t fTSNumber;
    UInt_t fTSCounter;
    TStopwatch fTimer;

    Bool_t fBufferFillNeeded; /** True if the input buffer has to be filled again **/

    TH1I*     fHistoMissedTS; //!
    TProfile* fHistoMissedTSEvo; //!
    Int_t fNofTSSinceLastTS;

    UInt_t fuTsReduction;

    std::unique_ptr<fles::TimesliceSource> fSource; //!

    Bool_t CheckTimeslice(const fles::Timeslice& ts);
    void PrintMicroSliceDescriptor(const fles::MicrosliceDescriptor& mdsc);

    Int_t FillBuffer();
    Int_t GetNextEvent();

    ClassDef(CbmMcbm2018Source, 1)
};


#endif
