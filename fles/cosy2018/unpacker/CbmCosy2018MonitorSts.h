// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmCosy2018MonitorSts                         -----
// -----                Created 15/02/18  by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMCOSY2018MONITORSTS_H
#define CBMCOSY2018MONITORSTS_H

#ifndef __CINT__
    #include "Timeslice.hpp"
#endif

// Data
#include "StsXyterMessage.h"
#include "StsXyterFinalHit.h"

// CbmRoot
#include "CbmTSUnpack.h"
#include "CbmHistManager.h"

// C++11
#include <chrono>

// C/C++
#include <vector>
#include <set>
#include <map>

class CbmDigi;
class CbmCern2017UnpackParSts;

class CbmCosy2018MonitorSts: public CbmTSUnpack
{
public:

   CbmCosy2018MonitorSts();
   virtual ~CbmCosy2018MonitorSts();

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

   void SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize = 5 );

   void ResetAllHistos();
   void SaveAllHistos( TString sFileName = "" );

   void SetMsOverlap(size_t uOverlapMsNb = 1) { fuOverlapMsNb = uOverlapMsNb; }
   size_t GetMsOverlap()                      { return fuOverlapMsNb; }

   void SetPrintMessage( Bool_t bPrintMessOn = kTRUE,
                         stsxyter::MessagePrintMask ctrl = stsxyter::MessagePrintMask::msg_print_Hex |
                                                           stsxyter::MessagePrintMask::msg_print_Human )
                        { fbPrintMessages = bPrintMessOn; fPrintMessCtrl = ctrl; }
   void EnableChanHitDtPlot( Bool_t bEnable = kTRUE ) { fbChanHitDtEna = bEnable; }
   void EnableDualStsMode( Bool_t bEnable = kTRUE ) { fbDualStsEna = bEnable; }
   void SetPulserChannels( UInt_t uAsicA, UInt_t uChanA, UInt_t uAsicB, UInt_t uChanB,
                           UInt_t uAsicC, UInt_t uChanC, UInt_t uAsicD, UInt_t uChanD,
                           UInt_t uMaxNbMicroslices = 100 );
   void SetLongDurationLimits( UInt_t uDurationSeconds = 3600, UInt_t uBinSize = 1 );

private:
   size_t fuOverlapMsNb;      /** Ignore Overlap Ms: all fuOverlapMsNb MS at the end of timeslice **/

   // Parameters
   CbmCern2017UnpackParSts* fUnpackPar; //!
   UInt_t                   fuNrOfDpbs;       //! Total number of Sts DPBs in system
   std::map<UInt_t, UInt_t> fDpbIdIndexMap;   //! Map of DPB Identifier to DPB index
   UInt_t                   fuNbElinksPerDpb; //! Number of possible eLinks per DPB
   UInt_t                   fuNbStsXyters;    //! Number of StsXyter ASICs
   UInt_t                   fuNbChanPerAsic;  //! Number of channels per StsXyter ASIC => should be constant somewhere!!!!
   std::vector< std::vector< UInt_t > > fvuElinkToAsic;   //! Vector holding for each link the corresponding ASIC index [fuNrOfDpbs * fuNbElinksPerDpb]

   // Constants
   static const Int_t    kiMaxNbFlibLinks = 16;
   static const UInt_t   kuBytesPerMessage = 4;

   // Internal Control/status of monitor
      // Task configuration values
   Bool_t                fbPrintMessages;
   stsxyter::MessagePrintMask fPrintMessCtrl;
   Bool_t                fbChanHitDtEna;
   Bool_t                fbDualStsEna;
      // TS/MS info
   ULong64_t             fulCurrentTsIdx;
   ULong64_t             fulCurrentMsIdx;
      // Current data properties
   std::map< stsxyter::MessType, UInt_t > fmMsgCounter;
   UInt_t                fuCurrentEquipmentId;  //! Current equipment ID, tells from which DPB the current MS is originating
   UInt_t                fuCurrDpbId; //! Temp holder until Current equipment ID is properly filled in MS
   UInt_t                fuCurrDpbIdx;          //! Index of the DPB from which the MS currently unpacked is coming
   Int_t                 fiRunStartDateTimeSec; //! Start of run time since "epoch" in s, for the plots with date as X axis
   Int_t                 fiBinSizeDatePlots;    //! Bin size in s for the plots with date as X axis
//   std::vector< std::vector< UInt_t > > fvuCurrentTsMsb;       //! Current TS MSB for each eLink
      // Data format control
   std::vector< ULong64_t > fvulCurrentTsMsb;                   //! Current TS MSB for each DPB
   std::vector< UInt_t    > fvuCurrentTsMsbCycle;               //! Current TS MSB cycle for DPB
   std::vector< UInt_t    > fvuElinkLastTsHit;                  //! TS from last hit for DPB
      // Hits comparison
   std::vector< std::vector< ULong64_t > > fvulChanLastHitTime; //! Last hit time in bins for each Channel
   std::vector< std::vector< Double_t  > > fvdChanLastHitTime;  //! Last hit time in ns   for each Channel
   std::vector< Double_t >                               fvdMsTime;              //! Header time of each MS
   std::vector< std::vector< std::vector< UInt_t > > >   fvuChanNbHitsInMs;      //! Number of hits in each MS for each Channel
   std::vector< std::vector< std::vector< Double_t > > > fvdChanLastHitTimeInMs; //! Last hit time in bins in each MS for each Channel
   std::vector< std::vector< std::vector< UShort_t > > > fvusChanLastHitAdcInMs; //! Last hit ADC in bins in each MS for each Channel
   std::vector< std::vector< std::multiset< stsxyter::FinalHit > > > fvmChanHitsInTs; //! All hits (time & ADC) in bins in last TS for each Channel
      // Starting state book-keeping
   Double_t              fdStartTime;           /** Time of first valid hit (TS_MSB available), used as reference for evolution plots**/
   Double_t              fdStartTimeMsSz;       /** Time of first microslice, used as reference for evolution plots**/
   std::chrono::steady_clock::time_point ftStartTimeUnix; /** Time of run Start from UNIX system, used as reference for long evolution plots against reception time **/

      // Hits time-sorting
   std::multiset< stsxyter::FinalHit > fvmHitsInTs; //! All hits (time in bins, ADC in bins, asic, channel) in last TS, sorted by multiset with "<" operator
   stsxyter::FinalHit fLastSortedHit1N; //! Last sorted hit for STS 1 N
   stsxyter::FinalHit fLastSortedHit1P; //! Last sorted hit for STS 1 P
   stsxyter::FinalHit fLastSortedHit2N; //! Last sorted hit for STS 2 N
   stsxyter::FinalHit fLastSortedHit2P; //! Last sorted hit for STS 2 P
      // Coincidence histos
   UInt_t fuMaxNbMicroslices;

   // Histograms
   CbmHistManager* fHM;                 //! Histogram manager
   TH1* fhStsMessType;
   TH1* fhStsSysMessType;
   TH2* fhStsMessTypePerDpb;
   TH2* fhStsSysMessTypePerDpb;
   TH2* fhStsDpbRawTsMsb;
   TH2* fhStsDpbRawTsMsbSx;
   TH2* fhStsDpbRawTsMsbDpb;
   TH2* fhStsMessTypePerElink;
   TH2* fhStsSysMessTypePerElink;

   std::vector<TH1*> fhStsChanCounts;
   std::vector<TH2*> fhStsChanRawAdc;
   std::vector<TProfile*> fhStsChanRawAdcProf;
   std::vector<TH2*> fhStsChanRawTs;
   std::vector<TH2*> fhStsChanMissEvt;
   std::vector<TH2*> fhStsChanHitRateEvo;
   std::vector<TH1*> fhStsXyterRateEvo;
   std::vector<TH2*> fhStsChanHitRateEvoLong;
   std::vector<TH1*> fhStsXyterRateEvoLong;
   std::vector<TH2*> fhStsChanHitDt;
   std::vector<TH2*> fhStsChanHitDtNeg;
   std::vector<TH2*> fhStsChanHitsPerMs;
   std::vector<TH2*> fhStsChanSameMs;
   std::vector<TProfile2D*> fpStsChanSameMsTimeDiff;
   std::vector<TH2*> fhStsChanSameMsTimeDiff;

   Bool_t fbPulserTimeDiffOn;
   UInt_t fuPulserMaxNbMicroslices;
   std::vector<UInt_t> fvuPulserAsic;
   std::vector<UInt_t> fvuPulserChan;
   std::vector<TH1*> fhStsPulserChansTimeDiff;
   std::vector<TH2*> fhStsPulserChansTimeDiffEvo;
   std::vector<TH2*> fhStsPulserChansTimeDiffAdc;

   TH2* fhStsAsicTsMsb;

   Bool_t fbLongHistoEnable;
   UInt_t fuLongHistoNbSeconds;
   UInt_t fuLongHistoBinSizeSec;
   UInt_t fuLongHistoBinNb;
   std::vector<TH1*> fhFebRateEvoLong;
   std::vector<TH2*> fhFebChRateEvoLong;
/*
   std::vector<TH1*> fhFebRateEvoDate;
*/
      // Coincidences in same MS (unsorted hits)
   TH2 * fhStsSameMs1NP;
   TH2 * fhStsSameMs2NP;
   TH2 * fhStsSameMsN1N2;
   TH2 * fhStsSameMsP1P2;
   TH2 * fhStsSameMsN1P2;
   TH2 * fhStsSameMsP1N2;

   TH1 * fhStsSameMsCntEvoN1P1;
   TH1 * fhStsSameMsCntEvoN2P2;
   TH1 * fhStsSameMsCntEvoN1N2;
   TH1 * fhStsSameMsCntEvoP1P2;
   TH1 * fhStsSameMsCntEvoN1P2;
   TH1 * fhStsSameMsCntEvoP1N2;
   TH1 * fhStsSameMsCntEvoN1P1N2P2;

      // Coincidences in sorted hits
   TH1 * fhStsSortedDtN1P1;
   TH1 * fhStsSortedDtN2P2;
   TH1 * fhStsSortedDtN1N2;
   TH1 * fhStsSortedDtP1P2;
   TH1 * fhStsSortedDtN1P2;
   TH1 * fhStsSortedDtP1N2;
   TH2 * fhStsSortedMapN1P1;
   TH2 * fhStsSortedMapN2P2;
   TH2 * fhStsSortedMapN1N2;
   TH2 * fhStsSortedMapP1P2;
   TH2 * fhStsSortedMapN1P2;
   TH2 * fhStsSortedMapP1N2;

   TCanvas*  fcMsSizeAll;
   TH1*      fhMsSz[kiMaxNbFlibLinks];
   TProfile* fhMsSzTime[kiMaxNbFlibLinks];

   void CreateHistograms();

   void FillHitInfo(   stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx );
   void FillTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx = 0, UInt_t uMsIdx = 0);
   void FillEpochInfo( stsxyter::Message mess );

   CbmCosy2018MonitorSts(const CbmCosy2018MonitorSts&);
   CbmCosy2018MonitorSts operator=(const CbmCosy2018MonitorSts&);

   ClassDef(CbmCosy2018MonitorSts, 1)
};

#endif // CBMCOSY2018MONITORSTS_H
