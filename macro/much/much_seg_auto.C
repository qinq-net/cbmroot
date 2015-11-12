/**
 * Performs automatic segmentation of Much stations based on
 * 5% occupancy criterion.
 *
 * @author M.Ryzhinskiy m.ryzhinskiy@gsi.de
 * @param mcFile    Input transport file name
 * @param digiFile  Output file name containing segmentation parameters
 * @param nEvents   Number of events to process
 */
void much_seg_auto(const char* mcFile = "",
              const char* digiFile = "",
	      Int_t nEvents = 2)
{
  // ========================================================================
  //          Adjust this part according to your requirements

  if (strcmp (mcFile,"") == 0) {
    mcFile = "data/mc.root";
  }
  if (strcmp (digiFile,"") == 0) {
    digiFile = "data/much_digi.root";
  }
  TString parFile="data/params.root";


  // Verbosity level (0=quiet, 1=event level, 2=track level, 3=debug)
  Int_t iVerbose = 0;

  // Dummy ROOT file (neede as an output)
  TString outFile  = "data/dummy.root";

  // Function needed for CTest runtime dependency 
  TString depFile = Remove_CTest_Dependency_File("data", "much_seg");  


  // ------------------------------------------------------------------------

  // -----   Analysis run   -------------------------------------------------
  FairRunAna *fRun= new FairRunAna();
  fRun->SetInputFile(mcFile);
  fRun->SetOutputFile(outFile);
  // ------------------------------------------------------------------------

  // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
  FairParRootFileIo*  parIo1 = new FairParRootFileIo();
  parIo1->open(parFile);
  rtdb->setFirstInput(parIo1);
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  // ------------------------------------------------------------------------


  // -----  Segmentation task  ----------------------------------------------
  CbmMuchSegmentAuto* seg = new CbmMuchSegmentAuto(digiFile);

  // Number of stations
//new  seg->SetNStations(4);  // fix for much_v15b_STS100-B_125cm_no.geo
  seg->SetNStations(6);  // much_v12b.geo

  // Set minimum allowed resolution for each station
  Double_t sigmaXmin[] = {0.08, 0.08, 0.08, 0.08, 0.08, 0.08};
  Double_t sigmaYmin[] = {0.08, 0.08, 0.08, 0.08, 0.08, 0.08};
  seg->SetSigmaMin(sigmaXmin, sigmaYmin);

  // Set maximum allowed resolution for each station
  Double_t sigmaXmax[] = {0.6, 0.6, 0.6, 0.6, 0.8, 1.};
  Double_t sigmaYmax[] = {0.6, 0.6, 0.6, 0.6, 0.8, 1.};
  seg->SetSigmaMax(sigmaXmax, sigmaYmax);

  // Set maximum occupancy for each station
  Double_t occupancyMax[] = {0.05, 0.05, 0.05, 0.05, 0.05, 0.05};
  seg->SetOccupancyMax(occupancyMax);

  fRun->AddTask(seg);
  // ------------------------------------------------------------------------

  // Run segmentation
  fRun->Init();
  fRun->Run(0,nEvents);

  cout << " Test passed" << endl;
  cout << " All ok " << endl;

  // Function needed for CTest runtime dependency
  Generate_CTest_Dependency_File(depFile);

}
