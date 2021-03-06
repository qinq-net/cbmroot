/**
 * \file much_reco.C
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2009
 * \brief Reconstruction macro for MUCH.
 **/

#include <iostream>
using namespace std;

/*
flag =0 for sis 100 geometry
flag = 1 for mini-cbm geometry
 */

void much_reco( Int_t nEvents = 3, Int_t flag = 0)
{
   TString mcFile = "data/mc.root";
   TString rawFile = "data/raw.root";
   TString parFile = "data/params.root"; // Parameters file
   TString recoFile = "data/reco.root"; // Output file with reconstructed tracks and hits

   // Digi files
   TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters");
//   TList* parFileList = new TList();
//   TObjString stsDigiFile(parDir + "/sts/sts_v13d_std.digi.par"); // STS digi file
   TString muchDigiFile(parDir + "/much/much_v17b_digi_sector.root"); // MUCH digi file
   TString stsMatBudgetFile(parDir + "/sts/sts_matbudget_v15a.root");
//   parFileList->Add(&stsDigiFile);

	Int_t iVerbose = 1;
	TStopwatch timer;
	timer.Start();

	FairRunAna* run = new FairRunAna();
	run->SetInputFile(rawFile);
	run->AddFriend(mcFile);
	run->SetOutputFile(recoFile);
    run->SetGenerateRunInfo(kTRUE);
  Bool_t hasFairMonitor = Has_Fair_Monitor();
  if (hasFairMonitor) {
    FairMonitor::GetMonitor()->EnableMonitor(kTRUE);
  }

  // ----- MC Data Manager   ------------------------------------------------
  CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
  mcManager->AddFile(mcFile);
  run->AddTask(mcManager);
  // ------------------------------------------------------------------------


  // -----   STS Cluster Finder   --------------------------------------------
  CbmStsFindClusters* stsClusterFinder = new CbmStsFindClusters();
  stsClusterFinder->UseEventMode();
  run->AddTask(stsClusterFinder);
  // -------------------------------------------------------------------------


  // -----   STS hit finder   ------------------------------------------------
  FairTask* stsFindHits = new CbmStsFindHits();
  run->AddTask(stsFindHits);
  // -------------------------------------------------------------------------


   // --- STS reconstruction ---------------------------------------------------
   FairTask* kalman = new CbmKF();
   run->AddTask(kalman);
   CbmL1* l1 = new CbmL1();
   l1->SetStsMaterialBudgetFileName(stsMatBudgetFile);
   run->AddTask(l1);
   CbmStsTrackFinder* trackFinder = new CbmL1StsTrackFinder();
   FairTask* findTracks = new CbmStsFindTracks(iVerbose, trackFinder);
   run->AddTask(findTracks);
   // --------------------------------------------------------------------------

   // ----- MUCH hits----------   ----------------------------------------------
   CbmMuchFindHitsGem* muchFindHits = new CbmMuchFindHitsGem(muchDigiFile.Data(), flag);
   run->AddTask(muchFindHits);
//   CbmMuchFindHitsStraws* strawFindHits = new CbmMuchFindHitsStraws(muchDigiFile.Data());
//   run->AddTask(strawFindHits);
   // --------------------------------------------------------------------------

   // --- Global tracking ------------------------------------------------------
   CbmLitFindGlobalTracks* finder = new CbmLitFindGlobalTracks();
	finder->SetTrackingType("branch");
	finder->SetMergerType("nearest_hit");
	run->AddTask(finder);
	// --------------------------------------------------------------------------

   CbmMatchRecoToMC* matchTask = new CbmMatchRecoToMC();
   run->AddTask(matchTask);

   // -----   Primary vertex finding   -----------------------------------------
   CbmPrimaryVertexFinder* pvFinder = new CbmPVFinderKF();
   CbmFindPrimaryVertex* findVertex = new CbmFindPrimaryVertex(pvFinder);
   run->AddTask(findVertex);
   // --------------------------------------------------------------------------

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
   cout << "Output file is "    << recoFile << endl;
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
   RemoveGeoManager();
}
