/**
 * Performs segmentation of Much stations based on
 * user specified conditions.
 *
 * @author M.Ryzhinskiy m.ryzhinskiy@gsi.de
 * @param mcFile       Input transport file name
 * @param inDigiFile   Input file name containing initial segmentation parameters
 * @param outDigiFile  Output file name containing segmentation parameters
 */
void much_seg_sector(TString mcFile = "",
                     TString inDigiFile = "",
                     TString outDigiFile = "")
{
  // ========================================================================
  //          Adjust this part according to your requirements

  //  if (mcFile == "") {
  if (mcFile == "") {
    mcFile = "data/mc.root";
  }
  //  if (inDigiFile == "") {
  if (inDigiFile == "") {
    inDigiFile = "data/much_digi_sector_4station.seg";
    //    inDigiFile = "data/much_digi_sector.seg";
  }
  //  if (outDigiFile == "") {
  if (outDigiFile=="") {
    outDigiFile = "data/much_digi_sector.root";
  }

  // Verbosity level (0=quiet, 1=event level, 2=track level, 3=debug)
  Int_t iVerbose = 0;

  // Dummy ROOT file (needed as an output)
  TString outFile = "data/dummy.root";
  TString parFile = "data/params.root";

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
  CbmMuchSegmentSector* seg = new CbmMuchSegmentSector(inDigiFile.Data(), outDigiFile.Data());
  //  seg->DebugSwitchOn();
  fRun->AddTask(seg);
  // ------------------------------------------------------------------------

  // Run segmentation
  fRun->Init();

  cout << " Test passed" << endl;
  cout << " All ok " << endl;

  // Function needed for CTest runtime dependency
  Generate_CTest_Dependency_File(depFile);

}
