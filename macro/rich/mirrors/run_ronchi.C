void run_ronchi()
{
    gSystem->Load("libtiff");

    std::string dir = "/dir/";
    std::string fileName1 = "horizontal.tif";
    std::string fileName2 = "vertical.tif";

    CbmRichRonchiAna *ronchiAna = new CbmRichRonchiAna();
    ronchiAna->SetTiffFileNameH(fileName1);
    ronchiAna->SetTiffFileNameV(fileName2);
    ronchiAna->Run();
}
