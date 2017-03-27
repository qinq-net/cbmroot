void Import_GDML_Export_ROOT(TString axis="", TString nb="")
{
//	TString vmcDir = TString(gSystem->Getenv("VMCWORKDIR"));
	TString vmcDir = "/u/jbendar/cbmroot5/CBMSRC/geometry/rich/";

	TGeoManager *gdml = new TGeoManager("gdml", "FAIRGeom");

	TGDMLParse parser;
	// Define your input GDML file HERE
//	TGeoVolume* gdmlTop = parser.GDMLReadFile (vmcDir + "/geometry/rich/rich_v16a_1e.gdml");
//	TGeoVolume* gdmlTop = parser.GDMLReadFile (vmcDir + "/geometry/rich/Rich_jan2016_misalign.gdml");
//	TGeoVolume* gdmlTop = parser.GDMLReadFile (vmcDir + "/geometry/rich/Rich_jan2016_misalign_5mrad_study.gdml");
//	TGeoVolume* gdmlTop = parser.GDMLReadFile (vmcDir + "/geometry/rich/RICH_21Nov2016_simplified_test.gdml");

	TGeoVolume* gdmlTop = parser.GDMLReadFile (vmcDir + "matching/misalign_1mrad_jan16.gdml");
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
//	TFile* outfile = new TFile(vmcDir + "/geometry/rich/rich_v16a_1e.root", "RECREATE");
//	TFile* outfile = new TFile(vmcDir + "/geometry/rich/Rich_jan2016_misalign_1pt5.root", "RECREATE");
//	TFile* outfile = new TFile(vmcDir + "/geometry/rich/Rich_jan2016_misalign_5mrad_Tiles_1_5_0_1.root", "RECREATE");
//	TFile* outfile = new TFile(vmcDir + "/geometry/rich/Rich_jan2016_misalign_5mradXY_Tile1_3.root", "RECREATE");
//	TFile* outfile = new TFile(vmcDir + "/geometry/rich/RICH_21Nov2016_simplified_test.root", "RECREATE");
//	TFile* outfile = new TFile("/data/misalignment_correction/Sim_Outputs/RICH_Geometries/Tile_2_8/Rich_jan2016_misalign_" + nb + "mrad_" + axis + ".root", "RECREATE");
//	TFile* outfile = new TFile("/home/aghoehne/Desktop/Rich_jan2016_misalign_" + nb + "mrad_" + axis + ".root", "RECREATE");

	TFile* outfile = new TFile(vmcDir + "rich_v16a_matching_misalign_1mrad_jan16.geo.root", "RECREATE");
	rootTop->Write();
	outfile->Close();
}

