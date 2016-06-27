/** @file CbmHldSource.h
 ** @author Christian Simon <csimon@physi.uni-heidelberg.de>
 ** @date 2015-11-25
 **/

#ifndef CBMHLDSOURCE_H
#define CBMHLDSOURCE_H 1

#ifdef VERSION_LESS_151102
#include "FairSource.h"
#else
#include "FairOnlineSource.h"
#endif

#include "FairUnpack.h"

#include "TObjArray.h"
#include "TString.h"

namespace hadaq
{
  struct RawEvent;
}

class FairUnpack;

class TList;

//namespace std
//{
//  class ifstream;
//}


/** @class CbmHldSource
 ** @brief ...
 ** @author C. Simon <csimon@physi.uni-heidelberg.de>
 ** @version 1.0
 **
 ** ...
 **
 **/
#ifdef VERSION_LESS_151102
class CbmHldSource : public FairSource
#else
class CbmHldSource : public FairOnlineSource
#endif
{
  public:

    /** default constructor **/
    CbmHldSource();

    /** default destructor **/
    ~CbmHldSource();

    /** inherited pure virtual methods from FairSource **/
    Bool_t Init();
    Int_t ReadEvent(UInt_t = 0);

#ifdef VERSION_LESS_151102
    enum Source_Type {kONLINE, kFILE};
#endif

    Source_Type GetSourceType() {return kONLINE;}
    
    void Close();
#ifdef VERSION_LESS_151102
    void Reset();
#endif
    void AddUnpacker(FairUnpack* unpacker) { fUnpackers->Add(unpacker); }
    void AddFile(const TString& tFileName);
    void AddPath(const TString& tFileDirectory,
                 const TString& tFileNameWildCard);

  protected:

  private:
    /**private methods*/

    /** copy constructor **/
    CbmHldSource(const CbmHldSource&);

    /** assignment operator **/
    CbmHldSource& operator=(const CbmHldSource&);

    Bool_t OpenNextFile();
    void CloseFile();

    /**private members*/
    
#ifdef VERSION_LESS_151102
    TObjArray* fUnpackers;
    Int_t fNUnpackers;
#endif
    TList* fFileNames;
    Int_t fNFiles;
    Int_t fCurrentFile;
    TString fCurrentFileName;
  	Int_t fNEvents;
  	Int_t fCurrentEvent;
    std::ifstream* fFileStream;
    Char_t* fDataBuffer;
    Bool_t fNoMoreEvents;

    ClassDef(CbmHldSource,1)
};

#endif
