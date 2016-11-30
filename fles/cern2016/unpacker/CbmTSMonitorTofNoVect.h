#// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSMonitorTof                            -----
// -----               Created 11.11.2016 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmTSMonitorTofNoVect_H
#define CbmTSMonitorTofNoVect_H

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

class CbmTSMonitorTofNoVect: public CbmTSUnpack {
  public:

    CbmTSMonitorTofNoVect();
    virtual ~CbmTSMonitorTofNoVect();

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

    void SetMsLimitLevel(size_t uAcceptBoundaryPct = 100) {
      fuMsAcceptsPercent = uAcceptBoundaryPct;
    }
    size_t GetMsLimitLevel() {
      return fuMsAcceptsPercent;
    }

    void SetDiamondChannels(Int_t iGdpb = 0, Int_t iFeet = 2, Int_t iChannelA =
        78, Int_t iChannelB = 70, Int_t iChannelC = 94, Int_t iChannelD = 86);

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

    Int_t fGdpbId; // Id (hex number)of the GDPB which is read from the message
    Int_t fGdpbNr; // running number (0 to fNrOfGdpbs) of the GDPB in the
    Int_t fGet4Id; // running number (0 to fNrOfGet4PerGdpb) of the Get4 chip of a unique GDPB
    Int_t fGet4Nr; // running number (0 to fNrOfGet4) of the Get4 chip in the system

    std::vector<int> fMsgCounter;

    std::map<UInt_t, UInt_t> fGdpbIdIndexMap;

    CbmHistManager* fHM;  ///< Histogram manager

    /** Current epoch marker for each GDPB and GET4
     * (first epoch in the stream initializes the map item)
     * pointer points to an array of size fNrOfGdpbs * fNrOfGet4PerGdpb
     * The correct array index is calculated using the function
     * GetArrayIndex(gdpbId, get4Id)
     **/
    Int_t* fCurrentEpoch; //!

    Int_t fNofEpochs; /** Current epoch marker for each ROC **/
    ULong_t fCurrentEpochTime; /** Time stamp of current epoch **/
    Double_t fdStartTime; /** Time of first valid hit (epoch available), used as reference for evolution plots**/

    /** Used only if the channel rate plots are enabled **/
    /** Last Hit time for each ROC/GET4/Channel (first hit in the stream initializes the map item) **/
    std::map<Int_t, std::map<Int_t, std::map<Int_t, Double_t> > > fTsLastHit; // * 6.25 ns

    Int_t fEquipmentId;

    CbmTofUnpackPar* fUnpackPar;      //!

    TH1* fHistMessType;
    TH1* fHistSysMessType;
    TH2* fHistGet4MessType;
    TH2* fHistGet4ChanErrors;
    TH2* fHistGet4EpochFlags;
    TH2* fHistDiamond;

    std::vector<TH2*> fRaw_Tot_gDPB;
    std::vector<TH1*> fChCount_gDPB;
    std::vector<TH2*> fChannelRate_gDPB;
    std::vector<TH1*> fFeetRate_gDPB;

    void CreateHistograms();

#ifndef __CINT__
    void FillHitInfo(ngdpb::Message);
    void FillEpochInfo(ngdpb::Message);
    void PrintSlcInfo(ngdpb::Message);
    void PrintSysInfo(ngdpb::Message);
    void PrintGenInfo(ngdpb::Message);
#endif

    inline Int_t GetArrayIndex(Int_t gdpbId, Int_t get4Id) {
      return gdpbId * fNrOfGet4PerGdpb + get4Id;
    }

    CbmTSMonitorTofNoVect(const CbmTSMonitorTofNoVect&);
    CbmTSMonitorTofNoVect operator=(const CbmTSMonitorTofNoVect&);

  ClassDef(CbmTSMonitorTofNoVect, 1)
};

#endif
