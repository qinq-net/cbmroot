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
   fSpadic1(NULL),
   fSpadic1a(NULL),
   fUpdateInterval(10),
   fEventCounter(0)
{
}

// ---- Destructor ----------------------------------------------------
CbmTrdOnlineDisplay::~CbmTrdOnlineDisplay()
{
  delete fSpadic1;
  delete fSpadic1a;
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

  fSpadic1 = new TCanvas("fSpadic1", "Spadic1", 0, 0, 1010, 810);
  fSpadic1->Divide(3,2);

  // Should be set for each pad of the Canvas
  gPad->SetFillColor(0);

  fSpadic1->cd(1);
  h1=static_cast<TH1*>(gROOT->FindObjectAny("CountRate_SysCore1_Spadic1"));
  if (h1!=NULL) {
    h1->Draw("");
  }

  fSpadic1->cd(2);
  h2=static_cast<TH2*>(gROOT->FindObjectAny("BaseLine_SysCore1_Spadic1"));
  if (h2!=NULL) {
    h2->Draw("");
  }

  fSpadic1->cd(3);
  h2=static_cast<TH2*>(gROOT->FindObjectAny("Integrated_ADC_Spectrum_SysCore1_Spadic1"));
  if (h2!=NULL) {
    h2->Draw("");
  }

  fSpadic1->cd(4);
  h2=static_cast<TH2*>(gROOT->FindObjectAny("Trigger_Heatmap_SysCore1_Spadic1"));
  if (h2!=NULL) {
    h2->Draw("COLZ");
  }
  fSpadic1->cd(5);

 

  TCanvas* fSpadic1a = new TCanvas("fSpadic1a", "Spadic1_Signal_Shape", 50, 50, 1510, 810);
  fSpadic1a->Divide(8,4);	
  gPad->SetFillColor(0);

  for (Int_t i=0; i<32; i++) {
    fSpadic1a->cd(i+1);
    TString temphistname = "Signal_Shape_SysCore1_Spadic1_Ch";
    if(i<10) {
      temphistname = temphistname + std::to_string(0) + std::to_string(i);
      h2=static_cast<TH2*>(gROOT->FindObjectAny(temphistname.Data()));
      if (h2!=NULL) {
        h2->Draw("");
      }
    } else {
      temphistname = temphistname + std::to_string(i);
      h2=static_cast<TH2*>(gROOT->FindObjectAny(temphistname.Data()));
      if (h2!=NULL) {
        h2->Draw("");
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

  fEventCounter++;
  
  if ( 0 == fEventCounter%fUpdateInterval ) {
    LOG(DEBUG)<<"Update Canvas for Event "<< fEventCounter << FairLogger::endl;
    for(Int_t iCh=0; iCh<5; iCh++){
      fSpadic1->cd(iCh+1);
      gPad->Modified();
      gPad->Update();
    }
    fSpadic1->Modified();
    fSpadic1->Update();
  } 

}

// ---- Finish --------------------------------------------------------
void CbmTrdOnlineDisplay::Finish()
{
  fSpadic1->Update();
}

ClassImp(CbmTrdOnlineDisplay)
