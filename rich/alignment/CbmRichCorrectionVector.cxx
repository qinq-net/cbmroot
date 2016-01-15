// ---------- Original Headers ---------- //
#include "CbmRichCorrectionVector.h"
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

//#include "TLorentzVector.h"
#include "TVirtualMC.h"
#include "TGeoSphere.h"
class TGeoNode;
class TGeoVolume;
class TGeoShape;
class TGeoMatrix;

CbmRichCorrectionVector::CbmRichCorrectionVector() :
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
		fIsMeanCenter(kFALSE),
		fCopFit(NULL),
		fTauFit(NULL),
		fPathsMap(),
		fPathsMapEllipse()
{
	fMirrCounter = 0.;
	for (int i=0;i<3;i++) {
		fArray[i]=0.;
	}
}

CbmRichCorrectionVector::~CbmRichCorrectionVector() {
}

InitStatus CbmRichCorrectionVector::Init()
{
	FairRootManager* manager = FairRootManager::Instance();

	fRichHits = (TClonesArray*) manager->GetObject("RichHit");
	if (NULL == fRichHits) { Fatal("CbmRichCorrectionVector::Init", "No RichHit array !"); }

	fRichRings = (TClonesArray*) manager->GetObject("RichRing");
	if (NULL == fRichRings) { Fatal("CbmRichCorrectionVector::Init", "No RichRing array !"); }

	fRichProjections = (TClonesArray*) manager->GetObject("RichProjection");
	if (NULL == fRichProjections) { Fatal("CbmRichCorrectionVector::Init", "No RichProjection array !"); }

	fRichMirrorPoints = (TClonesArray*) manager->GetObject("RichMirrorPoint");
	if (NULL == fRichMirrorPoints) { Fatal("CbmRichCorrectionVector::Init", "No RichMirrorPoints array !"); }

	fRichMCPoints = (TClonesArray*) manager->GetObject("RichPoint");
	if (NULL == fRichMCPoints) { Fatal("CbmRichCorrectionVector::Init", "No RichMCPoints array !"); }

	fMCTracks = (TClonesArray*) manager->GetObject("MCTrack");
	if (NULL == fMCTracks) { Fatal("CbmRichCorrectionVector::Init", "No MCTracks array !"); }

	fRichRingMatches = (TClonesArray*) manager->GetObject("RichRingMatch");
	if (NULL == fRichRingMatches) { Fatal("CbmRichCorrectionVector::Init", "No RichRingMatches array !"); }

	fRichRefPlanePoints  = (TClonesArray*) manager->GetObject("RefPlanePoint");
	if (NULL == fRichRefPlanePoints) { Fatal("CbmRichCorrectionVector::Init", "No RichRefPlanePoint array !"); }

	fRichPoints = (TClonesArray*) manager->GetObject("RichPoint");
	if (NULL == fRichPoints) { Fatal("CbmRichCorrectionVector::Init", "No RichPoint array !"); }

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

void CbmRichCorrectionVector::InitHist()
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

void CbmRichCorrectionVector::Exec(Option_t* option)
{
	cout << endl << "//--------------------------------------------------------------------------------------------------------------//" << endl;
	cout << "//---------------------------------------- EXEC Function - Defining the entries ----------------------------------------//" << endl;
	cout << "//----------------------------------------------------------------------------------------------------------------------//" << endl;
	fEventNum++;
	//LOG(DEBUG2) << "CbmRichCorrectionVector : Event #" << fEventNum << FairLogger::endl;
	cout << "CbmRichCorrectionVector : Event #" << fEventNum << endl;

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

void CbmRichCorrectionVector::MatchFinder()
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

void CbmRichCorrectionVector::FillPMTMap(const Char_t* mirr_path, CbmRichPoint* pPoint)
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

void CbmRichCorrectionVector::FillPMTMapEllipse(const Char_t* mirr_path, Float_t CenterX, Float_t CenterY)
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

void CbmRichCorrectionVector::ProjectionProducer(TClonesArray* projectedPoint)
{
	cout << "//------------------------------ CbmRichCorrectionVector: Projection Producer ------------------------------//" << endl << endl;

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

	// Declaration of points coordinates.
	Double_t sphereRadius=300., constantePMT=0.;
	vector<Double_t> ptM(3), ptC(3), ptR1(3), momR1(3), normalPMT(3), ptR2Mirr(3), ptR2Center(3), ptPMirr(3), ptPR2(3);
	Double_t reflectedPtCooVectSphereUnity[] = {0., 0., 0.};
	// Declaration of ring parameters.
	Double_t ringCenter[] = {0., 0., 0.}, distToExtrapTrackHit=0., distToExtrapTrackHitInPlane=0.;
	//Declarations related to geometry.
	Int_t mirrTrackID=-1, pmtTrackID=-1, refPlaneTrackID=-1, motherID=-100, pmtMotherID=-100;
	CbmMCTrack *track=NULL, *track2=NULL;
	TGeoNavigator *navi;
	TGeoNode *mirrNode;
	TGeoMatrix *mirrMatrix, *pmtMatrix, *richMatrix;

	GetPmtNormal(NofPMTPoints, normalPMT, constantePMT);
	cout << "Calculated normal vector to PMT plane = {" << normalPMT.at(0) << ", " << normalPMT.at(1) << ", " << normalPMT.at(2) << "} and constante d = " << constantePMT << endl << endl;

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
			ptM.at(0) = mirrPoint->GetX(), ptM.at(1) = mirrPoint->GetY(), ptM.at(2) = mirrPoint->GetZ();
			//cout << "Mirror Point coordinates; x = " << ptM.at(0) << ", y = " << ptM.at(1) << " and z = " << ptM.at(2) << endl;
			mirrNode = gGeoManager->FindNode(ptM.at(0),ptM.at(1),ptM.at(2));
			if (mirrNode) {
				cout << "Mirror node found! Mirror node name = " << mirrNode->GetName() << endl;
				navi = gGeoManager->GetCurrentNavigator();
				cout << "Navigator path: " << navi->GetPath() << endl;
				cout << "Coordinates of sphere center: " << endl;
				navi->GetCurrentMatrix()->Print();
				if(fIsMeanCenter) GetMeanSphereCenter(navi, ptC);	//IF NO INFORMATION ON MIRRORS ARE KNOWN (TO BE USED IN RECONSTRUCTION STEP) !!!
				else {
					ptC.at(0) = navi->GetCurrentMatrix()->GetTranslation()[0];
					ptC.at(1) = navi->GetCurrentMatrix()->GetTranslation()[1];
					ptC.at(2) = navi->GetCurrentMatrix()->GetTranslation()[2];
				}
				cout << "Coordinates of tile center: " << endl;
				navi->GetMotherMatrix()->Print();
				cout << endl << "Sphere center coordinates of the rotated mirror tile = {" << ptC.at(0) << ", " << ptC.at(1) << ", " << ptC.at(2) << "} and sphere inner radius = " << sphereRadius  << endl;

				for (Int_t iRefl = 0; iRefl < NofRefPlanePoints; iRefl++) {
					new((*projectedPoint)[iRefl]) FairTrackParam(0., 0., 0., 0., 0., 0., covMat);
					CbmRichPoint *refPlanePoint = (CbmRichPoint*) fRichRefPlanePoints->At(iRefl);
					refPlaneTrackID = refPlanePoint->GetTrackID();
					//cout << "Reflective plane track ID = " << refPlaneTrackID << endl;
					if (mirrTrackID == refPlaneTrackID) {
						//cout << "IDENTICAL TRACK ID FOUND !!!" << endl << endl;
						ptR1.at(0) = refPlanePoint->GetX(), ptR1.at(1) = refPlanePoint->GetY(), ptR1.at(2) = refPlanePoint->GetZ();
						momR1.at(0) = refPlanePoint->GetPx(), momR1.at(1) = refPlanePoint->GetPy(), momR1.at(2) = refPlanePoint->GetPz();
						cout << "Reflective Plane Point coordinates = {" << ptR1.at(0) << ", " << ptR1.at(1) << ", " << ptR1.at(2) << "}" << endl;
						cout << "And reflective Plane Point momenta = {" << momR1.at(0) << ", " << momR1.at(1) << ", " << momR1.at(2) << "}" << endl;
						cout << "Mirror Point coordinates = {" << ptM.at(0) << ", " << ptM.at(1) << ", " << ptM.at(2) << "}" << endl << endl;

						if (fIsMeanCenter) {
							GetMirrorIntersection(ptM, ptR1, momR1, ptC, sphereRadius);
							//From ptM: how to retrieve tile ID ???
						}

						ComputeR2(ptR2Center, ptR2Mirr, ptC, ptM, ptR1);

						ComputeP(ptPMirr, ptPR2, normalPMT, ptM, ptR2Mirr, constantePMT);

						TVector3 pmtVector(ptPMirr.at(0), ptPMirr.at(1), ptPMirr.at(2));
						TVector3 pmtVectorNew;
						CbmRichHitProducer::TiltPoint(&pmtVector, &pmtVectorNew, fGP.fPmtPhi, fGP.fPmtTheta, fGP.fPmtZOrig);
						cout << "New coordinates of point P on PMT plane, after PMT plane rotation = {" << pmtVectorNew.X() << ", " << pmtVectorNew.Y() << ", " << pmtVectorNew.Z() << "}" << endl << endl;
						ptPMirr.at(0) = pmtVectorNew.X(), ptPMirr.at(1) = pmtVectorNew.Y(), ptPMirr.at(2) = pmtVectorNew.Z();

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
						if (ptPMirr.at(0) > (-fGP.fPmtXOrig-fGP.fPmtWidthX) && ptPMirr.at(0) < (fGP.fPmtXOrig+fGP.fPmtWidthX)) {
							if (TMath::Abs(ptPMirr.at(1)) > (fGP.fPmtY-fGP.fPmtWidthY) && TMath::Abs(ptPMirr.at(1)) < (fGP.fPmtY+fGP.fPmtWidthY)) {
								FairTrackParam richtrack(ptPMirr.at(0), ptPMirr.at(1), ptPMirr.at(2), 0., 0., 0., covMat);
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
					ringCenter[2] = -1*((normalPMT.at(0)*ringCenter[0] + normalPMT.at(1)*ringCenter[1] + constantePMT)/normalPMT.at(2));
					cout << "Ring center coordinates = {" << ringCenter[0] << ", " << ringCenter[1] << ", " << ringCenter[2] << "}" << endl;
					cout << "Difference in X = " << TMath::Abs(ringCenter[0] - ptPMirr.at(0)) << "\t" << "Difference in Y = " << TMath::Abs(ringCenter[1] - ptPMirr.at(1)) << "\t" << "Difference in Z = " << TMath::Abs(ringCenter[2] - ptPMirr.at(2)) << endl;
					fHM->H1("fhDifferenceX")->Fill(TMath::Abs(ringCenter[0]-ptPMirr.at(0)));
					fHM->H1("fhDifferenceY")->Fill(TMath::Abs(ringCenter[1]-ptPMirr.at(1)));
					distToExtrapTrackHit = TMath::Sqrt(TMath::Power(ringCenter[0]-ptPMirr.at(0),2) + TMath::Power(ringCenter[1]-ptPMirr.at(1),2) + TMath::Power(ringCenter[2]-ptPMirr.at(2),2));
					distToExtrapTrackHitInPlane = TMath::Sqrt(TMath::Power(ringCenter[0]-ptPMirr.at(0),2) + TMath::Power(ringCenter[1]-ptPMirr.at(1),2));
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

void CbmRichCorrectionVector::GetPmtNormal(Int_t NofPMTPoints, vector<Double_t> &normalPMT, Double_t &normalCste)
{
	//cout << endl << "//------------------------------ CbmRichCorrectionVector: Calculate PMT Normal ------------------------------//" << endl << endl;

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
		normalPMT.at(0) = buffNormX/TMath::Sqrt(TMath::Power(buffNormX,2)+TMath::Power(buffNormY,2)+TMath::Power(buffNormZ,2));
		normalPMT.at(1) = buffNormY/TMath::Sqrt(TMath::Power(buffNormX,2)+TMath::Power(buffNormY,2)+TMath::Power(buffNormZ,2));
		normalPMT.at(2) = buffNormZ/TMath::Sqrt(TMath::Power(buffNormX,2)+TMath::Power(buffNormY,2)+TMath::Power(buffNormZ,2));
	}

	CbmRichPoint *pmtPoint1 = (CbmRichPoint*) fRichPoints->At(20);
	scalarProd = normalPMT.at(0)*(pmtPoint1->GetX()-a[0]) + normalPMT.at(1)*(pmtPoint1->GetY()-a[1]) + normalPMT.at(2)*(pmtPoint1->GetZ()-a[2]);
	//cout << "1st scalar product between vectAM and normale = " << scalarProd << endl;
	// To determine the constant term of the plane equation, inject the coordinates of a pmt point, which should solve it: a*x+b*y+c*z+d=0.
	normalCste = -1*(normalPMT.at(0)*pmtPoint1->GetX() + normalPMT.at(1)*pmtPoint1->GetY() + normalPMT.at(2)*pmtPoint1->GetZ());
	CbmRichPoint *pmtPoint2 = (CbmRichPoint*) fRichPoints->At(15);
	scalarProd = normalPMT.at(0)*(pmtPoint2->GetX()-a[0]) + normalPMT.at(1)*(pmtPoint2->GetY()-a[1]) + normalPMT.at(2)*(pmtPoint2->GetZ()-a[2]);
	//cout << "2nd scalar product between vectAM and normale = " << scalarProd << endl;
	CbmRichPoint *pmtPoint3 = (CbmRichPoint*) fRichPoints->At(25);
	scalarProd = normalPMT.at(0)*(pmtPoint3->GetX()-a[0]) + normalPMT.at(1)*(pmtPoint3->GetY()-a[1]) + normalPMT.at(2)*(pmtPoint3->GetZ()-a[2]);
	//cout << "3nd scalar product between vectAM and normale = " << scalarProd << endl;
}

void CbmRichCorrectionVector::GetMeanSphereCenter(TGeoNavigator *navi, vector<Double_t> &ptC)
{
	const Char_t *topNodePath;
	topNodePath = gGeoManager->GetTopNode()->GetName();
	cout << "Top node path: " << topNodePath << endl;
	TGeoVolume *rootTop;
	rootTop = gGeoManager->GetTopVolume();
	rootTop->Print();

	TGeoIterator nextNode(rootTop);
	TGeoNode *curNode;
	const TGeoMatrix *curMatrix;
	const Double_t* curNodeTranslation; // 3 components - pointers to some memory which is provided by ROOT
	const Double_t* curNodeRotationM; // 9 components - pointers to some memory which is provided by ROOT
	TString filterName0("mirror_tile_type0");
	TString filterName1("mirror_tile_type1");
	TString filterName2("mirror_tile_type2");
	TString filterName3("mirror_tile_type3");
	TString filterName4("mirror_tile_type4");
	TString filterName5("mirror_tile_type5");
	Int_t counter = 0;
	Double_t sphereXTot=0.,sphereYTot=0., sphereZTot=0.;

	while ((curNode=nextNode())) {
		TString nodeName(curNode->GetName());
		TString nodePath;

		// Filter using volume name, not node name
		// But you can do 'if (nodeName.Contains("filter"))'
		if (curNode->GetVolume()->GetName() == filterName0 || curNode->GetVolume()->GetName() == filterName1 ||	curNode->GetVolume()->GetName() == filterName2 || curNode->GetVolume()->GetName() == filterName3 ||	curNode->GetVolume()->GetName() == filterName4 || curNode->GetVolume()->GetName() == filterName5 ) {
			if (curNode->GetNdaughters() == 0) {
				// All deepest nodes of mirror tiles here (leaves)
				// Thus we get spherical surface centers
				nextNode.GetPath(nodePath);
	            curMatrix = nextNode.GetCurrentMatrix();
	            curNodeTranslation = curMatrix->GetTranslation();
	            curNodeRotationM = curMatrix->GetRotationMatrix();
	            printf ("%s tr:\t", nodePath.Data());
	            printf ("%08f\t%08f\t%08f\t\n", curNodeTranslation[0], curNodeTranslation[1], curNodeTranslation[2]);
	            if (curNodeTranslation[1] > 0) {			// CONDITION FOR UPPER MIRROR WALL STUDY
	            	sphereXTot+=curNodeTranslation[0];
		            sphereYTot+=curNodeTranslation[1];
		            sphereZTot+=curNodeTranslation[2];
		            counter++;
	            }
			}
		}
	}
	ptC.at(0) = sphereXTot/counter;
	ptC.at(1) = sphereYTot/counter;
	ptC.at(2) = sphereZTot/counter;

	counter = 0;
	nextNode.Reset();
}

void CbmRichCorrectionVector::GetMirrorIntersection(vector<Double_t> &ptM, vector<Double_t> ptR1, vector<Double_t> momR1, vector<Double_t> ptC, Double_t sphereRadius)
{
	Double_t a=0., b=0., c=0., d=0., k0=0., k1=0., k2=0.;

	a = TMath::Power(momR1.at(0),2) + TMath::Power(momR1.at(1),2) + TMath::Power(momR1.at(2),2);
	b = 2 * (momR1.at(0) * (ptR1.at(0) - ptC.at(0)) + momR1.at(1) * (ptR1.at(1) - ptC.at(1)) + momR1.at(2) * (ptR1.at(2) - ptC.at(2)));
	c = TMath::Power(ptR1.at(0) - ptC.at(0),2) + TMath::Power(ptR1.at(1) - ptC.at(1),2) + TMath::Power(ptR1.at(2) - ptC.at(2),2) - TMath::Power(sphereRadius,2);
	d = b*b - 4*a*c;
	cout << "d = " << d << endl;

	if (d < 0) {
		cout << "Error no solution to degree 2 equation found ; discriminant below 0." << endl;
		ptM.at(0) = 0., ptM.at(1) = 0., ptM.at(2) = 0.;
	}
	else if (d == 0) {
		cout << "One solution to degree 2 equation found." << endl;
		k0 = -b/(2*a);
		ptM.at(0) = ptR1.at(0) + k0*momR1.at(0);
		ptM.at(1) = ptR1.at(1) + k0*momR1.at(1);
		ptM.at(2) = ptR1.at(2) + k0*momR1.at(2);
	}
	else if (d > 0) {
		cout << "Two solutions to degree 2 equation found." << endl;
		k1 = ((-b - TMath::Sqrt(d)) / (2*a));
		k2 = ((-b + TMath::Sqrt(d)) / (2*a));

		if (ptR1.at(2) + k1*momR1.at(2) > ptR1.at(2) + k2*momR1.at(2)) {
			ptM.at(0) = ptR1.at(0) + k1*momR1.at(0);
			ptM.at(1) = ptR1.at(1) + k1*momR1.at(1);
			ptM.at(2) = ptR1.at(2) + k1*momR1.at(2);
		}
		else if (ptR1.at(2) + k1*momR1.at(2) < ptR1.at(2) + k2*momR1.at(2)) {
			ptM.at(0) = ptR1.at(0) + k2*momR1.at(0);
			ptM.at(1) = ptR1.at(1) + k2*momR1.at(1);
			ptM.at(2) = ptR1.at(2) + k2*momR1.at(2);
		}
	}
}

void CbmRichCorrectionVector::ComputeR2(vector<Double_t> &ptR2Center, vector<Double_t> &ptR2Mirr, vector<Double_t> ptM, vector<Double_t> ptC, vector<Double_t> ptR1)
{
	vector<Double_t> normalMirr(3);
	Double_t t1=0., t2=0., t3=0.;

	normalMirr.at(0) = (ptC.at(0) - ptM.at(0))/TMath::Sqrt(TMath::Power(ptC.at(0) - ptM.at(0),2)+TMath::Power(ptC.at(1) - ptM.at(1),2)+TMath::Power(ptC.at(2) - ptM.at(2),2));
	normalMirr.at(1) = (ptC.at(1) - ptM.at(1))/TMath::Sqrt(TMath::Power(ptC.at(0) - ptM.at(0),2)+TMath::Power(ptC.at(1) - ptM.at(1),2)+TMath::Power(ptC.at(2) - ptM.at(2),2));
	normalMirr.at(2) = (ptC.at(2) - ptM.at(2))/TMath::Sqrt(TMath::Power(ptC.at(0) - ptM.at(0),2)+TMath::Power(ptC.at(1) - ptM.at(1),2)+TMath::Power(ptC.at(2) - ptM.at(2),2));
	cout << "Calculated and normalized normal of mirror tile = {" << normalMirr.at(0) << ", " << normalMirr.at(1) << ", " << normalMirr.at(2) << "}" << endl;

	t1 = ((ptR1.at(0)-ptM.at(0))*(ptC.at(0)-ptM.at(0)) + (ptR1.at(1)-ptM.at(1))*(ptC.at(1)-ptM.at(1)) + (ptR1.at(2)-ptM.at(2))*(ptC.at(2)-ptM.at(2)))/(TMath::Power(ptC.at(0)-ptM.at(0),2) + TMath::Power(ptC.at(1)-ptM.at(1),2) + TMath::Power(ptC.at(2)-ptM.at(2),2));
	ptR2Center.at(0) = 2*(ptM.at(0)+t1*(ptC.at(0)-ptM.at(0)))-ptR1.at(0);
	ptR2Center.at(1) = 2*(ptM.at(1)+t1*(ptC.at(1)-ptM.at(1)))-ptR1.at(1);
	ptR2Center.at(2) = 2*(ptM.at(2)+t1*(ptC.at(2)-ptM.at(2)))-ptR1.at(2);
	t2 = ((ptR1.at(0)-ptC.at(0))*(ptC.at(0)-ptM.at(0)) + (ptR1.at(1)-ptC.at(1))*(ptC.at(1)-ptM.at(1)) + (ptR1.at(2)-ptC.at(2))*(ptC.at(2)-ptM.at(2)))/(TMath::Power(ptC.at(0)-ptM.at(0),2) + TMath::Power(ptC.at(1)-ptM.at(1),2) + TMath::Power(ptC.at(2)-ptM.at(2),2));
	ptR2Mirr.at(0) = 2*(ptC.at(0)+t2*(ptC.at(0)-ptM.at(0)))-ptR1.at(0);
	ptR2Mirr.at(1) = 2*(ptC.at(1)+t2*(ptC.at(1)-ptM.at(1)))-ptR1.at(1);
	ptR2Mirr.at(2) = 2*(ptC.at(2)+t2*(ptC.at(2)-ptM.at(2)))-ptR1.at(2);
	/*//SAME AS calculation of t2 above
	t3 = ((ptR1.at(0)-ptC.at(0))*(ptC.at(0)-ptM.at(0)) + (ptR1.at(1)-ptC.at(1))*(ptC.at(1)-ptM.at(1)) + (ptR1.at(2)-ptC.at(2))*(ptC.at(2)-ptM.at(2)))/TMath::Sqrt(TMath::Power(ptC.at(0) - ptM.at(0),2)+TMath::Power(ptC.at(1) - ptM.at(1),2)+TMath::Power(ptC.at(2) - ptM.at(2),2));
	reflectedPtCooVectSphereUnity[0] = 2*(ptC.at(0)+t3*(normalMirr.at(0)))-ptR1.at(0);
	reflectedPtCooVectSphereUnity[1] = 2*(ptC.at(1)+t3*(normalMirr.at(1)))-ptR1.at(1);
	reflectedPtCooVectSphereUnity[2] = 2*(ptC.at(2)+t3*(normalMirr.at(2)))-ptR1.at(2);*/
	cout << "Coordinates of point R2 on reflective plane after reflection on the mirror tile:" << endl;
	cout << "* using mirror point M to define \U00000394: {" << ptR2Center.at(0) << ", " << ptR2Center.at(1) << ", " << ptR2Center.at(2) << "}" << endl;
	cout << "* using sphere center C to define \U00000394: {" << ptR2Mirr.at(0) << ", " << ptR2Mirr.at(1) << ", " << ptR2Mirr.at(2) << "}" << endl << endl;
	//cout << "Ref Pt Coo using unity Mirror-Sphere vector & sphere pt = {" << reflectedPtCooVectSphereUnity[0] << ", " << reflectedPtCooVectSphereUnity[1] << ", " << reflectedPtCooVectSphereUnity[2] << "}" << endl << endl;
	//cout << "NofPMTPoints = " << NofPMTPoints << endl;
}

void CbmRichCorrectionVector::ComputeP(vector<Double_t> &ptPMirr, vector<Double_t> &ptPR2, vector<Double_t> normalPMT, vector<Double_t> ptM, vector<Double_t> ptR2Mirr, Double_t constantePMT)
{
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
	cout << "* using reflected point R2 to define \U0001D49F ': {" << ptPR2.at(0) << ", " << ptPR2.at(1) << ", " << ptPR2.at(2) << "}" << endl;
	checkCalc1 = ptPMirr.at(0)*normalPMT.at(0) + ptPMirr.at(1)*normalPMT.at(1) + ptPMirr.at(2)*normalPMT.at(2) + constantePMT;
	cout << "Check whether extrapolated track point on PMT plane verifies its equation (value should be 0.):" << endl;
	cout << "* using mirror point M, checkCalc = " << checkCalc1 << endl;
	checkCalc2 = ptPR2.at(0)*normalPMT.at(0) + ptPR2.at(1)*normalPMT.at(1) + ptPR2.at(2)*normalPMT.at(2) + constantePMT;
	cout << "* using reflected point R2, checkCalc = " << checkCalc2 << endl;
}

void CbmRichCorrectionVector::RotateAndCopyHitsToRingLight(const CbmRichRing* ring1, CbmRichRingLight* ring2)
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

void CbmRichCorrectionVector::DrawHist()
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

void CbmRichCorrectionVector::DrawHistFromFile(TString fileName)
{
	fHM = new CbmHistManager();
	TFile* file = new TFile(fileName, "READ");
	fHM->ReadFromFile(file);
	DrawHist();
}

void CbmRichCorrectionVector::Finish()
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
ClassImp(CbmRichCorrectionVector)
