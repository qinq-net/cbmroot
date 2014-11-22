#include "CbmTrdOnlineDisplay.h"

#include "CbmHistManager.h"
#include "CbmTrdRawBeamProfile.h"

#include "FairLogger.h"

#include "TCanvas.h"
#include "TPad.h"
#include "TStyle.h"
#include "TH1.h"
#include "TH2.h"
#include "TString.h"
#include "TROOT.h"
#include "TSeqCollection.h"
#include "TTask.h"
#include "TClass.h"
#include "TDataMember.h"
#include "TMethodCall.h"

// ---- Default constructor -------------------------------------------
CbmTrdOnlineDisplay::CbmTrdOnlineDisplay()
  :FairTask("CbmTrdOnlineDisplay"),
   fSpadic1(),
   fSpadic1a(),
   fUpdateInterval(10),
   fEventCounter(0)
{
}

// ---- Destructor ----------------------------------------------------
CbmTrdOnlineDisplay::~CbmTrdOnlineDisplay()
{
//  delete[] fSpadic1;
//  delete[] fSpadic1a;
}

// ----  Initialisation  ----------------------------------------------
void CbmTrdOnlineDisplay::SetParContainers()
{
}

// ---- Init ----------------------------------------------------------
InitStatus CbmTrdOnlineDisplay::Init()
{

  TH1* h1 = NULL;
  TH1* h2 = NULL;
  gROOT->cd();
  Float_t lsize=0.07;
  gStyle->SetPalette(1);
  gStyle->SetLabelSize(lsize);
  for (Int_t sys = 0; sys < 2; sys++){
    for (Int_t spa = 0; spa < 2; spa++){
      fSpadic1[spa][sys] = new TCanvas(TString("fSpadic" + std::to_string(spa) + "SysCore" + std::to_string(sys)), TString("Spadic" + std::to_string(spa) + "SysCore" + std::to_string(sys)), 0, 0, 600, 400);
      fSpadic1[spa][sys]->Divide(3,2);

      // Should be set for each pad of the Canvas
      gPad->SetFillColor(0);

      fSpadic1[spa][sys]->cd(1);
      h1=static_cast<TH1*>(gROOT->FindObjectAny(TString("CountRate_SysCore" + std::to_string(sys) + "_Spadic" + std::to_string(spa))));
      if (h1!=NULL) {
	h1->Draw("");
      }

      fSpadic1[spa][sys]->cd(2)->SetLogz(1);
      h2=static_cast<TH2*>(gROOT->FindObjectAny(TString("BaseLine_SysCore" + std::to_string(sys) + "_Spadic" + std::to_string(spa))));
      if (h2!=NULL) {
	h2->Draw("COLZ");
      }

      fSpadic1[spa][sys]->cd(3)->SetLogz(1);
      h2=static_cast<TH2*>(gROOT->FindObjectAny(TString("Integrated_ADC_Spectrum_SysCore" + std::to_string(sys) + "_Spadic" + std::to_string(spa))));
      if (h2!=NULL) {
	h2->Draw("COLZ");
      }

      fSpadic1[spa][sys]->cd(4);
      h2=static_cast<TH2*>(gROOT->FindObjectAny(TString("Trigger_Heatmap_SysCore" + std::to_string(sys) + "_Spadic" + std::to_string(spa))));
      if (h2!=NULL) {
	h2->Draw("COLZ");
      }
      fSpadic1[spa][sys]->cd(5);
      //Int_t maxTrigger = 0;
      //for (Int_t sys = 0; sys < 3; sys++){
      //for (Int_t spa = 0; spa < 3; spa++){
      //h1=static_cast<TH1*>(gROOT->FindObjectAny(TString("TriggerCounter_SysCore" + std::to_string(sys) + "_Spadic" + std::to_string(spa))));
      //if (h1->GetMaximum() > maxTrigger)
      //maxTrigger = h1->GetMaximum();
      //}
      //}
      h1=static_cast<TH1*>(gROOT->FindObjectAny(TString("TriggerCounter_SysCore" + std::to_string(sys) + "_Spadic" + std::to_string(spa))));
      h1->Draw("");

      fSpadic1[spa][sys]->cd(6);
      //h1=static_cast<TH1*>(gROOT->FindObjectAny(TString("CountRate_SysCore" + std::to_string(sys) + "_Spadic" + std::to_string(spa))));
      h1=static_cast<TH1*>(gROOT->FindObjectAny(TString("TriggerSum")));
      h1->Draw("");
    }
  }
  //if (maxTrigger < 10) maxTrigger = 10;
  /*
    h1=static_cast<TH1*>(gROOT->FindObjectAny("TriggerCounter_SysCore0_Spadic0"));
    if (h1!=NULL) {
    h1->GetYaxis()->SetRangeUser(-10,10+1.125*maxTrigger);
    h1->Draw("");
    }
    h1=static_cast<TH1*>(gROOT->FindObjectAny("TriggerCounter_SysCore0_Spadic1"));
    if (h1!=NULL) {
    h1->SetLineColor(2);
    h1->Draw("same");
    }
    h1=static_cast<TH1*>(gROOT->FindObjectAny("TriggerCounter_SysCore0_Spadic2"));
    if (h1!=NULL) {
    h1->SetLineColor(3);
    h1->Draw("same");
    }
  */
  for (Int_t sys = 0; sys < 2; sys++){
    for (Int_t spa = 0; spa < 2; spa++) {
      fSpadic1a[spa][sys] = new TCanvas(TString("fSpadic"+ std::to_string(spa) +"a_sys"+ std::to_string(sys)), TString("Spadic"+ std::to_string(spa) +"_Signal_Shape_sys"+ std::to_string(sys)), 50, 50, 600, 350);
      fSpadic1a[spa][sys]->Divide(8,4);	
      gPad->SetFillColor(0);

      for (Int_t i=0; i<32; i++) {
	fSpadic1a[spa][sys]->cd(i+1)->SetLogz(1);
      
	if (true){
	  TString temphistname = "Signal_Shape_SysCore"+std::to_string(sys)+"_Spadic"+std::to_string(spa)+"_Ch";
	  if(i<10) {
	    temphistname = temphistname + std::to_string(0) + std::to_string(i);
	    h2=static_cast<TH2*>(gROOT->FindObjectAny(temphistname.Data()));
	    if (h2!=NULL) {
	      h2->Draw("COLZ");
	    }
	  } else {
	    temphistname = temphistname + std::to_string(i);
	    h2=static_cast<TH2*>(gROOT->FindObjectAny(temphistname.Data()));
	    if (h2!=NULL) {
	      h2->Draw("COLZ");
	    }
	  }
	} else {
	  //for(Int_t sys = 0; sys < 2; sys++){
	  //for(Int_t spa = 0; spa < 3; spa++){
	  TString temphistname = "Signal_Shape_SysCore"+std::to_string(sys)+"_Spadic"+std::to_string(spa)+"_Ch" + std::to_string(0) + std::to_string(i);
	  h1=static_cast<TH1*>(gROOT->FindObjectAny(temphistname.Data()));
	  if (h1!=NULL) {
	    h1->SetLineColor(sys);
	    h1->SetLineStyle(spa);
	    if (sys == 0 && spa == 0)
	      h1->Draw("C");
	    else
	      h1->Draw("C,same");
	  }
	  // }
	}
      }
    }
  }

  /*
    CbmTrdRawBeamProfile* c = static_cast<CbmTrdRawBeamProfile*>(gROOT->FindObjectAny("CbmTrdRawBeamProfile")).Data()); 
    LOG(INFO)<<"Found class "<< c->GetName() << FairLogger::endl;

    TClass *cl = c->IsA();
    TDataMember *dm = cl->GetDataMember("fHM");
    TMethodCall *getter = dm->GetterMethod(c); 

  
    CbmHistManager* HM_RawProfile;
    getter->Execute(c,"",HM_RawProfile); 

    if (HM_RawProfile)
    LOG(INFO) << "Hier bin ich" << HM_RawProfile->GetName() << FairLogger::endl;

    TList* list = gROOT->GetListOfBrowsables();
    TIter nextT(list);
    TObject *t = NULL;
    while ((t = (TObject *) nextT())) {
    LOG(INFO) << "Found obj " << t->GetName();
    }
  */

  return kSUCCESS;

}

// ---- ReInit  -------------------------------------------------------
InitStatus CbmTrdOnlineDisplay::ReInit()
{
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void CbmTrdOnlineDisplay::Exec(Option_t* option)
{
  Int_t maxTrigger = 0;
  fEventCounter++;
  TH1* h1=NULL;
  if ( 0 == fEventCounter%(fUpdateInterval*10) ) {
    LOG(INFO)<<"Update Canvas for Event "<< fEventCounter << FairLogger::endl; 
    for(Int_t sys = 0; sys < 2; sys++){
      for (Int_t spa = 0; spa < 2; spa++){
	for(Int_t iCh=0; iCh<32; iCh++){
	  fSpadic1a[spa][sys]->cd(iCh+1);
	  gPad->Modified();
	  gPad->Update();
	}
      }
    } 
  }
  if ( 0 == fEventCounter%(fUpdateInterval/10)) {
   
    for (Int_t sys = 0; sys < 2; sys++){
      for (Int_t spa = 0; spa < 2; spa++){
	h1=static_cast<TH1*>(gROOT->FindObjectAny(TString("TriggerCounter_SysCore" + std::to_string(sys) + "_Spadic" + std::to_string(spa))));
	if (h1->GetBinContent(h1->GetMaximumBin()) > maxTrigger)
	  maxTrigger = h1->GetBinContent(h1->GetMaximumBin());
      }
    } 
    for (Int_t sys = 0; sys < 2; sys++){
      for (Int_t spa = 0; spa < 2; spa++){
	h1=static_cast<TH1*>(gROOT->FindObjectAny(TString("TriggerCounter_SysCore" + std::to_string(sys) + "_Spadic" + std::to_string(spa))));
	//h1->GetYaxis()->SetRangeUser(-10,10+1.125*maxTrigger);
	for(Int_t iCh=0; iCh<6; iCh++){
	  fSpadic1[spa][sys]->cd(iCh+1);
	  gPad->Modified();
	  gPad->Update();
	}
      }
    }
  }
}

  // ---- Finish --------------------------------------------------------
  void CbmTrdOnlineDisplay::Finish()
  {
    for (Int_t sys = 0; sys < 2; sys++){
      for (Int_t spa = 0; spa < 2; spa++)
	fSpadic1[spa][sys]->Update();
    }
  }
  ClassImp(CbmTrdOnlineDisplay)
