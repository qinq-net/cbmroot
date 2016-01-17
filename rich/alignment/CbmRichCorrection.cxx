// ---------- Original Headers ---------- //
#include "CbmRichCorrection.h"
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
#include "CbmRichHitProducer.h"
#include "CbmGlobalTrack.h"

#include "TLorentzVector.h"
#include "TVirtualMC.h"
#include "TGeoSphere.h"
class TGeoNode;
class TGeoVolume;
class TGeoShape;
class TGeoMatrix;

CbmRichCorrection::CbmRichCorrection() :
		FairTask(),
		fRichHits(NULL),
		fRichRings(NULL),
		fRichProjections(NULL),
		fRichMirrorPoints(NULL),
		fRichMCPoints(NULL),
		fMCTracks(NULL),
		fRichRingMatches(NULL),
		fRichRefPlanePoints(NULL),
		fRichPoints(NULL),
		fGlobalTracks(NULL),
		fHM(NULL),
		fGP(),
		fEventNum(0),
		fOutputDir(""),
		fRunTitle(""),
		fDrawHist(kFALSE),
		fCopFit(NULL),
		fTauFit(NULL),
		fPathsMap(),
		fPathsMapEllipse()
{
	fCounterMapping = 0.;
	fMirrCounter = 0.;
	for (int i=0;i<3;i++) {
		fArray[i]=0.;
	}
}

CbmRichCorrection::~CbmRichCorrection() {
}

InitStatus CbmRichCorrection::Init()
{
	FairRootManager* manager = FairRootManager::Instance();

	fRichHits = (TClonesArray*) manager->GetObject("RichHit");
	if (NULL == fRichHits) { Fatal("CbmRichCorrection::Init", "No RichHit array !"); }

	fRichRings = (TClonesArray*) manager->GetObject("RichRing");
	if (NULL == fRichRings) { Fatal("CbmRichCorrection::Init", "No RichRing array !"); }

	fRichProjections = (TClonesArray*) manager->GetObject("RichProjection");
	if (NULL == fRichProjections) { Fatal("CbmRichCorrection::Init", "No RichProjection array !"); }

	fRichMirrorPoints = (TClonesArray*) manager->GetObject("RichMirrorPoint");
	if (NULL == fRichMirrorPoints) { Fatal("CbmRichCorrection::Init", "No RichMirrorPoints array !"); }

	fRichMCPoints = (TClonesArray*) manager->GetObject("RichPoint");
	if (NULL == fRichMCPoints) { Fatal("CbmRichCorrection::Init", "No RichMCPoints array !"); }

	fMCTracks = (TClonesArray*) manager->GetObject("MCTrack");
	if (NULL == fMCTracks) { Fatal("CbmRichCorrection::Init", "No MCTracks array !"); }

	fRichRingMatches = (TClonesArray*) manager->GetObject("RichRingMatch");
	if (NULL == fRichRingMatches) { Fatal("CbmRichCorrection::Init", "No RichRingMatches array !"); }

	fRichRefPlanePoints  = (TClonesArray*) manager->GetObject("RefPlanePoint");
	if (NULL == fRichRefPlanePoints) { Fatal("CbmRichCorrection::Init", "No RichRefPlanePoint array !"); }

	fRichPoints = (TClonesArray*) manager->GetObject("RichPoint");
	if (NULL == fRichPoints) { Fatal("CbmRichCorrection::Init", "No RichPoint array !"); }

	fGlobalTracks = (TClonesArray*) manager->GetObject("GlobalTrack");
	if (NULL == fGlobalTracks) { Fatal("CbmAnaDielectronTask::Init","No GlobalTrack array!"); }

	fPathsMap["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip3_126/RICH_mirror_2_53"] = "2_53";
	fPathsMap["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip3_126/RICH_mirror_2_52"] = "2_52";
	fPathsMap["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip3_126/RICH_mirror_1_51"] = "1_51";
	fPathsMap["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip5_128/RICH_mirror_2_77"] = "2_77";
	fPathsMap["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip5_128/RICH_mirror_2_76"] = "2_76";
	fPathsMap["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip5_128/RICH_mirror_1_75"] = "1_75";
	fPathsMap["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip1_124/RICH_mirror_2_29"] = "2_29";
	fPathsMap["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip1_124/RICH_mirror_2_28"] = "2_28";
	fPathsMap["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip1_124/RICH_mirror_1_27"] = "1_27";
	fPathsMap["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip_cut_123/RICH_mirror_3_15"] = "3_15";
	fPathsMap["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip_cut_123/RICH_mirror_2_16"] = "2_16";
	fPathsMap["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip_cut_123/RICH_mirror_2_17"] = "2_17";

	fPathsMapEllipse["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip3_126/RICH_mirror_2_53"] = "2_53";
	fPathsMapEllipse["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip3_126/RICH_mirror_2_52"] = "2_52";
	fPathsMapEllipse["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip3_126/RICH_mirror_1_51"] = "1_51";
	fPathsMapEllipse["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip5_128/RICH_mirror_2_77"] = "2_77";
	fPathsMapEllipse["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip5_128/RICH_mirror_2_76"] = "2_76";
	fPathsMapEllipse["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip5_128/RICH_mirror_1_75"] = "1_75";
	fPathsMapEllipse["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip1_124/RICH_mirror_2_29"] = "2_29";
	fPathsMapEllipse["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip1_124/RICH_mirror_2_28"] = "2_28";
	fPathsMapEllipse["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip1_124/RICH_mirror_1_27"] = "1_27";
	fPathsMapEllipse["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip_cut_123/RICH_mirror_3_15"] = "3_15";
	fPathsMapEllipse["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip_cut_123/RICH_mirror_2_16"] = "2_16";
	fPathsMapEllipse["/cave_1/rich1_0/RICH_gas_221/RICH_mirror_half_total_208/RICH_mirror_and_support_belt_strip_cut_123/RICH_mirror_2_17"] = "2_17";

	fCopFit = new CbmRichRingFitterCOP();
	fTauFit = new CbmRichRingFitterEllipseTau();
	CbmRichConverter::Init();

	InitHist();

	return kSUCCESS;
}

void CbmRichCorrection::InitHist()
{
	fHM = new CbmHistManager();
	for (std::map<string,string>::iterator it=fPathsMap.begin(); it!=fPathsMap.end(); ++it) {		// Initialize all the histograms, using map IDs as inputs.
		string name = "fHMCPoints_" + it->second;														// it->first gives the paths; it->second gives the ID.
		fHM->Create2<TH2D>(name, name + ";X_Axis [];Y_Axis [];Entries", 2001, -100., 100.,2001, 60., 210.);
	}

	for (std::map<string,string>::iterator it=fPathsMapEllipse.begin(); it!=fPathsMapEllipse.end(); ++it) {
		string name = "fHPoints_Ellipse_" + it->second;
		fHM->Create2<TH2D>(name, name + ";X_Axis [];Y_Axis [];Entries", 2001, -100., 100.,2001, 60., 210.);
	}

	fHM->Create1<TH1D>("fhDistanceCenterToExtrapolatedTrack", "fhDistanceCenterToExtrapolatedTrack;Distance fitted center to extrapolated track;Number of entries", 750, 0., 20.);
//	fHM->Create1<TH1D>("fhDistanceCenterToExtrapolatedTrackInPlane", "fhDistanceCenterToExtrapolatedTrack;Distance fitted center to extrapolated track;Number of entries", 400, 0., 50.);
	fHM->Create1<TH1D>("fhDistanceCenterToExtrapolatedTrackInPlane", "fhDistanceCenterToExtrapolatedTrackInPlane;Distance fitted center to extrapolated track plane;Number of entries", 750, 0., 10.);
	fHM->Create1<TH1D>("fhDifferenceX", "fhDifferenceX;Difference in X (fitted center - extrapolated track);Number of entries", 750, 0., 10.);
	fHM->Create1<TH1D>("fhDifferenceY", "fhDifferenceY;Difference in Y (fitted center - extrapolated track);Number of entries", 750, 0., 10.);
}

void CbmRichCorrection::Exec(Option_t* option)
{
	cout << endl << "//--------------------------------------------------------------------------------------------------------------//" << endl;
	cout << "//---------------------------------------- EXEC Function - Defining the entries ----------------------------------------//" << endl;
	cout << "//----------------------------------------------------------------------------------------------------------------------//" << endl;
	fEventNum++;
	//LOG(DEBUG2) << "CbmRichCorrection : Event #" << fEventNum << FairLogger::endl;
	cout << "CbmRichCorrection : Event #" << fEventNum << endl;

	Int_t nofRingsInEvent = fRichRings->GetEntries();
	Int_t nofMirrorPoints = fRichMirrorPoints->GetEntries();
	Int_t nofHitsInEvent = fRichHits->GetEntries();
	Int_t NofMCPoints = fRichMCPoints->GetEntriesFast();
	Int_t NofMCTracks = fMCTracks->GetEntriesFast();
	cout << "Nb of rings in evt = " << nofRingsInEvent << ", nb of mirror points = " << nofMirrorPoints << ", nb of hits in evt = " << nofHitsInEvent << ", nb of Monte-Carlo points = " << NofMCPoints << " and nb of Monte-Carlo tracks = " << NofMCTracks << endl << endl;

	cout << "//----------------------------------------------------------------------------------------------------------//" << endl;
	cout << "//---------------------------------------- EXEC Function ---------------------------------------------------//" << endl;
	cout << "//------------------------------ Mapping for mirror - PMT relations ----------------------------------------//" << endl;
	cout << "//----------------------------------------------------------------------------------------------------------//" << endl << endl;

	TClonesArray* projectedPoint;
	if (nofRingsInEvent == 0) { cout << "Error no rings registered in event." << endl << endl; }
	else {
		//MatchFinder();
		fGP = CbmRichHitProducer::InitGeometry();
		fGP.Print();
		ProjectionProducer(projectedPoint);
	}
}

void CbmRichCorrection::MatchFinder()
{
	cout << "//---------------------------------------- MATCH_FINDER Function ----------------------------------------//" << endl;
	Int_t NofMirrorPoints = fRichMirrorPoints->GetEntries();
	Int_t NofRingsInEvent = fRichRings->GetEntries();
	Int_t NofMCPoints = fRichMCPoints->GetEntriesFast();
	Int_t NofMCTracks = fMCTracks->GetEntriesFast();
	//Int_t NofProjections = fRichProjections->GetEntries();

	Double_t x_Mirr = 0, y_Mirr = 0, z_Mirr = 0, x_PMT = 0, y_PMT = 0, z_PMT = 0;
	Double_t CenterX = 0, CenterY = 0;
	TGeoNode* mirr_node;
	const Char_t* mirr_path;
	UShort_t pHit;

	for (Int_t iMirr = 0; iMirr < NofMirrorPoints; iMirr++) {
		CbmRichPoint* MirrPoint = (CbmRichPoint*) fRichMirrorPoints->At(iMirr);
		Int_t trackID = MirrPoint->GetTrackID();
		//cout << "Track ID = " << trackID << endl;

		for (Int_t iMCPoint = 0; iMCPoint < NofMCPoints; iMCPoint++) {
			CbmRichPoint* pPoint = (CbmRichPoint*) fRichMCPoints->At(iMCPoint);
			if ( NULL == pPoint)
				continue;
			CbmMCTrack* pTrack = (CbmMCTrack*) fMCTracks->At(pPoint->GetTrackID());
			if ( NULL == pTrack)
				continue;
			Int_t gcode = pTrack->GetPdgCode();
			Int_t motherID = pTrack->GetMotherId();
			if (motherID == -1)
				continue;

			if (trackID == motherID) {
				//cout << "MATCH BETWEEN TRACK ID AND MOTHER ID FOUND !" << endl;
				//sleep(2);
				//cout << "TrackID from mirror point = " << trackID << " and mother ID from MC point = " << motherID << endl;
				x_Mirr = MirrPoint->GetX();
				y_Mirr = MirrPoint->GetY();
				z_Mirr = MirrPoint->GetZ();
				//cout << "x_Mirr: " << x_Mirr << ", y_Mirr: " << y_Mirr << " and z_Mirr: " << z_Mirr << endl;
				mirr_node = gGeoManager->FindNode(x_Mirr, y_Mirr, z_Mirr);
				mirr_path = gGeoManager->GetPath();
				FillPMTMap(mirr_path, pPoint);
				//break;
			}
		}

		// Loop filling the PMT map with ellipse points
		for (Int_t iRing = 0; iRing < NofRingsInEvent; iRing++) {
			CbmRichRing* ring = (CbmRichRing*) fRichRings->At(iRing);
			if ( NULL == ring)
				continue;
	    	CbmRichRingLight ringL;
	    	CbmRichConverter::CopyHitsToRingLight(ring, &ringL);
	    	//fCopFit->DoFit(&ringL);
	    	fTauFit->DoFit(&ringL);
	    	CenterX = ringL.GetCenterX();
	    	CenterY = ringL.GetCenterY();
	    	CbmTrackMatchNew* richRingMatch = static_cast<CbmTrackMatchNew*>(fRichRingMatches->At(iRing));
	    	Int_t richMCID = richRingMatch->GetMatchedLink().GetIndex();
			//Int_t trackID2 = ring->GetTrackID();
			//cout << "Track ID from ring = " << richMCID << endl;
			if (richMCID == -1)
				continue;

			if (trackID == richMCID) {

				cout << "MATCH BETWEEN TRACK_ID AND RICH_MC_ID FOUND !" << endl;
				cout << "Center X = " << CenterX << " and center Y = " << CenterY << endl;
				x_Mirr = MirrPoint->GetX();
				y_Mirr = MirrPoint->GetY();
				z_Mirr = MirrPoint->GetZ();
				cout << "x_Mirr: " << x_Mirr << ", y_Mirr: " << y_Mirr << " and z_Mirr: " << z_Mirr << endl;
				mirr_node = gGeoManager->FindNode(x_Mirr, y_Mirr, z_Mirr);
				mirr_path = gGeoManager->GetPath();
				cout << "Mirror path: " << mirr_path << endl;

				FillPMTMapEllipse(mirr_path, ringL.GetCenterX(), ringL.GetCenterY());
				//break;
			}
		}
	}
}

void CbmRichCorrection::FillPMTMap(const Char_t* mirr_path, CbmRichPoint* pPoint)
{
	//cout << "//---------------------------------------- FILL_PMT_MAP Function ----------------------------------------//" << endl;
	string name = string(mirr_path);
	for (std::map<string,string>::iterator it=fPathsMap.begin(); it!=fPathsMap.end(); ++it) {
		if ( name.compare(it->first) == 0 ) {
			//cout << "IDENTICAL PATHS FOUND !" << endl;
			//cout << "Mirror ID: " << it->second << endl;
			Double_t x_PMT = pPoint->GetX();
			Double_t y_PMT = pPoint->GetY();
			Double_t z_PMT = pPoint->GetZ();
			//cout << "x_PMT: " << x_PMT << ", y_PMT: " << y_PMT << " and z_PMT: " << z_PMT << endl << endl;
			//sleep(1);
			fHM->H2("fHMCPoints_" + it->second)->Fill(-x_PMT, y_PMT);
			fMirrCounter++;
		}
	}
}

void CbmRichCorrection::FillPMTMapEllipse(const Char_t* mirr_path, Float_t CenterX, Float_t CenterY)
{
	cout << "//---------------------------------------- FILL_PMT_MAP_ELLIPSE Function ----------------------------------------//" << endl;
	string name = string(mirr_path);
	for (std::map<string,string>::iterator it=fPathsMap.begin(); it!=fPathsMap.end(); ++it) {
		if ( name.compare(it->first) == 0 ) {
			cout << "IDENTICAL PATHS FOUND !" << endl;
			//sleep(2);
			cout << "Mirror ID: " << it->second << endl;
			//cout << "Center X: " << CenterX << " and Center Y: " << CenterY << endl << endl;
			fHM->H2("fHPoints_Ellipse_" + it->second)->Fill(-CenterX, CenterY);
		}
	}
	cout << endl;
}

void CbmRichCorrection::ProjectionProducer(TClonesArray* projectedPoint)
{
	cout << "//------------------------------ CbmRichCorrection: Projection Producer ------------------------------//" << endl << endl;

	Int_t NofMirrorPoints = fRichMirrorPoints->GetEntriesFast();
	Int_t NofRingsInEvent = fRichRings->GetEntries();
	Int_t NofGTracks = fGlobalTracks->GetEntriesFast();
	Int_t NofRefPlanePoints = fRichRefPlanePoints->GetEntriesFast();
	Int_t NofPMTPoints = fRichPoints->GetEntriesFast();

	projectedPoint->Clear();
	TMatrixFSym covMat(5);
	for (Int_t iMatrix = 0; iMatrix < 5; iMatrix++) {
		for (Int_t jMatrix = 0; jMatrix < 5; jMatrix++) {
			covMat(iMatrix,jMatrix) = 0;
		}
	}
	covMat(0,0) = covMat(1,1) = covMat(2,2) = covMat(3,3) = covMat(4,4) = 1.e-4;

	// Declarations of intermediate calculated variables.
	Double_t t1=0., t2=0., t3=0., k1=0., k2=0., checkCalc1=0., checkCalc2=0.;
	// Declaration of points coordinates.
	Double_t sphereRadius=0., constantePMT=0.;
	Double_t ptMirr[] = {0., 0., 0.}, ptC[] = {0., 0., 0.}, ptR1[] = {0., 0., 0.}, normalPMT[] = {0., 0., 0.}, normalMirr[] = {0., 0., 0.};
	Double_t ptR2Mirr[] = {0., 0., 0.}, ptR2Center[] = {0., 0., 0.}, ptPMirr[] = {0., 0., 0.}, ptPR2[] = {0., 0., 0.};
	Double_t reflectedPtCooVectSphereUnity[] = {0., 0., 0.};
	// Declaration of ring parameters.
	Double_t ringCenter[] = {0., 0., 0.}, distToExtrapTrackHit=0., distToExtrapTrackHitInPlane=0.;
	//Declarations related to geometry.
	Int_t mirrTrackID=-1, pmtTrackID=-1, refPlaneTrackID=-1, motherID=-100, pmtMotherID=-100;
	CbmMCTrack *track=NULL, *track2=NULL;
	TGeoNavigator *navi;
	TGeoNode *mirrNode;
	TGeoMatrix *mirrMatrix, *pmtMatrix, *richMatrix;

	GetPmtNormal(NofPMTPoints, normalPMT[0], normalPMT[1], normalPMT[2], constantePMT);
	cout << "Calculated normal vector to PMT plane = {" << normalPMT[0] << ", " << normalPMT[1] << ", " << normalPMT[2] << "} and constante d = " << constantePMT << endl << endl;

	for (Int_t iMirr = 0; iMirr < NofMirrorPoints; iMirr++) {
		//cout << "NofMirrorPoints = " << NofMirrorPoints << " and iMirr = " << iMirr << endl;
		CbmRichPoint *mirrPoint = (CbmRichPoint*) fRichMirrorPoints->At(iMirr);
		mirrTrackID = mirrPoint->GetTrackID();
		//cout << "Mirror track ID = " << mirrTrackID << endl;
		if(mirrTrackID <= -1) {
			cout << "Mirror track ID <= 1 !!!" << endl;
			cout << "----------------------------------- End of loop N°" << iMirr+1 << " on the mirror points. -----------------------------------" << endl << endl;
			continue;
		}
		track = (CbmMCTrack*) fMCTracks->At(mirrTrackID);
		motherID = track->GetMotherId();
		if (motherID == -1) {
			//cout << "Mirror motherID == -1 !!!" << endl << endl;
			ptMirr[0] = mirrPoint->GetX(), ptMirr[1] = mirrPoint->GetY(), ptMirr[2] = mirrPoint->GetZ();
			//cout << "Mirror Point coordinates; x = " << ptMirr[0] << ", y = " << ptMirr[1] << " and z = " << ptMirr[2] << endl;
			mirrNode = gGeoManager->FindNode(ptMirr[0],ptMirr[1],ptMirr[2]);
			if (mirrNode) {
				cout << "Mirror node found! Mirror node name = " << mirrNode->GetName() << endl;
				navi = gGeoManager->GetCurrentNavigator();
				cout << "Navigator path: " << navi->GetPath() << endl;
				cout << "Coordinates of sphere center: " << endl;
				navi->GetCurrentMatrix()->Print();
				ptC[0] = navi->GetCurrentMatrix()->GetTranslation()[0];
				ptC[1] = navi->GetCurrentMatrix()->GetTranslation()[1];
				ptC[2] = navi->GetCurrentMatrix()->GetTranslation()[2];
				cout << "Coordinates of tile center: " << endl;
				navi->GetMotherMatrix()->Print();
				cout << endl << "Sphere center coordinates of the rotated mirror tile = {" << ptC[0] << ", " << ptC[1] << ", " << ptC[2] << "} and sphere inner radius = " << sphereRadius  << endl;

				for (Int_t iRefl = 0; iRefl < NofRefPlanePoints; iRefl++) {
					new((*projectedPoint)[iRefl]) FairTrackParam(0., 0., 0., 0., 0., 0., covMat);
					CbmRichPoint *refPlanePoint = (CbmRichPoint*) fRichRefPlanePoints->At(iRefl);
					refPlaneTrackID = refPlanePoint->GetTrackID();
					//cout << "Reflective plane track ID = " << refPlaneTrackID << endl;
					if (mirrTrackID == refPlaneTrackID) {
						//cout << "IDENTICAL TRACK ID FOUND !!!" << endl << endl;
						ptR1[0] = refPlanePoint->GetX(), ptR1[1] = refPlanePoint->GetY(), ptR1[2] = refPlanePoint->GetZ();
						cout << "Reflective Plane Point coordinates = {" << ptR1[0] << ", " << ptR1[1] << ", " << ptR1[2] << "}" << endl;
						cout << "Mirror Point coordinates = {" << ptMirr[0] << ", " << ptMirr[1] << ", " << ptMirr[2] << "}" << endl << endl;
						normalMirr[0] = (ptC[0] - ptMirr[0])/TMath::Sqrt(TMath::Power(ptC[0] - ptMirr[0],2)+TMath::Power(ptC[1] - ptMirr[1],2)+TMath::Power(ptC[2] - ptMirr[2],2));
						normalMirr[1] = (ptC[1] - ptMirr[1])/TMath::Sqrt(TMath::Power(ptC[0] - ptMirr[0],2)+TMath::Power(ptC[1] - ptMirr[1],2)+TMath::Power(ptC[2] - ptMirr[2],2));
						normalMirr[2] = (ptC[2] - ptMirr[2])/TMath::Sqrt(TMath::Power(ptC[0] - ptMirr[0],2)+TMath::Power(ptC[1] - ptMirr[1],2)+TMath::Power(ptC[2] - ptMirr[2],2));
						cout << "Calculated and normalized normal of mirror tile = {" << normalMirr[0] << ", " << normalMirr[1] << ", " << normalMirr[2] << "}" << endl;

						t1 = ((ptR1[0]-ptMirr[0])*(ptC[0]-ptMirr[0]) + (ptR1[1]-ptMirr[1])*(ptC[1]-ptMirr[1]) + (ptR1[2]-ptMirr[2])*(ptC[2]-ptMirr[2]))/(TMath::Power(ptC[0]-ptMirr[0],2) + TMath::Power(ptC[1]-ptMirr[1],2) + TMath::Power(ptC[2]-ptMirr[2],2));
						ptR2Center[0] = 2*(ptMirr[0]+t1*(ptC[0]-ptMirr[0]))-ptR1[0];
						ptR2Center[1] = 2*(ptMirr[1]+t1*(ptC[1]-ptMirr[1]))-ptR1[1];
						ptR2Center[2] = 2*(ptMirr[2]+t1*(ptC[2]-ptMirr[2]))-ptR1[2];
						t2 = ((ptR1[0]-ptC[0])*(ptC[0]-ptMirr[0]) + (ptR1[1]-ptC[1])*(ptC[1]-ptMirr[1]) + (ptR1[2]-ptC[2])*(ptC[2]-ptMirr[2]))/(TMath::Power(ptC[0]-ptMirr[0],2) + TMath::Power(ptC[1]-ptMirr[1],2) + TMath::Power(ptC[2]-ptMirr[2],2));
						ptR2Mirr[0] = 2*(ptC[0]+t2*(ptC[0]-ptMirr[0]))-ptR1[0];
						ptR2Mirr[1] = 2*(ptC[1]+t2*(ptC[1]-ptMirr[1]))-ptR1[1];
						ptR2Mirr[2] = 2*(ptC[2]+t2*(ptC[2]-ptMirr[2]))-ptR1[2];
						/*//SAME AS calculation of t2 above
						t3 = ((ptR1[0]-ptC[0])*(ptC[0]-ptMirr[0]) + (ptR1[1]-ptC[1])*(ptC[1]-ptMirr[1]) + (ptR1[2]-ptC[2])*(ptC[2]-ptMirr[2]))/TMath::Sqrt(TMath::Power(ptC[0] - ptMirr[0],2)+TMath::Power(ptC[1] - ptMirr[1],2)+TMath::Power(ptC[2] - ptMirr[2],2));
						reflectedPtCooVectSphereUnity[0] = 2*(ptC[0]+t3*(normalMirr[0]))-ptR1[0];
						reflectedPtCooVectSphereUnity[1] = 2*(ptC[1]+t3*(normalMirr[1]))-ptR1[1];
						reflectedPtCooVectSphereUnity[2] = 2*(ptC[2]+t3*(normalMirr[2]))-ptR1[2];*/
						cout << "Coordinates of point R2 on reflective plane after reflection on the mirror tile:" << endl;
						cout << "* using mirror point M to define \U00000394: {" << ptR2Center[0] << ", " << ptR2Center[1] << ", " << ptR2Center[2] << "}" << endl;
						cout << "* using sphere center C to define \U00000394: {" << ptR2Mirr[0] << ", " << ptR2Mirr[1] << ", " << ptR2Mirr[2] << "}" << endl << endl;
						//cout << "Ref Pt Coo using unity Mirror-Sphere vector & sphere pt = {" << reflectedPtCooVectSphereUnity[0] << ", " << reflectedPtCooVectSphereUnity[1] << ", " << reflectedPtCooVectSphereUnity[2] << "}" << endl << endl;
						//cout << "NofPMTPoints = " << NofPMTPoints << endl;

						k1 = -1*((normalPMT[0]*ptMirr[0] + normalPMT[1]*ptMirr[1] + normalPMT[2]*ptMirr[2] + constantePMT)/(normalPMT[0]*(ptR2Mirr[0]-ptMirr[0]) + normalPMT[1]*(ptR2Mirr[1]-ptMirr[1]) + normalPMT[2]*(ptR2Mirr[2]-ptMirr[2])));
						ptPMirr[0] = ptMirr[0] + k1*(ptR2Mirr[0] - ptMirr[0]);
						ptPMirr[1] = ptMirr[1] + k1*(ptR2Mirr[1] - ptMirr[1]);
						ptPMirr[2] = ptMirr[2] + k1*(ptR2Mirr[2] - ptMirr[2]);
						k2 = -1*((normalPMT[0]*ptR2Mirr[0] + normalPMT[1]*ptR2Mirr[1] + normalPMT[2]*ptR2Mirr[2] + constantePMT)/(normalPMT[0]*(ptR2Mirr[0]-ptMirr[0]) + normalPMT[1]*(ptR2Mirr[1]-ptMirr[1]) + normalPMT[2]*(ptR2Mirr[2]-ptMirr[2])));
						ptPR2[0] = ptR2Mirr[0] + k2*(ptR2Mirr[0] - ptMirr[0]);
						ptPR2[1] = ptR2Mirr[1] + k2*(ptR2Mirr[1] - ptMirr[1]);
						ptPR2[2] = ptR2Mirr[2] + k2*(ptR2Mirr[2] - ptMirr[2]);
						cout << "Coordinates of point P on PMT plane, after reflection on the mirror tile and extrapolation to the PMT plane:" << endl;
						cout << "* using mirror point M to define \U0001D49F ': {" << ptPMirr[0] << ", " << ptPMirr[1] << ", " << ptPMirr[2] << "}" << endl;
						cout << "* using reflected point R2 to define \U0001D49F ': {" << ptPR2[0] << ", " << ptPR2[1] << ", " << ptPR2[2] << "}" << endl;
						checkCalc1 = ptPMirr[0]*normalPMT[0] + ptPMirr[1]*normalPMT[1] + ptPMirr[2]*normalPMT[2] + constantePMT;
						cout << "Check whether extrapolated track point on PMT plane verifies its equation (value should be 0.):" << endl;
						cout << "* using mirror point M, checkCalc = " << checkCalc1 << endl;
						checkCalc2 = ptPR2[0]*normalPMT[0] + ptPR2[1]*normalPMT[1] + ptPR2[2]*normalPMT[2] + constantePMT;
						cout << "* using reflected point R2, checkCalc = " << checkCalc2 << endl;

						TVector3 pmtVector(ptPMirr[0], ptPMirr[1], ptPMirr[2]);
						TVector3 pmtVectorNew;
						CbmRichHitProducer::TiltPoint(&pmtVector, &pmtVectorNew, fGP.fPmtPhi, fGP.fPmtTheta, fGP.fPmtZOrig);
						cout << "New coordinates of point P on PMT plane, after PMT plane rotation = {" << pmtVectorNew.X() << ", " << pmtVectorNew.Y() << ", " << pmtVectorNew.Z() << "}" << endl << endl;
						ptPMirr[0] = pmtVectorNew.X(), ptPMirr[1] = pmtVectorNew.Y(), ptPMirr[2] = pmtVectorNew.Z();

						/*for (Int_t iPmt = 0; iPmt < NofPMTPoints; iPmt++) {
						CbmRichPoint *pmtPoint = (CbmRichPoint*) fRichPoints->At(iPmt);
						pmtTrackID = pmtPoint->GetTrackID();
						CbmMCTrack* track3 = (CbmMCTrack*) fMCTracks->At(pmtTrackID);
						pmtMotherID = track3->GetMotherId();
						//cout << "pmt mother ID = " << pmtMotherID << endl;
						if (pmtMotherID == mirrTrackID) {
								ptP[0] = pmtPoint->GetX(), ptP[1] = pmtPoint->GetY(), ptP[2] = pmtPoint->GetZ();
								//cout << "Identical mirror track ID and PMT mother ID !!!" << endl;
								//cout << "PMT Point coordinates; x = " << pmtPoint->GetX() << ", y = " << pmtPoint->GetY() << " and z = " << pmtPoint->GetZ() << endl;
							}
					}
					cout << "Looking for PMT hits: end." << endl << endl;*/
						if (ptPMirr[0] > (-fGP.fPmtXOrig-fGP.fPmtWidthX) && ptPMirr[0] < (fGP.fPmtXOrig+fGP.fPmtWidthX)) {
							if (TMath::Abs(ptPMirr[1]) > (fGP.fPmtY-fGP.fPmtWidthY) && TMath::Abs(ptPMirr[1]) < (fGP.fPmtY+fGP.fPmtWidthY)) {
								FairTrackParam richtrack(ptPMirr[0], ptPMirr[1], ptPMirr[2], 0., 0., 0., covMat);
								*(FairTrackParam*)(projectedPoint->At(iRefl)) = richtrack;
							}
						}
					}
					//else { cout << "No identical track ID between mirror point and reflective plane point found ..." << endl << endl; }
				}

				/*for (Int_t iPmt = 0; iPmt < NofPMTPoints; iPmt++) {
					CbmRichPoint *pmtPoint = (CbmRichPoint*) fRichPoints->At(iPmt);
					cout << "PMT points = {" << pmtPoint->GetX() << ", " << pmtPoint->GetY() << ", " << pmtPoint->GetZ() << "}" << endl;
					TVector3 inputPoint(pmtPoint->GetX(), pmtPoint->GetY(), pmtPoint->GetZ());
					TVector3 outputPoint;
					CbmRichHitProducer::TiltPoint(&inputPoint, &outputPoint, fGP.fPmtPhi, fGP.fPmtTheta, fGP.fPmtZOrig);
					cout << "New PMT points after rotation of the PMT plane: " << endl;
					cout << outputPoint.X() << "\t" << outputPoint.Y() << "\t" << outputPoint.Z() << endl;
				}*/

				for (Int_t iGlobalTrack = 0; iGlobalTrack < NofGTracks; iGlobalTrack++) {
					//cout << "Nb of global tracks = " << NofGTracks << " and iGlobalTrack = " << iGlobalTrack << endl;
					CbmGlobalTrack* gTrack  = (CbmGlobalTrack*) fGlobalTracks->At(iGlobalTrack);
					if(NULL == gTrack) continue;
					Int_t richInd = gTrack->GetRichRingIndex();
				//cout << "Rich index = " << richInd << endl;
					if (richInd < 0) { continue; }
					CbmRichRing* ring = static_cast<CbmRichRing*>(fRichRings->At(richInd));
					if (NULL == ring) { continue; }
					Int_t ringTrackID = ring->GetTrackID();
					track2 = (CbmMCTrack*) fMCTracks->At(ringTrackID);
					Int_t ringMotherID = track2->GetMotherId();
					//cout << "Ring mother ID = " << ringMotherID << ", ring track ID = " << ringTrackID << " and track2 pdg = " << track2->GetPdgCode() << endl;
					CbmRichRingLight ringL;
					CbmRichConverter::CopyHitsToRingLight(ring, &ringL);
					//RotateAndCopyHitsToRingLight(ring, &ringL);
					fCopFit->DoFit(&ringL);
					fTauFit->DoFit(&ringL);
					ringCenter[0] = ringL.GetCenterX();
					ringCenter[1] = ringL.GetCenterY();
					ringCenter[2] = -1*((normalPMT[0]*ringCenter[0] + normalPMT[1]*ringCenter[1] + constantePMT)/normalPMT[2]);
					cout << "Ring center coordinates = {" << ringCenter[0] << ", " << ringCenter[1] << ", " << ringCenter[2] << "}" << endl;
					cout << "Difference in X = " << TMath::Abs(ringCenter[0] - ptPMirr[0]) << "\t" << "Difference in Y = " << TMath::Abs(ringCenter[1] - ptPMirr[1]) << "\t" << "Difference in Z = " << TMath::Abs(ringCenter[2] - ptPMirr[2]) << endl;
					fHM->H1("fhDifferenceX")->Fill(TMath::Abs(ringCenter[0]-ptPMirr[0]));
					fHM->H1("fhDifferenceY")->Fill(TMath::Abs(ringCenter[1]-ptPMirr[1]));
					distToExtrapTrackHit = TMath::Sqrt(TMath::Power(ringCenter[0]-ptPMirr[0],2) + TMath::Power(ringCenter[1]-ptPMirr[1],2) + TMath::Power(ringCenter[2]-ptPMirr[2],2));
					distToExtrapTrackHitInPlane = TMath::Sqrt(TMath::Power(ringCenter[0]-ptPMirr[0],2) + TMath::Power(ringCenter[1]-ptPMirr[1],2));
					fHM->H1("fhDistanceCenterToExtrapolatedTrack")->Fill(distToExtrapTrackHit);
					fHM->H1("fhDistanceCenterToExtrapolatedTrackInPlane")->Fill(distToExtrapTrackHitInPlane);
					cout << "Distance between fitted ring center and extrapolated track hit = " << distToExtrapTrackHit << endl;
					cout << "Distance between fitted ring center and extrapolated track hit in plane = " << distToExtrapTrackHitInPlane << endl << endl;
					//}
					//else { cout << "No identical ring mother ID and mirror track ID ..." << endl;}
				}
				cout << "End of loop on global tracks;" << endl;
			}
			else { cout << "Not a mother particle ..." << endl; }
			cout << "----------------------------------- " << "End of loop N°" << iMirr+1 << " on the mirror points." << " -----------------------------------" << endl << endl;
		}
	}
}

void CbmRichCorrection::GetPmtNormal(Int_t NofPMTPoints, Double_t &normalX, Double_t &normalY, Double_t &normalZ, Double_t &normalCste)
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
		CbmRichPoint *pmtPoint = (CbmRichPoint*) fRichPoints->At(iPmt);
		pmtTrackID = pmtPoint->GetTrackID();
		track = (CbmMCTrack*) fMCTracks->At(pmtTrackID);
		pmtMotherID = track->GetMotherId();
		a[0] = pmtPoint->GetX(), a[1] = pmtPoint->GetY(), a[2] = pmtPoint->GetZ();
		//cout << "a[0] = " << a[0] << ", a[1] = " << a[1] << " et a[2] = " << a[2] << endl;
		break;
	}
	for (Int_t iPmt = 0; iPmt < NofPMTPoints; iPmt++) {
		CbmRichPoint *pmtPoint = (CbmRichPoint*) fRichPoints->At(iPmt);
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
		CbmRichPoint *pmtPoint = (CbmRichPoint*) fRichPoints->At(iPmt);
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
		normalX = buffNormX/TMath::Sqrt(TMath::Power(buffNormX,2)+TMath::Power(buffNormY,2)+TMath::Power(buffNormZ,2));
		normalY = buffNormY/TMath::Sqrt(TMath::Power(buffNormX,2)+TMath::Power(buffNormY,2)+TMath::Power(buffNormZ,2));
		normalZ = buffNormZ/TMath::Sqrt(TMath::Power(buffNormX,2)+TMath::Power(buffNormY,2)+TMath::Power(buffNormZ,2));
	}

	CbmRichPoint *pmtPoint1 = (CbmRichPoint*) fRichPoints->At(20);
	scalarProd = normalX*(pmtPoint1->GetX()-a[0]) + normalY*(pmtPoint1->GetY()-a[1]) + normalZ*(pmtPoint1->GetZ()-a[2]);
	//cout << "1st scalar product between vectAM and normale = " << scalarProd << endl;
	// To determine the constant term of the plane equation, inject the coordinates of a pmt point, which should solve it: a*x+b*y+c*z+d=0.
	normalCste = -1*(normalX*pmtPoint1->GetX() + normalY*pmtPoint1->GetY() + normalZ*pmtPoint1->GetZ());
	CbmRichPoint *pmtPoint2 = (CbmRichPoint*) fRichPoints->At(15);
	scalarProd = normalX*(pmtPoint2->GetX()-a[0]) + normalY*(pmtPoint2->GetY()-a[1]) + normalZ*(pmtPoint2->GetZ()-a[2]);
	//cout << "2nd scalar product between vectAM and normale = " << scalarProd << endl;
	CbmRichPoint *pmtPoint3 = (CbmRichPoint*) fRichPoints->At(25);
	scalarProd = normalX*(pmtPoint3->GetX()-a[0]) + normalY*(pmtPoint3->GetY()-a[1]) + normalZ*(pmtPoint3->GetZ()-a[2]);
	//cout << "3nd scalar product between vectAM and normale = " << scalarProd << endl;
}

void CbmRichCorrection::RotateAndCopyHitsToRingLight(const CbmRichRing* ring1, CbmRichRingLight* ring2)
{
	Int_t nofHits = ring1->GetNofHits();

	for (Int_t i = 0; i < nofHits; i++) {
		Int_t hitInd = ring1->GetHit(i);
		CbmRichHit* hit = (CbmRichHit*) fRichHits->At(hitInd);
		if (NULL == hit) continue;
		TVector3 inputHit(hit->GetX(), hit->GetY(), hit->GetZ());
		TVector3 outputHit;
		CbmRichHitProducer::TiltPoint(&inputHit, &outputHit, fGP.fPmtPhi, fGP.fPmtTheta, fGP.fPmtZOrig);
		CbmRichHitLight hl(outputHit.X(), outputHit.Y());
		ring2->AddHit(hl);
	}
}

void CbmRichCorrection::DrawHist()
{
	TCanvas* can = new TCanvas(fRunTitle + "_Separated_Hits", fRunTitle + "_Separated_Hits", 1500, 900);
	can->SetGridx(1);
	can->SetGridy(1);
	can->Divide(4, 3);
	can->cd(9);
	DrawH2(fHM->H2("fHMCPoints_3_15"));
	can->cd(5);
	DrawH2(fHM->H2("fHMCPoints_2_16"));
	can->cd(1);
	DrawH2(fHM->H2("fHMCPoints_2_17"));
	can->cd(2);
	DrawH2(fHM->H2("fHMCPoints_2_29"));
	can->cd(3);
	DrawH2(fHM->H2("fHMCPoints_2_53"));
	can->cd(4);
	DrawH2(fHM->H2("fHMCPoints_2_77"));
	can->cd(6);
	DrawH2(fHM->H2("fHMCPoints_2_28"));
	can->cd(7);
	DrawH2(fHM->H2("fHMCPoints_2_52"));
	can->cd(8);
	DrawH2(fHM->H2("fHMCPoints_2_76"));
	can->cd(10);
	DrawH2(fHM->H2("fHMCPoints_1_27"));
	can->cd(11);
	DrawH2(fHM->H2("fHMCPoints_1_51"));
	can->cd(12);
	DrawH2(fHM->H2("fHMCPoints_1_75"));
	Cbm::SaveCanvasAsImage(can, string(fOutputDir.Data()), "png");

	TCanvas* can2 = new TCanvas(fRunTitle + "_Separated_Ellipse", fRunTitle + "_Separated_Ellipse", 1500, 900);
	can2->SetGridx(1);
	can2->SetGridy(1);
	can2->Divide(4, 3);
	can2->cd(9);
	DrawH2(fHM->H2("fHPoints_Ellipse_3_15"));
	can2->cd(5);
	DrawH2(fHM->H2("fHPoints_Ellipse_2_16"));
	can2->cd(1);
	DrawH2(fHM->H2("fHPoints_Ellipse_2_17"));
	can2->cd(2);
	DrawH2(fHM->H2("fHPoints_Ellipse_2_29"));
	can2->cd(3);
	DrawH2(fHM->H2("fHPoints_Ellipse_2_53"));
	can2->cd(4);
	DrawH2(fHM->H2("fHPoints_Ellipse_2_77"));
	can2->cd(6);
	DrawH2(fHM->H2("fHPoints_Ellipse_2_28"));
	can2->cd(7);
	DrawH2(fHM->H2("fHPoints_Ellipse_2_52"));
	can2->cd(8);
	DrawH2(fHM->H2("fHPoints_Ellipse_2_76"));
	can2->cd(10);
	DrawH2(fHM->H2("fHPoints_Ellipse_1_27"));
	can2->cd(11);
	DrawH2(fHM->H2("fHPoints_Ellipse_1_51"));
	can2->cd(12);
	DrawH2(fHM->H2("fHPoints_Ellipse_1_75"));
	Cbm::SaveCanvasAsImage(can2, string(fOutputDir.Data()), "png");

	TCanvas* can3 = new TCanvas(fRunTitle + "_Separated_Ellipse", fRunTitle + "_Separated_Ellipse", 1500, 900);
	can3->Divide(2,2);
	can3->cd(1);
	DrawH1(fHM->H1("fhDifferenceX"));
	can3->cd(2);
	DrawH1(fHM->H1("fhDifferenceY"));
	can3->cd(3);
	DrawH1(fHM->H1("fhDistanceCenterToExtrapolatedTrack"));
	can3->cd(4);
	DrawH1(fHM->H1("fhDistanceCenterToExtrapolatedTrackInPlane"));
}

void CbmRichCorrection::DrawHistFromFile(TString fileName)
{
	fHM = new CbmHistManager();
	TFile* file = new TFile(fileName, "READ");
	fHM->ReadFromFile(file);
	DrawHist();
}

void CbmRichCorrection::Finish()
{
	// ---------------------------------------------------------------------------------------------------------------------------------------- //
	// -------------------------------------------------- Mapping for mirror - PMT relations -------------------------------------------------- //
	// ---------------------------------------------------------------------------------------------------------------------------------------- //

	if (fDrawHist) {
		DrawHist();
	}
	cout << endl << "Mirror counter = " << fMirrCounter << endl;
	//cout << setprecision(6) << endl;
}
ClassImp(CbmRichCorrection)
