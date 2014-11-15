#include "CbmRichTrbRecoQa.h"
#include "FairRootManager.h"
#include "FairLogger.h"

#include "TClonesArray.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "CbmDrawHist.h"
#include "CbmRichHit.h"
#include "TEllipse.h"
#include "TLatex.h"

#include <iostream>
#include <vector>

CbmRichTrbRecoQa::CbmRichTrbRecoQa():
	fEventNum(0),
	fNofDrawnEvents(0)
{
	;
}

CbmRichTrbRecoQa::~CbmRichTrbRecoQa()
{
	;
}


InitStatus CbmRichTrbRecoQa::Init()
{
	FairRootManager* manager = FairRootManager::Instance();
	fRichHits = (TClonesArray*)manager->GetObject("RichHit");
	if (NULL == fRichHits) { Fatal("CbmRichTrbRawQa::Init","No RichHit array!"); }

	InitHist();
}

void CbmRichTrbRecoQa::InitHist()
{
	fhNofHitsInEvent = new TH1D("fhNofHitsInEvent", "fhNofHitsInEvent;Number of hits in event;Entries", 50, -0.5, 49.5);
	fhHitsXY = new TH2D("fhHitsXY", "fhHitsXY;X [cm];Y [cm];Entries", 32, 4., 210., 32, 4., 210.);
}

void CbmRichTrbRecoQa::Exec(
		Option_t* option)
{
	fEventNum++;
	LOG(INFO) << "Event #" << fEventNum << FairLogger::endl;

	LOG(INFO) << "nof hits in event = " << fRichHits->GetEntries() << FairLogger::endl;

	Int_t nofHitsInEvent = fRichHits->GetEntries();
	fhNofHitsInEvent->Fill(nofHitsInEvent);
	for (Int_t iH = 0; iH < nofHitsInEvent; iH++) {
		CbmRichHit* hit = static_cast<CbmRichHit*>(fRichHits->At(iH));
		fhHitsXY->Fill(hit->GetX(), hit->GetY());
	}

	if (fNofDrawnEvents <= 10) {
		DrawEvent();
	}
}

void CbmRichTrbRecoQa::DrawHist()
{

	TCanvas* c1 = new TCanvas("rich_trb_recoqa_nof_hits_in_event", "rich_trb_recoqa_nof_hits_in_event", 600, 600);
	DrawH1(fhNofHitsInEvent);

	TCanvas* c2 = new TCanvas("rich_trb_recoqa_hits_xy", "rich_trb_recoqa_hits_xy", 600, 600);
	DrawH2(fhHitsXY);
}

void CbmRichTrbRecoQa::DrawEvent()
{
   TString ss;
   ss.Form("rich_trb_recoqa_eventdisplay_%d", fNofDrawnEvents);
   fNofDrawnEvents++;
   TCanvas *c = new TCanvas(ss.Data(), ss.Data(), 800, 800);
   c->SetGrid(true, true);
   TH2D* pad = new TH2D("pad", ";X [cm];Y [cm]", 1, 4., 210., 1, 4., 210);
   DrawH2(pad);
   //gPad->SetLeftMargin(0.17);
   gPad->SetRightMargin(0.12);
  // gPad->SetBottomMargin(0.15);
   pad->SetStats(false);
   //pad->Draw();

   //Draw event hits
   Int_t nofHitsInEvent = fRichHits->GetEntries();
   for (Int_t iH = 0; iH < nofHitsInEvent; iH++){
	   CbmRichHit* hit = static_cast<CbmRichHit*>(fRichHits->At(iH));
	   TEllipse* hitDr = new TEllipse(hit->GetX(), hit->GetY(), 2.5);
	   hitDr->SetFillColor(kGreen);
	   hitDr->Draw();
   }
/*
   //Draw circle and center
   TEllipse* circle = new TEllipse(ringHit->GetCenterX() - xCur,
         ringHit->GetCenterY() - yCur, ringHit->GetRadius());
   circle->SetFillStyle(0);
   circle->SetLineWidth(3);
   circle->Draw();
   TEllipse* center = new TEllipse(ringHit->GetCenterX() - xCur, ringHit->GetCenterY() - yCur, .5);
   center->Draw();

   // Draw hits
   for (int i = 0; i < ringHit->GetNofHits(); i++){

   }

   // Draw MC Points
   for (int i = 0; i < ringPoint->GetNofHits(); i++){
      TEllipse* pointDr = new TEllipse(ringPoint->GetHit(i).fX - xCur, ringPoint->GetHit(i).fY - yCur, 0.15);
      pointDr->SetFillColor(kBlue);
      pointDr->Draw();
   }
*/
   //Draw information
   ss.Form("(nEv)=(%d)", nofHitsInEvent);
   TLatex* latex = new TLatex(5., 225., ss.Data());
   latex->SetTextFont(30);
   latex->Draw();
}

void CbmRichTrbRecoQa::Finish()
{
	DrawHist();
}

ClassImp(CbmRichTrbRecoQa)
