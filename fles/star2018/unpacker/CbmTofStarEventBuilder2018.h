// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTofStarEventBuilder2018                 -----
// -----               Created 08.12.2017 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmTofStarEventBuilder2018_H
#define CbmTofStarEventBuilder2018_H

#ifndef __CINT__
  #include "Timeslice.hpp"
  #include "rocMess_wGet4v2.h"
  #include "CbmTofStarData.h"
  #include "CbmTofStarData2018.h"
#endif

#include "CbmTSUnpack.h"
#include "CbmHistManager.h"
#include "CbmTofDigi.h"
#include "CbmTofDigiExp.h"
#include "CbmTbDaqBuffer.h"

#include "TClonesArray.h"

#include <vector>
#include <map>

class CbmDigi;
class CbmTofStar2018Par;

#ifdef STAR_SUBEVT_BUILDER
   /*
    ** Function to send sub-event block to the STAR DAQ system
    *       trg_word received is packed as:
    *
    *       trg_cmd|daq_cmd|tkn_hi|tkn_mid|tkn_lo
   */
   extern "C" int star_rhicf_write(unsigned int trg_word, void *dta, int bytes);
#endif // STAR_SUBEVT_BUILDER

class CbmTofStarEventBuilder2018 : public CbmTSUnpack
{
public:

   CbmTofStarEventBuilder2018( UInt_t uNbGdpb = 1 );
   virtual ~CbmTofStarEventBuilder2018();

   virtual Bool_t Init();
#ifndef __CINT__
   virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
#endif
   virtual void Reset();

   virtual void Finish();

   void SetParContainers();

   Bool_t InitContainers();

   Bool_t ReInitContainers();

   // Setting change methods
   void SetMsLimitLevel( size_t uAcceptBoundaryPct = 100 ) { fuMsAcceptsPercent = uAcceptBoundaryPct; }
   size_t GetMsLimitLevel( ) { return fuMsAcceptsPercent; }

   inline void SetTShiftRef( Double_t dTShiftRefIn ) {fdTShiftRef = dTShiftRefIn;}

   inline void SetEventBuildingMode( Bool_t bEventBuildingMode = kFALSE );
   inline void SetTimeSortOutput( Bool_t bTimeSort = kTRUE );

   inline void SetHistoryHistoSize( UInt_t inHistorySizeSec = 1800 ) { fuHistoryHistoSize = inHistorySizeSec; }
   inline void SetHistoryHistoSizeLong( UInt_t inHistorySizeMin = 1800 ) { fuHistoryHistoSizeLong = inHistorySizeMin; }


   // Output control methods
   void FillOutput(CbmDigi* digi);
   void SaveAllHistos( TString sFileName = "" );
   void ResetAllHistos();

private:

   size_t   fuMsAcceptsPercent; /** Reject Ms with index inside TS above this, assumes 100 MS per TS **/
   size_t   fuTotalMsNb;      /** Total nb of MS per link in timeslice **/
   size_t   fuOverlapMsNb;      /** Ignore Overlap Ms: all fuOverlapMsNb MS at the end of timeslice **/
   Double_t fdMsSizeInNs;
   UInt_t fuMinNbGdpb;
   UInt_t fuCurrNbGdpb;

   /** Settings from parameter file **/
   UInt_t fuNrOfGdpbs;           // Total number of GDPBs in the system
   UInt_t fuNrOfFebsPerGdpb;     // Number of FEBs per GDPB
   UInt_t fuNrOfGet4PerFeb;      // Number of GET4s per FEB
   UInt_t fuNrOfChannelsPerGet4; // Number of channels in each GET4

   UInt_t fuNrOfChannelsPerFeet; // Number of channels in each FEET
   UInt_t fuNrOfGet4;            // Total number of Get4 chips in the system
   UInt_t fuNrOfGet4PerGdpb;     // Number of GET4s per GDPB
   UInt_t fuNrOfChannelsPerGdpb; // Number of channels per GDPB

   std::vector<int> fMsgCounter;
   std::map<UInt_t, UInt_t> fGdpbIdIndexMap;

   std::vector< Double_t >  fdStarTriggerDeadtime;
   std::vector< Double_t >  fdStarTriggerDelay;
   std::vector< Double_t >  fdStarTriggerWinSize;

   /** Running indices **/
   UInt_t fuGdpbId; // Id (hex number) of the GDPB for current message
   UInt_t fuGdpbNr; // running number (0 to fNrOfGdpbs) of the GDPB for current message
   UInt_t fuGet4Id; // running number (0 to fNrOfGet4PerGdpb) of the Get4 chip of a unique GDPB for current message
   UInt_t fuGet4Nr; // running number (0 to fNrOfGet4) of the Get4 chip in the system for current message

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
   Double_t fdStartTime; /** Time of first valid hit (epoch available), used as reference for evolution plots**/
   Double_t fdStartTimeLong; /** Time of first valid hit (epoch available), used as reference for evolution plots**/
   Double_t fdStartTimeMsSz; /** Time of first microslice, used as reference for evolution plots**/
   TCanvas* fcMsSizeAll;

   Int_t fEquipmentId;
   Double_t fdMsIndex;
   Double_t fdTShiftRef;

   TClonesArray* fTofDigi;
   CbmTofDigiExp* fDigi;

   CbmTbDaqBuffer* fBuffer;

   CbmTofStar2018Par* fUnpackPar;      //!

   void CreateHistograms();

   // Variables used for histo filling
   UInt_t            fuHistoryHistoSize;
   UInt_t            fuHistoryHistoSizeLong;
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
//   Bool_t fbDetChanThere[64];
//   TH2*   fhDetChanCoinc;

   ///* STAR TRIGGER detection *///
   ULong64_t fulGdpbTsMsb;
   ULong64_t fulGdpbTsLsb;
   ULong64_t fulStarTsMsb;
   ULong64_t fulStarTsMid;
   ULong64_t fulGdpbTsFullLast;
   ULong64_t fulStarTsFullLast;
   UInt_t    fuStarTokenLast;
   UInt_t    fuStarDaqCmdLast;
   UInt_t    fuStarTrigCmdLast;
   TH1 *     fhTokenMsgType;
   TH1 *     fhTriggerRate;
   TH2 *     fhCmdDaqVsTrig;
   TH2 *     fhStarTokenEvo;

   ///* STAR event building *///
   Bool_t fbEventBuilding; //! If false => build subevents in each MS => M1, if true => store data for full TS, then build subevents in TS => M2
   Bool_t fbTimeSortOutput;
   CbmTofStarSubevent2018 fStarSubEvent;
   ULong64_t fulNbBuiltSubEvent;
   ULong64_t fulNbStarSubEvent;
   ULong64_t fulNbBuiltSubEventLastPrintout;
   ULong64_t fulNbStarSubEventLastPrintout;
   Double_t fdCurrentMsStartTime;                          //! M1, Used in case of single link per subevent: test mode in 2018 S1, sector mode in 2018 S2
   Double_t fdCurrentMsEndTime;                            //! M1, Used in case of single link per subevent: test mode in 2018 S1, sector mode in 2018 S2
   std::vector< gdpb::FullMessage > fvmCurrentLinkBuffer;  //! M1, Used in case of single link per subevent: test mode in 2018 S1, sector mode in 2018 S2
   std::vector< CbmTofStarTrigger  > fvtCurrentLinkBuffer; //! M1, Used in case of single link per subevent: test mode in 2018 S1, sector mode in 2018 S2
   Double_t fdCurrentTsStartTime;                                      //! M2, Used in case of all links in same subevent: Sector mode in 2018 S1, Full eTOF mode in 2018 S2
   std::vector< std::vector < gdpb::FullMessage > > fvmTsLinksBuffer;  //! M2, Used in case of all links in same subevent: Sector mode in 2018 S1, Full eTOF mode in 2018 S2
   std::vector< std::vector < CbmTofStarTrigger  > > fvtTsLinksBuffer; //! M2, Used in case of all links in same subevent: Sector mode in 2018 S1, Full eTOF mode in 2018 S2
   std::vector<TH1*> fhStarHitToTrigAll_gDPB;
   std::vector<TH1*> fhStarHitToTrigWin_gDPB;
   std::vector<TH1*> fhStarEventSize_gDPB;
   std::vector<TH2*> fhStarEventSizeTime_gDPB;
   std::vector<TH2*> fhStarEventSizeTimeLong_gDPB;

#ifndef __CINT__
   std::vector< std::vector < gdpb::Message > > fvmEpSupprBuffer;

   void FillHitInfo(gdpb::Message);
   void FillStarTrigInfo(gdpb::Message);
   void FillEpochInfo(gdpb::Message);
   void PrintSlcInfo(gdpb::Message);
   void PrintSysInfo(gdpb::Message);
   void PrintGenInfo(gdpb::Message);
#endif
   inline Int_t GetArrayIndex(Int_t gdpbId, Int_t get4Id)
   {
      return gdpbId * fuNrOfGet4PerGdpb + get4Id;
   }

   void BuildStarEventsSingleLink();
   void BuildStarEventsAllLinks();

   CbmTofStarEventBuilder2018(const CbmTofStarEventBuilder2018&);
   CbmTofStarEventBuilder2018 operator=(const CbmTofStarEventBuilder2018&);

   ClassDef(CbmTofStarEventBuilder2018, 1)
};

#endif
