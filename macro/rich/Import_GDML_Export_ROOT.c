void Import_GDML_Export_ROOT()
{
	TString vmcDir = TString(gSystem->Getenv("VMCWORKDIR"));

	TGeoManager *gdml = new TGeoManager("gdml", "FAIRGeom");

	TGDMLParse parser;
	// Define your input GDML file HERE
	TGeoVolume* gdmlTop = parser.GDMLReadFile (vmcDir + "/geometry/rich/rich_v17a_1e_nobpmt.gdml");
	TGeoVolume* rootTop = new TGeoVolumeAssembly("TOP");

	gGeoManager->SetTopVolume(rootTop);

	// Define your position HERE
	// Z coordinate for v16a = 270, for v17a = 258.75
	TGeoRotation* rot = new TGeoRotation ("rot", 0., 0., 0.);
	TGeoCombiTrans* posrot = new TGeoCombiTrans (0., 0., 258.75, rot); // v16a - 270, v17a - 258.75

	rootTop->AddNode (gdmlTop, 1, posrot);

	gGeoManager->CloseGeometry();

	// Define you output ROOT file HERE
	TFile* outfile = new TFile(vmcDir + "/geometry/rich/rich_v17a_1e_nobpmt.geo.root", "RECREATE");
	rootTop->Write();
	outfile->Close();
}

