// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                   CbmMcbm2018MuchUnpacker                        -----
// -----                Created 11/05/18  by P.-A. Loizeau                 -----
// -----                Modified 07/12/18  by Ajit Kumar                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMMCBM2018MUCHUNPACKER_H
#define CBMMCBM2018MUCHUNPACKER_H

#include "Timeslice.hpp"

// Data
#include "StsXyterMessage.h"
#include "StsXyterFinalHit.h"

// CbmRoot
#include "CbmMcbmUnpack.h"
#include "CbmHistManager.h"
#include "CbmTbDaqBuffer.h"
#include "CbmMuchBeamTimeDigi.h" //Created a new Much Digi File for the SPS beamtime2016
#include "TClonesArray.h"
// C++11
#include <chrono>

// C/C++
#include <vector>
#include <set>
#include <map>

class CbmDigi;
class CbmMcbm2018MuchPar;
class CbmCern2017UnpackParSts;

class CbmMcbm2018MuchUnpacker: public CbmMcbmUnpack
{
public:

   CbmMcbm2018MuchUnpacker();
   virtual ~CbmMcbm2018MuchUnpacker();

   virtual Bool_t Init();

   virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);

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
   void CreateRawMessageOutput(Bool_t val) {fCreateRawMessage=val;}
   void SetMuchMode( Bool_t bMuchMode = kTRUE ) { fbMuchMode = bMuchMode; }

   void UseDaqBuffer(Bool_t) {};
private:
   // Parameters
      // Control flags
   Bool_t fbMuchMode;
      // FLES containers
   std::vector< size_t >    fvMsComponentsList; //!
   size_t                   fuNbCoreMsPerTs; //!
   size_t                   fuNbOverMsPerTs; //!
   Bool_t                   fbIgnoreOverlapMs; //! /** Ignore Overlap Ms: all fuOverlapMsNb MS at the end of timeslice **/
      // Unpacking and mapping
   CbmMcbm2018MuchPar*      fUnpackParMuch;  //!
   UInt_t                   fuNrOfDpbs;        //! Total number of STS DPBs in system
   std::map<UInt_t, UInt_t> fDpbIdIndexMap;    //! Map of DPB Identifier to DPB index
   std::vector< std::vector< Bool_t > > fvbCrobActiveFlag; //! Array to hold the active flag for all CROBs, [ NbDpb ][ NbCrobPerDpb ]
   UInt_t                   fuNbFebs;          //! Number of StsXyter ASICs
   UInt_t                   fuNbStsXyters;     //! Number of StsXyter ASICs

   // Constants
   static const Int_t    kiMaxNbFlibLinks = 32;
   static const UInt_t   kuBytesPerMessage = 4;

      /// Task configuration values
   Bool_t                fbPrintMessages;
   stsxyter::MessagePrintMask fPrintMessCtrl;
      /// TS/MS info
   ULong64_t             fulCurrentTsIdx;
   ULong64_t             fulCurrentMsIdx;
      /// Current data properties
   std::map< stsxyter::MessType, UInt_t > fmMsgCounter;
   UInt_t                fuCurrentEquipmentId;  //! Current equipment ID, tells from which DPB the current MS is originating
   UInt_t                fuCurrDpbId; //! Temp holder until Current equipment ID is properly filled in MS
   UInt_t                fuCurrDpbIdx;          //! Index of the DPB from which the MS currently unpacked is coming
   Int_t                 fiRunStartDateTimeSec; //! Start of run time since "epoch" in s, for the plots with date as X axis
   Int_t                 fiBinSizeDatePlots;    //! Bin size in s for the plots with date as X axis

      /// Data format control
   std::vector< ULong64_t > fvulCurrentTsMsb;                   //! Current TS MSB for each DPB
   std::vector< UInt_t    > fvuCurrentTsMsbCycle;               //! Current TS MSB cycle for DPB
   std::vector< UInt_t    > fvuInitialHeaderDone;               //! Flag set after seeing MS header in 1st MS for DPB
   std::vector< UInt_t    > fvuInitialTsMsbCycleHeader;         //! TS MSB cycle from MS header in 1st MS for DPB
   std::vector< UInt_t    > fvuElinkLastTsHit;                  //! TS from last hit for DPB
      /// Hits comparison
   std::vector< std::vector< ULong64_t > > fvulChanLastHitTime;   //! Last hit time in bins for each Channel
   std::vector< std::vector< Double_t  > > fvdChanLastHitTime;    //! Last hit time in ns   for each Channel
   std::vector< Double_t >                               fvdPrevMsTime;              //! Header time of previous MS per link
   std::vector< Double_t >                               fvdMsTime;                  //! Header time of each MS
   std::vector< std::vector< std::vector< UInt_t > > >   fvuChanNbHitsInMs;          //! Number of hits in each MS for each Channel
   std::vector< std::vector< std::vector< Double_t > > > fvdChanLastHitTimeInMs;     //! Last hit time in bins in each MS for each Channel
   std::vector< std::vector< std::vector< UShort_t > > > fvusChanLastHitAdcInMs;     //! Last hit ADC in bins in each MS for each Channel
//   std::vector< std::vector< std::multiset< stsxyter::FinalHit > > > fvmChanHitsInTs; //! All hits (time & ADC) in bins in last TS for each Channel
      /// Starting state book-keeping
   Double_t              fdStartTime;           /** Time of first valid hit (TS_MSB available), used as reference for evolution plots**/
   Double_t              fdStartTimeMsSz;       /** Time of first microslice, used as reference for evolution plots**/
   std::chrono::steady_clock::time_point ftStartTimeUnix; /** Time of run Start from UNIX system, used as reference for long evolution plots against reception time **/

      /// Hits time-sorting
   std::vector< stsxyter::FinalHit > fvmHitsInMs; //! All hits (time in bins, ADC in bins, asic, channel) in last TS, sorted with "<" operator
   std::vector< std::vector< stsxyter::FinalHit > > fvmAsicHitsInMs; //! All hits (time in bins, ADC in bins, asic, channel) in last TS, per ASIC, sorted with "<" operator
   std::vector< std::vector< stsxyter::FinalHit > > fvmFebHitsInMs; //! All hits (time in bins, ADC in bins, asic, channel) in last TS, per FEB, sorted with "<" operator
      /// Coincidence histos
   UInt_t fuMaxNbMicroslices;
      /// Mean Rate per channel plots
   Int_t                                  fiTimeIntervalRateUpdate;
   std::vector< Int_t >                   fviFebTimeSecLastRateUpdate;
   std::vector< Int_t >                   fviFebCountsSinceLastRateUpdate;
   std::vector< std::vector< Double_t > > fvdFebChanCountsSinceLastRateUpdate;

   /// Histograms
   CbmHistManager* fHM;                 //! Histogram manager

   TCanvas*  fcMsSizeAll;
   TH1*      fhMsSz[kiMaxNbFlibLinks];
   TProfile* fhMsSzTime[kiMaxNbFlibLinks];
   TClonesArray* fMuchRaw;
   TClonesArray* fMuchDigi;

   CbmMuchBeamTimeDigi* fDigi;  
   CbmTbDaqBuffer* fBuffer;

   Bool_t fCreateRawMessage;
   // Methods later going into Algo
   Bool_t InitMuchParameters();
   void CreateHistograms();
   Bool_t ProcessStsMs( const fles::Timeslice& ts, size_t uMsComp, UInt_t uMsIdx );

   //void FillHitInfo(   stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx );
   void FillHitInfo(   stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx, const UInt_t & uDpbId );
   void FillTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx = 0, UInt_t uMsIdx = 0);
   void FillEpochInfo( stsxyter::Message mess );

   CbmMcbm2018MuchUnpacker(const CbmMcbm2018MuchUnpacker&);
   CbmMcbm2018MuchUnpacker operator=(const CbmMcbm2018MuchUnpacker&);

   ClassDef(CbmMcbm2018MuchUnpacker, 1)
};

#endif // CBMMCBM2018MUCHUNPACKER_H
