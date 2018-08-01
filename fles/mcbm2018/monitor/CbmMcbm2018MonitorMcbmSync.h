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

private:
   // Parameters
      // FLES containers
   std::vector< size_t >    fvMsComponentsListSts; //!
   std::vector< size_t >    fvMsComponentsListTof; //!
   size_t                   fuNbCoreMsPerTs; //!
   size_t                   fuNbOverMsPerTs; //!
   Bool_t                   fbIgnoreOverlapMs; //! /** Ignore Overlap Ms: all fuOverlapMsNb MS at the end of timeslice **/
      // Unpacking and mapping
   CbmCern2017UnpackParHodo* fUnpackParHodo; //!
   UInt_t                   fuStsNrOfDpbs;       //! Total number of Sts DPBs in system
   std::map<UInt_t, UInt_t> fmStsDpbIdIndexMap;  //! Map of DPB Identifier to DPB index
   UInt_t                   fuStsNbElinksPerDpb; //! Number of possible eLinks per DPB
   UInt_t                   fuStsNbStsXyters;    //! Number of StsXyter ASICs
   UInt_t                   fuStsNbChanPerAsic;  //! Number of channels per StsXyter ASIC => should be constant somewhere!!!!
   std::vector< std::vector< UInt_t > > fvuStsElinkToAsic;   //! Vector holding for each link the corresponding ASIC index [fuNrOfDpbs * fuStsNbElinksPerDpb]

   // Constants
   static const UInt_t   kuStsBytesPerMessage = 4;

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
   Double_t              fdStartTime;           /** Time of first valid hit (TS_MSB available), used as reference for evolution plots**/
   Double_t              fdStartTimeMsSz;       /** Time of first microslice, used as reference for evolution plots**/
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

   void CreateStsHistograms();

   Bool_t ProcessStsMs( const fles::Timeslice& ts, size_t uMsComp, UInt_t uMsIdx );

   void FillStsHitInfo(   stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx );
   void FillStsTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx = 0, UInt_t uMsIdx = 0);
   void FillStsEpochInfo( stsxyter::Message mess );
/****************** STS Sync ******************************************/

/****************** TOF Sync ******************************************/
   ///* PADI channel to GET4 channel mapping and reverse *///
/*
   inline Int_t GetArrayIndex(Int_t gdpbId, Int_t get4Id)
   {
      return gdpbId * fuNrOfGet4PerGdpb + get4Id;
   }

   std::vector< UInt_t > fvuPadiToGet4;
   std::vector< UInt_t > fvuGet4ToPadi;

   void CreateTofHistograms();

   Bool_t ProcessTofMs( const fles::Timeslice& ts, size_t uMsComp, UInt_t uMsIdx );

   void FillHitInfo(       gdpbv100::Message );
   void FillEpochInfo(     gdpbv100::Message );
   void PrintSlcInfo(      gdpbv100::Message );
   void PrintSysInfo(      gdpbv100::Message );
   void PrintGenInfo(      gdpbv100::Message );
   void FillStarTrigInfo(  gdpbv100::Message );
*/
/****************** TOF Sync ******************************************/

   CbmMcbm2018MonitorMcbmSync(const CbmMcbm2018MonitorMcbmSync&);
   CbmMcbm2018MonitorMcbmSync operator=(const CbmMcbm2018MonitorMcbmSync&);

   ClassDef(CbmMcbm2018MonitorMcbmSync, 1)
};

#endif // CBMMCBM2018MONITORMCBMSYNC_H
