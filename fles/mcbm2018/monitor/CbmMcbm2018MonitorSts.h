// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                   CbmMcbm2018MonitorSts                        -----
// -----                Created 11/05/18  by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMMCBM2018MONITORSTS_H
#define CBMMCBM2018MONITORSTS_H

#ifndef __CINT__
    #include "Timeslice.hpp"
#endif

// Data
#include "StsXyterMessage.h"
#include "StsXyterFinalHit.h"

// CbmRoot
#include "CbmMcbmUnpack.h"
#include "CbmHistManager.h"

// C++11
#include <chrono>

// C/C++
#include <vector>
#include <set>
#include <map>

class CbmDigi;
class CbmCern2017UnpackParHodo;
class CbmCern2017UnpackParSts;

class CbmMcbm2018MonitorSts: public CbmMcbmUnpack
{
public:

   CbmMcbm2018MonitorSts();
   virtual ~CbmMcbm2018MonitorSts();

   virtual Bool_t Init();
#ifndef __CINT__
   virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
#endif
   virtual void Reset();

   virtual void Finish();

   void SetParContainers();

   Bool_t InitContainers();

   Bool_t ReInitContainers();

   virtual void AddMsComponentToList( size_t component, UShort_t usDetectorId );
   virtual void SetNbMsInTs( size_t uCoreMsNb, size_t uOverlapMsNb );
   void SetMsOverlap(size_t uOverlapMsNb = 1) { fuNbOverMsPerTs = uOverlapMsNb; }
   size_t GetMsOverlap()                      { return fuNbOverMsPerTs; }

   void FillOutput(CbmDigi* digi);

   void SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize = 5 );

   void ResetAllHistos();
   void SaveAllHistos( TString sFileName = "" );
   void SavePulserHistos( TString sFileName = "" );
   void SetHistoFileName( TString sFileName = "data/SetupHistos.root" ) { fsHistoFileFullname = sFileName; }

   void SetPrintMessage( Bool_t bPrintMessOn = kTRUE,
                         stsxyter::MessagePrintMask ctrl = stsxyter::MessagePrintMask::msg_print_Hex |
                                                           stsxyter::MessagePrintMask::msg_print_Human )
                        { fbPrintMessages = bPrintMessOn; fPrintMessCtrl = ctrl; }
   void SetLongDurationLimits( UInt_t uDurationSeconds = 3600, UInt_t uBinSize = 1 );
   void SetCoincidenceBorder( Double_t dCenterPos, Double_t dBorderVal );

private:
   // Parameters
      // FLES containers
   std::vector< size_t >    fvMsComponentsList; //!
   size_t                   fuNbCoreMsPerTs; //!
   size_t                   fuNbOverMsPerTs; //!
   Bool_t                   fbIgnoreOverlapMs; //! /** Ignore Overlap Ms: all fuOverlapMsNb MS at the end of timeslice **/
      // Unpacking and mapping
   CbmCern2017UnpackParHodo* fUnpackParHodo; //!
   UInt_t                   fuNrOfDpbs;       //! Total number of Sts DPBs in system
   std::map<UInt_t, UInt_t> fDpbIdIndexMap;   //! Map of DPB Identifier to DPB index
   UInt_t                   fuNbElinksPerDpb; //! Number of possible eLinks per DPB
   UInt_t                   fuNbStsXyters;    //! Number of StsXyter ASICs
   UInt_t                   fuNbChanPerAsic;  //! Number of channels per StsXyter ASIC => should be constant somewhere!!!!
   UInt_t                   fuNbFebs;         //! Number of StsXyter ASICs
   std::vector< std::vector< UInt_t > > fvuElinkToAsic;   //! Vector holding for each link the corresponding ASIC index [fuNrOfDpbs * fuNbElinksPerDpb]

   // Constants
   static const Int_t    kiMaxNbFlibLinks = 16;
   static const UInt_t   kuBytesPerMessage = 4;

   // Internal Control/status of monitor
      // Histo File name and path
   TString fsHistoFileFullname;
      // Task configuration values
   Bool_t                fbPrintMessages;
   stsxyter::MessagePrintMask fPrintMessCtrl;
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

      // Data format control
   std::vector< ULong64_t > fvulCurrentTsMsb;                   //! Current TS MSB for each DPB
   std::vector< UInt_t    > fvuCurrentTsMsbCycle;               //! Current TS MSB cycle for DPB
   std::vector< UInt_t    > fvuInitialHeaderDone;               //! Flag set after seeing MS header in 1st MS for DPB
   std::vector< UInt_t    > fvuInitialTsMsbCycleHeader;         //! TS MSB cycle from MS header in 1st MS for DPB
   std::vector< UInt_t    > fvuElinkLastTsHit;                  //! TS from last hit for DPB
      // Hits comparison
   std::vector< std::vector< ULong64_t > > fvulChanLastHitTime;   //! Last hit time in bins for each Channel
   std::vector< std::vector< Double_t  > > fvdChanLastHitTime;    //! Last hit time in ns   for each Channel
   std::vector< Double_t >                               fvdMsTime;                  //! Header time of each MS
   std::vector< std::vector< std::vector< UInt_t > > >   fvuChanNbHitsInMs;          //! Number of hits in each MS for each Channel
   std::vector< std::vector< std::vector< Double_t > > > fvdChanLastHitTimeInMs;     //! Last hit time in bins in each MS for each Channel
   std::vector< std::vector< std::vector< UShort_t > > > fvusChanLastHitAdcInMs;     //! Last hit ADC in bins in each MS for each Channel
//   std::vector< std::vector< std::multiset< stsxyter::FinalHit > > > fvmChanHitsInTs; //! All hits (time & ADC) in bins in last TS for each Channel
      // Starting state book-keeping
   Double_t              fdStartTime;           /** Time of first valid hit (TS_MSB available), used as reference for evolution plots**/
   Double_t              fdStartTimeMsSz;       /** Time of first microslice, used as reference for evolution plots**/
   std::chrono::steady_clock::time_point ftStartTimeUnix; /** Time of run Start from UNIX system, used as reference for long evolution plots against reception time **/

      // Hits time-sorting
   std::vector< stsxyter::FinalHit > fvmHitsInMs; //! All hits (time in bins, ADC in bins, asic, channel) in last TS, sorted with "<" operator
   std::vector< std::vector< stsxyter::FinalHit > > fvmAsicHitsInMs; //! All hits (time in bins, ADC in bins, asic, channel) in last TS, per ASIC, sorted with "<" operator
   std::vector< std::vector< stsxyter::FinalHit > > fvmFebHitsInMs; //! All hits (time in bins, ADC in bins, asic, channel) in last TS, per FEB, sorted with "<" operator
      // Coincidence histos
   UInt_t fuMaxNbMicroslices;
      // Rate evolution histos
   Bool_t fbLongHistoEnable;
   UInt_t fuLongHistoNbSeconds;
   UInt_t fuLongHistoBinSizeSec;
   UInt_t fuLongHistoBinNb;

      // Coincidences in sorted hits
   Double_t fdCoincCenter;  // ns
   Double_t fdCoincBorder;  // ns, +/-
   Double_t fdCoincMin;     // ns
   Double_t fdCoincMax;     // ns

   // Histograms
   CbmHistManager* fHM;                 //! Histogram manager
   TH1* fhStsMessType;
   TH1* fhStsSysMessType;
   TH2* fhStsMessTypePerDpb;
   TH2* fhStsSysMessTypePerDpb;
/*
   TH2* fhStsDpbRawTsMsb;
   TH2* fhStsDpbRawTsMsbSx;
   TH2* fhStsDpbRawTsMsbDpb;
*/
   TH2* fhStsMessTypePerElink;
   TH2* fhStsSysMessTypePerElink;

   std::vector<TH1 *> fhStsChanCntRaw;
   std::vector<TH1 *> fhStsChanCntRawGood;
   std::vector<TH2 *> fhStsChanAdcRaw;
   std::vector<TProfile*> fhStsChanAdcRawProf;
   std::vector<TH2*> fhStsChanRawTs;
   std::vector<TH2*> fhStsChanMissEvt;
   std::vector<TH2*> fhStsChanMissEvtEvo;
   std::vector<TH2*> fhStsChanHitRateEvo;
   std::vector<TH1*> fhStsAsicRateEvo;
   std::vector<TH1*> fhStsAsicMissEvtEvo;
   std::vector<TH2*> fhStsChanHitRateEvoLong;
   std::vector<TH1*> fhStsAsicRateEvoLong;

      /// Plots per FEB-8
   UInt_t   kuNbAsicPerFeb = 8;
   Double_t kdFebChanCoincidenceLimit = 300.0; // ns
   std::vector<TH1 *>     fhStsFebChanCntRaw;
   std::vector<TH2 *>     fhStsFebChanAdcRaw;
   std::vector<TProfile*> fhStsFebChanAdcRawProf;
   std::vector<TH2*>      fhStsFebChanHitRateEvo;
   std::vector<TH1*>      fhStsFebHitRateEvo;
   std::vector< std::vector<TH2*> > fhStsFebChanCoinc;

   TCanvas*  fcMsSizeAll;
   TH1*      fhMsSz[kiMaxNbFlibLinks];
   TProfile* fhMsSzTime[kiMaxNbFlibLinks];
/*
   /// Pulser time difference histos
   Double_t                          fdStartTs; //! Last hits in each ASIC
   std::vector< stsxyter::FinalHit > fvmLastHitAsic; //! Last hits in each ASIC
   std::vector<TH2 *>                fhStsTimeDiffPerAsic;
   std::vector< std::vector<TH1 *> > fhStsTimeDiffPerAsicPair;
   std::vector< std::vector<TH1 *> > fhStsTimeDiffClkPerAsicPair;
   std::vector< std::vector<TH2 *> > fhStsTimeDiffEvoPerAsicPair;
   std::vector< std::vector<TProfile *> > fhStsTimeDiffEvoPerAsicPairProf;
   std::vector< std::vector<TProfile *> > fhStsRawTimeDiffEvoPerAsicPairProf;
   std::vector< std::vector<TH2 *> > fhStsTsLsbMatchPerAsicPair;
   std::vector< std::vector<TH2 *> > fhStsTsMsbMatchPerAsicPair;
   std::vector<TH1 *>                fhStsIntervalAsic;
   std::vector<TH1 *>                fhStsIntervalLongAsic;

   static const UInt_t kuNbValuesForTimeDiffMean = 100;
   constexpr static const double kdPulserPeriod = 154000; /// ns
   constexpr static const double kdTimeDiffToMeanMargin = 50;
   std::vector< std::vector< std::vector< Double_t > > > fvdLastTimeDiffValuesAsicPair;
   std::vector< std::vector< UInt_t > >  fvuLastTimeDiffSlotAsicPair;
   std::vector< std::vector< Double_t > > fvdMeanTimeDiffAsicPair;

   void UpdatePairMeanValue( UInt_t uAsicA, UInt_t uAsicB, Double_t dNewValue );
*/
   void CreateHistograms();

   void FillHitInfo(   stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx );
   void FillTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx = 0, UInt_t uMsIdx = 0);
   void FillEpochInfo( stsxyter::Message mess );

   CbmMcbm2018MonitorSts(const CbmMcbm2018MonitorSts&);
   CbmMcbm2018MonitorSts operator=(const CbmMcbm2018MonitorSts&);

   ClassDef(CbmMcbm2018MonitorSts, 1)
};

#endif // CBMMCBM2018MONITORSTS_H
