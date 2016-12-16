// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSMonitorTofStar                        -----
// -----               Created 15.12.2016 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmTSMonitorTofStar_H
#define CbmTSMonitorTofStar_H

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

class CbmTSMonitorTofStar: public CbmTSUnpack {
  public:

    CbmTSMonitorTofStar();
    virtual ~CbmTSMonitorTofStar();

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

    void SetPerTsSpillOnThr( Int_t iThrIn = 10 ) { fiSpillOnThr = iThrIn; }
    void SetPerTsSpillOffThr( Int_t iThrIn = 3 ) { fiSpillOffThr = iThrIn; }
    void SetTsNbSpillOffThr( Int_t iThrIn = 10 ) { fiTsUnderOffThr = iThrIn; }

    void SetEpochSuppressedMode( Bool_t bEnable = kTRUE ) { fbEpochSuppModeOn = bEnable; }

    void SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize = 5 );

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

    Int_t fiCountsLastTs;
    Int_t fiSpillOnThr;
    Int_t fiSpillOffThr;
    Int_t fiTsUnderOff;
    Int_t fiTsUnderOffThr;
    Double_t fdDetLastTime;
    Double_t fdDetTimeLastTs;
    Bool_t fbSpillOn;
    UInt_t fSpillIdx;

    Bool_t fbEpochSuppModeOn;
#ifndef __CINT__
    std::vector< std::vector < ngdpb::Message > > fvmEpSupprBuffer;
#endif

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
    Double_t fdStartTimeMsSz; /** Time of first microslice, used as reference for evolution plots**/
    TCanvas* fcMsSizeAll;

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
    TH2* fHistSpill;
    TH1* fHistSpillLength;
    TH1* fHistSpillCount;
    TH2* fHistSpillQA;

    std::vector<TH2*> fRaw_Tot_gDPB;
    std::vector<TH1*> fChCount_gDPB;
    std::vector<TH2*> fChannelRate_gDPB;
    std::vector<TH1*> fFeetRate_gDPB;

    std::vector<TH1*> fFeetRateDate_gDPB;
    Int_t             fiRunStartDateTimeSec;
    Int_t             fiBinSizeDatePlots;
    
    ///* STAR TRIGGER detection *///
    ULong64_t fulGdpbTsMsb;
    ULong64_t fulGdpbTsLsb;
    ULong64_t fulStarTsMsb;
    ULong64_t fulStarTsMid;
    ULong64_t fulGdpbTsFullLast;
    ULong64_t fulStarTsFullLast;
    UInt_t    fuStarTokenLast;
    UInt_t    fuStarDaqCmdLast;
    UInt_t    fuStarTrigCmdLast;

    void CreateHistograms();

#ifndef __CINT__
    void FillHitInfo(ngdpb::Message);
    void FillEpochInfo(ngdpb::Message);
    void PrintSlcInfo(ngdpb::Message);
    void PrintSysInfo(ngdpb::Message);
    void PrintGenInfo(ngdpb::Message);
    void FillStarTrigInfo(ngdpb::Message);
#endif

    inline Int_t GetArrayIndex(Int_t gdpbId, Int_t get4Id) {
      return gdpbId * fNrOfGet4PerGdpb + get4Id;
    }

    CbmTSMonitorTofStar(const CbmTSMonitorTofStar&);
    CbmTSMonitorTofStar operator=(const CbmTSMonitorTofStar&);

  ClassDef(CbmTSMonitorTofStar, 1)
};

#endif
