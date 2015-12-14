#include "CbmTofOnlineDisplay.h"

#include "FairLogger.h"

#include "TCanvas.h"
#include "TPad.h"
#include "TStyle.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TF1.h"
#include "THStack.h"
#include "TLegend.h"
#include "TProfile.h"
#include "TString.h"
#include "TROOT.h"
#include "TPaveStats.h"

// ---- Default constructor -------------------------------------------
CbmTofOnlineDisplay::CbmTofOnlineDisplay()
  :FairTask("CbmTofOnlineDisplay"),
   fbMonitorTdcOcc(kTRUE),
   fTdcChannelOccupancy(NULL),
   fNumberOfTDC(30),
   fNumberOfSEB(8),
   fUpdateInterval(100),
   fEventCounter(0),
   fbMonitorCts(kFALSE),
   fCtsTriggerMonitor(NULL),
   fbMonitorSebStatus(kFALSE),
   fSebSizeMonitor(NULL),
   fSebStatusMonitor(NULL),
   fbMonitorTdcStatus(kFALSE),
   fTdcSizeMonitor(NULL),
   fTdcStatusMonitor(NULL),
   fbMonitorFSMockup(kFALSE),
   fFSMockupMonitor(NULL),
   fbMonitorRes(kFALSE),
   fOverviewRes(NULL),
   fhResolutionSummary(NULL),
   fhResolutionRmsSummary(NULL),
   fbMonitorDigiStatus(kFALSE),
   fDigiSizeMonitor(NULL),
   fDigiStatusMonitor(NULL),
   fbMonitorRates(kFALSE),
   fCanvRatesMonitor(NULL),
   fStackMbsTrloA(NULL),
   fStackMbsTrloB(NULL),
   fStackFreeTrloA(NULL),
   fStackFreeTrloB(NULL),
   fStackFreeTrloOutA(NULL),
   fStackFreeTrloOutB(NULL),
   fLegStackMbsTrloA(NULL),
   fLegStackMbsTrloB(NULL),
   fLegStackFreeTrloA(NULL),
   fLegStackFreeTrloB(NULL),
   fLegStackFreeTrloOutA(NULL),
   fLegStackFreeTrloOutB(NULL),
   fbRatesSlidingScale(kFALSE)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Default Constructor of CbmTofOnlineDisplay");
  for( Int_t iCh = 0; iCh < 16; iCh++)
   fsFreeTrloNames[iCh] = Form("Ch %02d", iCh);
}

// ---- Destructor ----------------------------------------------------
CbmTofOnlineDisplay::~CbmTofOnlineDisplay()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Destructor of CbmTofOnlineDisplay");
}

// ----  Initialisation  ----------------------------------------------
void CbmTofOnlineDisplay::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of CbmTofOnlineDisplay");
}

void CbmTofOnlineDisplay::SetFreeTrloNames( 
         TString sChA, TString sChB, TString sChC, TString sChD, 
         TString sChE, TString sChF, TString sChG, TString sChH, 
         TString sChI, TString sChJ, TString sChK, TString sChL, 
         TString sChM, TString sChN, TString sChO, TString sChP )
{
   fsFreeTrloNames[ 0] = sChA;
   fsFreeTrloNames[ 1] = sChB;
   fsFreeTrloNames[ 2] = sChC;
   fsFreeTrloNames[ 3] = sChD;
   fsFreeTrloNames[ 4] = sChE;
   fsFreeTrloNames[ 5] = sChF;
   fsFreeTrloNames[ 6] = sChG;
   fsFreeTrloNames[ 7] = sChH;
   fsFreeTrloNames[ 8] = sChI;
   fsFreeTrloNames[ 9] = sChJ;
   fsFreeTrloNames[10] = sChK;
   fsFreeTrloNames[11] = sChL;
   fsFreeTrloNames[12] = sChM;
   fsFreeTrloNames[13] = sChN;
   fsFreeTrloNames[14] = sChO;
   fsFreeTrloNames[15] = sChP;
}

// ---- Init ----------------------------------------------------------
InitStatus CbmTofOnlineDisplay::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of CbmTofOnlineDisplay");

  TH1 *h1;
  TH2 *h2;
  Float_t lsize=0.07;

  if( fbMonitorTdcOcc )
  {
    fTdcChannelOccupancy = new TCanvas("tCanvasOccupancy","TRB TDC channel occupancy",8,6,900,600);
    fTdcChannelOccupancy->Divide(8,5);

    for(Int_t iCh=0; iCh<fNumberOfTDC; iCh++){
      fTdcChannelOccupancy->cd(iCh+1);
      gROOT->cd();
      TString hname=Form("tof_trb_ch_occ_%03d",iCh);
      h1=(TH1 *)gROOT->FindObjectAny(hname);
      if (h1!=NULL) {
        h1->Draw("");
        gPad->SetFillColor(0);
        gStyle->SetPalette(1);
        gStyle->SetLabelSize(lsize);
        gPad->SetLogy();
      } else {
        LOG(INFO)<<"Histogram "<<hname<<" not existing. "<<FairLogger::endl;
      }
    }
  }
  
  if( fbMonitorCts )
  {
    fCtsTriggerMonitor = new TCanvas("tCanvasTrigger","TRB CTS trigger monitor",1080,0,500,200);
    fCtsTriggerMonitor->Divide(2,1);

    fCtsTriggerMonitor->cd(1);
    gROOT->cd();
    gPad->SetFillColor(0);
    gStyle->SetPalette(1);
    gStyle->SetLabelSize(lsize);
    gPad->SetLogy();
    h1 = ((TH1 *)gROOT->FindObjectAny("tof_trb_trigger_pattern"));
    if( h1 )
    {
      h1->Draw("");
    }

    fCtsTriggerMonitor->cd(2);
    gROOT->cd();
    gPad->SetFillColor(0);
    gStyle->SetPalette(1);
    gStyle->SetLabelSize(lsize);
    gPad->SetLogy();
    h1 = ((TH1 *)gROOT->FindObjectAny("tof_trb_trigger_types"));
    if( h1 )
    {
      h1->Draw("");
    }
  }

  if( fbMonitorSebStatus )
  {
	fSebSizeMonitor = new TCanvas("tCanvasSebSize","TRB subevent sizes",1080,36,500,300);
	fSebSizeMonitor->Divide(6,1);

	for(Int_t iCh=0; iCh<fNumberOfSEB; iCh++){
      fSebSizeMonitor->cd(iCh+1);
	  gROOT->cd();
	  TString hname=Form("tof_trb_size_subevent_%03d",iCh);
	  h1=(TH1 *)gROOT->FindObjectAny(hname);
	  if (h1!=NULL) {
	    h1->Draw("");
	    gPad->SetFillColor(0);
	    gStyle->SetPalette(1);
	    gStyle->SetLabelSize(lsize);
	    gPad->SetLogy();
	  } else {
	    LOG(INFO)<<"Histogram "<<hname<<" not existing. "<<FairLogger::endl;
	  }
    }

	fSebStatusMonitor = new TCanvas("tCanvasSebStatus","TrbNet subevent status bits",1080,56,500,300);
	fSebStatusMonitor->Divide(6,1);

	for(Int_t iCh=0; iCh<fNumberOfSEB; iCh++){
      fSebStatusMonitor->cd(iCh+1);
	  gROOT->cd();
	  TString hname=Form("tof_trb_status_subevent_%03d",iCh);
	  h1=(TH1 *)gROOT->FindObjectAny(hname);
	  if (h1!=NULL) {
	    h1->Draw("");
	    gPad->SetFillColor(0);
	    gStyle->SetPalette(1);
	    gStyle->SetLabelSize(lsize);
	    gPad->SetLogy();
	  } else {
	    LOG(INFO)<<"Histogram "<<hname<<" not existing. "<<FairLogger::endl;
	  }
    }
  }

  if( fbMonitorTdcStatus )
  {
	fTdcSizeMonitor = new TCanvas("tCanvasTdcSize","TDC data payload",980,76,500,300);
	fTdcSizeMonitor->Divide(6,6);

	for(Int_t iCh=0; iCh<fNumberOfTDC; iCh++){
      fTdcSizeMonitor->cd(iCh+1);
	  gROOT->cd();
	  TString hname=Form("tof_trb_words_tdc_%03d",iCh);
	  h1=(TH1 *)gROOT->FindObjectAny(hname);
	  if (h1!=NULL) {
	    h1->Draw("");
	    gPad->SetFillColor(0);
	    gStyle->SetPalette(1);
	    gStyle->SetLabelSize(lsize);
	    gPad->SetLogy();
	  } else {
	    LOG(INFO)<<"Histogram "<<hname<<" not existing. "<<FairLogger::endl;
	  }
    }

	fTdcStatusMonitor = new TCanvas("tCanvasTdcStatus","TDC processing status",48,56,1000,700);
	fTdcStatusMonitor->Divide(6,6);

	for(Int_t iCh=0; iCh<fNumberOfTDC; iCh++){
      fTdcStatusMonitor->cd(iCh+1);
	  gROOT->cd();
	  TString hname=Form("tof_trb_process_status_tdc_%03d",iCh);
	  h1=(TH1 *)gROOT->FindObjectAny(hname);
	  if (h1!=NULL) {
	    h1->Draw("");
	    gPad->SetFillColor(0);
	    gStyle->SetPalette(1);
	    gStyle->SetLabelSize(lsize);
	    gPad->SetLogy();
	  } else {
	    LOG(INFO)<<"Histogram "<<hname<<" not existing. "<<FairLogger::endl;
	  }
    }
  }

  if( fbMonitorFSMockup )
  {
	fFSMockupMonitor = new TCanvas("tCanvasMockup","TRB free-running mock-up monitor",1080,0,500,200);
	fFSMockupMonitor->Divide(2,1);

	fFSMockupMonitor->cd(1);
    gROOT->cd();
    gPad->SetFillColor(0);
    gStyle->SetPalette(1);
    gStyle->SetLabelSize(lsize);
    gPad->SetLogy();
    h1 = ((TH1 *)gROOT->FindObjectAny("tof_trb_event_jump"));
    if( h1 )
    {
      h1->Draw("");
      gPad->Update();
      TPaveStats* tStats = (TPaveStats*)h1->FindObject("stats");
      tStats->SetOptStat(111111);
      gPad->Update();
    }

    fFSMockupMonitor->cd(2);
    gROOT->cd();
    gPad->SetFillColor(0);
    gStyle->SetPalette(1);
    gStyle->SetLabelSize(lsize);
    gPad->SetLogy();
    h1 = ((TH1 *)gROOT->FindObjectAny("tof_trb_buffer_overflow"));
    if( h1 )
    {
      h1->Draw("");
    }
  }

//  fbMonitorDigiStatus=kTRUE;
  if( fbMonitorDigiStatus )
  {
	fDigiStatusMonitor = new TCanvas("tCanvasDigiSize","Digi status",1000,500,700,700);
	fDigiStatusMonitor->Divide(4,5);
	Int_t iType[9]={1,3,4,6,7,9,2,5,8};
	Int_t iNumSm[9]={1,1,1,1,1,1,4,3,1};
	Int_t iNumRpc[9]={1,1,1,1,1,1,1,1,6};
	Int_t iCanv=0;
	Int_t iNDet=9;
	for(Int_t iCh=0; iCh<iNDet; iCh++){
	  for(Int_t iSm=0; iSm<iNumSm[iCh];iSm++){
	    for(Int_t iRpc=0; iRpc<iNumRpc[iCh];iRpc++){
	      fDigiStatusMonitor->cd(iCanv+1); iCanv++;
	      gROOT->cd();
	      TString hname=Form("cl_SmT%01d_sm%03d_rpc%03d_DigiCor",iType[iCh],iSm,iRpc);
	      h2=(TH2 *)gROOT->FindObjectAny(hname);
	      if (h2!=NULL) {
            h2->Draw("colz");
            //gPad->SetLogz();
	      } else {
            LOG(INFO)<<"Histogram "<<hname<<" not existing. "<<FairLogger::endl;
            fbMonitorDigiStatus=kFALSE;
	      }
	    }
	  }
	}
  }

  if( kTRUE == fbMonitorRes )
  {
     fOverviewRes        = new TCanvas("OverviewRes","Overview TDC Resolution (RMS and Fit)",48,56,1000,700);
     fOverviewRes->Divide(2);
     fhResolutionSummary = new TH2D("fhResolutionSummary", "TDC Resolution summary (Fit); TDC #1 []; TDC #2 [] ",
                                      fNumberOfTDC - 1, 0, fNumberOfTDC - 1,
                                      fNumberOfTDC - 1, 1, fNumberOfTDC );
     fhResolutionSummary->SetStats(kFALSE);

     fhResolutionRmsSummary = new TH2D("fhResolutionRmsSummary", "TDC Resolution summary (RMS); TDC #1 []; TDC #2 [] ",
                                      fNumberOfTDC - 1, 0, fNumberOfTDC - 1,
                                      fNumberOfTDC - 1, 1, fNumberOfTDC );
     fhResolutionRmsSummary->SetStats(kFALSE);
  } // if( kTRUE == fbMonitorRes )

  if( kTRUE == fbMonitorRates )
  {
     fCanvRatesMonitor        = new TCanvas("OverviewRates","Overview TrigLog scaler rates (MBS and free)",48,56,1000,700);
     fCanvRatesMonitor->Divide(3, 2);
     fStackMbsTrloA     = new THStack("fStackMbsTrloA", 
                                           "Evolution of MBS scalers 1- 8 rates; Time since start [s]; Rate [Hz] ");
     fStackMbsTrloB     = new THStack("fStackMbsTrloB", 
                                           "Evolution of MBS scalers 9-16 rates; Time since start [s]; Rate [Hz] ");
     fStackFreeTrloA    = new THStack("fStackFreeTrloA", 
                                           "Evolution of Free scalers 1- 8 rates; Time since start [s]; Rate [Hz] ");
     fStackFreeTrloB    = new THStack("fStackFreeTrloB", 
                                           "Evolution of Free scalers 9-16 rates; Time since start [s]; Rate [Hz] ");
     fStackFreeTrloOutA = new THStack("fStackFreeTrloOutA", 
                                           "Evolution of Free output scalers 1- 8 counts; Time since start [s]; Counts [] ");
     fStackFreeTrloOutB = new THStack("fStackFreeTrloOutB", 
                                           "Evolution of Free output scalers 9-16 counts; Time since start [s]; Counts [] ");
     fLegStackMbsTrloA     = new TLegend(0.80,0.5,0.99,0.9);
     fLegStackMbsTrloB     = new TLegend(0.80,0.5,0.99,0.9);
     fLegStackFreeTrloA    = new TLegend(0.80,0.5,0.99,0.9);
     fLegStackFreeTrloB    = new TLegend(0.80,0.5,0.99,0.9);
     fLegStackFreeTrloOutA = new TLegend(0.80,0.5,0.99,0.9);
     fLegStackFreeTrloOutB = new TLegend(0.80,0.5,0.99,0.9);

     TProfile * pTemp = NULL;
     Int_t      iColorsArr[8] = { kRed,       kBlue,  kGreen+3,  kOrange+1, 
                                  kMagenta+1, kBlack, kViolet-6, kCyan+2};
     gROOT->cd();
     for( Int_t iCh = 0; iCh < 16; iCh++)
     {
	     TString hname;
        if( iCh < 8 )
        {
           hname=Form("tof_cal_triglog_00_scal_evo_00_%02d",iCh);
           pTemp=(TProfile *)gROOT->FindObjectAny(hname);
           if( NULL != pTemp) 
           {
               pTemp->SetLineColor( iColorsArr[iCh] );
               fStackMbsTrloA->Add(pTemp);
               fLegStackMbsTrloA->AddEntry(pTemp,Form("Ch %02d", iCh),"l");
           } // if( NULL != pTemp) 

           hname=Form("tof_cal_trigscal_07_scal_evo_00_%02d",iCh);
           pTemp=(TProfile *)gROOT->FindObjectAny(hname);
           if( NULL != pTemp) 
           {
               pTemp->SetLineColor( iColorsArr[iCh] );
               fStackFreeTrloA->Add(pTemp);
//               fLegStackFreeTrloA->AddEntry(pTemp,Form("Ch %02d", iCh),"l");
               fLegStackFreeTrloA->AddEntry(pTemp,fsFreeTrloNames[iCh],"l");
           } // if( NULL != pTemp)

           hname=Form("tof_trigscal_00_scalers_evo_01_%02d",iCh);
           h1=(TH1 *)gROOT->FindObjectAny(hname);
           if( NULL != h1) 
           {
               h1->SetLineColor( iColorsArr[iCh] );
               fStackFreeTrloOutA->Add(h1);
               fLegStackFreeTrloOutA->AddEntry(h1,Form("LmuO %01d", iCh),"l");
//               fLegStackFreeTrloOutA->AddEntry(pTemp,fsFreeTrloNames[iCh],"l");
           } // if( NULL != pTemp)
        } // if( iCh < 8 )
        else
        {
           hname=Form("tof_cal_triglog_00_scal_evo_00_%02d",iCh);
           pTemp=(TProfile *)gROOT->FindObjectAny(hname);
           if( NULL != pTemp) 
           {
               pTemp->SetLineColor( iColorsArr[iCh - 8] );
               fStackMbsTrloB->Add(pTemp);
               fLegStackMbsTrloB->AddEntry(pTemp,Form("Ch %02d", iCh),"l");
           } // if( NULL != pTemp) 

           hname=Form("tof_cal_trigscal_07_scal_evo_00_%02d",iCh);
           pTemp=(TProfile *)gROOT->FindObjectAny(hname);
           if( NULL != pTemp) 
           {
               pTemp->SetLineColor( iColorsArr[iCh - 8] );
               fStackFreeTrloB->Add(pTemp);
//               fLegStackFreeTrloB->AddEntry(pTemp,Form("Ch %02d", iCh),"l");
               fLegStackFreeTrloB->AddEntry(pTemp,fsFreeTrloNames[iCh],"l");
           } // if( NULL != pTemp)

           hname=Form("tof_trigscal_00_scalers_evo_01_%02d",iCh);
           h1=(TH1 *)gROOT->FindObjectAny(hname);
           if( NULL != h1) 
           {
               h1->SetLineColor( iColorsArr[iCh - 8] );
               fStackFreeTrloOutB->Add(h1);
               fLegStackFreeTrloOutB->AddEntry(h1,Form("LmuO %02d", iCh),"l");
//               fLegStackFreeTrloOutB->AddEntry(pTemp,fsFreeTrloNames[iCh],"l");
           } // if( NULL != pTemp)

        } // else of if( iCh < 8 )
     } // for( Int_t iCh = 0; iCh < 16; iCh++)

     fCanvRatesMonitor->cd(1);
     fStackMbsTrloA->Draw("nostack");
     fLegStackMbsTrloA->Draw();

     fCanvRatesMonitor->cd(2);
     fStackFreeTrloA->Draw("nostack");
     fLegStackFreeTrloA->Draw();

     fCanvRatesMonitor->cd(3);
     fStackFreeTrloOutA->Draw("nostack");
     fLegStackFreeTrloOutB->Draw();

     fCanvRatesMonitor->cd(4);
     fStackMbsTrloB->Draw("nostack");
     fLegStackMbsTrloB->Draw();

     fCanvRatesMonitor->cd(5);
     fStackFreeTrloB->Draw("nostack");
     fLegStackFreeTrloB->Draw();

     fCanvRatesMonitor->cd(6);
     fStackFreeTrloOutB->Draw("nostack");
     fLegStackFreeTrloOutB->Draw();
  } // if( kTRUE == fbMonitorRes )


  return kSUCCESS;

}

// ---- ReInit  -------------------------------------------------------
InitStatus CbmTofOnlineDisplay::ReInit()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of CbmTofOnlineDisplay");
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void CbmTofOnlineDisplay::Exec(Option_t* /*option*/)
{

  fEventCounter++;

  if ( 0 == fEventCounter%fUpdateInterval ) {
     LOG(DEBUG)<<"Update Canvas for Event "<< fEventCounter << FairLogger::endl;

     if( fbMonitorTdcOcc )
     {
        for(Int_t iCh=0; iCh<fNumberOfTDC; iCh++){
          fTdcChannelOccupancy->cd(iCh+1);
          gPad->Modified();
          gPad->Update();
        }
        fTdcChannelOccupancy->Modified();
        fTdcChannelOccupancy->Update();
     }

     if( fbMonitorCts )
     {
       fCtsTriggerMonitor->cd(1);
       gPad->Modified();
       gPad->Update();

       fCtsTriggerMonitor->cd(2);
       gPad->Modified();
       gPad->Update();

       fCtsTriggerMonitor->Modified();
       fCtsTriggerMonitor->Update();
     }

     if( fbMonitorSebStatus )
     {
       for(Int_t iCh=0; iCh<fNumberOfSEB; iCh++){
    	 fSebSizeMonitor->cd(iCh+1);
         gPad->Modified();
         gPad->Update();
       }

       fSebSizeMonitor->Modified();
       fSebSizeMonitor->Update();

       for(Int_t iCh=0; iCh<fNumberOfSEB; iCh++){
    	 fSebStatusMonitor->cd(iCh+1);
         gPad->Modified();
         gPad->Update();
       }

       fSebStatusMonitor->Modified();
       fSebStatusMonitor->Update();
     }

     if( fbMonitorTdcStatus )
     {
       for(Int_t iCh=0; iCh<fNumberOfTDC; iCh++){
    	 fTdcSizeMonitor->cd(iCh+1);
         gPad->Modified();
         gPad->Update();
       }

       fTdcSizeMonitor->Modified();
       fTdcSizeMonitor->Update();

       for(Int_t iCh=0; iCh<fNumberOfTDC; iCh++){
    	 fTdcStatusMonitor->cd(iCh+1);
         gPad->Modified();
         gPad->Update();
       }

       fTdcStatusMonitor->Modified();
       fTdcStatusMonitor->Update();
     }

     if( fbMonitorFSMockup )
     {
       fFSMockupMonitor->cd(1);
       gPad->Modified();
       gPad->Update();

       fFSMockupMonitor->cd(2);
       gPad->Modified();
       gPad->Update();

       fFSMockupMonitor->Modified();
       fFSMockupMonitor->Update();
     }

     if( kTRUE == fbMonitorRes )
     {
        fhResolutionSummary->Reset();
        fhResolutionRmsSummary->Reset();

        TH2 *h2;
//        TProfile *pTemp;
        TH1 *hTemp;
        TF1 *fitFunc;
        for(Int_t iTdc1 = 0; iTdc1 < fNumberOfTDC - 1; iTdc1++)
        {
           // First get the reference comp histogram
           TString hname=Form("tof_trb_RefChComp_b%03d", iTdc1);
           h2=(TH2 *)gROOT->FindObjectAny(hname);
           if (h2!=NULL)
           {
              if( 0 < h2->GetEntries() )
              {
                 for(Int_t iTdc2 = iTdc1 + 1; iTdc2 < fNumberOfTDC; iTdc2++)
                 {
                     hTemp = h2->ProjectionY("hTemp", iTdc2 - iTdc1, iTdc2 - iTdc1 ); // s to get RMS instead of standard error

                     if( 0 < hTemp->GetEntries() && 0 < hTemp->Integral() )
                     {
                        // Fill the RMS plot
                        fhResolutionRmsSummary->Fill(iTdc1, iTdc2, hTemp->GetRMS() );

                        // Fit and fill the Sigma plot
                        Int_t iBinPeak = hTemp->GetMaximumBin();
                        fitFunc = new TF1( Form("fitFunc_%03d_%03d",iTdc1, iTdc2), "gaus",
                                       hTemp->GetBinCenter(iBinPeak) - 1000 ,
                                       hTemp->GetBinCenter(iBinPeak) + 1000);
                        hTemp->Fit( Form("fitFunc_%03d_%03d",iTdc1, iTdc2), "QRM0");
                        fhResolutionSummary->Fill(iTdc1, iTdc2, fitFunc->GetParameter(2) );

                        delete hTemp;
                        if( NULL != gROOT->FindObjectAny( Form("fitFunc_%03d_%03d",iTdc1, iTdc2) ) )
                           delete gROOT->FindObjectAny( Form("fitFunc_%03d_%03d",iTdc1, iTdc2) ) ;
                     } // if( 0 < hTemp->GetEntries() )
                 } // for(Int_t iTdc2 = iTdc1 + 1; iTdc2 < fNumberOfTDC; iTdc2++)
               } // if( 0 < h2->GetEntries() )
           } // if (h2!=NULL)
               else LOG(INFO)<<"Histogram "<<hname<<" not existing. "<<FairLogger::endl;
        } // for(Int_t iCh=0; iCh<fNumberOfTDC; iCh++)
        fOverviewRes->cd(1);
        fhResolutionRmsSummary->Draw("colz");
        ( fhResolutionRmsSummary->GetZaxis() )->SetRangeUser(10.0, 50.0);

        fOverviewRes->cd(2);
        fhResolutionSummary->Draw("colz");
        ( fhResolutionSummary->GetZaxis() )->SetRangeUser(10.0, 50.0);

        fOverviewRes->Modified();
        fOverviewRes->Update();
     } // if( kTRUE == fbMonitorRes )

     if( fbMonitorDigiStatus )
     {
       for(Int_t iCh=0; iCh<20; iCh++){
    	 fDigiStatusMonitor->cd(iCh+1);
         gPad->Modified();
         gPad->Update();
       }
     }
     if( kTRUE == fbMonitorRates )
     {
        if( kTRUE == fbRatesSlidingScale )
        {
           TProfile *pTemp = (TProfile *)gROOT->FindObjectAny("tof_cal_triglog_00_scal_evo_00_15");
           Double_t dTimeEnd = pTemp->GetBinCenter( pTemp->FindLastBinAbove( 0.0 ) );
           if( 100.0 < dTimeEnd )
           {
              (fStackMbsTrloA->GetXaxis() )->SetRangeUser( dTimeEnd - 100.0, dTimeEnd + 10 );
              (fStackMbsTrloB->GetXaxis() )->SetRangeUser( dTimeEnd - 100.0, dTimeEnd + 10 );
              (fStackFreeTrloA->GetXaxis() )->SetRangeUser( dTimeEnd - 100.0, dTimeEnd + 10 );
              (fStackFreeTrloB->GetXaxis() )->SetRangeUser( dTimeEnd - 100.0, dTimeEnd + 10 );
              (fStackFreeTrloOutA->GetXaxis() )->SetRangeUser( dTimeEnd - 100.0, dTimeEnd + 10 );
              (fStackFreeTrloOutB->GetXaxis() )->SetRangeUser( dTimeEnd - 100.0, dTimeEnd + 10 );
           } // if( 100.0 < dTimeEnd )
              else
              {
                 (fStackMbsTrloA->GetXaxis() )->SetRangeUser( 0.0, dTimeEnd + 10 );
                 (fStackMbsTrloB->GetXaxis() )->SetRangeUser( 0.0, dTimeEnd + 10 );
                 (fStackFreeTrloA->GetXaxis() )->SetRangeUser( 0.0, dTimeEnd + 10 );
                 (fStackFreeTrloB->GetXaxis() )->SetRangeUser( 0.0, dTimeEnd + 10 );
                 (fStackFreeTrloOutA->GetXaxis() )->SetRangeUser( 0.0, dTimeEnd + 10 );
                 (fStackFreeTrloOutB->GetXaxis() )->SetRangeUser( 0.0, dTimeEnd + 10 );
              } // else of if( 100.0 < dTimeEnd )
        } // if( kTRUE == fbRatesSlidingScale )

        fCanvRatesMonitor->cd(1);
        gPad->SetLogy();
        gPad->SetGridx();
        gPad->SetGridy();
        gPad->Modified();
        gPad->Update();

        fCanvRatesMonitor->cd(2);
        gPad->SetLogy();
        gPad->SetGridx();
        gPad->SetGridy();
        gPad->Modified();
        gPad->Update();

        fCanvRatesMonitor->cd(3);
        gPad->SetLogy();
        gPad->SetGridx();
        gPad->SetGridy();
        gPad->Modified();
        gPad->Update();

        fCanvRatesMonitor->cd(4);
        gPad->SetLogy();
        gPad->SetGridx();
        gPad->SetGridy();
        gPad->Modified();
        gPad->Update();

        fCanvRatesMonitor->cd(5);
        gPad->SetLogy();
        gPad->SetGridx();
        gPad->SetGridy();
        gPad->Modified();
        gPad->Update();

        fCanvRatesMonitor->cd(6);
//        gPad->SetLogy();
        gPad->SetGridx();
        gPad->SetGridy();
        gPad->Modified();
        gPad->Update();

     } // if( kTRUE == fbMonitorRes )
  } 
}

// ---- Finish --------------------------------------------------------
void CbmTofOnlineDisplay::Finish()
{
  LOG(INFO)<<"Finish of CbmTofOnlineDisplay"<<FairLogger::endl;

  if( fbMonitorTdcOcc )
   fTdcChannelOccupancy->Update();

  if( fbMonitorCts )
  {
    fCtsTriggerMonitor->Update();
  }
  if( fbMonitorSebStatus )
  {
    fSebSizeMonitor->Update();
    fSebStatusMonitor->Update();
  }
  if( fbMonitorTdcStatus )
  {
    fTdcSizeMonitor->Update();
    fTdcStatusMonitor->Update();
  }
  if( fbMonitorFSMockup )
  {
    fFSMockupMonitor->Update();
  }
  if( kTRUE == fbMonitorRes )
     fOverviewRes->Update();

  if( fbMonitorDigiStatus )
  {
    fDigiStatusMonitor->Update();
  }
}

ClassImp(CbmTofOnlineDisplay)
