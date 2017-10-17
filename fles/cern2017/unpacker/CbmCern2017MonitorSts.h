// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmCern2017MonitorSts                         -----
// -----                Created 25/07/17  by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMCERN2017MONITORSTS_H
#define CBMCERN2017MONITORSTS_H

#ifndef __CINT__
    #include "Timeslice.hpp"
#endif

// Data
#include "StsXyterMessage.h"

// CbmRoot
#include "CbmTSUnpack.h"
#include "CbmHistManager.h"

// C++11

// C/C++
#include <vector>
#include <map>

class CbmDigi;
class CbmCern2017UnpackParSts;

class CbmCern2017MonitorSts: public CbmTSUnpack
{
public:

   CbmCern2017MonitorSts();
   virtual ~CbmCern2017MonitorSts();

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
                                                           stsxyter::MessagePrintMask::msg_print_Crc |
                                                           stsxyter::MessagePrintMask::msg_print_Human )
                        { fbPrintMessages = bPrintMessOn; fPrintMessCtrl = ctrl; }
   void EnableChanHitDtPlot( Bool_t bEnable = kTRUE ) { fbChanHitDtEna = bEnable; }

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
   static const UInt_t   kuBytesPerMessage = 8;

   // Internal Control/status of monitor
      // Task configuration values
   Bool_t                fbPrintMessages;
   stsxyter::MessagePrintMask fPrintMessCtrl;
   Bool_t                fbChanHitDtEna;
      // Current data properties
   std::map< stsxyter::MessType, UInt_t > fmMsgCounter;
   UInt_t                fuCurrentEquipmentId;  //! Current equipment ID, tells from which DPB the current MS is originating
   UInt_t                fuCurrDpbId; //! Temp holder until Current equipment ID is properly filled in MS
   UInt_t                fuCurrDpbIdx;          //! Index of the DPB from which the MS currently unpacked is coming
   Int_t                 fiRunStartDateTimeSec; //! Start of run time since "epoch" in s, for the plots with date as X axis
   Int_t                 fiBinSizeDatePlots;    //! Bin size in s for the plots with date as X axis
   std::vector< std::vector< UInt_t > > fvuCurrentTsMsb;       //! Current TS MSB for each eLink
   std::vector< std::vector< UInt_t > > fvuCurrentTsMsbCycle;  //! Current TS MSB cycle for each eLink
   std::vector< std::vector< UInt_t > > fvuCurrentTsMsbOver;   //! Current TS MSB overlap bits for each eLink
   std::vector< std::vector< ULong64_t > > fvulChanLastHitTime; //! Last hit time in bins for each Channel
   std::vector< std::vector<Double_t> > fvdChanLastHitTime;    //! Last hit time in ns   for each Channel
      // Starting state book-keeping
   Double_t              fdStartTime;           /** Time of first valid hit (TS_MSB available), used as reference for evolution plots**/
   Double_t              fdStartTimeMsSz;       /** Time of first microslice, used as reference for evolution plots**/

   // Historgrams
   CbmHistManager* fHM;                 //! Histogram manager
   TH1* fhStsMessType;
   TH1* fhStsSysMessType;
   TH2* fhStsMessTypePerDpb;
   TH2* fhStsSysMessTypePerDpb;

   std::vector<TH1*> fhStsChanCounts;
   std::vector<TH2*> fhStsChanRawAdc;
   std::vector<TProfile*> fhStsChanRawAdcProf;
   std::vector<TH2*> fhStsChanRawTs;
   std::vector<TH2*> fhStsChanMissEvt;
   std::vector<TH2*> fhStsChanOverDiff;
   std::vector<TH2*> fhStsChanHitRateEvo;
   std::vector<TH1*> fhStsXyterRateEvo;
   std::vector<TH2*> fhStsChanHitRateEvoLong;
   std::vector<TH1*> fhStsXyterRateEvoLong;
   std::vector<TH2*> fhStsChanHitDt;
   std::vector<TH2*> fhStsChanHitDtNeg;

   TH2* fhStsAsicTsMsb;
   TH2* fhStsAsicTsMsbMaj;
   std::vector<TH2*> fhStsElinkTsMsbCrc;
   std::vector<TH2*> fhStsElinkTsMsbMaj;

/*
   std::vector<TH1*> fhFebRateDate_nDPB;
*/

   TCanvas*  fcMsSizeAll;
   TH1*      fhMsSz[kiMaxNbFlibLinks];
   TProfile* fhMsSzTime[kiMaxNbFlibLinks];

   void CreateHistograms();

   void FillHitInfo(   stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx );
   void FillTsMsbInfo( stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx );

   CbmCern2017MonitorSts(const CbmCern2017MonitorSts&);
   CbmCern2017MonitorSts operator=(const CbmCern2017MonitorSts&);

   ClassDef(CbmCern2017MonitorSts, 1)
};

#endif // CBMCERN2017MONITORSTS_H
