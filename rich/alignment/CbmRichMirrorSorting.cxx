#include "CbmRichMirrorSorting.h"
#include "CbmRichMirror.h"
#include "FairRootManager.h"
#include "FairLogger.h"

	// ----- PART 1 ----- //
#include "TClonesArray.h"
#include "CbmGlobalTrack.h"
#include "CbmRichRing.h"
#include "CbmMCTrack.h"
#include "CbmRichConverter.h"
#include "CbmRichRingFitterCOP.h"
#include "CbmRichRingFitterEllipseTau.h"
#include "TVector3.h"
#include <vector>

	// ----- PART 2 ----- //
#include "TGeoManager.h"
#include "CbmRichGeoManager.h"
#include "CbmRichPoint.h"
#include "TGeoNavigator.h"
class TGeoNode;
class TGeoMatrix;

CbmRichMirrorSorting::CbmRichMirrorSorting() :
	fEventNb(0),
	fGlobalTracks(NULL),
	fRichRings(NULL),
	fMCTracks(NULL),
	fCopFit(NULL),
	fTauFit(NULL),
	fOutputDir(""),
	fMirrorPoints(NULL),
	fRefPlanePoints(NULL),
	fPmtPoints(NULL),
	fRichProjections(NULL)
{
}

CbmRichMirrorSorting::~CbmRichMirrorSorting() {}

InitStatus CbmRichMirrorSorting::Init()
{
	FairRootManager* manager = FairRootManager::Instance();

	fGlobalTracks = (TClonesArray*) manager->GetObject("GlobalTrack");
	if (NULL == fGlobalTracks) { Fatal("CbmRichMirrorSorting::Init", "No GlobalTrack array!"); }

	fRichRings = (TClonesArray*) manager->GetObject("RichRing");
	if (NULL == fRichRings) { Fatal("CbmRichMirrorSorting::Init", "No RichRing array !"); }

	fMCTracks = (TClonesArray*) manager->GetObject("MCTrack");
	if (NULL == fMCTracks) { Fatal("CbmRichMirrorSorting::Init", "No MCTracks array !"); }

	fMirrorPoints = (TClonesArray*) manager->GetObject("RichMirrorPoint");
	if (NULL == fMirrorPoints) { Fatal("CbmRichMirrorSorting::Init", "No RichMirrorPoints array !"); }

	fRefPlanePoints  = (TClonesArray*) manager->GetObject("RefPlanePoint");
	if (NULL == fRefPlanePoints) { Fatal("CbmRichMirrorSorting::Init", "No RichRefPlanePoint array !"); }

	fPmtPoints = (TClonesArray*) manager->GetObject("RichPoint");
	if (NULL == fPmtPoints) { Fatal("CbmRichMirrorSorting::Init", "No RichPoint array !"); }

	fRichProjections = (TClonesArray*) manager->GetObject("RichProjection");
	if (NULL == fRichProjections) { Fatal("CbmRichMirrorSorting::Init", "No RichProjection array !"); }

	fCopFit = new CbmRichRingFitterCOP();
	fTauFit = new CbmRichRingFitterEllipseTau();
	CbmRichConverter::Init();

	return kSUCCESS;
}

void CbmRichMirrorSorting::Exec(Option_t* Option)
{
	fEventNb++;
	cout << "CbmRichMirrorSorting: Event #" << fEventNb << endl;
	Int_t nofRingsInEvent = fRichRings->GetEntries();
	Int_t nofPmtPoints = fPmtPoints->GetEntries();
	cout << "Nb of rings in evt = " << nofRingsInEvent << endl;
	TVector3 momentum, outPos;
	Double_t constantePMT = 0.;
	vector<Double_t> vect(2,0), ptM(3,0), ptC(3,0), ptCIdeal(3,0), ptR1(3,0), ptR2Center(3,0), ptR2Mirr(3,0), ptPR2(3,0), ptPMirr(3,0), normalPMT(3,0);
	ptC.at(0) = 0., ptC.at(1) = 132.594000, ptC.at(2) = 54.267226;

	GetPmtNormal(nofPmtPoints, normalPMT, constantePMT);
	cout << "Calculated normal vector to PMT plane = {" << normalPMT.at(0) << ", " << normalPMT.at(1) << ", " << normalPMT.at(2) << "} and constante d = " << constantePMT << endl << endl;

	for (Int_t iGlobalTrack = 0; iGlobalTrack < fGlobalTracks->GetEntriesFast(); iGlobalTrack++)
	{
		CbmRichMirror* mirrorObject = new CbmRichMirror();																			// Create CbmRichMirror object, to be later filled.
		// ----- PART 1 ----- //
		// Ring-Track matching + Ring fit + Track momentum:
		CbmGlobalTrack* gTrack  = (CbmGlobalTrack*) fGlobalTracks->At(iGlobalTrack);
		Int_t richInd = gTrack->GetRichRingIndex();
		if (richInd < 0) {
			cout << "CbmRichMirrorSorting::Exec : richInd < 0." << endl;
			continue;
		}
		CbmRichRing* ring = (CbmRichRing*) fRichRings->At(richInd);
		if (ring == NULL) {
			cout << "CbmRichMirrorSorting::Exec : ring = NULL." << endl;
			continue;
		}
		CbmRichRingLight ringL;
		CbmRichConverter::CopyHitsToRingLight(ring, &ringL);
		fCopFit->DoFit(&ringL);
		//fTauFit->DoFit(&ringL);
		cout << "Ring parameters = " << ringL.GetCenterX() << ", " << ringL.GetCenterY() << endl;
		mirrorObject->setFittedRing(ringL.GetCenterX(), ringL.GetCenterY());														// Fill fitted Ring Center coo inside mirrorObject.
		Int_t ringTrackID = ring->GetTrackID();
		CbmMCTrack* track = (CbmMCTrack*) fMCTracks->At(ringTrackID);
		//Int_t ringMotherId = track->GetMotherId();
		track->GetMomentum(momentum);
		mirrorObject->setMomentum(momentum);																						// Fill track momentum inside mirrorObject.

		// ----- PART 2 ----- //
		// Mirror ID via TGeoNavigator + Extrap hit:
		Int_t trackMotherId = track->GetMotherId();
		if (trackMotherId == -1) {
			CbmRichPoint* mirrPoint = (CbmRichPoint*) fMirrorPoints->At(ringTrackID);
			ptM.at(0) = mirrPoint->GetX(), ptM.at(1) = mirrPoint->GetY(), ptM.at(2) = mirrPoint->GetZ();
			TGeoNode* mirrNode = gGeoManager->FindNode(ptM.at(0),ptM.at(1),ptM.at(2));
			cout << "Mirror node name: " << mirrNode->GetName() << endl;
			mirrorObject->setMirrorId(mirrNode->GetName());
			if (mirrNode) {
				TGeoNavigator* navi = gGeoManager->GetCurrentNavigator();
				cout << "Navigator path: " << navi->GetPath() << endl;
				ptCIdeal.at(0) = navi->GetCurrentMatrix()->GetTranslation()[0];
				ptCIdeal.at(1) = navi->GetCurrentMatrix()->GetTranslation()[1];
				ptCIdeal.at(2) = navi->GetCurrentMatrix()->GetTranslation()[2];
				CbmRichPoint *refPlanePoint = (CbmRichPoint*) fRefPlanePoints->At(ringTrackID);
				ptR1.at(0) = refPlanePoint->GetX(), ptR1.at(1) = refPlanePoint->GetY(), ptR1.at(2) = refPlanePoint->GetZ();
				ComputeR2(ptR2Center, ptR2Mirr, ptM, ptC, ptR1, navi, "Uncorrected");
				ComputeP(ptPMirr, ptPR2, normalPMT, ptM, ptR2Mirr, constantePMT);
				TVector3 inPos (ptPMirr.at(0), ptPMirr.at(1), ptPMirr.at(2));
				CbmRichGeoManager::GetInstance().RotatePoint(&inPos, &outPos);
				cout << "New mirror points coordinates = {" << outPos.x() << ", " << outPos.y() << ", " << outPos.z() << "}" << endl;
				mirrorObject->setExtrapHit(outPos.x(), outPos.y());
			}
		}
		else { cout << "Not a mother particle." << endl; }

		//ComputeAngles();

		fMirrorMap[mirrorObject->getMirrorId()].push_back(mirrorObject);
		delete mirrorObject;
	}
}

/*void CbmRichMirrorSorting::ComputeAngles()
{
	cout << "//------------------------------ CbmRichAlignment: Calculate Angles & Draw Distrib ------------------------------//" << endl << endl;

	Double_t trackX=0., trackY=0.;
    GetTrackPosition(trackX, trackY);

    Int_t nofRingsInEvent = fRichRings->GetEntries();
    Float_t DistCenters, Theta_Ch, Theta_0, Angles_0;
    Float_t Pi = 3.14159265;
    Float_t TwoPi = 2.*3.14159265;
    Float_t phi = 0.;
    //fPhi.resize(kMAX_NOF_HITS);

    // ------------------------- Loop to get ring center coordinates and photon hit coordinates per ring and per event ------------------------- //
    if (nofRingsInEvent >= 1) {
    	cout << "Number of Rings in event: " << nofRingsInEvent << endl;
    	//sleep(2);
    	for (Int_t iR = 0; iR < nofRingsInEvent; iR++) {
    		// ----- Convert Ring to Ring Light ----- //
    		CbmRichRing* ring = static_cast<CbmRichRing*>(fRichRings->At(iR));
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
    			CbmRichHit* hit = static_cast<CbmRichHit*>(fPmtPoints->At(HitIndex));
    			Float_t xHit = hit->GetX();
    			Float_t yHit = hit->GetY();
    			Angles_0 = TMath::ATan2((hit->GetX() - ringL.GetCenterX()), (ringL.GetCenterY() - hit->GetY())); //* TMath::RadToDeg();
    			//cout << "Angles_0 = " << Angles_0[iH] << endl;

    			if (xRing - xHit == 0 || yRing - yHit == 0) continue;
    			phi = TMath::ATan2(yHit - yRing, xHit - xRing);
    			fHM->H1("fHPhi")->Fill(phi);

    			// ----- Theta_Ch and Theta_0 calculations ----- //
    			Theta_Ch = sqrt(TMath::Power(trackX - hit->GetX(), 2) + TMath::Power(trackY - hit->GetY(), 2));
    			Theta_0 = sqrt(TMath::Power(ringL.GetCenterX() - hit->GetX(), 2) + TMath::Power(ringL.GetCenterY() - hit->GetY(), 2));
    			//cout << "Theta_0 = " << Theta_0 << endl;
    			fHM->H1("fHThetaDiff")->Fill(Theta_Ch - Theta_0);

    			// ----- Filling of final histograms ----- //
    			fHM->H2("fHCherenkovHitsDistribTheta0")->Fill(Angles_0, Theta_0);
    			fHM->H2("fHCherenkovHitsDistribThetaCh")->Fill(phi, Theta_Ch);
    			fHM->H2("fHCherenkovHitsDistribReduced")->Fill(phi, (Theta_Ch - Theta_0));
    		}
    		//cout << endl;
    	}
    }
}

void CbmRichMirrorSorting::GetTrackPosition(Double_t &x, Double_t &y)
{
	Int_t NofProjections = fRichProjections->GetEntries();
	//cout << "!!! NB PROJECTIONS !!! " << NofProjections << endl;
	for (Int_t iP = 0; iP < NofProjections; iP++) {
		FairTrackParam* pr = (FairTrackParam*) fRichProjections->At(iP);
		if (NULL == pr) {
			x = 0.;
			y = 0.;
			cout << "Error: CbmRichAlignment::GetTrackPosition. No fair track param found." << endl;
    	}
		x = pr->GetX();
		y = pr->GetY();
		//cout << "Center X: " << *x << " and Center y: " << *y << endl;
    }
}
*/
void CbmRichMirrorSorting::GetPmtNormal(Int_t NofPMTPoints, vector<Double_t> &normalPMT, Double_t &normalCste)
{
	//cout << endl << "//------------------------------ CbmRichCorrection: Calculate PMT Normal ------------------------------//" << endl << endl;

	Int_t pmtTrackID, pmtMotherID;
	Double_t buffNormX=0., buffNormY=0., buffNormZ=0., k=0., scalarProd=0.;
	Double_t pmtPt[] = {0., 0., 0.};
	Double_t a[] = {0., 0., 0.}, b[] = {0., 0., 0.}, c[] = {0., 0., 0.};
	CbmMCTrack *track;

/*
 * Selection of three points (A, B, C), which form a plan and from which the calculation of the normal of the plan can be computed.
 * Formula used is: vect(AB) x vect(AC) = normal.
 * Normalize the normal vector obtained and check with three random points from the PMT plane, whether the scalar product is equal to zero.
 */
	for (Int_t iPmt = 0; iPmt < NofPMTPoints; iPmt++) {
		CbmRichPoint *pmtPoint = (CbmRichPoint*) fPmtPoints->At(iPmt);
		pmtTrackID = pmtPoint->GetTrackID();
		track = (CbmMCTrack*) fMCTracks->At(pmtTrackID);
		pmtMotherID = track->GetMotherId();
		a[0] = pmtPoint->GetX(), a[1] = pmtPoint->GetY(), a[2] = pmtPoint->GetZ();
		//cout << "a[0] = " << a[0] << ", a[1] = " << a[1] << " et a[2] = " << a[2] << endl;
		break;
	}
	for (Int_t iPmt = 0; iPmt < NofPMTPoints; iPmt++) {
		CbmRichPoint *pmtPoint = (CbmRichPoint*) fPmtPoints->At(iPmt);
		pmtTrackID = pmtPoint->GetTrackID();
		track = (CbmMCTrack*) fMCTracks->At(pmtTrackID);
		pmtMotherID = track->GetMotherId();
		//cout << "PMT Point coordinates; x = " << pmtPoint->GetX() << ", y = " << pmtPoint->GetY() << " and z = " << pmtPoint->GetZ() << endl;
		if (TMath::Sqrt(TMath::Power(a[0]-pmtPoint->GetX(),2) + TMath::Power(a[1]-pmtPoint->GetY(),2) + TMath::Power(a[2]-pmtPoint->GetZ(),2)) > 7) {
			b[0] = pmtPoint->GetX(), b[1] = pmtPoint->GetY(), b[2] = pmtPoint->GetZ();
			//cout << "b[0] = " << b[0] << ", b[1] = " << b[1] << " et b[2] = " << b[2] << endl;
			break;
		}
	}
	for (Int_t iPmt = 0; iPmt < NofPMTPoints; iPmt++) {
		CbmRichPoint *pmtPoint = (CbmRichPoint*) fPmtPoints->At(iPmt);
		pmtTrackID = pmtPoint->GetTrackID();
		track = (CbmMCTrack*) fMCTracks->At(pmtTrackID);
		pmtMotherID = track->GetMotherId();
		//cout << "PMT Point coordinates; x = " << pmtPoint->GetX() << ", y = " << pmtPoint->GetY() << " and z = " << pmtPoint->GetZ() << endl;
		if (TMath::Sqrt(TMath::Power(a[0]-pmtPoint->GetX(),2) + TMath::Power(a[1]-pmtPoint->GetY(),2) + TMath::Power(a[2]-pmtPoint->GetZ(),2)) > 7 && TMath::Sqrt(TMath::Power(b[0]-pmtPoint->GetX(),2) + TMath::Power(b[1]-pmtPoint->GetY(),2) + TMath::Power(b[2]-pmtPoint->GetZ(),2)) > 7) {
			c[0] = pmtPoint->GetX(), c[1] = pmtPoint->GetY(), c[2] = pmtPoint->GetZ();
			//cout << "c[0] = " << c[0] << ", c[1] = " << c[1] << " et c[2] = " << c[2] << endl;
			break;
		}
	}

	k = (b[0]-a[0])/(c[0]-a[0]);
	if ( (b[1]-a[1])-(k*(c[1]-a[1])) == 0 || (b[2]-a[2])-(k*(c[2]-a[2])) == 0 ) { cout << "Error in normal calculation, vect_AB and vect_AC are collinear." << endl; }
	else {
		buffNormX = (b[1]-a[1])*(c[2]-a[2]) - (b[2]-a[2])*(c[1]-a[1]);
		buffNormY = (b[2]-a[2])*(c[0]-a[0]) - (b[0]-a[0])*(c[2]-a[2]);
		buffNormZ = (b[0]-a[0])*(c[1]-a[1]) - (b[1]-a[1])*(c[0]-a[0]);
		normalPMT.at(0) = buffNormX/TMath::Sqrt(TMath::Power(buffNormX,2)+TMath::Power(buffNormY,2)+TMath::Power(buffNormZ,2));
		normalPMT.at(1) = buffNormY/TMath::Sqrt(TMath::Power(buffNormX,2)+TMath::Power(buffNormY,2)+TMath::Power(buffNormZ,2));
		normalPMT.at(2) = buffNormZ/TMath::Sqrt(TMath::Power(buffNormX,2)+TMath::Power(buffNormY,2)+TMath::Power(buffNormZ,2));
	}

	CbmRichPoint *pmtPoint1 = (CbmRichPoint*) fPmtPoints->At(20);
	scalarProd = normalPMT.at(0)*(pmtPoint1->GetX()-a[0]) + normalPMT.at(1)*(pmtPoint1->GetY()-a[1]) + normalPMT.at(2)*(pmtPoint1->GetZ()-a[2]);
	//cout << "1st scalar product between vectAM and normale = " << scalarProd << endl;
	// To determine the constant term of the plane equation, inject the coordinates of a pmt point, which should solve it: a*x+b*y+c*z+d=0.
	normalCste = -1*(normalPMT.at(0)*pmtPoint1->GetX() + normalPMT.at(1)*pmtPoint1->GetY() + normalPMT.at(2)*pmtPoint1->GetZ());
	CbmRichPoint *pmtPoint2 = (CbmRichPoint*) fPmtPoints->At(15);
	scalarProd = normalPMT.at(0)*(pmtPoint2->GetX()-a[0]) + normalPMT.at(1)*(pmtPoint2->GetY()-a[1]) + normalPMT.at(2)*(pmtPoint2->GetZ()-a[2]);
	//cout << "2nd scalar product between vectAM and normale = " << scalarProd << endl;
	CbmRichPoint *pmtPoint3 = (CbmRichPoint*) fPmtPoints->At(25);
	scalarProd = normalPMT.at(0)*(pmtPoint3->GetX()-a[0]) + normalPMT.at(1)*(pmtPoint3->GetY()-a[1]) + normalPMT.at(2)*(pmtPoint3->GetZ()-a[2]);
	//cout << "3nd scalar product between vectAM and normale = " << scalarProd << endl;
}

void CbmRichMirrorSorting::ComputeR2(vector<Double_t> &ptR2Center, vector<Double_t> &ptR2Mirr, vector<Double_t> ptM, vector<Double_t> ptC, vector<Double_t> ptR1, TGeoNavigator* navi, TString s)
{
	cout << endl << "//------------------------------ CbmRichCorrection: ComputeR2 ------------------------------//" << endl << endl;

	vector<Double_t> normalMirr(3), ptCNew(3), ptTileCenter(3);
	Double_t t1=0., t2=0., t3=0.;

	if (s == "Corrected") {
		// Use the correction information from text file, to the tile sphere center:
		// Reading misalignment information from correction_param.txt text file.
		vector<Double_t> outputFit(4);
		ifstream corr_file;
		//TString str = fOutputDir + "correction_param_" + fNumbAxis + fTile + ".txt";
		TString str = fOutputDir + "correction_param.txt";
		corr_file.open(str);
		if (corr_file.is_open())
		{
			for (Int_t i=0; i<4; i++) {corr_file >> outputFit.at(i);}
			corr_file.close();
		}
		else {
			cout << "Error in CbmRichCorrection: unable to open parameter file!" << endl;
			cout << "Parameter file path = " << str << endl << endl;
			sleep(5);
		}
		cout << "Misalignment parameters read from file = [" << outputFit.at(0) << " ; " << outputFit.at(1) << " ; " << outputFit.at(2) << " ; " << outputFit.at(3) << "]" << endl;

		//ptCNew.at(0) = TMath::Abs(ptC.at(0) - TMath::Abs(outputFit.at(3)));
		//ptCNew.at(1) = TMath::Abs(ptC.at(1) - TMath::Abs(outputFit.at(2)));
		ptCNew.at(0) = ptC.at(0) + outputFit.at(3);
		ptCNew.at(1) = ptC.at(1) + outputFit.at(2);
		ptCNew.at(2) = ptC.at(2);
		ptTileCenter.at(0) = navi->GetMotherMatrix()->GetTranslation()[0];
		ptTileCenter.at(1) = navi->GetMotherMatrix()->GetTranslation()[1];
		ptTileCenter.at(2) = navi->GetMotherMatrix()->GetTranslation()[2];
		cout << "Mirror tile center coordinates = {" << ptTileCenter.at(0) << ", " << ptTileCenter.at(1) << ", " << ptTileCenter.at(2) << "}" << endl;
		Double_t x=0., y=0., z=0., dist=0., dist2=0., z2=0.;
		x = TMath::Power(ptCNew.at(0)-ptTileCenter.at(0), 2);
		y = TMath::Power(ptCNew.at(1)-ptTileCenter.at(1), 2);
		z = TMath::Power(ptCNew.at(2)-ptTileCenter.at(2), 2);
		dist = TMath::Sqrt(x + y + z);
		z2 = ptTileCenter.at(2) - TMath::Sqrt(TMath::Power(300,2) - x - y) - ptCNew.at(2);
		cout << "{x, y, z} = {" << x << ", " << y << ", " << z << "}, dist = " << dist << " and z2 = " << z2 << endl;
		dist2 = TMath::Sqrt(x + y + TMath::Power(z2-ptTileCenter.at(2), 2));
		cout << "dist2 = " << dist2 << endl;
		ptCNew.at(2) += z2;
		cout << "Sphere center coordinates of the rotated mirror tile, after correction, = {" << ptCNew.at(0) << ", " << ptCNew.at(1) << ", " << ptCNew.at(2) << "}" << endl;
	}
	else if (s == "Uncorrected") {
		// Keep the same tile sphere center, with no correction information.
		ptCNew = ptC;
		cout << "Sphere center coordinates of the rotated mirror tile, without correction = {" << ptCNew.at(0) << ", " << ptCNew.at(1) << ", " << ptCNew.at(2) << "}" << endl;
	}
	else {
		cout << "No input given in function ComputeR2! Uncorrected parameters for the sphere center of the tile will be used!" << endl;
		ptCNew = ptC;
		cout << "Sphere center coordinates of the rotated mirror tile, without correction = {" << ptCNew.at(0) << ", " << ptCNew.at(1) << ", " << ptCNew.at(2) << "}" << endl;
	}

	normalMirr.at(0) = (ptCNew.at(0) - ptM.at(0))/TMath::Sqrt(TMath::Power(ptCNew.at(0) - ptM.at(0),2)+TMath::Power(ptCNew.at(1) - ptM.at(1),2)+TMath::Power(ptCNew.at(2) - ptM.at(2),2));
	normalMirr.at(1) = (ptCNew.at(1) - ptM.at(1))/TMath::Sqrt(TMath::Power(ptCNew.at(0) - ptM.at(0),2)+TMath::Power(ptCNew.at(1) - ptM.at(1),2)+TMath::Power(ptCNew.at(2) - ptM.at(2),2));
	normalMirr.at(2) = (ptCNew.at(2) - ptM.at(2))/TMath::Sqrt(TMath::Power(ptCNew.at(0) - ptM.at(0),2)+TMath::Power(ptCNew.at(1) - ptM.at(1),2)+TMath::Power(ptCNew.at(2) - ptM.at(2),2));
	//cout << "Calculated and normalized normal of mirror tile = {" << normalMirr.at(0) << ", " << normalMirr.at(1) << ", " << normalMirr.at(2) << "}" << endl;

	t1 = ((ptR1.at(0)-ptM.at(0))*(ptCNew.at(0)-ptM.at(0)) + (ptR1.at(1)-ptM.at(1))*(ptCNew.at(1)-ptM.at(1)) + (ptR1.at(2)-ptM.at(2))*(ptCNew.at(2)-ptM.at(2)))/(TMath::Power(ptCNew.at(0)-ptM.at(0),2) + TMath::Power(ptCNew.at(1)-ptM.at(1),2) + TMath::Power(ptCNew.at(2)-ptM.at(2),2));
	ptR2Center.at(0) = 2*(ptM.at(0)+t1*(ptCNew.at(0)-ptM.at(0)))-ptR1.at(0);
	ptR2Center.at(1) = 2*(ptM.at(1)+t1*(ptCNew.at(1)-ptM.at(1)))-ptR1.at(1);
	ptR2Center.at(2) = 2*(ptM.at(2)+t1*(ptCNew.at(2)-ptM.at(2)))-ptR1.at(2);
	t2 = ((ptR1.at(0)-ptCNew.at(0))*(ptCNew.at(0)-ptM.at(0)) + (ptR1.at(1)-ptCNew.at(1))*(ptCNew.at(1)-ptM.at(1)) + (ptR1.at(2)-ptCNew.at(2))*(ptCNew.at(2)-ptM.at(2)))/(TMath::Power(ptCNew.at(0)-ptM.at(0),2) + TMath::Power(ptCNew.at(1)-ptM.at(1),2) + TMath::Power(ptCNew.at(2)-ptM.at(2),2));
	ptR2Mirr.at(0) = 2*(ptCNew.at(0)+t2*(ptCNew.at(0)-ptM.at(0)))-ptR1.at(0);
	ptR2Mirr.at(1) = 2*(ptCNew.at(1)+t2*(ptCNew.at(1)-ptM.at(1)))-ptR1.at(1);
	ptR2Mirr.at(2) = 2*(ptCNew.at(2)+t2*(ptCNew.at(2)-ptM.at(2)))-ptR1.at(2);
	/*//SAME AS calculation of t2 above
	t3 = ((ptR1.at(0)-ptCNew.at(0))*(ptCNew.at(0)-ptM.at(0)) + (ptR1.at(1)-ptCNew.at(1))*(ptCNew.at(1)-ptM.at(1)) + (ptR1.at(2)-ptCNew.at(2))*(ptCNew.at(2)-ptM.at(2)))/TMath::Sqrt(TMath::Power(ptCNew.at(0) - ptM.at(0),2)+TMath::Power(ptCNew.at(1) - ptM.at(1),2)+TMath::Power(ptCNew.at(2) - ptM.at(2),2));
	reflectedPtCooVectSphereUnity[0] = 2*(ptCNew.at(0)+t3*(normalMirr.at(0)))-ptR1.at(0);
	reflectedPtCooVectSphereUnity[1] = 2*(ptCNew.at(1)+t3*(normalMirr.at(1)))-ptR1.at(1);
	reflectedPtCooVectSphereUnity[2] = 2*(ptCNew.at(2)+t3*(normalMirr.at(2)))-ptR1.at(2);*/
	cout << "Coordinates of point R2 on reflective plane after reflection on the mirror tile:" << endl;
	//cout << "* using mirror point M to define \U00000394: {" << ptR2Center.at(0) << ", " << ptR2Center.at(1) << ", " << ptR2Center.at(2) << "}" << endl;
	cout << "* using sphere center C to define \U00000394: {" << ptR2Mirr.at(0) << ", " << ptR2Mirr.at(1) << ", " << ptR2Mirr.at(2) << "}" << endl;
	//cout << "Ref Pt Coo using unity Mirror-Sphere vector & sphere pt = {" << reflectedPtCooVectSphereUnity[0] << ", " << reflectedPtCooVectSphereUnity[1] << ", " << reflectedPtCooVectSphereUnity[2] << "}" << endl << endl;
	//cout << "NofPMTPoints = " << NofPMTPoints << endl;
}

void CbmRichMirrorSorting::ComputeP(vector<Double_t> &ptPMirr, vector<Double_t> &ptPR2, vector<Double_t> normalPMT, vector<Double_t> ptM, vector<Double_t> ptR2Mirr, Double_t constantePMT)
{
	cout << endl << "//------------------------------ CbmRichCorrection: ComputeP ------------------------------//" << endl << endl;

	Double_t k1=0., k2=0., checkCalc1=0., checkCalc2=0.;

	k1 = -1*((normalPMT.at(0)*ptM.at(0) + normalPMT.at(1)*ptM.at(1) + normalPMT.at(2)*ptM.at(2) + constantePMT)/(normalPMT.at(0)*(ptR2Mirr.at(0)-ptM.at(0)) + normalPMT.at(1)*(ptR2Mirr.at(1)-ptM.at(1)) + normalPMT.at(2)*(ptR2Mirr.at(2)-ptM.at(2))));
	ptPMirr.at(0) = ptM.at(0) + k1*(ptR2Mirr.at(0) - ptM.at(0));
	ptPMirr.at(1) = ptM.at(1) + k1*(ptR2Mirr.at(1) - ptM.at(1));
	ptPMirr.at(2) = ptM.at(2) + k1*(ptR2Mirr.at(2) - ptM.at(2));
	k2 = -1*((normalPMT.at(0)*ptR2Mirr.at(0) + normalPMT.at(1)*ptR2Mirr.at(1) + normalPMT.at(2)*ptR2Mirr.at(2) + constantePMT)/(normalPMT.at(0)*(ptR2Mirr.at(0)-ptM.at(0)) + normalPMT.at(1)*(ptR2Mirr.at(1)-ptM.at(1)) + normalPMT.at(2)*(ptR2Mirr.at(2)-ptM.at(2))));
	ptPR2.at(0) = ptR2Mirr.at(0) + k2*(ptR2Mirr.at(0) - ptM.at(0));
	ptPR2.at(1) = ptR2Mirr.at(1) + k2*(ptR2Mirr.at(1) - ptM.at(1));
	ptPR2.at(2) = ptR2Mirr.at(2) + k2*(ptR2Mirr.at(2) - ptM.at(2));
	cout << "Coordinates of point P on PMT plane, after reflection on the mirror tile and extrapolation to the PMT plane:" << endl;
	cout << "* using mirror point M to define \U0001D49F ': {" << ptPMirr.at(0) << ", " << ptPMirr.at(1) << ", " << ptPMirr.at(2) << "}" << endl;
	//cout << "* using reflected point R2 to define \U0001D49F ': {" << ptPR2.at(0) << ", " << ptPR2.at(1) << ", " << ptPR2.at(2) << "}" << endl;
	checkCalc1 = ptPMirr.at(0)*normalPMT.at(0) + ptPMirr.at(1)*normalPMT.at(1) + ptPMirr.at(2)*normalPMT.at(2) + constantePMT;
	cout << "Check whether extrapolated track point on PMT plane verifies its equation (value should be 0.):" << endl;
	cout << "* using mirror point M, checkCalc = " << checkCalc1 << endl;
	checkCalc2 = ptPR2.at(0)*normalPMT.at(0) + ptPR2.at(1)*normalPMT.at(1) + ptPR2.at(2)*normalPMT.at(2) + constantePMT;
	//cout << "* using reflected point R2, checkCalc = " << checkCalc2 << endl;
}

void CbmRichMirrorSorting::Finish()
{
	for (std::map<string, vector<CbmRichMirror*>>::iterator it=fMirrorMap.begin(); it!=fMirrorMap.end(); ++it) {
		cout << "Mirror objects:" << endl;
		// to get mirror Id:
		string curMirrorId =  it->first;
		// get first CbmRichMirrors
		if (it->first == "Hello") {
			for (int i = 0; i < it->second.size(); i++) {
				CbmRichMirror* mirror = it->second[i];
				cout << "mirror ID: " << mirror->getMirrorId() << endl;
			}
		}
	}
}

ClassImp(CbmRichMirrorSorting)
