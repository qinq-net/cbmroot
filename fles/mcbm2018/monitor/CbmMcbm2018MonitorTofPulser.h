// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                    CbmMcbm2018MonitorTofPulser                          -----
// -----               Created 10.07.2018 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmMcbm2018MonitorTofPulser_H
#define CbmMcbm2018MonitorTofPulser_H

#include "Timeslice.hpp"
#include "gDpbMessv100.h"
//#include "CbmTofStarData.h"
//#include "CbmTofStarData2018.h"

#include "CbmMcbmUnpack.h"

#include "TClonesArray.h"
#include "Rtypes.h"

#include <vector>
#include <map>
#include <chrono>

class CbmDigi;
class CbmMcbm2018TofPar;

class TCanvas;
class TH1;
class TH2;
class TProfile;
class TProfile2D;

class CbmMcbm2018MonitorTofPulser: public CbmMcbmUnpack {
   public:

      CbmMcbm2018MonitorTofPulser();
      virtual ~CbmMcbm2018MonitorTofPulser();

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

      void SetMsLimitLevel(size_t uAcceptBoundaryPct = 100) { fuMsAcceptsPercent = uAcceptBoundaryPct; }
      size_t GetMsLimitLevel() { return fuMsAcceptsPercent; }

      virtual void AddMsComponentToList( size_t component, UShort_t usDetectorId );
      virtual void SetNbMsInTs( size_t uCoreMsNb, size_t uOverlapMsNb );
      void SetIgnoreMsOverlap( Bool_t bEnaFlag = kTRUE ) { fbIgnoreOverlapMs = bEnaFlag; }
      void SetMsOverlap(size_t uOverlapMsNb = 1) { fuOverlapMsNb = uOverlapMsNb; }
      size_t GetMsOverlap() { return fuOverlapMsNb; }

      inline void SetFitZoomWidthPs( Double_t inZoomWidth = 1000.0 ) { fdFitZoomWidthPs = inZoomWidth; }
      inline void SetHistoryHistoSize( UInt_t inHistorySizeSec = 1800 ) { fuHistoryHistoSize = inHistorySizeSec; }
      inline void SetHistoryHistoSizeLong( UInt_t inHistorySizeMin = 1800 ) { fuHistoryHistoSizeLong = inHistorySizeMin; }

      inline void SetDiamondDpbIdx( UInt_t uIdx = 2 ) { fuDiamondDpbIdx = uIdx; }

      void SaveAllHistos( TString sFileName = "" );
      void ResetAllHistos();
      void ResetEvolutionHistograms();
      void ResetLongEvolutionHistograms();
      void UpdateNormedFt();
      void UpdateZoomedFit();

      void SetHistoFileName( TString sFileName = "data/SetupHistos.root" ) { fsHistoFileFullname = sFileName; }

   private:
      /// FLES containers
      std::vector< size_t >    fvMsComponentsList; //!
      size_t                   fuNbCoreMsPerTs; //!
      size_t                   fuNbOverMsPerTs; //!
      Bool_t                   fbIgnoreOverlapMs; //! /** Ignore Overlap Ms: all fuOverlapMsNb MS at the end of timeslice **/

      /// Histo File name and path
      TString fsHistoFileFullname;

      /// OLD, to be cleaned out !!!!!
      size_t   fuMsAcceptsPercent; /** Reject Ms with index inside TS above this, assumes 100 MS per TS **/
      size_t   fuTotalMsNb;        /** Total nb of MS per link in timeslice **/
      size_t   fuOverlapMsNb;      /** Overlap Ms: all fuOverlapMsNb MS at the end of timeslice **/
      size_t   fuCoreMs;           /** Number of non overlap MS at beginning of TS **/
      Double_t fdMsSizeInNs;
      Double_t fdTsCoreSizeInNs;
      UInt_t   fuMinNbGdpb;
      UInt_t   fuCurrNbGdpb;

      /** Settings from parameter file **/
      CbmMcbm2018TofPar* fUnpackPar;      //!
      UInt_t fuNrOfGdpbs;           // Total number of GDPBs in the system
      UInt_t fuNrOfFeePerGdpb;      // Number of FEBs per GDPB
      UInt_t fuNrOfGet4PerFee;      // Number of GET4s per FEE
      UInt_t fuNrOfChannelsPerGet4; // Number of channels in each GET4

      UInt_t fuNrOfChannelsPerFee;  // Number of channels in each FEE
      UInt_t fuNrOfGet4;            // Total number of Get4 chips in the system
      UInt_t fuNrOfGet4PerGdpb;     // Number of GET4s per GDPB
      UInt_t fuNrOfChannelsPerGdpb; // Number of channels per GDPB

      UInt_t fuNrOfGbtx;
      UInt_t fuNrOfModules;
      std::vector< Int_t > fviNrOfRpc;
      std::vector< Int_t > fviRpcType;
      std::vector< Int_t > fviRpcSide;
      std::vector< Int_t > fviModuleId;

      const UInt_t kuNbFeePerGbtx  = 5;
      const UInt_t kuNbGbtxPerGdpb = 6;

      /** Control Flags **/
      UInt_t fuRawDataPrintMsgNb;
      UInt_t fuRawDataPrintMsgIdx;
      Bool_t fbPrintAllHitsEnable;
      Bool_t fbPrintAllEpochsEnable;
      Bool_t fbOldFwData;
      UInt_t fuDiamondDpbIdx;

      /** Running indices **/
      uint64_t fulCurrentTsIndex;  // Idx of the current TS
      size_t   fuCurrentMs; // Idx of the current MS in TS (0 to fuTotalMsNb)
      Double_t fdMsIndex;   // Time in ns of current MS from its index
      UInt_t   fuGdpbId;    // Id (hex number) of the GDPB for current message
      UInt_t   fuGdpbNr;    // running number (0 to fuNrOfGdpbs) of the GDPB for current message
      UInt_t   fuGet4Id;    // running number (0 to fuNrOfGet4PerGdpb) of the Get4 chip of a unique GDPB for current message
      UInt_t   fuGet4Nr;    // running number (0 to fuNrOfGet4) of the Get4 chip in the system for current message
      Int_t    fiEquipmentId;
      std::vector<int> fviMsgCounter;

      ///* STAR TRIGGER detection *///
      std::vector< ULong64_t > fvulGdpbTsMsb;
      std::vector< ULong64_t > fvulGdpbTsLsb;
      std::vector< ULong64_t > fvulStarTsMsb;
      std::vector< ULong64_t > fvulStarTsMid;
      std::vector< ULong64_t > fvulGdpbTsFullLast;
      std::vector< ULong64_t > fvulStarTsFullLast;
      std::vector< UInt_t    > fvuStarTokenLast;
      std::vector< UInt_t    > fvuStarDaqCmdLast;
      std::vector< UInt_t    > fvuStarTrigCmdLast;

      /** Current epoch marker for each GDPB and GET4
        * (first epoch in the stream initializes the map item)
        * pointer points to an array of size fuNrOfGdpbs * fuNrOfGet4PerGdpb
        * The correct array index is calculated using the function
        * GetArrayIndex(gdpbId, get4Id)
        **/
      std::vector< ULong64_t > fvulCurrentEpoch; //!
      std::vector< Bool_t >    fvbFirstEpochSeen; //!
      std::vector< ULong64_t > fvulCurrentEpochCycle; //! Epoch cycle from the Ms Start message and Epoch counter flip
      std::vector< ULong64_t > fvulCurrentEpochFull; //! Epoch + Epoch Cycle

      ULong64_t fulCurrentEpochTime;     /** Time stamp of current epoch **/

      /// Map of ID to index for the gDPBs
      std::map<UInt_t, UInt_t> fGdpbIdIndexMap;

      /// Buffer for suppressed epoch processing
      std::vector< std::vector < gdpbv100::Message > > fvmEpSupprBuffer;

      /// Buffer for pulser channels
      std::vector< UInt_t >   fvuFeeNbHitsLastMs; //! [ fuFeeNr ]
      std::vector< Double_t > fdTsLastPulserHit;  //! [ fuFeeNr ]

      /// Buffers for coincidence maps
      std::vector< std::vector< UInt_t > >   fvuCoincNbHitsLastMs; //! [ fuNrOfGdpbs ][ fuNrOfChannelsPerGdpb ]
      std::vector< std::vector< Double_t > > fvdCoincTsLastHit;  //! [ fuNrOfGdpbs ][ fuNrOfChannelsPerGdpb ]

      /// Histograms and histogram control variables
         // Default value for nb bins in Pulser time difference histos
      const UInt_t kuNbBinsDt    = 2000;
      Double_t dMinDt;
      Double_t dMaxDt;
         // Default number of FEE per channels histograms
      UInt_t fuNbFeePlot;
      UInt_t fuNbFeePlotsPerGdpb;
         // Evolution plots control
      Double_t fdStartTime; /** Time of first valid hit (epoch available), used as reference for evolution plots**/
      Double_t fdStartTimeLong; /** Time of first valid hit (epoch available), used as reference for evolution plots**/
      Double_t fdStartTimeMsSz; /** Time of first microslice, used as reference for evolution plots**/
      UInt_t   fuHistoryHistoSize; /** Size in seconds of the evolution histograms **/
      UInt_t   fuHistoryHistoSizeLong; /** Size in minutes of the long evolution histograms **/
         // Pulser plots
      Double_t fdLastRmsUpdateTime;
      Double_t fdFitZoomWidthPs;
         // Flesnet
      TCanvas* fcMsSizeAll;
      std::vector< TH1      * > fvhMsSzPerLink;
      std::vector< TProfile * > fvhMsSzTimePerLink;

         // Messages types and flags
      ///* Pulser monitoring *///
      const Double_t kdMaxDtPulserPs = 300e3;
      std::vector< std::vector< TH1 * > > fvhTimeDiffPulser;
      TH2 * fhTimeMeanPulser;
      TH2 * fhTimeRmsPulser;
      TH2 * fhTimeRmsZoomFitPuls;
      TH2 * fhTimeResFitPuls;
      std::vector< TH2 * >     fvhPulserCountEvoPerFeeGdpb;
      std::vector< TProfile * >fvhPulserTimeDiffEvoGbtxGbtx;
      std::vector< std::vector< TProfile * > > fvvhPulserTimeDiffEvoGdpbGdpb;

      static const UInt_t kuNbRefFeeEvo = 2;
      const UInt_t kuRefFeeEvoIdx[ kuNbRefFeeEvo ] = { 0, 60 };
      std::vector< std::vector< TProfile * > > fvvhPulserTimeDiffEvoFeeFee;

      void CreateHistograms();

      void ProcessEpochCycle( uint64_t ulCycleData );
      void FillHitInfo(       gdpbv100::Message );
      void FillEpochInfo(     gdpbv100::Message );
      void PrintSlcInfo(      gdpbv100::Message );
      void PrintSysInfo(      gdpbv100::Message );
      void FillPattInfo(      gdpbv100::Message );
      void PrintGenInfo(      gdpbv100::Message );
      void FillStarTrigInfo(  gdpbv100::Message );

      inline Int_t GetArrayIndex(Int_t gdpbId, Int_t get4Id)
      {
         return gdpbId * fuNrOfGet4PerGdpb + get4Id;
      }

      ///* PADI channel to GET4 channel mapping and reverse *///
      std::vector< UInt_t > fvuPadiToGet4;
      std::vector< UInt_t > fvuGet4ToPadi;

      ///* GET4 to eLink mapping and reverse *///
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

      ///* Periodic histos saving *///
      std::chrono::time_point<std::chrono::system_clock> fTimeLastHistoSaving;

      CbmMcbm2018MonitorTofPulser(const CbmMcbm2018MonitorTofPulser&);
      CbmMcbm2018MonitorTofPulser operator=(const CbmMcbm2018MonitorTofPulser&);

   ClassDef(CbmMcbm2018MonitorTofPulser, 1)
};

#endif
