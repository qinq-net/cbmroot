void check_overlaps(const char* setup = "sis300_electron")
{
  // 2014-07-04 - DE - test CBM setups for collisions in nightly tests
  // 2014-07-04 - DE - currently there are 2 overlaps between the PIPE and STS layer 8
  // 2014-07-04 - DE - set the default to 0 overlaps, anyway
  // 2017-29-11 - FU - define some expected overlaps between magnet and rich or much
  //                   these overlas are accepted for the time beeing till
  //                   there is a new magnet geometry 

  UInt_t unexpectedOverlaps{0};

  TString outDir  = "data/";
  TString geoFile = outDir + setup + "_geofile_full.root";
  TFile* f = new TFile(geoFile);

  gGeoManager = (TGeoManager*) f->Get("FAIRGeom"); 

  gGeoManager->CheckOverlaps(0.0001);
  TIter next(gGeoManager->GetListOfOverlaps());
  TGeoOverlap *ov;
  while ((ov=(TGeoOverlap*)next())) {
    TString OverlapName = ov->GetTitle();
    if (OverlapName.Contains("cave/magnet_v15b_0 overlapping cave/much_0/absorber_1/absblock1_1/absorber_1")) {
      cout << "Expected Overlap" << endl; 
      cout << ov->GetTitle() << endl;    
      cout << endl;
    } else if(OverlapName.Contains("cave/magnet_v15b_0 overlapping cave/much_0/absorber_1/absblock0_0/absorber_0")) {
      cout << "Expected Overlap" << endl; 
      cout << ov->GetTitle() << endl;    
      cout << endl;
    } else if(OverlapName.Contains("cave/magnet_v15a_0 overlapping cave/rich_v17a_1e_0/rich_container_290")) {
      cout << "Expected Overlap" << endl; 
      cout << ov->GetTitle() << endl;    
      cout << endl;
    } else {
      cout << "Unexpected Overlap:" << endl; 
      ov->PrintInfo();
      cout << endl;
      unexpectedOverlaps++;
    }
  }
  cout << endl;

  gGeoManager->CheckOverlaps(0.0001, "s");
  TIter next1(gGeoManager->GetListOfOverlaps());
  while ((ov=(TGeoOverlap*)next1())) {
    TString OverlapName = ov->GetTitle();
    if (OverlapName.Contains("magnet_v15b_0 overlapping much_0/absorber_1/absblock1_1/absorber_1")) {
      cout << "Expected Overlap" << endl; 
      cout << ov->GetTitle() << endl;    
      cout << endl;
    } else if(OverlapName.Contains("magnet_v15a_0 overlapping rich_v17a_1e_0/rich_container_290")) {
      cout << "Expected Overlap" << endl; 
      cout << ov->GetTitle() << endl;    
      cout << endl;
    } else {
      cout << "Unexpected Overlap:" << endl; 
      cout << endl;
      ov->PrintInfo();
      unexpectedOverlaps++;
    }
  }
  cout << endl;

  if ( unexpectedOverlaps != 0 ) {
    cout << " Test failed" << endl;
    cout << " We have in total "<< unexpectedOverlaps <<" unexpected overlaps."<<endl;
  } else {
    cout << " There are no unexpected overlaps." << endl;
    cout << " Test passed" << endl;
    cout << " All ok " << endl;
  }


  RemoveGeoManager();
}
