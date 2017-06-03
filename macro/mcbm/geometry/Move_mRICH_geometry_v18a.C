///                                             
/// \file Move_mRICH_geometry_v18a.C
/// \brief Generates TRD geometry in Root format.
///                                             

// 2017-06-03 - DE - v18a - add script to move the mRICH

//void Create_TRD_Geometry_v18g(const Int_t setupid = 1) {
void Move_mRICH_geometry_v18a() {

  //  rich->Export(FileNameSim);   // an alternative way of writing the rich volume

  TFile* outfile = new TFile(FileNameSim, "UPDATE");
  //  TGeoTranslation* rich_placement = new TGeoTranslation("rich_trans", 0., 0., 0.);  
  TGeoTranslation* rich_placement = new TGeoTranslation("rich_trans", 0., 0., zfront[setupid]);  
  rich_placement->Write();
  outfile->Close();

  //  top->Draw("ogl");
}

