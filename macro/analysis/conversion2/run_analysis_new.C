// --------------------------------------------------------------------------
//
// Macro for reconstruction of simulated events with standard settings
//
// HitProducers in MVD, RICH, TRD, TOF, ECAL
// Digitizer and HitFinder in STS
// FAST MC for ECAL
// STS track finding and fitting (L1 / KF)
// TRD track finding and fitting (L1 / KF)
// RICH ring finding (ideal) and fitting
// Global track finding (ideal), rich assignment
// Primary vertex finding (ideal)
// Matching of reconstructed and MC tracks in STS, RICH and TRD
//
// V. Friese   24/02/2006
// Version     04/03/2015 (V. Friese)
//
// --------------------------------------------------------------------------

TString caveGeom="";
TString pipeGeom="";
TString magnetGeom="";
TString mvdGeom="";
TString stsGeom="";
TString richGeom="";
TString muchGeom="";
TString shieldGeom="";
TString trdGeom="";
TString tofGeom="";
TString ecalGeom="";
TString platformGeom="";
TString psdGeom="";
Double_t psdZpos=0.;
Double_t psdXpos=0.;

TString mvdTag="";
TString stsTag="";
TString trdTag="";
TString tofTag="";

TString stsDigi="";
TString muchDigi="";
TString trdDigi="";
TString tofDigi="";

TString mvdMatBudget="";
TString stsMatBudget="";

TString  fieldMap="";
Double_t fieldZ=0.;
//Double_t fieldScale=1.;
Int_t    fieldSymType=0;

TString defaultInputFile="";

void run_analysis_new(Int_t nEvents = 2, Int_t file_nr = 1, const char* setup = "light_sis100_electron")
{

	Char_t filenr[5];
	sprintf(filenr,"%05d",file_nr);
	printf("Filenr: %s\n", filenr);
	TString temp = filenr;

  // ========================================================================
  //          Adjust this part according to your requirements

  // Verbosity level (0=quiet, 1=event level, 2=track level, 3=debug)
  Int_t iVerbose = 0;
  FairLogger* logger = FairLogger::GetLogger();
  logger->SetLogScreenLevel("INFO");
  logger->SetLogVerbosityLevel("LOW");
	
//  TString outDir  = "copy/";
  TString outDir  = "/common/cbmb/Users/kres/simulations/just_simulated/";
  TString inFile  = outDir + setup + "_test.mc."+ temp +".root";       // Input file (MC events)
  TString parFile = outDir + setup + "_params."+ temp +".root";        // Parameter file
  TString outFile = outDir + setup + "_test.eds."+ temp +".root";      // Output file
  TString analysisFile = outDir + setup + "_analysis."+ temp +".root";

  // Function needed for CTest runtime dependency
//  TString depFile = Remove_CTest_Dependency_File(outDir, "run_reco" , setup);

  //  Digitisation files.
  // Add TObjectString containing the different file names to
  // a TList which is passed as input to the FairParAsciiFileIo.
  // The FairParAsciiFileIo will take care to create on the fly 
  // a concatenated input parameter file which is then used during
  // the reconstruction.
  TList *parFileList = new TList();

  TString inDir = gSystem->Getenv("VMCWORKDIR");
  TString paramDir = inDir + "/parameters/";

  TString setupFile = inDir + "/geometry/setup/" + setup + "_setup.C";
  TString setupFunct = setup;
  setupFunct += "_setup()";

  gROOT->LoadMacro(setupFile);
  gInterpreter->ProcessLine(setupFunct);
  
  // --- STS digipar file is there only for L1. It is no longer required
  // --- for STS digitisation and should be eventually removed.
  //TObjString stsDigiFile(paramDir + stsDigi);
  //parFileList->Add(&stsDigiFile);
  //cout << "macro/run/run_reco.C using: " << stsDigi << endl;

  TObjString trdDigiFile(paramDir + trdDigi);
  parFileList->Add(&trdDigiFile);
  cout << "macro/run/run_reco.C using: " << trdDigi << endl;

  TObjString tofDigiFile(paramDir + tofDigi);
  parFileList->Add(&tofDigiFile);
  cout << "macro/run/run_reco.C using: " << tofDigi << endl;

  // In general, the following parts need not be touched
  // ========================================================================


  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  // ------------------------------------------------------------------------


  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------

  // -----   Reconstruction run   -------------------------------------------
  FairRunAna *run = new FairRunAna();
  run->SetInputFile(inFile);
  run->SetOutputFile(analysisFile);
  run->AddFriend(outFile);
//  run->SetGenerateRunInfo(kTRUE);
//  Bool_t hasFairMonitor = Has_Fair_Monitor();
//  if (hasFairMonitor) {
//    FairMonitor::GetMonitor()->EnableMonitor(kTRUE);
//  }

  CbmKF* kalman = new CbmKF();
  run->AddTask(kalman);
  CbmL1* l1 = new CbmL1();
  TString mvdMatBudgetFileName = paramDir + mvdMatBudget;
  TString stsMatBudgetFileName = paramDir + stsMatBudget;
  l1->SetStsMaterialBudgetFileName(stsMatBudgetFileName.Data());
  l1->SetMvdMaterialBudgetFileName(mvdMatBudgetFileName.Data());
  run->AddTask(l1);

  CbmStsTrackFinder* stsTrackFinder = new CbmL1StsTrackFinder();
  FairTask* stsFindTracks = new CbmStsFindTracks(iVerbose, stsTrackFinder);
  run->AddTask(stsFindTracks);
// ##########################################################################################################
	// ----- PID for KF Particle Finder --------------------------------------------
	Int_t kf_useMC = 1;
	CbmKFParticleFinderPID* kfParticleFinderPID = new CbmKFParticleFinderPID();
	if(kf_useMC == 1) {
		kfParticleFinderPID->SetPIDMode(1); 	// 1 - use MC-true PID, 2 - Rich etc
	}
	else {
		kfParticleFinderPID->SetPIDMode(2); 	// use reco PID
//		kfParticleFinderPID->UseRICHRvspPID();	// use RICH PID
//		kfParticleFinderPID->DoNotUseTRD();		// do not use TRD PID
//		kfParticleFinderPID->SetSIS100();		// use SIS100 mode
	}
	run->AddTask(kfParticleFinderPID);
  
	// ----- KF Particle Finder --------------------------------------------
	CbmKFParticleFinder* kfParticleFinder = new CbmKFParticleFinder();
	kfParticleFinder->SetPIDInformation(kfParticleFinderPID);
	//kfParticleFinder->SetPVToZero();
	run->AddTask(kfParticleFinder);

	// ----- KF Particle Finder QA --------------------------------------------
	CbmKFParticleFinderQA* kfParticleFinderQA = new CbmKFParticleFinderQA("CbmKFParticleFinderQA", 0, kfParticleFinder->GetTopoReconstructor(),"");
	kfParticleFinderQA->SetPrintEffFrequency(50000);
	TString effFileName = analysisFile;
	effFileName += ".eff_mc.txt";
	kfParticleFinderQA->SetEffFileName(effFileName.Data());
	kfParticleFinderQA->SaveMCParticles(true);
	run->AddTask(kfParticleFinderQA);
// ##########################################################################################################


  // ------------------------------------------------------------------------

   CbmAnaConversion2* conversionAna2 = new CbmAnaConversion2();
	conversionAna2->SetKF(kfParticleFinder, kfParticleFinderQA);
   run->AddTask(conversionAna2);

  // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
  parIo1->open(parFile.Data());
  parIo2->open(parFileList, "in");
  rtdb->setFirstInput(parIo1);
  rtdb->setSecondInput(parIo2);
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  // ------------------------------------------------------------------------


  // -----   Intialise and run   --------------------------------------------
  run->Init();
  cout << "Starting run" << endl;
  run->Run(0, nEvents);
  // ------------------------------------------------------------------------

  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Macro finished succesfully." << endl;
  cout << "Output file is " << outFile << endl;
  cout << "Parameter file is " << parFile << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
  cout << endl;
  // ------------------------------------------------------------------------

/*  if (hasFairMonitor) {
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
*/
  cout << " Test passed" << endl;
  cout << " All ok " << endl;

  // Function needed for CTest runtime dependency
//  Generate_CTest_Dependency_File(depFile);
}
