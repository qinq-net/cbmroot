/** @file CbmHldSource.h
 ** @author Christian Simon <csimon@physi.uni-heidelberg.de>
 ** @date 2015-11-25
 **/

#ifndef CBMHLDSOURCE_H
#define CBMHLDSOURCE_H 1

#include "FairSource.h"

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
class CbmHldSource : public FairSource
{
  public:

    /** default constructor **/
    CbmHldSource();

    /** default destructor **/
    ~CbmHldSource();

    /** inherited pure virtual methods from FairSource **/
    Bool_t Init();
    Int_t ReadEvent(UInt_t=0);
    void Close();
    void Reset();

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
    
    TObjArray* fUnpackers;
    Int_t fNUnpackers;
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
