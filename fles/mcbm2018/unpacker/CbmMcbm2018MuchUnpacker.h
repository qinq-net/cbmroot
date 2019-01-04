// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmMcbm2018MuchUnpacker                    -----
// -----                 Modified 13.12.2018 by V. Singhal                 -----
// -----                 Created 26.06.2018 by V. Singhal and A. Kumar     -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMMCBM2018MUCHUNPACKER_H
#define CBMMCBM2018MUCHUNPACKER_H

#ifndef __CINT__
	#include "Timeslice.hpp"
//#include "rocMess_wGet4v1.h" used new class named StsXyterMessage
#endif
// Data
#include "StsXyterMessage.h"

//CbmRoot
#include "CbmTSUnpack.h"
//#include "CbmNxyterRawMessage.h"
#include "CbmMuchBeamTimeDigi.h" //Created a new Much Beam Time  Digi File 
#include "CbmMuchDigi.h" //Created a new Much Digi File for Global reconstruction
#include "CbmTbDaqBuffer.h"

#include <chrono>


#include "TClonesArray.h"

#include <vector>
#include <map>
#include <set>

class CbmDigi;
class CbmMcbm2018MuchPar; // Unpaker Class which will read the MuchUnpackPar.par
//class CbmMuchBeamTimeDigi;
class CbmMcbm2018MuchUnpacker: public CbmTSUnpack
{
 public:
  
  CbmMcbm2018MuchUnpacker();
  virtual ~CbmMcbm2018MuchUnpacker();
  
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
  
  void CreateRawMessageOutput(Bool_t val) {fCreateRawMessage=val;}
  
  //  Int_t CreateAddress(Int_t rocId, Int_t febId, Int_t stationId, Int_t layerId, Int_t sideId, Int_t moduleId, Int_t channelId);

  void SetMsOverlap(size_t uOverlapMsNb = 1) { fuOverlapMsNb = uOverlapMsNb; }
  size_t GetMsOverlap()                      { return fuOverlapMsNb; }
  
 private:
   size_t fuOverlapMsNb;      /** Ignore Overlap Ms: all fuOverlapMsNb MS at the end of timeslice **/

   // Parameters
   //CbmVeccSetupUnpackParGem* fUnpackPar; //!
   CbmMcbm2018MuchPar* fUnpackPar; //!
   UInt_t                   fuNrOfDpbs;       //! Total number of Gem DPBs in system
   std::map<UInt_t, UInt_t> fDpbIdIndexMap;   //! Map of DPB Identifier to DPB index
//   Int_t fMuchStationMapX[fUnpackPar->GetNbStsXyters()][fUnpackPar->GetNbChanPerAsic()]; //fMuchStationMap for map of row number via getX
//   Int_t fMuchStationMapY[fUnpackPar->GetNbStsXyters()][fUnpackPar->GetNbChanPerAsic()]; // For map of column number via getY>
   Int_t fMuchStationMapX[4][128]; //fMuchStationMap for map of row number via getX
   Int_t fMuchStationMapY[4][128]; // For map of column number via getY>

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
   Bool_t                fbChanHitDtEna;
      // Current data properties
   std::map< stsxyter::MessType, UInt_t > fmMsgCounter;
   UInt_t                fuCurrentEquipmentId;  //! Current equipment ID, tells from which DPB the current MS is originating
   UInt_t                fuCurrDpbId; //! Temp holder until Current equipment ID is properly filled in MS
   UInt_t                fuCurrDpbIdx;          //! Index of the DPB from which the MS currently unpacked is coming
   Int_t                 fiRunStartDateTimeSec; //! Start of run time since "epoch" in s, for the plots with date as X axis
   Int_t                 fiBinSizeDatePlots;    //! Bin size in s for the plots with date as X axis
   //std::vector< std::vector< UInt_t > > fvuCurrentTsMsbOver;   //! Current TS MSB overlap bits for each eLink
   std::vector< std::vector< ULong64_t > > fvulChanLastHitTime; //! Last hit time in bins for each Channel
   std::vector< std::vector<Double_t> > fvdChanLastHitTime;    //! Last hit time in ns   for each Channel
   //std::vector< std::vector<ULong64_t> > fvdChanLastHitTime;    //! Last hit time in ns   for each Channel
   std::vector< Double_t >         fvdMsTime;                  //! Header time of each MS

   std::vector< std::vector< std::vector< UInt_t > > > fvuChanNbHitsInMs;    //! Number of hits in each MS for each Channel
   std::vector< std::vector< std::vector< Double_t > > > fvdChanLastHitTimeInMs; //! Last hit time in bins in each MS for each Channel
   std::vector< std::vector< std::vector< UShort_t > > > fvusChanLastHitAdcInMs; //! Last hit ADC in bins in each MS for each Channel
   //std::vector< std::vector< std::multiset< stsxyter::BetaHit > > > fvmChanHitsInTs; //! All hits (time & ADC) in bins in last TS for each Channel
      // Starting state book-keeping
   Double_t              fdStartTime;           /** Time of first valid hit (TS_MSB available), used as reference for evolution plots**/
   Double_t              fdStartTimeMsSz;       /** Time of first microslice, used as reference for evolution plots**/
//   UShort_t              prevTime;
   //UShort_t                    adc;
   //std::chrono::steady_clock::time_point ftStartTimeUnix; /** Time of run Start from UNIX system, used as reference for long evolution plots against reception time **/
   // Data format control
   std::vector< ULong64_t > fvulCurrentTsMsb;                   //! Current TS MSB for each DPB
   std::vector< UInt_t    > fvuCurrentTsMsbCycle;               //! Current TS MSB cycle for DPB
   std::vector< UInt_t    > fvuElinkLastTsHit;                  //! TS from last hit for DPB


   Bool_t fbBetaFormat;
   // std::vector< std::vector< UInt_t > > fvuElinkLastTsHit;       //! TS from last hit for each eLink
    // TS/MS info
   ULong64_t             fulCurrentTsIdx;
   ULong64_t             fulCurrentMsIdx;

    // Coincidence histos
   UInt_t fuMaxNbMicroslices;
   UInt_t fuNbTsMsbSinceLastHit;

  //ULong_t fCurrentEpochTime;     /** Time stamp of current epoch **/
  //Double_t fdStartTime; /** Time of first valid hit (epoch available), used as reference for evolution plots**/
  
  //Int_t fEquipmentId;
  
  //For Unknown Message from stsxyter
  ULong64_t fulUnknownMessage = 0;


  
  TClonesArray* fMuchRaw;
  TClonesArray* fMuchBeamDigi;
  TClonesArray* fMuchDigi;
  //CbmNxyterRawMessage* fRawMessage;  
  //CbmSpsMuchDigi* fDigi;  //If CbmSpsMuchDigi will not work then will use this.
  CbmMuchBeamTimeDigi* fDigi;  
  //CbmMuchDigi* fDigi;  
  CbmTbDaqBuffer* fBuffer;
  
  Bool_t fCreateRawMessage;
  
  //CbmMuchUnpackPar* fUnpackPar;    //!
  //CbmVeccSetupUnpackParGemSmall* fUnpackPar;  
#ifndef __CINT__
	void FillHitInfo(   stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx );
	void FillTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx = 0, UInt_t uMsIdx = 0);
	void FillEpochInfo( stsxyter::Message mess );

#endif
  
  CbmMcbm2018MuchUnpacker(const CbmMcbm2018MuchUnpacker&);
  CbmMcbm2018MuchUnpacker operator=(const CbmMcbm2018MuchUnpacker&);
  
  ClassDef(CbmMcbm2018MuchUnpacker, 1)
    };

#endif
