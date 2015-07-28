#include "CbmTofOnlineDisplay.h"

#include "FairLogger.h"

#include "TCanvas.h"
#include "TPad.h"
#include "TStyle.h"
#include "TH1.h"
#include "TH2.h"
#include "TString.h"
#include "TROOT.h"
#include "TPaveStats.h"

// ---- Default constructor -------------------------------------------
CbmTofOnlineDisplay::CbmTofOnlineDisplay()
  :FairTask("CbmTofOnlineDisplay"),
   fTdcChannelOccupancy(NULL),
   fNumberOfTDC(24),
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
   fbMonitorDigiStatus(kFALSE),
   fDigiSizeMonitor(NULL),
   fDigiStatusMonitor(NULL)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Default Constructor of CbmTofOnlineDisplay");
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

// ---- Init ----------------------------------------------------------
InitStatus CbmTofOnlineDisplay::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of CbmTofOnlineDisplay");

  fTdcChannelOccupancy = new TCanvas("tCanvasOccupancy","TRB TDC channel occupancy",8,6,900,600);
  fTdcChannelOccupancy->Divide(5,4);
  Float_t lsize=0.07;

  TH1 *h1;
  TH2 *h2;
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

  fbMonitorDigiStatus=kTRUE;
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
     fOverviewRes        = new TCanvas("OverviewRes","Overview TDc Resolution",48,56,1000,700);
     fhResolutionSummary = new TH2D("fhResolutionSummary", "TDC Resolution summary; TDC #1 []; TDC #2 [] ",
                                      fNumberOfTDC - 1, 0, fNumberOfTDC - 1,
                                      fNumberOfTDC - 1, 1, fNumberOfTDC );
     fhResolutionSummary->SetStats(kFALSE);
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
void CbmTofOnlineDisplay::Exec(Option_t* option)
{

  fEventCounter++;

  if ( 0 == fEventCounter%fUpdateInterval ) {
     LOG(DEBUG)<<"Update Canvas for Event "<< fEventCounter << FairLogger::endl;
     for(Int_t iCh=0; iCh<fNumberOfTDC; iCh++){
       fTdcChannelOccupancy->cd(iCh+1);
       gPad->Modified();
       gPad->Update();
     }
     fTdcChannelOccupancy->Modified();
     fTdcChannelOccupancy->Update();

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

        TH2 *h2;
        for(Int_t iTdc1 = 0; iTdc1 < fNumberOfTDC - 1; iTdc1++)
        {
           // First get the reference comp histogram
           TString hname=Form("tof_trb_RefChComp_b%03d", iTdc1);
           h2=(TH2 *)gROOT->FindObjectAny(hname);
           if (h2!=NULL)
           {
              // Fit a gaussian for each pair
              TObjArray aSlices;
              h2->FitSlicesY( 0, 0, -1, 0, "QNR", &aSlices);

              // Read the obtained sigma and fill it in the 2D summary histogram
              for(Int_t iTdc2 = iTdc1 + 1; iTdc2 < fNumberOfTDC; iTdc2++)
                 fhResolutionSummary->Fill(iTdc1, iTdc2, ((TH2*)aSlices[2])->GetBinContent( iTdc2 - iTdc1 ) );
            } // if (h2!=NULL)
            else LOG(INFO)<<"Histogram "<<hname<<" not existing. "<<FairLogger::endl;
        } // for(Int_t iCh=0; iCh<fNumberOfTDC; iCh++)
        fOverviewRes->cd();
        fhResolutionSummary->Draw("colz");
        ( fhResolutionSummary->GetZaxis() )->SetRangeUser(0.0, 100.0);
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
  } 
}

// ---- Finish --------------------------------------------------------
void CbmTofOnlineDisplay::Finish()
{
  LOG(INFO)<<"Finish of CbmTofOnlineDisplay"<<FairLogger::endl;
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
