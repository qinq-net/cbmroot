#ifndef CBMRICHTRBUNPACK2_H
#define CBMRICHTRBUNPACK2_H

#include "CbmRichTrbDefines.h"

#include <TH1.h>
#include <TString.h>
#include "FairSource.h"

#include <vector>
#include <map>

class CbmRawSubEvent;
class CbmTrbRawHit;
class TClonesArray;
class CbmTrbIterator;

class CbmRichTrbUnpack2 : public FairSource
{
public: // methods

   CbmRichTrbUnpack2(TString hldFileName);
   virtual ~CbmRichTrbUnpack2();

   Bool_t Init();
   Int_t ReadEvent();
   void Close();
   void Reset();

private: // methods

   void ReadInputFileToMemory();
   //void ReadEvents();

   void ProcessSubEvent(CbmRawSubEvent* subEvent);
   void ProcessSubSubEvent(UInt_t* data, UInt_t size, UInt_t subEventId);

   CbmRichTrbUnpack2(const CbmRichTrbUnpack2&) {;}
   CbmRichTrbUnpack2 operator=(const CbmRichTrbUnpack2&) {;}

private: // data members

   TString fHldFileName;

   CbmTrbIterator* fTrbIter;
   UInt_t fNofRawEvents;
   UInt_t fMaxNofRawEvents;

   Char_t* fDataPointer;
   UInt_t fDataSize;

   TClonesArray* fTrbRawHits; // Output raw hits

   ClassDef(CbmRichTrbUnpack2,1)
};

#endif // CBMRICHTRBUNPACK2_H
