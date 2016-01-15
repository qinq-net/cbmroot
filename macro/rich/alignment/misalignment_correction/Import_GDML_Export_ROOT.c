void Import_GDML_Export_ROOT()
{
	TString vmcDir = TString(gSystem->Getenv("VMCWORKDIR"));

	TGeoManager *gdml = new TGeoManager("gdml", "FAIRGeom");

	TGDMLParse parser;
	// Define your input GDML file HERE
	//TGeoVolume* gdmlTop = parser.GDMLReadFile (vmcDir + "/geometry/rich/alignment/misalignment_correction/Study_OneMirr/X_axis/Deg/rich_v14b_misaligned_Mirr3_2_2deg_X.gdml");
//        TGeoVolume* gdmlTop = parser.GDMLReadFile (vmcDir + "/geometry/rich/alignment/misalignment_correction/PMTMapping/rich_v14b_Mirr_mis_+2X_151.gdml");
//        TGeoVolume* gdmlTop = parser.GDMLReadFile (vmcDir + "/geometry/rich/alignment/misalignment_correction/PMTMapping/rich_v14b_misaligned_pmtNotRot.gdml");
        TGeoVolume* gdmlTop = parser.GDMLReadFile (vmcDir + "/geometry/rich/alignment/misalignment_correction/v15_rot/Rich_dec_2015_misalign.gdml");
	TGeoVolume* rootTop = new TGeoVolumeAssembly("TOP");

	gGeoManager->SetTopVolume(rootTop);

	// Define your position HERE
	TGeoRotation* rot = new TGeoRotation ("rot", 0., 0., 0.);
	TGeoCombiTrans* posrot = new TGeoCombiTrans (0., 0., 270., rot);

	rootTop->AddNode (gdmlTop, 1, posrot);

//	gGeoManager->CheckGeometryFull();
	gGeoManager->CloseGeometry();
	gGeoManager->CheckOverlaps();
	gGeoManager->PrintOverlaps();

	// Define you output ROOT file HERE
	//TFile* outfile = new TFile(vmcDir + "/geometry/rich/alignment/misalignment_correction/Study_OneMirr/X_axis/Deg/rich_v14b_misaligned_Mirr3_2_2deg_X.root", "RECREATE");
//        TFile* outfile = new TFile(vmcDir + "/geometry/rich/alignment/misalignment_correction/PMTMapping/rich_v14b_Mirr_mis_+2X_151.root", "RECREATE");
//        TFile* outfile = new TFile(vmcDir + "/geometry/rich/alignment/misalignment_correction/PMTMapping/rich_v14b_misaligned_pmtRot.root", "RECREATE");
        TFile* outfile = new TFile(vmcDir + "/geometry/rich/alignment/misalignment_correction/v15_rot/Rich_dec_2015_misalign.root", "RECREATE");
	rootTop->Write();
	outfile->Close();
}

