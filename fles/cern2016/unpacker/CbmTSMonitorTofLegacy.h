#// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSMonitorTof                            -----
// -----               Created 11.11.2016 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmTSMonitorTofLegacy_H
#define CbmTSMonitorTofLegacy_H

#ifndef __CINT__
  #include "Timeslice.hpp"
  #include "rocMess_wGet4v1.h"
#endif

#include "CbmTSUnpack.h"
#include "CbmHistManager.h"

#include "TClonesArray.h"
#include "Rtypes.h"

#include <vector>
#include <map>

class CbmDigi;
class CbmTofUnpackPar;

class CbmTSMonitorTofLegacy : public CbmTSUnpack
{
public:
  
   CbmTSMonitorTofLegacy();
   virtual ~CbmTSMonitorTofLegacy();
    
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
   
   void SetDiamondChannels( Int_t iGdpb = 0, Int_t iFeet = 2, Int_t iChannelA = 78, 
                            Int_t iChannelB = 70, Int_t iChannelC = 94, Int_t iChannelD = 86 );
                            
   void ResetAllHistos();

private:

   size_t fuMsAcceptsPercent; /** Reject Ms with index inside TS above this, assumes 100 MS per TS **/
   UInt_t fuMinNbGdpb;
   UInt_t fuCurrNbGdpb;

   Int_t fNrOfGdpbs;           // Total number of GDPBs in the system
   Int_t fNrOfFebsPerGdpb;     // Number of FEBs per GDPB
   Int_t fNrOfGet4PerFeb;      // Number of GET4s per FEB
   Int_t fNrOfChannelsPerGet4; // Number of channels in each GET4

   Int_t fNrOfGet4;            // Total number of Get4 chips in the system
   Int_t fNrOfGet4PerGdpb;     // Number of GET4s per GDPB
   
   Int_t fDiamondGdpb;
   Int_t fDiamondFeet;
   Int_t fDiamondChanA;
   Int_t fDiamondChanB;
   Int_t fDiamondChanC;
   Int_t fDiamondChanD;
   Int_t fDiamondTimeLastReset;

   std::vector<int> fMsgCounter;

   std::map<UInt_t, UInt_t> fGdpbIdIndexMap;

   CbmHistManager* fHM;  ///< Histogram manager

   /** Current epoch marker for each ROC and GET4 (first epoch in the stream initialises the map item) **/
   std::map<Int_t, std::map<Int_t, UInt_t> > fCurrentEpoch; 
   Int_t fNofEpochs; /** Current epoch marker for each ROC **/
   ULong_t fCurrentEpochTime;     /** Time stamp of current epoch **/
   Double_t fdStartTime; /** Time of first valid hit (epoch available), used as reference for evolution plots**/

   /** Used only if the channel rate plots are enabled **/
   /** Last Hit time for each ROC/GET4/Channel (first hit in the stream initialises the map item) **/
   std::map<Int_t, std::map<Int_t, std::map<Int_t, Double_t> > > fTsLastHit;    // * 6.25 ns

   Int_t fEquipmentId;

   CbmTofUnpackPar* fUnpackPar;      //!

   void CreateHistograms();

#ifndef __CINT__
   void FillHitInfo(ngdpb::Message, std::vector<TH2*>,
                    std::vector<TH1*>, std::vector<TH2*>,
                    std::vector<TH1*>, TH2*
                   );
   void FillEpochInfo(ngdpb::Message, TH2*);
   void PrintSlcInfo(ngdpb::Message);
   void PrintSysInfo(ngdpb::Message);
   void PrintGenInfo(ngdpb::Message);
#endif

   CbmTSMonitorTofLegacy(const CbmTSMonitorTofLegacy&);
   CbmTSMonitorTofLegacy operator=(const CbmTSMonitorTofLegacy&);

   ClassDef(CbmTSMonitorTofLegacy, 1)
};

#endif