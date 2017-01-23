// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSMonitorTofStar                        -----
// -----               Created 15.12.2016 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTSMonitorTofStar.h"
#include "CbmTofUnpackPar.h"

#include "CbmFlibCern2016Source.h"
#include "CbmHistManager.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRunOnline.h"

#include "TClonesArray.h"
#include "TString.h"
#include "THttpServer.h"
#include "Rtypes.h"
#include "TH1.h"
#include "TCanvas.h"
#include "THStack.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPaveStats.h"
#include "TDatime.h"

#include <algorithm>
#include <iostream>
#include <stdint.h>
#include <iomanip>

static Int_t iMess = 0;
Bool_t bResetTofStarHistos = kFALSE;
Bool_t bTofCyclePulserFee  = kFALSE;

namespace get4v1x {
   // Size of one clock cycle (=1 coarse bin)
   const double   kdClockCycleSize  = 6250.0; //[ps]
   // TODO:For now make 100ps default, maybe need later an option for it
   const double   kdTotBinSize      =   50.0; //ps

   const uint32_t kuFineTime    = 0x0000007F; // Fine Counter value
   const uint32_t kuFtShift     =          0; // Fine Counter offset
   const uint32_t kuCoarseTime  = 0x0007FF80; // Coarse Counter value
   const uint32_t kuCtShift     =          7; // Coarse Counter offset

   const uint32_t kuFineCounterSize    = ( (kuFineTime>>kuFtShift)+1 );
   const uint32_t kuCoarseCounterSize  = ( (kuCoarseTime>>kuCtShift)+1 );
   const uint32_t kuCoarseOverflowTest = kuCoarseCounterSize / 2 ; // Limit for overflow check
   const uint32_t kuTotCounterSize     = 256;

   // Nominal bin size of NL are neglected
   const double   kdBinSize     = kdClockCycleSize / static_cast<double>(kuFineCounterSize);
   // Epoch Size in bins
   const uint32_t kuEpochInBins = kuFineTime + kuCoarseTime + 1;
   // Epoch Size in ps
   // alternatively: (kiCoarseTime>>kiCtShift + 1)*kdClockCycleSize
   const double   kdEpochInPs   = kuEpochInBins*kdBinSize;
}

// Default value for nb bins in Pulser time difference histos
//const UInt_t kuNbBinsDt    = 5000;
const UInt_t kuNbBinsDt    = 150;
Double_t dMinDt     = -1.*(kuNbBinsDt*get4v1x::kdBinSize/2.) - get4v1x::kdBinSize/2.;
Double_t dMaxDt     =  1.*(kuNbBinsDt*get4v1x::kdBinSize/2.) + get4v1x::kdBinSize/2.;
//const UInt_t kuNbBinsDt    = 299;
//Double_t dMinDt     = -1.*((kuNbBinsDt+1)*get4v1x::kdBinSize/2.);
//Double_t dMaxDt     =  1.*((kuNbBinsDt+1)*get4v1x::kdBinSize/2.);

// Default number of FEET per channels histograms
UInt_t uNbFeetPlot = 2;

CbmTSMonitorTofStar::CbmTSMonitorTofStar() :
    CbmTSUnpack(),
    fuMsAcceptsPercent(100),
    fuMinNbGdpb(),
    fuCurrNbGdpb(0),
    fNrOfGdpbs(-1),
    fNrOfFebsPerGdpb(-1),
    fNrOfGet4PerFeb(-1),
    fNrOfChannelsPerGet4(-1),
    fNrOfChannelsPerFeet(-1),
    fNrOfGet4(-1),
    fNrOfGet4PerGdpb(-1),
    fiCountsLastTs(0),
    fiSpillOnThr(10),
    fiSpillOffThr(3),
    fiTsUnderOff(0),
    fiTsUnderOffThr(10),
    fdDetLastTime(-1.),
    fdDetTimeLastTs(-1.),
    fbSpillOn(kFALSE),
    fSpillIdx(0),
    fbEpochSuppModeOn(kFALSE),
    fvmEpSupprBuffer(),
    fGdpbId(0),
    fGdpbNr(0),
    fGet4Id(0),
    fGet4Nr(0),
    fMsgCounter(11, 0), // length of enum MessageTypes initialized with 0
    fGdpbIdIndexMap(),
    fHM(new CbmHistManager()),
    fCurrentEpoch(NULL),
    fNofEpochs(0),
    fCurrentEpochTime(0.),
    fdStartTime(-1.),
    fdStartTimeMsSz(-1.),
    fcMsSizeAll(NULL),
    fTsLastHit(),
    fEquipmentId(0),
    fUnpackPar(NULL),
    fHistMessType(NULL),
    fHistSysMessType(NULL),
    fHistGet4MessType(NULL),
    fHistGet4ChanErrors(NULL),
    fHistGet4EpochFlags(NULL),
    fHistSpill(NULL),
    fHistSpillLength(NULL),
    fHistSpillCount(NULL),
    fHistSpillQA(NULL),
    fRaw_Tot_gDPB(),
    fChCount_gDPB(),
    fChannelRate_gDPB(),
    fFeetRate_gDPB(),
    fFeetRateDate_gDPB(),
    fiRunStartDateTimeSec( -1 ),
    fiBinSizeDatePlots( -1 ),
    fulGdpbTsMsb(0),
    fulGdpbTsLsb(0),
    fulStarTsMsb(0),
    fulStarTsMid(0),
    fulGdpbTsFullLast(0),
    fulStarTsFullLast(0),
    fuStarTokenLast(0),
    fuStarDaqCmdLast(0),
    fuStarTrigCmdLast(0),
    fhTriggerRate(NULL),
    fhCmdDaqVsTrig(NULL),
    fhStarTokenEvo(NULL),
    fbGet4v20( kFALSE ),
    fbPulserMode( kFALSE ),
    fuPulserGdpb(0),
    fuPulserFee(0),
    fhTimeDiffPulserChosenFee(),
    fhTimeRmsPulserChosenFee(NULL),
    fhTimeRmsPulserChosenChPairs(NULL),
    fdLastRmsUpdateTime(-1),
    fbStarSortAndCutMode(kFALSE),
    fiStarActiveAsicMask(),
    fdStarTriggerDelay(),
    fdStarTriggerWinSize(),
    fuCurrentEpGdpb(),
    fiStarCurrentEpFound(),
    fiStarNextBufferUse(),
    fdStarLastTrigTimeG(),
    fiStarBuffIdxPrev(),
    fiStarBuffIdxCurr(),
    fiStarBuffIdxNext(),
    fvGdpbEpMsgBuffer(),
    fvGdpbEpTrgBuffer()
{
}

CbmTSMonitorTofStar::~CbmTSMonitorTofStar()
{
  delete fHM; //TODO: Who deletes the histograms stored in the CbmHistManager???
  delete[] fCurrentEpoch;
}

Bool_t CbmTSMonitorTofStar::Init()
{
  LOG(INFO) << "Initializing Get4 monitor" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  return kTRUE;
}

void CbmTSMonitorTofStar::SetParContainers()
{
  LOG(INFO) << "Setting parameter containers for " << GetName()
               << FairLogger::endl;
  fUnpackPar =
      (CbmTofUnpackPar*) (FairRun::Instance()->GetRuntimeDb()->getContainer(
          "CbmTofUnpackPar"));

}

Bool_t CbmTSMonitorTofStar::InitContainers()
{
  LOG(INFO) << "Init parameter containers for " << GetName()
               << FairLogger::endl;
  Bool_t initOK = ReInitContainers();

  CreateHistograms();

  fCurrentEpoch = new Int_t[fNrOfGdpbs * fNrOfGet4PerGdpb];
  for (Int_t i = 0; i < fNrOfGdpbs; ++i) {
    for (Int_t j = 0; j < fNrOfGet4PerGdpb; ++j) {
      fCurrentEpoch[GetArrayIndex(i, j)] = -111;
    }
  }

  return initOK;
}

Bool_t CbmTSMonitorTofStar::ReInitContainers()
{
  LOG(INFO) << "ReInit parameter containers for " << GetName()
               << FairLogger::endl;

  fNrOfGdpbs = fUnpackPar->GetNrOfRocs();
  LOG(INFO) << "Nr. of Tof GDPBs: " << fNrOfGdpbs << FairLogger::endl;
  fuMinNbGdpb = fNrOfGdpbs;

  fNrOfFebsPerGdpb = fUnpackPar->GetNrOfFebsPerGdpb();
  LOG(INFO) << "Nr. of FEBS per Tof GDPB: " << fNrOfFebsPerGdpb
               << FairLogger::endl;

  fNrOfGet4PerFeb = fUnpackPar->GetNrOfGet4PerFeb();
  LOG(INFO) << "Nr. of GET4 per Tof FEB: " << fNrOfGet4PerFeb
               << FairLogger::endl;

  fNrOfChannelsPerGet4 = fUnpackPar->GetNrOfChannelsPerGet4();
  LOG(INFO) << "Nr. of channels per GET4: " << fNrOfChannelsPerGet4
               << FairLogger::endl;
               
  fNrOfChannelsPerFeet = fNrOfGet4PerFeb * fNrOfChannelsPerGet4;
  LOG(INFO) << "Nr. of channels per FEET: " << fNrOfChannelsPerFeet
               << FairLogger::endl;

  fNrOfGet4 = fNrOfGdpbs * fNrOfFebsPerGdpb * fNrOfGet4PerFeb;
  LOG(INFO) << "Nr. of GET4s: " << fNrOfGet4 << FairLogger::endl;

  fNrOfGet4PerGdpb = fNrOfFebsPerGdpb * fNrOfGet4PerFeb;
  LOG(INFO) << "Nr. of GET4s per GDPB: " << fNrOfGet4PerGdpb
               << FairLogger::endl;

  fGdpbIdIndexMap.clear();
  for (Int_t i = 0; i < fNrOfGdpbs; ++i) {
    fGdpbIdIndexMap[fUnpackPar->GetRocId(i)] = i;
    LOG(INFO) << "GDPB Id of TOF  " << i << " : " << fUnpackPar->GetRocId(i)
                 << FairLogger::endl;
  }
  Int_t NrOfChannels = fUnpackPar->GetNumberOfChannels();
  LOG(INFO) << "Nr. of mapped Tof channels: " << NrOfChannels;
  for (Int_t i = 0; i < NrOfChannels; ++i) {
    if (i % 8 == 0)
      LOG(INFO) << FairLogger::endl;
    LOG(INFO) << Form(" 0x%08x", fUnpackPar->GetChannelToDetUIdMap(i));
  }
  LOG(INFO) << FairLogger::endl;

  LOG(INFO) << "Plot Channel Rate => "
               << (fUnpackPar->IsChannelRateEnabled() ? "ON" : "OFF")
               << FairLogger::endl;

  if( fbEpochSuppModeOn )
      fvmEpSupprBuffer.resize( fNrOfGet4 );
      
  if( fbStarSortAndCutMode )
  {
    fuCurrentEpGdpb.resize( fNrOfGdpbs );
    fiStarActiveAsicMask.resize( fNrOfGdpbs );
    fiStarCurrentEpFound.resize( fNrOfGdpbs );
    fiStarNextBufferUse.resize( fNrOfGdpbs );
    fdStarTriggerDelay.resize(   fNrOfGdpbs );
    fdStarTriggerWinSize.resize( fNrOfGdpbs );
    fdStarLastTrigTimeG.resize( fNrOfGdpbs );
    fiStarBuffIdxPrev.resize(    fNrOfGdpbs );
    fiStarBuffIdxCurr.resize(    fNrOfGdpbs );
    fiStarBuffIdxNext.resize(    fNrOfGdpbs );
    fvGdpbEpMsgBuffer.resize(      fNrOfGdpbs );
    fvGdpbEpTrgBuffer.resize(      fNrOfGdpbs );
    for (Int_t iGdpb = 0; iGdpb < fNrOfGdpbs; ++iGdpb)
    {
       fuCurrentEpGdpb[ iGdpb ] = 0;
       fiStarActiveAsicMask[ iGdpb ] = fUnpackPar->GetStarActiveMask( iGdpb );
       fiStarCurrentEpFound[ iGdpb ] = 0;
       fiStarNextBufferUse[ iGdpb ] = 0;
       fdStarTriggerDelay[ iGdpb ]   = fUnpackPar->GetStarTriggDelay( iGdpb );
       fdStarTriggerWinSize[ iGdpb ] = fUnpackPar->GetStarTriggWinSize( iGdpb );
       fdStarLastTrigTimeG[ iGdpb ]  = -1.0;
       fiStarBuffIdxPrev[ iGdpb ] = -1;
       fiStarBuffIdxCurr[ iGdpb ] =  0;
       fiStarBuffIdxNext[ iGdpb ] =  1;
       fvGdpbEpMsgBuffer[ iGdpb ].resize( 3 ); // 1 buff. for Prev, Curr and Next
       fvGdpbEpTrgBuffer[ iGdpb ].resize( 3 ); // 1 buff. for Prev, Curr and Next
       
       LOG(INFO) << "STAR trigger par of gDPB  " << iGdpb << " are: mask " << fiStarActiveAsicMask[ iGdpb ]
                 << " Delay " << fdStarTriggerDelay[ iGdpb ] 
                 << " and WinSz " << fdStarTriggerWinSize[ iGdpb ] 
                    << FairLogger::endl;
    } // for (Int_t iGdpb = 0; iGdpb < fNrOfGdpbs; ++iGdpb)
  } // if( fbStarSortAndCutMode )

  return kTRUE;
}

void CbmTSMonitorTofStar::CreateHistograms()
{
#ifdef USE_HTTP_SERVER
  THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
//  server->SetJSROOT("https://root.cern.ch/js/latest");
#endif

  TString name { "" };
  TString title { "" };

  name = "hMessageType";
  title = "Nb of message for each type; Type";
  // Test Big Data readout with plotting
  TH1I* hMessageType = new TH1I(name, title, 16, 0., 16.);
  //  TH1I* hMessageType = new TH1I(name, title, 16, -0.5, 15.5);
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_NOP, "NOP");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_HIT, "HIT");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_EPOCH, "EPOCH");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_SYNC, "SYNC");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_AUX, "AUX");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_EPOCH2, "EPOCH2");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4, "GET4");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_SYS, "SYS");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4_SLC,
      "MSG_GET4_SLC");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4_32B,
      "MSG_GET4_32B");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4_SYS,
      "MSG_GET4_SYS");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_STAR_TRI,
      "MSG_STAR_TRI");
  hMessageType->GetXaxis()->SetBinLabel(1 + 15, "GET4 Hack 32B");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_NOP, "NOP");
  fHM->Add(name.Data(), hMessageType);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H1(name.Data()));
#endif

  name = "hSysMessType";
  title = "Nb of system message for each type; System Type";
  TH1I* hSysMessType = new TH1I(name, title, 17, 0., 17.);
//  TH1I* hSysMessType = new TH1I(name, title, 17, -0.5, 16.5);
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,
      "DAQ START");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,
      "DAQ FINISH");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_NX_PARITY,
      "NX PARITY");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_SYNC_PARITY,
      "SYNC PARITY");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_RESUME,
      "DAQ RESUME");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_FIFO_RESET,
      "FIFO RESET");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_USER, "USER");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_PCTIME, "PCTIME");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_ADC, "ADC");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_PACKETLOST,
      "PACKET LOST");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_GET4_EVENT,
      "GET4 ERROR");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_CLOSYSYNC_ERROR,
      "CLOSYSYNC ERROR");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_TS156_SYNC,
      "TS156 SYNC");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_GDPB_UNKWN,
      "UNKW GET4 MSG");
  hSysMessType->GetXaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
  fHM->Add(name.Data(), hSysMessType);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H1(name.Data()));
#endif

  name = "hGet4MessType";
  title = "Nb of message for each type per GET4; GET4 chip # ; Type";
  TH2I* hGet4MessType = new TH2I(name, title, fNrOfGet4, 0., fNrOfGet4, 5, 0.,
      5.);
//  TH2I* hGet4MessType = new TH2I(name, title, uNbFeets*feetv1::kuChipPerFeet, -0.5, uNbFeets*feetv1::kuChipPerFeet -0.5, 5, -0.5, 4.5);
  hGet4MessType->GetYaxis()->SetBinLabel(1 + ngdpb::GET4_32B_EPOCH, "EPOCH");
  hGet4MessType->GetYaxis()->SetBinLabel(1 + ngdpb::GET4_32B_SLCM, "S.C. M");
  hGet4MessType->GetYaxis()->SetBinLabel(1 + ngdpb::GET4_32B_ERROR, "ERROR");
  hGet4MessType->GetYaxis()->SetBinLabel(1 + ngdpb::GET4_32B_DATA, "DATA 32b");
  hGet4MessType->GetYaxis()->SetBinLabel(1 + ngdpb::GET4_32B_DATA + 1,
      "DATA 24b");
  fHM->Add(name.Data(), hGet4MessType);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H2(name.Data()));
#endif

  name = "hGet4ChanErrors";
  title = "Error messages per GET4 channel; GET4 channel # ; Error";
  TH2I* hGet4ChanErrors = new TH2I(name, title,
      fNrOfGet4 * fNrOfChannelsPerGet4, 0., fNrOfGet4 * fNrOfChannelsPerGet4,
      32, 0., 32.);
//  TH2I* hGet4ChanErrors    = new TH2I(name, title, uNbFeets*feetv1::kuChanPerFeet*2, -0.5, uNbFeets*feetv1::kuChanPerFeet -0.5, 32, -0.5, 31.5);
  hGet4ChanErrors->GetYaxis()->SetBinLabel(1, "0x00: Readout Init    ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(2, "0x01: Sync            ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(3, "0x02: Epoch count sync");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(4, "0x03: Epoch           ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(5, "0x04: FIFO Write      ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(6, "0x05: Lost event      ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(7, "0x06: Channel state   ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(8, "0x07: Token Ring state");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(9, "0x08: Token           ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(10, "0x09: Error Readout   ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(11, "0x0a: SPI             ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(12, "0x0b: DLL Lock error  "); // <- From GET4 v1.2
  hGet4ChanErrors->GetYaxis()->SetBinLabel(13, "0x0c: DLL Reset invoc."); // <- From GET4 v1.2
  hGet4ChanErrors->GetYaxis()->SetBinLabel(14, "0x11: Overwrite       ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(15, "0x12: ToT out of range");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(16, "0x13: Event Discarded ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(17, "0x7f: Unknown         ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(18,
      "Corrupt error or unsupported yet");
  fHM->Add(name.Data(), hGet4ChanErrors);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H2(name.Data()));
#endif

  name = "hGet4EpochFlags";
  title = "Epoch flags per GET4; GET4 chip # ; Type";
  TH2I* hGet4EpochFlags = new TH2I(name, title, fNrOfGet4, 0., fNrOfGet4, 4, 0.,
      4.);
//  hEpochFlags = new TH2I(name, title, uNbFeets*feetv1::kuChipPerFeet, -0.5, uNbFeets*feetv1::kuChipPerFeet -0.5, 4, -0.5, 3.5);
  hGet4EpochFlags->GetYaxis()->SetBinLabel(1, "SYNC");
  hGet4EpochFlags->GetYaxis()->SetBinLabel(2, "Ep LOSS");
  hGet4EpochFlags->GetYaxis()->SetBinLabel(3, "Da LOSS");
  hGet4EpochFlags->GetYaxis()->SetBinLabel(4, "MISSMAT");
  fHM->Add(name.Data(), hGet4EpochFlags);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H2(name.Data()));
#endif

   // Prepare storing of hit time info
   if( fUnpackPar->IsChannelRateEnabled() || fbPulserMode )
   {
      fTsLastHit.resize( fNrOfGdpbs );
      for( UInt_t uGdpb = 0; uGdpb < fNrOfGdpbs; uGdpb++)
      {
         fTsLastHit[ uGdpb ].resize( fNrOfGet4PerGdpb );
         for( UInt_t uGet4 = 0; uGet4 < fNrOfGet4PerGdpb; uGet4++)
         {
            fTsLastHit[ uGdpb ][ uGet4 ].resize( fNrOfChannelsPerGet4 );
            for( UInt_t uCh = 0; uCh < fNrOfChannelsPerGet4; uCh++)
               fTsLastHit[ uGdpb ][ uGet4 ][ uCh ] = -1;
         } // for( UInt_t uGet4 = 0; uGet4 < fNrOfGet4PerGdpb; uGet4++)
      } // for( UInt_t uGdpb = 0; uGdpb < fNrOfGdpbs; uGdpb++)
   } // if( fUnpackPar->IsChannelRateEnabled() || fbPulserMode )

  if (fUnpackPar->IsChannelRateEnabled()) {
    for (UInt_t uGdpb = 0; uGdpb < fNrOfGdpbs; uGdpb++) {
      const Int_t iNbBinsRate = 82;
      Double_t dBinsRate[iNbBinsRate] = { 1e0, 2e0, 3e0, 4e0, 5e0, 6e0, 7e0,
          8e0, 9e0, 1e1, 2e1, 3e1, 4e1, 5e1, 6e1, 7e1, 8e1, 9e1, 1e2, 2e2, 3e2,
          4e2, 5e2, 6e2, 7e2, 8e2, 9e2, 1e3, 2e3, 3e3, 4e3, 5e3, 6e3, 7e3, 8e3,
          9e3, 1e4, 2e4, 3e4, 4e4, 5e4, 6e4, 7e4, 8e4, 9e4, 1e5, 2e5, 3e5, 4e5,
          5e5, 6e5, 7e5, 8e5, 9e5, 1e6, 2e6, 3e6, 4e6, 5e6, 6e6, 7e6, 8e6, 9e6,
          1e7, 2e7, 3e7, 4e7, 5e7, 6e7, 7e7, 8e7, 9e7, 1e8, 2e8, 3e8, 4e8, 5e8,
          6e8, 7e8, 8e8, 9e8, 1e9 };
      name = Form("ChannelRate_gDPB_%02u_0", uGdpb);
      title = Form("Channel instant rate gDPB %02u; Dist[ns] ; Channel", uGdpb);
      fHM->Add(name.Data(),
          new TH2F( name.Data(), title.Data(), iNbBinsRate - 1, dBinsRate, 
                    uNbFeetPlot*fNrOfChannelsPerFeet, 0, uNbFeetPlot*fNrOfChannelsPerFeet));
#ifdef USE_HTTP_SERVER
      if (server)
        server->Register("/TofRaw", fHM->H2(name.Data()));
#endif
      if( uNbFeetPlot < fNrOfFebsPerGdpb  )
      {
         name = Form("ChannelRate_gDPB_%02u_1", uGdpb);
         title = Form("Channel instant rate gDPB %02u; Dist[ns] ; Channel", uGdpb);
         fHM->Add(name.Data(),
             new TH2F( name.Data(), title.Data(), iNbBinsRate - 1, dBinsRate, 
                        uNbFeetPlot*fNrOfChannelsPerFeet, 
                        uNbFeetPlot*fNrOfChannelsPerFeet, 
                        2*uNbFeetPlot*fNrOfChannelsPerFeet));
#ifdef USE_HTTP_SERVER
         if (server)
           server->Register("/TofRaw", fHM->H2(name.Data()));
#endif
      } // if( uNbFeetPlot < fNrOfFebsPerGdpb  )
      if( 2 * uNbFeetPlot < fNrOfFebsPerGdpb )
      {
         name = Form("ChannelRate_gDPB_%02u_2", uGdpb);
         title = Form("Channel instant rate gDPB %02u; Dist[ns] ; Channel", uGdpb);
         fHM->Add(name.Data(),
             new TH2F( name.Data(), title.Data(), iNbBinsRate - 1, dBinsRate, 
                        uNbFeetPlot*fNrOfChannelsPerFeet, 
                        2*uNbFeetPlot*fNrOfChannelsPerFeet, 
                        3*uNbFeetPlot*fNrOfChannelsPerFeet));
#ifdef USE_HTTP_SERVER
         if (server)
           server->Register("/TofRaw", fHM->H2(name.Data()));
#endif
      } // if( 2 * uNbFeetPlot < fNrOfFebsPerGdpb )
      
      LOG(INFO) << "Adding the rate histos" << FairLogger::endl;
    } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
  }
  
  if( fbPulserMode )
  {
      // Full Fee time difference test
      UInt_t uNbBinsDt = kuNbBinsDt + 1; // To account for extra bin due to shift by 1/2 bin of both ranges
      for( UInt_t uChanFeeA = 0; uChanFeeA < fNrOfChannelsPerFeet; uChanFeeA++)
      {
         for( UInt_t uChanFeeB = uChanFeeA + 1; uChanFeeB < fNrOfChannelsPerFeet; uChanFeeB++)
         {
            fhTimeDiffPulserChosenFee.push_back( 
               new TH1I(
                  Form("hTimeDiffPulserChosenFee_%03u_%03u", uChanFeeA, uChanFeeB),
                  Form("Time difference for channels %03u and %03u in chosen Fee; DeltaT [ps]; Counts",
                        uChanFeeA, uChanFeeB),
                  uNbBinsDt, dMinDt, dMaxDt) );
         } // for any unique pair of channel in chosen Fee
      } // for( UInt_t uChanFeeA = 0; uChanFeeA < kuNbChanFee; uChanFeeA++)
      
      // Selected channels test
      for( UInt_t uChan = 0; uChan < kuNbChanTest - 1; uChan++)
      {
         fhTimeDiffPulserChosenChPairs[uChan]  = new TH1I(
               Form("hTimeDiffPulserChosenChPairs_%03u_%03u", fuPulserChan[uChan], fuPulserChan[uChan+1]),
               Form("Time difference for selected channels %03u and %03u in the first gDPB; DeltaT [ps]; Counts",
                     fuPulserChan[uChan], fuPulserChan[uChan+1]),
               uNbBinsDt, dMinDt, dMaxDt);
#ifdef USE_HTTP_SERVER
         if (server)
           server->Register("/TofRaw", fhTimeDiffPulserChosenChPairs[uChan] );
#endif
      } // for( UInt_t uChan = 0; uChan < kuNbChanTest - 1; uChan++)
      
      name = "hTimeRmsPulserChosenFee";
      fhTimeRmsPulserChosenFee = new TH2D( name.Data(),
            "Time difference RMS for any channels pair in chosen Fee; Ch A; Ch B; RMS [ps]",
            fNrOfChannelsPerFeet - 1, -0.5, fNrOfChannelsPerFeet - 1.5,
            fNrOfChannelsPerFeet - 1,  0.5, fNrOfChannelsPerFeet - 0.5);
      fHM->Add( name.Data(), fhTimeRmsPulserChosenFee);
#ifdef USE_HTTP_SERVER
      if (server)
        server->Register("/TofRaw", fHM->H2( name.Data() ) );
#endif

      name = "hTimeRmsPulserChosenChPairs";
      fhTimeRmsPulserChosenChPairs = new TH1D( name.Data(),
            "Time difference RMS for chosen channels pairs; Pair # ; [ps]",
            kuNbChanTest - 1, -0.5, kuNbChanTest - 1.5);
      fHM->Add( name.Data(), fhTimeRmsPulserChosenChPairs);
#ifdef USE_HTTP_SERVER
      if (server)
        server->Register("/TofRaw", fHM->H1( name.Data() ) );
#endif
  } // if( fbPulserMode )

  for (UInt_t uGdpb = 0; uGdpb < fNrOfGdpbs; uGdpb++) {
    name = Form("Raw_Tot_gDPB_%02u_0", uGdpb);
    title = Form("Raw TOT gDPB %02u; channel; TOT [bin]", uGdpb);
    fHM->Add(name.Data(),
        new TH2F(name.Data(), title.Data(), 
                 uNbFeetPlot*fNrOfChannelsPerFeet, 0, uNbFeetPlot*fNrOfChannelsPerFeet, 
                 256, 0, 256));
#ifdef USE_HTTP_SERVER
    if (server)
      server->Register("/TofRaw", fHM->H2(name.Data()));
#endif
    if( uNbFeetPlot < fNrOfFebsPerGdpb )
    {
      name = Form("Raw_Tot_gDPB_%02u_1", uGdpb);
      title = Form("Raw TOT gDPB %02u RPC 1; channel; TOT [bin]", uGdpb);
      fHM->Add(name.Data(),
               new TH2F(name.Data(), title.Data(), 
                  uNbFeetPlot*fNrOfChannelsPerFeet, 
                  uNbFeetPlot*fNrOfChannelsPerFeet, 
                  2*uNbFeetPlot*fNrOfChannelsPerFeet, 
                  256, 0, 256));
#ifdef USE_HTTP_SERVER
      if (server)
         server->Register("/TofRaw", fHM->H2(name.Data()));
#endif
    } // if( uNbFeetPlot < fNrOfFebsPerGdpb  ) 
    if( 2 * uNbFeetPlot < fNrOfFebsPerGdpb )
    {
      name = Form("Raw_Tot_gDPB_%02u_2", uGdpb);
      title = Form("Raw TOT gDPB %02u RPC 2; channel; TOT [bin]", uGdpb);
      fHM->Add(name.Data(),
               new TH2F(name.Data(), title.Data(), 
                  uNbFeetPlot*fNrOfChannelsPerFeet, 
                  2*uNbFeetPlot*fNrOfChannelsPerFeet, 
                  3*uNbFeetPlot*fNrOfChannelsPerFeet, 
                  256, 0, 256));
#ifdef USE_HTTP_SERVER
      if (server)
         server->Register("/TofRaw", fHM->H2(name.Data()));
#endif
    } // if( 2 * uNbFeetPlot < fNrOfFebsPerGdpb )

    name = Form("ChCount_gDPB_%02u", uGdpb);
    title = Form("Channel counts gDPB %02u; channel; Hits", uGdpb);
    fHM->Add(name.Data(), new TH1I(name.Data(), title.Data(), 
             fNrOfFebsPerGdpb*fNrOfChannelsPerFeet, 0, fNrOfFebsPerGdpb*fNrOfChannelsPerFeet));

#ifdef USE_HTTP_SERVER
    if (server)
      server->Register("/TofRaw", fHM->H1(name.Data()));
#endif

    for (UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; uFeet++) {
      name = Form("FeetRate_gDPB_g%02u_f%1u", uGdpb, uFeet);
      title = Form(
          "Counts per second in Feet %1u of gDPB %02u; Time[s] ; Counts", uFeet,
          uGdpb);
      fHM->Add(name.Data(), new TH1F(name.Data(), title.Data(), 1800, 0, 1800));
#ifdef USE_HTTP_SERVER
      if (server)
        server->Register("/TofRaw", fHM->H1(name.Data()));
#endif
      if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec ) {
        name = Form("FeetRateDate_gDPB_g%02u_f%1u", uGdpb, uFeet);
        title = Form(
          "Counts per second in Feet %1u of gDPB %02u; Time[s] ; Counts", uFeet,
          uGdpb);
        fHM->Add(name.Data(), new TH1F(name.Data(), title.Data(), (5400 / fiBinSizeDatePlots), -10, 5400 - 10));
        ( fHM->H1(name.Data()) )->GetXaxis()->SetTimeDisplay(1);
        ( fHM->H1(name.Data()) )->GetXaxis()->SetTimeOffset( fiRunStartDateTimeSec );
#ifdef USE_HTTP_SERVER
        if (server)
          server->Register("/TofRaw", fHM->H1(name.Data()));
#endif
      } // if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec )
    } // for( UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; uFeet ++)
  } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)

  name = "hSpill";
  title = "Counts per channel in Current Spill; X [Strip]; Y [End]; Counts";
  TH2I* hSpill = new TH2I(name, title, fNrOfChannelsPerFeet, 0., fNrOfChannelsPerFeet, 2, 0., 2.);
  fHM->Add(name.Data(), hSpill);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H2(name.Data()));
#endif

  name = "hSpillLength";
  title = "Length of spill interval as found from detectors; Length [s]; Counts";
  TH1* hSpillLength = new TH1F(name, title, 3000, 0., 300.);
  fHM->Add(name.Data(), hSpillLength);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H1(name.Data()));
#endif

  name = "hSpillCount";
  title = "Total counts in detector in each spill; Spill; Counts";
  TH1* hSpillCount = new TH1F(name, title, 300, 0., 300.);
  fHM->Add(name.Data(), hSpillCount);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H1(name.Data()));
#endif

  name = "hSpillQA";
  title = "Total counts in detector per spill VS Spill length; Length [s]; Counts";
  TH2* hSpillQA = new TH2F(name, title, 120, 0., 120., 150, 0., 150000.);
  fHM->Add(name.Data(), hSpillQA);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H2(name.Data()));
#endif

  name = "hTriggerRate";
  title = "STAR trigger signals per second; Time[s] ; Counts";
  fhTriggerRate = new TH1F(name, title, 1800, 0, 1800);
  fHM->Add(name.Data(), fhTriggerRate);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H1(name.Data()));
#endif

  name = "hCmdDaqVsTrig";
  title = "STAR daq command VS STAR trigger command; DAQ ; TRIGGER";
  fhCmdDaqVsTrig = new TH2I(name, title, 16, 0, 16, 16, 0, 16 );
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel( 1, "0x0: no-trig "); // idle link 
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel( 2, "0x1: clear   "); // clears redundancy counters on the readout boards
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel( 3, "0x2: mast-rst"); // general reset of the whole front-end logic
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel( 4, "0x3: spare   "); // reserved
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel( 5, "0x4: trigg. 0"); // Default physics readout, all det support required
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel( 6, "0x5: trigg. 1"); // 
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel( 7, "0x6: trigg. 2"); // 
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel( 8, "0x7: trigg. 3"); // 
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel( 9, "0x8: puls.  0"); // 
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel(10, "0x9: puls.  1"); // 
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel(11, "0xA: puls.  2"); // 
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel(12, "0xB: puls.  3"); // 
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel(13, "0xC: config  "); // housekeeping trigger: return geographic info of FE
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel(14, "0xD: abort   "); // aborts and clears an active event
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel(15, "0xE: L1accept"); // 
  fhCmdDaqVsTrig->GetXaxis()->SetBinLabel(16, "0xF: L2accept"); // 
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel( 1, "0x0:  0"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel( 2, "0x1:  1"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel( 3, "0x2:  2"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel( 4, "0x3:  3"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel( 5, "0x4:  4"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel( 6, "0x5:  5"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel( 7, "0x6:  6"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel( 8, "0x7:  7"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel( 9, "0x8:  8"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel(10, "0x9:  9"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel(11, "0xA: 10"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel(12, "0xB: 11"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel(13, "0xC: 12"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel(14, "0xD: 13"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel(15, "0xE: 14"); // To be filled at STAR
  fhCmdDaqVsTrig->GetYaxis()->SetBinLabel(16, "0xF: 15"); // To be filled at STAR
  fHM->Add(name.Data(), fhCmdDaqVsTrig);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H2(name.Data()));
#endif

  name = "hStarTokenEvo";
  title = "STAR token value VS time; Time in Run [s] ; STAR Token; Counts";
  fhStarTokenEvo = new TH2I(name, title, 1800, 0, 1800, 410, 0, 4100 ); // 4096
  fHM->Add(name.Data(), fhStarTokenEvo);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H2(name.Data()));
#endif
  
  
#ifdef USE_HTTP_SERVER
  if (server)
    server->RegisterCommand("/Reset_All_TOF", "bResetTofStarHistos=kTRUE");
  if (server)
    server->RegisterCommand("/Cycle_Pulser_FEE", "bTofCyclePulserFee=kTRUE");
  if (server)
    server->Restrict("/Reset_All_TOF", "allow=admin");
#endif

  /** Create summary Canvases for STAR 2017 **/
  Double_t w = 10;
  Double_t h = 10;
  TCanvas* cSummary = new TCanvas("cSummary", "gDPB Monitoring Summary", w, h);
  cSummary->Divide(2, 3);

  // 1st Column: Messages types
  cSummary->cd(1);
  gPad->SetLogy();
  hMessageType->Draw();

  cSummary->cd(2);
  gPad->SetLogy();
  hSysMessType->Draw();

  cSummary->cd(3);
  gPad->SetLogz();
  hGet4MessType->Draw("colz");

  // 2nd Column: GET4 Errors + Epoch flags +
  cSummary->cd(4);
  gPad->SetLogz();
  hGet4ChanErrors->Draw("colz");

  cSummary->cd(5);
  gPad->SetLogz();
  hGet4EpochFlags->Draw("colz");

  cSummary->cd(6);
  hSpill->Draw("col text");
  /*****************************/
  
  
  /** Create FEET rates Canvas for STAR 2017 **/
  TCanvas* cFeeRates = new TCanvas("cFeeRates", "gDPB Monitoring FEET rates", w, h);
  cFeeRates->Divide(fNrOfFebsPerGdpb, fNrOfGdpbs );
  
  TH1* histPnt = NULL;
  for( UInt_t uGdpb = 0; uGdpb < fNrOfGdpbs; ++uGdpb ) {
    for (UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; ++uFeet ) {
      name = Form("FeetRate_gDPB_g%02u_f%1u", uGdpb, uFeet);
      histPnt = fHM->H1(name.Data());
      
      cFeeRates->cd( 1 + uGdpb * fNrOfFebsPerGdpb + uFeet );
      gPad->SetLogy();
      
      histPnt->Draw();
    } // for (UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; ++uFeet )
  } // for( UInt_t uGdpb = 0; uGdpb < fNrOfGdpbs; ++uGdpb )
  /*****************************/
  
  /** Create TOT Canvas(es) for STAR 2017 **/
  TCanvas* cTotPnt = NULL;
  for( UInt_t uGdpb = 0; uGdpb < fNrOfGdpbs; ++uGdpb ) {
     
    cTotPnt = new TCanvas( Form("cTotPnt_g%02u", uGdpb), 
                           Form("gDPB %02u TOT distributions", uGdpb), 
                           w, h);
    cTotPnt->Divide( fNrOfFebsPerGdpb/uNbFeetPlot );
    TH2* histPntTot = NULL;
    for (UInt_t uFeetPlot = 0; uFeetPlot < fNrOfFebsPerGdpb/uNbFeetPlot; ++uFeetPlot ) {
      name = Form("Raw_Tot_gDPB_%02u_%1u", uGdpb, uFeetPlot);
      histPntTot = fHM->H2(name.Data());
      
      cTotPnt->cd( 1 + uFeetPlot );
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogz();
      
      histPntTot->Draw( "colz" );
    } // for (UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; ++uFeet )
  } // for( UInt_t uGdpb = 0; uGdpb < fNrOfGdpbs; ++uGdpb )
  /*****************************/
   
  /** Create Pulser mode Canvas for STAR 2017 **/
  if( fbPulserMode )
  {
     TCanvas* cPulserRms = new TCanvas("cPulserRms", "Time difference RMS for chosen FEE and channels in pulser mode", w, h);
     cPulserRms->Divide( 2 );
     
     cPulserRms->cd(1);
     fhTimeRmsPulserChosenFee->Draw( "colz" );
     
     cPulserRms->cd(2);
     fhTimeRmsPulserChosenChPairs->Draw( "colz" );
  } // if( fbPulserMode )
  /*****************************/
     
  /** Recovers/Create Ms Size Canvas for STAR 2017 **/  
  // Try to recover canvas in case it was created already by another monitor
  // If not existing, create it
  fcMsSizeAll = dynamic_cast<TCanvas *>( gROOT->FindObject( "cMsSizeAll" ) );
  if( NULL == fcMsSizeAll )
  {
     fcMsSizeAll = new TCanvas("cMsSizeAll", "Evolution of MS size in last 300 s", w, h);
     fcMsSizeAll->Divide( 4, 4 );
      LOG(INFO) << "Created MS size canvas in TOF monitor" << FairLogger::endl; 
  } // if( NULL == fcMsSizeAll )
      else LOG(INFO) << "Recovered MS size canvas in TOF monitor" << FairLogger::endl; 
  /*****************************/

  /** Store pointers to histograms for STAR 2017 **/  
  fHistMessType = fHM->H1("hMessageType");
  fHistSysMessType = fHM->H1("hSysMessType");
  fHistGet4MessType = fHM->H2("hGet4MessType");
  fHistGet4ChanErrors = fHM->H2("hGet4ChanErrors");
  fHistGet4EpochFlags = fHM->H2("hGet4EpochFlags");
  fHistSpill = fHM->H2("hSpill");
  fHistSpillLength = fHM->H1("hSpillLength");
  fHistSpillCount = fHM->H1("hSpillCount");
  fHistSpillQA = fHM->H2("hSpillQA");

  for (Int_t i = 0; i < fNrOfGdpbs; ++i) {
    name = Form("Raw_Tot_gDPB_%02u_0", i);
    fRaw_Tot_gDPB.push_back(fHM->H2(name.Data()));
    if( uNbFeetPlot < fNrOfFebsPerGdpb )
    {
       name = Form("Raw_Tot_gDPB_%02u_1", i);
       fRaw_Tot_gDPB.push_back(fHM->H2(name.Data()));
    } // if( uNbFeetPlot < fNrOfFebsPerGdpb )
    if( 2 * uNbFeetPlot < fNrOfFebsPerGdpb )
    {
       name = Form("Raw_Tot_gDPB_%02u_2", i);
       fRaw_Tot_gDPB.push_back(fHM->H2(name.Data()));
    } // if( 2 * uNbFeetPlot < fNrOfFebsPerGdpb )
    
    name = Form("ChCount_gDPB_%02u", i);
    fChCount_gDPB.push_back(fHM->H1(name.Data()));
    
    if (fUnpackPar->IsChannelRateEnabled()) {
      name = Form("ChannelRate_gDPB_%02u_0", i);
      fChannelRate_gDPB.push_back(fHM->H2(name.Data()));
      if( uNbFeetPlot < fNrOfFebsPerGdpb )
      {
        name = Form("ChannelRate_gDPB_%02u_1", i);
        fChannelRate_gDPB.push_back(fHM->H2(name.Data()));
      } // if( uNbFeetPlot < fNrOfFebsPerGdpb )
      if( 2 * uNbFeetPlot < fNrOfFebsPerGdpb )
      {
        name = Form("ChannelRate_gDPB_%02u_2", i);
        fChannelRate_gDPB.push_back(fHM->H2(name.Data()));
      } // if( 2 * uNbFeetPlot < fNrOfFebsPerGdpb )
    } // if (fUnpackPar->IsChannelRateEnabled())
    
    for (UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; uFeet++) {
      name = Form("FeetRate_gDPB_g%02u_f%1u", i, uFeet);
      fFeetRate_gDPB.push_back(fHM->H1(name.Data()));
      if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec ) {
        name = Form("FeetRateDate_gDPB_g%02u_f%1u", i, uFeet);
        fFeetRateDate_gDPB.push_back(fHM->H1(name.Data()));
      } // if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec )
    } // for( UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; uFeet ++)
  }
  /*****************************/
  
  LOG(INFO) << "Leaving CreateHistograms" << FairLogger::endl;
}

Bool_t CbmTSMonitorTofStar::DoUnpack(const fles::Timeslice& ts,
    size_t component)
{
#ifdef USE_HTTP_SERVER
  THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
#endif

  if (bResetTofStarHistos) {
    ResetAllHistos();
    bResetTofStarHistos = kFALSE;
  }
  if (bTofCyclePulserFee) {
     CyclePulserFee();
     bTofCyclePulserFee = kFALSE;
  }

  LOG(DEBUG1) << "Timeslice contains " << ts.num_microslices(component)
                 << "microslices." << FairLogger::endl;

  // Getting the pointer to the correct histogram needs a lot more time then
  // the actual filling procedure. If one gets the pointer in a loop this kills
  // the performance. A test shows that extracting the pointer from the CbmHistManger
  // is slower by a factor of 20-100 (depending on the number of histos managed by the
  // CbmHistManager) compared to using the pointer directly
  // So get the pointer once outside the loop and use it in the loop

  TString sMsSzName = Form("MsSz_link_%02lu", component);
  TH1* hMsSz = NULL;
  TProfile* hMsSzTime = NULL;
  if (fHM->Exists(sMsSzName.Data())) {
    hMsSz = fHM->H1(sMsSzName.Data());
    sMsSzName = Form("MsSzTime_link_%02lu", component);
    hMsSzTime = fHM->P1(sMsSzName.Data());
  } // if( fHM->Exists(sMsSzName.Data() ) )
  else {
    TString sMsSzTitle = Form(
        "Size of MS for gDPB of link %02lu; Ms Size [bytes]", component);
    fHM->Add(sMsSzName.Data(),
        new TH1F(sMsSzName.Data(), sMsSzTitle.Data(), 160000, 0., 20000.));
    hMsSz = fHM->H1(sMsSzName.Data());
#ifdef USE_HTTP_SERVER
    if (server)
      server->Register("/FlibRaw", hMsSz);
#endif
    sMsSzName = Form("MsSzTime_link_%02lu", component);
    sMsSzTitle = Form(
        "Size of MS vs time for gDPB of link %02lu; Time[s] ; Ms Size [bytes]",
        component);
    fHM->Add(sMsSzName.Data(),
        new TProfile(sMsSzName.Data(), sMsSzTitle.Data(), 180000, 0., 3600.));
    hMsSzTime = fHM->P1(sMsSzName.Data());
#ifdef USE_HTTP_SERVER
    if (server)
      server->Register("/FlibRaw", hMsSzTime);
#endif
    if( NULL != fcMsSizeAll )
    {
      fcMsSizeAll->cd( 1 + component );
      gPad->SetLogy();
      hMsSzTime->Draw("hist le0");
    } // if( NULL != fcMsSizeAll )
    LOG(INFO) << "Added MS size histo for component: " << component << " (gDPB)"
                 << FairLogger::endl;
  } // else of if( fHM->Exists(sMsSzName.Data() ) )

  // Initialize spill detection
  fiCountsLastTs = 0;

  Int_t messageType = -111;
  Double_t dTsStartTime = -1;
  // Loop over microslices
  for (size_t m = 0; m < ts.num_microslices(component); ++m) {     
    if (fuMsAcceptsPercent < m)
      continue;

    constexpr uint32_t kuBytesPerMessage = 8;

    auto msDescriptor = ts.descriptor(component, m);
    fEquipmentId = msDescriptor.eq_id;
    const uint8_t* msContent = reinterpret_cast<const uint8_t*>(ts.content(
        component, m));

    uint32_t size = msDescriptor.size;
    if (size > 0)
      LOG(DEBUG) << "Microslice: " << msDescriptor.idx << " has size: " << size
                    << FairLogger::endl;

    if( 0 == m )
      dTsStartTime = (1e-9) * static_cast<double>(msDescriptor.idx);

    if( fdStartTimeMsSz < 0 )
      fdStartTimeMsSz = (1e-9) * static_cast<double>(msDescriptor.idx);
    hMsSz->Fill(size);
    hMsSzTime->Fill((1e-9) * static_cast<double>(msDescriptor.idx) - fdStartTimeMsSz, size);

    // If not integer number of message in input buffer, print warning/error
    if (0 != (size % kuBytesPerMessage))
      LOG(ERROR) << "The input microslice buffer does NOT "
                    << "contain only complete nDPB messages!"
                    << FairLogger::endl;

    // Compute the number of complete messages in the input microslice buffer
    uint32_t uNbMessages = (size - (size % kuBytesPerMessage))
        / kuBytesPerMessage;

    // Prepare variables for the loop on contents
    const uint64_t* pInBuff = reinterpret_cast<const uint64_t*>(msContent);
    for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx++) {
      // Fill message
      uint64_t ulData = static_cast<uint64_t>(pInBuff[uIdx]);
      ngdpb::Message mess(ulData);

      if (gLogger->IsLogNeeded(DEBUG2)) {
        mess.printDataCout();
      }

      // Increment counter for different message types 
      // and fill the corresponding histogram
      messageType = mess.getMessageType();
      fMsgCounter[messageType]++;
      fHistMessType->Fill(messageType);

      fGdpbId = mess.getRocNumber();
      fGdpbNr = fGdpbIdIndexMap[fGdpbId];

      fGet4Id = mess.getGdpbGenChipId();
      fGet4Nr = (fGdpbNr * fNrOfGet4PerGdpb) + fGet4Id;

      switch (messageType) {
        case ngdpb::MSG_HIT:
          //           FillHitInfo(mess);
          LOG(ERROR) << "Message type " << messageType
                        << " not yet included in unpacker." << FairLogger::endl;
          break;
        case ngdpb::MSG_EPOCH:
          //           FillEpochInfo(mess);
          LOG(ERROR) << "Message type " << messageType
                        << " not yet included in unpacker." << FairLogger::endl;
          break;
        case ngdpb::MSG_SYS:
          fHistSysMessType->Fill(mess.getSysMesType());
          break;
        case ngdpb::MSG_EPOCH2:
          fHistGet4MessType->Fill(fGet4Nr, ngdpb::GET4_32B_EPOCH);
          FillEpochInfo(mess);
          break;
        case ngdpb::MSG_GET4:
          fHistGet4MessType->Fill(fGet4Nr, ngdpb::GET4_32B_DATA + 1);
          PrintGenInfo(mess);
          break;
        case ngdpb::MSG_GET4_32B:
          fHistGet4MessType->Fill(fGet4Nr, ngdpb::GET4_32B_DATA);
          if( fbEpochSuppModeOn )
            fvmEpSupprBuffer[fGet4Nr].push_back( mess );
            else FillHitInfo(mess);
          break;
        case ngdpb::MSG_GET4_SLC:
          fHistGet4MessType->Fill(fGet4Nr, ngdpb::GET4_32B_SLCM);
          PrintSlcInfo(mess);
          break;
        case ngdpb::MSG_GET4_SYS: {
          fHistSysMessType->Fill(mess.getGdpbSysSubType());
          if (ngdpb::SYSMSG_GET4_EVENT == mess.getGdpbSysSubType()) {
            fHistGet4MessType->Fill(fGet4Nr, ngdpb::GET4_32B_ERROR);
            Int_t dFullChId =  fGet4Nr * fNrOfChannelsPerGet4 + mess.getGdpbHitChanId();
            switch (mess.getGdpbSysErrData()) {
              case ngdpb::GET4_V1X_ERR_READ_INIT:
                fHistGet4ChanErrors->Fill(dFullChId, 0);
                break;
              case ngdpb::GET4_V1X_ERR_SYNC:
                fHistGet4ChanErrors->Fill(dFullChId, 1);
                break;
              case ngdpb::GET4_V1X_ERR_EP_CNT_SYNC:
                fHistGet4ChanErrors->Fill(dFullChId, 2);
                break;
              case ngdpb::GET4_V1X_ERR_EP:
                fHistGet4ChanErrors->Fill(dFullChId, 3);
                break;
              case ngdpb::GET4_V1X_ERR_FIFO_WRITE:
                fHistGet4ChanErrors->Fill(dFullChId, 4);
                break;
              case ngdpb::GET4_V1X_ERR_LOST_EVT:
                fHistGet4ChanErrors->Fill(dFullChId, 5);
                break;
              case ngdpb::GET4_V1X_ERR_CHAN_STATE:
                fHistGet4ChanErrors->Fill(dFullChId, 6);
                break;
              case ngdpb::GET4_V1X_ERR_TOK_RING_ST:
                fHistGet4ChanErrors->Fill(dFullChId, 7);
                break;
              case ngdpb::GET4_V1X_ERR_TOKEN:
                fHistGet4ChanErrors->Fill(dFullChId, 8);
                break;
              case ngdpb::GET4_V1X_ERR_READOUT_ERR:
                fHistGet4ChanErrors->Fill(dFullChId, 9);
                break;
              case ngdpb::GET4_V1X_ERR_SPI:
                fHistGet4ChanErrors->Fill(dFullChId, 10);
                break;
              case ngdpb::GET4_V1X_ERR_DLL_LOCK:
                fHistGet4ChanErrors->Fill(dFullChId, 11);
                break;
              case ngdpb::GET4_V1X_ERR_DLL_RESET:
                fHistGet4ChanErrors->Fill(dFullChId, 12);
                break;
              case ngdpb::GET4_V1X_ERR_TOT_OVERWRT:
                fHistGet4ChanErrors->Fill(dFullChId, 13);
                break;
              case ngdpb::GET4_V1X_ERR_TOT_RANGE:
                fHistGet4ChanErrors->Fill(dFullChId, 14);
                break;
              case ngdpb::GET4_V1X_ERR_EVT_DISCARD:
                fHistGet4ChanErrors->Fill(dFullChId, 15);
                break;
              case ngdpb::GET4_V1X_ERR_UNKNOWN:
                fHistGet4ChanErrors->Fill(dFullChId, 16);
                break;
              default: // Corrupt error or not yet supported error
                fHistGet4ChanErrors->Fill(dFullChId, 17);
                break;
            } // Switch( mess.getGdpbSysErrData() )

          }
          if (100 > iMess++)
            PrintSysInfo(mess);
          break;
        }
        case ngdpb::MSG_STAR_TRI:
          fHistGet4MessType->Fill(fGet4Nr, ngdpb::MSG_STAR_TRI);
          FillStarTrigInfo(mess);
          break;
        default:
          if (100 > iMess++)
            LOG(ERROR) << "Message (" << iMess << ") type " << std::hex
                          << std::setw(2) << static_cast<uint16_t>(messageType)
                          << " not yet included in Get4 unpacker."
                          << FairLogger::endl;
          if (100 == iMess)
            LOG(ERROR) << "Stop reporting MSG errors... " << FairLogger::endl;
      } // switch( mess.getMessageType() ) 
    } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)
  } // for (size_t m = 0; m < ts.num_microslices(component); ++m)

  // Spill detection using Detectors
  if( !fbSpillOn && (fiSpillOnThr < fiCountsLastTs) )
  {
    fbSpillOn = kTRUE;
    fiTsUnderOff = 0;

    if( 0 < fdDetTimeLastTs )
    {
      fHistSpillLength->Fill( fdDetLastTime - fdDetTimeLastTs );
      fHistSpillCount->Fill( fSpillIdx, fHistSpill->GetEntries() );
      fHistSpillQA->Fill( fdDetLastTime - fdDetTimeLastTs, 
                                 fHistSpill->GetEntries() );
    } // if( 0 < fdTimeLastTs )
    fdDetTimeLastTs = fdDetLastTime;
    fSpillIdx++;
    fHistSpill->Reset();
  } // if( !fbSpillOn && (fiSpillOnThr < fiCountsLastTs) )
  else if( fbSpillOn  )
  {
    if( fiCountsLastTs < fiSpillOffThr )
    {
      fiTsUnderOff ++;
      if( fiTsUnderOffThr < fiTsUnderOff )
        fbSpillOn = kFALSE;
    } //  if( fiCountsLastTs < fiSpillOffThr )
      else fiTsUnderOff = 0;
  } // else if( fbSpillOn  )

   if( kTRUE == fbPulserMode )
   {
      // Update RMS plots only every 10s in data
      if( 10.0 < dTsStartTime - fdLastRmsUpdateTime )
      {
         // Reset summary histograms for safety
         fhTimeRmsPulserChosenFee->Reset();
         fhTimeRmsPulserChosenChPairs->Reset();

         UInt_t uHistoFeeIdx = 0;
         for( UInt_t uChanFeeA = 0; uChanFeeA < fNrOfChannelsPerFeet; uChanFeeA++)
         {
            for( UInt_t uChanFeeB = uChanFeeA + 1; uChanFeeB < fNrOfChannelsPerFeet; uChanFeeB++)
            {
               fhTimeRmsPulserChosenFee->Fill(uChanFeeA, uChanFeeB, fhTimeDiffPulserChosenFee[uHistoFeeIdx]->GetRMS() );
               uHistoFeeIdx++;
            } // for( UInt_t uChanFeeB = uChanFeeA + 1; uChanFeeB < fNrOfChannelsPerFeet; uChanFeeB++)
         } // for( UInt_t uChanFeeA = 0; uChanFeeA < fNrOfChannelsPerFeet; uChanFeeA++)
         for( UInt_t uChan = 0; uChan < kuNbChanTest - 1; uChan++)
         {
            fhTimeRmsPulserChosenChPairs->Fill( uChan, fhTimeDiffPulserChosenChPairs[uChan]->GetRMS() );
         } // for( UInt_t uChan = 0; uChan < kuNbChanTest - 1; uChan++)
         fdLastRmsUpdateTime = dTsStartTime;
      } // if( 10.0 < dTsStartTime - fdLastRmsUpdateTime )
   } // if( kTRUE == fbPulserMode )

  return kTRUE;
}

void CbmTSMonitorTofStar::FillHitInfo(ngdpb::Message mess)
{
  Int_t channel = mess.getGdpbHitChanId();
  Int_t tot = mess.getGdpbHit32Tot();
  ULong_t hitTime = mess.getMsgFullTime(0);

  Int_t curEpochGdpbGet4 = fCurrentEpoch[fGet4Nr];
  
  if (curEpochGdpbGet4 != -111) {

    if( fbEpochSuppModeOn )
      curEpochGdpbGet4 --; // In Ep. Suppr. Mode, receive following epoch instead of previous
  
    Int_t channelNr = fGet4Id * fNrOfChannelsPerGet4 + channel;
    Int_t iFeetNr   = (fGet4Id / fNrOfGet4PerFeb);
    fRaw_Tot_gDPB[fGdpbNr + iFeetNr/uNbFeetPlot]->Fill(channelNr, tot);
    fChCount_gDPB[fGdpbNr]->Fill(channelNr);

    if (fUnpackPar->IsChannelRateEnabled()) {
      // Check if at least one hit before in this channel
      if( -1 < fTsLastHit[fGdpbNr][fGet4Id][channel] )
      {
         fChannelRate_gDPB[fGdpbNr + iFeetNr/uNbFeetPlot]->Fill(
             1e9 / (mess.getMsgFullTimeD(curEpochGdpbGet4)
                     - fTsLastHit[fGdpbNr][fGet4Id][channel]),
             fGet4Id * fNrOfChannelsPerGet4 + channel);
      } // if( -1 < fTsLastHit[fGdpbNr][fGet4Id][channel] )
    } // if( fUnpackPar->IsChannelRateEnabled() )

    // Save last hist time if channel rate histos or pulser mode enabled
    if( fUnpackPar->IsChannelRateEnabled() || fbPulserMode )
      fTsLastHit[fGdpbNr][fGet4Id][channel] = mess.getMsgFullTimeD(
          curEpochGdpbGet4);

    // In Run rate evolution
    if (fdStartTime < 0)
      fdStartTime = mess.getMsgFullTimeD(curEpochGdpbGet4);

    if (0 <= fdStartTime)
    {
      fFeetRate_gDPB[(fGdpbNr * fNrOfFebsPerGdpb) + iFeetNr]->Fill(
          1e-9 * (mess.getMsgFullTimeD(curEpochGdpbGet4) - fdStartTime));
          
       // General Time (date + time) rate evolution
       // Add offset of -1H as the filenames were using some times offset by 1 hour (Summer time?!?)
       if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec ) {
         fFeetRateDate_gDPB[(fGdpbNr * fNrOfFebsPerGdpb) + iFeetNr]->Fill(
             1e-9 * (mess.getMsgFullTimeD(curEpochGdpbGet4) - fdStartTime)  );
       } // if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec )
    }

    Int_t iChanInGdpb = fGet4Id * fNrOfChannelsPerGet4 + channel;
    Int_t increment = static_cast<CbmFlibCern2016Source*>(FairRunOnline::Instance()->GetSource())->GetNofTSSinceLastTS();
/*
    // if condition to find the right strip/end index
      fHistSpill->Fill(0., 0., increment);
*/      
    fiCountsLastTs ++;
    fdDetLastTime = 1e-9 * mess.getMsgFullTimeD(curEpochGdpbGet4);


    ///* STAR event building/cutting *///
    if( fbStarSortAndCutMode )
    {
      /// Only use GET4s declared as active
      if( fiStarActiveAsicMask[ fGdpbNr ] & (0x1 << fGet4Id) )
      {
         /// If chip is already ready for current epoch, save the message in the buffer for the next epoch
         if( 0x1 & ( fiStarNextBufferUse[ fGdpbNr ] >> fGet4Id ) )
            fvGdpbEpMsgBuffer[ fGdpbNr ][ fiStarBuffIdxNext[ fGdpbNr ] ].push_back( mess );
            else fvGdpbEpMsgBuffer[ fGdpbNr ][ fiStarBuffIdxCurr[ fGdpbNr ] ].push_back( mess );
      } // if( fiStarActiveAsicMask[ fGdpbNr ] & (0x1 << fGet4Id) )
    } // if( fbStarSortAndCutMode )

    hitTime = mess.getMsgFullTime(curEpochGdpbGet4);
    Int_t Ft = mess.getGdpbHitFineTs();

    if (100 > iMess++)
       LOG(DEBUG) << "Hit: " << Form("0x%08x ", fGdpbId) << ", " << fGet4Id
                     << ", " << channel << ", " << tot << ", epoch "
                     << curEpochGdpbGet4 << ", FullTime "
                     << hitTime << ", FineTime " << Ft << FairLogger::endl;
  } // if( fCurrentEpoch[rocId].end() != fCurrentEpoch[rocId].find( get4Id ) )
    //} // if( fCurrentEpoch.end() != fCurrentEpoch.find( rocId ) )
  //} // if( fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find( rocId ) )
  else
    LOG(WARNING) << "Found hit in gdpbId w/o epoch yet: "
                    << Form("0x%08x g4 %02u", fGdpbId, fGet4Nr) << FairLogger::endl;
}

void CbmTSMonitorTofStar::FillEpochInfo(ngdpb::Message mess)
{
  Int_t epochNr = mess.getEpoch2Number();
  fCurrentEpoch[fGet4Nr] = epochNr;

  if (100 > iMess++)
      LOG(DEBUG) << "Epoch message for get4 " << fGet4Nr << " with epoch number "
                 << fCurrentEpoch[fGet4Nr] << FairLogger::endl;


  if( fbEpochSuppModeOn )
  {
    Int_t iBufferSize = fvmEpSupprBuffer[fGet4Nr].size();
    if( 0 < iBufferSize )
    {
      LOG(DEBUG) << "Now processing stored messages for for get4 " << fGet4Nr << " with epoch number "
                 << (fCurrentEpoch[fGet4Nr] - 1) << FairLogger::endl;

      for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
      {
        FillHitInfo( fvmEpSupprBuffer[fGet4Nr][ iMsgIdx ] );
      } // for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
      
      fvmEpSupprBuffer[fGet4Nr].clear();
    } // if( 0 < fvmEpSupprBuffer[fGet4Nr] )
  } // if( fbEpochSuppModeOn )

  if (1 == mess.getGdpbEpSync())
    fHistGet4EpochFlags->Fill(fGet4Nr, 0);
  if (1 == mess.getGdpbEpDataLoss())
    fHistGet4EpochFlags->Fill(fGet4Nr, 1);
  if (1 == mess.getGdpbEpEpochLoss())
    fHistGet4EpochFlags->Fill(fGet4Nr, 2);
  if (1 == mess.getGdpbEpMissmatch())
    fHistGet4EpochFlags->Fill(fGet4Nr, 3);

  fCurrentEpochTime = mess.getMsgFullTime(epochNr);
  fNofEpochs++;

   // Fill Pulser test histos if needed
   if( fbPulserMode && 0 == (fGet4Id % fNrOfGet4PerFeb) // GET4 index in FEET
         && fuPulserFee == (fGet4Nr / fNrOfGet4PerFeb)) // FEET index in full system
   {
      // Fill the time difference for all channels pairs in
      // the chosen Fee
      UInt_t uHistoFeeIdx = 0;
      
      UInt_t uOffsetChip = fuPulserFee * fNrOfGet4PerFeb;
      for( UInt_t uChanFeeA = 0; uChanFeeA < fNrOfChannelsPerFeet; uChanFeeA++)
      {
         UInt_t uChipA = uChanFeeA / fNrOfChannelsPerGet4 + uOffsetChip;
         UInt_t uChanA = uChanFeeA % fNrOfChannelsPerGet4;
         
         for( UInt_t uChanFeeB = uChanFeeA + 1; uChanFeeB < fNrOfChannelsPerFeet; uChanFeeB++)
         {
            UInt_t uChipB = uChanFeeB / fNrOfChannelsPerGet4 + uOffsetChip;
            UInt_t uChanB = uChanFeeB % fNrOfChannelsPerGet4;
            
            Double_t dTimeDiff =
                  fTsLastHit[ fuPulserGdpb ][ uChipB ][ uChanB ] 
                - fTsLastHit[ fuPulserGdpb ][ uChipA ][ uChanA ];
            dTimeDiff *= 1e3;  // ns -> ps
            if( ( 10.0 * dMinDt < dTimeDiff ) && ( dTimeDiff < 10.0 * dMaxDt ) &&
                ( 0 < fTsLastHit[ fuPulserGdpb ][ uChipA ][ uChanA ] ) && 
                ( 0 < fTsLastHit[ fuPulserGdpb ][ uChipB ][ uChanB ] ) )
            {
               fhTimeDiffPulserChosenFee[uHistoFeeIdx]->Fill( dTimeDiff );
            } // if both channels have already 1 hit and these are not too far away
            
            uHistoFeeIdx++;
         } // for( UInt_t uChanFeeB = uChanFeeA + 1; uChanFeeB < fNrOfChannelsPerFeet; uChanFeeB++)
      } // for( UInt_t uChanFeeA = 0; uChanFeeA < fNrOfChannelsPerFeet; uChanFeeA++)

      // Fill the time difference for the chosen channel pairs
      for( UInt_t uChan = 0; uChan < kuNbChanTest - 1; uChan++)
      {
         UInt_t uChipA = fuPulserChan[ uChan     ] / fNrOfChannelsPerGet4;
         UInt_t uChanA = fuPulserChan[ uChan     ] % fNrOfChannelsPerGet4;
         UInt_t uChipB = fuPulserChan[ uChan + 1 ] / fNrOfChannelsPerGet4;
         UInt_t uChanB = fuPulserChan[ uChan + 1 ] % fNrOfChannelsPerGet4;
         
         Double_t dTimeDiff =
               fTsLastHit[0][uChipB][ uChanB ] - fTsLastHit[0][uChipA][ uChanA ];
            dTimeDiff *= 1e3;  // ns -> ps
         if( ( 10.0 * dMinDt < dTimeDiff ) && ( dTimeDiff < 10.0 * dMaxDt ) &&
             ( 0 < fTsLastHit[0][uChipA][ uChanA ] ) && 
             ( 0 < fTsLastHit[0][uChipB][ uChanB ] ) )
         {
            fhTimeDiffPulserChosenChPairs[uChan]->Fill( dTimeDiff );
         } // if both channels have already 1 hit and these are not too far away
      } // for( UInt_t uChan = 0; uChan < kuNbChanTest - 1; uChan++)
   } // if( fbPulserMode && First GET4 on chosen FEET )
   
   
   ///* STAR event building/cutting *///
   if( fbStarSortAndCutMode )
   {
      /// Only use GET4s declared as active
      if( fiStarActiveAsicMask[ fGdpbNr ] & (0x1 << fGet4Id) )
      {
         if( 0 == fuCurrentEpGdpb[fGdpbNr] )
         {
            /// First epoch for this gDPB board => initialize everything
            fuCurrentEpGdpb[ fGdpbNr ] = epochNr;
            fiStarCurrentEpFound[ fGdpbNr ] = 0;
            fiStarBuffIdxPrev[ fGdpbNr ] = 0;
            fiStarBuffIdxCurr[ fGdpbNr ] = 1;
            fiStarBuffIdxNext[ fGdpbNr ] = 2;
            
            // Clear data from before as we are not sure they belong to the epoch before
            fvGdpbEpMsgBuffer[ fGdpbNr ][ fiStarBuffIdxPrev[fGdpbNr] ].clear();
            fvGdpbEpTrgBuffer[ fGdpbNr ][ fiStarBuffIdxPrev[fGdpbNr] ].clear();
         } // if( 0 == fuCurrentEpGdpb[fGdpbNr] )
         else if( epochNr == fuCurrentEpGdpb[ fGdpbNr ] )
         {
            /// Epoch currently waiting for all GET4s to be ready
            /// => This one is ready and starts storing its data in Next buffer
            fiStarCurrentEpFound[ fGdpbNr ] |= 0x1 << fGet4Id; // !!! This implies a limit to 32 GET4 per gDPB !!!
            fiStarNextBufferUse[ fGdpbNr ]  |= 0x1 << fGet4Id; // !!! This implies a limit to 32 GET4 per gDPB !!!
         } // else if( epochNr == fuCurrentEpGdpb[ fGdpbNr ] )
         else if( epochNr == (fuCurrentEpGdpb[ fGdpbNr ] + 1) )
         {
            if( 0 == fiStarCurrentEpFound[ fGdpbNr ] )
            {
               /// First GET4 sending its message for the Epoch currently waiting for all GET4s to be ready
               fuCurrentEpGdpb[ fGdpbNr ] = epochNr;
               /// => This one is ready and starts storing its data in Next buffer
               fiStarCurrentEpFound[ fGdpbNr ] |= 0x1 << fGet4Id; // !!! This implies a limit to 32 GET4 per gDPB !!!
               fiStarNextBufferUse[ fGdpbNr ]  |= 0x1 << fGet4Id; // !!! This implies a limit to 32 GET4 per gDPB !!!
            } // if( 0 == fiStarCurrentEpFound[ fGdpbNr ] )
               else
               {
                  /// Message for next epoch received before all GET4s sent the messages for the current one
                  /// => Either some GET4s are missing or we are in epoch suppressed mode
                  if( !fbEpochSuppModeOn )
                  {
                     /// If not in epoch suppressed mode, print error for the missing GET4s
                     for( Int_t iGet4 = 0; iGet4 < fNrOfGet4PerGdpb; iGet4++)
                        if( !( 0x1 & ( fiStarCurrentEpFound[ fGdpbNr ] >> fGet4Id ) ) )
                        {
                           LOG(ERROR) << "In STAR sort and cut mode, gDPB " << Form("0x%08x,", fGdpbId)
                                      << " found epoch " << Form( "%12u", epochNr) 
                                      << " for get4 "<< Form( "%2u", fGet4Id) 
                                      << " while waiting for epoch " 
                                      << Form( "%12u", fuCurrentEpGdpb[ fGdpbNr ]) 
                                      << " for get4 "<< Form( "%2u", iGet4) 
                                      << FairLogger::endl;
                        } // if( !( 0x1 & ( fiStarCurrentEpFound[ fGdpbNr ] >> fGet4Id ) )
                  } // if( !fbEpochSuppModeOn )
                  
                  /// => In both cases, for processing of current epoch buffer + go to next
                  StarSort( fGdpbNr );
                  StarSelect( fGdpbNr);
                  
                  /// Then update flags and indices to be ready for next epoch
                  fuCurrentEpGdpb[ fGdpbNr ] = epochNr;
                  fiStarCurrentEpFound[ fGdpbNr ] = 0;
                  fiStarBuffIdxPrev[ fGdpbNr ] = (fiStarBuffIdxPrev[ fGdpbNr ] + 1)%3;
                  fiStarBuffIdxCurr[ fGdpbNr ] = (fiStarBuffIdxCurr[ fGdpbNr ] + 1)%3;
                  fiStarBuffIdxNext[ fGdpbNr ] = (fiStarBuffIdxNext[ fGdpbNr ] + 1)%3;
                  
               } // else of if( 0 == fiStarCurrentEpFound[ fGdpbNr ] )
         } // else if( epochNr == (fuCurrentEpGdpb[ fGdpbNr ] + 1) )
         else if( epochNr > (fuCurrentEpGdpb[ fGdpbNr ] + 1) )
         {
            /// Message for later epoch received before all GET4s sent the messages for the current one
            /// => Either some GET4s and/or messages are missing or we are in epoch suppressed mode
            if( !fbEpochSuppModeOn )
            {
               /// If not in epoch suppressed mode, print error for the missing GET4s
               for( Int_t iGet4 = 0; iGet4 < fNrOfGet4PerGdpb; iGet4++)
               {
                  LOG(ERROR) << "In STAR sort and cut mode, gDPB " << Form("0x%08x,", fGdpbId)
                             << " found epoch " << Form( "%12u", epochNr) 
                             << " for get4 "<< Form( "%2u", fGet4Id) 
                             << " while waiting for epoch " 
                             << Form( "%12u", fuCurrentEpGdpb[ fGdpbNr ]) 
                             << " for get4 "<< Form( "%2u", iGet4) 
                             << FairLogger::endl;
               } // for( Int_t iGet4 = 0; iGet4 < fNrOfGet4PerGdpb; iGet4++)
            } // if( !fbEpochSuppModeOn )
            
            /// => In both cases, do processing of current epoch buffer + go to next
            StarSort( fGdpbNr );
            StarSelect( fGdpbNr);
            
            /// => Then go to next (incomplete) buffer
            fiStarBuffIdxPrev[ fGdpbNr ] = (fiStarBuffIdxPrev[ fGdpbNr ] + 1)%3;
            fiStarBuffIdxCurr[ fGdpbNr ] = (fiStarBuffIdxCurr[ fGdpbNr ] + 1)%3;
            fiStarBuffIdxNext[ fGdpbNr ] = (fiStarBuffIdxNext[ fGdpbNr ] + 1)%3;
            
            /// => In both cases, do processing of this epoch buffer also
            StarSort( fGdpbNr );
            StarSelect( fGdpbNr);
            
            /// Then update flags and indices to be ready for next epoch
            fuCurrentEpGdpb[ fGdpbNr ] = epochNr;
            fiStarCurrentEpFound[ fGdpbNr ] = 0;
            fiStarBuffIdxPrev[ fGdpbNr ] = (fiStarBuffIdxPrev[ fGdpbNr ] + 1)%3;
            fiStarBuffIdxCurr[ fGdpbNr ] = (fiStarBuffIdxCurr[ fGdpbNr ] + 1)%3;
            fiStarBuffIdxNext[ fGdpbNr ] = (fiStarBuffIdxNext[ fGdpbNr ] + 1)%3;
         } // else if( epochNr > (fuCurrentEpGdpb[ fGdpbNr ] + 1) )
         else
         {
            LOG(FATAL) << "In STAR sort and cut mode, gDPB " << Form("0x%08x,", fGdpbId)
                       << " found epoch " << Form( "%12u", epochNr) 
                       << " for get4 "<< Form( "%2u", fGet4Id) 
                       << " while waiting for epoch " 
                       << Form( "%12u", fuCurrentEpGdpb[ fGdpbNr ]) 
                       << std::endl 
                       << " -----> GET4 active mask is 0x" << std::hex 
                       << fiStarActiveAsicMask[ fGdpbNr ] << " and GET4 ready flags is "
                       << fiStarCurrentEpFound[ fGdpbNr ]
                       << std::endl
                       << " => corrupted epoch number ordering, exiting now!" 
                       << FairLogger::endl;
         } // else of many ifs => Correspond to epochNr < fuCurrentEpGdpb[ fGdpbNr ]
      } // if( fiStarActiveAsicMask[ fGdpbNr ] & (0x1 << fGet4Id) )
   } // if( fbStarSortAndCutMode )
         
  /*
   LOG(DEBUG) << "Epoch message "
   << fNofEpochs << ", epoch " << static_cast<Int_t>(fCurrentEpoch[gdpbId][get4Id])
   << ", time " << std::setprecision(9) << std::fixed
   << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
   << " for board ID " << std::hex << std::setw(4) << gdpbId << std::dec
   << " and chip " << mess.getEpoch2ChipNumber()
   << FairLogger::endl;
   */
}

void CbmTSMonitorTofStar::PrintSlcInfo(ngdpb::Message mess)
{
  if (fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find(fGdpbId))
    LOG(INFO) << "GET4 Slow Control message, epoch "
                 << static_cast<Int_t>(fCurrentEpoch[fGet4Nr]) << ", time " << std::setprecision(9)
                 << std::fixed << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
                 << " for board ID " << std::hex << std::setw(4) << fGdpbId
                 << std::dec << FairLogger::endl << " +++++++ > Chip = "
                 << std::setw(2) << mess.getGdpbGenChipId() << ", Chan = "
                 << std::setw(1) << mess.getGdpbSlcChan() << ", Edge = "
                 << std::setw(1) << mess.getGdpbSlcEdge() << ", Type = "
                 << std::setw(1) << mess.getGdpbSlcType() << ", Data = "
                 << std::hex << std::setw(6) << mess.getGdpbSlcData()
                 << std::dec << ", Type = " << mess.getGdpbSlcCrc()
                 << FairLogger::endl;

}

void CbmTSMonitorTofStar::PrintGenInfo(ngdpb::Message mess)
{
  Int_t mType = mess.getMessageType();
  Int_t channel = mess.getGdpbHitChanId();
  uint64_t uData = mess.getData();

  if (100 > iMess++)
    LOG(INFO) << "Get4 MSG type " << mType << " from gdpbId " << fGdpbId
                 << ", getId " << fGet4Id << ", (hit channel) " << channel
                 << " data " << std::hex << uData
//Form(" data 0x%01Fx ",uData)
                 << FairLogger::endl;
}

void CbmTSMonitorTofStar::PrintSysInfo(ngdpb::Message mess)
{
  if (fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find(fGdpbId))
    LOG(DEBUG) << "GET4 System message,       epoch "
                  << static_cast<Int_t>(fCurrentEpoch[fGet4Nr]) << ", time " << std::setprecision(9)
                  << std::fixed << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
                  << " for board ID " << std::hex << std::setw(4) << fGdpbId
                  << std::dec << FairLogger::endl;

  switch (mess.getGdpbSysSubType()) {
    case ngdpb::SYSMSG_GET4_EVENT: {
      uint32_t uData = mess.getGdpbSysErrData();
      if (ngdpb::GET4_V1X_ERR_TOT_OVERWRT == uData
          || ngdpb::GET4_V1X_ERR_TOT_RANGE == uData
          || ngdpb::GET4_V1X_ERR_EVT_DISCARD == uData)
        LOG(DEBUG) << " +++++++ > gDPB: " << std::hex << std::setw(4) << fGdpbId
                      << std::dec << ", Chip = " << std::setw(2)
                      << mess.getGdpbGenChipId() << ", Chan = " << std::setw(1)
                      << mess.getGdpbSysErrChanId() << ", Edge = "
                      << std::setw(1) << mess.getGdpbSysErrEdge() << ", Empt = "
                      << std::setw(1) << mess.getGdpbSysErrUnused()
                      << ", Data = " << std::hex << std::setw(2) << uData
                      << std::dec << " -- GET4 V1 Error Event"
                      << FairLogger::endl;
      else
        LOG(INFO) << " +++++++ >gDPB: " << std::hex << std::setw(4) << fGdpbId
                     << std::dec << ", Chip = " << std::setw(2)
                     << mess.getGdpbGenChipId() << ", Chan = " << std::setw(1)
                     << mess.getGdpbSysErrChanId() << ", Edge = "
                     << std::setw(1) << mess.getGdpbSysErrEdge() << ", Empt = "
                     << std::setw(1) << mess.getGdpbSysErrUnused()
                     << ", Data = " << std::hex << std::setw(2) << uData
                     << std::dec << " -- GET4 V1 Error Event"
                     << FairLogger::endl;
      break;
    } // case ngdpb::SYSMSG_GET4_EVENT
    case ngdpb::SYSMSG_CLOSYSYNC_ERROR:
      LOG(INFO) << "Closy synchronization error" << FairLogger::endl;
      break;
    case ngdpb::SYSMSG_TS156_SYNC:
      LOG(DEBUG) << "160.00 MHz timestamp reset" << FairLogger::endl;
      break;
    case ngdpb::SYSMSG_GDPB_UNKWN:
      LOG(INFO) << "Unknown GET4 message, data: " << std::hex << std::setw(8)
                   << mess.getGdpbSysUnkwData() << std::dec << FairLogger::endl;
      break;
  } // switch( getGdpbSysSubType() )
}

void CbmTSMonitorTofStar::FillStarTrigInfo(ngdpb::Message mess)
{
  Int_t iMsgIndex = mess.getStarTrigMsgIndex();
  
  switch( iMsgIndex )
  {
      case 0:
         fulGdpbTsMsb = mess.getGdpbTsMsbStarA();
         break;
      case 1:
         fulGdpbTsLsb = mess.getGdpbTsLsbStarB();
         fulStarTsMsb = mess.getStarTsMsbStarB();
         break;
      case 2:
         fulStarTsMid = mess.getStarTsMidStarC();
         break;
      case 3:
      {  
         ULong64_t ulNewGdpbTsFull = ( fulGdpbTsMsb << 24 )
                           + ( fulGdpbTsLsb       );
         ULong64_t ulNewStarTsFull = ( fulStarTsMsb << 48 )
                           + ( fulStarTsMid <<  8 )
                           + mess.getStarTsLsbStarD();
         UInt_t uNewToken  = mess.getStarTokenStarD();
         UInt_t uNewDaqCmd  = mess.getStarDaqCmdStarD();
         UInt_t uNewTrigCmd = mess.getStarTrigCmdStarD();

       
		 if( (uNewToken != fuStarTokenLast + 1) && 0 < fulGdpbTsFullLast && 0 < fulStarTsFullLast )
			LOG(WARNING) << "Possible error: STAR token did not increase by exactly 1! " 
			             << Form("old = %5u vs new = %5u ", fuStarTokenLast,   uNewToken)
			             << Form("old = %12lu vs new = %12lu ", fulGdpbTsFullLast, ulNewGdpbTsFull)
			             << Form("old = %12lu vs new = %12lu ", fulStarTsFullLast, ulNewStarTsFull)
			             << Form("old = %2u vs new = %2u ", fuStarDaqCmdLast,  uNewDaqCmd)
			             << Form("old = %2u vs new = %2u ", fuStarTrigCmdLast, uNewTrigCmd)
                         << FairLogger::endl;

      
         fulGdpbTsFullLast = ulNewGdpbTsFull;
         fulStarTsFullLast = ulNewStarTsFull;
         fuStarTokenLast   = uNewToken;
         fuStarDaqCmdLast  = uNewDaqCmd;
         fuStarTrigCmdLast = uNewTrigCmd;
         

         ///* STAR event building/cutting *///
         if( fbStarSortAndCutMode )
         {
            CbmTofStarTrigger newTrig( fulGdpbTsFullLast, fulStarTsFullLast, fuStarTokenLast, 
                                       fuStarDaqCmdLast, fuStarTrigCmdLast );
            fvGdpbEpTrgBuffer[ fGdpbNr ][ fiStarBuffIdxCurr[fGdpbNr] ].push_back( newTrig );
                    
                    
            /// => For testing, generate imediately sub-event
            CbmTofStarSubevent subEvent( newTrig );
            
            // Add fake data messages for the time being: 33 messages of 2 words, counting from 42 ;)
            for( ULong64_t uFakeData = 0; uFakeData < 33; uFakeData++ )
            {
               ngdpb::Message msg( uFakeData + 42 );
               subEvent.AddMsg( msg );
            } // for( ULong64_t uFakeData = 0; uFakeData < 33; uFakeData++ )
            
            /*
             ** Function to send sub-event block to the STAR DAQ system
             *       trg_word received is packed as:
             *
             *       trg_cmd|daq_cmd|tkn_hi|tkn_mid|tkn_lo
             */
            /** TODO: clarify how we deal with multiple sub-events (eg one for each gDPB) **/
            Int_t  iBuffSzByte = 0;
            void * pDataBuff = subEvent.BuildOutput( iBuffSzByte );
#ifdef STAR_SUBEVT_BUILDER
            star_rhicf_write( subEvent.GetTrigger().GetStarTrigerWord(), 
                              pDataBuff, iBuffSzByte );
#endif // STAR_SUBEVT_BUILDER

            LOG(INFO) << "Sent STAR event with size " << iBuffSzByte << " Bytes"
                       << FairLogger::endl;
         } // if( fbStarSortAndCutMode )
         
         if( 0 <= fdStartTime )
         {
            fhTriggerRate->Fill(
               1e-9 * ( fulGdpbTsFullLast * 6.25 - fdStartTime ) );
            fhStarTokenEvo->Fill(
               1e-9 * ( fulGdpbTsFullLast * 6.25 - fdStartTime ),
               fuStarTokenLast );
		   } // if( 0 < fdStartTime )
            else fdStartTime = fulGdpbTsFullLast * 6.25;
         fhCmdDaqVsTrig->Fill( fuStarDaqCmdLast, fuStarTrigCmdLast );

         LOG(INFO) << "Found full Star Trigger with gDPB TS " << Form("%16llu", fulGdpbTsFullLast)
                    << " STAR TS " << Form("%16llu", fulStarTsFullLast)
                    << " token " << Form("%8u", fuStarTokenLast)
                    << " DAQ CMD " << Form("%1X", fuStarDaqCmdLast)
                    << " TRIG CMD " << Form("%1X", fuStarTrigCmdLast)
                    << " Filler " << Form("%2X", mess.getStarFillerD())
                    << FairLogger::endl;

         break;
	  } // case 3
      default:
         LOG(FATAL) << "Unknown Star Trigger messageindex: " << iMsgIndex << FairLogger::endl;
  } // switch( iMsgIndex )
}

void CbmTSMonitorTofStar::Reset()
{
}

void CbmTSMonitorTofStar::Finish()
{
  TString message_type;

  for (unsigned int i = 0; i < fMsgCounter.size(); ++i) {
    switch (i) {
      case 0:
        message_type = "NOP";
        break;
      case 1:
        message_type = "HIT";
        break;
      case 2:
        message_type = "EPOCH";
        break;
      case 3:
        message_type = "SYNC";
        break;
      case 4:
        message_type = "AUX";
        break;
      case 5:
        message_type = "EPOCH2";
        break;
      case 6:
        message_type = "GET4";
        break;
      case 7:
        message_type = "SYS";
        break;
      case 8:
        message_type = "GET4_SLC";
        break;
      case 9:
        message_type = "GET4_32B";
        break;
      case 10:
        message_type = "GET4_SYS";
        break;
      default:
        message_type = "UNKNOWN";
        break;
    } // switch(i)
    LOG(INFO) << message_type << " messages: " << fMsgCounter[i]
                 << FairLogger::endl;
  } // for (unsigned int i=0; i< fMsgCounter.size(); ++i)

  LOG(INFO) << "-------------------------------------" << FairLogger::endl;
  for (Int_t i = 0; i < fNrOfGdpbs; ++i) {
    for (Int_t j = 0; j < fNrOfGet4PerGdpb; ++j) {
      LOG(INFO) << "Last epoch for gDPB: " << std::hex << std::setw(4) << i
                   << std::dec << " , GET4  " << std::setw(4) << j << " => "
                   << fCurrentEpoch[GetArrayIndex(i, j)] << FairLogger::endl;
    }
  }
  LOG(INFO) << "-------------------------------------" << FairLogger::endl;

  gDirectory->mkdir("Tof_Raw_gDPB");
  gDirectory->cd("Tof_Raw_gDPB");
  for (UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb++) {
//    fHM->H2(Form("Raw_Tot_gDPB_%02u", uGdpb))->Write();
    fHM->H1(Form("ChCount_gDPB_%02u", uGdpb))->Write();
//    if (fUnpackPar->IsChannelRateEnabled())
//      fHM->H2(Form("ChannelRate_gDPB_%02u", uGdpb))->Write();
    for (UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; uFeet++) {
      fHM->H1(Form("FeetRate_gDPB_g%02u_f%1u", uGdpb, uFeet))->Write();
      if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec ) {
        fHM->H1(Form("FeetRateDate_gDPB_g%02u_f%1u", uGdpb, uFeet))->Write();
      }
    }
  } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
  fHM->H1("hMessageType")->Write();
  fHM->H1("hSysMessType")->Write();
  fHM->H2("hGet4MessType")->Write();
  fHM->H2("hGet4ChanErrors")->Write();
  fHM->H2("hGet4EpochFlags")->Write();
  fHM->H1("hDSpillLength")->Write();
  fHM->H1("hSpillCount")->Write();
  fHM->H1("hSpillQA")->Write();

  fHM->H1("hTriggerRate")->Write();
  fHM->H1("hCmdDaqVsTrig")->Write();
  
  gDirectory->cd("..");

  gDirectory->mkdir("Flib_Raw");
  gDirectory->cd("Flib_Raw");
  for (UInt_t uLinks = 0; uLinks < 16; uLinks++) {
    TString sMsSzName = Form("MsSz_link_%02u", uLinks);
    if (fHM->Exists(sMsSzName.Data()))
      fHM->H1(sMsSzName.Data())->Write();

    sMsSzName = Form("MsSzTime_link_%02u", uLinks);
    if (fHM->Exists(sMsSzName.Data()))
      fHM->P1(sMsSzName.Data())->Write();
  } // for( UInt_t uLinks = 0; uLinks < 16; uLinks ++)
  gDirectory->cd("..");

}

void CbmTSMonitorTofStar::FillOutput(CbmDigi* /*digi*/)
{
}

void CbmTSMonitorTofStar::ResetAllHistos()
{
  LOG(INFO) << "Reseting all TOF histograms." << FairLogger::endl;
  fHM->H1("hMessageType")->Reset();
  fHM->H1("hSysMessType")->Reset();
  fHM->H2("hGet4MessType")->Reset();
  fHM->H2("hGet4ChanErrors")->Reset();
  fHM->H2("hGet4EpochFlags")->Reset();
  fHM->H1("hSpillLength")->Reset();
  fHM->H1("hSpillCount")->Reset();
  fHM->H2("hSpillQA")->Reset();
  fSpillIdx = 0;

  for (UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb++) {
    fHM->H1(Form("ChCount_gDPB_%02u", uGdpb))->Reset();
    
    for (UInt_t uFeetPlot = 0; uFeetPlot < fNrOfFebsPerGdpb/uNbFeetPlot; ++uFeetPlot )
      fHM->H2(Form("Raw_Tot_gDPB_%02u_%1u", uGdpb, uFeetPlot))->Reset();
      
//    if (fUnpackPar->IsChannelRateEnabled())
//      fHM->H2(Form("ChannelRate_gDPB_%02u", uGdpb))->Reset();
    for (UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; uFeet++) {
      fHM->H1(Form("FeetRate_gDPB_g%02u_f%1u", uGdpb, uFeet))->Reset();
    } // for( UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; uFeet ++)
  } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
  
  
  fHM->H1("hTriggerRate")->Reset();
  fHM->H1("hCmdDaqVsTrig")->Reset();
  fHM->H1("hStarTokenEvo")->Reset();
  

  for (UInt_t uLinks = 0; uLinks < 16; uLinks++) {
    TString sMsSzName = Form("MsSz_link_%02u", uLinks);
    if (fHM->Exists(sMsSzName.Data()))
      fHM->H1(sMsSzName.Data())->Reset();

    sMsSzName = Form("MsSzTime_link_%02u", uLinks);
    if (fHM->Exists(sMsSzName.Data()))
      fHM->P1(sMsSzName.Data())->Reset();
  } // for( UInt_t uLinks = 0; uLinks < 16; uLinks ++)
  
  if( fbPulserMode )
  {
     fHM->H2("hTimeRmsPulserChosenFee")->Reset();
     fHM->H1("hTimeRmsPulserChosenChPairs")->Reset();
     fdLastRmsUpdateTime = -1;
  
      // Full Fee time difference test
      UInt_t uHistoFeeIdx = 0;
      for( UInt_t uChanFeeA = 0; uChanFeeA < fNrOfChannelsPerFeet; uChanFeeA++)
      {
         for( UInt_t uChanFeeB = uChanFeeA + 1; uChanFeeB < fNrOfChannelsPerFeet; uChanFeeB++)
         {
            fhTimeDiffPulserChosenFee[uHistoFeeIdx]->Reset();
            uHistoFeeIdx++;
         } // for any unique pair of channel in chosen Fee
      } // for( UInt_t uChanFeeA = 0; uChanFeeA < kuNbChanFee; uChanFeeA++)
      
      // Selected channels test
      for( UInt_t uChan = 0; uChan < kuNbChanTest - 1; uChan++)
         fhTimeDiffPulserChosenChPairs[uChan]->Reset();
  } // if( fbPulserMode )

  fdStartTime = -1;
  fdStartTimeMsSz = -1;
}
void CbmTSMonitorTofStar::CyclePulserFee()
{
  if( fbPulserMode )
  {
     fHM->H2("hTimeRmsPulserChosenFee")->Reset();
     fdLastRmsUpdateTime = -1;
  
      // Full Fee time difference test
      UInt_t uHistoFeeIdx = 0;
      for( UInt_t uChanFeeA = 0; uChanFeeA < fNrOfChannelsPerFeet; uChanFeeA++)
      {
         for( UInt_t uChanFeeB = uChanFeeA + 1; uChanFeeB < fNrOfChannelsPerFeet; uChanFeeB++)
         {
            fhTimeDiffPulserChosenFee[uHistoFeeIdx]->Reset();
            uHistoFeeIdx++;
         } // for any unique pair of channel in chosen Fee
      } // for( UInt_t uChanFeeA = 0; uChanFeeA < kuNbChanFee; uChanFeeA++)
      
      // Cycle the chosen FEE index within the current chosen gDPB
      fuPulserFee = (fuPulserFee + 1)%fNrOfFebsPerGdpb;
      
      LOG(INFO) << "FEE index for pulser RMS histo changed to : " << fuPulserFee << FairLogger::endl;
  } // if( fbPulserMode )
}


void CbmTSMonitorTofStar::SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize )
{
   TDatime * fRunStartDateTime     = new TDatime( dateIn, timeIn);
   fiRunStartDateTimeSec = fRunStartDateTime->Convert();
   fiBinSizeDatePlots    = iBinSize;
   
   LOG(INFO) << "Assigned new TOF Run Start Date-Time: " << fRunStartDateTime->AsString() << FairLogger::endl;
}

void CbmTSMonitorTofStar::SetPulserChans(
      UInt_t inPulserChanA, UInt_t inPulserChanB, UInt_t inPulserChanC, UInt_t inPulserChanD,
      UInt_t inPulserChanE, UInt_t inPulserChanF, UInt_t inPulserChanG, UInt_t inPulserChanH,
      UInt_t inPulserChanI, UInt_t inPulserChanJ, UInt_t inPulserChanK, UInt_t inPulserChanL,
      UInt_t inPulserChanM, UInt_t inPulserChanN, UInt_t inPulserChanO, UInt_t inPulserChanP )
{
   fuPulserChan[ 0] = inPulserChanA;
   fuPulserChan[ 1] = inPulserChanB;
   fuPulserChan[ 2] = inPulserChanC;
   fuPulserChan[ 3] = inPulserChanD;
   fuPulserChan[ 4] = inPulserChanE;
   fuPulserChan[ 5] = inPulserChanF;
   fuPulserChan[ 6] = inPulserChanG;
   fuPulserChan[ 7] = inPulserChanH;
   fuPulserChan[ 8] = inPulserChanI;
   fuPulserChan[ 9] = inPulserChanJ;
   fuPulserChan[10] = inPulserChanK;
   fuPulserChan[11] = inPulserChanL;
   fuPulserChan[12] = inPulserChanM;
   fuPulserChan[13] = inPulserChanN;
   fuPulserChan[14] = inPulserChanO;
   fuPulserChan[15] = inPulserChanP;
}

    ///* STAR event building/cutting *///
Bool_t CbmTSMonitorTofStar::StarSort( Int_t iGdpbIdx )
{
   // Sort the messages in current buffer relative to time
   std::stable_sort( fvGdpbEpMsgBuffer[ iGdpbIdx ][ fiStarBuffIdxCurr[ iGdpbIdx ] ].begin(), 
                     fvGdpbEpMsgBuffer[ iGdpbIdx ][ fiStarBuffIdxCurr[ iGdpbIdx ] ].end() );
                     
   // Sort the triggers in current buffer relative to time
   std::stable_sort( fvGdpbEpTrgBuffer[ iGdpbIdx ][ fiStarBuffIdxCurr[ iGdpbIdx ] ].begin(), 
                     fvGdpbEpTrgBuffer[ iGdpbIdx ][ fiStarBuffIdxCurr[ iGdpbIdx ] ].end() );
   
   return kTRUE;
}
Bool_t CbmTSMonitorTofStar::StarSelect( Int_t iGdpbIdx )
{
   /** Read the triggers one by one and for each scan the messages until one outside of the 
    ** event window is found
    **/
//   /**** Start with the remaining trigger in the previous buffer (ones that could have data in current) ****/
//   for( UInt_t uTrigIdx = 0; 
//        uTrigIdx < fvGdpbEpTrgBuffer[ iGdpbIdx ][ fiStarBuffIdxPrev[ iGdpbIdx ] ].size();
//        uTrigIdx ++)
//   {
//      ULong64_t ulTriggerTime = fvGdpbEpTrgBuffer[ iGdpbIdx ][ fiStarBuffIdxPrev[ iGdpbIdx ] ][uTrigIdx].GetFullGdpbTs();
//      
//   } // Loop on remaining triggers in previous buffer
   /**** Then process all triggers in the current buffer (ones that could have data in current) ****/
   for( UInt_t uTrigIdx = 0; 
        uTrigIdx < fvGdpbEpTrgBuffer[ iGdpbIdx ][ fiStarBuffIdxCurr[ iGdpbIdx ] ].size();
        uTrigIdx ++)
   {
      CbmTofStarSubevent subEvent( fvGdpbEpTrgBuffer[ iGdpbIdx ][ fiStarBuffIdxCurr[ iGdpbIdx ] ][uTrigIdx] );
      
      // Add fake data messages for the time being: 33 messages of 2 words, counting from 42 ;)
      for( ULong64_t uFakeData = 0; uFakeData < 33; uFakeData++ )
      {
         ngdpb::Message msg( uFakeData + 42 );
         subEvent.AddMsg( msg );
      } // for( ULong64_t uFakeData = 0; uFakeData < 33; uFakeData++ )
      
      /*
       ** Function to send sub-event block to the STAR DAQ system
       *       trg_word received is packed as:
       *
       *       trg_cmd|daq_cmd|tkn_hi|tkn_mid|tkn_lo
       */
      /** TODO: clarify how we deal with multiple sub-events (eg one for each gDPB) **/
      Int_t  iBuffSzByte = 0;
      void * pDataBuff = subEvent.BuildOutput( iBuffSzByte );
#ifdef STAR_SUBEVT_BUILDER
      star_rhicf_write( subEvent.GetTrigger().GetStarTrigerWord(), 
                        pDataBuff, iBuffSzByte );
#endif // STAR_SUBEVT_BUILDER
   } // Loop on triggers in current buffer
    
   
   /// Data from the Prev buffer needed only for triggers in beginning of Current
   /// Triggers from the Prev buffer needed only until Current buffer is ready
   /// => Both ok, clear the Prev buffer so that it can be re-used of Next buffer later
   fvGdpbEpMsgBuffer[ iGdpbIdx ][ fiStarBuffIdxPrev[ iGdpbIdx ] ].clear();
   fvGdpbEpTrgBuffer[ iGdpbIdx ][ fiStarBuffIdxPrev[ iGdpbIdx ] ].clear();

   return kTRUE;
}

ClassImp(CbmTSMonitorTofStar)
