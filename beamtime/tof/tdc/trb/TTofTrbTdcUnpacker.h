// -----------------------------------------------------------------------------
// ----- TTofTrbTdcUnpacker header file                                    -----
// -----                                                                   -----
// ----- created by C. Simon on 2014-04-02                                 -----
// -----                                                                   -----
// ----- based on TTofVftxUnpacker by P.-A. Loizeau                        -----
// ----- https://subversion.gsi.de/fairroot/cbmroot/development/ploizeau/  -----
// -----   main/unpack/tof/tdc/vftx/TTofVftxUnpacker.h                     -----
// ----- revision 20754, 2013-07-17                                        -----
// -----------------------------------------------------------------------------

#ifndef TTOFTRBTDCUNPACKER_H_
#define TTOFTRBTDCUNPACKER_H_

#include "TObject.h"

#include <vector>
#include <map>

class TMbsUnpackTofPar;

namespace hadaq
{
  class RawSubevent;
}
/*
namespace trbtdc
{
  enum TrbTdcProcessStatus;
}
*/

class TClonesArray;
class TH1;
class TDirectory;

class TTofTrbTdcUnpacker : public TObject
{
   public:
      TTofTrbTdcUnpacker( TMbsUnpackTofPar * parIn );
      ~TTofTrbTdcUnpacker();

      // unsure if needed
      // never called in the unpacker
      void Clear(Option_t *option);

      Int_t ProcessData( hadaq::RawSubevent* tSubevent, UInt_t uStartIndex );

      void SetCalibTrigger( UInt_t uTriggerType ) { fbCalibTrigger = (uTriggerType & 0xd); };

      void CreateHistos();
      void FillHistos();
      void WriteHistos( TDirectory* inDir);
      void DeleteHistos();

   private:   
      // no default Copy constructor and = OP as class not meant to be copied
      TTofTrbTdcUnpacker(const TTofTrbTdcUnpacker&);
      TTofTrbTdcUnpacker& operator=(const TTofTrbTdcUnpacker&);

      TMbsUnpackTofPar * fParUnpack;
      UInt_t   fuNbActiveTrbTdc;
      Bool_t fbCalibTrigger;
      Bool_t fbJointEdgesChannel;
      TClonesArray * fTrbTdcBoardCollection;

      // TDC histograms
      TH1* fTrbTdcRingBufferOverflow;
      static std::map<Int_t,Bool_t> fmRingBufferOverflow;

      // TDC board histograms
      // channel occupancy distribution
      std::vector<TH1*> fTrbTdcChannelOccupancy;
      // channel fine time overshoot distribution
      std::vector<TH1*> fTrbTdcChannelFineTimeOvershoot;
      // channel undetected hits distribution
      std::vector<TH1*> fTrbTdcChannelUnprocessedHits;

      // TDC channel histograms
      // channel fine time distribution
      std::vector< std::vector<TH1*> > fTrbTdcChannelFineTime;
      // channel coarse time distribution
      std::vector< std::vector<TH1*> > fTrbTdcChannelCoarseTime;

      
   ClassDef(TTofTrbTdcUnpacker, 1)
};

#endif
