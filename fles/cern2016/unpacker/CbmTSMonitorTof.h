// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSMonitorTof                            -----
// -----               Created 11.11.2016 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmTSMonitorTof_H
#define CbmTSMonitorTof_H

#ifndef __CINT__
  #include "Timeslice.hpp"
  #include "rocMess_wGet4v1.h"
#endif

#include "CbmTSUnpack.h"
#include "CbmHistManager.h"
//#include "CbmNxyterRawMessage.h"
//#include "CbmTofDigi.h"
//#include "CbmTofDigiExp.h"
//#include "CbmTbDaqBuffer.h"

#include "TClonesArray.h"

#include <vector>
#include <map>

class CbmDigi;
class CbmTofUnpackPar;

class CbmTSMonitorTof : public CbmTSUnpack
{
public:
  
   CbmTSMonitorTof( UInt_t uNbGdpb = 1 );
   virtual ~CbmTSMonitorTof();
    
   virtual Bool_t Init();
#ifndef __CINT__
   virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
#endif
   virtual void Reset();

   virtual void Finish();

   void SetParContainers();

   Bool_t InitContainers();

   Bool_t ReInitContainers();


   void FillOutput(CbmDigi* digi);

   void SetMsLimitLevel( size_t uAcceptBoundaryPct = 100 ) { fuMsAcceptsPercent = uAcceptBoundaryPct; }
   size_t GetMsLimitLevel( ) { return fuMsAcceptsPercent; }

private:

   size_t fuMsAcceptsPercent; /** Reject Ms with index inside TS above this, assumes 100 MS per TS **/
   UInt_t fuMinNbGdpb;
   UInt_t fuCurrNbGdpb;

   std::vector<int> fMsgCounter;
   std::map<UInt_t, UInt_t> fGdpbIdIndexMap;

   CbmHistManager* fHM;  ///< Histogram manager

   /** Current epoch marker for each ROC and GET4 (first epoch in the stream initialises the map item) **/
   std::map<Int_t, std::map<Int_t, UInt_t> > fCurrentEpoch; 
   Int_t fNofEpochs; /** Current epoch marker for each ROC **/
   ULong_t fCurrentEpochTime;     /** Time stamp of current epoch **/

   /** Used only if the channel rate plots are enabled **/
   /** Last Hit time for each ROC/GET4/Channel (first hit in the stream initialises the map item) **/
   std::map<Int_t, std::map<Int_t, std::map<Int_t, Double_t> > > fTsLastHit;    // * 6.25 ns

   Int_t fEquipmentId;

   CbmTofUnpackPar* fUnpackPar;      //!

   void CreateHistograms();

#ifndef __CINT__
   void FillHitInfo(ngdpb::Message);
   void FillEpochInfo(ngdpb::Message);
   void PrintSlcInfo(ngdpb::Message);
   void PrintSysInfo(ngdpb::Message);
   void PrintGenInfo(ngdpb::Message);
#endif

   CbmTSMonitorTof(const CbmTSMonitorTof&);
   CbmTSMonitorTof operator=(const CbmTSMonitorTof&);

   ClassDef(CbmTSMonitorTof, 1)
};

#endif