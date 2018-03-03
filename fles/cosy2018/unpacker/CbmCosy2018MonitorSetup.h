// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmCosy2018MonitorSetup                        -----
// -----                Created 27/02/18  by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMCOSY2018MONITORSETUP_H
#define CBMCOSY2018MONITORSETUP_H

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
class CbmCern2017UnpackParHodo;
class CbmCern2017UnpackParSts;

class CbmCosy2018MonitorSetup: public CbmTSUnpack
{
public:

   CbmCosy2018MonitorSetup();
   virtual ~CbmCosy2018MonitorSetup();

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
//   void EnableDualStsMode( Bool_t bEnable = kTRUE ) { fbDualStsEna = bEnable; }
   void SetLongDurationLimits( UInt_t uDurationSeconds = 3600, UInt_t uBinSize = 1 );
   void SetCoincidenceBorderHodo( Double_t dNewValue ){ fdCoincBorderHodo = dNewValue;}
   void SetCoincidenceBorderSts( Double_t dNewValue ){  fdCoincBorderSts  = dNewValue;}
   void SetCoincidenceBorder( Double_t dNewValue ){     fdCoincBorder     = dNewValue;}

private:
   size_t fuOverlapMsNb;      /** Ignore Overlap Ms: all fuOverlapMsNb MS at the end of timeslice **/

   // Parameters
   CbmCern2017UnpackParHodo* fUnpackParHodo; //!
   CbmCern2017UnpackParSts*  fUnpackParSts; //!
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
//   Bool_t                fbDualStsEna;
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
   std::vector< stsxyter::FinalHit > fvmHitsInTs; //! All hits (time in bins, ADC in bins, asic, channel) in last TS, sorted by multiset with "<" operator
   stsxyter::FinalHit fLastSortedHit1X; //! Last sorted hit for Hodo 1 X
   stsxyter::FinalHit fLastSortedHit1Y; //! Last sorted hit for Hodo 1 Y
   stsxyter::FinalHit fLastSortedHit2X; //! Last sorted hit for Hodo 2 X
   stsxyter::FinalHit fLastSortedHit2Y; //! Last sorted hit for Hodo 2 Y
   stsxyter::FinalHit fLastSortedHit1N; //! Last sorted hit for STS 1 N
   stsxyter::FinalHit fLastSortedHit1P; //! Last sorted hit for STS 1 P
   stsxyter::FinalHit fLastSortedHit2N; //! Last sorted hit for STS 2 N
   stsxyter::FinalHit fLastSortedHit2P; //! Last sorted hit for STS 2 P
      // Coincidence histos
   UInt_t fuMaxNbMicroslices;
      // Rate evolution histos
   Bool_t fbLongHistoEnable;
   UInt_t fuLongHistoNbSeconds;
   UInt_t fuLongHistoBinSizeSec;
   UInt_t fuLongHistoBinNb;

   // Histograms
   CbmHistManager* fHM;                 //! Histogram manager
   TH1* fhHodoMessType;
   TH1* fhHodoSysMessType;
   TH2* fhHodoMessTypePerDpb;
   TH2* fhHodoSysMessTypePerDpb;
/*
   TH2* fhStsDpbRawTsMsb;
   TH2* fhStsDpbRawTsMsbSx;
   TH2* fhStsDpbRawTsMsbDpb;
*/
   TH2* fhHodoMessTypePerElink;
   TH2* fhHodoSysMessTypePerElink;

   std::vector<TH1 *> fhHodoChanCntRaw;
   std::vector<TH2 *> fhHodoChanAdcRaw;
   std::vector<TProfile*> fhHodoChanAdcRawProf;
   std::vector<TH2*> fhHodoChanRawTs;
   std::vector<TH2*> fhHodoChanMissEvt;
   std::vector<TH2*> fhHodoChanMissEvtEvo;
   std::vector<TH2*> fhHodoChanHitRateEvo;
   std::vector<TH1*> fhHodoFebRateEvo;
   std::vector<TH1*> fhHodoFebMissEvtEvo;
   std::vector<TH2*> fhHodoChanHitRateEvoLong;
   std::vector<TH1*> fhHodoFebRateEvoLong;

      // Coincidences in sorted hits
   Double_t fdCoincBorderHodo; // ns, +/-
   Double_t fdCoincBorderSts; // ns, +/-
   Double_t fdCoincBorder; // ns, +/-
         // Single detector maps
   TH1 * fhSetupSortedDtX1Y1;
   TH1 * fhSetupSortedDtX2Y2;
   TH1 * fhSetupSortedDtN1P1;
   TH1 * fhSetupSortedDtX1Y1X2Y2;
   TH1 * fhSetupSortedDtX1Y1X2Y2N1P1;
   TH2 * fhSetupSortedMapX1Y1;
   TH2 * fhSetupSortedMapX2Y2;
   TH2 * fhSetupSortedMapN1P1;
   TH1 * fhSetupSortedCntEvoX1Y1;
   TH1 * fhSetupSortedCntEvoX2Y2;
   TH1 * fhSetupSortedCntEvoN1P1;

         // Full Hodo maps
   TH1 * fhBothHodoSortedDtX1Y1;
   TH1 * fhBothHodoSortedDtX2Y2;
   TH1 * fhBothHodoSortedDtX1Y1X2Y2N1P1;
   TH2 * fhBothHodoSortedMapX1Y1;
   TH2 * fhBothHodoSortedMapX2Y2;
   TH1 * fhBothHodoSortedCntEvoX1Y1;
   TH1 * fhBothHodoSortedCntEvoX2Y2;

         // Full System maps
   TH1 * fhSystSortedDtX1Y1;
   TH1 * fhSystSortedDtX2Y2;
   TH1 * fhSystSortedDtN1P1;
   TH1 * fhSystSortedDtX1Y1X2Y2;
   TH2 * fhSystSortedMapX1Y1;
   TH2 * fhSystSortedMapX2Y2;
   TH2 * fhSystSortedMapN1P1;
   TH1 * fhSystSortedCntEvoX1Y1;
   TH1 * fhSystSortedCntEvoX2Y2;
   TH1 * fhSystSortedCntEvoN1P1;

   TH2 * fhSystSortedDtN1X1vsN1X2;
   TH2 * fhSystSortedDtP1X1vsP1X2;
   TH2 * fhSystSortedDtN1X1vsP1X1;
   TH2 * fhSystSortedDtSts1Hodo1vsSts1Hodo2;
   TH2 * fhSystSortedDtAllVsMapX1;
   TH2 * fhSystSortedDtAllVsMapY1;
   TH2 * fhSystSortedDtAllVsMapX2;
   TH2 * fhSystSortedDtAllVsMapY2;

   TCanvas*  fcMsSizeAll;
   TH1*      fhMsSz[kiMaxNbFlibLinks];
   TProfile* fhMsSzTime[kiMaxNbFlibLinks];

   void CreateHistograms();

   void FillHitInfo(   stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx );
   void FillTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx = 0, UInt_t uMsIdx = 0);
   void FillEpochInfo( stsxyter::Message mess );

   CbmCosy2018MonitorSetup(const CbmCosy2018MonitorSetup&);
   CbmCosy2018MonitorSetup operator=(const CbmCosy2018MonitorSetup&);

   ClassDef(CbmCosy2018MonitorSetup, 1)
};

#endif // CBMCOSY2018MONITORSETUP_H
