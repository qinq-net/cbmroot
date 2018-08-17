// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                   CbmMcbm2018MonitorMcbmSync                      -----
// -----                Created 11/05/18  by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMMCBM2018MONITORMCBMSYNC_H
#define CBMMCBM2018MONITORMCBMSYNC_H

#ifndef __CINT__
    #include "Timeslice.hpp"
#endif

// Data
#include "StsXyterMessage.h"
#include "StsXyterFinalHit.h"
#include "gDpbMessv100.h"

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

class CbmTofStar2018Par;

class CbmMcbm2018MonitorMcbmSync: public CbmMcbmUnpack
{
public:

   CbmMcbm2018MonitorMcbmSync();
   virtual ~CbmMcbm2018MonitorMcbmSync();

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
   void AddMsComponentToListSts( size_t component );
   void AddMsComponentToListTof( size_t component );

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
                        { fbPrintMessages = bPrintMessOn;
                          fPrintMessCtrlSts = ctrl;
                        }
   void SetLongDurationLimits( UInt_t uDurationSeconds = 3600, UInt_t uBinSize = 1 );
   void SetStsCoincidenceBorder( Double_t dCenterPos, Double_t dBorderVal );

   inline void SetHistoryHistoSize( UInt_t inHistorySizeSec = 1800 ) { fuTofHistoryHistoSize = inHistorySizeSec; }

   inline void SetTofFitZoomWidthPs( Double_t inZoomWidth = 1000.0 ) { fdTofFitZoomWidthPs = inZoomWidth; }
   void UpdateZoomedFitTof();

private:
   /// Parameters
      /// FLES containers
   std::vector< size_t >    fvMsComponentsListSts; //!
   std::vector< size_t >    fvMsComponentsListTof; //!
   size_t                   fuNbCoreMsPerTs; //!
   size_t                   fuNbOverMsPerTs; //!
   Bool_t                   fbIgnoreOverlapMs; //! /** Ignore Overlap Ms: all fuOverlapMsNb MS at the end of timeslice **/
      /// Unpacking and mapping parameters for STS
   CbmCern2017UnpackParHodo* fUnpackParHodo; //!
   UInt_t                    fuStsNrOfDpbs;       //! Total number of Sts DPBs in system
   std::map<UInt_t, UInt_t>  fmStsDpbIdIndexMap;  //! Map of DPB Identifier to DPB index
   UInt_t                    fuStsNbElinksPerDpb; //! Number of possible eLinks per DPB
   UInt_t                    fuStsNbStsXyters;    //! Number of StsXyter ASICs
   UInt_t                    fuStsNbChanPerAsic;  //! Number of channels per StsXyter ASIC => should be constant somewhere!!!!
   std::vector< std::vector< UInt_t > > fvuStsElinkToAsic;   //! Vector holding for each link the corresponding ASIC index [fuNrOfDpbs * fuStsNbElinksPerDpb]
      /// Unpacking and mapping parameters for TOF
   CbmTofStar2018Par* fUnpackParTof;         //!
   UInt_t             fuTofNrOfGdpbs;           //! Total number of GDPBs in the system
   UInt_t             fuTofNrOfFeePerGdpb;      //! Number of FEBs per GDPB
   UInt_t             fuTofNrOfGet4PerFee;      //! Number of GET4s per FEE <= Constant, should be treated as such, maybe in param class?
   UInt_t             fuTofNrOfChannelsPerGet4; //! Number of channels in each GET4 <= Constant, should be treated as such, maybe in param class?
   UInt_t             fuTofNrOfChannelsPerFee;  //! Number of channels in each FEE <= Constant, should be treated as such, maybe in param class?
   UInt_t             fuTofNrOfGet4;            //! Total number of Get4 chips in the system
   UInt_t             fuTofNrOfGet4PerGdpb;     //! Number of GET4s per GDPB
   UInt_t             fuTofNrOfChannelsPerGdpb; //! Number of channels per GDPB

   // Constants
   static const UInt_t   kuStsBytesPerMessage = 4; //! TODO => move to the message class!!
   static const UInt_t   kuTofBytesPerMessage = 8;

   // Internal Control/status of monitor
      // Histo File name and path
   TString fsHistoFileFullname;
      // Task configuration values
   Bool_t                fbPrintMessages;
   stsxyter::MessagePrintMask fPrintMessCtrlSts;
      // TS/MS info
   ULong64_t             fulCurrentTsIdx;
   ULong64_t             fulCurrentMsIdx;
      // Current data properties
   std::map< stsxyter::MessType, UInt_t > fmMsgCounter;
   UInt_t                fuCurrentEquipmentId;  //! Current equipment ID, tells from which DPB the current MS is originating
   UInt_t                fuCurrDpbId;           //! Temp holder until Current equipment ID is properly filled in MS
   UInt_t                fuCurrDpbIdx;          //! Index of the DPB from which the MS currently unpacked is coming
   Int_t                 fiRunStartDateTimeSec; //! Start of run time since "epoch" in s, for the plots with date as X axis
   Int_t                 fiBinSizeDatePlots;    //! Bin size in s for the plots with date as X axis

/****************** STS Sync ******************************************/
      // Data format control
   std::vector< ULong64_t > fvulStsCurrentTsMsb;                   //! Current TS MSB for each DPB
   std::vector< UInt_t    > fvuStsCurrentTsMsbCycle;               //! Current TS MSB cycle for DPB

      // STS Hits comparison
   std::vector< std::vector< ULong64_t > > fvulStsChanLastHitTime;   //! Last hit time in bins for each Channel
   std::vector< std::vector< Double_t  > > fvdStsChanLastHitTime;    //! Last hit time in ns   for each Channel

      // Starting state book-keeping
   Double_t              fdStsStartTime;           /** Time of first valid hit (TS_MSB available), used as reference for evolution plots**/
   Double_t              fdStsStartTimeMsSz;       /** Time of first microslice, used as reference for evolution plots**/
   std::chrono::steady_clock::time_point ftStartTimeUnix; /** Time of run Start from UNIX system, used as reference for long evolution plots against reception time **/

      // Hits time-sorting
   std::vector< stsxyter::FinalHit > fvmStsHitsInMs; //! All hits (time in bins, ADC in bins, asic, channel) in last TS, sorted with "<" operator
   std::vector< std::vector< stsxyter::FinalHit > > fvmStsAsicHitsInMs; //! All hits (time in bins, ADC in bins, asic, channel) in last TS, per ASIC, sorted with "<" operator

      // Rate evolution histos
   Bool_t fbLongHistoEnable;
   UInt_t fuLongHistoNbSeconds;
   UInt_t fuLongHistoBinSizeSec;
   UInt_t fuLongHistoBinNb;

      // Coincidences in sorted hits
   Double_t fdStsCoincCenter;  // ns
   Double_t fdStsCoincBorder;  // ns, +/-
   Double_t fdStsCoincMin;     // ns
   Double_t fdStsCoincMax;     // ns

   // Histograms
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
   std::vector<TH1*> fhStsFebRateEvo;
   std::vector<TH1*> fhStsFebMissEvtEvo;
   std::vector<TH2*> fhStsChanHitRateEvoLong;
   std::vector<TH1*> fhStsFebRateEvoLong;

   /// Pulser time difference histos
   Double_t                          fdStsStartTs; //! Last hits in each ASIC
   std::vector< stsxyter::FinalHit > fvmStsLastHitAsic; //! Last hits in each ASIC
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

   /// Processing methods
   void CreateStsHistograms();

   Bool_t ProcessStsMs( const fles::Timeslice& ts, size_t uMsComp, UInt_t uMsIdx );

   void FillStsHitInfo(   stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx );
   void FillStsTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx = 0, UInt_t uMsIdx = 0);
   void FillStsEpochInfo( stsxyter::Message mess );
/****************** STS Sync ******************************************/

/****************** TOF Sync ******************************************/

   /// Running indices
   uint64_t fulTofCurrentTsIndex;  // Idx of the current TS
   size_t   fuTofCurrentMs; // Idx of the current MS in TS (0 to fuTotalMsNb)
   Double_t fdTofMsIndex;   // Time in ns of current MS from its index
   UInt_t   fuTofGdpbId;    // Id (hex number) of the GDPB for current message
   UInt_t   fuTofGdpbNr;    // running number (0 to fuNrOfGdpbs) of the GDPB for current message
   UInt_t   fuTofGet4Id;    // running number (0 to fuNrOfGet4PerGdpb) of the Get4 chip of a unique GDPB for current message
   UInt_t   fuTofGet4Nr;    // running number (0 to fuNrOfGet4) of the Get4 chip in the system for current message
   Int_t    fiTofEquipmentId;
   std::vector<int> fviTofMsgCounter;

   /// STAR TRIGGER detection
   std::vector< ULong64_t > fvulTofGdpbTsMsb;
   std::vector< ULong64_t > fvulTofGdpbTsLsb;
   std::vector< ULong64_t > fvulTofStarTsMsb;
   std::vector< ULong64_t > fvulTofStarTsMid;
   std::vector< ULong64_t > fvulTofGdpbTsFullLast;
   std::vector< ULong64_t > fvulTofStarTsFullLast;
   std::vector< UInt_t    > fvuTofStarTokenLast;
   std::vector< UInt_t    > fvuTofStarDaqCmdLast;
   std::vector< UInt_t    > fvuTofStarTrigCmdLast;

   /** Current epoch marker for each GDPB and GET4
     * (first epoch in the stream initializes the map item)
     * pointer points to an array of size fuNrOfGdpbs * fuNrOfGet4PerGdpb
     * The correct array index is calculated using the function
     * GetArrayIndex(gdpbId, get4Id)
     **/
   std::vector< ULong64_t > fvulTofCurrentEpoch; //!
   std::vector< Bool_t >    fvbTofFirstEpochSeen; //!

   ULong64_t fulTofCurrentEpochTime;     /** Time stamp of current epoch **/

   /// Map of ID to index for the gDPBs
   std::map<UInt_t, UInt_t> fmTofGdpbIdIndexMap;

   /// Buffer for suppressed epoch processing
   std::vector< std::vector < gdpbv100::Message > > fvmTofEpSupprBuffer;

   /// Buffer for pulser channels
   std::vector<  Double_t  > fvdTofTsLastPulserHit;

   /// Histograms and histogram control variables
      /// Default value for nb bins in Pulser time difference histos
   const UInt_t kuTofNbBinsDt    = 300;
   Double_t fdTofMinDt;
   Double_t fdTofMaxDt;
      /// Default number of FEET per channels histograms
   UInt_t fuTofNbFeePlot;
   UInt_t fuTofNbFeePlotsPerGdpb;
      /// Evolution plots control
   Double_t fdTofTsStartTime;
   Double_t fdTofStartTime; /** Time of first valid hit (epoch available), used as reference for evolution plots**/
   Double_t fdTofStartTimeLong; /** Time of first valid hit (epoch available), used as reference for evolution plots**/
   UInt_t   fuTofHistoryHistoSize; /** Size in seconds of the evolution histograms **/
   UInt_t   fuTofHistoryHistoSizeLong; /** Size in seconds of the evolution histograms **/
      /// Pulser plots
   Double_t fdTofLastRmsUpdateTime;
   Double_t fdTofFitZoomWidthPs;
      /// Messages types and flags
   TH1* fhTofMessType;
   TH1* fhTofSysMessType;
   TH2* fhTofGet4MessType;
   TH2* fhTofGet4ChanScm;
   TH2* fhTofGet4ChanErrors;
   TH2* fhTofGet4EpochFlags;
      /// Slow control messages
   TH2* fhTofScmScalerCounters;
   TH2* fhTofScmDeadtimeCounters;
   TH2* fhTofScmSeuCounters;
   TH2* fhTofScmSeuCountersEvo;
      /// Hit messages
   std::vector< TH2      * > fvhTofRawFt_gDPB;
   std::vector< TH2      * > fvhTofRawTot_gDPB;
   std::vector< TH1      * > fvhTofChCount_gDPB;
   std::vector< TH2      * > fvhTofChannelRate_gDPB;
   std::vector< TH2      * > fvhTofRemapTot_gDPB;
   std::vector< TH1      * > fvhTofRemapChCount_gDPB;
   std::vector< TH2      * > fvhTofRemapChRate_gDPB;
   std::vector< TH1      * > fvhTofFeeRate_gDPB;
   std::vector< TH1      * > fvhTofFeeErrorRate_gDPB;
   std::vector< TProfile * > fvhTofFeeErrorRatio_gDPB;
   std::vector< TH1      * > fvhTofFeeRateLong_gDPB;
   std::vector< TH1      * > fvhTofFeeErrorRateLong_gDPB;
   std::vector< TProfile * > fvhTofFeeErrorRatioLong_gDPB;

      /// STAR TRIGGER detection
   std::vector< TH1 *      > fvhTofTokenMsgType;
   std::vector< TH1 *      > fvhTofTriggerRate;
   std::vector< TH2 *      > fvhTofCmdDaqVsTrig;
   std::vector< TH2 *      > fvhTofStarTokenEvo;
   std::vector< TProfile * > fvhTofStarTrigGdpbTsEvo;
   std::vector< TProfile * > fvhTofStarTrigStarTsEvo;

      /// Pulser monitoring
   const Double_t kdTofMaxDtPulserPs = 100e3;
   std::vector< std::vector< TH1 * > > fvhTofTimeDiffPulser;
   TH2 * fhTofTimeRmsPulser;
   TH2 * fhTofTimeRmsZoomFitPuls;
   TH2 * fhTofTimeResFitPuls;


   /// Processing methods
   void CreateTofHistograms();

   Bool_t ProcessTofMs( const fles::Timeslice& ts, size_t uMsComp, UInt_t uMsIdx );

   void FillTofHitInfo(      gdpbv100::Message );
   void FillTofEpochInfo(    gdpbv100::Message );
   void FillTofSlcInfo(      gdpbv100::Message );
   void FillTofSysInfo(      gdpbv100::Message );
   void PrintTofGenInfo(     gdpbv100::Message );
   void FillTofStarTrigInfo( gdpbv100::Message );

   inline Int_t GetArrayIndexGet4(Int_t gdpbId, Int_t get4Id)
   {
      return gdpbId * fuTofNrOfGet4PerGdpb + get4Id;
   }

   /// PADI channel to GET4 channel mapping and reverse
   std::vector< UInt_t > fvuPadiToGet4;
   std::vector< UInt_t > fvuGet4ToPadi;

   /// GET4 to eLink mapping and reverse
   static const UInt_t kuNbGet4PerGbtx = 5 * 8; /// 5 FEE with 8 GET4 each
   std::vector< UInt_t > fvuElinkToGet4;
   std::vector< UInt_t > fvuGet4ToElink;
   inline UInt_t ConvertElinkToGet4( UInt_t uElinkIdx )
   {
      return fvuElinkToGet4[ uElinkIdx % kuNbGet4PerGbtx ] + kuNbGet4PerGbtx * (uElinkIdx / kuNbGet4PerGbtx);
   }
   inline UInt_t ConvertGet4ToElink( UInt_t uGet4Idx )
   {
      return fvuGet4ToElink[ uGet4Idx % kuNbGet4PerGbtx ] + kuNbGet4PerGbtx * (uGet4Idx / kuNbGet4PerGbtx);
   }
/****************** TOF Sync ******************************************/

   CbmMcbm2018MonitorMcbmSync(const CbmMcbm2018MonitorMcbmSync&);
   CbmMcbm2018MonitorMcbmSync operator=(const CbmMcbm2018MonitorMcbmSync&);

   ClassDef(CbmMcbm2018MonitorMcbmSync, 1)
};

#endif // CBMMCBM2018MONITORMCBMSYNC_H
