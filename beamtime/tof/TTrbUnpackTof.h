// -----------------------------------------------------------------------------
// ----- TTrbUnpackTof header file                                         -----
// -----                                                                   -----
// ----- created by C. Simon on 2014-03-08                                 -----
// -----                                                                   -----
// ----- based on TTrbUnpackTof by P.-A. Loizeau                           -----
// ----- https://subversion.gsi.de/fairroot/cbmroot/development/ploizeau/  -----
// -----   main/unpack/tof/TTrbUnpackTof.h                                 -----
// ----- revision 21379, 2013-08-24                                        -----
// -----------------------------------------------------------------------------

#ifndef TTRBUNPACKTOF_H
#define TTRBUNPACKTOF_H

// STL
#include <map>
#include <utility>

// General FairRoot unpacker of GSI/MBS events
#include "FairUnpack.h"

// Parameters
class TMbsUnpackTofPar;

// Iterator
namespace hadaq
{
  class TrbIterator;
  struct RawSubevent;
}

// Subunpacker
class TTofTrbTdcUnpacker;

// ROOT
class TClonesArray;
class TH1;

class TTrbUnpackTof : public FairUnpack
{
   public:
      TTrbUnpackTof( Short_t type, Short_t subType, Short_t procId, Short_t subCrate, Short_t control);
      virtual ~TTrbUnpackTof();

      // FairUnpack pure virtual methods
      virtual Bool_t Init();
      virtual Bool_t DoUnpack(Int_t* data, Int_t size);
      virtual void Reset();
      
      void WriteHistograms();

      void   SetSaveRawData( Bool_t bSaveRaw=kTRUE );
   protected:
      // FairUnpack pure virtual method
      // neither called by a FairSource daughter class nor by FairRunOnline
      // why made pure virtual in FairUnpack?
      virtual void Register();
    
   private:
      TTrbUnpackTof(const TTrbUnpackTof&);
      TTrbUnpackTof operator=(const TTrbUnpackTof&);

      // Methods
      Bool_t InitParameters();
      Bool_t CreateSubunpackers();
      Bool_t RegisterOutput();
      Bool_t ClearOutput();

      void DataErrorHandling(UInt_t uSubeventId, UInt_t uErrorPattern);

      // Parameters
      TMbsUnpackTofPar * fMbsUnpackPar;

      Int_t fiNbEvents;
      UInt_t fiCurrentEventNumber;
      UInt_t fiPreviousEventNumber;

      // Iterator
      hadaq::TrbIterator * fTrbIterator;
      
      // Subunpacker
      TTofTrbTdcUnpacker * fTrbTdcUnpacker;

      // Output objects
      TClonesArray * fTrbTdcBoardCollection;
      Bool_t         fbSaveRawTdcBoards;

      // Map to delay TDC data unpacking until a decision is made based on the CTS trigger pattern
      // key:   active TDC index
      // value: pair of hadaq rawsubevent pointer and subsubevent data index
      //        (input to TTofTrbTdcUnpacker::ProcessData)
      std::map<Int_t,std::pair<hadaq::RawSubevent*,UInt_t> > fTdcUnpackMap;

      void CreateHistograms();
//      void DeleteHistograms();

      UInt_t fuInDataTrbSebNb;
      UInt_t fuActiveTrbTdcNb;

      // Histograms
      TH1* fTrbTriggerPattern;
      TH1* fTrbTriggerType;
      TH1* fTrbEventNumberJump;
      std::vector<TH1*> fTrbSubeventSize;
      std::vector<TH1*> fTrbSubeventStatus;
      std::vector<TH1*> fTrbTdcWords;
      std::vector<TH1*> fTrbTdcProcessStatus;

      ClassDef(TTrbUnpackTof,1)
};

#endif
