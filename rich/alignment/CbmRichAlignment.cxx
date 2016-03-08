// ---------- Original Headers ---------- //
#include "CbmRichAlignment.h"
#include "FairRootManager.h"
#include "FairLogger.h"

#include "TClonesArray.h"

#include "TF1.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "CbmDrawHist.h"
#include "CbmRichHit.h"

#include <iostream>
#include <vector>

// ---------- Included Headers ---------- //
#include "TGeoManager.h"
#include "CbmRichPoint.h"
#include "FairVolume.h"
#include "CbmRichConverter.h"
#include "TEllipse.h"
#include "CbmRichRingLight.h"
#include "CbmRichRingFitterCOP.h"
#include "CbmRichRingFitterEllipseTau.h"
#include "CbmUtils.h"
#include "FairTrackParam.h"
#include "CbmTrackMatchNew.h"
#include "CbmMCTrack.h"
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
//#include <stdio.h>


CbmRichAlignment::CbmRichAlignment()
  : FairTask(),
    fRichHits(NULL),
    fRichRings(NULL),
    fRichProjections(NULL),
    fRichPoints(NULL),
	fRichMirrorPoints(NULL),
    fMCTracks(NULL),
    fRichRingMatches(NULL),
    fHM(NULL),
    fEventNum(0),
    /*fNofDrawnEvents(0),
    fMaxNofEventsToDraw(0),*/
    fOutputDir(""),
    fRunTitle(""),
    fAxisRotTitle(""),
    fDrawHist(kFALSE),
    fCopFit(NULL),
    fTauFit(NULL),
    fIsSimulationAna(true), // Default use simulation data.
    fIsMeanPosition(false),
    fPhi()
{
	counter = 0.;
	XmeanCircle = 0.;
	YmeanCircle = 0.;
	XmeanEllipse = 0.;
	YmeanEllipse = 0.;
}

CbmRichAlignment::~CbmRichAlignment()
{
}

InitStatus CbmRichAlignment::Init()
{
    FairRootManager* manager = FairRootManager::Instance();

    fRichHits = (TClonesArray*) manager->GetObject("RichHit");
    if (NULL == fRichHits) { Fatal("CbmRichAlignment::Init","No RichHit array!"); }

    fRichRings = (TClonesArray*) manager->GetObject("RichRing");
    if (NULL == fRichRings) { Fatal("CbmRichAlignment::Init","No RichRing array!"); }

    if (fIsSimulationAna) {
	fRichProjections = (TClonesArray*) manager->GetObject("RichProjection");
	if (NULL == fRichProjections) { Fatal("CbmRichAlignment::Init","No RichProjection array!"); }

	fRichPoints = (TClonesArray*) manager->GetObject("RichPoint");
	if ( NULL == fRichPoints) { Fatal("CbmRichAlignment::Init","No RichPoint array!"); }

	fMCTracks = (TClonesArray*) manager->GetObject("MCTrack");
	if ( NULL == fMCTracks) { Fatal("CbmRichAlignment::Init","No MCTrack array!"); }

	fRichRingMatches = (TClonesArray*) manager->GetObject("RichRingMatch");
	if ( NULL == fRichRingMatches) { Fatal("CbmRichAlignment::Init","No RichRingMatch array!"); }
    }

    fRichMirrorPoints = (TClonesArray*) manager->GetObject("RichMirrorPoint");
    if ( NULL == fRichMirrorPoints) { Fatal("CbmRichPMTMapping::Init","No RichMirrorPoints array!"); }

    fCopFit = new CbmRichRingFitterCOP();
    fTauFit = new CbmRichRingFitterEllipseTau();
    CbmRichConverter::Init();

    InitHist();

    return kSUCCESS;
}

void CbmRichAlignment::InitHist()
{
    fHM = new CbmHistManager();

    fHM->Create2<TH2D>("fHMeanCirclePosition", "fHMeanCirclePosition;X position [cm];Y position [cm];Entries", 128, 0., 15., 100, 5., 20.);

    fHM->Create1<TH1D>("fHCenterDistance", "fHCenterDistance;Distance C-C';Nb of entries", 100, -0.1, 5.);
    fHM->Create1<TH1D>("fHPhi", "fHPhi;Phi_Ch [rad];Nb of entries", 200, -3.4, 3.4);
    fHM->Create1<TH1D>("fHThetaDiff", "fHThetaDiff;Th_Ch-Th_0 [cm];Nb of entries", 252, -5., 5.);
    fHM->Create2<TH2D>("fHCherenkovHitsDistribTheta0", "fHCherenkovHitsDistribTheta0;Phi_0 [rad];Theta_0 [cm];Entries", 200, -2., 2., 600, 2., 8.);
    fHM->Create2<TH2D>("fHCherenkovHitsDistribThetaCh", "fHCherenkovHitsDistribThetaCh;Phi_Ch [rad];Theta_Ch [cm];Entries", 200, -3.4, 3.4, 600, 0., 20);
    fHM->Create2<TH2D>("fHCherenkovHitsDistribReduced", "fHCherenkovHitsDistribReduced;Phi_Ch [rad];Th_Ch-Th_0 [cm];Entries", 200, -3.4, 3.4, 500, -5., 5.);
}

void CbmRichAlignment::Exec(Option_t* /*option*/)
{
    fEventNum++;
    LOG(DEBUG2) << "CbmRichAlignment : Event #" << fEventNum << FairLogger::endl;

    Int_t nofRingsInEvent = fRichRings->GetEntries();
    Int_t nofHitsInEvent = fRichHits->GetEntries();
    //cout << "Nb of rings in evt = " << nofRingsInEvent << " and nb of hits in evt = " << nofHitsInEvent << endl;

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------ //
    // ---------------------------------------- Determination of the mean ring center position from the reference data set ---------------------------------------- //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------ //

    if(fIsMeanPosition) { CalculateMeanRingCenterPosition(); }

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------ //
    // --------------------------------------------------------- Cherenkov angle and azimuth calculations --------------------------------------------------------- //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------ //

    else { CalculateAnglesAndDrawDistrib(); }
}

void CbmRichAlignment::GetTrackPosition(Double_t *x, Double_t *y)
{
    if (!fIsSimulationAna) {
    	// {8.071980625, 13.57793125} ADD THE VALUE OF THE RING MEAN POSITION FROM THE REF DATA SET HERE !!! //
    	*x = 8.071980625;
    	*y = 13.57793125;
    }
    else {
    	Int_t NofProjections = fRichProjections->GetEntries();
    	//cout << "!!! NB PROJECTIONS !!! " << NofProjections << endl;
    	for (Int_t iP = 0; iP < NofProjections; iP++) {
    		FairTrackParam* pr = (FairTrackParam*) fRichProjections->At(iP);
    		if (NULL == pr) {
    			*x = 8.071980625;
    			*y = 13.57793125;
    			cout << "GetTrackPosition: No fair track param found, values from ref data set used." << endl;
    			//sleep(2);
    		}
    		*x = pr->GetX();
    		*y = pr->GetY();
    		//cout << "Center X: " << *x << " and Center y: " << *y << endl;
    	}
    }
}

Bool_t CbmRichAlignment::IsRingMcElectron(Int_t ringIndex)
{
    if (!fIsSimulationAna) return true;
    CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(ringIndex);
    if (NULL == ringMatch) return false;
    Int_t mcTrackId = ringMatch->GetMatchedLink().GetIndex();
    if (mcTrackId < 0) return false;
    CbmMCTrack* mcTrack = (CbmMCTrack*)fMCTracks->At(mcTrackId);
    if (!mcTrack) return false;
    Int_t motherId = mcTrack->GetMotherId();
    Int_t pdg = TMath::Abs(mcTrack->GetPdgCode());
    if (pdg == 11 && motherId == -1) {
    	return true;
    }
    return false;
}

void CbmRichAlignment::CalculateMeanRingCenterPosition()
{
    Int_t nofRingsInEvent = fRichRings->GetEntries();
    for (Int_t i = 0; i < nofRingsInEvent; i++) {
    	CbmRichRing* ring = static_cast<CbmRichRing*>(fRichRings->At(i));
    	CbmRichRingLight ringL;
    	CbmRichConverter::CopyHitsToRingLight(ring, &ringL);
    	fCopFit->DoFit(&ringL);
    	XmeanCircle += ringL.GetCenterX();
    	YmeanCircle += ringL.GetCenterY();
    	//cout << "X mean circle fit = " << XmeanCircle << " and Y mean circle fit = " << YmeanCircle << endl;
    	fHM->H2("fHMeanCirclePosition")->Fill(ringL.GetCenterX(), ringL.GetCenterY());

    	fTauFit->DoFit(&ringL);
    	XmeanEllipse += ringL.GetCenterX();
    	YmeanEllipse += ringL.GetCenterY();
    	//cout << "X mean ellipse fit = " << XmeanEllipse << " and Y mean ellipse fit = " << YmeanEllipse << endl;
    	//cout << "counter: " << counter << endl;
    	counter++;
    }
}

void CbmRichAlignment::CalculateAnglesAndDrawDistrib()
{
	Double_t trackX, trackY;
    GetTrackPosition(&trackX, &trackY);

    Int_t nofRingsInEvent = fRichRings->GetEntries();
    Float_t DistCenters, Theta_Ch, Theta_0, Angles_0;
    Float_t Pi = 3.14159265;
    Float_t TwoPi = 2.*3.14159265;
    fPhi.resize(kMAX_NOF_HITS);

    // ------------------------- Loop to get ring center coordinates and photon hit coordinates per ring and per event ------------------------- //
    if (nofRingsInEvent >= 1) {
    	cout << "Number of Rings in event: " << nofRingsInEvent << endl;
    	//sleep(2);
    	for (Int_t iR = 0; iR < nofRingsInEvent; iR++) {
    		// ----- Convert Ring to Ring Light ----- //
    		CbmRichRing* ring = static_cast<CbmRichRing*>(fRichRings->At(iR));
    		if (!IsRingMcElectron(iR)) continue;
    		CbmRichRingLight ringL;
    		CbmRichConverter::CopyHitsToRingLight(ring, &ringL);
    		fCopFit->DoFit(&ringL);
    		// ----- Distance between mean center and fitted center calculation ----- //
    		DistCenters = sqrt(TMath::Power(ringL.GetCenterX() - trackX, 2) + TMath::Power(ringL.GetCenterY() - trackY, 2));
    		fHM->H1("fHCenterDistance")->Fill(DistCenters);
    		// ----- Declaration of new variables ----- //
    		Int_t NofHits = ringL.GetNofHits();
    		Float_t xRing = ringL.GetCenterX();
    		Float_t yRing = ringL.GetCenterY();

    		for (Int_t iH = 0; iH < NofHits; iH++) {
    			// ----- Phi angle calculation ----- //
    			Int_t HitIndex = ring->GetHit(iH);
    			CbmRichHit* hit = static_cast<CbmRichHit*>(fRichHits->At(HitIndex));
    			Float_t xHit = hit->GetX();
    			Float_t yHit = hit->GetY();
    			Angles_0 = TMath::ATan2((hit->GetX() - ringL.GetCenterX()), (ringL.GetCenterY() - hit->GetY())); //* TMath::RadToDeg();
    			//cout << "Angles_0 = " << Angles_0[iH] << endl;

    			if (xRing - xHit == 0 || yRing - yHit == 0) continue;
    			fPhi[iH] = TMath::ATan2(yHit - yRing, xHit - xRing);
    			fHM->H1("fHPhi")->Fill(fPhi[iH]);

    			// ----- Theta_Ch and Theta_0 calculations ----- //
    			Theta_Ch = sqrt(TMath::Power(trackX - hit->GetX(), 2) + TMath::Power(trackY - hit->GetY(), 2));
    			Theta_0 = sqrt(TMath::Power(ringL.GetCenterX() - hit->GetX(), 2) + TMath::Power(ringL.GetCenterY() - hit->GetY(), 2));
    			//cout << "Theta_0 = " << Theta_0 << endl;
    			fHM->H1("fHThetaDiff")->Fill(Theta_Ch - Theta_0);

    			// ----- Filling of final histograms ----- //
    			fHM->H2("fHCherenkovHitsDistribTheta0")->Fill(Angles_0, Theta_0);
    			fHM->H2("fHCherenkovHitsDistribThetaCh")->Fill(fPhi[iH], Theta_Ch);
    			fHM->H2("fHCherenkovHitsDistribReduced")->Fill(fPhi[iH], (Theta_Ch - Theta_0));
    		}
    		//cout << endl;
    	}
    }
}

void CbmRichAlignment::DrawHist()
{
    if(!fIsMeanPosition) {
    	TCanvas* c1 = new TCanvas(fRunTitle + "_Main_Analysis_" + fAxisRotTitle, fRunTitle + "_Main_Analysis_" + fAxisRotTitle, 1500, 600);
    	c1->Divide(3, 2);
    	c1->cd(1);
    	/*c1->SetGridx(1);
    	c1->SetGridy(1);
		fHM->H1("fHCenterDistance")->GetXaxis()->SetLabelSize(0.03);
		fHM->H1("fHCenterDistance")->GetXaxis()->SetTitleSize(0.03);
		fHM->H1("fHCenterDistance")->GetXaxis()->CenterTitle();
		fHM->H1("fHCenterDistance")->GetXaxis()->SetNdivisions(612,kTRUE);
		fHM->H1("fHCenterDistance")->GetYaxis()->CenterTitle();
		fHM->H1("fHCenterDistance")->GetYaxis()->SetTitleSize(0.03);
    	fHM->H1("fHCenterDistance")->GetYaxis()->SetTitleOffset(1.);
		fHM->H1("fHCenterDistance")->Draw();*/
    	DrawH1(fHM->H1("fHCenterDistance"));
    	c1->cd(2);
    	DrawH1(fHM->H1("fHThetaDiff"));
    	c1->cd(3);
    	DrawH2(fHM->H2("fHCherenkovHitsDistribTheta0"));
        c1->cd(4);
        DrawH1(fHM->H1("fHPhi"));
        c1->cd(5);
        DrawH2(fHM->H2("fHCherenkovHitsDistribThetaCh"));
        c1->cd(6);
        DrawH2(fHM->H2("fHCherenkovHitsDistribReduced"));
        Cbm::SaveCanvasAsImage(c1, string(fOutputDir.Data()), "png");
    }

    /*if(!fIsMeanPosition) {
	TCanvas* c2 = new TCanvas(fRunTitle + "_Phi_New_" + fAxisRotTitle, fRunTitle + "_Phi_New_" + fAxisRotTitle, 1100, 600);
	c2->Divide(2,2);
	c2->cd(1);
	DrawH1(fHM->H1("fHPhi_New"));
	c2->cd(2);
    DrawH2(fHM->H2("fHCherenkovHitsDistribThetaCh_PhiNew"));
	c2->cd(3);
	DrawH2(fHM->H2("fHCherenkovHitsDistribReduced_PhiNew"));
	Cbm::SaveCanvasAsImage(c2, string(fOutputDir.Data()), "png");
    }*/

    if(fIsMeanPosition) {
    	TCanvas* c2 = new TCanvas(fRunTitle + "_Mean_Circle_Position", fRunTitle + "_Mean_Circle_Position", 600, 600);
    	DrawH2(fHM->H2("fHMeanCirclePosition"));
    	Cbm::SaveCanvasAsImage(c2, string(fOutputDir.Data()), "png");
    }
}

std::vector<Float_t> CbmRichAlignment::DrawFit()
{
	std::vector<Float_t> param_vector;
	param_vector.reserve(5);
	if(!fIsMeanPosition) {
    	TCanvas* c3 = new TCanvas(fRunTitle + "_Fit_Slices_" + fAxisRotTitle, fRunTitle + "_Fit_Slices_" + fAxisRotTitle, 1100, 600);
    	c3->SetFillColor(42);
    	c3->Divide(4,2);
    	gPad->SetTopMargin(0.1);
    	gPad->SetFillColor(33);
    	c3->cd(1);
    	TH2D* CloneArr = (TH2D*)fHM->H2("fHCherenkovHitsDistribReduced")->Clone();
    	CloneArr->GetXaxis()->SetLabelSize(0.03);
    	CloneArr->GetXaxis()->SetTitleSize(0.03);
    	CloneArr->GetXaxis()->CenterTitle();
        CloneArr->GetXaxis()->SetNdivisions(612,kTRUE);
        CloneArr->GetYaxis()->SetLabelSize(0.03);
        CloneArr->GetYaxis()->SetTitleSize(0.03);
        CloneArr->GetYaxis()->SetNdivisions(612, kTRUE);
        CloneArr->GetYaxis()->CenterTitle();
        //CloneArr->GetYaxis()->SetRangeUser(-2.5,2.5);
        CloneArr->GetZaxis()->SetLabelSize(0.03);
        CloneArr->GetZaxis()->SetTitleSize(0.03);
        CloneArr->GetYaxis()->SetTitleOffset(1.0);
        CloneArr->Draw("colz");
        //Double_t ymax = CloneArr->GetYaxis()->GetXmax();
        //Double_t ymin = CloneArr->GetYaxis()->GetXmin();
        //TF1 *fgauss = TF1 fgauss("gauss", "[0]*exp(-0.5*((x-[1])/[2])**2)", 0, 100);

        // ------------------------------ APPLY THRESHOLD TO 2D-HISTO ------------------------------ //
        TH2D* CloneArr_2 = (TH2D*)fHM->H2("fHCherenkovHitsDistribReduced")->Clone();
        Int_t thresh = 5;
        for (Int_t y_bin=1; y_bin<=500; y_bin++) {
        	for (Int_t x_bin=1; x_bin<=200; x_bin++) {
        		/*if (CloneArr_2->GetBinContent(x_bin, y_bin)!=0) {
		    	cout << "Bin Content: " << CloneArr_2->GetBinContent(x_bin, y_bin) << endl;
		    	sleep(1);
				}
				else;*/
        		if (CloneArr_2->GetBinContent(x_bin, y_bin) < thresh) {
        			CloneArr_2->SetBinContent(x_bin, y_bin, 0);
        		}
        	}
        }
        c3->cd(2);
        CloneArr_2->GetXaxis()->SetLabelSize(0.03);
        CloneArr_2->GetXaxis()->SetTitleSize(0.03);
        CloneArr_2->GetXaxis()->CenterTitle();
        CloneArr_2->GetXaxis()->SetNdivisions(612,kTRUE);
        CloneArr_2->GetYaxis()->SetLabelSize(0.03);
        CloneArr_2->GetYaxis()->SetTitleSize(0.03);
        CloneArr_2->GetYaxis()->SetNdivisions(612, kTRUE);
        CloneArr_2->GetYaxis()->CenterTitle();
        //CloneArr_2->GetYaxis()->SetRangeUser(-2.5,2.5);
        CloneArr_2->GetZaxis()->SetLabelSize(0.03);
        CloneArr_2->GetZaxis()->SetTitleSize(0.03);
        CloneArr_2->GetYaxis()->SetTitleOffset(1.0);
        CloneArr_2->Draw("colz");

        // -------------------- FIT SLICES AND FIT THE MEAN OF THE RESULT TO A SIN FUNCTION -------------------- //
        CloneArr_2->FitSlicesY(0,0,-1,1);
        c3->cd(3);
        TH1D *histo_0 = (TH1D*)gDirectory->Get("fHCherenkovHitsDistribReduced_0");
        histo_0->Draw();
        c3->cd(4);
        TH1D *histo_1 = (TH1D*)gDirectory->Get("fHCherenkovHitsDistribReduced_1");
        //histo_1->GetYaxis()->SetRangeUser(-2.5, 2.5);
        histo_1->Draw();
        c3->cd(5);
        TH1D* histo_2 = (TH1D*)gDirectory->Get("fHCherenkovHitsDistribReduced_2");
        histo_2->Draw();
        c3->cd(6);
        TH1D* histo_chi2 = (TH1D*)gDirectory->Get("fHCherenkovHitsDistribReduced_chi2");
        histo_chi2->Draw();

        c3->cd(7);
        //Float_t Pi = 3.14159265;
        TF1 *f1 = new TF1("f1", "[2]+[0]*cos(x)+[1]*sin(x)", -3.5, 3.5);
        f1->SetParameters(0,0,0);
        f1->SetParNames("Delta_phi", "Delta_lambda", "Offset");
        histo_1->Fit("f1","","");
        TF1 *fit = histo_1->GetFunction("f1");
        Float_t p1 = fit->GetParameter("Delta_phi");
        Float_t p2 = fit->GetParameter("Delta_lambda");
        Float_t p3 = fit->GetParameter("Offset");
        Float_t chi2 = fit->GetChisquare();
        //cout << setprecision(6) << "Delta_phi = " << fit->GetParameter(0) << " and delta_lambda = " << fit->GetParameter(1) << endl;
        //cout << "Delta_phi error = " << fit->GetParError(0) << " and delta_lambda error = " << fit->GetParError(1) << endl;
        //cout << endl << "Chi2: " << chi2 << endl;

        param_vector.push_back(fit->GetParameter("Delta_phi"));
        param_vector.push_back(fit->GetParameter("Delta_lambda"));
        param_vector.push_back(fit->GetParameter("Offset"));
        param_vector.push_back(fit->GetChisquare());
        //cout << "Vectors: Delta_phi = " << param_vector[0] << ", Delta_lambda = " << param_vector[1] << ", Offset = " << param_vector[2] << endl;

        f1->SetParameters(fit->GetParameter(0), fit->GetParameter(1));
        char leg[128];
        f1->SetLineColor(2);
        f1->Draw();

        // ------------------------------ CALCULATION OF MISALIGNMENT ANGLE ------------------------------ //
        Float_t Focal_length = 150;
        Float_t q = TMath::ATan(fit->GetParameter(0)/fit->GetParameter(1));
        //cout << endl << "fit_1 = " << fit->GetParameter(0) << " and fit_2 = " << fit->GetParameter(1) << endl;
        //cout << "q = " << q << endl;
        Float_t A = fit->GetParameter(1)/TMath::Cos(q);
        //cout << "Parameter a = " << A << endl;
        Float_t Alpha = TMath::ATan(A/1.5)*0.5*TMath::Power(10,3);                                  // *0.5, because a mirror rotation of alpha implies a rotation in the particle trajectory of 2*alpha ; 1.5 meters = Focal length = Radius_of_curvature/2
        //cout << setprecision(6) << "Total angle of misalignment alpha = " << Alpha << endl;       // setprecision(#) gives the number of digits in the cout.
        Float_t mis_x = TMath::ATan(fit->GetParameter(0)/Focal_length)*0.5*TMath::Power(10,3);
        Float_t mis_y = TMath::ATan(fit->GetParameter(1)/Focal_length)*0.5*TMath::Power(10,3);

        TLegend* LEG= new TLegend(0.30,0.7,0.72,0.85); // Set legend position
        LEG->SetBorderSize(1);
        LEG->SetFillColor(0);
        LEG->SetMargin(0.2);
        LEG->SetTextSize(0.03);
        sprintf(leg, "Fitted sinusoid");
        LEG->AddEntry(f1, leg, "l");
        sprintf(leg, "Misalign in X = %f", mis_x);
        LEG->AddEntry("", leg, "l");
        sprintf(leg, "Misalign in Y = %f", mis_y);
        LEG->AddEntry("", leg, "l");
        sprintf(leg, "Offset = %f", fit->GetParameter(2));
        LEG->AddEntry("", leg, "l");
        LEG->Draw();

        Cbm::SaveCanvasAsImage(c3, string(fOutputDir.Data()), "png");

        // ------------------------------ APPLY SECOND FIT USING LOG-LIKELIHOOD METHOD ------------------------------ //
        TCanvas* c4 = new TCanvas(fRunTitle + "_Second_Fit_" + fAxisRotTitle, fRunTitle + "_Second_Fit_" + fAxisRotTitle, 600, 600);
        c4->SetFillColor(42);
        gPad->SetTopMargin(0.1);
        gPad->SetFillColor(33);
        f1->SetParameters(fit->GetParameter(0), fit->GetParameter(1), fit->GetParameter(2));
        histo_1->Fit("f1","L","");
        TF1 *fit2 = histo_1->GetFunction("f1");
        f1->SetParameters(fit2->GetParameter(0), fit2->GetParameter(1), fit2->GetParameter(2));
        f1->Draw();

        Float_t q_2 = TMath::ATan(fit2->GetParameter(0)/fit2->GetParameter(1));
        //cout << endl << "fit2_1 = " << fit2->GetParameter(0) << " and fit2_2 = " << fit2->GetParameter(1) << endl;
        //cout << "q_2 = " << q_2 << endl;
        Float_t A_2 = fit2->GetParameter(1)/TMath::Cos(q_2);
        //cout << "Parameter a_2 = " << A_2 << endl;
        Float_t Alpha_2 = TMath::ATan(A_2/1.5)*0.5*TMath::Power(10,3);
        //cout << setprecision(6) << "Total angle of misalignment alpha_2 = " << Alpha_2 << endl;
        Float_t mis_x_2 = TMath::ATan(fit2->GetParameter(0)/Focal_length)*0.5*TMath::Power(10,3);
        Float_t mis_y_2 = TMath::ATan(fit2->GetParameter(1)/Focal_length)*0.5*TMath::Power(10,3);

        TLegend* LEG2= new TLegend(0.31,0.7,0.72,0.85); // Set legend position
        LEG2->SetBorderSize(1);
        LEG2->SetFillColor(0);
        LEG2->SetMargin(0.2);
        LEG2->SetTextSize(0.03);
        sprintf(leg, "Fitted sinusoid");
        LEG2->AddEntry(f1, leg, "l");
        sprintf(leg, "Misalign in X = %f", mis_x_2);
        LEG2->AddEntry("", leg, "l");
        sprintf(leg, "Misalign in Y = %f", mis_y_2);
        LEG2->AddEntry("", leg, "l");
        sprintf(leg, "Offset = %f", fit2->GetParameter(2));
        LEG2->AddEntry("", leg, "l");
        LEG2->Draw();

        Cbm::SaveCanvasAsImage(c4, string(fOutputDir.Data()), "png");

        //param_vector.push_back(1);
        //param_vector.push_back(1);
	}
	return param_vector;
}

void CbmRichAlignment::DrawHistFromFile(TString fileName)
{
    fHM = new CbmHistManager();
    TFile* file = new TFile(fileName, "READ");
    fHM->ReadFromFile(file);
    DrawHist();
}

void CbmRichAlignment::WriteToFile(TString fileName, Double_t Xposition, Double_t Yposition)
{
    ofstream myfile;
    myfile.open(fileName, std::ofstream::out | std::ofstream::app);
    myfile << "X mean center: " << Xposition << " and Y mean center: " << Yposition << endl;
    myfile.close();
}

void CbmRichAlignment::Finish()
{
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------ //
    // ---------------------------------------- Determination of the mean ring center position from the reference data set ---------------------------------------- //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------ //

    if(fIsMeanPosition) {
    	XmeanCircle = XmeanCircle/counter;
    	YmeanCircle = YmeanCircle/counter;
    	cout << endl;
    	cout << "X mean circle fit position = " << XmeanCircle << " and Y mean circle fit position = " << YmeanCircle << endl;
    	TString OutputFile = fOutputDir + "CircleCenter.txt";
    	WriteToFile(OutputFile, XmeanCircle, YmeanCircle);

    	XmeanEllipse = XmeanEllipse/counter;
    	YmeanEllipse = YmeanEllipse/counter;
    	cout << endl;
    	cout << "X mean ellipse fit position = " << XmeanEllipse << " and Y mean ellipse fit position = " << YmeanEllipse << endl;

    	if (fDrawHist) {
    		DrawHist();
    	}
    	fHM->WriteToFile();
    }

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------ //
    // ----------------------------------------------------- Cherenkov angle and azimuthal angle calculations ----------------------------------------------------- //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------ //

    else {
    	if (fDrawHist) {
    		DrawHist();
    		std::vector<Float_t> vector = DrawFit();

    		Float_t Focal_length = 150;
    		Float_t q = TMath::ATan(vector[0]/vector[1]);
    		//cout << endl << "fit_1 = " << fit->GetParameter(0) << " and fit_2 = " << fit->GetParameter(1) << endl;
    		//cout << "q = " << q << endl;
    		Float_t A = vector[1]/TMath::Cos(q);
    		//cout << "Parameter a = " << A << endl;
    		Float_t Alpha = TMath::ATan(A/Focal_length)*0.5*TMath::Power(10,3);                                // *0.5, because a mirror rotation of alpha implies a rotation in the particle trajectory of 2*alpha ; 1.5 meters = Focal length = Radius_of_curvature/2
    		cout << endl << setprecision(6) << "Angle of misalignment alpha [mrad] = " << Alpha << endl;       // setprecision(#) gives the number of digits in the cout.

    		Float_t mis_x = TMath::ATan(vector[0]/Focal_length)*0.5*TMath::Power(10,3);
    		Float_t mis_y = TMath::ATan(vector[1]/Focal_length)*0.5*TMath::Power(10,3);
    		cout << "Misalignment in X [mrad] = " << mis_x << " and misalignment in Y [mrad] = " << mis_y << endl;
    	}
    	//fHM->WriteToFile();
    }

    /*TCanvas *can = new TCanvas("histo", "histo", 600, 600);
    can->Divide(2,1);
    Int_t nbinsx = fHM->H2("fHCherenkovHitsDistribReduced_PhiNew")->GetXaxis()->GetNbins();
    Int_t nbinsy = fHM->H2("fHCherenkovHitsDistribReduced_PhiNew")->GetYaxis()->GetNbins();
    cout << endl << "x bins: " << nbinsx << " and y bins: " << nbinsy << endl;
    TH1D *histo_buffer = new TH1D("Hist","Hist",200,-3.5,3.5);
    TH1D *histo1D = new TH1D("Hist2","Hist2",200,-3.5,3.5);
    for (Int_t i=0; i<nbinsy ; i++) {
    	for (Int_t j=0; j<nbinsx ; j++) {
    		//cout << "BIN = " << fHM->H2("fHCherenkovHitsDistribReduced_PhiNew")->GetBinContent(i, j)  << endl;
    		//if(fHM->H2("fHCherenkovHitsDistribReduced_PhiNew")->GetBinContent(i, j) != 0) {
    		//cout << "STOP" << endl;
    		//sleep(10);
    		//}
    		histo_buffer->SetBinContent(j,fHM->H2("fHCherenkovHitsDistribReduced_PhiNew")->GetBinContent(i,j));
    	}
    	histo_buffer->Draw();
    	sleep(5);
    	histo1D->Fill(histo_buffer->Fit("gaus"));
    }
    can->cd(1);
    histo1D->Draw();*/
}
ClassImp(CbmRichAlignment)
