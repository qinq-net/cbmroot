void Import_GDML_Export_ROOT(TString FileName)
{
  TString InFile=FileName+".gdml";
  TString OutFile=FileName+".root";
  TString vmcDir = TString(gSystem->Getenv("VMCWORKDIR"));
  
  TGeoManager *gdml = new TGeoManager("gdml", "FAIRGeom");
  
  TGDMLParse parser;
  // Define your input GDML file HERE
  TGeoVolume* gdmlTop = parser.GDMLReadFile (vmcDir + InFile);
  TGeoVolume* rootTop = new TGeoVolumeAssembly("TOP");
  
  gGeoManager->SetTopVolume(rootTop);
  
  // Define your position HERE
  TGeoRotation* rot = new TGeoRotation ("rot", 0., 0., 0.);
  TGeoCombiTrans* posrot = new TGeoCombiTrans (0., 0., 270., rot);
  
  rootTop->AddNode (gdmlTop, 1, posrot);
  
  gGeoManager->CloseGeometry();
  
  // Define you output ROOT file HERE
  TFile* outfile = new TFile(vmcDir + OutFile, "RECREATE");
  rootTop->Write();
  outfile->Close();
}

