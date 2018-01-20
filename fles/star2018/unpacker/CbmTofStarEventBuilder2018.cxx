// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTofStarEventBuilder2018                 -----
// -----               Created 08.12.2017 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTofStarEventBuilder2018.h"
#include "CbmTofStar2018Par.h"
#include "CbmTofDigiExp.h"
#include "CbmTbEvent.h"

#include "CbmTbDaqBuffer.h"

#include "CbmHistManager.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRunOnline.h"

#include "TClonesArray.h"
#include "TString.h"
#include "THttpServer.h"

#include <iostream>
#include <stdint.h>
#include <iomanip>

static Int_t iMess=0;
const  Int_t DetMask = 0x0001FFFF;


CbmTofStarEventBuilder2018::CbmTofStarEventBuilder2018( UInt_t uNbGdpb )
  : CbmTSUnpack(),
    fuMsAcceptsPercent(100),
    fuTotalMsNb(0),
    fuOverlapMsNb(0),
    fuCoreMs(0),
    fdMsSizeInNs(0.0),
    fdTsCoreSizeInNs(0.0),
    fuMinNbGdpb( uNbGdpb ),
    fuCurrNbGdpb( 0 ),
    fuNrOfGdpbs(0),
    fuNrOfFebsPerGdpb(0),
    fuNrOfGet4PerFeb(0),
    fuNrOfChannelsPerGet4(0),
    fuNrOfChannelsPerFeet(0),
    fuNrOfGet4(0),
    fuNrOfGet4PerGdpb(0),
    fuNrOfChannelsPerGdpb(0),
    fMsgCounter(11,0), // length of enum MessageTypes initialized with 0
    fGdpbIdIndexMap(),
    fdStarTriggerDeadtime(),
    fdStarTriggerDelay(),
    fdStarTriggerWinSize(),
    fdTsDeadtimePeriod(0.0),
    fuCurrentMs(0),
    fuGdpbId(0),
    fuGdpbNr(0),
    fuGet4Id(0),
    fuGet4Nr(0),
    fvulCurrentEpoch(),
    fvbFirstEpochSeen(),
    fNofEpochs(0),
    fulCurrentEpochTime(0.),
    fdStartTime(-1.),
    fdStartTimeLong(-1.),
    fdStartTimeMsSz(-1.),
    fcMsSizeAll(NULL),
    fEquipmentId(0),
    fdMsIndex(0.),
    fUnpackPar(NULL),
    fuHistoryHistoSize( 1800 ),
    fuHistoryHistoSizeLong( 600 ),
    fulGdpbTsMsb(),
    fulGdpbTsLsb(),
    fulStarTsMsb(),
    fulStarTsMid(),
    fulGdpbTsFullLast(),
    fulStarTsFullLast(),
    fuStarTokenLast(),
    fuStarDaqCmdLast(),
    fuStarTrigCmdLast(),
    fhTokenMsgType(),
    fhTriggerRate(),
    fhCmdDaqVsTrig(),
    fhStarTokenEvo(),
    fbEventBuilding( kFALSE ),
    fbTimeSortOutput( kFALSE ),
    fStarSubEvent(),
    fulNbBuiltSubEvent( 0 ),
    fulNbStarSubEvent( 0 ),
    fulNbBuiltSubEventLastPrintout( 0 ),
    fulNbStarSubEventLastPrintout( 0 ),
    fTimeLastPrintoutNbStarEvent(),
    fdCurrentMsStartTime( 0.0 ),
    fdCurrentMsEndTime( 0.0 ),
    fvmCurrentLinkBuffer(),
    fvtCurrentLinkBuffer(),
    fdCurrentTsStartTime( 0.0 ),
    fdCurrentTsCoreEndTime( 0.0 ),
    fvmTsLinksBuffer(),
    fvtTsLinksBuffer(),
    fvmTsOverLinksBuffer(),
    fvtTsOverLinksBuffer(),
    fhStarHitToTrigAll_gDPB(),
    fhStarHitToTrigWin_gDPB(),
    fhStarEventSize_gDPB(),
    fhStarEventSizeTime_gDPB(),
    fhStarEventSizeTimeLong_gDPB(),
    fhStarTrigTimeToMeanTrig_gDPB(),
    fhStarEventSize( NULL ),
    fhStarEventSizeTime( NULL ),
    fhStarEventSizeTimeLong( NULL ),
    fvmEpSupprBuffer()
{
}

CbmTofStarEventBuilder2018::~CbmTofStarEventBuilder2018()
{
}

Bool_t CbmTofStarEventBuilder2018::Init()
{
   LOG(INFO) << "Initializing flib Get4 unpacker" << FairLogger::endl;

   FairRootManager* ioman = FairRootManager::Instance();
   if( NULL == ioman )
   {
      LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
   }

   fUnpackPar = (CbmTofStar2018Par*)(FairRun::Instance());

   return kTRUE;
}

void CbmTofStarEventBuilder2018::SetParContainers()
{
  LOG(INFO) << "Setting parameter containers for " << GetName()
	    << FairLogger::endl;
  fUnpackPar = (CbmTofStar2018Par*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmTofStar2018Par"));

}

Bool_t CbmTofStarEventBuilder2018::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
               << FairLogger::endl;
   Bool_t initOK = ReInitContainers();

   CreateHistograms();

   fvulCurrentEpoch.resize( fuNrOfGdpbs * fuNrOfGet4PerGdpb );
   fvbFirstEpochSeen.resize( fuNrOfGdpbs * fuNrOfGet4PerGdpb );
   for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )
   {
      for( UInt_t j = 0; j < fuNrOfGet4PerGdpb; ++j )
      {
         fvulCurrentEpoch[GetArrayIndex(i, j)] = 0;
      } // for( UInt_t j = 0; j < fuNrOfGet4PerGdpb; ++j )
   } // for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )

   return initOK;
}

Bool_t CbmTofStarEventBuilder2018::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
             << FairLogger::endl;

   fuNrOfGdpbs = fUnpackPar->GetNrOfRocs();
   LOG(INFO) << "Nr. of Tof GDPBs: " << fuNrOfGdpbs << FairLogger::endl;
   fuMinNbGdpb = fuNrOfGdpbs;

   fuNrOfFebsPerGdpb = fUnpackPar->GetNrOfFebsPerGdpb();
   LOG(INFO) << "Nr. of FEBS per Tof GDPB: " << fuNrOfFebsPerGdpb
               << FairLogger::endl;

   fuNrOfGet4PerFeb = fUnpackPar->GetNrOfGet4PerFeb();
   LOG(INFO) << "Nr. of GET4 per Tof FEB: " << fuNrOfGet4PerFeb
               << FairLogger::endl;

   fuNrOfChannelsPerGet4 = fUnpackPar->GetNrOfChannelsPerGet4();
   LOG(INFO) << "Nr. of channels per GET4: " << fuNrOfChannelsPerGet4
               << FairLogger::endl;

   fuNrOfChannelsPerFeet = fuNrOfGet4PerFeb * fuNrOfChannelsPerGet4;
   LOG(INFO) << "Nr. of channels per FEET: " << fuNrOfChannelsPerFeet
               << FairLogger::endl;

   fuNrOfGet4 = fuNrOfGdpbs * fuNrOfFebsPerGdpb * fuNrOfGet4PerFeb;
   LOG(INFO) << "Nr. of GET4s: " << fuNrOfGet4 << FairLogger::endl;

   fuNrOfGet4PerGdpb = fuNrOfFebsPerGdpb * fuNrOfGet4PerFeb;
   LOG(INFO) << "Nr. of GET4s per GDPB: " << fuNrOfGet4PerGdpb
               << FairLogger::endl;

   fuNrOfChannelsPerGdpb = fuNrOfGet4PerGdpb * fuNrOfChannelsPerGet4;
   LOG(INFO) << "Nr. of channels per GDPB: " << fuNrOfChannelsPerGdpb
               << FairLogger::endl;

   fGdpbIdIndexMap.clear();
   for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )
   {
      fGdpbIdIndexMap[fUnpackPar->GetRocId(i)] = i;
      LOG(INFO) << "GDPB Id of TOF  " << i << " : " << std::hex << fUnpackPar->GetRocId(i)
                 << std::dec << FairLogger::endl;
   } // for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )
   UInt_t uNrOfChannels = fUnpackPar->GetNumberOfChannels();
   LOG(INFO) << "Nr. of mapped Tof channels: " << uNrOfChannels;
   for( UInt_t i = 0; i < uNrOfChannels; ++i)
   {
      if (i % 8 == 0)
         LOG(INFO) << FairLogger::endl;
      LOG(INFO) << Form(" 0x%08x", fUnpackPar->GetChannelToDetUIdMap(i) );
   } // for( UInt_t i = 0; i < uNrOfChannels; ++i)
   LOG(INFO) << FairLogger::endl;

   fuTotalMsNb   = fUnpackPar->GetNbMsTot();
   fuOverlapMsNb = fUnpackPar->GetNbMsOverlap();
   fuCoreMs      = fuTotalMsNb - fuOverlapMsNb;
   fdMsSizeInNs  = fUnpackPar->GetSizeMsInNs();
   fdTsCoreSizeInNs = fdMsSizeInNs * fuCoreMs;
   LOG(INFO) << "Timeslice parameters: "
             << fuTotalMsNb << " MS per link, of which "
             << fuOverlapMsNb << " overlap MS, each MS is "
             << fdMsSizeInNs << " ns"
             << FairLogger::endl;

   fdStarTriggerDeadtime.resize( fuNrOfGdpbs );
   fdStarTriggerDelay.resize(    fuNrOfGdpbs );
   fdStarTriggerWinSize.resize(  fuNrOfGdpbs );
   for (UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb)
   {
      fdStarTriggerDeadtime[ uGdpb ] = fUnpackPar->GetStarTriggDeadtime( uGdpb );
      fdStarTriggerDelay[ uGdpb ]    = fUnpackPar->GetStarTriggDelay( uGdpb );
      fdStarTriggerWinSize[ uGdpb ]  = fUnpackPar->GetStarTriggWinSize( uGdpb );
      LOG(INFO) << "STAR trigger par of gDPB  " << uGdpb << " are: "
                << " Deadtime "  <<  fdStarTriggerDeadtime[ uGdpb ]
                << " Delay "     << -fdStarTriggerDelay[ uGdpb ]
                << " and WinSz " <<  fdStarTriggerWinSize[ uGdpb ]
                << FairLogger::endl;
      if( fdStarTriggerDelay[ uGdpb ] < fdStarTriggerWinSize[ uGdpb ] )
         LOG(FATAL) << "STAR trigger window should end at latest on token itself!"
                    << " => Delay has to be greater or equal with window size!"
                    << FairLogger::endl;
   } // for (Int_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb)
   fdTsDeadtimePeriod  = fUnpackPar->GetTsDeadtimePeriod();
   LOG(INFO) << "Timeslice deadtime period at beginning: "
             << fdTsDeadtimePeriod << "(dealt with using the overlap MS)"
             << FairLogger::endl;

   /// STAR Trigger decoding and monitoring
   fulGdpbTsMsb.resize(  fuNrOfGdpbs );
   fulGdpbTsLsb.resize(  fuNrOfGdpbs );
   fulStarTsMsb.resize(  fuNrOfGdpbs );
   fulStarTsMid.resize(  fuNrOfGdpbs );
   fulGdpbTsFullLast.resize(  fuNrOfGdpbs );
   fulStarTsFullLast.resize(  fuNrOfGdpbs );
   fuStarTokenLast.resize(  fuNrOfGdpbs );
   fuStarDaqCmdLast.resize(  fuNrOfGdpbs );
   fuStarTrigCmdLast.resize(  fuNrOfGdpbs );
   fhTokenMsgType.resize(  fuNrOfGdpbs );
   fhTriggerRate.resize(  fuNrOfGdpbs );
   fhCmdDaqVsTrig.resize(  fuNrOfGdpbs );
   fhStarTokenEvo.resize(  fuNrOfGdpbs );
   for (UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb)
   {
      fulGdpbTsMsb[ uGdpb ] = 0;
      fulGdpbTsLsb[ uGdpb ] = 0;
      fulStarTsMsb[ uGdpb ] = 0;
      fulStarTsMid[ uGdpb ] = 0;
      fulGdpbTsFullLast[ uGdpb ] = 0;
      fulStarTsFullLast[ uGdpb ] = 0;
      fuStarTokenLast[ uGdpb ]   = 0;
      fuStarDaqCmdLast[ uGdpb ]  = 0;
      fuStarTrigCmdLast[ uGdpb ] = 0;
      fhTokenMsgType[ uGdpb ] = NULL;
      fhTriggerRate[ uGdpb ]  = NULL;
      fhCmdDaqVsTrig[ uGdpb ] = NULL;
      fhStarTokenEvo[ uGdpb ] = NULL;
   } // for (Int_t iGdpb = 0; iGdpb < fuNrOfGdpbs; ++iGdpb)

   /// STAR subevent building
   if( kTRUE == fbEventBuilding )
   {
      fvmTsLinksBuffer.resize(  fuNrOfGdpbs );
      fvtTsLinksBuffer.resize(  fuNrOfGdpbs );
      fvmTsOverLinksBuffer.resize(  fuNrOfGdpbs );
      fvtTsOverLinksBuffer.resize(  fuNrOfGdpbs );
   } // if( kTRUE == fbEventBuilding )

/// TODO clean epoch suppression in STAR 2018!
//   fvmEpSupprBuffer.resize( fuNrOfGet4 );

	return kTRUE;
}

void CbmTofStarEventBuilder2018::SetEventBuildingMode( Bool_t bEventBuildingMode )
{
   fbEventBuilding = bEventBuildingMode;
   if( fbEventBuilding )
      LOG(INFO) << "Event building mode = single link per subevent: " << FairLogger::endl
                << "test mode in 2018 S1, sector mode in 2018 S2"
                << FairLogger::endl;
      else LOG(INFO) << "Event building mode = all links in same subevent: " << FairLogger::endl
                     << "Sector mode in 2018 S1, Full eTOF mode in 2018 S2"
                     << FairLogger::endl;
}
void CbmTofStarEventBuilder2018::SetTimeSortOutput( Bool_t bTimeSort )
{
   fbTimeSortOutput = bTimeSort;
   if( fbTimeSortOutput )
      LOG(INFO) << "Output buffer will be time sorted before being sent to STAR DAQ " << FairLogger::endl
                << "=> This will have an effect (build time) only in event building mode with all links in one subevent "
                << " as data for single links are already time sorted"
                << FairLogger::endl;
      else LOG(INFO) << "Output buffer will NOT be time sorted before being sent to STAR DAQ " << FairLogger::endl
                     << "=> This will have an effect (build time) only in event building mode with all links in one subevent "
                     << " as data for single links are already time sorted"
                     << FairLogger::endl;
}

void CbmTofStarEventBuilder2018::CbmTofStarEventBuilder2018::CreateHistograms()
{
   LOG(INFO) << "create Histos for " << fuNrOfGdpbs <<" gDPBs "
	          << FairLogger::endl;

#ifdef USE_HTTP_SERVER
   THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
#endif

   TString name { "" };
   TString title { "" };

   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
   {
      /// Event building
      name = Form("StarHitToTrigAll_gDPB_%02u", uGdpb);
      title = Form("Time to trigger for all hits gDPB %02u; t(Hit) - t(Trigg) [ns]", uGdpb);
      fhStarHitToTrigAll_gDPB.push_back(
         new TH1I( name.Data(), title.Data(),
                   5000, -100000.0, 50000.0) ); // TODO make offset parameter
//                   4000, -800000.0, 0.0) ); // TODO make offset parameter
#ifdef USE_HTTP_SERVER
      if (server)
         server->Register("/StarRaw", fhStarHitToTrigAll_gDPB[ uGdpb ] );
#endif

      name = Form("StarHitToTrigWin_gDPB_%02u", uGdpb);
      title = Form("Time to trigger for hits in trigger window gDPB %02u; t(Hit) - t(Trigg) [ns]", uGdpb);
      UInt_t uNbBins = static_cast< UInt_t >( fdStarTriggerWinSize[uGdpb] / 10.0 );
      Double_t dLowBin  = -fdStarTriggerDelay[uGdpb];
      Double_t dHighBin = -fdStarTriggerDelay[uGdpb] + fdStarTriggerWinSize[uGdpb];
      fhStarHitToTrigWin_gDPB.push_back(
         new TH1I( name.Data(), title.Data(),
                   uNbBins, dLowBin, dHighBin) ); // TODO make size parameter
#ifdef USE_HTTP_SERVER
      if (server)
         server->Register("/StarRaw", fhStarHitToTrigWin_gDPB[ uGdpb ] );
#endif

      /// STAR Trigger decoding and monitoring
      name = Form( "hTokenMsgType_gDPB_%02u", uGdpb);
      title = Form( "STAR trigger Messages type gDPB %02u; Type ; Counts", uGdpb);
      fhTokenMsgType[ uGdpb ] =  new TH1F(name, title, 4, 0, 4);
      fhTokenMsgType[ uGdpb ]->GetXaxis()->SetBinLabel( 1, "A"); // gDPB TS high
      fhTokenMsgType[ uGdpb ]->GetXaxis()->SetBinLabel( 2, "B"); // gDPB TS low, STAR TS high
      fhTokenMsgType[ uGdpb ]->GetXaxis()->SetBinLabel( 3, "C"); // STAR TS mid
      fhTokenMsgType[ uGdpb ]->GetXaxis()->SetBinLabel( 4, "D"); // STAR TS low, token, CMDs
#ifdef USE_HTTP_SERVER
      if (server)
         server->Register("/StarRaw", fhTokenMsgType[ uGdpb ] );
#endif

      name = Form( "hTriggerRate_gDPB_%02u", uGdpb);
      title = Form( "STAR trigger signals per second gDPB %02u; Time[s] ; Counts", uGdpb);
      fhTriggerRate[ uGdpb ] =  new TH1F(name, title, fuHistoryHistoSize, 0, fuHistoryHistoSize);
#ifdef USE_HTTP_SERVER
      if (server)
         server->Register("/StarRaw", fhTriggerRate[ uGdpb ] );
#endif

      name = Form( "hCmdDaqVsTrig_gDPB_%02u", uGdpb);
      title = Form( "STAR daq command VS STAR trigger command gDPB %02u; DAQ ; TRIGGER", uGdpb);
      fhCmdDaqVsTrig[ uGdpb ] =  new TH2I(name, title, 16, 0, 16, 16, 0, 16 );
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 1, "0x0: no-trig "); // idle link
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 2, "0x1: clear   "); // clears redundancy counters on the readout boards
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 3, "0x2: mast-rst"); // general reset of the whole front-end logic
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 4, "0x3: spare   "); // reserved
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 5, "0x4: trigg. 0"); // Default physics readout, all det support required
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 6, "0x5: trigg. 1"); //
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 7, "0x6: trigg. 2"); //
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 8, "0x7: trigg. 3"); //
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel( 9, "0x8: puls.  0"); //
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(10, "0x9: puls.  1"); //
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(11, "0xA: puls.  2"); //
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(12, "0xB: puls.  3"); //
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(13, "0xC: config  "); // housekeeping trigger: return geographic info of FE
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(14, "0xD: abort   "); // aborts and clears an active event
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(15, "0xE: L1accept"); //
      fhCmdDaqVsTrig[ uGdpb ]->GetXaxis()->SetBinLabel(16, "0xF: L2accept"); //
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 1, "0x0:  0"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 2, "0x1:  1"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 3, "0x2:  2"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 4, "0x3:  3"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 5, "0x4:  4"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 6, "0x5:  5"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 7, "0x6:  6"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 8, "0x7:  7"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel( 9, "0x8:  8"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(10, "0x9:  9"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(11, "0xA: 10"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(12, "0xB: 11"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(13, "0xC: 12"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(14, "0xD: 13"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(15, "0xE: 14"); // To be filled at STAR
      fhCmdDaqVsTrig[ uGdpb ]->GetYaxis()->SetBinLabel(16, "0xF: 15"); // To be filled at STAR
#ifdef USE_HTTP_SERVER
      if (server)
         server->Register("/StarRaw", fhCmdDaqVsTrig[ uGdpb ] );
#endif

      name = Form( "hStarTokenEvo_gDPB_%02u", uGdpb);
      title = Form( "STAR token value VS time gDPB %02u; Time in Run [s] ; STAR Token; Counts", uGdpb);
      fhStarTokenEvo[ uGdpb ] =  new TH2I(name, title, fuHistoryHistoSize, 0, fuHistoryHistoSize, 410, 0, 4100 ); // 4096
#ifdef USE_HTTP_SERVER
      if (server)
         server->Register("/StarRaw", fhStarTokenEvo[ uGdpb ] );
#endif

      /// Check if we are in "single link per sub-event" building mode
      if( kFALSE == fbEventBuilding )
      {
         name = Form("StarEventSize_gDPB_%02u", uGdpb);
         title = Form("STAR SubEvent size gDPB %02u; SubEvent size [bytes]", uGdpb);
         uNbBins = static_cast< UInt_t >( CbmTofStarSubevent::GetMaxOutputSize() / 8 ); // 1 bin = 1 long 64b uint
         fhStarEventSize_gDPB.push_back(
            new TH1I( name.Data(), title.Data(),
                      uNbBins, 0.0, CbmTofStarSubevent::GetMaxOutputSize() ) );
#ifdef USE_HTTP_SERVER
         if (server)
            server->Register("/StarRaw", fhStarEventSize_gDPB[ uGdpb ] );
#endif

         name = Form("StarEventSizeTime_gDPB_%02u", uGdpb);
         title = Form("STAR SubEvent size gDPB %02u; run time [s]; SubEvent size [bytes]", uGdpb);
         uNbBins = static_cast< UInt_t >( CbmTofStarSubevent::GetMaxOutputSize()
                                               / (sizeof( ngdpb::Message )) );
         fhStarEventSizeTime_gDPB.push_back(
            new TH2I( name.Data(), title.Data(),
                      fuHistoryHistoSize, 0.0, fuHistoryHistoSize,
                      uNbBins, 0.0, CbmTofStarSubevent::GetMaxOutputSize() ) );
#ifdef USE_HTTP_SERVER
         if (server)
            server->Register("/StarRaw", fhStarEventSizeTime_gDPB[ uGdpb ] );
#endif

         name = Form("StarEventSizeTimeLong_gDPB_%02u", uGdpb);
         title = Form("STAR SubEvent size gDPB %02u; run time [min]; SubEvent size [bytes]", uGdpb);
         uNbBins = static_cast< UInt_t >( CbmTofStarSubevent::GetMaxOutputSize()
                                               / (sizeof( ngdpb::Message )) );
         fhStarEventSizeTimeLong_gDPB.push_back(
            new TH2I( name.Data(), title.Data(),
                      fuHistoryHistoSizeLong, 0.0, fuHistoryHistoSizeLong,
                      uNbBins, 0.0, CbmTofStarSubevent::GetMaxOutputSize() ) );
#ifdef USE_HTTP_SERVER
         if (server)
            server->Register("/StarRaw", fhStarEventSizeTimeLong_gDPB[ uGdpb ] );
#endif
      } // if( kFALSE == fbEventBuilding )
         else
         {

            name = Form("StarTrigTimeToMeanTrig_gDPB_%02u", uGdpb);
            title = Form("Time difference between gDPB %02u trigger and mean trigger; dT [ns]; counts []", uGdpb);
            uNbBins = static_cast< UInt_t >( CbmTofStarSubevent::GetMaxOutputSize()
                                                  / (sizeof( ngdpb::Message )) );
            fhStarTrigTimeToMeanTrig_gDPB.push_back(
               new TH1I( name.Data(), title.Data(),
                         625, -312.5, 312.5 ) );
#ifdef USE_HTTP_SERVER
            if (server)
               server->Register("/StarRaw", fhStarTrigTimeToMeanTrig_gDPB[ uGdpb ] );
#endif
         } // else of if( kFALSE == fbEventBuilding )
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)

   /// Check if we are in "single sub-event for all links" building mode
   if( kTRUE == fbEventBuilding )
   {
      name = "StarEventSize_gDPB";
      title = "STAR SubEvent size for all gDPBs; SubEvent size [bytes]";
      UInt_t uNbBins = static_cast< UInt_t >( CbmTofStarSubevent::GetMaxOutputSize() / 8 ); // 1 bin = 1 long 64b uint
      fhStarEventSize = new TH1I( name.Data(), title.Data(), uNbBins, 0.0, CbmTofStarSubevent::GetMaxOutputSize() );
#ifdef USE_HTTP_SERVER
      if (server)
         server->Register("/StarRaw", fhStarEventSize );
#endif

      name = "StarEventSizeTime";
      title = "STAR SubEvent size for all gDPBS; run time [s]; SubEvent size [bytes]";
      uNbBins = static_cast< UInt_t >( CbmTofStarSubevent::GetMaxOutputSize()
                                            / (sizeof( ngdpb::Message )) );
      fhStarEventSizeTime = new TH2I( name.Data(), title.Data(),
                   fuHistoryHistoSize, 0.0, fuHistoryHistoSize,
                   uNbBins, 0.0, CbmTofStarSubevent::GetMaxOutputSize() );
#ifdef USE_HTTP_SERVER
      if (server)
         server->Register("/StarRaw", fhStarEventSizeTime );
#endif

      name = "StarEventSizeTimeLong";
      title = "STAR SubEvent size for all gDPB; run time [min]; SubEvent size [bytes]";
      uNbBins = static_cast< UInt_t >( CbmTofStarSubevent::GetMaxOutputSize()
                                            / (sizeof( ngdpb::Message )) );
      fhStarEventSizeTimeLong = new TH2I( name.Data(), title.Data(),
                   fuHistoryHistoSizeLong, 0.0, fuHistoryHistoSizeLong,
                   uNbBins, 0.0, CbmTofStarSubevent::GetMaxOutputSize() );
#ifdef USE_HTTP_SERVER
      if (server)
         server->Register("/StarRaw", fhStarEventSizeTimeLong );
#endif
   } // if( kFALSE == fbEventBuilding )

   /** Create summary Canvases for STAR 2017 **/
   Double_t w = 10;
   Double_t h = 10;
   /*****************************/


   /** Create STAR token Canvas for STAR 2017 **/
   for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   {
      TCanvas* cStarToken = new TCanvas( Form("cStarToken_g%02u", uGdpb),
                                           Form("STAR token detection info for gDPB %02u", uGdpb),
                                           w, h);
      cStarToken->Divide( 2, 2 );

      cStarToken->cd(1);
      fhTriggerRate[uGdpb]->Draw();

      cStarToken->cd(2);
      fhCmdDaqVsTrig[uGdpb]->Draw( "colz" );

      cStarToken->cd(3);
      fhStarTokenEvo[uGdpb]->Draw();
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   /*****************************/

   /** Create Event building mode Canvas(es) for STAR 2017 **/
   if( kTRUE == fbEventBuilding )
   {
      TCanvas* cStarEvtBuild = new TCanvas( "cStarEvtAll",
                                           "STAR SubEvent Building for all gDPBs",
                                           w, h);
      cStarEvtBuild->Divide( 2, 2 );

      cStarEvtBuild->cd(1);
      gPad->SetLogx();
      gPad->SetLogy();
      fhStarEventSize->Draw();

      cStarEvtBuild->cd(2);
      gPad->SetLogy();
      gPad->SetLogz();
      fhStarEventSizeTime->Draw( "colz" );

      cStarEvtBuild->cd(3);
      gPad->SetLogy();
      gPad->SetLogz();
      fhStarEventSizeTimeLong->Draw( "colz" );
   } // if( kTRUE == fbEventBuilding )

   for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   {
      TCanvas* cStarEvtBuild = new TCanvas( Form("cStarEvt_g%02u", uGdpb),
                                           Form("STAR SubEvent Building for gDPB %02u", uGdpb),
                                           w, h);
      if( kTRUE == fbEventBuilding )
         cStarEvtBuild->Divide( 2 );
         else cStarEvtBuild->Divide( 2, 2 );

      cStarEvtBuild->cd(1);
      fhStarHitToTrigAll_gDPB[uGdpb]->Draw();

      cStarEvtBuild->cd(2);
      fhStarHitToTrigWin_gDPB[uGdpb]->Draw();

      if( kFALSE == fbEventBuilding )
      {
         cStarEvtBuild->cd(3);
         gPad->SetLogx();
         gPad->SetLogy();
         fhStarEventSize_gDPB[uGdpb]->Draw();

         cStarEvtBuild->cd(4);
         gPad->SetLogy();
         gPad->SetLogz();
         fhStarEventSizeTime_gDPB[uGdpb]->Draw( "colz" );
      } // if( kFALSE == fbEventBuilding )
         else
         {
            cStarEvtBuild->cd(3);
            gPad->SetLogy();
            fhStarTrigTimeToMeanTrig_gDPB[uGdpb]->Draw();
         } // else of if( kFALSE == fbEventBuilding )
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   /*****************************/

}

Bool_t CbmTofStarEventBuilder2018::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   LOG(DEBUG) << "Timeslice contains " << ts.num_microslices(component)
              << " microslices of component " << component << FairLogger::endl;

   /// Printout of nb star events log
   std::chrono::time_point<std::chrono::system_clock> timeCurrent = std::chrono::system_clock::now();
   std::chrono::duration<double> elapsed_seconds = timeCurrent - fTimeLastPrintoutNbStarEvent;
   if( 0 == fTimeLastPrintoutNbStarEvent.time_since_epoch().count() )
   {
      fTimeLastPrintoutNbStarEvent = timeCurrent;
      fulNbBuiltSubEventLastPrintout = fulNbBuiltSubEvent;
      fulNbStarSubEventLastPrintout  = fulNbStarSubEvent;
   } // if( 0 == fTimeLastPrintoutNbStarEvent.time_since_epoch().count() )
   else if( 30 < elapsed_seconds.count() )
   {
      std::time_t cTimeCurrent = std::chrono::system_clock::to_time_t( timeCurrent );
      char tempBuff[80];
      std::strftime( tempBuff, 80, "%F %T", localtime (&cTimeCurrent) );

      LOG(INFO) << "CbmTofStarEventBuilder2018::DoUnpack => " << tempBuff
               << " Total number of Built events: " << std::setw(9) << fulNbBuiltSubEvent
               << ", " << std::setw(9) << (fulNbBuiltSubEvent - fulNbBuiltSubEventLastPrintout)
               << " events in last " << std::setw(4) << elapsed_seconds.count() << " s"
               << FairLogger::endl;
      fTimeLastPrintoutNbStarEvent = timeCurrent;
      fulNbBuiltSubEventLastPrintout   = fulNbBuiltSubEvent;

      LOG(INFO) << "CbmTofStarEventBuilder2018::DoUnpack => " << tempBuff
               << " Total number of events sent to STAR: " << std::setw(9) << fulNbStarSubEvent
               << ", " << std::setw(9) << (fulNbStarSubEvent - fulNbStarSubEventLastPrintout)
               << " events in last " << std::setw(4) << elapsed_seconds.count() << " s"
               << FairLogger::endl;
      fTimeLastPrintoutNbStarEvent = timeCurrent;
      fulNbStarSubEventLastPrintout   = fulNbStarSubEvent;

      if( kTRUE == fbEventBuilding )
      {
      } // if( kTRUE == fbEventBuilding )
         else LOG(INFO) << "CbmTofStarEventBuilder2018::DoUnpack => " << tempBuff
                        << " Buffers size: messages " << std::setw(9) << fvmCurrentLinkBuffer.size()
                        << " triggers " << std::setw(9) << fvtCurrentLinkBuffer.size()
                        << FairLogger::endl;

      SaveAllHistos( "data/histos_test.root" );
   } // else if( 300 < elapsed_seconds.count() )

   // Loop over microslices
   Int_t iMessageType = -111;
   size_t numCompMsInTs = ts.num_microslices(component);
   for( fuCurrentMs = 0; fuCurrentMs < numCompMsInTs; ++fuCurrentMs )
   {
      // Jump some microslices if needed
//      if( fuMsAcceptsPercent < m)
//         continue;

      // Ignore overlap ms if number defined by user
      // => Should be dealt with in the methods for each message type
//      if( numCompMsInTs - fuOverlapMsNb <= fuCurrentMs )
//         continue;

      constexpr uint32_t kuBytesPerMessage = 8;

      auto msDescriptor = ts.descriptor(component, fuCurrentMs);
      fEquipmentId = msDescriptor.eq_id;
      fdMsIndex = static_cast<double>(msDescriptor.idx);
      const uint8_t* msContent = reinterpret_cast< const uint8_t* >( ts.content(component, fuCurrentMs) );

      uint32_t size = msDescriptor.size;
      if( 0 < size )
         LOG(DEBUG1) << "Microslice "<< fuCurrentMs <<": " << fdMsIndex
                     << " has size: " << size << FairLogger::endl;

      if( fdStartTimeMsSz < 0 )
         fdStartTimeMsSz = (1e-9) * fdMsIndex;

      // If not integer number of message in input buffer, print warning/error
      if( 0 != (size % kuBytesPerMessage) )
         LOG(ERROR) << "The input microslice buffer does NOT "
                    << "contain only complete nDPB messages!"
                    << FairLogger::endl;

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
         gdpb::Message mess( ulData );

         if( gLogger->IsLogNeeded(DEBUG2) )
         {
            mess.printDataCout();
         } // if( gLogger->IsLogNeeded(DEBUG2) )

         // Increment counter for different message types
         iMessageType = mess.getMessageType();
         fMsgCounter[ iMessageType ]++;

         if( 0 == uIdx )
         {
            fuGdpbId = mess.getRocNumber();

            /// if unmapped gDPB, just ignore this block
            if( fGdpbIdIndexMap.end() == fGdpbIdIndexMap.find( fuGdpbId ) )
               break;

            fuGdpbNr = fGdpbIdIndexMap[ fuGdpbId ];
         } // if( 0 = uIdx )

         fuGet4Id = mess.getGdpbGenChipId();
         fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + fuGet4Id;

         if( fuNrOfGet4PerGdpb <= fuGet4Id &&
             gdpb::MSG_STAR_TRI != iMessageType &&
             get4v2x::kuChipIdMergedEpoch != fuGet4Id )
            LOG(WARNING) << "Message with Get4 ID too high: " << fuGet4Id
                         << " VS " << fuNrOfGet4PerGdpb << " set in parameters." << FairLogger::endl;

         /// First message in MS should ALWAYS be a merged epoch
         if( 0 == uIdx )
         {
            if( gdpb::MSG_EPOCH2 != iMessageType && get4v2x::kuChipIdMergedEpoch == fuGet4Id )
               LOG(WARNING) << "First message in MS is not a merged epoch!!!!!"
                                 << FairLogger::endl;
         } // if( 0 == uIdx )

         switch( iMessageType )
         {
            case gdpb::MSG_GET4:
            {
               //           FillEpochInfo(mess);
               LOG(ERROR) << "Message type " << iMessageType
                          << " not included in unpacker."
                          << FairLogger::endl;
               break;
            } // case old non tof messages
            case gdpb::MSG_EPOCH2:
            {
               if( get4v2x::kuChipIdMergedEpoch == fuGet4Id )
               {
                  /// Propagate for all ASICs corresponding to this merged epoch
                  for( uint32_t uGet4Index = 0; uGet4Index < fuNrOfGet4PerGdpb; uGet4Index ++ )
                  {
                     gdpb::Message tmpMess( mess );
                     tmpMess.setGdpbGenChipId( uGet4Index );
                     fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + uGet4Index;

                     FillEpochInfo( tmpMess );
                  } // for( uint32_t uGet4Index = 0; uGet4Index < fuNrOfGet4PerGdpb; uGetIndex ++ )

                  /// Keep track of extended epoch index for each gDPB
                  // TO BE DONE

                  /// Save the starting time of the TS or of the MS
                  if( 0 == uIdx )
                  {
                     if( kTRUE == fbEventBuilding )
                     {
                        gdpb::FullMessage fullMess( mess, mess.getGdpbEpEpochNb() );
                        fdCurrentTsStartTime = fullMess.GetFullTimeNs();
                        fdCurrentTsCoreEndTime = fdCurrentTsStartTime + fdTsCoreSizeInNs;
                     } // if( kTRUE == fbEventBuilding )
                        else
                        {
                           gdpb::FullMessage fullMess( mess, mess.getGdpbEpEpochNb() );
                           fdCurrentMsStartTime = fullMess.GetFullTimeNs();
                           fdCurrentMsEndTime   = fdCurrentMsStartTime + fdMsSizeInNs;
                        } // else of if( kTRUE == fbEventBuilding )
                  } // if( 0 == uIdx )
               } // if this epoch message is a merged one valiud for all chips
                  else
                  {
//                     fHistGet4MessType->Fill( fuGet4Nr, gdpb::GET4_32B_EPOCH );
                     FillEpochInfo(mess);
                  } // if single chip epoch message
               break;
            } // case gdpb::MSG_EPOCH2:
            case gdpb::MSG_GET4_32B:
            {
               FillHitInfo( mess );
/// TODO clean epoch suppression in STAR 2018!
//               fvmEpSupprBuffer[ fuGet4Nr ].push_back( mess );
               break;
            } // case gdpb::MSG_GET4_32B:
            case gdpb::MSG_GET4_SLC:
            {
               PrintSlcInfo(mess);
               break;
            } // case gdpb::MSG_GET4_SLC:
            case gdpb::MSG_GET4_SYS:
            {
               PrintSysInfo(mess);
               break;
            } // case gdpb::MSG_GET4_SYS:
            case gdpb::MSG_STAR_TRI:
            {
               FillStarTrigInfo(mess);
               break;
            } // case gdpb::MSG_STAR_TRI:
            default:
            {
               if(100 > iMess++)
                  LOG(ERROR) << "Message ("<<iMess<<") type " << std::hex << std::setw(2)
                             << static_cast< uint16_t >( iMessageType )
                             << " not included in Get4 unpacker."
                             << FairLogger::endl;
               if(100 == iMess)
                  LOG(ERROR) << "Stop reporting MSG errors... "
                             << FairLogger::endl;
            } // default:
         } // switch( iMessageType )

      } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)

      /// Check if we are in "single link per sub-event" building mode
      if( kFALSE == fbEventBuilding )
      {
         /// Make sure that the current MS is not one of the overlap ones
         /// and build sub-events in it if that is the case
         if( fuCurrentMs < fuCoreMs )
            BuildStarEventsSingleLink();
            else
            {
               /// clear message buffer
               fvmCurrentLinkBuffer.clear();

               /// Clear trigger buffer
               fvtCurrentLinkBuffer.clear();
            } // else of if( fuCurrentMs < fuCoreMs )
      } // if( kFALSE == fbEventBuilding )

   } // for( fuCurrentMs = 0; fuCurrentMs < numCompMsInTs; ++fuCurrentMs )

   /// Check if we are in "single sub-event for all links" building mode
   if( kTRUE == fbEventBuilding )
   {
      BuildStarEventsAllLinks();
   } // if( kTRUE == fbEventBuilding )


  return kTRUE;
}

void CbmTofStarEventBuilder2018::FillHitInfo( gdpb::Message mess )
{
   // --- Get absolute time, GET4 ID and channel number
   UInt_t uGet4Id     = mess.getGdpbGenChipId();
   UInt_t uChannel    = mess.getGdpbHitChanId();
   UInt_t uTot        = mess.getGdpbHit32Tot();
   // In 32b mode the coarse counter is already computed back to 112 FTS bins
   // => need to hide its contribution from the Finetime
   // => FTS = Fullt TS modulo 112
   UInt_t uFts        = mess.getGdpbHitFullTs() % 112;

   ULong_t  ulCurEpochGdpbGet4 = fvulCurrentEpoch[ fuGet4Nr ];

   if( kTRUE == fvbFirstEpochSeen[ fuGet4Nr ] )
   {

/// TODO clean epoch suppression in STAR 2018!
/*
      // In Ep. Suppr. Mode, receive following epoch instead of previous
      if( 0 < ulCurEpochGdpbGet4 )
         ulCurEpochGdpbGet4 --;
         else ulCurEpochGdpbGet4 = get4v2x::kuEpochCounterSz; // Catch epoch cycle!
*/
      ULong_t  ulhitTime = mess.getMsgFullTime(  ulCurEpochGdpbGet4 );
      Double_t dHitTime  = mess.getMsgFullTimeD( ulCurEpochGdpbGet4 );
      Double_t dHitTot   = uTot;     // in bins


      UInt_t uFebIdx     = (uGet4Id / fuNrOfGet4PerFeb);
      UInt_t uFullFebIdx = (fuGdpbNr * fuNrOfFebsPerGdpb) + uFebIdx;

      UInt_t uChanInGdpb = uGet4Id * fuNrOfChannelsPerGet4 + uChannel;
      UInt_t uChanInSyst = fuGdpbNr * fuNrOfChannelsPerGdpb + uChanInGdpb;
      if( fUnpackPar->GetNumberOfChannels() < uChanInSyst )
      {
         LOG(ERROR) << "Invalid mapping index " << uChanInSyst
                    << " VS " << fUnpackPar->GetNumberOfChannels()
                    <<", from " << fuGdpbNr
                    <<", " << uGet4Id
                    <<", " << uChannel
                    << FairLogger::endl;
         return;
      } // if( fUnpackPar->GetNumberOfChannels() < uChanUId )

      /// Histograms filling
         /// In Run rate evolution
      if (fdStartTime < 0)
         fdStartTime = dHitTime;
      if (fdStartTimeLong < 0)
         fdStartTimeLong = dHitTime;
         /// Reset the evolution Histogram and the start time when we reach the end of the range
      if( fuHistoryHistoSize < 1e-9 * (dHitTime - fdStartTime) )
      {
         fdStartTime = dHitTime;
      } // if( fuHistoryHistoSize < 1e-9 * (dHitTime - fdStartTime) )
          /// Reset the long evolution Histogram and the start time when we reach the end of the range
      if( fuHistoryHistoSizeLong < 1e-9 * (dHitTime - fdStartTimeLong) )
      {
         fdStartTimeLong = dHitTime;
      } // if( fuHistoryHistoSizeLong < 1e-9 * (dHitTime - fdStartTimeLong) )

      gdpb::FullMessage fullMess( mess, ulCurEpochGdpbGet4 );
      if( kTRUE == fbEventBuilding )
      {
         if( fuCurrentMs < fuCoreMs )
            fvmTsLinksBuffer[fuGdpbNr].push_back( fullMess );
            else fvmTsOverLinksBuffer[fuGdpbNr].push_back( fullMess );
      } // if( kTRUE == fbEventBuilding )
         else
            fvmCurrentLinkBuffer.push_back( fullMess );

   } // if( kTRUE == fvbFirstEpochSeen[ fuGet4Nr ] )
}

void CbmTofStarEventBuilder2018::FillEpochInfo( gdpb::Message mess )
{
   ULong64_t ulEpochNr = mess.getGdpbEpEpochNb();

   fvulCurrentEpoch[ fuGet4Nr ] = ulEpochNr;

   if( kFALSE == fvbFirstEpochSeen[ fuGet4Nr ] )
      fvbFirstEpochSeen[ fuGet4Nr ] = kTRUE;

   fulCurrentEpochTime = mess.getMsgFullTime(ulEpochNr);
   fNofEpochs++;

   /// In Ep. Suppr. Mode, receive following epoch instead of previous
   /// Re-align the epoch number of the message in case it will be used later:
   /// We received the epoch after the data instead of the one before!
   /// TODO FIXME: commented out for the time being so that the order in the STAR
   ///             date is the same as in the CBM data
/*
   if( 0 < ulEpochNr )
      mess.setEpoch2Number( ulEpochNr - 1 );
   else mess.setEpoch2Number( get4v2x::kuEpochCounterSz );
*/

/// TODO clean epoch suppression in STAR 2018!
/*
   Int_t iBufferSize = fvmEpSupprBuffer[ fuGet4Nr ].size();
   if( 0 < iBufferSize )
   {
      LOG(DEBUG) << "Now processing stored messages for for get4 " << fuGet4Nr << " with epoch number "
                 << (fvulCurrentEpoch[ fuGet4Nr ] - 1) << FairLogger::endl;

      for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
      {
         FillHitInfo( fvmEpSupprBuffer[ fuGet4Nr ][ iMsgIdx ] );
      } // for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )

      fvmEpSupprBuffer[fuGet4Nr].clear();
   } // if( 0 < fvmEpSupprBuffer[fGet4Nr] )
*/
   gdpb::FullMessage fullMess( mess, ulEpochNr );
   if( kTRUE == fbEventBuilding )
   {
      if( fuCurrentMs < fuCoreMs )
         fvmTsLinksBuffer[fuGdpbNr].push_back( fullMess );
         else fvmTsOverLinksBuffer[fuGdpbNr].push_back( fullMess );
   } // if( kTRUE == fbEventBuilding )
      else
         fvmCurrentLinkBuffer.push_back( fullMess );
}

void CbmTofStarEventBuilder2018::PrintSlcInfo(gdpb::Message mess)
{
   /// Nothing to do, maybe later use it to trakc parameter changes like treshold?
/*
  if( fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find( rocId ) )
     LOG(INFO) << "GET4 Slow Control message, epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][get4Id])
                << ", time " << std::setprecision(9) << std::fixed
                << Double_t(fulCurrentEpochTime) * 1.e-9 << " s "
                << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
                << FairLogger::endl
                << " +++++++ > Chip = " << std::setw(2) << mess.getGdpbGenChipId()
                << ", Chan = " << std::setw(1) << mess.getGdpbSlcChan()
                << ", Edge = " << std::setw(1) << mess.getGdpbSlcEdge()
                << ", Type = " << std::setw(1) << mess.getGdpbSlcType()
                << ", Data = " << std::hex << std::setw(6) << mess.getGdpbSlcData() << std::dec
                << ", Type = " << mess.getGdpbSlcCrc()
                << FairLogger::endl;
*/
   gdpb::FullMessage fullMess( mess, fulCurrentEpochTime );
   if( kTRUE == fbEventBuilding )
   {
      if( fuCurrentMs < fuCoreMs )
         fvmTsLinksBuffer[fuGdpbNr].push_back( fullMess );
         else fvmTsOverLinksBuffer[fuGdpbNr].push_back( fullMess );
   } // if( kTRUE == fbEventBuilding )
      else
         fvmCurrentLinkBuffer.push_back( fullMess );

   /// TODO FIXME: Add the error messages to the buffer for inclusion in the STAR event
   ///             NEEDS the proper "<" operator in FullMessage or Message to allow time sorting
}

void CbmTofStarEventBuilder2018::PrintGenInfo(gdpb::Message mess)
{
   Int_t mType        = mess.getMessageType();
   Int_t rocId          = mess.getRocNumber();
   Int_t get4Id     = mess.getGdpbGenChipId();
   Int_t channel    = mess.getGdpbHitChanId();
   uint64_t            uData = mess.getData();
   if(100 > iMess++)
      LOG(INFO) << "Get4 MSG type "<<mType<<" from rocId "<<rocId<<", getId "<<get4Id
                << ", (hit channel) "<<channel<<Form(" hex data %0lx ",uData)
                << FairLogger::endl;
}

void CbmTofStarEventBuilder2018::PrintSysInfo(gdpb::Message mess)
{
/*
      LOG(INFO) << "GET4 System message,       epoch " << (fvulCurrentEpoch[ fuGet4Nr])
                   << ", time " << std::setprecision(9) << std::fixed
                   << Double_t(fulCurrentEpochTime) * 1.e-9 << " s "
                   << " for board ID " << std::hex << std::setw(4) << fuGdpbId << std::dec
                   << FairLogger::endl;
*/

   switch( mess.getGdpbSysSubType() )
   {
      case gdpb::SYSMSG_GET4_EVENT:
      {
/*
            LOG(INFO) << " +++++++ > Chip = " << std::setw(2) << mess.getGdpbGenChipId()
                      << ", Chan = " << std::setw(1) << mess.getGdpbSysErrChanId()
                      << ", Edge = " << std::setw(1) << mess.getGdpbSysErrEdge()
                      << ", Empt = " << std::setw(1) << mess.getGdpbSysErrUnused()
                      << ", Data = " << std::hex << std::setw(2) << mess.getGdpbSysErrData() << std::dec
                      << " -- GET4 V1 Error Event"
                      << FairLogger::endl;
*/
         /// TODO FIXME: Add the error messages to the buffer for inclusion in the STAR event
         ///             NEEDS the proper "<" operator in FullMessage or Message to allow time sorting
         gdpb::FullMessage fullMess( mess, fulCurrentEpochTime );
         if( kTRUE == fbEventBuilding )
         {
            if( fuCurrentMs < fuCoreMs )
               fvmTsLinksBuffer[fuGdpbNr].push_back( fullMess );
               else fvmTsOverLinksBuffer[fuGdpbNr].push_back( fullMess );
         } // if( kTRUE == fbEventBuilding )
            else
               fvmCurrentLinkBuffer.push_back( fullMess );
         break;
      } //
      case gdpb::SYSMSG_CLOSYSYNC_ERROR:
         if(100 > iMess++)
            LOG(INFO) << "Closy synchronization error" << FairLogger::endl;
         break;
      case gdpb::SYSMSG_TS156_SYNC:
         if(100 > iMess++)
            LOG(INFO) << "156.25MHz timestamp reset" << FairLogger::endl;
         break;
      case gdpb::SYSMSG_GDPB_UNKWN:
         if(100 > iMess++)
            LOG(INFO) << "Unknown GET4 message, data: " << std::hex << std::setw(8)
                      << mess.getGdpbSysUnkwData() << std::dec
                      << FairLogger::endl;
         break;
   } // switch( getGdpbSysSubType() )
}

void CbmTofStarEventBuilder2018::FillStarTrigInfo(gdpb::Message mess)
{
   Int_t iMsgIndex = mess.getStarTrigMsgIndex();

   switch( iMsgIndex )
   {
      case 0:
         fhTokenMsgType[fuGdpbNr]->Fill(0);
         fulGdpbTsMsb[fuGdpbNr] = mess.getGdpbTsMsbStarA();
         break;
      case 1:
         fhTokenMsgType[fuGdpbNr]->Fill(1);
         fulGdpbTsLsb[fuGdpbNr] = mess.getGdpbTsLsbStarB();
         fulStarTsMsb[fuGdpbNr] = mess.getStarTsMsbStarB();
         break;
      case 2:
         fhTokenMsgType[fuGdpbNr]->Fill(2);
         fulStarTsMid[fuGdpbNr] = mess.getStarTsMidStarC();
         break;
      case 3:
      {
         fhTokenMsgType[fuGdpbNr]->Fill(3);

         ULong64_t ulNewGdpbTsFull = ( fulGdpbTsMsb[fuGdpbNr] << 24 )
                                   + ( fulGdpbTsLsb[fuGdpbNr]       );
         ULong64_t ulNewStarTsFull = ( fulStarTsMsb[fuGdpbNr] << 48 )
                                   + ( fulStarTsMid[fuGdpbNr] <<  8 )
                                   + mess.getStarTsLsbStarD();
         UInt_t uNewToken  = mess.getStarTokenStarD();
         UInt_t uNewDaqCmd  = mess.getStarDaqCmdStarD();
         UInt_t uNewTrigCmd = mess.getStarTrigCmdStarD();
         if( ( uNewToken == fuStarTokenLast[fuGdpbNr] ) && ( ulNewGdpbTsFull == fulGdpbTsFullLast[fuGdpbNr] ) &&
             ( ulNewStarTsFull == fulStarTsFullLast[fuGdpbNr] ) && ( uNewDaqCmd == fuStarDaqCmdLast[fuGdpbNr] ) &&
             ( uNewTrigCmd == fuStarTrigCmdLast[fuGdpbNr] ) )
         {
            LOG(DEBUG) << "Possible error: identical STAR tokens found twice in a row => ignore 2nd! "
                         << " gDBB #" << fuGdpbNr << " "
                         << Form("token = %5u ", fuStarTokenLast[fuGdpbNr] )
                         << Form("gDPB ts  = %12llu ", fulGdpbTsFullLast[fuGdpbNr] )
                         << Form("STAR ts = %12llu ", fulStarTsFullLast[fuGdpbNr] )
                         << Form("DAQ cmd = %2u ", fuStarDaqCmdLast[fuGdpbNr] )
                         << Form("TRG cmd = %2u ", fuStarTrigCmdLast[fuGdpbNr] )
                         << FairLogger::endl;
            return;
         } // if exactly same message repeated

         if( (uNewToken != fuStarTokenLast[fuGdpbNr] + 1) &&
             0 < fulGdpbTsFullLast[fuGdpbNr] && 0 < fulStarTsFullLast[fuGdpbNr] &&
             ( 4095 != fuStarTokenLast[fuGdpbNr] || 1 != uNewToken)  )
            LOG(WARNING) << "Possible error: STAR token did not increase by exactly 1! "
                         << " gDBB #" << fuGdpbNr << " "
                         << Form("old = %5u vs new = %5u ", fuStarTokenLast[fuGdpbNr],   uNewToken)
                         << Form("old = %12llu vs new = %12llu ", fulGdpbTsFullLast[fuGdpbNr], ulNewGdpbTsFull)
                         << Form("old = %12llu vs new = %12llu ", fulStarTsFullLast[fuGdpbNr], ulNewStarTsFull)
                         << Form("old = %2u vs new = %2u ", fuStarDaqCmdLast[fuGdpbNr],  uNewDaqCmd)
                         << Form("old = %2u vs new = %2u ", fuStarTrigCmdLast[fuGdpbNr], uNewTrigCmd)
                         << FairLogger::endl;


         // STAR TS counter reset detection
         if( ulNewStarTsFull < fulStarTsFullLast[fuGdpbNr] )
            LOG(DEBUG) << "Probable reset of the STAR TS: old = " << Form("%16llu", fulStarTsFullLast[fuGdpbNr])
                       << " new = " << Form("%16llu", ulNewStarTsFull)
                       << " Diff = -" << Form("%8llu", fulStarTsFullLast[fuGdpbNr] - ulNewStarTsFull)
                       << FairLogger::endl;

         ULong64_t ulGdpbTsDiff = ulNewGdpbTsFull - fulGdpbTsFullLast[fuGdpbNr];
         fulGdpbTsFullLast[fuGdpbNr] = ulNewGdpbTsFull;
         fulStarTsFullLast[fuGdpbNr] = ulNewStarTsFull;
         fuStarTokenLast[fuGdpbNr]   = uNewToken;
         fuStarDaqCmdLast[fuGdpbNr]  = uNewDaqCmd;
         fuStarTrigCmdLast[fuGdpbNr] = uNewTrigCmd;

         /// Histograms filling
         /// In Run rate evolution
         if( 0 <= fdStartTime )
         {
            /// Reset the evolution Histogram and the start time when we reach the end of the range
            if( fuHistoryHistoSize < 1e-9 * (fulGdpbTsFullLast[fuGdpbNr] * 6.25 - fdStartTime) )
            {
               fdStartTime = fulGdpbTsFullLast[fuGdpbNr] * 6.25;
            } // if( fuHistoryHistoSize < 1e-9 * (fulGdpbTsFullLast * 6.25 - fdStartTime) )

            fhTriggerRate[fuGdpbNr]->Fill( 1e-9 * ( fulGdpbTsFullLast[fuGdpbNr] * 6.25 - fdStartTime ) );
            fhStarTokenEvo[fuGdpbNr]->Fill( 1e-9 * ( fulGdpbTsFullLast[fuGdpbNr] * 6.25 - fdStartTime ), fuStarTokenLast[fuGdpbNr] );
         } // if( 0 < fdStartTime )
            else fdStartTime = fulGdpbTsFullLast[fuGdpbNr] * 6.25;
         fhCmdDaqVsTrig[fuGdpbNr]->Fill( fuStarDaqCmdLast[fuGdpbNr], fuStarTrigCmdLast[fuGdpbNr] );

         /// Generate Trigger object and store it for event building ///
         CbmTofStarTrigger newTrig( fulGdpbTsFullLast[fuGdpbNr], fulStarTsFullLast[fuGdpbNr], fuStarTokenLast[fuGdpbNr],
                                    fuStarDaqCmdLast[fuGdpbNr], fuStarTrigCmdLast[fuGdpbNr] );
         if( kTRUE == fbEventBuilding )
         {
            if( fuCurrentMs < fuCoreMs )
               fvtTsLinksBuffer[fuGdpbNr].push_back( newTrig );
               else fvtTsOverLinksBuffer[fuGdpbNr].push_back( newTrig );
         } // if( kTRUE == fbEventBuilding )
            else
               fvtCurrentLinkBuffer.push_back( newTrig );
         ///---------------------------------------------------------///
         break;
      } // case 3
      default:
         LOG(ERROR) << "Unknown Star Trigger messageindex: " << iMsgIndex << FairLogger::endl;
   } // switch( iMsgIndex )
}

void CbmTofStarEventBuilder2018::Reset()
{
}

void CbmTofStarEventBuilder2018::FillOutput(CbmDigi* /*digi*/)
{
}

void CbmTofStarEventBuilder2018::Finish()
{
   TString message_type;

   for( UInt_t uType = 0; uType < fMsgCounter.size(); ++uType)
   {
      switch(uType)
      {
         case 0:   message_type ="NOP"; break;
         case 1:   message_type ="HIT"; break;
         case 2:   message_type ="EPOCH"; break;
         case 3:   message_type ="SYNC"; break;
         case 4:   message_type ="AUX"; break;
         case 5:   message_type ="EPOCH2"; break;
         case 6:   message_type ="GET4"; break;
         case 7:   message_type ="SYS"; break;
         case 8:   message_type ="GET4_SLC"; break;
         case 9:   message_type ="GET4_32B"; break;
         case 10:  message_type ="GET4_SYS"; break;
         default:  message_type ="UNKNOWN"; break;
      } // switch(uType)
      LOG(INFO) << message_type << " messages: "
                << fMsgCounter[uType] << FairLogger::endl;
   } // for( UInt_t uType = 0; uType < fMsgCounter.size(); ++uType)

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      for( UInt_t uGet4 = 0; uGet4 < fuNrOfGet4PerGdpb; ++uGet4 )
         LOG(INFO) << "Last epoch for gDPB: "<< std::setw(4) << uGdpb
                   << " , GET4  " << std::setw(4) << uGet4
                   << " => " << fvulCurrentEpoch[GetArrayIndex(uGdpb, uGet4)]
                   << FairLogger::endl;
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;

  SaveAllHistos();

}

void CbmTofStarEventBuilder2018::SaveAllHistos( TString sFileName )
{
   TDirectory * oldDir = NULL;
   TFile * histoFile = NULL;
   if( "" != sFileName )
   {
      // Store current directory position to allow restore later
      oldDir = gDirectory;
      // open separate histo file in recreate mode
      histoFile = new TFile( sFileName , "RECREATE");
      histoFile->cd();
   } // if( "" != sFileName )

   gDirectory->mkdir("Tof_Raw_gDPB");
   gDirectory->cd("Tof_Raw_gDPB");
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
   {
      /// Token detection
      fhTokenMsgType[ uGdpb ]->Write();
      fhTriggerRate[ uGdpb ]->Write();
      fhCmdDaqVsTrig[ uGdpb ]->Write();
      fhStarTokenEvo[ uGdpb ]->Write();

      /// Event building
      fhStarHitToTrigAll_gDPB[ uGdpb ]->Write();
      fhStarHitToTrigWin_gDPB[ uGdpb ]->Write();

      if( kFALSE == fbEventBuilding )
      {
         fhStarEventSize_gDPB[ uGdpb ]->Write();
         fhStarEventSizeTime_gDPB[ uGdpb ]->Write();
         fhStarEventSizeTimeLong_gDPB[ uGdpb ]->Write();
      } // if( kFALSE == fbEventBuilding )
         else fhStarTrigTimeToMeanTrig_gDPB[ uGdpb ]->Write();
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)

   if( kTRUE == fbEventBuilding )
   {
      fhStarEventSize->Write();
      fhStarEventSizeTime->Write();
      fhStarEventSizeTimeLong->Write();
   } // if( kTRUE == fbEventBuilding )
   gDirectory->cd("..");

   if( "" != sFileName )
   {
      // Restore original directory position
      histoFile->Close();
      oldDir->cd();
   } // if( "" != sFileName )
   if( "" != sFileName )
   {
      // Restore original directory position
      histoFile->Close();
      oldDir->cd();
   } // if( "" != sFileName )

}

void CbmTofStarEventBuilder2018::ResetAllHistos()
{
   LOG(INFO) << "Reseting all Event building histograms." << FairLogger::endl;

   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
   {
      /// Token detection
      fhTokenMsgType[ uGdpb ]->Reset();
      fhTriggerRate[ uGdpb ]->Reset();
      fhCmdDaqVsTrig[ uGdpb ]->Reset();
      fhStarTokenEvo[ uGdpb ]->Reset();

      /// Event building
      fhStarHitToTrigAll_gDPB[ uGdpb ]->Reset();
      fhStarHitToTrigWin_gDPB[ uGdpb ]->Reset();

      if( kFALSE == fbEventBuilding )
      {
         fhStarEventSize_gDPB[ uGdpb ]->Reset();
         fhStarEventSizeTime_gDPB[ uGdpb ]->Reset();
         fhStarEventSizeTimeLong_gDPB[ uGdpb ]->Reset();
      } // if( kFALSE == fbEventBuilding )
         else fhStarTrigTimeToMeanTrig_gDPB[ uGdpb ]->Reset();
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)

   if( kTRUE == fbEventBuilding )
   {
      fhStarEventSize->Reset();
      fhStarEventSizeTime->Reset();
      fhStarEventSizeTimeLong->Reset();
   } // if( kTRUE == fbEventBuilding )

   fdStartTime = -1;
   fdStartTimeLong = -1;
   fdStartTimeMsSz = -1;
}

void CbmTofStarEventBuilder2018::BuildStarEventsSingleLink()
{

   std::vector< gdpb::FullMessage >::iterator itFirstMessageNextEvent   = fvmCurrentLinkBuffer.begin();
   Double_t dPrevEventEnd = 0.0;

   for( UInt_t uTriggIdx = 0; uTriggIdx < fvtCurrentLinkBuffer.size(); uTriggIdx++ )
   {
      Double_t dTriggerTime   = get4v2x::kdClockCycleSizeNs * fvtCurrentLinkBuffer[uTriggIdx].GetFullGdpbTs();
      Double_t dTriggerWinBeg = dTriggerTime - fdStarTriggerDelay[ fuGdpbNr ];
      Double_t dTriggerWinEnd = dTriggerTime - fdStarTriggerDelay[ fuGdpbNr ] + fdStarTriggerWinSize[ fuGdpbNr ];
      Double_t dClosestTriggerWinStart = dTriggerTime - fdStarTriggerDelay[ fuGdpbNr ] + fdStarTriggerDeadtime[ fuGdpbNr ];
      Bool_t bFirstMessClosestEventFound = kFALSE;

      /// Associate this trigger to its subevent
      fStarSubEvent.SetTrigger( fvtCurrentLinkBuffer[uTriggIdx] );
      /// Set the source index of the subevent
      fStarSubEvent.SetSource( fuGdpbNr );

      /// Check if this trigger may correspond to an event with data in previous MS
      if( dTriggerWinBeg < fdCurrentMsStartTime )
         fStarSubEvent.SetStartBorderEventFlag();

      /// Check if this trigger may correspond to an event with data in next MS
      if( fdCurrentMsEndTime < dTriggerWinEnd )
         fStarSubEvent.SetEndBorderEventFlag();

      /// Check if this trigger leads to an event overlapping with the previous one
      if( 0 < dPrevEventEnd && dTriggerWinBeg < dPrevEventEnd )
         fStarSubEvent.SetOverlapEventFlag();

      for( std::vector< gdpb::FullMessage >::iterator itMess = itFirstMessageNextEvent;
           itMess != fvmCurrentLinkBuffer.end();
           ++ itMess )
      {
         Double_t dMessageFullTime = (*itMess).GetFullTimeNs();
         if( kFALSE == bFirstMessClosestEventFound &&
             dClosestTriggerWinStart < dMessageFullTime )
         {
            itFirstMessageNextEvent = itMess;
            bFirstMessClosestEventFound = kTRUE;
         } // If first possible hit of closest event not found and current hit fits

         /// Plotting of time to trigger for all hits
         fhStarHitToTrigAll_gDPB[ fuGdpbNr ]->Fill( dMessageFullTime - dTriggerTime );

         if( dTriggerWinBeg   <= dMessageFullTime && dMessageFullTime <= dTriggerWinEnd )
         {
            /// Message belongs to this event
            fStarSubEvent.AddMsg( (*itMess) );

            /// Plotting of time to trigger for hits within the event window
            fhStarHitToTrigWin_gDPB[ fuGdpbNr ]->Fill( dMessageFullTime - dTriggerTime );
         } // if( dTriggerWinBeg   <= dMessageFullTime && dMessageFullTime <= dTriggerWinEnd )
            else if( dTriggerWinEnd < dMessageFullTime )
               /// First Message out of the window for this event => Stop there and go to the next
               break;
      } // Loop on message from first allowed by last event + deadtime to end

      /** TODO: clarify how we deal with multiple sub-events (eg one for each gDPB) **/
      /// Send the sub-event to the STAR systems
      Int_t  iBuffSzByte = 0;
      void * pDataBuff = fStarSubEvent.BuildOutput( iBuffSzByte );
      if( NULL != pDataBuff )
      {
         /// Valid output, do stuff with it!
         fulNbBuiltSubEvent++;

#ifdef STAR_SUBEVT_BUILDER
         /*
          ** Function to send sub-event block to the STAR DAQ system
          *       trg_word received is packed as:
          *
          *       trg_cmd|daq_cmd|tkn_hi|tkn_mid|tkn_lo
          */
         star_rhicf_write( fStarSubEvent.GetTrigger().GetStarTrigerWord(),
                           pDataBuff, iBuffSzByte );
         fulNbStarSubEvent++;
/*
         LOG(INFO) << "Sent STAR event with size " << iBuffSzByte << " Bytes"
                   << " and token " << fStarSubEvent.GetTrigger().GetStarToken()
                   << FairLogger::endl;
*/
#endif // STAR_SUBEVT_BUILDER
      } // if( NULL != pDataBuff )
         else LOG(ERROR) << "Invalid STAR SubEvent Output, can only happen if trigger "
                         << " object was not set => Do Nothing more with it!!! "
                         << FairLogger::endl;

      /// Fill plot of event size
      fhStarEventSize_gDPB[ fuGdpbNr ]->Fill( iBuffSzByte );
      /// Fill plot of event size as function of trigger time
      if( 0 < fdStartTime )
         fhStarEventSizeTime_gDPB[ fuGdpbNr ]->Fill( 1e-9 *(dTriggerTime - fdStartTime), iBuffSzByte );
      if( 0 < fdStartTimeLong )
         fhStarEventSizeTimeLong_gDPB[ fuGdpbNr ]->Fill( 1e-9 *(dTriggerTime - fdStartTimeLong) / 60.0, iBuffSzByte/60.0 );

      /// Now clear the sub-event
      fStarSubEvent.ClearSubEvent();
   } // for( UInt_t uTriggIdx = 0; uTriggIdx < fvtCurrentLinkBuffer.size(); uTriggIdx++ )

   /// catch case of MS without triggers
   if( 0 == fvtCurrentLinkBuffer.size() )
      itFirstMessageNextEvent   = fvmCurrentLinkBuffer.end();

   /// Remove message until the first which could fit in an event in next MS
   fvmCurrentLinkBuffer.erase( fvmCurrentLinkBuffer.begin(), itFirstMessageNextEvent);

   /// Clear trigger buffer
   fvtCurrentLinkBuffer.clear();

}
void CbmTofStarEventBuilder2018::BuildStarEventsAllLinks()
{
   /// TODO:
   /// 1) use overlap MS to fill events where the event window would go over the lower edge of the TS

   /// First check if any trigger was present in this TS
   Bool_t bEmptyTs = kTRUE;
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
      if( 0 < fvtTsLinksBuffer[uGdpb].size() )
      {
         bEmptyTs = kFALSE;
         break;
      } // if( 0 < fvtTsLinksBuffer[0].size() )
   if( kTRUE == bEmptyTs )
   {
      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
      {
         fvmTsLinksBuffer[uGdpb].clear();
         fvmTsOverLinksBuffer[uGdpb].clear();
         fvtTsOverLinksBuffer[uGdpb].clear();
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
   } // if( kTRUE == bEmptyTS )

   /// Initialize the iterator for first possible message in event to beginning of buffer
   std::vector< std::vector< gdpb::FullMessage >::iterator > itFirstMessageNextEvent( fuNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
      itFirstMessageNextEvent[ uGdpb ] = fvmTsLinksBuffer[ uGdpb ].begin();

   /// Initialize the time of previous event end to 0
   std::vector< Double_t > dPrevEventEnd( fuNrOfGdpbs, 0.0 );

   /// Checking whether all Trigger buffers start with the same trigger
   Bool_t bFirstTriggerMatch = kTRUE;
   std::vector< UInt_t > vuIndexMatchingTrigger(fuNrOfGdpbs);
   if( 0 < fvtTsLinksBuffer[0].size() )
   {
      vuIndexMatchingTrigger[0] = 0;
//      ULong64_t ulFirstBoardFirstTriggerTime = fvtTsLinksBuffer[0][0].GetFullGdpbTs();
      UInt_t    uFirstBoardFirstTriggerWord  = fvtTsLinksBuffer[0][0].GetStarTrigerWord();

      for( UInt_t uGdpb = 1; uGdpb < fuNrOfGdpbs; uGdpb ++)
      {
         if( 0 < fvtTsLinksBuffer[0].size() )
         {
            vuIndexMatchingTrigger[uGdpb] = 0;
            if( fvtTsLinksBuffer[uGdpb][0].GetStarTrigerWord() != uFirstBoardFirstTriggerWord )
            {
               bFirstTriggerMatch = kFALSE;
               break;
            } // if( fvtTsLinksBuffer[uGdpb][0].GetStarTrigerWord() != uFirstBoardFirstTriggerWord )
         } // if( 0 < fvtTsLinksBuffer[0].size() )
            else bFirstTriggerMatch = kFALSE;
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
   } // if( 0 < fvtTsLinksBuffer[0].size() )
      else
      {
         bFirstTriggerMatch = kFALSE;
      } // else of if( 0 < fvtTsLinksBuffer[0].size() )

   if( kFALSE == bFirstTriggerMatch )
   {
      LOG(ERROR) << "First trigger in TS not the same for all boards!"
                 << FairLogger::endl;
      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
      {
         fvtTsLinksBuffer[uGdpb].clear();
         fvmTsLinksBuffer[uGdpb].clear();
         fvmTsOverLinksBuffer[uGdpb].clear();
         fvtTsOverLinksBuffer[uGdpb].clear();
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
      return;
   } // if( kFALSE == bFirstTriggerMatch )

   /// We now have a matching trigger for starting
   Bool_t bMatchingTriggerFound = kTRUE;
   while( kTRUE == bMatchingTriggerFound )
   {
      Double_t dMeanTriggerGdpbTs = 0;
      Double_t dMeanTriggerStarTs = 0;
      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
      {
         dMeanTriggerGdpbTs += get4v2x::kdClockCycleSizeNs * fvtTsLinksBuffer[uGdpb][ vuIndexMatchingTrigger[uGdpb] ].GetFullGdpbTs() / fuNrOfGdpbs;
         dMeanTriggerStarTs += get4v2x::kdClockCycleSizeNs * fvtTsLinksBuffer[uGdpb][ vuIndexMatchingTrigger[uGdpb] ].GetFullStarTs() / fuNrOfGdpbs;
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
      dMeanTriggerGdpbTs /= fuNrOfGdpbs;
      dMeanTriggerStarTs /= fuNrOfGdpbs;

      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
         fhStarTrigTimeToMeanTrig_gDPB[ uGdpb ]->Fill(
                     get4v2x::kdClockCycleSizeNs * fvtTsLinksBuffer[uGdpb][ vuIndexMatchingTrigger[uGdpb] ].GetFullGdpbTs()
                   - dMeanTriggerGdpbTs );

      CbmTofStarTrigger meanTrigger( static_cast< ULong64_t >( dMeanTriggerGdpbTs ), static_cast< ULong64_t >( dMeanTriggerStarTs ),
                                     fvtTsLinksBuffer[0][ vuIndexMatchingTrigger[0] ].GetStarToken(),
                                     fvtTsLinksBuffer[0][ vuIndexMatchingTrigger[0] ].GetStarDaqCmd(),
                                     fvtTsLinksBuffer[0][ vuIndexMatchingTrigger[0] ].GetStarTrigCmd() );

      /// Associate this trigger to its subevent
      fStarSubEvent.SetTrigger( meanTrigger );
      /// Set the source index of the subevent
      fStarSubEvent.SetSource( 16 );

      for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
      {

         Double_t dTriggerTime   = get4v2x::kdClockCycleSizeNs * fvtTsLinksBuffer[uGdpb][ vuIndexMatchingTrigger[uGdpb] ].GetFullGdpbTs();
         Double_t dTriggerWinBeg = dTriggerTime - fdStarTriggerDelay[ fuGdpbNr ];
         Double_t dTriggerWinEnd = dTriggerTime - fdStarTriggerDelay[ fuGdpbNr ] + fdStarTriggerWinSize[ fuGdpbNr ];
         Double_t dClosestTriggerWinStart = dTriggerTime - fdStarTriggerDelay[ fuGdpbNr ] + fdStarTriggerDeadtime[ fuGdpbNr ];
         Bool_t bFirstMessClosestEventFound = kFALSE;

         /// Check if this trigger may correspond to an event with data in previous TS
         if( dTriggerWinBeg < fdCurrentTsStartTime )
            fStarSubEvent.SetStartBorderEventFlag();

         /// Check if this trigger may correspond to an event with data in next TS
         if( fdCurrentTsCoreEndTime < dTriggerWinEnd )
            fStarSubEvent.SetEndBorderEventFlag();

         /// Check if this trigger leads to an event overlapping with the previous one
         if( 0 < dPrevEventEnd[ fuGdpbNr ] && dTriggerWinBeg < dPrevEventEnd[ fuGdpbNr ] )
            fStarSubEvent.SetOverlapEventFlag();

         for( std::vector< gdpb::FullMessage >::iterator itMess = itFirstMessageNextEvent[ uGdpb ];
              itMess != fvmTsLinksBuffer[ uGdpb ].end();
              ++ itMess )
         {
            Double_t dMessageFullTime = (*itMess).GetFullTimeNs();
            if( kFALSE == bFirstMessClosestEventFound &&
                dClosestTriggerWinStart < dMessageFullTime )
            {
               itFirstMessageNextEvent[ uGdpb ] = itMess;
               bFirstMessClosestEventFound = kTRUE;
            } // If first possible hit of closest event not found and current hit fits

            /// Plotting of time to trigger for all hits
            fhStarHitToTrigAll_gDPB[ fuGdpbNr ]->Fill( dMessageFullTime - dTriggerTime );

            if( dTriggerWinBeg   <= dMessageFullTime && dMessageFullTime <= dTriggerWinEnd )
            {
               /// Message belongs to this event
               fStarSubEvent.AddMsg( (*itMess) );

               /// Plotting of time to trigger for hits within the event window
               fhStarHitToTrigWin_gDPB[ fuGdpbNr ]->Fill( dMessageFullTime - dTriggerTime );
            } // if( dTriggerWinBeg   <= dMessageFullTime && dMessageFullTime <= dTriggerWinEnd )
               else if( dTriggerWinEnd < dMessageFullTime )
                  /// First Message out of the window for this event => Stop there and go to the next
                  break;
         } // Loop on message from first allowed by last event + deadtime to end
      } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)

      /// Time sort the sub-event if required by the user
      if( kTRUE == fbTimeSortOutput )
         fStarSubEvent.SortMessages(  );

      /** TODO: clarify how we deal with multiple sub-events (eg one for each gDPB) **/
      /// Send the sub-event to the STAR systems
      Int_t  iBuffSzByte = 0;
      void * pDataBuff = fStarSubEvent.BuildOutput( iBuffSzByte );
      if( NULL != pDataBuff )
      {
         /// Valid output, do stuff with it!
         fulNbBuiltSubEvent++;

#ifdef STAR_SUBEVT_BUILDER
         /*
          ** Function to send sub-event block to the STAR DAQ system
          *       trg_word received is packed as:
          *
          *       trg_cmd|daq_cmd|tkn_hi|tkn_mid|tkn_lo
          */
         star_rhicf_write( fStarSubEvent.GetTrigger().GetStarTrigerWord(),
                           pDataBuff, iBuffSzByte );
         fulNbStarSubEvent++;
/*
         LOG(INFO) << "Sent STAR event with size " << iBuffSzByte << " Bytes"
                   << " and token " << fStarSubEvent.GetTrigger().GetStarToken()
                   << FairLogger::endl;
*/
#endif // STAR_SUBEVT_BUILDER
      } // if( NULL != pDataBuff )
         else LOG(ERROR) << "Invalid STAR SubEvent Output, can only happen if trigger "
                         << " object was not set => Do Nothing more with it!!! "
                         << FairLogger::endl;

      /// Fill plot of event size
      fhStarEventSize->Fill( iBuffSzByte );
      /// Fill plot of event size as function of trigger time
      if( 0 < fdStartTime )
         fhStarEventSizeTime->Fill( 1e-9 *(dMeanTriggerGdpbTs - fdStartTime), iBuffSzByte );
      if( 0 < fdStartTimeLong )
         fhStarEventSizeTimeLong->Fill( 1e-9 *(dMeanTriggerGdpbTs - fdStartTimeLong) / 60.0, iBuffSzByte/60.0 );

      /// Now clear the sub-event
      fStarSubEvent.ClearSubEvent();

      /// Check if we still can find a Matching trigger
      vuIndexMatchingTrigger[0] ++;
      bMatchingTriggerFound = kTRUE;
      if( vuIndexMatchingTrigger[0] < fvtTsLinksBuffer[0].size() )
      {
//         ULong64_t ulFirstBoardNextTriggerTime = fvtTsLinksBuffer[0][ vuIndexMatchingTrigger[0] ].GetFullGdpbTs();
         UInt_t    uFirstBoardNextTriggerWord  = fvtTsLinksBuffer[0][ vuIndexMatchingTrigger[0] ].GetStarTrigerWord();
         for( UInt_t uGdpb = 1; uGdpb < fuNrOfGdpbs; uGdpb ++)
         {
            vuIndexMatchingTrigger[uGdpb] ++;
            if( vuIndexMatchingTrigger[uGdpb] < fvtTsLinksBuffer[uGdpb].size() )
            {
               if( fvtTsLinksBuffer[uGdpb][ vuIndexMatchingTrigger[uGdpb] ].GetStarTrigerWord() != uFirstBoardNextTriggerWord )
               {
                  bMatchingTriggerFound = kFALSE;
                  break;
               } // if( fvtTsLinksBuffer[uGdpb][ vuIndexMatchingTrigger[uGdpb] ].GetStarTrigerWord() != uFirstBoardNextTriggerWord )
            } // if( vuIndexMatchingTrigger[uGdpb] < fvtTsLinksBuffer[uGdpb].size() )
               else bMatchingTriggerFound = kFALSE;
         } // for( UInt_t uGdpb = 1; uGdpb < fuNrOfGdpbs; uGdpb ++)
      } // if( vuIndexMatchingTrigger[0] < fvtTsLinksBuffer[0].size() )
         else
         {
            bMatchingTriggerFound = kFALSE;
         } // else of if( 0 < fvtTsLinksBuffer[0].size() )

      /// If no matching trigger, check if matching trigger building not possible
      /// using the deadtime period of the overlap MS
      if( kFALSE == bMatchingTriggerFound )
      {
         /// First find out for which gDPBs the trigger is potentially in the overlap MS
         std::vector< Bool_t > bTriggerInOverMs( fuNrOfGdpbs, kFALSE );
         for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
            if( fvtTsLinksBuffer[uGdpb].size() < vuIndexMatchingTrigger[uGdpb] &&
                0 < fvtTsOverLinksBuffer[uGdpb].size() )
               bTriggerInOverMs[uGdpb] = kTRUE;

//         ULong64_t ulFirstBoardNextTriggerTime = 0;
         UInt_t    uFirstBoardNextTriggerWord  = 0;
         bFirstTriggerMatch = kTRUE;
         if( vuIndexMatchingTrigger[0] < fvtTsLinksBuffer[0].size() )
         {
//            ulFirstBoardNextTriggerTime = fvtTsLinksBuffer[0][ vuIndexMatchingTrigger[0] ].GetFullGdpbTs();
            uFirstBoardNextTriggerWord  = fvtTsLinksBuffer[0][ vuIndexMatchingTrigger[0] ].GetStarTrigerWord();
         } // if( vuIndexMatchingTrigger[0] < fvtTsLinksBuffer[0].size() )
            else if( bTriggerInOverMs[0] )
            {
//               ulFirstBoardNextTriggerTime = fvtTsOverLinksBuffer[0][ 0 ].GetFullGdpbTs();
               uFirstBoardNextTriggerWord  = fvtTsOverLinksBuffer[0][ 0 ].GetStarTrigerWord();
            } // else if( bTriggerInOverMs[0] ) of if( vuIndexMatchingTrigger[0] < fvtTsLinksBuffer[0].size() )
            else bFirstTriggerMatch = kFALSE;

         for( UInt_t uGdpb = 1; uGdpb < fuNrOfGdpbs; uGdpb ++)
         {
            if( vuIndexMatchingTrigger[uGdpb] < fvtTsLinksBuffer[uGdpb].size() )
            {
               /// Check if triggger words are matching
               if( fvtTsLinksBuffer[uGdpb][ vuIndexMatchingTrigger[uGdpb] ].GetStarTrigerWord() != uFirstBoardNextTriggerWord )
               {
                  bFirstTriggerMatch = kFALSE;
                  break;
               } // if( fvtTsLinksBuffer[uGdpb][ vuIndexMatchingTrigger[uGdpb] ].GetStarTrigerWord() != uFirstBoardNextTriggerWord )
            } // if( vuIndexMatchingTrigger[uGdpb] < fvtTsLinksBuffer[uGdpb].size() )
               else if( bTriggerInOverMs[uGdpb] )
               {
                  /// Check if triggger words are matching
                  if( fvtTsOverLinksBuffer[uGdpb][ 0 ].GetStarTrigerWord() != uFirstBoardNextTriggerWord )
                  {
                     bFirstTriggerMatch = kFALSE;
                     break;
                  } // if( fvtTsOverLinksBuffer[uGdpb][ 0 ].GetStarTrigerWord() != uFirstBoardNextTriggerWord )
                  /// Check if this trigger is still without the period declared as TS deadtime
                  if( fdCurrentTsCoreEndTime + fdTsDeadtimePeriod < fvtTsOverLinksBuffer[uGdpb][ 0 ].GetFullGdpbTs() )
                  {
                     bFirstTriggerMatch = kFALSE;
                     break;
                  } // if( fdCurrentTsCoreEndTime + fdTsDeadtimePeriod < fvtTsOverLinksBuffer[uGdpb][ 0 ].GetFullGdpbTs() )
               } // else if( bTriggerInOverMs[uGdpb] ) of if( vuIndexMatchingTrigger[uGdpb] < fvtTsLinksBuffer[uGdpb].size() )
               else bFirstTriggerMatch = kFALSE;
         } // for( UInt_t uGdpb = 1; uGdpb < fuNrOfGdpbs; uGdpb ++)

         if( bFirstTriggerMatch )
         {
            Double_t dMeanTriggerGdpbTsOver = 0;
            Double_t dMeanTriggerStarTsOver = 0;
            for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
            {
               if( bTriggerInOverMs[uGdpb] )
               {
                  dMeanTriggerGdpbTsOver += get4v2x::kdClockCycleSizeNs * fvtTsOverLinksBuffer[uGdpb][ 0 ].GetFullGdpbTs() / fuNrOfGdpbs;
                  dMeanTriggerStarTsOver += get4v2x::kdClockCycleSizeNs * fvtTsOverLinksBuffer[uGdpb][ 0 ].GetFullStarTs() / fuNrOfGdpbs;
               } // if( bTriggerInOverMs[uGdpb] )
                  else
                  {
                     dMeanTriggerGdpbTsOver += get4v2x::kdClockCycleSizeNs * fvtTsLinksBuffer[uGdpb][ vuIndexMatchingTrigger[uGdpb] ].GetFullGdpbTs() / fuNrOfGdpbs;
                     dMeanTriggerStarTsOver += get4v2x::kdClockCycleSizeNs * fvtTsLinksBuffer[uGdpb][ vuIndexMatchingTrigger[uGdpb] ].GetFullStarTs() / fuNrOfGdpbs;
                  } // else of if( bTriggerInOverMs[uGdpb] )
            } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
            dMeanTriggerGdpbTsOver /= fuNrOfGdpbs;
            dMeanTriggerStarTsOver /= fuNrOfGdpbs;

            if( bTriggerInOverMs[0] )
            {
               CbmTofStarTrigger meanTriggerOver( static_cast< ULong64_t >( dMeanTriggerGdpbTsOver ), static_cast< ULong64_t >( dMeanTriggerStarTsOver ),
                                                 fvtTsOverLinksBuffer[0][ 0 ].GetStarToken(),
                                                 fvtTsOverLinksBuffer[0][ 0 ].GetStarDaqCmd(),
                                                 fvtTsOverLinksBuffer[0][ 0 ].GetStarTrigCmd() );

               /// Associate this trigger to its subevent
               fStarSubEvent.SetTrigger( meanTriggerOver );
            } // if( bTriggerInOverMs[0] )
               else
               {
                  CbmTofStarTrigger meanTriggerOver( static_cast< ULong64_t >( dMeanTriggerGdpbTsOver ), static_cast< ULong64_t >( dMeanTriggerStarTsOver ),
                                                    fvtTsLinksBuffer[0][ vuIndexMatchingTrigger[0] ].GetStarToken(),
                                                    fvtTsLinksBuffer[0][ vuIndexMatchingTrigger[0] ].GetStarDaqCmd(),
                                                    fvtTsLinksBuffer[0][ vuIndexMatchingTrigger[0] ].GetStarTrigCmd() );

                  /// Associate this trigger to its subevent
                  fStarSubEvent.SetTrigger( meanTriggerOver );
               } // else of if( bTriggerInOverMs[0] )
            /// Set the source index of the subevent
            fStarSubEvent.SetSource( 16 );

            for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
            {
               Double_t dTriggerTime   = 0.0;
               if( bTriggerInOverMs[uGdpb] )
                  dTriggerTime = get4v2x::kdClockCycleSizeNs * fvtTsOverLinksBuffer[uGdpb][ 0 ].GetFullGdpbTs();
                  else dTriggerTime = get4v2x::kdClockCycleSizeNs * fvtTsLinksBuffer[uGdpb][ vuIndexMatchingTrigger[uGdpb] ].GetFullGdpbTs();
               Double_t dTriggerWinBeg = dTriggerTime - fdStarTriggerDelay[ fuGdpbNr ];
               Double_t dTriggerWinEnd = dTriggerTime - fdStarTriggerDelay[ fuGdpbNr ] + fdStarTriggerWinSize[ fuGdpbNr ];
               Double_t dClosestTriggerWinStart = dTriggerTime - fdStarTriggerDelay[ fuGdpbNr ] + fdStarTriggerDeadtime[ fuGdpbNr ];
               Bool_t bFirstMessClosestEventFound = kFALSE;
               Bool_t bEventWindowInOverlap = ( fdCurrentTsCoreEndTime < dTriggerWinEnd ? kTRUE : kFALSE );

               /// Check if this trigger may correspond to an event with data in next TS
               if( kTRUE == bEventWindowInOverlap )
                  fStarSubEvent.SetEndBorderEventFlag();

               /// Check if this trigger leads to an event overlapping with the previous one
               if( 0 < dPrevEventEnd[ fuGdpbNr ] && dTriggerWinBeg < dPrevEventEnd[ fuGdpbNr ] )
                  fStarSubEvent.SetOverlapEventFlag();

               for( std::vector< gdpb::FullMessage >::iterator itMess = itFirstMessageNextEvent[ uGdpb ];
                    itMess != fvmTsLinksBuffer[ uGdpb ].end();
                    ++ itMess )
               {
                  Double_t dMessageFullTime = (*itMess).GetFullTimeNs();
                  if( kFALSE == bFirstMessClosestEventFound &&
                      dClosestTriggerWinStart < dMessageFullTime )
                  {
                     itFirstMessageNextEvent[ uGdpb ] = itMess;
                     bFirstMessClosestEventFound = kTRUE;
                  } // If first possible hit of closest event not found and current hit fits

                  /// Plotting of time to trigger for all hits
                  fhStarHitToTrigAll_gDPB[ fuGdpbNr ]->Fill( dMessageFullTime - dTriggerTime );

                  if( dTriggerWinBeg   <= dMessageFullTime && dMessageFullTime <= dTriggerWinEnd )
                  {
                     /// Message belongs to this event
                     fStarSubEvent.AddMsg( (*itMess) );

                     /// Plotting of time to trigger for hits within the event window
                     fhStarHitToTrigWin_gDPB[ fuGdpbNr ]->Fill( dMessageFullTime - dTriggerTime );
                  } // if( dTriggerWinBeg   <= dMessageFullTime && dMessageFullTime <= dTriggerWinEnd )
                     else if( dTriggerWinEnd < dMessageFullTime )
                        /// First Message out of the window for this event => Stop there and go to the next
                        break;
               } // Loop on message from first allowed by last event + deadtime to end

               /// If event window is going in overlap MS, check also these messages
               if( kTRUE == bEventWindowInOverlap )
               {
                  for( std::vector< gdpb::FullMessage >::iterator itMess = fvmTsOverLinksBuffer[ fuGdpbNr ].begin();
                       itMess != fvmTsOverLinksBuffer[ uGdpb ].end();
                       ++ itMess )
                  {
                     Double_t dMessageFullTime = (*itMess).GetFullTimeNs();
                     if( kFALSE == bFirstMessClosestEventFound &&
                         dClosestTriggerWinStart < dMessageFullTime )
                     {
                        itFirstMessageNextEvent[ uGdpb ] = itMess;
                        bFirstMessClosestEventFound = kTRUE;
                     } // If first possible hit of closest event not found and current hit fits

                     /// Plotting of time to trigger for all hits
                     fhStarHitToTrigAll_gDPB[ fuGdpbNr ]->Fill( dMessageFullTime - dTriggerTime );

                     if( dTriggerWinBeg   <= dMessageFullTime && dMessageFullTime <= dTriggerWinEnd )
                     {
                        /// Message belongs to this event
                        fStarSubEvent.AddMsg( (*itMess) );

                        /// Plotting of time to trigger for hits within the event window
                        fhStarHitToTrigWin_gDPB[ fuGdpbNr ]->Fill( dMessageFullTime - dTriggerTime );
                     } // if( dTriggerWinBeg   <= dMessageFullTime && dMessageFullTime <= dTriggerWinEnd )
                        else if( dTriggerWinEnd < dMessageFullTime )
                           /// First Message out of the window for this event => Stop there and go to the next
                           break;
                  } // Loop on message from beginning to end
               } // if( kTRUE == bEventWindowInOverlap )
            } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)

            /// Time sort the sub-event if required by the user
            if( kTRUE == fbTimeSortOutput )
               fStarSubEvent.SortMessages(  );

            /** TODO: clarify how we deal with multiple sub-events (eg one for each gDPB) **/
            /// Send the sub-event to the STAR systems
            iBuffSzByte = 0;
            pDataBuff = fStarSubEvent.BuildOutput( iBuffSzByte );
            if( NULL != pDataBuff )
            {
               /// Valid output, do stuff with it!
               fulNbBuiltSubEvent++;

#ifdef STAR_SUBEVT_BUILDER
               /*
                ** Function to send sub-event block to the STAR DAQ system
                *       trg_word received is packed as:
                *
                *       trg_cmd|daq_cmd|tkn_hi|tkn_mid|tkn_lo
                */
               star_rhicf_write( fStarSubEvent.GetTrigger().GetStarTrigerWord(),
                                 pDataBuff, iBuffSzByte );
               fulNbStarSubEvent++;
/*
               LOG(INFO) << "Sent STAR event with size " << iBuffSzByte << " Bytes"
                         << " and token " << fStarSubEvent.GetTrigger().GetStarToken()
                         << FairLogger::endl;
*/
#endif // STAR_SUBEVT_BUILDER
               } // if( NULL != pDataBuff )
                  else LOG(ERROR) << "Invalid STAR SubEvent Output, can only happen if trigger "
                                  << " object was not set => Do Nothing more with it!!! "
                                  << FairLogger::endl;

            /// Fill plot of event size
            fhStarEventSize->Fill( iBuffSzByte );
            /// Fill plot of event size as function of trigger time
            if( 0 < fdStartTime )
               fhStarEventSizeTime->Fill( 1e-9 *(dMeanTriggerGdpbTsOver - fdStartTime), iBuffSzByte );
            if( 0 < fdStartTimeLong )
               fhStarEventSizeTimeLong->Fill( 1e-9 *(dMeanTriggerGdpbTsOver - fdStartTimeLong) / 60.0, iBuffSzByte/60.0 );

            /// Now clear the sub-event
            fStarSubEvent.ClearSubEvent();
         } // if( bFirstTriggerMatch )
      } // if( kFALSE == bMatchingTriggerFound )
   } // while( kTRUE == bMatchingTriggerFound )

   /// Clear buffers, eventual border crossing should be taken care thanks to the overlap MS
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
   {
      fvtTsLinksBuffer[uGdpb].clear();
      fvmTsLinksBuffer[uGdpb].clear();
      fvmTsOverLinksBuffer[uGdpb].clear();
      fvtTsOverLinksBuffer[uGdpb].clear();
   } // for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
}

ClassImp(CbmTofStarEventBuilder2018)
