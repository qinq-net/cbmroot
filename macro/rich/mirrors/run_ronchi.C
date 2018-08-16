
void run_ronchi() {

    gSystem->Load("libtiff");

    std::string dir = "/dir/";
    std::string fileName1 = "ronchi.tiff";

    CbmRichRonchiAna *ronchiAna = new CbmRichRonchiAna();
    ronchiAna->Run();
}
