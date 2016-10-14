#include "CbmRichMirrorSortingCorrection.h"
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
#include "TCanvas.h"
#include "TF1.h"
#include "TLegend.h"
#include "TH2D.h"
#include "CbmUtils.h"

	// ----- PART 2 ----- //
#include "TGeoManager.h"
#include "CbmRichGeoManager.h"
#include "CbmRichPoint.h"
#include "TGeoNavigator.h"
class TGeoNode;
class TGeoMatrix;
#include "CbmTrackMatchNew.h"
#include "TGeoNavigator.h"
#include "TStyle.h"
#include "string.h"
#include <fstream>
#include <iostream>
using namespace std;

CbmRichMirrorSortingCorrection::CbmRichMirrorSortingCorrection() :
    	    fEventNb(0),
			fHM(NULL),
			fHM2(NULL),
			fDiffHistoMap(),
			fCopFit(NULL),
			fTauFit(NULL),
    		fGlobalTracks(NULL),
    	    fRichRings(NULL),
    	    fMCTracks(NULL),
			fMirrorPoints(NULL),
			fRefPlanePoints(NULL),
			fPmtPoints(NULL),
			fRichProjections(NULL),
			fTrackParams(NULL),
			fRichRingMatches(NULL),
			fStsTrackMatches(NULL),
			fTrackCenterDistanceIdeal(0),
			fTrackCenterDistanceCorrected(0),
			fTrackCenterDistanceUncorrected(0)
{
}

CbmRichMirrorSortingCorrection::~CbmRichMirrorSortingCorrection() {}

InitStatus CbmRichMirrorSortingCorrection::Init()
{
	FairRootManager* manager = FairRootManager::Instance();

	fGlobalTracks = (TClonesArray*) manager->GetObject("GlobalTrack");
	if (NULL == fGlobalTracks) { Fatal("CbmRichMirrorSortingAlignment::Init", "No GlobalTrack array!"); }

	fRichRings = (TClonesArray*) manager->GetObject("RichRing");
	if (NULL == fRichRings) { Fatal("CbmRichMirrorSortingAlignment::Init", "No RichRing array !"); }

	fMCTracks = (TClonesArray*) manager->GetObject("MCTrack");
	if (NULL == fMCTracks) { Fatal("CbmRichMirrorSortingAlignment::Init", "No MCTracks array !"); }

	fMirrorPoints = (TClonesArray*) manager->GetObject("RichMirrorPoint");
	if (NULL == fMirrorPoints) { Fatal("CbmRichMirrorSortingAlignment::Init", "No RichMirrorPoints array !"); }

	fRefPlanePoints  = (TClonesArray*) manager->GetObject("RefPlanePoint");
	if (NULL == fRefPlanePoints) { Fatal("CbmRichMirrorSortingAlignment::Init", "No RichRefPlanePoint array !"); }

	fPmtPoints = (TClonesArray*) manager->GetObject("RichPoint");
	if (NULL == fPmtPoints) { Fatal("CbmRichMirrorSortingAlignment::Init", "No RichPoint array !"); }

	fRichProjections = (TClonesArray*) manager->GetObject("RichProjection");
	if (NULL == fRichProjections) { Fatal("CbmRichMirrorSortingAlignment::Init", "No RichProjection array !"); }

    fTrackParams = (TClonesArray*)manager->GetObject("RichTrackParamZ");
    if ( NULL == fTrackParams) { Fatal("CbmRichMirrorSortingAlignment::Init", "No RichTrackParamZ array!"); }

    fRichRingMatches = (TClonesArray*) manager->GetObject("RichRingMatch");
    if ( NULL == fRichRingMatches) { Fatal("CbmRichMirrorSortingAlignment::Init","No RichRingMatch array!"); }

    fStsTrackMatches = (TClonesArray*) manager->GetObject("StsTrackMatch");
    if ( NULL == fStsTrackMatches) {Fatal("CbmRichMirrorSortingAlignment::Init", "No StsTrackMatch array!");}

	fCopFit = new CbmRichRingFitterCOP();
	fTauFit = new CbmRichRingFitterEllipseTau();
	CbmRichConverter::Init();

	InitHistProjection();

	InitHistProjectionList();

	return kSUCCESS;
}

void CbmRichMirrorSortingCorrection::InitHistProjection()
{
	fHM = new CbmHistManager();

	Double_t upperScaleLimit = 6., bin = 400.;
	// fhDistance => fhDistanceCenterToExtrapolatedTrack.
	fHM->Create1<TH1D>("fhDistanceCenterToExtrapolatedTrack", "fhDistanceCenterToExtrapolatedTrack;Distance fitted center to extrapolated track;Number of entries", bin, 0., upperScaleLimit);
	fHM->Create1<TH1D>("fhDistanceCorrected", "fhDistanceCorrected;Distance a [cm];A.U.", bin, 0., upperScaleLimit);
	fHM->Create1<TH1D>("fhDifferenceX", "fhDifferenceX;Difference in X (fitted center - extrapolated track);A.U.", bin, -upperScaleLimit, upperScaleLimit);
	fHM->Create1<TH1D>("fhDifferenceY", "fhDifferenceY;Difference in Y (fitted center - extrapolated track);A.U.", bin, -upperScaleLimit, upperScaleLimit);

	fHM->Create1<TH1D>("fhDistanceUncorrected", "fhDistanceUncorrected;Distance a [cm];A.U.", bin, 0., upperScaleLimit);
	fHM->Create1<TH1D>("fhDifferenceXUncorrected", "fhDifferenceXUncorrected;Difference in X uncorrected [cm];A.U.", bin, -upperScaleLimit, upperScaleLimit);
	fHM->Create1<TH1D>("fhDifferenceYUncorrected", "fhDifferenceYUncorrected;Difference in Y uncorrected [cm];A.U.", bin, -upperScaleLimit, upperScaleLimit);

	fHM->Create1<TH1D>("fhDistanceIdeal", "fhDistanceIdeal;Distance a [cm];A.U.", bin, 0., upperScaleLimit);
	fHM->Create1<TH1D>("fhDifferenceXIdeal", "fhDifferenceXIdeal;Difference in X ideal [cm];A.U.", bin, -upperScaleLimit, upperScaleLimit);
	fHM->Create1<TH1D>("fhDifferenceYIdeal", "fhDifferenceYIdeal;Difference in Y ideal [cm];A.U.", bin, -upperScaleLimit, upperScaleLimit);

	fHM->Create1<TH1D>("fHistoDiffX", "fHistoDiffX;Histogram difference between corrected and ideal X positions;A.U.", bin, 0., upperScaleLimit);
	fHM->Create1<TH1D>("fHistoDiffY", "fHistoDiffY;Histogram difference between corrected and ideal Y positions;A.U.", bin, 0., upperScaleLimit);

	fHM->Create1<TH1D>("fHistoBoA", "fHistoBoA;Histogram B axis over A axis;A.U.", bin, 0., upperScaleLimit);
}

void CbmRichMirrorSortingCorrection::InitHistProjectionList()
{
	//fHM2 = new CbmHistManager();

	Double_t upperScaleLimit = 6., bin = 400.;
	fDiffHistoMap["DiffCorrX_mirror_tile_2_8"] = new TH1D("fhDifferenceCorrectedX_mirror_tile_2_8", ";Difference in X (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffCorrY_mirror_tile_2_8"] = new TH1D("fhDifferenceCorrectedY_mirror_tile_2_8", ";Difference in Y (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffUncorrX_mirror_tile_2_8"] = new TH1D("fhDifferenceUncorrectedX_mirror_tile_2_8", ";Difference in X (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffUncorrY_mirror_tile_2_8"] = new TH1D("fhDifferenceUncorrectedY_mirror_tile_2_8", ";Difference in Y (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffIdealX_mirror_tile_2_8"] = new TH1D("fhDifferenceIdealX_mirror_tile_2_8", ";Difference in X (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffIdealY_mirror_tile_2_8"] = new TH1D("fhDifferenceIdealY_mirror_tile_2_8", ";Difference in Y (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);

	fDiffHistoMap["DiffCorrX_mirror_tile_1_3"] = new TH1D("fhDifferenceCorrectedX_mirror_tile_1_3", ";Difference in X (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffCorrY_mirror_tile_1_3"] = new TH1D("fhDifferenceCorrectedY_mirror_tile_1_3", ";Difference in Y (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffUncorrX_mirror_tile_1_3"] = new TH1D("fhDifferenceUncorrectedX_mirror_tile_1_3", ";Difference in X (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffUncorrY_mirror_tile_1_3"] = new TH1D("fhDifferenceUncorrectedY_mirror_tile_1_3", ";Difference in Y (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffIdealX_mirror_tile_1_3"] = new TH1D("fhDifferenceIdealX_mirror_tile_1_3", ";Difference in X (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffIdealY_mirror_tile_1_3"] = new TH1D("fhDifferenceIdealY_mirror_tile_1_3", ";Difference in Y (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);

	fDiffHistoMap["DiffCorrX_mirror_tile_1_4"] = new TH1D("fhDifferenceCorrectedX_mirror_tile_1_4", ";Difference in X (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffCorrY_mirror_tile_1_4"] = new TH1D("fhDifferenceCorrectedY_mirror_tile_1_4", ";Difference in Y (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffUncorrX_mirror_tile_1_4"] = new TH1D("fhDifferenceUncorrectedX_mirror_tile_1_4", ";Difference in X (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffUncorrY_mirror_tile_1_4"] = new TH1D("fhDifferenceUncorrectedY_mirror_tile_1_4", ";Difference in Y (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffIdealX_mirror_tile_1_4"] = new TH1D("fhDifferenceIdealX_mirror_tile_1_4", ";Difference in X (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);
	fDiffHistoMap["DiffIdealY_mirror_tile_1_4"] = new TH1D("fhDifferenceIdealY_mirror_tile_1_4", ";Difference in Y (fitted center - extrapolated track);A.U.", bin, 0., upperScaleLimit);

	/*fDiffHistoMap["fhDifferenceX_mirror_tile_2_8"] = "X_mirror_tile_2_8";
	fDiffHistoMap["fhDifferenceY_mirror_tile_2_8"] = "Y_mirror_tile_2_8";
	fDiffHistoMap["fhDifferenceX_mirror_tile_1_3"] = "X_mirror_tile_1_3";
	fDiffHistoMap["fhDifferenceY_mirror_tile_1_3"] = "Y_mirror_tile_1_3";
	fDiffHistoMap["fhDifferenceX_mirror_tile_1_4"] = "X_mirror_tile_1_4";
	fDiffHistoMap["fhDifferenceY_mirror_tile_1_4"] = "Y_mirror_tile_1_4";

	for (std::map<string,string>::iterator it=fDiffHistoMap.begin(); it!=fDiffHistoMap.end(); ++it) {			// Initialize all the histograms, using map IDs as inputs.
		cout << "first: " << it->first << " and second: " << it->second << endl;
		fHM2->Create1<TH1D>(it->first, it->first + ";Difference in X (fitted center - extrapolated track);A.U.", bin, -upperScaleLimit, upperScaleLimit);
	}*/
}

void CbmRichMirrorSortingCorrection::Exec(Option_t* Option)
{
	fEventNb++;
	cout << "CbmRichMirrorSortingCorrection: Event #" << fEventNb << endl;
	TVector3 momentum, outPos, outPosUnCorr, outPosIdeal;
	Double_t constantePMT = 0., trackX=0., trackY=0.;
	vector<Double_t> vect(2,0), ptM(3,0), ptC(3,0), ptCIdeal(3,0), ptR1(3,0), ptR2Center(3,0), ptR2Mirr(3,0), ptPR2(3,0), ptPMirr(3,0), normalPMT(3,0);
	vector<Double_t> ptR2CenterUnCorr(3,0), ptR2CenterIdeal(3,0), ptR2MirrUnCorr(3,0), ptR2MirrIdeal(3,0), ptPMirrUnCorr(3,0), ptPMirrIdeal(3,0), ptPR2UnCorr(3,0), ptPR2Ideal(3,0);
	ptC.at(0) = 0., ptC.at(1) = 132.594000, ptC.at(2) = 54.267226;
	TVector3 mirrorPoint, dirCos, pos;
	Double_t nx=0., ny=0., nz=0.;
	TGeoNode* mirrNode;
	CbmRichPoint *mirrPoint, *refPlanePoint;

	if (fRichRings->GetEntries() != 0) {
		cout << "Nb of rings in evt = " << fRichRings->GetEntries() << endl << endl;
		GetPmtNormal(fPmtPoints->GetEntries(), normalPMT, constantePMT);
		//cout << "Calculated normal vector to PMT plane = {" << normalPMT.at(0) << ", " << normalPMT.at(1) << ", " << normalPMT.at(2) << "} and constante d = " << constantePMT << endl;

		for (Int_t iGlobalTrack = 0; iGlobalTrack < fGlobalTracks->GetEntriesFast(); iGlobalTrack++) {
				// ----- PART 1 ----- //
			// Ring-Track matching + Ring fit + Track momentum:
			CbmGlobalTrack* gTrack  = (CbmGlobalTrack*) fGlobalTracks->At(iGlobalTrack);
			Int_t richInd = gTrack->GetRichRingIndex();
			Int_t stsInd = gTrack->GetStsTrackIndex();
			//cout << "richInd: " << richInd << endl;
			if (richInd < 0) {
				cout << "Error richInd < 0" << endl;
				continue;
			}
			CbmRichRing* ring = (CbmRichRing*) fRichRings->At(richInd);
			if (ring == NULL) {
				cout << "Error ring == NULL!" << endl;
				continue;
			}
			Int_t ringTrackID = ring->GetTrackID();
			//cout << "ringTrackID: " << ringTrackID << endl;
			CbmTrackMatchNew* cbmRichTrackMatch = (CbmTrackMatchNew*) fRichRingMatches->At(richInd);
			CbmTrackMatchNew* cbmStsTrackMatch = (CbmTrackMatchNew*) fStsTrackMatches->At(stsInd);
			if (NULL == cbmRichTrackMatch) { continue; }
			cout << "Nof true hits = " << cbmRichTrackMatch->GetNofTrueHits() << endl;
			cout << "Nof wrong hits = " << cbmRichTrackMatch->GetNofWrongHits() << endl;
			Int_t mcRichTrackId = cbmRichTrackMatch->GetMatchedLink().GetIndex();
			Int_t mcStsTrackId = cbmStsTrackMatch->GetMatchedLink().GetIndex();
			//cout << "mcTrackId: " << mcRichTrackId << endl;
			if (mcRichTrackId < 0) continue;
			if (mcStsTrackId != ringTrackID) {
				cout << "Error StsTrackIndex and TrackIndex from Ring do not match!" << endl;
				continue;
			}
			CbmMCTrack* mcTrack = (CbmMCTrack*) fMCTracks->At(mcRichTrackId);
			if (!mcTrack) continue;

			CbmRichRingLight ringL;
			CbmRichConverter::CopyHitsToRingLight(ring, &ringL);
			fCopFit->DoFit(&ringL);
			//fTauFit->DoFit(&ringL);
			cout << "ring Center Coo: " << ringL.GetCenterX() << ", " << ringL.GetCenterY() << endl;
			mcTrack->GetMomentum(momentum);
			FairTrackParam* pr = (FairTrackParam*) fRichProjections->At(ringTrackID);
			if (pr == NULL) {
				cout << "CbmRichMirrorSortingCorrection::Exec : pr = NULL." << endl;
				continue;
			}
			trackX = pr->GetX(), trackY = pr->GetY();
			cout << "Track: " << trackX << ", " << trackY << endl;

				// ----- PART 2 ----- //
			// Mirror ID via TGeoNavigator + Extrap hit:
			Int_t trackMotherId = mcTrack->GetMotherId();
			Int_t pdg = TMath::Abs(mcTrack->GetPdgCode());
			if (trackMotherId == -1) {
				//loop on mirrorPoint and compare w/ TrackID->GetTrackId to get correct one
				for (Int_t iMirrPt=0 ; iMirrPt<fMirrorPoints->GetEntries(); iMirrPt++) {
					mirrPoint = (CbmRichPoint*) fMirrorPoints->At(iMirrPt);
					if (mirrPoint == 0) { continue; }
					//cout << "Mirror point track ID: " << mirrPoint->GetTrackID() << endl;
					if (mirrPoint->GetTrackID() == mcRichTrackId) { break; }
				}
				ptM.at(0) = mirrPoint->GetX(), ptM.at(1) = mirrPoint->GetY(), ptM.at(2) = mirrPoint->GetZ();
				//cout << "mirrPoint: {" << mirrPoint->GetX() << ", " << mirrPoint->GetY() << ", " << mirrPoint->GetZ() << "}" << endl;
				mirrNode = gGeoManager->FindNode(ptM.at(0),ptM.at(1),ptM.at(2));
				//cout << "Mirror node name: " << mirrNode->GetName() << " and full path " << gGeoManager->GetPath() << endl;
				string str1 = gGeoManager->GetPath(), str2 = "mirror_tile_", str3 = "";
				std::size_t found = str1.find(str2);
				if (found!=std::string::npos) {
					//cout << "first 'mirror_tile_type' found at: " << found << '\n';
					Int_t end = str2.length() + 3;
					str3 = str1.substr(found, end);
				}
				cout << "Mirror ID: " << str3 << endl;

				if (mirrNode) {
					TGeoNavigator* navi = gGeoManager->GetCurrentNavigator();
					//cout << "Navigator path: " << navi->GetPath() << endl;
					ptCIdeal.at(0) = navi->GetCurrentMatrix()->GetTranslation()[0];
					ptCIdeal.at(1) = navi->GetCurrentMatrix()->GetTranslation()[1];
					ptCIdeal.at(2) = navi->GetCurrentMatrix()->GetTranslation()[2];
					cout << "Sphere center coordinates of the aligned mirror tile, ideal = {" << ptCIdeal.at(0) << ", " << ptCIdeal.at(1) << ", " << ptCIdeal.at(2) << "}" << endl;
					for (Int_t iRefPt=0 ; iRefPt<fRefPlanePoints->GetEntries(); iRefPt++) {
						refPlanePoint = (CbmRichPoint*) fRefPlanePoints->At(iRefPt);
						//cout << "Refl plane point track ID: " << refPlanePoint->GetTrackID() << endl;
						if (refPlanePoint->GetTrackID() == mcRichTrackId) { break; }
					}
					ptR1.at(0) = refPlanePoint->GetX(), ptR1.at(1) = refPlanePoint->GetY(), ptR1.at(2) = refPlanePoint->GetZ();
					cout << "Refl plane point coo = {" << ptR1[0] << ", " << ptR1[1] << ", " << ptR1[2] << "}" << endl;
					ComputeR2(ptR2Center, ptR2Mirr, ptM, ptC, ptR1, navi, "Corrected", str3);
					ComputeR2(ptR2CenterUnCorr, ptR2MirrUnCorr, ptM, ptC, ptR1, navi, "Uncorrected", str3);
					ComputeR2(ptR2CenterIdeal, ptR2MirrIdeal, ptM, ptCIdeal, ptR1, navi, "Uncorrected", str3);
					ComputeP(ptPMirr, ptPR2, normalPMT, ptM, ptR2Mirr, constantePMT);
					ComputeP(ptPMirrUnCorr, ptPR2UnCorr, normalPMT, ptM, ptR2MirrUnCorr, constantePMT);
					ComputeP(ptPMirrIdeal, ptPR2Ideal, normalPMT, ptM, ptR2MirrIdeal, constantePMT);
					cout << "PMT points mirr coordinates before rotation = {" << ptPMirr[0] << ", " << ptPMirr[1] << ", " << ptPMirr[2] << "}" << endl;
					cout << "PMT points mirr uncorr coordinates before rotation = {" << ptPMirrUnCorr[0] << ", " << ptPMirrUnCorr[1] << ", " << ptPMirrUnCorr[2] << "}" << endl;
					cout << "PMT points mirr ideal coordinates before rotation = {" << ptPMirrIdeal[0] << ", " << ptPMirrIdeal[1] << ", " << ptPMirrIdeal[2] << "}" << endl;

					TVector3 inPos (ptPMirr.at(0), ptPMirr.at(1), ptPMirr.at(2));
					CbmRichGeoManager::GetInstance().RotatePoint(&inPos, &outPos);
					cout << endl << "New PMT points coordinates = {" << outPos.x() << ", " << outPos.y() << ", " << outPos.z() << "}" << endl;
					TVector3 inPosUnCorr (ptPMirrUnCorr.at(0), ptPMirrUnCorr.at(1), ptPMirrUnCorr.at(2));
					CbmRichGeoManager::GetInstance().RotatePoint(&inPosUnCorr, &outPosUnCorr);
					cout << "New mirror points coordinates = {" << outPosUnCorr.x() << ", " << outPosUnCorr.y() << ", " << outPosUnCorr.z() << "}" << endl;
					TVector3 inPosIdeal (ptPMirrIdeal.at(0), ptPMirrIdeal.at(1), ptPMirrIdeal.at(2));
					CbmRichGeoManager::GetInstance().RotatePoint(&inPosIdeal, &outPosIdeal);
					cout << "New mirror points coordinates = {" << outPosIdeal.x() << ", " << outPosIdeal.y() << ", " << outPosIdeal.z() << "}" << endl << endl;

					FillHistProjection(outPosIdeal, outPosUnCorr, outPos, ringL, normalPMT, constantePMT, str3);
				}
			}
			else { cout << "Not a mother particle." << endl; }
			//ComputeAngles();
		}
	}
	else { cout << "CbmRichMirrorSortingCorrection::Exec No rings in event were found." << endl; }
}

void CbmRichMirrorSortingCorrection::GetPmtNormal(Int_t NofPMTPoints, vector<Double_t> &normalPMT, Double_t &normalCste)
{
	//cout << endl << "//------------------------------ CbmRichMirrorSortingAlignment: Calculate PMT Normal ------------------------------//" << endl << endl;

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

void CbmRichMirrorSortingCorrection::ComputeR2(vector<Double_t> &ptR2Center, vector<Double_t> &ptR2Mirr, vector<Double_t> ptM, vector<Double_t> ptC, vector<Double_t> ptR1, TGeoNavigator* navi, TString option, TString mirrorTileName)
{
	//cout << endl << "//------------------------------ CbmRichCorrection: ComputeR2 ------------------------------//" << endl << endl;

	vector<Double_t> normalMirr(3), ptCNew(3), ptTileCenter(3);
	Double_t t1=0., t2=0., t3=0.;

	if (option == "Corrected") {
		// Use the correction information from text file, to the tile sphere center:
		// Reading misalignment information from correction_param.txt text file.
		Int_t lineCounter=1, lineIndex=0;
		TString str = fOutputDir + "correction_param_array_" + fStudyName + ".txt";
		string fileLine = "", strMisX = "", strMisY = "";
		Double_t misX=0., misY=0.;
		ifstream corrFile;
		corrFile.open(str);

		/*std::ifstream inFile(corrFile);
		if(!inFile) {
			cout << endl << "Failed to open file " << corrFile;
			return;
		}
		double d1 = 0.;
		double d2 = 0.;
		while(!inFile.eof()) {
			inFile >> d1 >> d2;
			cout << d1 << " " << d2 << endl;;
		}*/

		if (corrFile.is_open())
		{
			while (!corrFile.eof())
			{
				getline(corrFile, fileLine);
				lineIndex = fileLine.find(mirrorTileName, 0);
				if (lineIndex != string::npos)
				{
					//cout << mirrorTileName << " has been found in the file at line: " << lineCounter << " and position: " << lineIndex << "." << endl;
					break;
				}
				lineCounter++;
			}
//			getline(corrFile, strMisX);
			corrFile >> misY;
			//cout << "number at line: " << lineCounter+1 << " = " << misX << "." << endl;
//			getline(corrFile, strMisY);
			corrFile >> misX;
			//cout << "number at line: " << lineCounter+2 << " = " << misY << "." << endl;

			/*std::istringstream i1(strMisX);
			i1 >> misX;
			std::istringstream i2(strMisY);
			i2 >> misY;
			double sum = misX + misY;
			cout << "x1 = " << misX << ", x2 = " << misY << ", sum = " << sum << endl;*/

			corrFile.close();
		}
		else {
			cout << "Error in CbmRichCorrection: unable to open parameter file!" << endl;
			cout << "Parameter file path: " << str << endl << endl;
			sleep(5);
		}
		//cout << "Misalignment parameters read from file = [" << outputFit.at(0) << " ; " << outputFit.at(1) << " ; " << outputFit.at(2) << " ; " << outputFit.at(3) << "]" << endl;

		//ptCNew.at(0) = TMath::Abs(ptC.at(0) - TMath::Abs(outputFit.at(3)));
		//ptCNew.at(1) = TMath::Abs(ptC.at(1) - TMath::Abs(outputFit.at(2)));
		cout << "Correction parameters = " << misX << ", " << misY << endl;
		ptCNew.at(0) = ptC.at(0) + misX;
		ptCNew.at(1) = ptC.at(1) + misY;
		ptCNew.at(2) = ptC.at(2);
		ptTileCenter.at(0) = navi->GetMotherMatrix()->GetTranslation()[0];
		ptTileCenter.at(1) = navi->GetMotherMatrix()->GetTranslation()[1];
		ptTileCenter.at(2) = navi->GetMotherMatrix()->GetTranslation()[2];
		//cout << "Mirror tile center coordinates = {" << ptTileCenter.at(0) << ", " << ptTileCenter.at(1) << ", " << ptTileCenter.at(2) << "}" << endl;
		Double_t x=0., y=0., z=0., dist=0., dist2=0., z2=0.;
		x = TMath::Power(ptCNew.at(0)-ptTileCenter.at(0), 2);
		y = TMath::Power(ptCNew.at(1)-ptTileCenter.at(1), 2);
		z = TMath::Power(ptCNew.at(2)-ptTileCenter.at(2), 2);
		dist = TMath::Sqrt(x + y + z);
		z2 = ptTileCenter.at(2) - TMath::Sqrt(TMath::Power(300,2) - x - y) - ptCNew.at(2);
		//cout << "{x, y, z} = {" << x << ", " << y << ", " << z << "}, dist = " << dist << " and z2 = " << z2 << endl;
		dist2 = TMath::Sqrt(x + y + TMath::Power(z2-ptTileCenter.at(2), 2));
		//cout << "dist2 = " << dist2 << endl;
		ptCNew.at(2) += z2;
		cout << "Sphere center coordinates of the rotated mirror tile, after correction, = {" << ptCNew.at(0) << ", " << ptCNew.at(1) << ", " << ptCNew.at(2) << "}" << endl;
	}
	else if (option == "Uncorrected") {
		// Keep the same tile sphere center, with no correction information.
		ptCNew = ptC;
		cout << "Sphere center coordinates of the rotated mirror tile, without correction = {" << ptCNew.at(0) << ", " << ptCNew.at(1) << ", " << ptCNew.at(2) << "}" << endl;
	}
	else {
		//cout << "No input given in function ComputeR2! Uncorrected parameters for the sphere center of the tile will be used!" << endl;
		ptCNew = ptC;
		//cout << "Sphere center coordinates of the rotated mirror tile, without correction = {" << ptCNew.at(0) << ", " << ptCNew.at(1) << ", " << ptCNew.at(2) << "}" << endl;
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
	//cout << "* using mirror point M to define \U00000394: {" << ptR2Center.at(0) << ", " << ptR2Center.at(1) << ", " << ptR2Center.at(2) << "}" << endl;
	//cout << "Ref Pt Coo using unity Mirror-Sphere vector & sphere pt = {" << reflectedPtCooVectSphereUnity[0] << ", " << reflectedPtCooVectSphereUnity[1] << ", " << reflectedPtCooVectSphereUnity[2] << "}" << endl << endl;
	//cout << "NofPMTPoints = " << NofPMTPoints << endl;

	//cout << "Coordinates of point R2 on reflective plane after reflection on the mirror tile:" << endl;
	//cout << "* using sphere center C to define \U00000394: {" << ptR2Mirr.at(0) << ", " << ptR2Mirr.at(1) << ", " << ptR2Mirr.at(2) << "}" << endl;
}

void CbmRichMirrorSortingCorrection::ComputeP(vector<Double_t> &ptPMirr, vector<Double_t> &ptPR2, vector<Double_t> normalPMT, vector<Double_t> ptM, vector<Double_t> ptR2Mirr, Double_t constantePMT)
{
	//cout << endl << "//------------------------------ CbmRichCorrection: ComputeP ------------------------------//" << endl << endl;

	Double_t k1=0., k2=0., checkCalc1=0., checkCalc2=0.;

	k1 = -1*((normalPMT.at(0)*ptM.at(0) + normalPMT.at(1)*ptM.at(1) + normalPMT.at(2)*ptM.at(2) + constantePMT)/(normalPMT.at(0)*(ptR2Mirr.at(0)-ptM.at(0)) + normalPMT.at(1)*(ptR2Mirr.at(1)-ptM.at(1)) + normalPMT.at(2)*(ptR2Mirr.at(2)-ptM.at(2))));
	ptPMirr.at(0) = ptM.at(0) + k1*(ptR2Mirr.at(0) - ptM.at(0));
	ptPMirr.at(1) = ptM.at(1) + k1*(ptR2Mirr.at(1) - ptM.at(1));
	ptPMirr.at(2) = ptM.at(2) + k1*(ptR2Mirr.at(2) - ptM.at(2));
	k2 = -1*((normalPMT.at(0)*ptR2Mirr.at(0) + normalPMT.at(1)*ptR2Mirr.at(1) + normalPMT.at(2)*ptR2Mirr.at(2) + constantePMT)/(normalPMT.at(0)*(ptR2Mirr.at(0)-ptM.at(0)) + normalPMT.at(1)*(ptR2Mirr.at(1)-ptM.at(1)) + normalPMT.at(2)*(ptR2Mirr.at(2)-ptM.at(2))));
	ptPR2.at(0) = ptR2Mirr.at(0) + k2*(ptR2Mirr.at(0) - ptM.at(0));
	ptPR2.at(1) = ptR2Mirr.at(1) + k2*(ptR2Mirr.at(1) - ptM.at(1));
	ptPR2.at(2) = ptR2Mirr.at(2) + k2*(ptR2Mirr.at(2) - ptM.at(2));
	//cout << "Coordinates of point P on PMT plane, after reflection on the mirror tile and extrapolation to the PMT plane:" << endl;
	//cout << "* using mirror point M to define \U0001D49F ': {" << ptPMirr.at(0) << ", " << ptPMirr.at(1) << ", " << ptPMirr.at(2) << "}" << endl;
	//cout << "* using reflected point R2 to define \U0001D49F ': {" << ptPR2.at(0) << ", " << ptPR2.at(1) << ", " << ptPR2.at(2) << "}" << endl;
	checkCalc1 = ptPMirr.at(0)*normalPMT.at(0) + ptPMirr.at(1)*normalPMT.at(1) + ptPMirr.at(2)*normalPMT.at(2) + constantePMT;
	//cout << "Check whether extrapolated track point on PMT plane verifies its equation (value should be 0.):" << endl;
	//cout << "* using mirror point M, checkCalc = " << checkCalc1 << endl;
	checkCalc2 = ptPR2.at(0)*normalPMT.at(0) + ptPR2.at(1)*normalPMT.at(1) + ptPR2.at(2)*normalPMT.at(2) + constantePMT;
	//cout << "* using reflected point R2, checkCalc = " << checkCalc2 << endl;
}

void CbmRichMirrorSortingCorrection::FillHistProjection(TVector3 outPosIdeal, TVector3 outPosUnCorr, TVector3 outPos, CbmRichRingLight ringL, vector<Double_t> normalPMT, Double_t constantePMT, string str)
{
	Double_t ringCenter[] = {0, 0, 0}, distToExtrapTrackHit = 0, distToExtrapTrackHitInPlane = 0, distToExtrapTrackHitInPlaneUnCorr = 0, distToExtrapTrackHitInPlaneIdeal = 0;
	string histoNameX = "", histoNameY = "";
	string nameX = "", nameY = "";

	ringCenter[0] = ringL.GetCenterX();
	ringCenter[1] = ringL.GetCenterY();
	ringCenter[2] = -1*((normalPMT.at(0)*ringCenter[0] + normalPMT.at(1)*ringCenter[1] + constantePMT)/normalPMT.at(2));

	// Calculation using the corrected mirror hit/position, using the correction method
	vector<Double_t> r(3), p(3); // Absolute coordinates of fitted ring Center r and PMT extrapolated point p
	r.at(0) = ringCenter[0], r.at(1) = ringCenter[1], r.at(2) = ringCenter[2];
	p.at(0) = outPos.x(), p.at(1) = outPos.y(), p.at(2) = outPos.z();
	cout << "Ring center coordinates = {" << ringCenter[0] << ", " << ringCenter[1] << ", " << ringCenter[2] << "}" << endl;
	cout << "Difference in X = " << TMath::Abs(r.at(0) - p.at(0)) << "; \t" << "Difference in Y = " << TMath::Abs(r.at(1) - p.at(1)) << "; \t" << "Difference in Z = " << TMath::Abs(r.at(2) - p.at(2)) << endl;

	nameX = string("DiffCorrX_") + str;
	nameY = string("DiffCorrY_") + str;
	for (std::map<string,TH1D*>::iterator it=fDiffHistoMap.begin(); it!=fDiffHistoMap.end(); ++it) {
		if ( nameX.compare(it->first) == 0 ) {
			fDiffHistoMap[it->first]->Fill(TMath::Abs(r.at(0) - p.at(0)));
		}
		if ( nameY.compare(it->first) == 0 ) {
			fDiffHistoMap[it->first]->Fill(TMath::Abs(r.at(1) - p.at(1)));
		}
	}

	distToExtrapTrackHit = TMath::Sqrt(TMath::Power(r.at(0) - p.at(0),2) + TMath::Power(r.at(1) - p.at(1),2) + TMath::Power(r.at(2) - p.at(2),2));
	distToExtrapTrackHitInPlane = TMath::Sqrt(TMath::Power(r.at(0) - p.at(0),2) + TMath::Power(r.at(1) - p.at(1),2));
	fHM->H1("fhDistanceCenterToExtrapolatedTrack")->Fill(distToExtrapTrackHit);
	fHM->H1("fhDistanceCorrected")->Fill(distToExtrapTrackHitInPlane);
	//cout << "Distance between fitted ring center and extrapolated track hit = " << distToExtrapTrackHit << endl;
	cout << "Distance between fitted ring center and extrapolated track hit in plane = " << distToExtrapTrackHitInPlane << endl;

	// Calculation using the uncorrected mirror hit/position
	vector<Double_t> pUncorr(3); // Absolute coordinates of fitted ring Center r and PMT extrapolated point p
	pUncorr.at(0) = outPosUnCorr.x(), pUncorr.at(1) = outPosUnCorr.y(), pUncorr.at(2) = outPosUnCorr.z();
	//cout << "Ring center coordinates = {" << ringCenter[0] << ", " << ringCenter[1] << ", " << ringCenter[2] << "}" << endl;
	cout << "Difference in X w/o correction = " << TMath::Abs(r.at(0) - pUncorr.at(0)) << "; \t" << "Difference in Y = " << TMath::Abs(r.at(1) - pUncorr.at(1)) << "; \t" << "Difference in Z = " << TMath::Abs(r.at(2) - pUncorr.at(2)) << endl;

	nameX = string("DiffUncorrX_") + str;
	nameY = string("DiffUncorrY_") + str;
	for (std::map<string,TH1D*>::iterator it=fDiffHistoMap.begin(); it!=fDiffHistoMap.end(); ++it) {
		if ( nameX.compare(it->first) == 0 ) {
			fDiffHistoMap[it->first]->Fill(TMath::Abs(r.at(0) - pUncorr.at(0)));
		}
		if ( nameY.compare(it->first) == 0 ) {
			fDiffHistoMap[it->first]->Fill(TMath::Abs(r.at(1) - pUncorr.at(1)));
		}
	}

	distToExtrapTrackHitInPlaneUnCorr = TMath::Sqrt(TMath::Power(r.at(0) - pUncorr.at(0),2) + TMath::Power(r.at(1) - pUncorr.at(1),2));
	fHM->H1("fhDistanceUncorrected")->Fill(distToExtrapTrackHitInPlaneUnCorr);
	cout << "Distance between fitted ring center and extrapolated track hit in plane = " << distToExtrapTrackHitInPlaneUnCorr << endl;

	// Calculation using the ideally corrected mirror hit/position
	vector<Double_t> pIdeal(3); // Absolute coordinates of fitted ring Center r and PMT extrapolated point p
	pIdeal.at(0) = outPosIdeal.x(), pIdeal.at(1) = outPosIdeal.y(), pIdeal.at(2) = outPosIdeal.z();
	//cout << "Ring center coordinates = {" << ringCenter[0] << ", " << ringCenter[1] << ", " << ringCenter[2] << "}" << endl;
	cout << "Difference in X w/ ideal correction = " << TMath::Abs(r.at(0) - pIdeal.at(0)) << "; \t" << "Difference in Y = " << TMath::Abs(r.at(1) - pIdeal.at(1)) << "; \t" << "Difference in Z = " << TMath::Abs(r.at(2) - pIdeal.at(2)) << endl;

	nameX = string("DiffIdealX_") + str;
	nameY = string("DiffIdealY_") + str;
	for (std::map<string,TH1D*>::iterator it=fDiffHistoMap.begin(); it!=fDiffHistoMap.end(); ++it) {
		if ( nameX.compare(it->first) == 0 ) {
			fDiffHistoMap[it->first]->Fill(TMath::Abs(r.at(0) - pIdeal.at(0)));
		}
		if ( nameY.compare(it->first) == 0 ) {
			fDiffHistoMap[it->first]->Fill(TMath::Abs(r.at(1) - pIdeal.at(1)));
		}
	}

	distToExtrapTrackHitInPlaneIdeal = TMath::Sqrt(TMath::Power(r.at(0) - pIdeal.at(0),2) + TMath::Power(r.at(1) - pIdeal.at(1),2));
	fHM->H1("fhDistanceIdeal")->Fill(distToExtrapTrackHitInPlaneIdeal);
	cout << "Distance between fitted ring center and extrapolated track hit in plane = " << distToExtrapTrackHitInPlaneIdeal << endl << endl;
	//}
	//else { cout << "No identical ring mother ID and mirror track ID ..." << endl;}

	fTrackCenterDistanceCorrected += distToExtrapTrackHitInPlane;
	fTrackCenterDistanceUncorrected += distToExtrapTrackHitInPlaneUnCorr;
	fTrackCenterDistanceIdeal += distToExtrapTrackHitInPlaneIdeal;
}

void CbmRichMirrorSortingCorrection::DrawHistProjection()
{
	int counter1 = 1, counter2 = 1, counter3 = 1, counter4 = 1, counter5 = 1, counter6 = 1;
	Int_t thresh = 50;

/*	TCanvas* can1 = new TCanvas("X_mirror_tile_2_8","X_mirror_tile_2_8",1500,400);
	can1->Divide(3,1);
	TCanvas* can2 = new TCanvas("X_mirror_tile_1_3","X_mirror_tile_1_3",1500,400);
	can2->Divide(3,1);*/
	TCanvas* can3 = new TCanvas("X_mirror_tile_1_4","X_mirror_tile_1_4",1500,400);
	can3->Divide(3,1);
/*	TCanvas* can4 = new TCanvas("Y_mirror_tile_2_8","Y_mirror_tile_2_8",1500,400);
	can4->Divide(3,1);
	TCanvas* can5 = new TCanvas("Y_mirror_tile_1_3","Y_mirror_tile_1_3",1500,400);
	can5->Divide(3,1);*/
	TCanvas* can6 = new TCanvas("Y_mirror_tile_1_4","Y_mirror_tile_1_4",1500,400);
	can6->Divide(3,1);

	for (std::map<string,TH1D*>::iterator it=fDiffHistoMap.begin(); it!=fDiffHistoMap.end(); ++it) {
/*		if ( it->first.find("X_mirror_tile_2_8")!=std::string::npos && it->second->GetEntries() > thresh ) {
			can1->cd(counter1);
			fDiffHistoMap[it->first]->Draw();
			fDiffHistoMap[it->first]->SetTitle((it->first).c_str());
			fDiffHistoMap[it->first]->SetLineColor(counter1+1);
			fDiffHistoMap[it->first]->SetLineWidth(2);
			counter1++;
		}
		else if ( it->first.find("X_mirror_tile_1_3")!=std::string::npos && it->second->GetEntries() > thresh ) {
			can2->cd(counter2);
			fDiffHistoMap[it->first]->Draw();
			fDiffHistoMap[it->first]->SetTitle((it->first).c_str());
			fDiffHistoMap[it->first]->SetLineColor(counter2+1);
			fDiffHistoMap[it->first]->SetLineWidth(2);
			counter2++;
		}*/
		if ( it->first.find("X_mirror_tile_1_4")!=std::string::npos && it->second->GetEntries() > thresh ) {
			can3->cd(counter3);
			fDiffHistoMap[it->first]->Draw();
			fDiffHistoMap[it->first]->SetTitle((it->first).c_str());
			fDiffHistoMap[it->first]->SetLineColor(counter3+1);
			fDiffHistoMap[it->first]->SetLineWidth(2);
			counter3++;
		}
/*		else if ( it->first.find("Y_mirror_tile_2_8")!=std::string::npos && it->second->GetEntries() > thresh ) {
			can4->cd(counter4);
			fDiffHistoMap[it->first]->Draw();
			fDiffHistoMap[it->first]->SetTitle((it->first).c_str());
			fDiffHistoMap[it->first]->SetLineColor(counter4+1);
			fDiffHistoMap[it->first]->SetLineWidth(2);
			counter4++;
		}
		else if ( it->first.find("Y_mirror_tile_1_3")!=std::string::npos && it->second->GetEntries() > thresh ) {
			can5->cd(counter5);
			fDiffHistoMap[it->first]->Draw();
			fDiffHistoMap[it->first]->SetTitle((it->first).c_str());
			fDiffHistoMap[it->first]->SetLineColor(counter5+1);
			fDiffHistoMap[it->first]->SetLineWidth(2);
			counter5++;
		}*/
		else if ( it->first.find("Y_mirror_tile_1_4")!=std::string::npos && it->second->GetEntries() > thresh ) {
			can6->cd(counter6);
			fDiffHistoMap[it->first]->Draw();
			fDiffHistoMap[it->first]->SetTitle((it->first).c_str());
			fDiffHistoMap[it->first]->SetLineColor(counter6+1);
			fDiffHistoMap[it->first]->SetLineWidth(2);
			counter6++;
		}
		else if ( it->second->GetEntries() > thresh ) {
			TCanvas* can = new TCanvas();
			fDiffHistoMap[it->first]->Draw();
			fDiffHistoMap[it->first]->SetTitle((it->first).c_str());
			fDiffHistoMap[it->first]->SetLineColor(4);
			fDiffHistoMap[it->first]->SetLineWidth(2);
		}
	}
	//Cbm::SaveCanvasAsImage(can1, string(fOutputDir.Data()), "png");
	//Cbm::SaveCanvasAsImage(can2, string(fOutputDir.Data()), "png");
	//Cbm::SaveCanvasAsImage(can3, string(fOutputDir.Data()), "png");
	//Cbm::SaveCanvasAsImage(can4, string(fOutputDir.Data()), "png");
	//Cbm::SaveCanvasAsImage(can5, string(fOutputDir.Data()), "png");
	//Cbm::SaveCanvasAsImage(can6, string(fOutputDir.Data()), "png");

/*	char title[128];
	for (std::map<string,TH1D*>::iterator it=fDiffHistoMap.begin(); it!=fDiffHistoMap.end(); ++it) {
		TCanvas* can = new TCanvas();
		fDiffHistoMap[it->first]->Draw();
		//title = (it->first).c_str();
		fDiffHistoMap[it->first]->SetTitle((it->first).c_str());
		fDiffHistoMap[it->first]->SetLineColor(4);
		fDiffHistoMap[it->first]->SetLineWidth(2);
		//Cbm::SaveCanvasAsImage(can, string(fOutputDir.Data()), "png");
	}
*/
}

void CbmRichMirrorSortingCorrection::Finish()
{
	DrawHistProjection();
	cout << setprecision(9) << "Mean distance between track hit and ring center ; corrected case = " << fTrackCenterDistanceCorrected/fEventNb << " and total sum = " << fTrackCenterDistanceCorrected << endl;
	cout << "Mean distance between track hit and ring center ; uncorrected case = " << fTrackCenterDistanceUncorrected/fEventNb << " and total sum = " << fTrackCenterDistanceUncorrected << endl;
	cout << "Mean distance between track hit and ring center ; ideal case = " << fTrackCenterDistanceIdeal/fEventNb << " and total sum = " << fTrackCenterDistanceIdeal << endl;
}
ClassImp(CbmRichMirrorSortingCorrection)


/*void CbmRichMirrorSortingCorrection::FillHistProjection(TVector3 outPosIdeal, TVector3 outPosUnCorr, TVector3 outPos, CbmRichRingLight ringL, vector<Double_t> normalPMT, Double_t constantePMT, string str)
{
	Double_t ringCenter[] = {0, 0, 0}, distToExtrapTrackHit = 0, distToExtrapTrackHitInPlane = 0, distToExtrapTrackHitInPlaneUnCorr = 0, distToExtrapTrackHitInPlaneIdeal = 0;
	string histoNameX = "", histoNameY = "";
	string nameX = "", nameY = "";

	ringCenter[0] = ringL.GetCenterX();
	ringCenter[1] = ringL.GetCenterY();
	ringCenter[2] = -1*((normalPMT.at(0)*ringCenter[0] + normalPMT.at(1)*ringCenter[1] + constantePMT)/normalPMT.at(2));

	vector<Double_t> r(3), p(3); // Absolute coordinates of fitted ring Center r and PMT extrapolated point p
	r.at(0) = ringCenter[0], r.at(1) = ringCenter[1], r.at(2) = ringCenter[2];
	p.at(0) = outPos.x(), p.at(1) = outPos.y(), p.at(2) = outPos.z();
	cout << "Ring center coordinates = {" << ringCenter[0] << ", " << ringCenter[1] << ", " << ringCenter[2] << "}" << endl;
	cout << "Difference in X = " << TMath::Abs(r.at(0) - p.at(0)) << "; \t" << "Difference in Y = " << TMath::Abs(r.at(1) - p.at(1)) << "; \t" << "Difference in Z = " << TMath::Abs(r.at(2) - p.at(2)) << endl;

	nameX = string("X_") + str;
	nameY = string("Y_") + str;
	for (std::map<string,string>::iterator it=fDiffHistoMap.begin(); it!=fDiffHistoMap.end(); ++it) {
			if ( nameX.compare(it->second) == 0 ) {
				fHM2->H1(it->first)->Fill(TMath::Abs(r.at(0) - p.at(0)));
			}
			if ( nameY.compare(it->second) == 0 ) {
				fHM2->H1(it->first)->Fill(TMath::Abs(r.at(1) - p.at(1)));
			}
	}

	distToExtrapTrackHit = TMath::Sqrt(TMath::Power(r.at(0) - p.at(0),2) + TMath::Power(r.at(1) - p.at(1),2) + TMath::Power(r.at(2) - p.at(2),2));
	distToExtrapTrackHitInPlane = TMath::Sqrt(TMath::Power(r.at(0) - p.at(0),2) + TMath::Power(r.at(1) - p.at(1),2));
	fHM->H1("fhDistanceCenterToExtrapolatedTrack")->Fill(distToExtrapTrackHit);
	fHM->H1("fhDistanceCorrected")->Fill(distToExtrapTrackHitInPlane);
	//cout << "Distance between fitted ring center and extrapolated track hit = " << distToExtrapTrackHit << endl;
	cout << "Distance between fitted ring center and extrapolated track hit in plane = " << distToExtrapTrackHitInPlane << endl;

	vector<Double_t> pUnCorr(3); // Absolute coordinates of fitted ring Center r and PMT extrapolated point p
	pUnCorr.at(0) = outPosUnCorr.x(), pUnCorr.at(1) = outPosUnCorr.y(), pUnCorr.at(2) = outPosUnCorr.z();
	//cout << "Ring center coordinates = {" << ringCenter[0] << ", " << ringCenter[1] << ", " << ringCenter[2] << "}" << endl;
	cout << "Difference in X w/o correction = " << TMath::Abs(r.at(0) - pUnCorr.at(0)) << "; \t" << "Difference in Y = " << TMath::Abs(r.at(1) - pUnCorr.at(1)) << "; \t" << "Difference in Z = " << TMath::Abs(r.at(2) - pUnCorr.at(2)) << endl;
	fHM->H1("fhDifferenceXUncorrected")->Fill(TMath::Abs(r.at(0) - pUnCorr.at(0)));
	fHM->H1("fhDifferenceYUncorrected")->Fill(TMath::Abs(r.at(1) - pUnCorr.at(1)));
	distToExtrapTrackHitInPlaneUnCorr = TMath::Sqrt(TMath::Power(r.at(0) - pUnCorr.at(0),2) + TMath::Power(r.at(1) - pUnCorr.at(1),2));
	fHM->H1("fhDistanceUncorrected")->Fill(distToExtrapTrackHitInPlaneUnCorr);
	cout << "Distance between fitted ring center and extrapolated track hit in plane = " << distToExtrapTrackHitInPlaneUnCorr << endl;

	vector<Double_t> pIdeal(3); // Absolute coordinates of fitted ring Center r and PMT extrapolated point p
	pIdeal.at(0) = outPosIdeal.x(), pIdeal.at(1) = outPosIdeal.y(), pIdeal.at(2) = outPosIdeal.z();
	//cout << "Ring center coordinates = {" << ringCenter[0] << ", " << ringCenter[1] << ", " << ringCenter[2] << "}" << endl;
	cout << "Difference in X w/ ideal correction = " << TMath::Abs(r.at(0) - pIdeal.at(0)) << "; \t" << "Difference in Y = " << TMath::Abs(r.at(1) - pIdeal.at(1)) << "; \t" << "Difference in Z = " << TMath::Abs(r.at(2) - pIdeal.at(2)) << endl;
	fHM->H1("fhDifferenceXIdeal")->Fill(TMath::Abs(r.at(0) - pIdeal.at(0)));
	fHM->H1("fhDifferenceYIdeal")->Fill(TMath::Abs(r.at(1) - pIdeal.at(1)));
	distToExtrapTrackHitInPlaneIdeal = TMath::Sqrt(TMath::Power(r.at(0) - pIdeal.at(0),2) + TMath::Power(r.at(1) - pIdeal.at(1),2));
	fHM->H1("fhDistanceIdeal")->Fill(distToExtrapTrackHitInPlaneIdeal);
	cout << "Distance between fitted ring center and extrapolated track hit in plane = " << distToExtrapTrackHitInPlaneIdeal << endl << endl;
	//}
	//else { cout << "No identical ring mother ID and mirror track ID ..." << endl;}
}

/*void CbmRichMirrorSortingCorrection::FillHistProjection(TVector3 outPosIdeal, TVector3 outPosUnCorr, TVector3 outPos, CbmRichRingLight ringL, vector<Double_t> normalPMT, Double_t constantePMT)
{
	Double_t ringCenter[] = {0, 0, 0}, distToExtrapTrackHit = 0, distToExtrapTrackHitInPlane = 0, distToExtrapTrackHitInPlaneUnCorr = 0, distToExtrapTrackHitInPlaneIdeal = 0;

	ringCenter[0] = ringL.GetCenterX();
	ringCenter[1] = ringL.GetCenterY();
	ringCenter[2] = -1*((normalPMT.at(0)*ringCenter[0] + normalPMT.at(1)*ringCenter[1] + constantePMT)/normalPMT.at(2));

	vector<Double_t> r(3), p(3); // Absolute coordinates of fitted ring Center r and PMT extrapolated point p
	r.at(0) = TMath::Abs(ringCenter[0]), r.at(1) = TMath::Abs(ringCenter[1]), r.at(2) = TMath::Abs(ringCenter[2]);
	p.at(0) = TMath::Abs(outPos.x()), p.at(1) = TMath::Abs(outPos.y()), p.at(2) = TMath::Abs(outPos.z());
	cout << "Ring center coordinates = {" << ringCenter[0] << ", " << ringCenter[1] << ", " << ringCenter[2] << "}" << endl;
	cout << "Difference in X = " << TMath::Abs(r.at(0) - p.at(0)) << "; \t" << "Difference in Y = " << TMath::Abs(r.at(1) - p.at(1)) << "; \t" << "Difference in Z = " << TMath::Abs(r.at(2) - p.at(2)) << endl;
	fHM->H1("fhDifferenceX")->Fill(TMath::Abs(r.at(0) - p.at(0)));
	fHM->H1("fhDifferenceY")->Fill(TMath::Abs(r.at(1) - p.at(1)));
	distToExtrapTrackHit = TMath::Sqrt(TMath::Power(r.at(0) - p.at(0),2) + TMath::Power(r.at(1) - p.at(1),2) + TMath::Power(r.at(2) - p.at(2),2));
	distToExtrapTrackHitInPlane = TMath::Sqrt(TMath::Power(r.at(0) - p.at(0),2) + TMath::Power(r.at(1) - p.at(1),2));
	fHM->H1("fhDistanceCenterToExtrapolatedTrack")->Fill(distToExtrapTrackHit);
	fHM->H1("fhDistanceCorrected")->Fill(distToExtrapTrackHitInPlane);
	//cout << "Distance between fitted ring center and extrapolated track hit = " << distToExtrapTrackHit << endl;
	cout << "Distance between fitted ring center and extrapolated track hit in plane = " << distToExtrapTrackHitInPlane << endl;

	vector<Double_t> pUnCorr(3); // Absolute coordinates of fitted ring Center r and PMT extrapolated point p
	pUnCorr.at(0) = TMath::Abs(outPosUnCorr.x()), pUnCorr.at(1) = TMath::Abs(outPosUnCorr.y()), pUnCorr.at(2) = TMath::Abs(outPosUnCorr.z());
	//cout << "Ring center coordinates = {" << ringCenter[0] << ", " << ringCenter[1] << ", " << ringCenter[2] << "}" << endl;
	cout << "Difference in X w/o correction = " << TMath::Abs(r.at(0) - pUnCorr.at(0)) << "; \t" << "Difference in Y = " << TMath::Abs(r.at(1) - pUnCorr.at(1)) << "; \t" << "Difference in Z = " << TMath::Abs(r.at(2) - pUnCorr.at(2)) << endl;
	fHM->H1("fhDifferenceXUncorrected")->Fill(TMath::Abs(r.at(0) - pUnCorr.at(0)));
	fHM->H1("fhDifferenceYUncorrected")->Fill(TMath::Abs(r.at(1) - pUnCorr.at(1)));
	distToExtrapTrackHitInPlaneUnCorr = TMath::Sqrt(TMath::Power(r.at(0) - pUnCorr.at(0),2) + TMath::Power(r.at(1) - pUnCorr.at(1),2));
	fHM->H1("fhDistanceUncorrected")->Fill(distToExtrapTrackHitInPlaneUnCorr);
	cout << "Distance between fitted ring center and extrapolated track hit in plane = " << distToExtrapTrackHitInPlaneUnCorr << endl;

	vector<Double_t> pIdeal(3); // Absolute coordinates of fitted ring Center r and PMT extrapolated point p
	pIdeal.at(0) = TMath::Abs(outPosIdeal.x()), pIdeal.at(1) = TMath::Abs(outPosIdeal.y()), pIdeal.at(2) = TMath::Abs(outPosIdeal.z());
	//cout << "Ring center coordinates = {" << ringCenter[0] << ", " << ringCenter[1] << ", " << ringCenter[2] << "}" << endl;
	cout << "Difference in X w/ ideal correction = " << TMath::Abs(r.at(0) - pIdeal.at(0)) << "; \t" << "Difference in Y = " << TMath::Abs(r.at(1) - pIdeal.at(1)) << "; \t" << "Difference in Z = " << TMath::Abs(r.at(2) - pIdeal.at(2)) << endl;
	fHM->H1("fhDifferenceXIdeal")->Fill(TMath::Abs(r.at(0) - pIdeal.at(0)));
	fHM->H1("fhDifferenceYIdeal")->Fill(TMath::Abs(r.at(1) - pIdeal.at(1)));
	distToExtrapTrackHitInPlaneIdeal = TMath::Sqrt(TMath::Power(r.at(0) - pIdeal.at(0),2) + TMath::Power(r.at(1) - pIdeal.at(1),2));
	fHM->H1("fhDistanceIdeal")->Fill(distToExtrapTrackHitInPlaneIdeal);
	cout << "Distance between fitted ring center and extrapolated track hit in plane = " << distToExtrapTrackHitInPlaneIdeal << endl << endl;
	//}
	//else { cout << "No identical ring mother ID and mirror track ID ..." << endl;}
}


void CbmRichMirrorSortingCorrection::DrawHistProjection()
{
	char leg[128];
	int colorInd = 1;
	string diffCorrX = "DiffX_mirror_tile_2_8", diffCorrY = "DiffY_mirror_tile_2_8";

	TCanvas* can1 = new TCanvas("Distance_Histos_Difference_X", "Distance_Histos_Difference_X", 1500, 400);
	can1->SetGrid(1,1);
	can1->Divide(3,1);
	can1->cd(1);
	fHM->H1("fhDifferenceXIdeal")->Draw();
	fHM->H1("fhDifferenceXIdeal")->SetTitle("Difference in X ideal");
	fHM->H1("fhDifferenceXIdeal")->SetLineColor(kBlue);
	fHM->H1("fhDifferenceXIdeal")->SetLineWidth(2);
	can1->cd(2);
	fDiffHistoMap[diffCorrX]->Draw();
	fDiffHistoMap[diffCorrX]->SetTitle("Difference in X corrected");
	fDiffHistoMap[diffCorrX]->SetLineColor(kRed);
	fDiffHistoMap[diffCorrX]->SetLineWidth(2);
	can1->cd(3);
	fHM->H1("fhDifferenceXUncorrected")->Draw();
	fHM->H1("fhDifferenceXUncorrected")->SetTitle("Difference in X uncorrected");
	fHM->H1("fhDifferenceXUncorrected")->SetLineColor(kGreen);
	fHM->H1("fhDifferenceXUncorrected")->SetLineWidth(2);

	TCanvas* can2 = new TCanvas("Distance_Histos_Difference_Y", "Distance_Histos_Difference_Y", 1500, 400);
	can2->SetGrid(1,1);
	can2->Divide(3,1);
	can2->cd(1);
	fHM->H1("fhDifferenceYIdeal")->Draw();
	fHM->H1("fhDifferenceYIdeal")->SetTitle("Difference in Y ideal");
	fHM->H1("fhDifferenceYIdeal")->SetLineColor(kBlue);
	fHM->H1("fhDifferenceYIdeal")->SetLineWidth(2);
	can2->cd(2);
	fDiffHistoMap[diffCorrY]->Draw();
	fDiffHistoMap[diffCorrY]->SetTitle("Difference in Y corrected");
	fDiffHistoMap[diffCorrY]->SetLineColor(kRed);
	fDiffHistoMap[diffCorrY]->SetLineWidth(2);
	can2->cd(3);
	fHM->H1("fhDifferenceYUncorrected")->Draw();
	fHM->H1("fhDifferenceYUncorrected")->SetTitle("Difference in Y uncorrected");
	fHM->H1("fhDifferenceYUncorrected")->SetLineColor(kGreen);
	fHM->H1("fhDifferenceYUncorrected")->SetLineWidth(2);

	/*can3->SetGrid(1,1);
	can3->Divide(2,1);
	can3->cd(1)->SetGrid(1,1);
	can3->cd(2)->SetGrid(1,1);
	can3->cd(1);
	TH1D* Clone1 = (TH1D*)fHM->H1("fhDifferenceXIdeal")->Clone();
	Clone1->GetXaxis()->SetTitleSize(0.04);
	Clone1->GetYaxis()->SetTitleSize(0.04);
	Clone1->GetXaxis()->SetLabelSize(0.03);
	Clone1->GetYaxis()->SetLabelSize(0.03);
	Clone1->GetXaxis()->CenterTitle();
	Clone1->GetYaxis()->CenterTitle();
	Clone1->SetTitle("Difference in X");
	Clone1->SetLineColor(kBlue);
	Clone1->SetLineWidth(2);
	Clone1->Rebin(2);
	Clone1->Draw();
	TH1D* Clone2 = (TH1D*)fHM->H1("fhDifferenceX")->Clone();
	Clone2->SetTitleSize(0.04);
	Clone2->SetLabelSize(0.03);
	Clone2->SetLineColor(kGreen);
	Clone2->SetLineWidth(2);
	Clone2->Rebin(2);
	Clone2->Draw("same");
	TH1D* Clone3 = (TH1D*)fHM->H1("fhDifferenceXUncorrected")->Clone();
	Clone3->SetTitleSize(0.04);
	Clone3->SetLabelSize(0.03);
	Clone3->SetLineColor(kRed);
	Clone3->SetLineWidth(2);
	Clone3->Rebin(2);
	Clone3->Draw("same");
	gStyle->Clear();

	TLegend* LEG = new TLegend(0.3,0.78,0.5,0.88); // Set legend position
	LEG->SetBorderSize(1);
	LEG->SetFillColor(0);
	LEG->SetMargin(0.2);
	LEG->SetTextSize(0.02);
	sprintf(leg, "X diff uncorr");
	LEG->AddEntry(Clone3, leg, "l");
	sprintf(leg, "X diff corr");
	LEG->AddEntry(Clone2, leg, "l");
	sprintf(leg, "X diff ideal");
	LEG->AddEntry(Clone1, leg, "l");
	LEG->Draw();

	can3->cd(2);
	can3->SetGrid(1,1);
	TH1D* Clone4 = (TH1D*)fHM->H1("fhDifferenceYIdeal")->Clone();
	Clone4->GetXaxis()->SetTitleSize(0.04);
	Clone4->GetYaxis()->SetTitleSize(0.04);
	Clone4->GetXaxis()->SetLabelSize(0.03);
	Clone4->GetYaxis()->SetLabelSize(0.03);
	Clone4->GetXaxis()->CenterTitle();
	Clone4->GetYaxis()->CenterTitle();
	Clone4->SetTitle("Difference in Y");
	Clone4->SetLineColor(kBlue);
	Clone4->SetLineWidth(2);
	Clone4->Rebin(2);
	Clone4->Draw();
	TH1D* Clone5 = (TH1D*)fHM->H1("fhDifferenceY")->Clone();
	Clone5->SetTitleSize(0.04);
	Clone5->SetLabelSize(0.03);
	Clone5->SetLineColor(kGreen);
	Clone5->SetLineWidth(2);
	Clone5->Rebin(2);
	Clone5->Draw("same");
	TH1D* Clone6 = (TH1D*)fHM->H1("fhDifferenceYUncorrected")->Clone();
	Clone6->SetTitleSize(0.04);
	Clone6->SetLabelSize(0.03);
	Clone6->SetLineColor(kRed);
	Clone6->SetLineWidth(2);
	Clone6->Rebin(2);
	Clone6->Draw("same");

	TLegend* LEG1 = new TLegend(0.3,0.78,0.5,0.88); // Set legend position
	LEG1->SetBorderSize(1);
	LEG1->SetFillColor(0);
	LEG1->SetMargin(0.2);
	LEG1->SetTextSize(0.02);
	sprintf(leg, "Y diff uncorr");
	LEG1->AddEntry(Clone6, leg, "l");
	sprintf(leg, "Y diff corr");
	LEG1->AddEntry(Clone5, leg, "l");
	sprintf(leg, "Y diff ideal");
	LEG1->AddEntry(Clone4, leg, "l");
	LEG1->Draw();

	/*can3->cd(3);
	//DrawH1(list_of(fHM->H1("fhDistanceCorrected"))(fHM->H1("fhDistanceUncorrected"))(fHM->H1("fhDistanceIdeal")), list_of("a corrected")("a uncorrected")("a ideal"), kLinear, kLog, true, 0.7, 0.7, 0.99, 0.99);
	TH1D* CloneDist1 = (TH1D*)fHM->H1("fhDistanceIdeal")->Clone();
	CloneDist1->GetXaxis()->SetTitleSize(0.04);
	CloneDist1->GetYaxis()->SetTitleSize(0.04);
	CloneDist1->GetXaxis()->SetLabelSize(0.03);
	CloneDist1->GetYaxis()->SetLabelSize(0.03);
	CloneDist1->GetXaxis()->CenterTitle();
	CloneDist1->GetYaxis()->CenterTitle();
	CloneDist1->SetTitle("Distance between extrapolated track center and fitted ring center");
	CloneDist1->SetLineColor(kBlue);
	CloneDist1->SetLineWidth(2);
	CloneDist1->Rebin(2);
	CloneDist1->Draw();
	TH1D* CloneDist2 = (TH1D*)fHM->H1("fhDistanceUncorrected")->Clone();
	CloneDist2->SetTitleSize(0.04);
	CloneDist2->SetTitleSize(0.04);
	CloneDist2->SetLineColor(kRed);
	CloneDist2->SetLineWidth(2);
	CloneDist2->Rebin(2);
	CloneDist2->Draw("same");
	TH1D* CloneDist3 = (TH1D*)fHM->H1("fhDistanceCorrected")->Clone();
	CloneDist3->SetTitleSize(0.04);
	CloneDist3->SetTitleSize(0.04);
	CloneDist3->SetLineColor(kGreen);
	CloneDist3->SetLineWidth(2);
	CloneDist3->Rebin(2);
	CloneDist3->Draw("same");

	TLegend* LEG2 = new TLegend(0.3,0.78,0.5,0.88); // Set legend position
	LEG2->SetBorderSize(1);
	LEG2->SetFillColor(0);
	LEG2->SetMargin(0.2);
	LEG2->SetTextSize(0.02);
	sprintf(leg, "Distance uncorr");
	LEG2->AddEntry(CloneDist2, leg, "l");
	sprintf(leg, "Distance corr");
	LEG2->AddEntry(CloneDist3, leg, "l");
	sprintf(leg, "Distance ideal");
	LEG2->AddEntry(CloneDist1, leg, "l");
	LEG2->Draw();*/
/*	gStyle->SetOptStat(000000);

	Cbm::SaveCanvasAsImage(can1, string(fOutputDir.Data()), "png");
	Cbm::SaveCanvasAsImage(can2, string(fOutputDir.Data()), "png");
}
*/
