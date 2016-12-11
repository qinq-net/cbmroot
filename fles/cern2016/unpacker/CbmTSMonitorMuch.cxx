// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSMonitorMuch                            -----
// -----               Created 11.11.2016 by V. Singhal and A. Kumar                      -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTSMonitorMuch.h"
#include "CbmMuchUnpackPar.h"

#include "CbmMuchAddress.h"
#include "CbmHistManager.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRunOnline.h"

#include "TClonesArray.h"
#include "TString.h"
#include "TRandom.h"
#include "THttpServer.h"
#include "TROOT.h"

#include <iostream>
#include <stdint.h>
#include <iomanip>

class CbmMuchAddress;
using std::hex;
using std::dec;
using namespace std;

Bool_t bResetMuchHistos = kFALSE;

CbmTSMonitorMuch::CbmTSMonitorMuch()
  : CbmTSUnpack(),
    fNrOfNdpbs(0),
    fNrOfNdpbsA(0),
    fNrOfNdpbsB(0),
    fNrOfFebsPerNdpb(0),
    fMsgCounter(11,0), // length of enum MessageTypes initialized with 0
    fNdpbIdIndexMap(),
    fMuchStationMapX(),
    fMuchStationMapY(),
    fHM(new CbmHistManager()),
    fCurrentEpoch(),
    fNofEpochs(0),
    fCurrentEpochTime(0.),
    fdStartTime( -1 ),
    fdStartTimeMsSz(-1.),
    fcMsSizeAll(NULL),
    fEquipmentId(0),
	fUnpackPar(NULL),
    fFebRateDate_nDPB(),
    fHitDtDate_nDPB(),
    fdLastHitTime_nDPB(),
    fiRunStartDateTimeSec( -1 ),
    fiBinSizeDatePlots( -1 )
{
}

CbmTSMonitorMuch::~CbmTSMonitorMuch()
{
}

Bool_t CbmTSMonitorMuch::Init()
{
  LOG(INFO) << "Initializing flib nxyter unpacker for MUCH" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  return kTRUE;
}

void CbmTSMonitorMuch::SetParContainers()
{
	LOG(INFO) << "Setting parameter containers for " << GetName()
			<< FairLogger::endl;
	fUnpackPar = (CbmMuchUnpackPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmMuchUnpackPar"));
}


Bool_t CbmTSMonitorMuch::InitContainers()
{
	LOG(INFO) << "Init parameter containers for " << GetName()
			<< FairLogger::endl;
         
   Bool_t bReInit = ReInitContainers();
   CreateHistograms();
   
   return bReInit;
}

Bool_t CbmTSMonitorMuch::ReInitContainers()
{
	LOG(INFO) << "ReInit parameter containers for " << GetName()
			<< FairLogger::endl;

	fNrOfNdpbsA = fUnpackPar->GetNrOfnDpbsModA();
	fNrOfNdpbsB = fUnpackPar->GetNrOfnDpbsModB();
  fNrOfNdpbs = fNrOfNdpbsA + fNrOfNdpbsB;
  fNrOfFebsPerNdpb = fUnpackPar->GetNrOfFebsPerNdpb();
  
	LOG(INFO) << "Nr. of nDPBs Mod. A: " << fNrOfNdpbsA
    		<< FairLogger::endl;
  
	LOG(INFO) << "Nr. of nDPBs Mod. B: " << fNrOfNdpbsB
    		<< FairLogger::endl;
      
   fNdpbIdIndexMap.clear();
   for (Int_t i = 0; i< fNrOfNdpbsA; ++i) 
   {
     fNdpbIdIndexMap[fUnpackPar->GetNdpbIdA(i)] = i;
     LOG(INFO) << "nDPB Id of MUCH A " << i
               << " : 0x" << std::hex << fUnpackPar->GetNdpbIdA(i)
               << std::dec
               << FairLogger::endl;
   } // for (Int_t i = 0; i< NrOfnDpbsModA; ++i) 
   for (Int_t i = 0; i< fNrOfNdpbsB; ++i) 
   {
     fNdpbIdIndexMap[fUnpackPar->GetNdpbIdB(i)] = i + fNrOfNdpbsA;
     LOG(INFO) << "nDPB Id of MUCH B " << i
               << " : 0x" << std::hex << fUnpackPar->GetNdpbIdB(i)
               << std::dec
               << FairLogger::endl;
   } // for (Int_t i = 0; i< NrOfnDpbsModB; ++i) 

	Int_t NrOfFebs = fUnpackPar->GetNrOfFebs();

	LOG(INFO) << "Nr. of FEBs : " << NrOfFebs
    		<< FairLogger::endl;


	Int_t NrOfChannels = fUnpackPar->GetNrOfChannels();
	LOG(INFO) << "Nr. of Channels : " << NrOfChannels
    		<< FairLogger::endl;

	// Need to clear the 2 Arrays fMuchStationMapX and MapY

	// log all the DPBs number

	// Filling all the Mapping values in the 2 X and Y arrays
	//	Int_t nrOfChannels = fUnpackPar->GetNumberOfChannels();
	
	for (Int_t febId = 0 ; febId<NrOfFebs; febId++){// looping on all the FEB IDs
		for (Int_t channelId=0; channelId<NrOfChannels; channelId++){

			//Have to check GetPadX and GetPadY values.
//			LOG(INFO) << "Value of GetPadX " << 	fUnpackPar->GetPadX(febId,channelId) << FairLogger::endl;
			fMuchStationMapX[febId][channelId] = fUnpackPar->GetPadX(febId,channelId);
//			LOG(INFO) << "Value of GetPadY " << 	fUnpackPar->GetPadY(febId,channelId) <<FairLogger::endl;		
			fMuchStationMapY[febId][channelId] = fUnpackPar->GetPadY(febId,channelId);
		}
	}

	return kTRUE;
}

void CbmTSMonitorMuch::CreateHistograms()
{
#ifdef USE_HTTP_SERVER
    THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
#endif

	TString sHistName{""};
    	TString title{""};

  sHistName = "hMessageTypeMuch";
  title = "Nb of message for each type; Type";
  TH1I* hMessageType = new TH1I(sHistName, title, 16, 0., 16.);
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
  fHM->Add(sHistName.Data(), hMessageType);
#ifdef USE_HTTP_SERVER
      if (server) server->Register("/MuchRaw", fHM->H1(sHistName.Data()));
#endif

  sHistName = "hSysMessTypeMuch";
  title = "Nb of system message for each type; System Type";
  TH1I* hSysMessType = new TH1I(sHistName, title, 17, 0., 17.);
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
  fHM->Add(sHistName.Data(), hSysMessType);
#ifdef USE_HTTP_SERVER
  if (server) server->Register("/MuchRaw", fHM->H1(sHistName.Data()));
#endif

   // Number of rqte bins =
   //      9 for the sub-unit decade
   //    + 9 for each unit of each decade * 10 for the subdecade range
   //    + 1 for the closing bin top edge
   const Int_t iNbDecadesRate    = 9;
   const Int_t iNbStepsDecade    = 9;
   const Int_t iNbSubStepsInStep = 10;
   const Int_t iNbBinsRate = iNbStepsDecade 
                           + iNbStepsDecade * iNbSubStepsInStep * iNbDecadesRate 
                           + 1;
   Double_t dBinsRate[iNbBinsRate];
      // First fill sub-unit decade
   for( Int_t iSubU = 0; iSubU < iNbStepsDecade; iSubU ++ )
      dBinsRate[ iSubU ] = 0.1 * ( 1 + iSubU );
   std::cout << std::endl;
      // Then fill the main decades
   Double_t dSubstepSize = 1.0 / iNbSubStepsInStep;
   for( Int_t iDecade = 0; iDecade < iNbDecadesRate; iDecade ++)
   {
      Double_t dBase = std::pow( 10, iDecade );
      Int_t iDecadeIdx = iNbStepsDecade 
                       + iDecade * iNbStepsDecade * iNbSubStepsInStep;
      for( Int_t iStep = 0; iStep < iNbStepsDecade; iStep++ )
      {
         Int_t iStepIdx = iDecadeIdx + iStep * iNbSubStepsInStep;
         for( Int_t iSubStep = 0; iSubStep < iNbSubStepsInStep; iSubStep++ )
         {
            dBinsRate[ iStepIdx + iSubStep ] = dBase * (1 + iStep)
                                             + dBase * dSubstepSize * iSubStep;
         } // for( Int_t iSubStep = 0; iSubStep < iNbSubStepsInStep; iSubStep++ )
      } // for( Int_t iStep = 0; iStep < iNbStepsDecade; iStep++ )
   } // for( Int_t iDecade = 0; iDecade < iNbDecadesRate; iDecade ++)
   dBinsRate[ iNbBinsRate - 1 ] = std::pow( 10, iNbDecadesRate );
   
   TString sNdpbTag = "";
	TString sDateHistName{""};
   for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)
   {// looping on all the nDPBS IDs
   
      if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
      {
         sNdpbTag = Form("%04X", fUnpackPar->GetNdpbIdA(dpbId) );
      } // if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
         else 
         {
            sNdpbTag = Form("%04X", fUnpackPar->GetNdpbIdB(dpbId - fNrOfNdpbsA) );
         } // else of if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
      
      for( Int_t febId = 0; febId < fNrOfFebsPerNdpb; febId++)
      {// looping on all the FEB IDs 
         sHistName = Form("Chan_Counts_Much_n%s_f%1u", sNdpbTag.Data(), febId);
         title = Form("Channel counts Much nDPB %s FEB %02u; channel; Counts", 
                     sNdpbTag.Data(), febId);
		  fHM->Add( sHistName.Data(), new TH1F( sHistName.Data(), title.Data(), 128, 0, 128) );
#ifdef USE_HTTP_SERVER
        if (server) server->Register("/MuchRaw", fHM->H1(sHistName.Data()));
#endif
      
        sHistName = Form("Raw_ADC_Much_n%s_f%1u", sNdpbTag.Data(), febId);
        title = Form("Raw ADC Much nDPB %s FEB %02u; channel; ADC value",
               sNdpbTag.Data(), febId);
		  fHM->Add( sHistName.Data(), new TH2F( sHistName.Data(), title.Data(), 128, 0, 128, 4096, 0, 4096) );
#ifdef USE_HTTP_SERVER
        if (server) server->Register("/MuchRaw", fHM->H2(sHistName.Data()));
#endif

        sHistName = Form("FebRate_n%s_f%1u", sNdpbTag.Data(), febId);
        title = Form("Counts per second in nDPB %s FEB %02u; Time[s] ; Counts",
                        sNdpbTag.Data(), febId);
		  fHM->Add( sHistName.Data(), new TH1F( sHistName.Data(), title.Data(), 1800, 0, 1800 ) );
#ifdef USE_HTTP_SERVER
        if (server) server->Register("/MuchRaw", fHM->H1(sHistName.Data()));
#endif

        if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec ) {
          sDateHistName = Form("FebRateDate_n%s_f%1u", sNdpbTag.Data(), febId);
          fHM->Add(sDateHistName.Data(), new TH1F(sDateHistName.Data(), title.Data(),
                        (5400 / fiBinSizeDatePlots), 
                        fiRunStartDateTimeSec -10, fiRunStartDateTimeSec + 5400 - 10));
          ( fHM->H1(sDateHistName.Data()) )->GetXaxis()->SetTimeDisplay(1);
#ifdef USE_HTTP_SERVER
          if (server)
            server->Register("/MuchRaw", fHM->H1(sDateHistName.Data()));
#endif

          sHistName = Form("HitDtDate_n%s_f%1u", sNdpbTag.Data(), febId );
          title = Form("Inverse Hit distance VS time in second in nDPB %s FEB %02u; Time[s] ; F [Hz]; Counts",
                     sNdpbTag.Data() );
          fHM->Add(sHistName.Data(), new TH2F(sHistName.Data(), title.Data(),
                        (5400 / 2*fiBinSizeDatePlots), 
                        fiRunStartDateTimeSec -10, fiRunStartDateTimeSec + 5400 - 10,
                        iNbBinsRate - 1, dBinsRate ));
          ( fHM->H2(sHistName.Data()) )->GetXaxis()->SetTimeDisplay(1);
#ifdef USE_HTTP_SERVER
          if (server)
            server->Register("/MuchRaw", fHM->H2(sHistName.Data()));
#endif
        } // if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec )

        sHistName = Form("HitMissEvo_n%s_f%1u", sNdpbTag.Data(), febId);
        title = Form("Minimal hit loss per second in nDPB %s FEB %02u; Time[s] ; Min Loss",
                        sNdpbTag.Data(), febId);
		  fHM->Add( sHistName.Data(), new TH1F( sHistName.Data(), title.Data(), 1800, 0, 1800 ) );
#ifdef USE_HTTP_SERVER
        if (server) server->Register("/MuchRaw", fHM->H1(sHistName.Data()));
#endif

      } // for( Int_t febId = 0; febId < fNrOfFebsPerNdpb; febId++)
   } // for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)

	sHistName = "Pad_Distribution";
	title = "Pad_Distribution; Sectors in Horizontal Direction; Channels in Vertical Direction";
	fHM->Add( sHistName.Data(), new TH2F(sHistName.Data(), title.Data(), 79, -0.5, 78.5, 23, -0.5, 22.5) );

/*	TH2* histPadDistr = fHM->H2("Pad_Distribution");
	for( Int_t iFeb = 0; iFeb < 15; iFeb ++)
		for( Int_t iChan = 0; iChan <  128; iChan ++)
		{
			Int_t sector  = fUnpackPar->GetPadX(iFeb, iChan);
			Int_t channel = fUnpackPar->GetPadY(iFeb, iChan);
			if(!(sector<0||channel<0)){
	
			histPadDistr->Fill((78-sector),(22-channel), (iFeb + 1));
			if (sector==25 && channel==0) cout <<	"Pad X value" << fUnpackPar->GetPadX(iFeb, iChan) << "Pad Y value" << fUnpackPar->GetPadY(iFeb, iChan) << endl;
		}
	}
//	Int_t iFeb = 3, iChan =32;	

*/	
#ifdef USE_HTTP_SERVER
    if (server) server->Register("/MuchRaw", fHM->H2(sHistName.Data()));
#endif

#ifdef USE_HTTP_SERVER
  if (server)
    server->RegisterCommand("/Reset_All_MUCH", "bResetMuchHistos=kTRUE");
  if (server)
    server->Restrict("/Reset_All_Much", "allow=admin");
#endif

  /** Create summary Canvases for CERN 2016 **/
  Double_t w = 10;
  Double_t h = 10;
  Int_t iNbPadsPerDpb = fNrOfFebsPerNdpb/2 + fNrOfFebsPerNdpb%2;

  TCanvas* cMuchChCounts = new TCanvas("cMuchChCounts", "MUCH Channels counts", w, h);
  cMuchChCounts->Divide( fNrOfNdpbs/2 + fNrOfNdpbs%2, fNrOfFebsPerNdpb );

  TCanvas* cMuchFebRate = new TCanvas("cMuchFebRate", "MUCH FEB rate", w, h);
  cMuchFebRate->Divide( fNrOfNdpbs/2 + fNrOfNdpbs%2, fNrOfFebsPerNdpb );

   TH1* histPnt = NULL;
   for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)
   {// looping on all the nDPBS IDs
      if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
      {
         sNdpbTag = Form("%04X", fUnpackPar->GetNdpbIdA(dpbId) );
      } // if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
         else 
         {
            sNdpbTag = Form("%04X", fUnpackPar->GetNdpbIdB(dpbId - fNrOfNdpbsA) );
         } // else of if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )

      for( Int_t febId = 0; febId < fNrOfFebsPerNdpb; febId++)
      {// looping on all the FEB IDs
         cMuchChCounts->cd( 1 + dpbId * iNbPadsPerDpb + febId/2 );
         gPad->SetLogy();
         sHistName = Form("Chan_Counts_Much_n%s_f%1u", sNdpbTag.Data(), febId);
         histPnt = fHM->H1(sHistName.Data());
         
         if( 0 == febId%2 )
         {
            histPnt->SetLineColor( kRed );  // => Change color for 1st of the 2/pad!
            histPnt->Draw();
         } // if( 0 == febId%2 )
            else
            {
               histPnt->SetLineColor( kBlue );  // => Change color for 1nd of the 2/pad!
               histPnt->Draw("same");
            } // if( 0 == febId%2 )
            
         cMuchFebRate->cd( 1 + dpbId * iNbPadsPerDpb + febId/2 );
         gPad->SetLogy();
         sHistName = Form("FebRate_n%s_f%1u", sNdpbTag.Data(), febId);
         histPnt = fHM->H1(sHistName.Data());
         
         if( 0 == febId%2 )
         {
            histPnt->SetLineColor( kRed );  // => Change color for 1st of the 2/pad!
            histPnt->Draw();
         } // if( 0 == febId%2 )
            else
            {
               histPnt->SetLineColor( kBlue );  // => Change color for 1nd of the 2/pad!
               histPnt->Draw("same");
            } // if( 0 == febId%2 )
      } // for( Int_t febId = 0; febId < fNrOfFebsPerNdpb; febId++)
   } // for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)
   
  /** Recovers/Create Ms Size Canvase for CERN 2016 **/  
  // Try to recover canvas in case it was created already by another monitor
  // If not existing, create it
  fcMsSizeAll = dynamic_cast<TCanvas *>( gROOT->FindObject( "cMsSizeAll" ) );
  if( NULL == fcMsSizeAll )
  {
     fcMsSizeAll = new TCanvas("cMsSizeAll", "Evolution of MS size in last 300 s", w, h);
     fcMsSizeAll->Divide( 4, 4 );
      LOG(INFO) << "Created MS size canvas in MUCH monitor" << FairLogger::endl; 
  } // if( NULL == fcMsSizeAll )
      else LOG(INFO) << "Recovered MS size canvas in MUCH monitor" << FairLogger::endl; 
  
  /*****************************/
  
  for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)
  {
      if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
      {
         sNdpbTag = Form("%04X", fUnpackPar->GetNdpbIdA(dpbId) );
      } // if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
         else 
         {
            sNdpbTag = Form("%04X", fUnpackPar->GetNdpbIdB(dpbId - fNrOfNdpbsA) );
         } // else of if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
      for( Int_t febId = 0; febId < fNrOfFebsPerNdpb; febId++)
      {
         if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec ) {
            sDateHistName = Form("FebRateDate_n%s_f%1u", sNdpbTag.Data(), febId);
            fFebRateDate_nDPB.push_back(fHM->H1(sDateHistName.Data()));
            
            sHistName = Form("HitDtDate_n%s_f%1u", sNdpbTag.Data(), febId );
            fHitDtDate_nDPB.push_back(fHM->H2(sHistName.Data()));
            fdLastHitTime_nDPB.push_back( -1.0 );
         } // if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec )
         
      } // for( Int_t febId = 0; febId < fNrOfFebsPerNdpb; febId++)
  } // for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)
  
  
  fHistMessType = fHM->H1("hMessageTypeMuch");
  fHistSysMessType = fHM->H1("hSysMessTypeMuch");
   
  std::vector<TH1*> Chan_Counts_Much;
  std::vector<TH2*> Raw_ADC_Much;
  std::vector<TH1*> FebRate;
  std::vector<TH1*> HitMissEvo;
/*
  TString sHistName{""};
  TString title{""};
  TString sNdpbTag("");
*/
   for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)
   {// looping on all the nDPBS IDs
      if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
      {
         sNdpbTag = Form("%04X", fUnpackPar->GetNdpbIdA(dpbId) );
      } // if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
         else 
         {
            sNdpbTag = Form("%04X", fUnpackPar->GetNdpbIdB(dpbId - fNrOfNdpbsA) );
         } // else of if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
      for( Int_t febId = 0; febId < fNrOfFebsPerNdpb; febId++)
      {// looping on all the FEB IDs
         sHistName = Form("Chan_Counts_Much_n%s_f%1u", sNdpbTag.Data(), febId);
         fChan_Counts_Much.push_back(fHM->H1(sHistName.Data()));
         sHistName = Form("Raw_ADC_Much_n%s_f%1u", sNdpbTag.Data(), febId);
         fRaw_ADC_Much.push_back(fHM->H2(sHistName.Data()));
         sHistName = Form("FebRate_n%s_f%1u", sNdpbTag.Data(), febId);
         fFebRate.push_back(fHM->H1(sHistName.Data()));
         sHistName = Form("HitMissEvo_n%s_f%1u", sNdpbTag.Data(), febId);
         fHitMissEvo.push_back(fHM->H1(sHistName.Data()));
      } // for( Int_t febId = 0; febId < fNrOfFebsPerNdpb; febId++)
   } // for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)
  fHistPadDistr = fHM->H2("Pad_Distribution");
}

Bool_t CbmTSMonitorMuch::DoUnpack(const fles::Timeslice& ts, size_t component)
{
#ifdef USE_HTTP_SERVER
	  THttpServer* server = FairRunOnline::Instance()->GetHttpServer();
#endif

  if (bResetMuchHistos) {
    ResetAllHistos();
    bResetMuchHistos = kFALSE;
  }

  LOG(DEBUG) << "Timeslice contains " << ts.num_microslices(component)
             << "microslices." << FairLogger::endl;

   TString sMsSzName = Form("MsSz_link_%02u", component);
   TH1* hMsSz = NULL;
   TProfile* hMsSzTime = NULL;
   if( fHM->Exists(sMsSzName.Data() ) )
   {
      hMsSz = fHM->H1(sMsSzName.Data());
      sMsSzName = Form("MsSzTime_link_%02u", component);
      hMsSzTime = fHM->P1(sMsSzName.Data());
   } // if( fHM->Exists(sMsSzName.Data() ) )
      else
      {
         TString sMsSzTitle = Form("Size of MS for nDPB of link %02u; Ms Size [bytes]", component);
         fHM->Add(sMsSzName.Data(), new TH1F( sMsSzName.Data(), sMsSzTitle.Data(), 
                                       160000, 0., 20000. ) );
         hMsSz = fHM->H1(sMsSzName.Data());
#ifdef USE_HTTP_SERVER
         if (server) server->Register("/FlibRaw", hMsSz );
#endif
         sMsSzName = Form("MsSzTime_link_%02u", component);
         sMsSzTitle = Form("Size of MS vs time for gDPB of link %02u; Time[s] ; Ms Size [bytes]", component);
         fHM->Add(sMsSzName.Data(), new TProfile( sMsSzName.Data(), sMsSzTitle.Data(), 
                                       15000, 0., 300. ) );
         hMsSzTime = fHM->P1(sMsSzName.Data());
#ifdef USE_HTTP_SERVER
         if (server) server->Register("/FlibRaw", hMsSzTime );
#endif
         if( NULL != fcMsSizeAll )
         {
            fcMsSizeAll->cd( 1 + component );
            gPad->SetLogy();
            hMsSzTime->Draw("hist le0");
         } // if( NULL != fcMsSizeAll )
         LOG(INFO) << "Added MS size histo for component: " << component 
                << " (nDPB)" << FairLogger::endl; 
      } // else of if( fHM->Exists(sMsSzName.Data() ) )

   Int_t messageType = -111;
  // Loop over microslices
  for (size_t m = 0; m < ts.num_microslices(component); ++m)
    {

      constexpr uint32_t kuBytesPerMessage = 8;

      auto msDescriptor = ts.descriptor(component, m);
      fEquipmentId = msDescriptor.eq_id;
      const uint8_t* msContent = reinterpret_cast<const uint8_t*>(ts.content(component, m));

      uint32_t size = msDescriptor.size;
      LOG(DEBUG) << "Microslice: " << msDescriptor.idx 
                << " has size: " << size << FairLogger::endl; 
      
       if( fdStartTimeMsSz < 0 )
         fdStartTimeMsSz = (1e-9) * static_cast<double>(msDescriptor.idx);
      hMsSz->Fill( size );
      hMsSzTime->Fill( (1e-9) * static_cast<double>( msDescriptor.idx) - fdStartTimeMsSz, size);

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
          messageType = mess.getMessageType();
          fMsgCounter[messageType]++;
          fHistMessType->Fill(messageType);
          
          switch( messageType ) {
          case ngdpb::MSG_HIT: 
            FillHitInfo(mess, fChan_Counts_Much, fRaw_ADC_Much, fFebRate, fHistPadDistr);
            break;
          case ngdpb::MSG_EPOCH:
            FillEpochInfo(mess, fHitMissEvo);
            break;
          case ngdpb::MSG_SYNC:
            // Do nothing, this message is just there to make sure we get all Epochs
            break;
          case ngdpb::MSG_SYS:
            // Just keep track of which type of System message we receive
            fHistSysMessType->Fill(mess.getSysMesType());
            break;
          default: 
            LOG(ERROR) << "Message type " << std::hex << std::setw(2) 
                       << static_cast< uint16_t >( mess.getMessageType() )
                       << " not yet include in nXYTER unpacker."
                       << FairLogger::endl;
          }
          


        } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)
      
    }


  return kTRUE;
}

void CbmTSMonitorMuch::FillHitInfo(ngdpb::Message mess, std::vector<TH1*> Chan_Counts_Much,
                                   std::vector<TH2*> Raw_ADC_Much,
                                   std::vector<TH1*> FebRate,
                                   TH2* histPadDistr)
{
  // --- Get absolute time, NXYTER and channel number
  Int_t rocId      = mess.getRocNumber();
  Int_t nxyterId   = mess.getNxNumber();
  Int_t nxChannel  = mess.getNxChNum(); 
  Int_t charge     = mess.getNxAdcValue();

   // First check if nDPB is mapped
   if( fNdpbIdIndexMap.end() == fNdpbIdIndexMap.find( rocId ) )
   {
		LOG(FATAL) << "Unmapped nDPB Id " << std::hex << rocId << std::dec << FairLogger::endl;
   } // if( fNdpbIdIndexMap.end() == fNdpbIdIndexMap.find( rocId ) )
 
  ULong_t hitTime  = mess.getMsgFullTime(fCurrentEpoch[rocId][nxyterId]);
 
  LOG(DEBUG) << "Hit: " << rocId << ", " << nxyterId 
             << ", " << nxChannel << ", " << charge << FairLogger::endl;

  //here converting channel number into the MUCH Digi.

	Int_t address = CreateAddress(rocId,nxyterId,0, 0, 0, 0, nxChannel, histPadDistr);
	if (address){	
		LOG(DEBUG) << "Create digi with time " << hitTime
               << " at epoch " << fCurrentEpoch[rocId][nxyterId] << FairLogger::endl;
	}
	else {
		LOG(ERROR) << "Unknown Roc Id " << rocId << " or nxyterId "<< nxyterId << " or channelId "
                 << nxChannel << FairLogger::endl;
	}
	Int_t channelNr = fNdpbIdIndexMap[rocId]*fUnpackPar->GetNrOfFebsPerNdpb() + nxyterId;
	Chan_Counts_Much[channelNr]->Fill(nxChannel);
	Raw_ADC_Much[channelNr]->Fill(nxChannel, charge);
	histPadDistr->Fill(nxChannel,charge);

   if( fCurrentEpoch.end() != fCurrentEpoch.find( rocId ) ) {
      if( fCurrentEpoch[rocId].end() != fCurrentEpoch[rocId].find( nxyterId ) ) {
         if( fdStartTime <= 0 )
         {
            fdStartTime = mess.getMsgFullTimeD( fCurrentEpoch[rocId][nxyterId] );
            
           LOG(INFO) << "Start time set to " << (fdStartTime/1e9) 
                     << " s using first hit on channel " << nxChannel 
                     << " of FEB " << nxyterId 
                     << " on nDPB " << fNdpbIdIndexMap[rocId] 
                     << " in epoch " << fCurrentEpoch[rocId][nxyterId] << FairLogger::endl;
         }
            
         if( 0 < fdStartTime )
         {
        	FebRate[channelNr]->Fill( 1e-9*( mess.getMsgFullTimeD( fCurrentEpoch[rocId][nxyterId] )
                                      - fdStartTime)  );

           // General Time (date + time) rate evolution
           // Add offset of -1H as the filenames were using some times offset by 1 hour (Summer time?!?)
           if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec )
           {
             fFebRateDate_nDPB[channelNr]->Fill(
                        1e-9 * ( mess.getMsgFullTimeD( fCurrentEpoch[rocId][nxyterId] )
                                - fdStartTime ) + fiRunStartDateTimeSec  );
             if( 0 < fdLastHitTime_nDPB[ fNdpbIdIndexMap[rocId] ] )
               fHitDtDate_nDPB[ fNdpbIdIndexMap[rocId] ]->Fill(
                           1e-9 * ( mess.getMsgFullTimeD( fCurrentEpoch[rocId][nxyterId] )
                                              - fdStartTime ) + fiRunStartDateTimeSec,
                           1e9/( mess.getMsgFullTimeD( fCurrentEpoch[rocId][nxyterId] )
                             - fdLastHitTime_nDPB[ fNdpbIdIndexMap[rocId] ] )
                                            );
             fdLastHitTime_nDPB[ fNdpbIdIndexMap[rocId] ] = mess.getMsgFullTimeD( fCurrentEpoch[rocId][nxyterId] );
           } // if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec )
        }
     }
   }

}

Int_t CbmTSMonitorMuch::CreateAddress(Int_t rocId, Int_t febId, Int_t stationId,
		Int_t layerId, Int_t sideId, Int_t moduleId, Int_t channelId,
		TH2* histPadDistr)
{
	Int_t febNr = fNdpbIdIndexMap[rocId]*fUnpackPar->GetNrOfFebsPerNdpb() + febId;
	Int_t sector  = fUnpackPar->GetPadX(febNr, channelId);
	Int_t channel = fUnpackPar->GetPadY(febNr, channelId);
   
	Int_t address = CbmMuchAddress::GetAddress(stationId, layerId, sideId, moduleId, sector, channel);
	if(!(sector<0||channel<0)){
		
		histPadDistr->Fill(78-sector,22-channel);
		
	}
//	fHM->H2("Pad_Distribution")->Fill(sector,channel);
	
	return address;
}

void CbmTSMonitorMuch::FillEpochInfo(ngdpb::Message mess, std::vector<TH1*> HitMissEvo)
{
  Int_t rocId          = mess.getRocNumber();
  Int_t nxyterId       = mess.getEpochNxNum();

   // First check if nDPB is mapped
   if( fNdpbIdIndexMap.end() == fNdpbIdIndexMap.find( rocId ) )
   {
		LOG(FATAL) << "Unmapped nDPB Id " << std::hex << rocId << std::dec << FairLogger::endl;
   } // if( fNdpbIdIndexMap.end() == fNdpbIdIndexMap.find( rocId ) )
   
  fCurrentEpoch[rocId][nxyterId] = mess.getEpochNumber();
  
  if( fdStartTime <= 0 )
  {
    Int_t channelNr = fNdpbIdIndexMap[rocId]*fUnpackPar->GetNrOfFebsPerNdpb() + nxyterId;
    HitMissEvo[channelNr]->Fill( mess.getMsgFullTimeD( fCurrentEpoch[rocId][nxyterId] ) 
                                 - fdStartTime);
  }

  //  LOG(INFO) << "Epoch message for ROC " << rocId << " with epoch number "
  //            << fCurrentEpoch[rocId] << FairLogger::endl;
  fCurrentEpochTime = mess.getMsgFullTime(fCurrentEpoch[rocId][nxyterId]);
  fNofEpochs++;
  LOG(DEBUG) << "Epoch message "
             << fNofEpochs << ", epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][nxyterId])
             << ", time " << std::setprecision(9) << std::fixed
             << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
             << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
             << FairLogger::endl;
}

void CbmTSMonitorMuch::Reset()
{
}

void CbmTSMonitorMuch::Finish()
{
  TString message_type;

  for (unsigned int i=0; i< fMsgCounter.size(); ++i) {
    switch(i) {
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
    }
    LOG(INFO) << message_type << " messages: " 
              << fMsgCounter[i] << FairLogger::endl;
  }

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;                
   for( auto it = fCurrentEpoch.begin(); it != fCurrentEpoch.end(); ++it)
      for( auto itN = (it->second).begin(); itN != (it->second).end(); ++itN)
      LOG(INFO) << "Last epoch for nDPB: " 
                << std::hex << std::setw(4) << it->first << std::dec
                << " , FEB  " << std::setw(4) << itN->first 
                << " => " << itN->second 
                << FairLogger::endl;
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;

   gDirectory->mkdir("Much_Raw");
   gDirectory->cd("Much_Raw");

   TString sNdpbTag("");
   for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)
   {// looping on all the nDPBS IDs
      if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
      {
         sNdpbTag = Form("%04X", fUnpackPar->GetNdpbIdA(dpbId) );
      } // if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
         else 
         {
            sNdpbTag = Form("%04X", fUnpackPar->GetNdpbIdB(dpbId - fNrOfNdpbsA) );
         } // else of if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
      for( Int_t febId = 0; febId < fNrOfFebsPerNdpb; febId++)
      {// looping on all the FEB IDs
        fHM->H1( Form("Chan_Counts_Much_n%s_f%1u", sNdpbTag.Data(), febId) )->Write();
        fHM->H2( Form("Raw_ADC_Much_n%s_f%1u", sNdpbTag.Data(), febId) )->Write();  
        fHM->H1( Form("FebRate_n%s_f%1u", sNdpbTag.Data(), febId) )->Write();
        fHM->H1( Form("HitMissEvo_n%s_f%1u", sNdpbTag.Data(), febId) )->Write();
        if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec )
        {
          fHM->H1( Form("FebRateDate_n%s_f%1u", sNdpbTag.Data(), febId) )->Write();
          fHM->H2( Form("HitDtDate_n%s_f%1u", sNdpbTag.Data(), febId) )->Write();
        } // if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec )
      } // for( Int_t febId = 0; febId < fNrOfFebsPerNdpb; febId++)
   } // for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)
   fHM->H2("Pad_Distribution")->Write();        
   gDirectory->cd("..");
   
   gDirectory->mkdir("Flib_Raw");
   gDirectory->cd("Flib_Raw");
   for( UInt_t uLinks = 0; uLinks < 16; uLinks ++)
   {
      TString sMsSzName = Form("MsSz_link_%02u", uLinks);
      if( fHM->Exists(sMsSzName.Data() ) )
         fHM->H1( sMsSzName.Data() )->Write();
         
      sMsSzName = Form("MsSzTime_link_%02u", uLinks);
      if( fHM->Exists(sMsSzName.Data() ) )
         fHM->P1( sMsSzName.Data() )->Write();
   } // for( UInt_t uLinks = 0; uLinks < 16; uLinks ++)
   gDirectory->cd("..");

}


void CbmTSMonitorMuch::FillOutput(CbmDigi* digi)
{
}

void CbmTSMonitorMuch::ResetAllHistos()
{
  LOG(INFO) << "Reseting all MUCH histograms." << FairLogger::endl;
  fHM->H1("hMessageTypeMuch")->Reset();
  fHM->H1("hSysMessTypeMuch")->Reset();

   TString sNdpbTag("");
   for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)
   {// looping on all the nDPBS IDs
      if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
      {
         sNdpbTag = Form("%04X", fUnpackPar->GetNdpbIdA(dpbId) );
      } // if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
         else 
         {
            sNdpbTag = Form("%04X", fUnpackPar->GetNdpbIdB(dpbId - fNrOfNdpbsA) );
         } // else of if( dpbId < fUnpackPar->GetNrOfnDpbsModA() )
      for( Int_t febId = 0; febId < fNrOfFebsPerNdpb; febId++)
      {// looping on all the FEB IDs
        fHM->H1( Form("Chan_Counts_Much_n%s_f%1u", sNdpbTag.Data(), febId) )->Reset();
        fHM->H2( Form("Raw_ADC_Much_n%s_f%1u", sNdpbTag.Data(), febId) )->Reset();  
        fHM->H1( Form("FebRate_n%s_f%1u", sNdpbTag.Data(), febId) )->Reset();
        fHM->H1( Form("HitMissEvo_n%s_f%1u", sNdpbTag.Data(), febId) )->Reset();
        if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec )
        {
          fHM->H1( Form("FebRateDate_n%s_f%1u", sNdpbTag.Data(), febId) )->Reset();
          fHM->H2( Form("HitDtDate_n%%s_f%1u", sNdpbTag.Data(), febId) )->Reset();
        } // if( 0 < fiBinSizeDatePlots && 0 < fiRunStartDateTimeSec )
      } // for( Int_t febId = 0; febId < fNrOfFebsPerNdpb; febId++)
   } // for( Int_t dpbId = 0; dpbId < fNrOfNdpbs; dpbId++)
   fHM->H2("Pad_Distribution")->Write();  

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

void CbmTSMonitorMuch::SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize )
{
   TDatime * fRunStartDateTime     = new TDatime( dateIn, timeIn);
   fiRunStartDateTimeSec = fRunStartDateTime->Convert();
   fiBinSizeDatePlots    = iBinSize;
   
   LOG(INFO) << "Assigned new MUCH Run Start Date-Time: " << fRunStartDateTime->AsString() << FairLogger::endl;
}

ClassImp(CbmTSMonitorMuch)
