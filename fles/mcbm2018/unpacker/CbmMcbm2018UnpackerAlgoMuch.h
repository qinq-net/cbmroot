// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                  CbmMcbm2018UnpackerAlgoMuch                      -----
// -----               Created 02.02.2019 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmMcbm2018UnpackerAlgoMuch_H
#define CbmMcbm2018UnpackerAlgoMuch_H

#include "CbmStar2019Algo.h"

// Data
#include "StsXyterMessage.h"
#include "StsXyterFinalHit.h"
#include "CbmMuchBeamTimeDigi.h"

// CbmRoot

// C++11
#include <chrono>

// C/C++
#include <vector>
#include <map>

class CbmMcbm2018MuchPar;
/*
class TCanvas;
class THttpServer;
*/
class TH1;
class TH2;
class TProfile;

class CbmMcbm2018UnpackerAlgoMuch : public CbmStar2019Algo<CbmMuchBeamTimeDigi>
{
   public:
      CbmMcbm2018UnpackerAlgoMuch();
      ~CbmMcbm2018UnpackerAlgoMuch();

      virtual Bool_t Init();
      virtual void Reset();
      virtual void Finish();

      Bool_t InitContainers();
      Bool_t ReInitContainers();
      TList* GetParList();

      Bool_t InitParameters();

      Bool_t ProcessTs( const fles::Timeslice& ts );
      Bool_t ProcessTs( const fles::Timeslice& ts, size_t component ) { return ProcessTs( ts ); }
      Bool_t ProcessMs( const fles::Timeslice& ts, size_t uMsCompIdx, size_t uMsIdx );

      void AddMsComponentToList( size_t component, UShort_t usDetectorId );

      Bool_t CreateHistograms();
      Bool_t FillHistograms();
      Bool_t ResetHistograms();

      inline void SetMonitorMode( Bool_t bFlagIn = kTRUE ) { fbMonitorMode = bFlagIn; }
      inline void SetTimeOffsetNs( Double_t dOffsetIn = 0.0 ) { fdTimeOffsetNs = dOffsetIn; }

   private:
      /// Control flags
      Bool_t fbMonitorMode;      //! Switch ON the filling of a minimal set of histograms
      Bool_t fbDebugMonitorMode; //! Switch ON the filling of a additional set of histograms
      std::vector< Bool_t >    fvbMaskedComponents;

      /// Settings from parameter file
      CbmMcbm2018MuchPar* fUnpackPar;      //!
         /// Readout chain dimensions and mapping
      UInt_t                   fuNrOfDpbs;        //! Total number of STS DPBs in system
      std::map<UInt_t, UInt_t> fDpbIdIndexMap;    //! Map of DPB Identifier to DPB index
      std::vector< std::vector< Bool_t > > fvbCrobActiveFlag; //! Array to hold the active flag for all CROBs, [ NbDpb ][ NbCrobPerDpb ]
      UInt_t                   fuNbFebs;          //! Number of FEBs with StsXyter ASICs
      UInt_t                   fuNbStsXyters;     //! Number of StsXyter ASICs

      /// User settings: Data correction parameters
      Double_t fdTimeOffsetNs;

      /// Constants
      static const Int_t    kiMaxNbFlibLinks = 32;
      static const UInt_t   kuBytesPerMessage = 4;

      /// Running indices
         /// TS/MS info
      ULong64_t             fulCurrentTsIdx;
      ULong64_t             fulCurrentMsIdx;
      Double_t              fdTsStartTime;         //! Time in ns of current TS from the index of the first MS first component
      Double_t              fdTsStopTimeCore;      //! End Time in ns of current TS Core from the index of the first MS first component
      Double_t              fdMsTime;              //! Start Time in ns of current MS from its index field in header
      UInt_t                fuMsIndex;             //! Index of current MS within the TS
         /// Current data properties
      std::map< stsxyter::MessType, UInt_t > fmMsgCounter;
      UInt_t                fuCurrentEquipmentId;  //! Current equipment ID, tells from which DPB the current MS is originating
      UInt_t                fuCurrDpbId;           //! Temp holder until Current equipment ID is properly filled in MS
      UInt_t                fuCurrDpbIdx;          //! Index of the DPB from which the MS currently unpacked is coming
      Int_t                 fiRunStartDateTimeSec; //! Start of run time since "epoch" in s, for the plots with date as X axis
      Int_t                 fiBinSizeDatePlots;    //! Bin size in s for the plots with date as X axis
         /// Data format control
      std::vector< ULong64_t > fvulCurrentTsMsb;                   //! Current TS MSB for each DPB
      std::vector< UInt_t    > fvuCurrentTsMsbCycle;               //! Current TS MSB cycle for DPB
         /// Starting state book-keeping
      Double_t              fdStartTime;           /** Time of first valid hit (TS_MSB available), used as reference for evolution plots**/
      Double_t              fdStartTimeMsSz;       /** Time of first microslice, used as reference for evolution plots**/
      std::chrono::steady_clock::time_point ftStartTimeUnix; /** Time of run Start from UNIX system, used as reference for long evolution plots against reception time **/

      /// Hits time-sorting
      std::vector< stsxyter::FinalHit > fvmHitsInMs; //! All hits (time in bins, ADC in bins, asic, channel) in last MS, sorted with "<" operator

      /// Histograms
      TH1 *               fhDigisTimeInRun;                //!
/*
      std::vector< TH1* > fvhHitsTimeToTriggerRaw;       //! [sector]
      std::vector< TH1* > fvhMessDistributionInMs;       //! [sector], extra monitor for debug
      TH1 *               fhEventNbPerTs;                //!
      TCanvas *           fcTimeToTrigRaw;               //! All sectors
*/
      void ProcessHitInfo(   stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx );
      void ProcessTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx = 0, UInt_t uMsIdx = 0);
      void ProcessEpochInfo( stsxyter::Message mess );
      void ProcessStatusInfo( stsxyter::Message mess );

      CbmMcbm2018UnpackerAlgoMuch(const CbmMcbm2018UnpackerAlgoMuch&);
      CbmMcbm2018UnpackerAlgoMuch operator=(const CbmMcbm2018UnpackerAlgoMuch&);

      ClassDef(CbmMcbm2018UnpackerAlgoMuch, 1)
};

#endif
