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
#include "CbmRichRingLight.h"
#include "CbmRichRingFitterCOP.h"
#include "CbmRichRingFitterEllipseTau.h"
#include "CbmRichConverter.h"


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

	fRichRings = (TClonesArray*)manager->GetObject("RichRing");
	if (NULL == fRichRings) { Fatal("CbmRichTrbRawQa::Init","No RichRing array!"); }

	fCopFit = new CbmRichRingFitterCOP();
	fTauFit = new CbmRichRingFitterEllipseTau();
	InitHist();

	return kSUCCESS;
}

void CbmRichTrbRecoQa::InitHist()
{
	fhNofHitsInEvent = new TH1D("fhNofHitsInEvent", "fhNofHitsInEvent;Number of hits in event;Entries", 50, -0.5, 49.5);
	fhHitsXY = new TH2D("fhHitsXY", "fhHitsXY;X [mm];Y [mm];Entries", 32, 4., 210., 32, 4., 210.);
	fhNofRingsInEvent = new TH1D("fhNofRingsInEvent", "fhNofRingsInEvent;# rings per event;Entries", 5, -0.5, 4.5);
    fhBoverAEllipse = new TH1D("fhBoverAEllipse", "fhBoverAEllipse;B/A;Entries", 100, 0.0, 1.0);
    fhXcYcEllipse = new TH2D("fhXcYcEllipse", "fhXcYcEllipse;X [mm];Y [mm];Entries", 100, 0., 210., 100, 0., 210.);
    fhBaxisEllipse = new TH1D("fhBaxisEllipse", "fhBaxisEllipse;B axis [mm];Entries", 100, 0.0, 10.0);
    fhAaxisEllipse = new TH1D("fhAaxisEllipse", "fhAaxisEllipse;A axis [mm];Entries", 100, 0.0, 10.0);
    fhChi2Ellipse = new TH1D("fhChi2Ellipse", "fhChi2Ellipse;#Chi^{2};Entries", 100, 0.0, 10.);
    fhXcYcCircle = new TH2D("fhXcYcCircle", "fhXcYcCircle;X [mm];Y [mm];Entries", 100, 0., 210., 100, 0., 210.);
    fhRadiusCircle = new TH1D("fhRadiusCircle", "fhRadiusCircle;Radius [mm];Entries", 100, 0.0, 10.0);
    fhChi2Circle = new TH1D("fhChi2Circle", "fhChi2Circle;#Chi^{2};Entries", 100, 0.0, 10.0);
    fhDrCircle = new TH1D("fhDrCircle", "fhDrCircle;dR;Entries", 100, -1.0, 1.0);
}

void CbmRichTrbRecoQa::Exec(
		Option_t* option)
{
	fEventNum++;
	LOG(DEBUG) << "Event #" << fEventNum << FairLogger::endl;

	LOG(DEBUG) << "nof hits in event = " << fRichHits->GetEntries() << FairLogger::endl;

	Int_t nofHitsInEvent = fRichHits->GetEntries();
	fhNofHitsInEvent->Fill(nofHitsInEvent);
	for (Int_t iH = 0; iH < nofHitsInEvent; iH++) {
		CbmRichHit* hit = static_cast<CbmRichHit*>(fRichHits->At(iH));
		fhHitsXY->Fill(hit->GetX(), hit->GetY());
	}

	Int_t nofRingsInEvent = fRichRings->GetEntries();
	fhNofRingsInEvent->Fill(nofRingsInEvent);
	vector<CbmRichRingLight> fitCircleRing;
	vector<CbmRichRingLight> fitEllipseRing;
	for (Int_t iR = 0; iR < nofRingsInEvent; iR++) {
		CbmRichRing* ring = static_cast<CbmRichRing*>(fRichRings->At(iR));
		CbmRichRingLight ringL;
		CbmRichConverter::CopyHitsToRingLight(ring, &ringL);
		fTauFit->DoFit(&ringL);
		FillHistEllipse(&ringL);
		fitEllipseRing.push_back(ringL);
		fCopFit->DoFit(&ringL);
		FillHistCircle(&ringL);
		fitCircleRing.push_back(ringL);
	}

	if (fNofDrawnEvents <= 10) {
		DrawEvent(fitCircleRing, fitEllipseRing);
	}
}

void CbmRichTrbRecoQa::FillHistEllipse(
      CbmRichRingLight* ring)
{
   Double_t axisA = ring->GetAaxis();
   Double_t axisB = ring->GetBaxis();
   Double_t xcEllipse = ring->GetCenterX();
   Double_t ycEllipse = ring->GetCenterY();
   Int_t nofHitsRing = ring->GetNofHits();

   fhBoverAEllipse->Fill(axisB/axisA);
   fhXcYcEllipse->Fill(xcEllipse, ycEllipse);
   fhNofHitsInRing->Fill(nofHitsRing);
   fhBaxisEllipse->Fill(axisB);
   fhAaxisEllipse->Fill(axisA);
   fhChi2Ellipse->Fill(ring->GetChi2()/ring->GetNofHits());
}

void CbmRichTrbRecoQa::FillHistCircle(
      CbmRichRingLight* ring)
{
   Double_t radius = ring->GetRadius();
   Double_t xcCircle = ring->GetCenterX();
   Double_t ycCircle = ring->GetCenterY();
   Int_t nofHitsRing = ring->GetNofHits();
   fhXcYcCircle->Fill(xcCircle, ycCircle);
   fhRadiusCircle->Fill(radius);
   fhChi2Circle->Fill(ring->GetChi2()/ring->GetNofHits());

   for (Int_t iH = 0; iH < ring->GetNofHits(); iH++){
      Double_t xh = ring->GetHit(iH).fX;
      Double_t yh = ring->GetHit(iH).fY;
      Double_t dr = radius - sqrt((xcCircle - xh)*(xcCircle - xh) + (ycCircle - yh)*(ycCircle - yh));
      fhDrCircle->Fill(dr);
   }
}

void CbmRichTrbRecoQa::DrawHist()
{
	TCanvas* c1 = new TCanvas("rich_trb_recoqa_nof_hits_in_event", "rich_trb_recoqa_nof_hits_in_event", 600, 600);
	DrawH1(fhNofHitsInEvent);

	TCanvas* c2 = new TCanvas("rich_trb_recoqa_hits_xy", "rich_trb_recoqa_hits_xy", 600, 600);
	DrawH2(fhHitsXY);
}

void CbmRichTrbRecoQa::DrawEvent(
    		const vector<CbmRichRingLight>& fitRingCircle,
    		const vector<CbmRichRingLight>& fitRingEllipse)
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

   //Draw all found rings and centers
   if (fitRingCircle.size() != fitRingEllipse.size()) {
	   LOG(ERROR) << "CbmRichTrbRecoQa::DrawEvent fitRingCircle.size() != fitRingEllipse.size()" << FairLogger::endl;
   }
   Int_t nofRings = fitRingCircle.size();
   for (Int_t iR = 0; iR < nofRings; iR++) {
	   CbmRichRingLight r = fitRingCircle[iR];
	   TEllipse* circle = new TEllipse(r.GetCenterX(), r.GetCenterY(), r.GetRadius());
	   circle->SetFillStyle(0);
	   circle->SetLineWidth(3);
	   circle->Draw();
	   TEllipse* center = new TEllipse(r.GetCenterX(), r.GetCenterY(), 1.5);
	   center->Draw();

	   // Draw hits from the ring
	   for (int iH = 0; iH < r.GetNofHits(); iH++){
		   CbmRichHitLight hit = r.GetHit(iH);
		   TEllipse* hitDr = new TEllipse(hit.fX, hit.fY, 1.5);
		   hitDr->SetFillColor(kRed);
		   hitDr->Draw();
	   }
   }


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
