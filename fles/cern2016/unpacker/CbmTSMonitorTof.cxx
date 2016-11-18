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

#include <iostream>
#include <stdint.h>
#include <iomanip>

static Int_t iMess=0;

CbmTSMonitorTof::CbmTSMonitorTof( UInt_t uNbGdpb )
  : CbmTSUnpack(),
    fuMsAcceptsPercent(100),
    fuMinNbGdpb( uNbGdpb ),
	fNrOfGdpbs(-1),
	fNrOfFebsPerGdpb(-1),
	fNrOfGet4PerFeb(-1),
    fNrOfChannelsPerGet4(-1),
	fNrOfGet4(-1),
	fNrOfGet4PerGdpb(-1),
    fuCurrNbGdpb( 0 ),
    fMsgCounter(11,0), // length of enum MessageTypes initialized with 0
    fGdpbIdIndexMap(),
    fHM(new CbmHistManager()),
    fCurrentEpoch(),
    fTsLastHit(),
    fNofEpochs(0),
    fCurrentEpochTime(0.),
    fEquipmentId(0),
    fUnpackPar(NULL)
{
}

CbmTSMonitorTof::~CbmTSMonitorTof()
{
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
   fUnpackPar = (CbmTofUnpackPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmTofUnpackPar"));

}

Bool_t CbmTSMonitorTof::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
             << FairLogger::endl;
   Bool_t initOK = ReInitContainers();

   CreateHistograms();

   return initOK;
}

Bool_t CbmTSMonitorTof::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
             << FairLogger::endl;

   fNrOfGdpbs = fUnpackPar->GetNrOfRocs();
   LOG(INFO) << "Nr. of Tof GDPBs: " << fNrOfGdpbs
             << FairLogger::endl;

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
   LOG(INFO) << "Nr. of GET4s: " << fNrOfGet4
             << FairLogger::endl;

   fNrOfGet4PerGdpb = fNrOfFebsPerGdpb * fNrOfGet4PerFeb;
   LOG(INFO) << "Nr. of GET4s per GDPB: " << fNrOfGet4PerGdpb
             << FairLogger::endl;

   
   fGdpbIdIndexMap.clear();
   for (Int_t i = 0; i< fNrOfGdpbs; ++i) {
     fGdpbIdIndexMap[fUnpackPar->GetRocId(i)] = i;
     LOG(INFO) << "GDPB Id of TOF  " << i
               << " : " << fUnpackPar->GetRocId(i)
               << FairLogger::endl;
   }
   Int_t NrOfChannels = fUnpackPar->GetNumberOfChannels();
   LOG(INFO) << "Nr. of mapped Tof channels: " << NrOfChannels;
   for (Int_t i = 0; i< NrOfChannels; ++i) {
      if(i%8 == 0)
         LOG(INFO) << FairLogger::endl;
      LOG(INFO) << Form(" 0x%08x",fUnpackPar->GetChannelToDetUIdMap(i));
   }
   LOG(INFO) << FairLogger::endl;
   
   LOG(INFO) << "Plot Channel Rate => " << ( fUnpackPar->IsChannelRateEnabled() ? "ON" : "OFF") 
             << FairLogger::endl;
   
   return kTRUE;
}

void CbmTSMonitorTof::CreateHistograms()
{
#ifdef USE_HTTP_SERVER
	  THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
#endif

  TString name{""};
  TString title{""};

  name = "hMessageType";
  title = "Nb of message for each type; Type";
  // Test Big Data readout with plotting
  TH1I* hMessageType = new TH1I(name, title, 16, 0., 16.);
  //  TH1I* hMessageType = new TH1I(name, title, 16, -0.5, 15.5);
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_NOP,      "NOP");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_HIT,      "HIT");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_EPOCH,    "EPOCH");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_SYNC,     "SYNC");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_AUX,      "AUX");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_EPOCH2,   "EPOCH2");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4,     "GET4");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_SYS,      "SYS");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4_SLC, "MSG_GET4_SLC");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4_32B, "MSG_GET4_32B");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_GET4_SYS, "MSG_GET4_SYS");
  hMessageType->GetXaxis()->SetBinLabel(1 + 15, "GET4 Hack 32B");
  hMessageType->GetXaxis()->SetBinLabel(1 + ngdpb::MSG_NOP,      "NOP");
  fHM->Add(name.Data(), hMessageType);
#ifdef USE_HTTP_SERVER
      server->Register("/TofRaw", fHM->H1(name.Data()));
#endif

  name = "hSysMessType";
  title = "Nb of system message for each type; System Type";
  TH1I* hSysMessType = new TH1I(name, title, 17, 0., 17.);
//  TH1I* hSysMessType = new TH1I(name, title, 17, -0.5, 16.5);
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_START,       "DAQ START");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_NX_PARITY,       "NX PARITY");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_SYNC_PARITY,     "SYNC PARITY");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_DAQ_RESUME,      "DAQ RESUME");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_FIFO_RESET,      "FIFO RESET");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_USER,            "USER");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_PCTIME,          "PCTIME");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_ADC,             "ADC");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_PACKETLOST,      "PACKET LOST");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_GET4_EVENT,      "GET4 ERROR");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_CLOSYSYNC_ERROR, "CLOSYSYNC ERROR");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_TS156_SYNC,        "TS156 SYNC");
  hSysMessType->GetXaxis()->SetBinLabel(1 + ngdpb::SYSMSG_GDPB_UNKWN,        "UNKW GET4 MSG");
  hSysMessType->GetXaxis()->SetBinLabel(1 + 16, "GET4 Hack 32B");
  fHM->Add(name.Data(), hSysMessType);
#ifdef USE_HTTP_SERVER
      server->Register("/TofRaw", fHM->H1(name.Data()));
#endif

  name = "hGet4MessType";
  title = "Nb of message for each type per GET4; GET4 chip # ; Type";
  TH2I* hGet4MessType = new TH2I(name, title, fNrOfGet4, 0., fNrOfGet4, 5 , 0., 5.);
//  TH2I* hGet4MessType = new TH2I(name, title, uNbFeets*feetv1::kuChipPerFeet, -0.5, uNbFeets*feetv1::kuChipPerFeet -0.5, 5, -0.5, 4.5);
  hGet4MessType->GetYaxis()->SetBinLabel(1 + ngdpb::GET4_32B_EPOCH, "EPOCH");
  hGet4MessType->GetYaxis()->SetBinLabel(1 + ngdpb::GET4_32B_SLCM,  "S.C. M");
  hGet4MessType->GetYaxis()->SetBinLabel(1 + ngdpb::GET4_32B_ERROR, "ERROR");
  hGet4MessType->GetYaxis()->SetBinLabel(1 + ngdpb::GET4_32B_DATA,  "DATA 32b");
  hGet4MessType->GetYaxis()->SetBinLabel(1 + ngdpb::GET4_32B_DATA+1,"DATA 24b");
  fHM->Add(name.Data(), hGet4MessType);
#ifdef USE_HTTP_SERVER
      server->Register("/TofRaw", fHM->H2(name.Data()));
#endif

  name = "hGet4ChanErrors";
  title = "Error messages per GET4 channel; GET4 channel # ; Error";
  TH2I* hGet4ChanErrors    = new TH2I(name, title, fNrOfGet4*fNrOfChannelsPerGet4 , 0., fNrOfGet4*fNrOfChannelsPerGet4, 32, 0., 32.);
//  TH2I* hGet4ChanErrors    = new TH2I(name, title, uNbFeets*feetv1::kuChanPerFeet*2, -0.5, uNbFeets*feetv1::kuChanPerFeet -0.5, 32, -0.5, 31.5);
  hGet4ChanErrors->GetYaxis()->SetBinLabel(1,  "0x00: Readout Init    ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(2,  "0x01: Sync            ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(3,  "0x02: Epoch count sync");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(4,  "0x03: Epoch           ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(5,  "0x04: FIFO Write      ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(6,  "0x05: Lost event      ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(7,  "0x06: Channel state   ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(8,  "0x07: Token Ring state");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(9,  "0x08: Token           ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(10, "0x09: Error Readout   ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(11, "0x0a: SPI             ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(12, "0x0b: DLL Lock error  "); // <- From GET4 v1.2
  hGet4ChanErrors->GetYaxis()->SetBinLabel(13, "0x0c: DLL Reset invoc."); // <- From GET4 v1.2
  hGet4ChanErrors->GetYaxis()->SetBinLabel(14, "0x11: Overwrite       ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(15, "0x12: ToT out of range");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(16, "0x13: Event Discarded ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(17, "0x7f: Unknown         ");
  hGet4ChanErrors->GetYaxis()->SetBinLabel(18, "Corrupt error or unsupported yet");
  fHM->Add(name.Data(), hGet4ChanErrors);
#ifdef USE_HTTP_SERVER
      server->Register("/TofRaw", fHM->H2(name.Data()));
#endif

  Double_t w = 10;
  Double_t h = 10;

  TCanvas* c1 = new TCanvas("c1", "Test canvas c1", w, h);
  c1->Divide( 2, 2 );

  c1->cd(1);
  gPad->SetLogy();
  hMessageType->Draw();

  c1->cd(2);
  hSysMessType->Draw();

  c1->cd(3);
  hGet4MessType->Draw("colz");
  gPad->SetLogz();


  c1->cd(4);
  hGet4ChanErrors->Draw("colz");

 /*
  hEpochFlags = new TH2I("hEpochFlags",
		  "Epoch flags per chip; GET4 chip #",
		  uNbFeets*feetv1::kuChipPerFeet, -0.5, uNbFeets*feetv1::kuChipPerFeet -0.5,
		  4, -0.5, 3.5);
  hEpochFlags->GetYaxis()->SetBinLabel(1,       "SYNC");
  hEpochFlags->GetYaxis()->SetBinLabel(2,       "Ep LOSS");
  hEpochFlags->GetYaxis()->SetBinLabel(3,       "Da LOSS");
  hEpochFlags->GetYaxis()->SetBinLabel(4,       "MISSMAT");

  hDataAvail = new TH1I( "hDataAvail",
		  "Data available before readout; # Data Avail [msg]; Checks []",
		  32769, -0.5, 32769 - 0.5);

  hChannelCounts = new TH1I("hChannelCounts",
		  "Hit counts per channel; Channel # []; Counts []",
		  uNbFeets*feetv1::kuChanPerFeet, -0.5, uNbFeets*feetv1::kuChanPerFeet - 0.5);

  hEpochCounts = new TH1I("hEpochCounts",
		  "Epoch counts per channel (chips); Channel # []; Counts []",
		  uNbFeets*feetv1::kuChipPerFeet, -0.5, uNbFeets*feetv1::kuChanPerFeet - 0.5);

  hTotDistribution = new TH2I("hTotDistribution",
		  "Distribution of TOT counts per channel; Channel # []; TOT [bin]",
		  uNbFeets*feetv1::kuChanPerFeet, -0.5, uNbFeets*feetv1::kuChanPerFeet - 0.5,
		  feetv1::kuTotBinsNb, -0.5, feetv1::kuTotBinsNb - 0.5);

  hTsDistribution = new TH2I("hTsDistribution",
		  "Distribution of TS counts per channel; Channel # []; Ts [bin]",
		  uNbFeets*feetv1::kuChanPerFeet, -0.5, uNbFeets*feetv1::kuChanPerFeet - 0.5,
		  feetv1::kuTsBinsNb/16, -0.5, feetv1::kuTsBinsNb - 0.5);

      hEpochDistribution = new TH2I("hEpochDistribution",
            "Distribution of Epochs per chip; Chip # []; Epoch [bin]",
            uNbFeets*feetv1::kuChipPerFeet, -0.5, uNbFeets*feetv1::kuChipPerFeet - 0.5,
            4096, -0.5, 0x7FFFFFFF - 0.5);
 */
  /*
      hChanCoinc = new TH2I("hChanCoinc",
            "Channels coincidences; Channel A # []; Channel B # []",
            uNbFeets*feetv1::kuChanPerFeet, -0.5, uNbFeets*feetv1::kuChanPerFeet - 0.5,
            uNbFeets*feetv1::kuChanPerFeet, -0.5, uNbFeets*feetv1::kuChanPerFeet - 0.5);
   */
/*
       hTimeDiffNext = new TH2I("hTimeDiffNext",
            "Time difference to next channel; First Channel # []; TS(Ch + 1) - TS (Ch) [Bin]",
            uNbFeets*feetv1::kuChanPerFeet - 1, -0.5, uNbFeets*feetv1::kuChanPerFeet - 1.5,
*/
  //         1025, -512.5, +512.5);
   //         10250, -5120.5, +5120.5);
   //         10251, -10250.5, +10250.5);

  if( fUnpackPar->IsChannelRateEnabled() ) {
	for( UInt_t uGdpb = 0; uGdpb < fNrOfGdpbs; uGdpb ++)
	{
	  const Int_t iNbBinsRate = 82;
	  Double_t dBinsRate[iNbBinsRate] =
	        { 1e0, 2e0, 3e0, 4e0, 5e0, 6e0, 7e0, 8e0, 9e0,
			  1e1, 2e1, 3e1, 4e1, 5e1, 6e1, 7e1, 8e1, 9e1,
			  1e2, 2e2, 3e2, 4e2, 5e2, 6e2, 7e2, 8e2, 9e2,
			  1e3, 2e3, 3e3, 4e3, 5e3, 6e3, 7e3, 8e3, 9e3,
			  1e4, 2e4, 3e4, 4e4, 5e4, 6e4, 7e4, 8e4, 9e4,
			  1e5, 2e5, 3e5, 4e5, 5e5, 6e5, 7e5, 8e5, 9e5,
			  1e6, 2e6, 3e6, 4e6, 5e6, 6e6, 7e6, 8e6, 9e6,
			  1e7, 2e7, 3e7, 4e7, 5e7, 6e7, 7e7, 8e7, 9e7,
			  1e8, 2e8, 3e8, 4e8, 5e8, 6e8, 7e8, 8e8, 9e8,
			  1e9 };
	  name = Form("ChannelRate_gDPB_%02u", uGdpb);
	  title = Form("Channel instant rate gDPB %02u; Rate[Hz] ; Channel", uGdpb);
	  fHM->Add(name.Data(), new TH2F(name.Data(), title.Data(),
			   iNbBinsRate-1, dBinsRate, 96, 0, 95) );
#ifdef USE_HTTP_SERVER
      server->Register("/TofRaw", fHM->H2(name.Data()));
#endif
	  LOG(INFO) << "Adding the rate histos" << FairLogger::endl;
	} // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
  }

  for( UInt_t uGdpb = 0; uGdpb < fNrOfGdpbs; uGdpb ++)
  {
    name = Form("Raw_Tot_gDPB_%02u", uGdpb);
    title = Form("Raw TOT gDPB %02u; channel; TOT [bin]", uGdpb);
    fHM->Add(name.Data(), new TH2F(name.Data(), title.Data(), 96, 0, 95, 256, 0, 255) );
#ifdef USE_HTTP_SERVER
    server->Register("/TofRaw", fHM->H2(name.Data()));
#endif

    name = Form("ChCount_gDPB_%02u", uGdpb);
    title = Form("Channel counts gDPB %02u; channel; Hits", uGdpb);
    fHM->Add(name.Data(), new TH1I(name.Data(), title.Data(), 96, 0, 95 ) );


#ifdef USE_HTTP_SERVER
    server->Register("/TofRaw", fHM->H1(name.Data()));
#endif
/*
      if( fUnpackPar->IsChannelRateEnabled() )
      {
         const Int_t iNbBinsRate = 10;
         Double_t dBinsRate[iNbBinsRate] = { 0, 10, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9 };
         fHM->Add( Form("ChannelRate_gDPB_%02u", uGdpb),
              new TH2F( Form("ChannelRate_gDPB_%02u", uGdpb),
                        Form("Channel instant rate gDPB %02u; Rate[Hz] ; Channel", uGdpb),
                        iNbBinsRate, dBinsRate, 96, 0, 95) );
      } // if( fUnpackPar->IsChannelRateEnabled() )
*/
  } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)

  LOG(INFO) << "Leaving CreateHistograms" << FairLogger::endl;
}

Bool_t CbmTSMonitorTof::DoUnpack(const fles::Timeslice& ts, size_t component)
{

   LOG(DEBUG1) << "Timeslice contains " << ts.num_microslices(component)
               << "microslices." << FairLogger::endl;


   // Getting the pointer to the correct histogram needs a lot more time then
   // the actual filling procedure. If one gets the pointer in a loop this kills
   // the performance. A test shows that extracting the pointer from the CbmHistManger
   // is slower by a factor of 20-100 (depending on the number of histos managed by the
   // CbmHistManager) compared to using the pointer directly
   // So get the pointer once outside the loop and use it in the loop

   TH1* histMessType = fHM->H1("hMessageType");
   TH1* histSysMessType = fHM->H1("hSysMessType");
   TH2* histGet4MessType = fHM->H2("hGet4MessType");
   TH2* histGet4ChanErrors = fHM->H2("hGet4ChanErrors");

   std::vector<TH2*> Raw_Tot_gDPB;
   std::vector<TH1*> ChCount_gDPB;
   std::vector<TH2*> ChannelRate_gDPB;

   for(Int_t i=0; i<fNrOfGdpbs; ++i) {
     TString name = Form("Raw_Tot_gDPB_%02u", i);
     Raw_Tot_gDPB.push_back(fHM->H2(name.Data()));
     name = Form("ChCount_gDPB_%02u", i);
     ChCount_gDPB.push_back(fHM->H1(name.Data()));
  	 name = Form("ChannelRate_gDPB_%02u", i);
     if( fUnpackPar->IsChannelRateEnabled() ) {
       ChannelRate_gDPB.push_back(fHM->H2(name.Data()));
     }
   }

   // Loop over microslices
   for (size_t m = 0; m < ts.num_microslices(component); ++m)
   {
      if( fuMsAcceptsPercent < m )
      continue;

      constexpr uint32_t kuBytesPerMessage = 8;

      auto msDescriptor = ts.descriptor(component, m);
      fEquipmentId = msDescriptor.eq_id;
      const uint8_t* msContent = reinterpret_cast<const uint8_t*>(ts.content(component, m));

      uint32_t size = msDescriptor.size;
      if(size>0)
      LOG(DEBUG) << "Microslice: " << msDescriptor.idx 
      << " has size: " << size << FairLogger::endl; 

      // If not integer number of message in input buffer, print warning/error
      if( 0 != (size % kuBytesPerMessage) )
      LOG(ERROR) << "The input microslice buffer does NOT " 
      << "contain only complete nDPB messages!" 
      << FairLogger::endl;

      // Compute the number of complete messages in the input microslice buffer
      uint32_t uNbMessages = (size - (size % kuBytesPerMessage) )
      / kuBytesPerMessage;

      // Prepare variables for the loop on contents
      const uint64_t* pInBuff = reinterpret_cast<const uint64_t*>( msContent );
      for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)
      {
         // Fill message
         uint64_t ulData = static_cast<uint64_t>( pInBuff[uIdx] );
         ngdpb::Message mess( ulData );

         if(gLogger->IsLogNeeded(DEBUG)) {
            mess.printDataCout();
         }

         // Increment counter for different message types 
         // and fill the corresponding histogram
         fMsgCounter[mess.getMessageType()]++;
         histMessType->Fill(mess.getMessageType());

         Int_t rocId      = fGdpbIdIndexMap[mess.getRocNumber()];
         Int_t get4Id     = mess.getGdpbGenChipId();
         Int_t get4Nr    = (rocId*fNrOfGet4PerGdpb) + get4Id;

         switch( mess.getMessageType() ) 
         {
            case ngdpb::MSG_HIT: 
               //           FillHitInfo(mess);
               LOG(ERROR) << "Message type " << mess.getMessageType() 
                          << " not yet included in unpacker."
                          << FairLogger::endl;
               break;
            case ngdpb::MSG_EPOCH:
               //           FillEpochInfo(mess);
               LOG(ERROR) << "Message type " << mess.getMessageType() 
                          << " not yet included in unpacker."
                          << FairLogger::endl;
               break;
            case ngdpb::MSG_SYS:
               histSysMessType->Fill(mess.getSysMesType());
               break;
            case ngdpb::MSG_EPOCH2:
           	   histGet4MessType->Fill(get4Nr, ngdpb::GET4_32B_EPOCH);
               FillEpochInfo(mess);
               break;
            case ngdpb::MSG_GET4:
           	   histGet4MessType->Fill(get4Nr, ngdpb::GET4_32B_DATA + 1);
               PrintGenInfo(mess);
               break;
            case ngdpb::MSG_GET4_32B:
               histGet4MessType->Fill(get4Nr, ngdpb::GET4_32B_DATA);
               FillHitInfo(mess, Raw_Tot_gDPB, ChCount_gDPB, ChannelRate_gDPB);
               break;
            case ngdpb::MSG_GET4_SLC:
           	   histGet4MessType->Fill(get4Nr, ngdpb::GET4_32B_SLCM);
               PrintSlcInfo(mess);
               break;
            case ngdpb::MSG_GET4_SYS:
               {
                 histSysMessType->Fill( mess.getGdpbSysSubType() );
                 if( ngdpb::SYSMSG_GET4_EVENT == mess.getGdpbSysSubType() ) {
                   histGet4MessType->Fill(get4Nr, ngdpb::GET4_32B_ERROR);
                   Int_t dFullChId  = (rocId*fNrOfGet4PerGdpb
                				      + get4Id)*fNrOfChannelsPerGet4
									  + mess.getGdpbHitChanId();
                   switch( mess.getGdpbSysErrData() )
                   {
                      case ngdpb::GET4_V1X_ERR_READ_INIT:
                         histGet4ChanErrors->Fill( dFullChId, 0);
                         break;
                      case ngdpb::GET4_V1X_ERR_SYNC:
                         histGet4ChanErrors->Fill( dFullChId, 1);
                         break;
                      case ngdpb::GET4_V1X_ERR_EP_CNT_SYNC:
                         histGet4ChanErrors->Fill( dFullChId, 2);
                         break;
                      case ngdpb::GET4_V1X_ERR_EP:
                         histGet4ChanErrors->Fill( dFullChId, 3);
                         break;
                      case ngdpb::GET4_V1X_ERR_FIFO_WRITE:
                         histGet4ChanErrors->Fill( dFullChId, 4);
                         break;
                      case ngdpb::GET4_V1X_ERR_LOST_EVT:
                         histGet4ChanErrors->Fill( dFullChId, 5);
                         break;
                      case ngdpb::GET4_V1X_ERR_CHAN_STATE:
                         histGet4ChanErrors->Fill( dFullChId, 6);
                         break;
                      case ngdpb::GET4_V1X_ERR_TOK_RING_ST:
                         histGet4ChanErrors->Fill( dFullChId, 7);
                         break;
                      case ngdpb::GET4_V1X_ERR_TOKEN:
                         histGet4ChanErrors->Fill( dFullChId, 8);
                         break;
                      case ngdpb::GET4_V1X_ERR_READOUT_ERR:
                         histGet4ChanErrors->Fill( dFullChId, 9);
                         break;
                      case ngdpb::GET4_V1X_ERR_SPI:
                         histGet4ChanErrors->Fill( dFullChId, 10);
                         break;
                      case ngdpb::GET4_V1X_ERR_DLL_LOCK:
                         histGet4ChanErrors->Fill( dFullChId, 11);
                         break;
                      case ngdpb::GET4_V1X_ERR_DLL_RESET:
                         histGet4ChanErrors->Fill( dFullChId, 12);
                         break;
                      case ngdpb::GET4_V1X_ERR_TOT_OVERWRT:
                         histGet4ChanErrors->Fill( dFullChId, 13);
                         break;
                      case ngdpb::GET4_V1X_ERR_TOT_RANGE:
                         histGet4ChanErrors->Fill( dFullChId, 14);
                         break;
                      case ngdpb::GET4_V1X_ERR_EVT_DISCARD:
                         histGet4ChanErrors->Fill( dFullChId, 15);
                         break;
                      case ngdpb::GET4_V1X_ERR_UNKNOWN:
                         histGet4ChanErrors->Fill( dFullChId, 16);
                         break;
                      default: // Corrupt error or not yet supported error
                         histGet4ChanErrors->Fill( dFullChId, 17);
                         break;
                   } // Switch( mess.getGdpbSysErrData() )

                 }
                 if(100 > iMess++)
                 PrintSysInfo(mess);
                 break;
               }
               default:
               if(100 > iMess++)
                  LOG(ERROR) << "Message ("<<iMess<<") type " << std::hex << std::setw(2) 
                             << static_cast< uint16_t >( mess.getMessageType() ) 
                             << " not yet included in Get4 unpacker."
                             << FairLogger::endl;
               if(100 == iMess)
                  LOG(ERROR) << "Stop reporting MSG errors... "
                             << FairLogger::endl;
         } // switch( mess.getMessageType() ) 
      } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)
   } // for (size_t m = 0; m < ts.num_microslices(component); ++m)

   return kTRUE;
}

void CbmTSMonitorTof::FillHitInfo(ngdpb::Message mess,
                                  std::vector<TH2*> Raw_Tot_gDPB,
                                  std::vector<TH1*> ChCount_gDPB,
                                  std::vector<TH2*> ChannelRate_gDPB
)
   {
   // --- Get absolute time, NXYTER and channel number
   Int_t rocId      = mess.getRocNumber();
   Int_t get4Id     = mess.getGdpbGenChipId();
   Int_t channel    = mess.getGdpbHitChanId(); 
   Int_t tot        = mess.getGdpbHit32Tot();
   ULong_t hitTime  = mess.getMsgFullTime( 0 );

   if( fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find( rocId ) )
   {
	  Int_t gdpbNr = fGdpbIdIndexMap[ rocId ];
	  Raw_Tot_gDPB[gdpbNr]->Fill( get4Id*fNrOfChannelsPerGet4 + channel, tot);
	  ChCount_gDPB[gdpbNr]->Fill( get4Id*fNrOfChannelsPerGet4 + channel );
        
      if( fUnpackPar->IsChannelRateEnabled() )
      {
         // Check if at least one hit before in this gDPB
         if( fTsLastHit.end() != fTsLastHit.find( rocId ) )
         {
            // Check if at least one hit before in this Get4
            if( fTsLastHit[rocId].end() != fTsLastHit[rocId].find( get4Id ) )
            {
               // Check if at least one hit before in this channel
               if( fTsLastHit[rocId][get4Id].end() != fTsLastHit[rocId][get4Id].find( channel ) )
               {
            	   ChannelRate_gDPB[gdpbNr]->Fill( 1e9/ ( mess.getMsgFullTimeD( fCurrentEpoch[rocId][get4Id] )
                                     	           - fTsLastHit[rocId][get4Id][channel] ),
                             	 	 	           get4Id*fNrOfChannelsPerGet4 + channel);
               } // if( fTsLastHit[rocId][get4Id].end() != fTsLastHit[rocId][get4Id].find( channel ) )
            } // if( fTsLastHit[rocId].end() != fTsLastHit[rocId].find( get4Id ) )
         } // if( fTsLastHit.end() != fTsLastHit.find( rocId ) )
             
         fTsLastHit[rocId][get4Id][channel] = mess.getMsgFullTimeD( fCurrentEpoch[rocId][get4Id] );
      } // if( fUnpackPar->IsChannelRateEnabled() )
    
      hitTime  = mess.getMsgFullTime(fCurrentEpoch[rocId][get4Id]);
      Int_t Ft = mess.getGdpbHitFineTs();

      if(100 > iMess++)
         LOG(DEBUG) << "Hit: " << Form("0x%08x ",rocId) << ", " << get4Id 
                    << ", " << channel << ", " << tot
                    << ", epoch " << fCurrentEpoch[rocId][get4Id]
                    << ", FullTime " << hitTime 
                    << ", FineTime " << Ft
                    << FairLogger::endl;
   } // if( fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find( rocId ) )
      else LOG(WARNING) << "found rocId w/o epoch yet: " << Form("0x%08x ",rocId) << FairLogger::endl;
      
}

void CbmTSMonitorTof::FillEpochInfo(ngdpb::Message mess)
{
   Int_t rocId          = mess.getRocNumber();
   Int_t get4Id     = mess.getGdpbGenChipId();
   fCurrentEpoch[rocId][get4Id] = mess.getEpoch2Number();

   // Create histograms for new ROCs in the data stream
   if( fGdpbIdIndexMap.end() == fGdpbIdIndexMap.find( rocId ) )
   {
      fGdpbIdIndexMap[ rocId ] = fuCurrNbGdpb;
      fuCurrNbGdpb ++;
     
      if( fuMinNbGdpb < fuCurrNbGdpb )
      {
      } // if( fuMinNbGdpb < fuCurrNbGdpb )
   } // if( std::map::end == fGdpbIdIndexMap.find( rocId ) )

   //  LOG(INFO) << "Epoch message for ROC " << rocId << " with epoch number "
   //            << fCurrentEpoch[rocId] << FairLogger::endl;

   fCurrentEpochTime = mess.getMsgFullTime(fCurrentEpoch[rocId][get4Id]);
   fNofEpochs++;
   LOG(DEBUG) << "Epoch message "
              << fNofEpochs << ", epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][get4Id])
              << ", time " << std::setprecision(9) << std::fixed
              << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
              << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
              << " and chip " << mess.getEpoch2ChipNumber()
              << FairLogger::endl;

}

void CbmTSMonitorTof::PrintSlcInfo(ngdpb::Message mess)
{
   Int_t rocId      = mess.getRocNumber();
   Int_t get4Id     = mess.getGdpbGenChipId();

   if( fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find( rocId ) )
      LOG(INFO) << "GET4 Slow Control message, epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][get4Id])
                << ", time " << std::setprecision(9) << std::fixed
                << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
                << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
                << FairLogger::endl
                << " +++++++ > Chip = " << std::setw(2) << mess.getGdpbGenChipId()
                << ", Chan = " << std::setw(1) << mess.getGdpbSlcChan()
                << ", Edge = " << std::setw(1) << mess.getGdpbSlcEdge()
                << ", Type = " << std::setw(1) << mess.getGdpbSlcType()
                << ", Data = " << std::hex << std::setw(6) << mess.getGdpbSlcData() << std::dec
                << ", Type = " << mess.getGdpbSlcCrc()
                << FairLogger::endl;

}

void CbmTSMonitorTof::PrintGenInfo(ngdpb::Message mess)
{
   Int_t mType    = mess.getMessageType();
   Int_t rocId    = mess.getRocNumber();
   Int_t get4Id   = mess.getGdpbGenChipId();
   Int_t channel  = mess.getGdpbHitChanId();
   uint64_t uData = mess.getData(); 

   if(100 > iMess++)
   LOG(INFO) << "Get4 MSG type "<<mType<<" from rocId "<<rocId<<", getId "<<get4Id
             << ", (hit channel) "<< channel << " data " << std::hex
             << uData
//Form(" data 0x%01Fx ",uData)
             << FairLogger::endl;  
}

void CbmTSMonitorTof::PrintSysInfo(ngdpb::Message mess)
{
   Int_t rocId          = mess.getRocNumber();
   Int_t get4Id     = mess.getGdpbGenChipId();

   if( fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find( rocId ) )
      LOG(INFO) << "GET4 System message,       epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][get4Id])
                << ", time " << std::setprecision(9) << std::fixed
                << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
                << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
                << FairLogger::endl;
             
   switch( mess.getGdpbSysSubType() )
   {
      case ngdpb::SYSMSG_GET4_EVENT:
      {
         LOG(INFO) << " +++++++ > Chip = " << std::setw(2) << mess.getGdpbGenChipId()
                   << ", Chan = " << std::setw(1) << mess.getGdpbSysErrChanId()
                   << ", Edge = " << std::setw(1) << mess.getGdpbSysErrEdge()
                   << ", Empt = " << std::setw(1) << mess.getGdpbSysErrUnused()
                   << ", Data = " << std::hex << std::setw(2) << mess.getGdpbSysErrData() << std::dec
                   << " -- GET4 V1 Error Event"
                   << FairLogger::endl;
         break;
      } // case ngdpb::SYSMSG_GET4_EVENT
      case ngdpb::SYSMSG_CLOSYSYNC_ERROR:
         LOG(INFO) << "Closy synchronization error" << FairLogger::endl;
         break;
      case ngdpb::SYSMSG_TS156_SYNC:
         LOG(INFO) << "156.25MHz timestamp reset" << FairLogger::endl;
         break;
      case ngdpb::SYSMSG_GDPB_UNKWN:
         LOG(INFO) << "Unknown GET4 message, data: " << std::hex << std::setw(8)
                   << mess.getGdpbSysUnkwData() << std::dec
                   << FairLogger::endl;
         break;
   } // switch( getGdpbSysSubType() )
}

void CbmTSMonitorTof::Reset()
{
}

void CbmTSMonitorTof::Finish()
{
   TString message_type;

   for (unsigned int i=0; i< fMsgCounter.size(); ++i)
   {
      switch(i)
      {
         case 0: message_type ="NOP"; break;
         case 1: message_type ="HIT"; break;
         case 2: message_type ="EPOCH"; break;
         case 3: message_type ="SYNC"; break;
         case 4: message_type ="AUX"; break;
         case 5: message_type ="EPOCH2"; break;
         case 6: message_type ="GET4"; break;
         case 7: message_type ="SYS"; break;
         case 8: message_type ="GET4_SLC"; break;
         case 9: message_type ="GET4_32B"; break;
         case 10: message_type ="GET4_SYS"; break;
         default:  message_type ="UNKNOWN"; break;
      } // switch(i)
      LOG(INFO) << message_type << " messages: " 
                << fMsgCounter[i] << FairLogger::endl;
   } // for (unsigned int i=0; i< fMsgCounter.size(); ++i)
  
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   for( auto it = fCurrentEpoch.begin(); it != fCurrentEpoch.end(); ++it)
      for( auto itG = (it->second).begin(); itG != (it->second).end(); ++itG)
      LOG(INFO) << "Last epoch for gDPB: " 
                << std::hex << std::setw(4) << it->first 
                << " , GET4  " << std::setw(4) << itG->first 
                << " => " << itG->second 
                << FairLogger::endl;
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   
   
   gDirectory->mkdir("Tof_Raw_gDPB");
   gDirectory->cd("Tof_Raw_gDPB");
   for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   {
      fHM->H2( Form("Raw_Tot_gDPB_%02u", uGdpb) )->Write();
      fHM->H1( Form("ChCount_gDPB_%02u", uGdpb) )->Write();
      if( fUnpackPar->IsChannelRateEnabled() )
         fHM->H2( Form("ChannelRate_gDPB_%02u", uGdpb) )->Write();
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   gDirectory->cd("..");

}


void CbmTSMonitorTof::FillOutput(CbmDigi* /*digi*/)
{
}


ClassImp(CbmTSMonitorTof)
