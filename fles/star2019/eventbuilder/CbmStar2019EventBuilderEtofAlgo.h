// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                  CbmTofStarEventBuilderAlgo2019                   -----
// -----               Created 03.11.2018 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmTofStarEventBuilderAlgo2019_H
#define CbmTofStarEventBuilderAlgo2019_H

#include "Timeslice.hpp"
#include "gDpbMessv100.h"
#include "CbmTofStarData2019.h"

#include <vector>
#include <map>
#include <chrono>
#include <chrono>

class CbmTofStar2019Par;

class TCanvas;
class TH1;
class TH2;
class TProfile;
class THttpServer;

#ifdef STAR_SUBEVT_BUILDER
   /*
    ** Function to send sub-event block to the STAR DAQ system
    *       trg_word received is packed as:
    *
    *       trg_cmd|daq_cmd|tkn_hi|tkn_mid|tkn_lo
   */
   extern "C" int star_rhicf_write(unsigned int trg_word, void *dta, int bytes);
#endif // STAR_SUBEVT_BUILDER

class CbmTofStarEventBuilderAlgo2019 : // public CbmAlgo
{
   public:
      CbmTofStarEventBuilderAlgo2019( THttpServer* server = nullptr );
      ~CbmTofStarEventBuilderAlgo2019();

      Bool_t InitStsParameters();
      Bool_t CreateHistograms();
      Bool_t ProcessMs( const fles::Timeslice& ts, size_t uMsComp, UInt_t uMsIdx );
      Bool_t BuildEvents();

   private:
      /// Constants
      constexpr uint32_t kuBytesPerMessage = 8;

      /// Control flags
      Bool_t fbMonitorMode;

      /// FLES containers
      std::vector< size_t > fvMsComponentsList; //!
      size_t                fuNbCoreMsPerTs; //!
      size_t                fuNbOverMsPerTs; //!
      Bool_t                fbIgnoreOverlapMs; //! /** Ignore Overlap Ms: all fuOverlapMsNb MS at the end of timeslice **/

      /// Settings from parameter file
      CbmTofStar2019Par* fUnpackPar;      //!
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

      const UInt_t kuNbFeePerGbtx  = 5; /// TODO => In parameter!
      const UInt_t kuNbGbtxPerGdpb = 6; /// TODO => In parameter!

      /// Running indices
      uint64_t fulCurrentTsIndex;  // Idx of the current TS
      size_t   fuCurrentMs; // Idx of the current MS in TS (0 to fuTotalMsNb)
      Double_t fdMsIndex;   // Time in ns of current MS from its index
      UInt_t   fuGdpbId;    // Id (hex number) of the GDPB for current message
      UInt_t   fuGdpbNr;    // running number (0 to fuNrOfGdpbs) of the GDPB for current message
      UInt_t   fuGet4Id;    // running number (0 to fuNrOfGet4PerGdpb) of the Get4 chip of a unique GDPB for current message
      UInt_t   fuGet4Nr;    // running number (0 to fuNrOfGet4) of the Get4 chip in the system for current message
      Int_t    fiEquipmentId;

      /// Current time references for each GDPB: merged epoch marker, epoch cycle, full epoch [fuNrOfGdpbs]
      std::vector< < ULong64_t > fvvulCurrentEpoch; //!
      std::vector< < ULong64_t > fvvulCurrentEpochCycle; //! Epoch cycle from the Ms Start message and Epoch counter flip
      std::vector< < ULong64_t > fvvulCurrentEpochFull; //! Epoch + Epoch Cycle

      /// Buffers
      std::vector< gdpbv100::Message >     fvmEpSupprBuffer;        //! [sector]
      std::vector< gdpbv100::FullMessage > fvBufferMessages;        //! [sector]
      std::vector< gdpbv100::FullMessage > fvBufferMessagesOverlap; //! [sector]
      std::vector< CbmTofStarTrigger2019 > fvBufferTriggers;        //! [sector]
      std::vector< CbmTofStarTrigger2019 > fvBufferTriggersOverlap; //! [sector]

      void ProcessEpochCycle( uint64_t ulCycleData );
      void ProcessEpoch( gdpbv100::Message mess );
      void ProcessStarTrigger( gdpbv100::Message mess );
      
      void ProcessEpSupprBuffer( uint32_t uGdpbNr );
      
      void ProcessHit( gdpbv100::Message mess );
      void ProcessSlCtrl( gdpbv100::Message mess );
      void ProcessSysMess( gdpbv100::Message mess );
      void ProcessPattern( gdpbv100::Message mess );

      CbmTofStarEventBuilderAlgo2019(const CbmTofStarEventBuilderAlgo2019&);
      CbmTofStarEventBuilderAlgo2019 operator=(const CbmTofStarEventBuilderAlgo2019&);

      ClassDef(CbmTofStarEventBuilder2018, 1)
};

#endif
