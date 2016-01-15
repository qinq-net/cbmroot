void Import_GDML_Export_ROOT()
{
	TString vmcDir = TString(gSystem->Getenv("VMCWORKDIR"));

	TGeoManager *gdml = new TGeoManager("gdml", "FAIRGeom");

	TGDMLParse parser;
	// Define your input GDML file HERE
	TGeoVolume* gdmlTop = parser.GDMLReadFile (vmcDir + "/geometry/rich/rich_v14c_array_no_belts_and_small_frame.gdml");
	TGeoVolume* rootTop = new TGeoVolumeAssembly("TOP");

	gGeoManager->SetTopVolume(rootTop);

	// Define your position HERE
	TGeoRotation* rot = new TGeoRotation ("rot", 0., 0., 0.);
	TGeoCombiTrans* posrot = new TGeoCombiTrans (0., 0., 270., rot);

	rootTop->AddNode (gdmlTop, 1, posrot);

	gGeoManager->CloseGeometry();
	gGeoManager->CheckOverlaps();
/*
	// Define your output ROOT file HERE
	TFile* outfile = new TFile(vmcDir + "/geometry/rich/rich_v13c_pipe_1_al_1.root", "RECREATE");
	rootTop->Write();
	outfile->Close();
*/
}

