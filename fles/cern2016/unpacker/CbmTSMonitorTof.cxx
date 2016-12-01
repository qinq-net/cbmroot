// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSMonitorTof                            -----
// -----               Created 27.10.2016 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTSMonitorTof.h"
#include "CbmTofUnpackPar.h"
//#include "CbmTofDigiExp.h"

//#include "CbmTbDaqBuffer.h"

//#include "CbmFiberHodoAddress.h"
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

#include <iostream>
#include <stdint.h>
#include <iomanip>

static Int_t iMess = 0;
Bool_t bResetTofHistos = kFALSE;


CbmTSMonitorTof::CbmTSMonitorTof() :
    CbmTSUnpack(),
    fuMsAcceptsPercent(100),
    fuMinNbGdpb(),
    fNrOfGdpbs(-1),
    fNrOfFebsPerGdpb(-1),
    fNrOfGet4PerFeb(-1),
    fNrOfChannelsPerGet4(-1),
    fNrOfGet4(-1),
    fNrOfGet4PerGdpb(-1),
    fDiamondGdpb(-1),
    fDiamondFeet(-1),
    fDiamondChanA(-1),
    fDiamondChanB(-1),
    fDiamondChanC(-1),
    fDiamondChanD(-1),
    fDiamondTimeLastReset(-1),
    fiDiamCountsLastTs(0),
    fiDiamSpillOnThr(10), 
    fiDiamSpillOffThr(3),
    fiTsUnderOff(0),
    fiTsUnderOffThr(10),
    fdDiamondLastTime(-1.),
    fdDiamondTimeLastTs(-1.),
    fbSpillOn(kFALSE),
    fuCurrNbGdpb(0),
    fMsgCounter(11, 0), // length of enum MessageTypes initialized with 0
    fGdpbIdIndexMap(),
    fHM(new CbmHistManager()),
    fCurrentEpoch(NULL),
    fTsLastHit(),
    fNofEpochs(0),
    fCurrentEpochTime(0.),
    fdStartTime(-1.),
    fdStartTimeMsSz(-1.),
    fcMsSizeAll(NULL),
    fEquipmentId(0),
    fUnpackPar(NULL)
{
}

CbmTSMonitorTof::~CbmTSMonitorTof()
{
  delete fHM; //TODO: Who deletes the histograms stored in the CbmHistManager???
  delete[] fCurrentEpoch;
}

Bool_t CbmTSMonitorTof::Init()
{
  LOG(INFO) << "Initializing Get4 monitor" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  return kTRUE;
}

void CbmTSMonitorTof::SetParContainers()
{
  LOG(INFO) << "Setting parameter containers for " << GetName()
               << FairLogger::endl;
  fUnpackPar =
      (CbmTofUnpackPar*) (FairRun::Instance()->GetRuntimeDb()->getContainer(
          "CbmTofUnpackPar"));

}

Bool_t CbmTSMonitorTof::InitContainers()
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

Bool_t CbmTSMonitorTof::ReInitContainers()
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

  return kTRUE;
}

void CbmTSMonitorTof::CreateHistograms()
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
      name = Form("ChannelRate_gDPB_%02u", uGdpb);
      title = Form("Channel instant rate gDPB %02u; Dist[ns] ; Channel", uGdpb);
      fHM->Add(name.Data(),
          new TH2F(name.Data(), title.Data(), iNbBinsRate - 1, dBinsRate, 96, 0,
              96));
#ifdef USE_HTTP_SERVER
      if (server)
        server->Register("/TofRaw", fHM->H2(name.Data()));
#endif
      LOG(INFO) << "Adding the rate histos" << FairLogger::endl;
    } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
  }

  for (UInt_t uGdpb = 0; uGdpb < fNrOfGdpbs; uGdpb++) {
    name = Form("Raw_Tot_gDPB_%02u", uGdpb);
    title = Form("Raw TOT gDPB %02u; channel; TOT [bin]", uGdpb);
    fHM->Add(name.Data(),
        new TH2F(name.Data(), title.Data(), 96, 0, 96, 256, 0, 256));
#ifdef USE_HTTP_SERVER
    if (server)
      server->Register("/TofRaw", fHM->H2(name.Data()));
#endif

    name = Form("ChCount_gDPB_%02u", uGdpb);
    title = Form("Channel counts gDPB %02u; channel; Hits", uGdpb);
    fHM->Add(name.Data(), new TH1I(name.Data(), title.Data(), 96, 0, 96));

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
    } // for( UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; uFeet ++)
  } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)

  name = "hDiamond";
  title = "Counts per diamond in last 10s; X [pad]; Y [pad]; Counts";
  TH2I* hDiamond = new TH2I(name, title, 2, 0., 2, 2, 0., 2.);
  fHM->Add(name.Data(), hDiamond);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H2(name.Data()));
#endif

  name = "hDiamondSpill";
  title = "Counts per diamond in Current Spill; X [pad]; Y [pad]; Counts";
  TH2I* hDiamondSpill = new TH2I(name, title, 2, 0., 2, 2, 0., 2.);
  fHM->Add(name.Data(), hDiamondSpill);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H2(name.Data()));
#endif

  name = "hDiamondSpillLength";
  title = "Length of spill interval as found from diamond; Length [s]; Counts";
  TH1* hDiamondSpillLength = new TH1F(name, title, 3000, 0., 300.);
  fHM->Add(name.Data(), hDiamondSpillLength);
#ifdef USE_HTTP_SERVER
  if (server)
    server->Register("/TofRaw", fHM->H1(name.Data()));
#endif

#ifdef USE_HTTP_SERVER
  if (server)
    server->RegisterCommand("/Reset_All_TOF", "bResetTofHistos=kTRUE");
  if (server)
    server->Restrict("/Reset_All_TOF", "allow=admin");
#endif

  /** Create summary Canvases for CERN 2016 **/
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
  hDiamondSpill->Draw("col text");
  
  
  /** Create FEET rates Canvase for CERN 2016 **/
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
   
  /** Recovers/Create Ms Size Canvase for CERN 2016 **/  
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

  /** Create beam tuning Canvases for CERN 2016 **/
  TCanvas* cBeamTunning = new TCanvas("cBeamTunning", "USTC D1 rate + DIam Rate + Diam centering", w, h);
  cBeamTunning->Divide(2, 2);
  
  gStyle->Reset();

  cBeamTunning->cd(1);
  histPnt = fHM->H1( "FeetRate_gDPB_g00_f0" );
  gPad->SetLogy();
  histPnt->Draw();

  cBeamTunning->cd(2);
  histPnt = fHM->H1( "FeetRate_gDPB_g00_f2" );
  gPad->SetLogy();
  histPnt->Draw();
  
  cBeamTunning->cd(3);
  hDiamond->Draw("col text");

  cBeamTunning->cd(4);
  hDiamondSpill->Draw("col text");
  /*****************************/


  fHistMessType = fHM->H1("hMessageType");
  fHistSysMessType = fHM->H1("hSysMessType");
  fHistGet4MessType = fHM->H2("hGet4MessType");
  fHistGet4ChanErrors = fHM->H2("hGet4ChanErrors");
  fHistGet4EpochFlags = fHM->H2("hGet4EpochFlags");
  fHistDiamond = fHM->H2("hDiamond");
  fHistDiamondSpill = fHM->H2("hDiamondSpill");
  fHistDiamondSpillLength = fHM->H1("hDiamondSpillLength");

  for (Int_t i = 0; i < fNrOfGdpbs; ++i) {
    name = Form("Raw_Tot_gDPB_%02u", i);
    fRaw_Tot_gDPB.push_back(fHM->H2(name.Data()));
    name = Form("ChCount_gDPB_%02u", i);
    fChCount_gDPB.push_back(fHM->H1(name.Data()));
    name = Form("ChannelRate_gDPB_%02u", i);
    if (fUnpackPar->IsChannelRateEnabled()) {
      fChannelRate_gDPB.push_back(fHM->H2(name.Data()));
    }
    for (UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; uFeet++) {
      name = Form("FeetRate_gDPB_g%02u_f%1u", i, uFeet);
      fFeetRate_gDPB.push_back(fHM->H1(name.Data()));
    } // for( UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; uFeet ++)
  }
  /*****************************/
  
  LOG(INFO) << "Leaving CreateHistograms" << FairLogger::endl;
}

Bool_t CbmTSMonitorTof::DoUnpack(const fles::Timeslice& ts,
    size_t component)
{
#ifdef USE_HTTP_SERVER
  THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
#endif

  if (bResetTofHistos) {
    ResetAllHistos();
    bResetTofHistos = kFALSE;
  }

  LOG(DEBUG1) << "Timeslice contains " << ts.num_microslices(component)
                 << "microslices." << FairLogger::endl;

  // Getting the pointer to the correct histogram needs a lot more time then
  // the actual filling procedure. If one gets the pointer in a loop this kills
  // the performance. A test shows that extracting the pointer from the CbmHistManger
  // is slower by a factor of 20-100 (depending on the number of histos managed by the
  // CbmHistManager) compared to using the pointer directly
  // So get the pointer once outside the loop and use it in the loop

  TString sMsSzName = Form("MsSz_link_%02u", component);
  TH1* hMsSz = NULL;
  TProfile* hMsSzTime = NULL;
  if (fHM->Exists(sMsSzName.Data())) {
    hMsSz = fHM->H1(sMsSzName.Data());
    sMsSzName = Form("MsSzTime_link_%02u", component);
    hMsSzTime = fHM->P1(sMsSzName.Data());
  } // if( fHM->Exists(sMsSzName.Data() ) )
  else {
    TString sMsSzTitle = Form(
        "Size of MS for gDPB of link %02u; Ms Size [bytes]", component);
    fHM->Add(sMsSzName.Data(),
        new TH1F(sMsSzName.Data(), sMsSzTitle.Data(), 160000, 0., 20000.));
    hMsSz = fHM->H1(sMsSzName.Data());
#ifdef USE_HTTP_SERVER
    if (server)
      server->Register("/FlibRaw", hMsSz);
#endif
    sMsSzName = Form("MsSzTime_link_%02u", component);
    sMsSzTitle = Form(
        "Size of MS vs time for gDPB of link %02u; Time[s] ; Ms Size [bytes]",
        component);
    fHM->Add(sMsSzName.Data(),
        new TProfile(sMsSzName.Data(), sMsSzTitle.Data(), 15000, 0., 300.));
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
  fiDiamCountsLastTs = 0;

  Int_t messageType = -111;
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

      if (gLogger->IsLogNeeded(DEBUG)) {
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
          FillHitInfo(mess);
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

  // Spill detection using Diamond
  if( !fbSpillOn && (fiDiamSpillOnThr < fiDiamCountsLastTs) )
  {
    fbSpillOn = kTRUE;
    fHistDiamondSpill->Reset();
    fiTsUnderOff = 0;
    
    if( 0 < fdDiamondTimeLastTs )
      fHistDiamondSpillLength->Fill( fdDiamondLastTime - fdDiamondTimeLastTs );
    fdDiamondTimeLastTs = fdDiamondLastTime;
  } // if( !fbSpillOn && (fiDiamSpillOnThr < fiDiamCountsLastTs) )
  else if( fbSpillOn  )
  {
    if( fiDiamCountsLastTs < fiDiamSpillOffThr )
    {
      fiTsUnderOff ++;
      if( fiTsUnderOffThr < fiTsUnderOff )
        fbSpillOn = kFALSE;
    } //  if( fiDiamCountsLastTs < fiDiamSpillOffThr )
      else fiTsUnderOff = 0;
  } // else if( fbSpillOn  )

  return kTRUE;
}

void CbmTSMonitorTof::FillHitInfo(ngdpb::Message mess)
{
  Int_t channel = mess.getGdpbHitChanId();
  Int_t tot = mess.getGdpbHit32Tot();
  ULong_t hitTime = mess.getMsgFullTime(0);

  Int_t curEpochGdpbGet4 = fCurrentEpoch[fGet4Nr];
  if (curEpochGdpbGet4 != -111) {

    Int_t channelNr = fGet4Id * fNrOfChannelsPerGet4 + channel;
    fRaw_Tot_gDPB[fGdpbNr]->Fill(channelNr, tot);
    fChCount_gDPB[fGdpbNr]->Fill(channelNr);

    if (fUnpackPar->IsChannelRateEnabled()) {
      // Check if at least one hit before in this gDPB
      if (fTsLastHit.end() != fTsLastHit.find(fGdpbNr)) {
        // Check if at least one hit before in this Get4
        if (fTsLastHit[fGdpbNr].end() != fTsLastHit[fGdpbNr].find(fGet4Id)) {
          // Check if at least one hit before in this channel
          if (fTsLastHit[fGdpbNr][fGet4Id].end()
              != fTsLastHit[fGdpbNr][fGet4Id].find(channel)) {
            fChannelRate_gDPB[fGdpbNr]->Fill(
                1e9
                    / (mess.getMsgFullTimeD(curEpochGdpbGet4)
                        - fTsLastHit[fGdpbNr][fGet4Id][channel]),
                fGet4Id * fNrOfChannelsPerGet4 + channel);
          } // if( fTsLastHit[gdpbId][get4Id].end() != fTsLastHit[gdpbId][get4Id].find( channel ) )
        } // if( fTsLastHit[gdpbId].end() != fTsLastHit[gdpbId].find( get4Id ) )
      } // if( fTsLastHit.end() != fTsLastHit.find( gdpbId ) )

      fTsLastHit[fGdpbNr][fGet4Id][channel] = mess.getMsgFullTimeD(
          curEpochGdpbGet4);
    } // if( fUnpackPar->IsChannelRateEnabled() )

    if (fdStartTime < 0)
      fdStartTime = mess.getMsgFullTimeD(curEpochGdpbGet4);

    if (0 < fdStartTime)
      fFeetRate_gDPB[(fGdpbNr * fNrOfFebsPerGdpb) + (fGet4Id / fNrOfGet4PerFeb)]->Fill(
          1e-9 * (mess.getMsgFullTimeD(curEpochGdpbGet4) - fdStartTime));

     if (fDiamondGdpb == fGdpbNr
         && (fGet4Id / fNrOfGet4PerFeb == fDiamondFeet)) {
       Int_t iChanInGdpb = fGet4Id * fNrOfChannelsPerGet4 + channel;
       if (fDiamondChanA == iChanInGdpb)
       {
         fHistDiamond->Fill(0., 0.);
         fHistDiamondSpill->Fill(0., 0.);
       } // if (fDiamondChanA == iChanInGdpb)
       else if (fDiamondChanB == iChanInGdpb)
       {
         fHistDiamond->Fill(1., 0.);
         fHistDiamondSpill->Fill(1., 0.);
       } // else if (fDiamondChanB == iChanInGdpb)
       else if (fDiamondChanC == iChanInGdpb)
       {
         fHistDiamond->Fill(0., 1.);
         fHistDiamondSpill->Fill(0., 1.);
       } // else if (fDiamondChanC == iChanInGdpb)
       else if (fDiamondChanD == iChanInGdpb)
       {
         fHistDiamond->Fill(1., 1.);
         fHistDiamondSpill->Fill(1., 1.);
       } // else if (fDiamondChanD == iChanInGdpb)
         
       if( fDiamondChanA == iChanInGdpb || fDiamondChanB == iChanInGdpb ||
           fDiamondChanC == iChanInGdpb || fDiamondChanD == iChanInGdpb )
       {
         fiDiamCountsLastTs ++;
         fdDiamondLastTime = 1e-9 * mess.getMsgFullTimeD(curEpochGdpbGet4);
       } // if channel match one of the diamond ones

     } // if( fDiamondGdpb == gdpbNr && ( get4Id/fNrOfGet4PerFeb == fDiamondFeet ) )

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
    LOG(WARNING) << "found unmapped gdpbId w/o epoch yet: "
                    << Form("0x%08x ", fGdpbId) << FairLogger::endl;
}

void CbmTSMonitorTof::FillEpochInfo(ngdpb::Message mess)
{
  Int_t epochNr = mess.getEpoch2Number();
  fCurrentEpoch[fGet4Nr] = epochNr;

  //  LOG(INFO) << "Epoch message for ROC " << gdpbId << " with epoch number "
  //            << fCurrentEpoch[gdpbId] << FairLogger::endl;

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
  

  if (fDiamondGdpb == fGdpbNr
      && (fGet4Id / fNrOfGet4PerFeb == fDiamondFeet)) {
    if (10
        < ((mess.getMsgFullTimeD(epochNr) / 1e9)
            - fDiamondTimeLastReset)) {
      fHistDiamond->Reset();
      fDiamondTimeLastReset = mess.getMsgFullTimeD(epochNr) / 1e9;
    } // if( 1e10 < ( mess.getMsgFullTimeD( fCurrentEpoch[rocId][get4Id] ) - fDiamondTimeLastReset )
  } // if( fDiamondGdpb == gdpbNr && ( get4Id/fNrOfGet4PerFeb == fDiamondFeet ) )

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

void CbmTSMonitorTof::PrintSlcInfo(ngdpb::Message mess)
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

void CbmTSMonitorTof::PrintGenInfo(ngdpb::Message mess)
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

void CbmTSMonitorTof::PrintSysInfo(ngdpb::Message mess)
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
      LOG(DEBUG) << "156.25MHz timestamp reset" << FairLogger::endl;
      break;
    case ngdpb::SYSMSG_GDPB_UNKWN:
      LOG(INFO) << "Unknown GET4 message, data: " << std::hex << std::setw(8)
                   << mess.getGdpbSysUnkwData() << std::dec << FairLogger::endl;
      break;
  } // switch( getGdpbSysSubType() )
}

void CbmTSMonitorTof::Reset()
{
}

void CbmTSMonitorTof::Finish()
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
    fHM->H2(Form("Raw_Tot_gDPB_%02u", uGdpb))->Write();
    fHM->H1(Form("ChCount_gDPB_%02u", uGdpb))->Write();
    if (fUnpackPar->IsChannelRateEnabled())
      fHM->H2(Form("ChannelRate_gDPB_%02u", uGdpb))->Write();
    for (UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; uFeet++) {
      fHM->H1(Form("FeetRate_gDPB_g%02u_f%1u", uGdpb, uFeet))->Write();
    }
  } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
  fHM->H1("hMessageType")->Write();
  fHM->H1("hSysMessType")->Write();
  fHM->H2("hGet4MessType")->Write();
  fHM->H2("hGet4ChanErrors")->Write();
  fHM->H2("hGet4EpochFlags")->Write();

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

void CbmTSMonitorTof::FillOutput(CbmDigi* /*digi*/)
{
}

void CbmTSMonitorTof::SetDiamondChannels(Int_t iGdpb, Int_t iFeet,
    Int_t iChannelA, Int_t iChannelB, Int_t iChannelC, Int_t iChannelD)
{
  fDiamondGdpb = iGdpb;
  fDiamondFeet = iFeet;
  fDiamondChanA = iChannelA;
  fDiamondChanB = iChannelB;
  fDiamondChanC = iChannelC;
  fDiamondChanD = iChannelD;
}

void CbmTSMonitorTof::ResetAllHistos()
{
  LOG(INFO) << "Reseting all TOF histograms." << FairLogger::endl;
  fHM->H1("hMessageType")->Reset();
  fHM->H1("hSysMessType")->Reset();
  fHM->H2("hGet4MessType")->Reset();
  fHM->H2("hGet4ChanErrors")->Reset();
  fHM->H2("hGet4EpochFlags")->Reset();
  fHM->H2("hDiamondSpillLength")->Reset();

  for (UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb++) {
    fHM->H2(Form("Raw_Tot_gDPB_%02u", uGdpb))->Reset();
    fHM->H1(Form("ChCount_gDPB_%02u", uGdpb))->Reset();
    if (fUnpackPar->IsChannelRateEnabled())
      fHM->H2(Form("ChannelRate_gDPB_%02u", uGdpb))->Reset();
    for (UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; uFeet++) {
      fHM->H1(Form("FeetRate_gDPB_g%02u_f%1u", uGdpb, uFeet))->Reset();
    } // for( UInt_t uFeet = 0; uFeet < fNrOfFebsPerGdpb; uFeet ++)
  } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)

  for (UInt_t uLinks = 0; uLinks < 16; uLinks++) {
    TString sMsSzName = Form("MsSz_link_%02u", uLinks);
    if (fHM->Exists(sMsSzName.Data()))
      fHM->H1(sMsSzName.Data())->Reset();

    sMsSzName = Form("MsSzTime_link_%02u", uLinks);
    if (fHM->Exists(sMsSzName.Data()))
      fHM->P1(sMsSzName.Data())->Reset();
  } // for( UInt_t uLinks = 0; uLinks < 16; uLinks ++)

  fdStartTime = -1;
}

ClassImp(CbmTSMonitorTof)
