
void run_ronchi() {

    gSystem->Load("libtiff");

    std::string dir = "/dir/";
    std::string fileName1 = "test_grid_1.tiff";

    CbmRichRonchiAna *ronchiAna = new CbmRichRonchiAna();
    ronchiAna->SetTiffFileName("test_grid_1.tif");
    ronchiAna->Run();
}
