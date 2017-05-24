// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSMonitorTofLab                         -----
// -----               Created 22.05.2017 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmTSMonitorTofLab_H
#define CbmTSMonitorTofLab_H

#ifndef __CINT__
#include "Timeslice.hpp"
#include "rocMess_wGet4v1.h"
#include "CbmTofStarData.h"
#endif

#include "CbmTSUnpack.h"
#include "CbmHistManager.h"

#include "TClonesArray.h"
#include "Rtypes.h"

#include <vector>
#include <map>
#include <chrono>

class CbmDigi;
class CbmTofUnpackPar;

class CbmTSMonitorTofLab: public CbmTSUnpack {
  public:

    CbmTSMonitorTofLab();
    virtual ~CbmTSMonitorTofLab();

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

    void SetMsOverlap(size_t uOverlapMsNb = 1) {
      fuOverlapMsNb = uOverlapMsNb;
    }
    size_t GetMsOverlap() {
      return fuOverlapMsNb;
    }

    void SetPerTsSpillOnThr( Int_t iThrIn = 10 ) { fiSpillOnThr = iThrIn; }
    void SetPerTsSpillOffThr( Int_t iThrIn = 3 ) { fiSpillOffThr = iThrIn; }
    void SetTsNbSpillOffThr( Int_t iThrIn = 10 ) { fiTsUnderOffThr = iThrIn; }

    void SetEpochSuppressedMode( Bool_t bEnable = kTRUE ) { fbEpochSuppModeOn = bEnable; }

    void SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize = 5 );

    inline void SetGet4Mode24b( Bool_t inGet4Mode24b = kTRUE ) { fbGet4M24b = inGet4Mode24b; }
    inline void SetGet4v20Mode( Bool_t inGet4v20Mode = kTRUE ) { fbGet4v20 = inGet4v20Mode; }
    inline void SetPulserMode( Bool_t inPulserMode = kTRUE ) { fbPulserMode = inPulserMode; SetPulserChans(); }
    inline void SetPulserFee( UInt_t inPulserGdpb, UInt_t inPulserFee ) { fuPulserGdpb = inPulserGdpb; fuPulserFee = inPulserFee; }
           void SetPulserChans( UInt_t inPulserChanA =  0, UInt_t inPulserChanB =  1, UInt_t inPulserChanC =  2,
                                UInt_t inPulserChanD =  3, UInt_t inPulserChanE =  4, UInt_t inPulserChanF =  5,
                                UInt_t inPulserChanG =  6, UInt_t inPulserChanH =  7, UInt_t inPulserChanI =  8,
                                UInt_t inPulserChanJ =  9, UInt_t inPulserChanK = 10, UInt_t inPulserChanL = 11,
                                UInt_t inPulserChanM = 12, UInt_t inPulserChanN = 13, UInt_t inPulserChanO = 14,
                                UInt_t inPulserChanP = 15 );

    void SaveAllHistos( TString sFileName = "" );
    void ResetAllHistos();
    void CyclePulserFee();

  private:

    size_t fuMsAcceptsPercent; /** Reject Ms with index inside TS above this, assumes 100 MS per TS **/
    size_t fuOverlapMsNb;      /** Ignore Overlap Ms: all fuOverlapMsNb MS at the end of timeslice **/
    UInt_t fuMinNbGdpb;
    UInt_t fuCurrNbGdpb;

    Int_t fNrOfGdpbs;           // Total number of GDPBs in the system
    Int_t fNrOfFebsPerGdpb;     // Number of FEBs per GDPB
    Int_t fNrOfGet4PerFeb;      // Number of GET4s per FEB
    Int_t fNrOfChannelsPerGet4; // Number of channels in each GET4

    Int_t fNrOfChannelsPerFeet; // Number of channels in each FEET
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
    std::vector< std::vector < ngdpb::Message > > fvmEpSupprBufferErrors;
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
    Long64_t* fCurrentEpoch; //!
    UInt_t* fuCurrentEpochCycle; //!

    Int_t fNofEpochs; /** Current epoch marker for each ROC **/
    ULong_t fCurrentEpochTime; /** Time stamp of current epoch **/
    Double_t fdStartTime; /** Time of first valid hit (epoch available), used as reference for evolution plots**/
    Double_t fdStartTimeMsSz; /** Time of first microslice, used as reference for evolution plots**/
    TCanvas* fcMsSizeAll;

    /** Used only if the channel rate or pulse hit difference plots are enabled **/
    /** Last Hit time for each gDPB/GET4/Channel **/
    std::vector< std::vector< std::vector< Double_t > > > fTsLastHit; // * 6.25 ns

    Int_t fEquipmentId;

    CbmTofUnpackPar* fUnpackPar;      //!

    std::chrono::time_point<std::chrono::system_clock> fTimeLastHistoSave;

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
    std::vector<TH1*> fFeetErrorRate_gDPB;
    std::vector<TH1*> fFeetRateLong_gDPB;
    std::vector<TH1*> fFeetErrorRateLong_gDPB;

    std::vector<TH1*> fFeetRateDate_gDPB;
    Int_t             fiRunStartDateTimeSec;
    Int_t             fiBinSizeDatePlots;

    ///* ASIC coincidences & offsets mapping *///
/*
    ULong64_t fulLastMsIdx;
    Bool_t    fbHitsInLastTs;
    std::vector< std::vector< Long64_t > > fvulHitEpochBuffLastTs;  //! Dims: [gDPB][hits]
    std::vector< TH2 * >                   fvhCoincOffsetEpochGet4; //! Dims: [gDPB - 1]
*/

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
    TH1 *     fhTokenMsgType;
    TH1 *     fhTriggerRate;
    TH2 *     fhCmdDaqVsTrig;
    TH2 *     fhStarTokenEvo;

    ///* STAR and pulser monitoring *///
    static const UInt_t kuNbChanTest = 16;
    Bool_t fbGet4M24b;
    Bool_t fbGet4v20;
    Bool_t fbPulserMode;
    UInt_t fuPulserGdpb;
    UInt_t fuPulserFee;
    UInt_t fuPulserChan[kuNbChanTest]; //! Always in first gDPB !!!
    std::vector<TH1 *> fhTimeDiffPulserChosenFee;
    std::vector<TH1 *> fhTimeDiffPulserChosenChPairs;
    TH2 * fhTimeRmsPulserChosenFee;
    TH1 * fhTimeRmsPulserChosenChPairs;
    Double_t fdLastRmsUpdateTime;
    std::vector<TH2 *> fhFtDistribPerCh;
    std::vector<TH1*> fChCountFall_gDPB;
    std::vector<TH2 *> fhFtDistribPerChFall;
    std::vector< std::vector<Int_t> > fviFtLastRise24b;
    std::vector< std::vector<Int_t> > fviFtLastFall24b;
    std::vector< std::vector<Double_t> > fvdTimeLastRise24b;
    std::vector< std::vector<Double_t> > fvdTimeLastFall24b;
    UInt_t fuRiseFallChSel;
    std::vector<TH2 *> fhFtLastRiseCurrFall;
    std::vector<TH2 *> fhFtCurrRiseLastFall;
    std::vector<TH2 *> fhFtLastRiseDistRise;
    std::vector<TH2 *> fhFtLastRiseDistFall;

    void CreateHistograms();

#ifndef __CINT__
    void FillHitInfo(ngdpb::Message);
    void FillEpochInfo(ngdpb::Message);
    void FillGet4ErrorInfo(ngdpb::Message);
    void PrintSlcInfo(ngdpb::Message);
    void PrintSysInfo(ngdpb::Message);
    void PrintGenInfo(ngdpb::Message);
    void FillStarTrigInfo(ngdpb::Message);
    void FillTrigEpochInfo(ngdpb::Message);
#endif

    inline Int_t GetArrayIndex(Int_t gdpbId, Int_t get4Id) {
      return gdpbId * fNrOfGet4PerGdpb + get4Id;
    }

    CbmTSMonitorTofLab(const CbmTSMonitorTofLab&);
    CbmTSMonitorTofLab operator=(const CbmTSMonitorTofLab&);

  ClassDef(CbmTSMonitorTofLab, 1)
};

#endif