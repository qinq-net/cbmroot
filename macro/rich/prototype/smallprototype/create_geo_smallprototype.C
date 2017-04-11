#include <iostream>
using namespace std; 



void create_geo_smallprototype()
{
	gSystem->Load("libGeom");
	//gGeoMan = gGeoManager;// (TGeoManager*)gROOT->FindObject("FAIRGeom");
	//new TGeoManager ("Testbox", "Testbox");

	TString geoFileName= "/Users/slebedev/Development/cbm/trunk/cbmroot/geometry/rich/prototype/Testbox.geo.root";

	FairGeoLoader*    geoLoad = new FairGeoLoader("TGeo","FairGeoLoader");
	FairGeoInterface* geoFace = geoLoad->getGeoInterface();
	TString geoPath = gSystem->Getenv("VMCWORKDIR");
	TString medFile = geoPath + "/geometry/media.geo";

	geoFace->setMediaFile(medFile);
	geoFace->readMedia();
	gGeoMan = gGeoManager;

	FairGeoMedia*   geoMedia = geoFace->getMedia();
	FairGeoBuilder* geoBuild = geoLoad->getGeoBuilder();

	//Media
	FairGeoMedium* mCoating = geoMedia->getMedium("RICHglass");
	if (mCoating == NULL) Fatal("Main", "FairMedium RICHglass not found");
	geoBuild->createMedium(mCoating);
	TGeoMedium* medCoating = gGeoMan->GetMedium("RICHglass");
	if (medCoating == NULL) Fatal("Main", "Medium RICHglass not found");

	FairGeoMedium* mAluminium = geoMedia->getMedium("aluminium");
	if (mAluminium == NULL) Fatal("Main", "FairMedium aluminium not found");
	geoBuild->createMedium(mAluminium);
	TGeoMedium* medAl = gGeoMan->GetMedium("aluminium");
	if (medAl == NULL) Fatal("Main", "Medium vacuum not found");

	FairGeoMedium* mVacuum = geoMedia->getMedium("vacuum");
	if (mVacuum == NULL) Fatal("Main", "FairMedium vacuum not found");
	geoBuild->createMedium(mVacuum);
	TGeoMedium* medVacuum = gGeoMan->GetMedium("vacuum");
	if (medVacuum == NULL) Fatal("Main", "Medium vacuum not found");

	FairGeoMedium* mNitrogen = geoMedia->getMedium("RICHgas_N2_dis");
	if (mNitrogen == NULL) Fatal("Main", "FairMedium RICHgas_N2_dis not found");
	geoBuild->createMedium(mNitrogen);
	TGeoMedium* medNitrogen = gGeoMan->GetMedium("RICHgas_N2_dis");
	if (medNitrogen == NULL) Fatal("Main", "Medium RICHgas_N2_dis not found");

	FairGeoMedium* mPMT = geoMedia->getMedium("CsI");
	if (mPMT == NULL) Fatal("Main", "FairMedium CsI not found");
	geoBuild->createMedium(mPMT);
	TGeoMedium* medCsI = gGeoMan->GetMedium("CsI");
	if (medCsI == NULL) Fatal("Main", "Medium CsI not found");

	FairGeoMedium* mLenseGlass = geoMedia->getMedium("Rich_NBK7_glass");
	if (mLenseGlass == NULL) Fatal("Main", "FairMedium Rich_NBK7_glass not found");
	geoBuild->createMedium(mLenseGlass);
	TGeoMedium* medLenseGlass = gGeoMan->GetMedium("Rich_NBK7_glass");
	if (medLenseGlass == NULL) Fatal("Main", "Medium Rich_NBK7_glass  not found");

	FairGeoMedium* mElectronic = geoMedia->getMedium("air");
	if (mElectronic == NULL) Fatal("Main", "FairMedium air not found");
	geoBuild->createMedium(mElectronic);
	TGeoMedium* medElectronic = gGeoMan->GetMedium("air");
	if (medElectronic == NULL) Fatal("Main", "Medium air not found");

//	FairGeoMedium* mQuarz = geoMedia->getMedium("quarz");
//	if (mQuarz == NULL) Fatal("Main", "FairMedium quarz not found");
//	geoBuild->createMedium(mQuarz);
//	TGeoMedium* medQuarz = gGeoMan->GetMedium("quarz");
//	if (medQuarz == NULL) Fatal("Main", "Medium quarz not found");

	//Dimensions of the RICH prototype [cm]
	// Box
	const Double_t boxLength = 44;
	const Double_t boxWidth = 20;
	const Double_t boxHeight = 30;
	const Double_t wallWidth = 0.3;

	// PMT
	// PMT specs https://www.hamamatsu.com/resources/pdf/etd/H12700_TPMH1348E.pdf
	const Double_t pmtNofPixels = 8;
	const Double_t pmtSize = 5.2;
	const Double_t pmtSizeHalf = pmtSize / 2.;
	const Double_t pmtSensitiveSize = 4.85;
	const Double_t pmtPixelSize = pmtSensitiveSize / pmtNofPixels;
	const Double_t pmtPixelSizeHalf = pmtPixelSize / 2.;
	const Double_t pmtGap = 0.1;
	const Double_t pmtGapHalf = pmtGap / 2.;
	const Double_t pmtPadding = 0.175;
	const Double_t pmtMatrixGap = 3.5;

	// Electronics
	const Double_t elecLength = 10.;
	const Double_t elecWidth = 3. * pmtSize;
	const Double_t elecHeight = 2. * pmtSize;

	// Lense
	const Double_t lenseCThickness = 2.44;
	const Double_t lenseRadius = 15.51;
	const Double_t lensePmtDistance = 3.0;
	const Double_t lenseCoating = 0.1;

	// Absorber
	const Double_t absorberThickness = 0.1;
	const Double_t absorberRadius = 2.0;

	// Quarz plate can be exchanged with lense
	const Double_t quarzThickness = 0.1;
	const Double_t quarzWidth = 5.0;
	const Double_t quarzHeight = 5.0;

	// Imaginary sensitive plane
	const Double_t sensPlaneSize = 200.;
	const Double_t sensPlaneBoxDistance = 1.;

	TGeoRotation *rotBox= new TGeoRotation("rotBox", 0., 0., 0.);
	TGeoTranslation *trCave= new TGeoTranslation(0., 0., 0.);

	TGeoTranslation *trBox= new TGeoTranslation(0., 0., 0.); //Gasbox/Box Translation
	TGeoTranslation *trSensPlane= new TGeoTranslation(0., 0., boxLength / 2. + sensPlaneBoxDistance);

	Double_t pmtPlaneZ = -(lenseRadius - lenseCThickness - lensePmtDistance);
	Double_t pmtPlaneY = pmtSize + pmtMatrixGap/2. + pmtGap/2.;
	TGeoTranslation *trPmtPlaneUp = new TGeoTranslation(0., pmtPlaneY, pmtPlaneZ);
	TGeoTranslation *trPmtPlaneDown = new TGeoTranslation(0., -pmtPlaneY, pmtPlaneZ);

	TGeoTranslation *trPmt1 = new TGeoTranslation(-pmtSize - pmtGap,  pmtSizeHalf + pmtGapHalf, 0.);
	TGeoTranslation *trPmt2 = new TGeoTranslation(0., pmtSizeHalf + pmtGapHalf, 0.);
	TGeoTranslation *trPmt3 = new TGeoTranslation( pmtSize + pmtGap, pmtSizeHalf + pmtGapHalf, 0.);
	TGeoTranslation *trPmt4 = new TGeoTranslation(-pmtSize - pmtGap, -pmtSizeHalf - pmtGapHalf, 0.);
	TGeoTranslation *trPmt5 = new TGeoTranslation(0., -pmtSizeHalf - pmtGapHalf, 0.);
	TGeoTranslation *trPmt6 = new TGeoTranslation( pmtSize + pmtGap, -pmtSizeHalf - pmtGapHalf, 0.);

	//TGeoTranslation *trelec1= new TGeoTranslation(0., pmtsize+0.5 , -(lenseradius-centerthickness-lensepmtdistance)+eleclength/2+0.1);
	//TGeoTranslation *trelec2= new TGeoTranslation(0., -(pmtsize+0.5) ,-(lenseradius-centerthickness-lensepmtdistance)+eleclength/2+0.1);

	TGeoTranslation *trAbsorber= new TGeoTranslation(0., 0., -(lenseRadius - lenseCThickness) + absorberThickness/2);

	TGeoTranslation *trLense= new TGeoTranslation(0., 0., 0.);
	TGeoTranslation *trComp= new TGeoTranslation("trComp", 0., 0., -(lenseRadius - lenseCThickness) / 2 + 0.1);
	trComp->RegisterYourself();

	//Virtual Topbox
	TGeoVolume* topVol = new TGeoVolumeAssembly("rich");
	gGeoMan->SetTopVolume(topVol);

	TGeoVolume *caveVol = gGeoMan->MakeBox("Cave", medNitrogen, 300., 300., 300.);
	TGeoVolume *boxVol = gGeoMan->MakeBox("Box", medNitrogen, boxWidth/2., boxHeight/2., boxLength/2.);
	TGeoVolume *gasVol = gGeoMan->MakeBox("Gas", medNitrogen , boxWidth/2 - wallWidth, boxHeight/2 - wallWidth, boxLength/2 - wallWidth);
	TGeoVolume *pmtContVol = gGeoMan->MakeBox("PmtContainer", medCsI , 3 * pmtSizeHalf + pmtGap, 2 * pmtSizeHalf + pmtGap / 2., 0.1);
	TGeoVolume *pmtVol = gGeoMan->MakeBox("Pmt", medCsI , pmtSizeHalf, pmtSizeHalf, 0.1);
	TGeoVolume *pmtPixelVol = gGeoMan->MakeBox("pmt_pixel", medCsI, pmtPixelSize/2., pmtPixelSize/2., 0.1);
	//TGeoVolume *elecVol = gGeoMan->MakeBox("elec", medElectronic , elecwidth/2, elecheight/2, eleclength/2);

	// Lense composite shape
	TGeoSphere *lenseCoatingVol = new TGeoSphere("LenseCoating", lenseRadius, lenseRadius + lenseCoating, 90., 180., 0., 360.);
	TGeoSphere *lenseVol = new TGeoSphere("Lense", 0., lenseRadius, 90., 180., 0., 360.);
	TGeoBBox *lenseCut = new TGeoBBox("LenseCutShape", lenseRadius + lenseCoating, lenseRadius + lenseCoating, (lenseRadius - lenseCThickness)/2);
	TGeoCompositeShape *lenseCompShape = new TGeoCompositeShape("LenseCompShape", "Lense-LenseCutShape:trComp");
	TGeoCompositeShape *lenseCoatingCompShape = new TGeoCompositeShape("LenseCoatingCompShape", "LenseCoating-LenseCutShape:trComp");
	TGeoVolume *lenseCompVol = new TGeoVolume("LenseComp", lenseCompShape, medLenseGlass);
	TGeoVolume *lenseCoatingCompVol = new TGeoVolume("LenseCoatingComp", lenseCoatingCompShape, medCoating);

	TGeoVolume *absorberVol = gGeoMan->MakeTube("Absorber", medAl, 0., absorberRadius, absorberThickness/2);
	TGeoVolume *sensPlaneVol = gGeoMan->MakeBox("SensPlane", medCsI, sensPlaneSize/2 , sensPlaneSize/2  , 0.1);

	// Quarz plate
	//TGeoVolume *quarzVol = gGeoMan->MakeBox("QuarzPlate", medQuarz , quarzWidth / 2., quarzHeight / 2., quarzThickness / 2.);

	//Positioning
	topVol->AddNode(caveVol, 1, trCave);

	caveVol->AddNode(boxVol, 1, trBox);
	caveVol->AddNode(sensPlaneVol, 1, trSensPlane);

	boxVol->AddNode(gasVol, 1, rotBox);

	gasVol->AddNode(lenseCoatingCompVol, 1, trLense);
	gasVol->AddNode(lenseCompVol, 1, trLense);
	gasVol->AddNode(pmtContVol, 1, trPmtPlaneUp);
	gasVol->AddNode(pmtContVol, 2, trPmtPlaneDown);
	//gas->AddNode(elec, 1, trelec1);
	//gas->AddNode(elec, 2, trelec2);

	lenseCompVol->AddNode(absorberVol, 1, trAbsorber);

	pmtContVol->AddNode(pmtVol, 1, trPmt1);
	pmtContVol->AddNode(pmtVol, 2, trPmt2);
	pmtContVol->AddNode(pmtVol, 3, trPmt3);
	pmtContVol->AddNode(pmtVol, 4, trPmt4);
	pmtContVol->AddNode(pmtVol, 5, trPmt5);
	pmtContVol->AddNode(pmtVol, 6, trPmt6);

	Int_t halfPmtNofPixels = pmtNofPixels / 2;
	for(Int_t i = 0; i < pmtNofPixels; i++) {
		for(Int_t j = 0; j < pmtNofPixels; j++) {
			Double_t x = (i - (halfPmtNofPixels - 1)) * pmtPixelSize - pmtPixelSizeHalf;
			Double_t y = (j - (halfPmtNofPixels - 1)) * pmtPixelSize - pmtPixelSizeHalf;
			cout << "x:" << x << "  ";
			TGeoTranslation *trij = new TGeoTranslation(x, y, 0.);
			pmtVol->AddNode(pmtPixelVol, pmtNofPixels * i + j + 1, trij);
		}	
		cout << endl;
	}	

	gGeoMan->CheckOverlaps();
	gGeoMan->PrintOverlaps();

	//Draw
	lenseCoatingCompVol->SetLineColor(kCyan);
	lenseCompVol->SetLineColor(kGreen);
	boxVol->SetLineColor(kBlack);
	gasVol->SetLineColor(kBlue);
	pmtVol->SetLineColor(kOrange);
	pmtPixelVol->SetLineColor(kYellow+4);
	//gGeoMan->SetTopVisible();
	//boxVol->SetVisibility(false);

	//topVol->Draw("ogl");
	boxVol->Draw("ogl");

	TFile* geoFile = new TFile(geoFileName, "RECREATE");
	topVol->Write();
	cout << endl << "Geometry written to " << geoFileName << endl;
	geoFile->Close();


}
