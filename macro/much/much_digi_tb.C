/**
 * \file much_reco_tb.C
 * 
 * \author Vikas Singhal <vikas@vecc.gov.in>
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2009
 * \Time Based Digitization macro for MUCH, enabled only CbmMuchDigitizeGem in Time Based mode
 **/

#include <iostream>
using namespace std;

void much_digi_tb(Int_t nEvents = 100)
{
  TString dir  = "data/"; // Output directory
  TString mcFile = dir + "mc.root"; // MC transport file
  TString parFile = dir + "params.root"; // Parameters file
  TString globalRecoFile = dir + "digi_tb.root"; // Output file with reconstructed time based digis
  
  // Digi files
  TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters");
  //   TList* parFileList = new TList();
  //   TObjString stsDigiFile(parDir + "/sts/sts_v13d_std.digi.par"); // STS digi file
  //TString muchDigiFile(parDir + "/much/much_v15b_digi_sector.root"); // MUCH digi file
  TString muchDigiFile("data/much_digi_sector_jpsisetup.root"); // MUCH digi filei
  //   TString stsMatBudgetFile(parDir + "/sts/sts_matbudget_v15a.root");
  //   parFileList->Add(&stsDigiFile);
  // Specify interaction rate in 1/se
  Double_t eventRate = 1.e7;
  //
  // Specify duration of time slices in output [ns]
  Double_t timeSliceSize = 1000.;
  // --- Logger settings ---------------------------------------------------
  TString logLevel     = "INFO";
  TString logVerbosity = "";
  // ------------------------------------------------------------------------
  Int_t iVerbose = 1;
  TStopwatch timer;
  timer.Start();
  
  
  //----------Reconstruction run ---------------------------------
  //FairRunAna* run = new FairRunAna();
  CbmRunAna* run = new CbmRunAna();
  run->SetAsync();
  run->SetInputFile(mcFile);
  run->SetOutputFile(globalRecoFile);
  run->SetGenerateRunInfo(kTRUE);
  run->SetEventMeanTime(1.e9 / eventRate);
  FairRootManager::Instance()->SetUseFairLinks(kTRUE);
  
  Bool_t hasFairMonitor = Has_Fair_Monitor();
  if (hasFairMonitor) {
    FairMonitor::GetMonitor()->EnableMonitor(kTRUE);
  }
  
  // -----   Logger settings   ----------------------------------------------
  FairLogger::GetLogger()->SetLogScreenLevel(logLevel.Data());
  FairLogger::GetLogger()->SetLogVerbosityLevel(logVerbosity.Data());
  // ------------------------------------------------------------------------
  
  // ----- MC Data Manager   ------------------------------------------------
  CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
  mcManager->AddFile(mcFile);
  run->AddTask(mcManager);
  // ------------------------------------------------------------------------
  
  
  // -----   STS digitizer   -------------------------------------------------
  
  //  FairTask* stsDigi = new CbmStsDigitize();
  //  run->AddTask(stsDigi);
  
  // -----   The parameters of the STS digitizer are set such as to match
  // -----   those in the old digitizer. Change them only if you know what you
  // -----   are doing.
  /*
    Double_t dynRange       =   40960.;  // Dynamic range [e]
    Double_t threshold      =    4000.;  // Digitisation threshold [e]
    Int_t nAdc              =    4096;   // Number of ADC channels (12 bit)
    Double_t timeResolution =       5.;  // time resolution [ns]
    Double_t deadTime       = 9999999.;  // infinite dead time (integrate entire event)
    Double_t noise          =       0.;  // ENC [e]
    Int_t digiModel         =       1;   // Model: 1 = uniform charge distribution along track
    
    
    
    CbmStsDigitize* stsDigi = new CbmStsDigitize(digiModel);
    stsDigi->SetParameters(dynRange, threshold, nAdc, timeResolution,
    deadTime, noise);
    run->AddTask(stsDigi);
  */
  // -------------------------------------------------------------------------
  
  
  // -----   STS Cluster Finder   --------------------------------------------
  //  FairTask* stsClusterFinder = new CbmStsFindClusters();
  //  run->AddTask(stsClusterFinder);
  // -------------------------------------------------------------------------
  
  
  // -----   STS hit finder   ------------------------------------------------
  //  FairTask* stsFindHits = new CbmStsFindHits();
  //  run->AddTask(stsFindHits);
  // -------------------------------------------------------------------------
  
  
  // --- STS reconstruction ---------------------------------------------------
  /*   FairTask* kalman = new CbmKF();
       run->AddTask(kalman);
       CbmL1* l1 = new CbmL1();
       l1->SetStsMaterialBudgetFileName(stsMatBudgetFile);
       run->AddTask(l1);
       CbmStsTrackFinder* trackFinder = new CbmL1StsTrackFinder();
       FairTask* findTracks = new CbmStsFindTracks(iVerbose, trackFinder);
       run->AddTask(findTracks);*/
  // --------------------------------------------------------------------------
  
  // ----- MUCH hits----------   ----------------------------------------------
  CbmMuchDigitizeGem* muchDigitize = new CbmMuchDigitizeGem(muchDigiFile.Data());
  muchDigitize->SetDaq(1); //0 for event by event mode and 1 for time based mode
  run->AddTask(muchDigitize);
  //   CbmMuchDigitizeStraws* strawDigitize = new CbmMuchDigitizeStraws(muchDigiFile.Data());
  // run->AddTask(strawDigitize);
  
  //  CbmMuchFindHitsGem* muchFindHits = new CbmMuchFindHitsGem(muchDigiFile.Data());
  //  run->AddTask(muchFindHits);
  //  CbmMuchFindHitsStraws* strawFindHits = new CbmMuchFindHitsStraws(muchDigiFile.Data());
  //  run->AddTask(strawFindHits);
  // --------------------------------------------------------------------------
  
  // --- Global tracking ------------------------------------------------------
  /*   CbmLitFindGlobalTracks* finder = new CbmLitFindGlobalTracks();
       finder->SetTrackingType("branch");
       finder->SetMergerType("nearest_hit");
       run->AddTask(finder);*/
  // --------------------------------------------------------------------------
  
  // CbmMatchRecoToMC* matchTask = new CbmMatchRecoToMC();
  // run->AddTask(matchTask);
  
  // -----   Primary vertex finding   -----------------------------------------
  //   CbmPrimaryVertexFinder* pvFinder = new CbmPVFinderKF();
  // CbmFindPrimaryVertex* findVertex = new CbmFindPrimaryVertex(pvFinder);
  // run->AddTask(findVertex);
  // --------------------------------------------------------------------------
  // ----- DAQ
  FairTask* daq = new CbmDaq(timeSliceSize);
  run->AddTask(daq);
  //
  // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  //   FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
  parIo1->open(parFile.Data());
  //   parIo2->open(parFileList, "in");
  rtdb->setFirstInput(parIo1);
  //   rtdb->setSecondInput(parIo2);
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  // ------------------------------------------------------------------------
  
  // -----   Initialize and run   --------------------------------------------
  run->Init();
  run->Run(0,nEvents);
  // ------------------------------------------------------------------------
  
  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << "Macro finished successfully." << endl;
  cout << "Output file is "    << globalRecoFile << endl;
  cout << "Parameter file is " << parFile << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
  cout << endl;
  // ------------------------------------------------------------------------
  
  if (hasFairMonitor) {
    // Extract the maximal used memory an add is as Dart measurement
    // This line is filtered by CTest and the value send to CDash
    FairSystemInfo sysInfo;
    Float_t maxMemory=sysInfo.GetMaxMemory();
    cout << "<DartMeasurement name=\"MaxMemory\" type=\"numeric/double\">";
    cout << maxMemory;
    cout << "</DartMeasurement>" << endl;
    
    Float_t cpuUsage=ctime/rtime;
    cout << "<DartMeasurement name=\"CpuLoad\" type=\"numeric/double\">";
    cout << cpuUsage;
    cout << "</DartMeasurement>" << endl;
    
    FairMonitor* tempMon = FairMonitor::GetMonitor();
    tempMon->Print();
  }
  
  cout << " Test passed" << endl;
  cout << " All ok " << endl;
}
