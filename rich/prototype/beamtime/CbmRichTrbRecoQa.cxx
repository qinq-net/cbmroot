#include "CbmRichTrbRecoQa.h"
#include "FairRootManager.h"
#include "FairLogger.h"

#include "TClonesArray.h"

#include "TROOT.h"
#include "TFolder.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "CbmDrawHist.h"
#include "CbmRichHit.h"
#include "CbmRichHitInfo.h"
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

	fRichHitInfos = (TClonesArray*)manager->GetObject("RichHitInfo");
	if (NULL == fRichHitInfos) { Fatal("CbmRichTrbRawQa::Init","No RichHitInfo array!"); }

	fCopFit = new CbmRichRingFitterCOP();
	fTauFit = new CbmRichRingFitterEllipseTau();
	CbmRichConverter::Init();

	InitHist();

	return kSUCCESS;
}

void CbmRichTrbRecoQa::InitHist()
{
   fhNofHitsInEvent = new TH1D("fhNofHitsInEvent", "fhNofHitsInEvent;Number of hits in event;Entries", 50, 0.5, 50.5);
   fhNofHitsInRing = new TH1D("fhNofHitsInRing", "fhNofHitsInRing;Number of hits in ring;Entries", 50, -0.5, 49.5);
   fhHitsXYPixel = new TH2D("fhHitsXYPixel", "fhHitsXYPixel;X [pixel];Y [pixel];Entries", 32, 0.5, 32.5, 32, 0.5, 32.5);
   fhNofRingsInEvent = new TH1D("fhNofRingsInEvent", "fhNofRingsInEvent;# rings per event;Entries", 5, -0.5, 4.5);
   fhBoverAEllipse = new TH1D("fhBoverAEllipse", "fhBoverAEllipse;B/A;Entries", 100, 0.0, 1.0);
   fhXcYcEllipse = new TH2D("fhXcYcEllipse", "fhXcYcEllipse;X [cm];Y [cm];Entries", 100, 0., 21., 100, 0., 21.);
   fhBaxisEllipse = new TH1D("fhBaxisEllipse", "fhBaxisEllipse;B axis [cm];Entries", 100, 0.0, 10.0);
   fhAaxisEllipse = new TH1D("fhAaxisEllipse", "fhAaxisEllipse;A axis [cm];Entries", 100, 0.0, 10.0);
   fhChi2Ellipse = new TH1D("fhChi2Ellipse", "fhChi2Ellipse;#Chi^{2};Entries", 100, 0.0, 2.);
   fhXcYcCircle = new TH2D("fhXcYcCircle", "fhXcYcCircle;X [cm];Y [cm];Entries", 100, 0., 21., 100, 0., 21.);
   fhRadiusCircle = new TH1D("fhRadiusCircle", "fhRadiusCircle;Radius [cm];Entries", 100, 0.0, 10.0);
   fhChi2Circle = new TH1D("fhChi2Circle", "fhChi2Circle;#Chi^{2};Entries", 100, 0.0, 2.0);
   fhDrCircle = new TH1D("fhDrCircle", "fhDrCircle;dR [cm];Entries", 100, -1.0, 1.0);

   fhEventsWithRings = new TH1D("fhEventsWithRings", "fhEventsWithRings", 2, 0, 2);
   fhEventsWithRings->SetMinimum(0.);
   fhEventsWithRings->GetXaxis()->SetBinLabel(1, "Num of events total");
   fhEventsWithRings->GetXaxis()->SetBinLabel(2, "Num of events w/rings");

   fhHitsPerPMT = new TH1D("fhHitsPerPMT", "fhHitsPerPMT", 16, 0, 16);
}

void CbmRichTrbRecoQa::Exec(
		Option_t* option)
{
	fEventNum++;
	LOG(DEBUG2) << "CbmRichTrbRecoQa : Event #" << fEventNum << FairLogger::endl;

	LOG(DEBUG2) << "nof hits in event = " << fRichHits->GetEntries() << FairLogger::endl;

	Int_t nofHitsInEvent = fRichHits->GetEntries();
	fhNofHitsInEvent->Fill(nofHitsInEvent);

	Int_t nofHitInfosInEvent = fRichHitInfos->GetEntries();
	for (Int_t iH = 0; iH < nofHitInfosInEvent; iH++) {
		CbmRichHitInfo* hit = static_cast<CbmRichHitInfo*>(fRichHitInfos->At(iH));
		fhHitsXYPixel->Fill(hit->GetXPixel(), hit->GetYPixel());

	  //LOG(DEBUG4) << hit->GetPmtId() << FairLogger::endl;
	  //fhHitsPerPMT->Fill(hit->GetPmtId());
	}



	Int_t nofRingsInEvent = fRichRings->GetEntries();
	fhNofRingsInEvent->Fill(nofRingsInEvent);

   fhEventsWithRings->Fill(0);
   if (nofRingsInEvent >= 1) fhEventsWithRings->Fill(1);

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


	if (nofHitsInEvent >= 5 && fNofDrawnEvents <= 10 ) {
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
   fhChi2Ellipse->Fill(ring->GetChi2()/nofHitsRing);
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

void CbmRichTrbRecoQa::SaveHist()
{
   TFolder* rootHistFolder; // Folder for histograms

   rootHistFolder = gROOT->GetRootFolder()->AddFolder("rich_trb_recoqa", "rich_trb_recoqa");

   rootHistFolder->Add(fhNofHitsInEvent);
   rootHistFolder->Add(fhHitsXYPixel);
   rootHistFolder->Add(fhNofRingsInEvent);
   rootHistFolder->Add(fhNofHitsInRing);

   rootHistFolder->Add(fhBaxisEllipse);
   rootHistFolder->Add(fhAaxisEllipse);
   rootHistFolder->Add(fhBoverAEllipse);
   rootHistFolder->Add(fhChi2Ellipse);
   rootHistFolder->Add(fhXcYcEllipse);

   rootHistFolder->Add(fhXcYcCircle);
   rootHistFolder->Add(fhRadiusCircle);
   rootHistFolder->Add(fhChi2Circle);
   rootHistFolder->Add(fhDrCircle);

   rootHistFolder->Add(fhEventsWithRings);
   rootHistFolder->Add(fhHitsPerPMT);



   TCanvas* c = new TCanvas("rich_trb_recoqa", "rich_trb_recoqa", 1000, 1000);

   c->Divide(2, 2);

   c->cd(1);
   DrawH2(fhHitsXYPixel);  // fhHitsXY instead of fhHitsXY2
   c->cd(2);
   DrawH1(fhNofHitsInRing);
   c->cd(3);
   DrawH1(fhEventsWithRings);
   c->cd(4);
   DrawH1(fhHitsPerPMT);


   fhNofHitsInRing->SetStats(true);
   fhEventsWithRings->SetStats(true);
   fhHitsPerPMT->SetStats(true);
   fhHitsXYPixel->SetStats(true);

   rootHistFolder->Add(c);

   c->SaveAs("out.png");

   rootHistFolder->Write();
}

void CbmRichTrbRecoQa::DrawHist()
{
   TCanvas* c1 = new TCanvas("rich_trb_recoqa_nof_hits_in_event", "rich_trb_recoqa_nof_hits_in_event", 600, 600);
   DrawH1(fhNofHitsInEvent);

   TCanvas* c3 = new TCanvas("rich_trb_recoqa_nof_rings_per_event", "rich_trb_recoqa_nof_rings_per_event", 600, 600);
   DrawH1(fhNofRingsInEvent);
   TCanvas* c4 = new TCanvas("rich_trb_recoqa_nof_hits_per_ring", "rich_trb_recoqa_nof_hits_per_ring", 600, 600);
   DrawH1(fhNofHitsInRing);

   TCanvas* c5 = new TCanvas("rich_trb_recoqa_ellipse_param", "rich_trb_recoqa_ellipse_param", 1200, 800);
   c5->Divide(3, 2);
	LOG(INFO) << "Number of rings = " << fhNofRingsInEvent->GetEntries() << FairLogger::endl;

   c5->cd(1);
   DrawH1(fhBaxisEllipse);
   c5->cd(2);
   DrawH1(fhAaxisEllipse);
   c5->cd(3);
   DrawH1(fhBoverAEllipse);
   c5->cd(4);
   DrawH1(fhChi2Ellipse);
   c5->cd(5);
   DrawH2(fhXcYcEllipse);

   TCanvas* c6 = new TCanvas("rich_trb_recoqa_circle_param", "rich_trb_recoqa_circle_param", 800, 800);
   c6->Divide(2, 2);

   c6->cd(1);
   DrawH2(fhXcYcCircle);
   c6->cd(2);
   DrawH1(fhRadiusCircle);
   c6->cd(3);
   DrawH1(fhChi2Circle);
   c6->cd(4);
   DrawH1(fhDrCircle);

   TCanvas* c7 = new TCanvas("rich_trb_recoqa_hits_xy_pixel", "rich_trb_recoqa_hits_xy_pixel", 800, 600);
   DrawH2(fhHitsXYPixel);

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
   TH2D* pad = new TH2D("pad", ";X [cm];Y [cm]", 1, 0.0, 21., 1, 0.0, 21.);
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
	   TEllipse* hitDr = new TEllipse(hit->GetX(), hit->GetY(), 0.25);
	   hitDr->SetFillColor(kGreen);
	   hitDr->Draw();
   }

   //Draw all found rings and centers
   if (fitRingCircle.size() != fitRingEllipse.size()) {
	   LOG(ERROR) << "CbmRichTrbRecoQa::DrawEvent fitRingCircle.size() != fitRingEllipse.size()" << FairLogger::endl;
   }

   Int_t nofRings = fitRingCircle.size();
   for (Int_t iR = 0; iR < nofRings; iR++) {
	   CbmRichRingLight rCircle = fitRingCircle[iR];
	   TEllipse* circle = new TEllipse(rCircle.GetCenterX(), rCircle.GetCenterY(), rCircle.GetRadius());
	   circle->SetFillStyle(0);
	   circle->SetLineWidth(3);
	   circle->Draw();
	   TEllipse* cCircle = new TEllipse(rCircle.GetCenterX(), rCircle.GetCenterY(), 0.15);
	   cCircle->Draw();

	   CbmRichRingLight rEllipse = fitRingEllipse[iR];
	   TEllipse* ellipse = new TEllipse(rEllipse.GetCenterX(), rEllipse.GetCenterY(), rEllipse.GetAaxis(), rEllipse.GetBaxis());
	   ellipse->SetFillStyle(0);
	   ellipse->SetLineWidth(3);
	   ellipse->SetLineColor(kBlue);
	   ellipse->Draw();
	   TEllipse* cEllipse = new TEllipse(rEllipse.GetCenterX(), rEllipse.GetCenterY(), 0.15);
	   cEllipse->Draw();

	   // Draw hits from the ring
	   for (int iH = 0; iH < rCircle.GetNofHits(); iH++){
		   CbmRichHitLight hit = rCircle.GetHit(iH);
		   TEllipse* hitDr = new TEllipse(hit.fX, hit.fY, 0.15);
		   hitDr->SetFillColor(kRed);
		   hitDr->Draw();
	   }
	   //Draw information
	   ss.Form("#font[12]{nEv=%d, nR=%d}", nofHitsInEvent, rCircle.GetNofHits());
	   TLatex* latex = new TLatex(0.5, 22.5 - 1.0 * iR, ss.Data());
	   //latex->SetTextFont(10);
	   latex->Draw();
   }
}

void CbmRichTrbRecoQa::Finish()
{
	//DrawHist();
   SaveHist();
}

ClassImp(CbmRichTrbRecoQa)
