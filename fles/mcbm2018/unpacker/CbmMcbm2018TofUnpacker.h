// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmMcbm2018TofUnpacker                       -----
// -----               Created 08.12.2017 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmMcbm2018TofUnpacker_H
#define CbmMcbm2018TofUnpacker_H

#include "Timeslice.hpp"
 #include "gDpbMessv100.h"

#include "CbmTSUnpack.h"
#include "CbmHistManager.h"
#include "CbmTofDigi.h"
#include "CbmTofDigiExp.h"
#include "CbmTbDaqBuffer.h"

#include "TClonesArray.h"

#include <vector>
#include <map>

class CbmDigi;
class CbmMcbm2018TofPar;

class CbmMcbm2018TofUnpacker : public CbmTSUnpack
{
public:

   CbmMcbm2018TofUnpacker();
   virtual ~CbmMcbm2018TofUnpacker();

   virtual Bool_t Init();
   virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
   virtual void Reset();

   virtual void Finish();

   void SetParContainers();

   Bool_t InitContainers();

   Bool_t ReInitContainers();


   void FillOutput(CbmDigi* digi);

   void SetMsLimitLevel( size_t uAcceptBoundaryPct = 100 ) { fuMsAcceptsPercent = uAcceptBoundaryPct; }
   size_t GetMsLimitLevel( ) { return fuMsAcceptsPercent; }

   void SetMsOverlap(size_t uOverlapMsNb = 1) { fuOverlapMsNb = uOverlapMsNb; }
   size_t GetMsOverlap()                      { return fuOverlapMsNb; }

   inline void SetTShiftRef(Double_t val) {fdTShiftRef = val;}

private:

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
   UInt_t fuNrOfFeetPerGdpb;     // Number of FEBs per GDPB
   UInt_t fuNrOfGet4PerFeb;      // Number of GET4s per FEB
   UInt_t fuNrOfChannelsPerGet4; // Number of channels in each GET4

   UInt_t fuNrOfChannelsPerFeet; // Number of channels in each FEET
   UInt_t fuNrOfGet4;            // Total number of Get4 chips in the system
   UInt_t fuNrOfGet4PerGdpb;     // Number of GET4s per GDPB
   UInt_t fuNrOfChannelsPerGdpb; // Number of channels per GDPB

   const UInt_t kuNbFeePerGbtx  = 5;
   const UInt_t kuNbGbtxPerGdpb = 6;

   /** Running indices **/
   uint64_t fulCurrentTsIndex;  // Idx of the current TS
   size_t   fuCurrentMs; // Idx of the current MS in TS (0 to fuTotalMsNb)
   Double_t fdMsIndex;   // Time in ns of current MS from its index
   UInt_t   fuGdpbId;    // Id (hex number) of the GDPB for current message
   UInt_t   fuGdpbNr;    // running number (0 to fuNrOfGdpbs) of the GDPB for current message
   UInt_t   fuGet4Id;    // running number (0 to fuNrOfGet4PerGdpb) of the Get4 chip of a unique GDPB for current message
   UInt_t   fuGet4Nr;    // running number (0 to fuNrOfGet4) of the Get4 chip in the system for current message
   Int_t    fiEquipmentId;

   std::vector<int> fMsgCounter;
   std::map<UInt_t, UInt_t> fGdpbIdIndexMap;

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

   CbmHistManager* fHM;  ///< Histogram manager

   /** Current epoch marker for each GDPB and GET4
     * (first epoch in the stream initializes the map item)
     * pointer points to an array of size fNrOfGdpbs * fNrOfGet4PerGdpb
     * The correct array index is calculated using the function
     * GetArrayIndex(gdpbId, get4Id)
     **/
   std::vector< ULong64_t > fvulCurrentEpoch; //!
   std::vector< Bool_t >    fvbFirstEpochSeen; //!

   Int_t fNofEpochs;              /** Current epoch marker for each ROC **/
   ULong64_t fulCurrentEpochTime;     /** Time stamp of current epoch **/

   Double_t fdTShiftRef;

   TClonesArray* fTofDigi;
   CbmTofDigiExp* fDigi;

   CbmTbDaqBuffer* fBuffer;

   void CreateHistograms();

   // Variables used for histo filling
   Double_t fdRefTime;
   Double_t fdLastDigiTime;
   Double_t fdFirstDigiTimeDif;
   Double_t fdEvTime0;
   TH1* fhRawTDigEvT0;
   TH1* fhRawTDigRef0;
   TH1* fhRawTDigRef;
   TH1* fhRawTRefDig0;
   TH1* fhRawTRefDig1;
   TH1* fhRawDigiLastDigi;
   std::vector< TH2* > fhRawTotCh;
   std::vector< TH1* > fhChCount;
   std::vector< Bool_t > fvbChanThere;
   std::vector< TH2* > fhChanCoinc;
   Bool_t fbDetChanThere[64];
   TH2*   fhDetChanCoinc;

   void FillHitInfo(gdpbv100::Message);
   void FillStarTrigInfo(gdpbv100::Message);
   void FillEpochInfo(gdpbv100::Message);
   void PrintSlcInfo(gdpbv100::Message);
   void PrintSysInfo(gdpbv100::Message);
   void PrintGenInfo(gdpbv100::Message);

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
   std::vector< Int_t > fviRpcType;
   std::vector< Int_t > fviModuleId;
   std::vector< Int_t > fviNrOfRpc;
   std::vector< Int_t > fviRpcSide;
   std::vector< Int_t > fviRpcChUId;

   std::vector< std::vector < gdpbv100::Message > > fvmEpSupprBuffer;

   CbmMcbm2018TofUnpacker(const CbmMcbm2018TofUnpacker&);
   CbmMcbm2018TofUnpacker operator=(const CbmMcbm2018TofUnpacker&);

   ClassDef(CbmMcbm2018TofUnpacker, 1)
};

#endif
