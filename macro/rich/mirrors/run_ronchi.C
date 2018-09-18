
void run_ronchi() 
{
    gSystem->Load("libtiff");

    std::string dir = "/dir/";
    std::string fileName1 = "horizontal.tiff";
    //std::string fileName2 = "vertical.tiff";

    CbmRichRonchiAna *ronchiAna = new CbmRichRonchiAna();
    ronchiAna->SetTiffFileNameV("horizontal.tif");
    //ronchiAna->SetTiffFileName("vertical.tif");
    ronchiAna->Run();
}
