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
TString trdDigi="";
TString tofDigi="";

TString mvdMatBudget="";
TString stsMatBudget="";

TString  fieldMap="";
Double_t fieldZ=0.;
Double_t fieldScale=0.;
Int_t    fieldSymType=0;

TString defaultInputFile="";



void run_reco(Int_t nEvents = 2, Int_t mode = 0, Int_t file_nr = 1, const char* inputsetup = "", TString output = "")
{

  // ========================================================================
  //          Adjust this part according to your requirements

  // Verbosity level (0=quiet, 1=event level, 2=track level, 3=debug)
  Int_t iVerbose = 0;
  FairLogger* logger = FairLogger::GetLogger();
  logger->SetLogScreenLevel("INFO");
  logger->SetLogVerbosityLevel("LOW");
  
	if(mode == 0) {
		cout << "ERROR: No mode specified! Exiting..." << endl;
		exit();
	}

	Char_t filenr[5];
	sprintf(filenr,"%05d",file_nr);
	printf("Filenr: %s\n", filenr);
	TString temp = filenr;
  
  	TString inFile = "";
	TString parFile = "";
	TString outFile ="";

	TString outName;
	if(output == "") {
		outName = "sis300electron_test";
	}
	else {
		outName = output;
	}
	if(mode == 1) {	// tomography
		TString dir = "/common/cbmb/Users/reinecke/simulations/outputs/tomography/" + outName;
		parFile		= dir + "/tomography." + outName + ".param.root";
		inFile		= dir + "/tomography." + outName + ".mc.root";
		outFile	= dir + "/tomography." + outName + ".reco.root";
	}
	if(mode == 2) {	// urqmd
		TString dir = "/common/cbmb/Users/reinecke/simulations/outputs/urqmd/" + outName;
		parFile		= dir + "/urqmd." + outName + ".param." + temp + ".root";
		inFile		= dir + "/urqmd." + outName + ".mc." + temp + ".root";
		outFile	= dir + "/urqmd." + outName + ".reco." + temp + ".root";
	}
	if(mode == 3) {	// pluto
		TString dir = "/common/cbmb/Users/reinecke/simulations/outputs/pluto/" + outName;
		parFile		= dir + "/pluto." + outName + ".param." + temp + ".root";
		inFile		= dir + "/pluto." + outName + ".mc." + temp + ".root";
		outFile	= dir + "/pluto." + outName + ".reco." + temp + ".root";
	}
	if(mode == 4) {	// pi0only
		TString dir = "/common/cbmb/Users/reinecke/simulations/outputs/pi0only/" + outName;
		parFile		= dir + "/pi0only." + outName + ".param." + temp + ".root";
		inFile		= dir + "/pi0only." + outName + ".mc." + temp + ".root";
		outFile	= dir + "/pi0only." + outName + ".reco." + temp + ".root";
	}

  //  Digitisation files.
  // Add TObjectString containing the different file names to
  // a TList which is passed as input to the FairParAsciiFileIo.
  // The FairParAsciiFileIo will take care to create on the fly 
  // a concatenated input parameter file which is then used during
  // the reconstruction.
  TList *parFileList = new TList();

	const char* setup;
	if(inputsetup == "") {
		setup = "sis100_electron";
	}
	else {
		setup = inputsetup;
	}
  //TString inDir = gSystem->Getenv("VMCWORKDIR");
  TString inDir = "/common/cbma/users/reinecke/software/simu_apr14/cbmroot_jul14";
  TString paramDir = inDir + "/parameters/";

  TString setupFile = inDir + "/geometry/setup/" + setup + "_setup.C";
  TString setupFunct = setup;
  setupFunct += "_setup()";

  gROOT->LoadMacro(setupFile);
  gInterpreter->ProcessLine(setupFunct);
  
  // --- STS digipar file is there only for L1. It is no longer required
  // ---  for STS digitisation and should be eventually removed.
  //TObjString stsDigiFile = paramDir + stsDigi;
  //parFileList->Add(&stsDigiFile);
  //cout << "macro/run/run_reco.C using: " << stsDigi << endl;

  TObjString trdDigiFile = paramDir + trdDigi;
  parFileList->Add(&trdDigiFile);
  cout << "macro/run/run_reco.C using: " << trdDigi << endl;

  TObjString tofDigiFile = paramDir + tofDigi;
  parFileList->Add(&tofDigiFile);
  cout << "macro/run/run_reco.C using: " << tofDigi << endl;

  // Function needed for CTest runtime dependency
  TString depFile = Remove_CTest_Dependency_File(outDir, "run_reco" , setup);



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
  run->SetOutputFile(outFile);
  run->SetGenerateRunInfo(kTRUE);       // Create FairRunInfo file
  // ------------------------------------------------------------------------

  // ----- MC Data Manager   ------------------------------------------------
  CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
  mcManager->AddFile(inFile);
  run->AddTask(mcManager);
  // ------------------------------------------------------------------------
	

  // =========================================================================
  // ===             Detector Response Simulation (Digitiser)              ===
  // ===                          (where available)                        ===
  // =========================================================================


  // -----   MVD Digitiser   -------------------------------------------------
  CbmMvdDigitizer* mvdDigitise = new CbmMvdDigitizer("MVD Digitiser", 0, iVerbose);
  run->AddTask(mvdDigitise);
  // -------------------------------------------------------------------------

  // -----   MVD Clusterfinder   ---------------------------------------------
  CbmMvdClusterfinder* mvdCluster = new CbmMvdClusterfinder("MVD Clusterfinder", 0, iVerbose);
  run->AddTask(mvdCluster);
  // -------------------------------------------------------------------------


// -----   STS digitizer   -------------------------------------------------
  // -----   The parameters of the STS digitizer are set such as to match
  // -----   those in the old digitizer. Change them only if you know what you
  // -----   are doing.
  Double_t dynRange       =   40960.;  // Dynamic range [e]
  Double_t threshold      =    4000.;  // Digitisation threshold [e]
  Int_t nAdc              =    4096;   // Number of ADC channels (12 bit)
  Double_t timeResolution =       5.;  // time resolution [ns]
  Double_t deadTime       = 9999999.;  // infinite dead time (integrate entire event)
  Double_t noise          =       0.;  // ENC [e]
  Int_t digiModel         =       1;   // User sensor type DSSD
	
  // The following settings correspond to a validated implementation. 
  // Changing them is on your own risk.
  Int_t  eLossModel       = 1;         // Energy loss model: uniform 
  Bool_t useLorentzShift  = kFALSE;    // Deactivate Lorentz shift
  Bool_t useDiffusion     = kFALSE;    // Deactivate diffusion
  Bool_t useCrossTalk     = kFALSE;    // Deactivate cross talk

  CbmStsDigitize* stsDigi = new CbmStsDigitize(digiModel);
  stsDigi->SetProcesses(eLossModel, useLorentzShift, useDiffusion, useCrossTalk);
  stsDigi->SetParameters(dynRange, threshold, nAdc, timeResolution,
  		                 deadTime, noise);
  run->AddTask(stsDigi);
  // -------------------------------------------------------------------------



  // =========================================================================
  // ===                     MVD local reconstruction                      ===
  // =========================================================================

  // -----   MVD Hit Finder   ------------------------------------------------
  CbmMvdHitfinder* mvdHitfinder = new CbmMvdHitfinder("MVD Hit Finder", 0, iVerbose);
  mvdHitfinder->UseClusterfinder(kTRUE);
  run->AddTask(mvdHitfinder);
  // -------------------------------------------------------------------------

  // ===                 End of MVD local reconstruction                   ===
  // =========================================================================




  // =========================================================================
  // ===                      STS local reconstruction                     ===
  // =========================================================================


  // -----   STS Cluster Finder   --------------------------------------------
  FairTask* stsClusterFinder = new CbmStsFindClusters();
  run->AddTask(stsClusterFinder);
  // -------------------------------------------------------------------------


  // -----   STS hit finder   ------------------------------------------------
  FairTask* stsFindHits = new CbmStsFindHits();
  run->AddTask(stsFindHits);
  // -------------------------------------------------------------------------


  // -----  STS hit matching   -----------------------------------------------
//  FairTask* stsMatchHits = new CbmStsMatchHits();
//  run->AddTask(stsMatchHits);
  // -------------------------------------------------------------------------


  // ---  STS track finding   ------------------------------------------------
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
  // -------------------------------------------------------------------------


  // ---   STS track matching   ----------------------------------------------
//  FairTask* stsMatchTracks = new CbmStsMatchTracks(iVerbose);
//  run->AddTask(stsMatchTracks);
  // -------------------------------------------------------------------------


  // ---   STS track fitting   -----------------------------------------------
  //CbmStsTrackFitter* stsTrackFitter = new CbmStsKFTrackFitter();
  //FairTask* stsFitTracks = new CbmStsFitTracks(stsTrackFitter, iVerbose);
  //run->AddTask(stsFitTracks);
  // -------------------------------------------------------------------------

  // ===                 End of STS local reconstruction                   ===
  // =========================================================================




  // =========================================================================
  // ===                     TRD local reconstruction                      ===
  // =========================================================================

  Bool_t  simpleTR  = kTRUE;  // use fast and simple version for TR production
  CbmTrdRadiator *radiator = new CbmTrdRadiator(simpleTR,"K++");
  //"K++" : micro structured POKALON
  //"H++" : PE foam foils
  //"G30" : ALICE fibers 30 layers

  Bool_t triangularPads = false;// Bucharest triangular pad-plane layout
  //Double_t triggerThreshold = 0.5e-6;//SIS100
  Double_t triggerThreshold = 1.0e-6;//SIS300
  Double_t trdNoiseSigma_keV = 0.1; //default best matching to test beam PRF

  CbmTrdDigitizerPRF* trdDigiPrf = new CbmTrdDigitizerPRF(radiator);
  trdDigiPrf->SetTriangularPads(triangularPads);
  trdDigiPrf->SetNoiseLevel(trdNoiseSigma_keV);
  run->AddTask(trdDigiPrf);

  CbmTrdClusterFinderFast* trdCluster = new CbmTrdClusterFinderFast();
  trdCluster->SetNeighbourTrigger(true);
  trdCluster->SetTriggerThreshold(triggerThreshold);
  trdCluster->SetNeighbourRowTrigger(false);
  trdCluster->SetPrimaryClusterRowMerger(true);
  trdCluster->SetTriangularPads(triangularPads);
  run->AddTask(trdCluster);

  CbmTrdHitProducerCluster* trdHit = new CbmTrdHitProducerCluster();
  trdHit->SetTriangularPads(triangularPads);
  run->AddTask(trdHit);

  // -------------------------------------------------------------------------
  // ===                 End of TRD local reconstruction                   ===
  // =========================================================================


  // =========================================================================
  // ===                     TOF local reconstruction                      ===
  // =========================================================================


  // ------   TOF hit producer   ---------------------------------------------
  CbmTofHitProducerNew* tofHitProd = new CbmTofHitProducerNew("TOF HitProducerNew",iVerbose); 
  tofHitProd->SetInitFromAscii(kFALSE);
  run->AddTask(tofHitProd);
  // -------------------------------------------------------------------------

  // ===                   End of TOF local reconstruction                 ===
  // =========================================================================




  // =========================================================================
  // ===                        Global tracking                            ===
  // =========================================================================

  CbmLitFindGlobalTracks* finder = new CbmLitFindGlobalTracks();
  // Tracking method to be used
  // "branch" - branching tracking
  // "nn" - nearest neighbor tracking
  // "weight" - weighting tracking
  finder->SetTrackingType("branch");

  // Hit-to-track merger method to be used
  // "nearest_hit" - assigns nearest hit to the track
  finder->SetMergerType("nearest_hit");

  run->AddTask(finder);

  // -----   Primary vertex finding   ---------------------------------------
  CbmPrimaryVertexFinder* pvFinder = new CbmPVFinderKF();
  CbmFindPrimaryVertex* findVertex = new CbmFindPrimaryVertex(pvFinder);
  run->AddTask(findVertex);
  // ------------------------------------------------------------------------

  // ===                      End of global tracking                       ===
  // =========================================================================



  // ----------- TRD track Pid Wkn ----------------------
  CbmTrdSetTracksPidWkn* trdSetTracksPidTask = new CbmTrdSetTracksPidWkn(
  		"trdFindTracks", "trdFindTracks");
  run->AddTask(trdSetTracksPidTask);
  // ----------------------------------------------------

  // ----------- TRD track Pid Ann ----------------------
  CbmTrdSetTracksPidANN* trdSetTracksPidAnnTask = new CbmTrdSetTracksPidANN(
  		"Ann", "Ann");
  run->AddTask(trdSetTracksPidAnnTask);
  // ----------------------------------------------------

  // ----------- TRD track Pid Like ----------------------
  // Since in the newest version of this method depends on the global
  // track the task has to move after the global tracking
// FU 08.02.12 Switch the task off since the input file needed for the new geometry has to be generated first.
//  CbmTrdSetTracksPidLike* trdSetTracksPidLikeTask =
//  		new CbmTrdSetTracksPidLike("Likelihood", "Likelihood");
//  run->AddTask(trdSetTracksPidLikeTask);
  // ----------------------------------------------------


  // =========================================================================
  // ===                        RICH reconstruction                        ===
  // =========================================================================

  if (richGeom.Length() != 0)  // if RICH is defined
    {
  // ---------------------RICH Hit Producer ----------------------------------
  CbmRichHitProducer* richHitProd  = new CbmRichHitProducer();
  richHitProd->SetDetectorType(4);
  richHitProd->SetNofNoiseHits(220);
  richHitProd->SetCollectionEfficiency(1.0);
  richHitProd->SetSigmaMirror(0.06);
  run->AddTask(richHitProd);
  //--------------------------------------------------------------------------

  //--------------------- RICH Reconstruction ----------------------------------
  CbmRichReconstruction* richReco = new CbmRichReconstruction();
  run->AddTask(richReco);

  // ------------------- RICH Ring matching  ---------------------------------
  CbmRichMatchRings* matchRings = new CbmRichMatchRings();
  run->AddTask(matchRings);
  // -------------------------------------------------------------------------
    }
  // ===                 End of RICH local reconstruction                  ===
  // =========================================================================


/*
  // =========================================================================
  // ===                        ECAL reconstruction                        ===
  // =========================================================================

  // -----   ECAL hit producer  ----------------------------------------------
  CbmEcalHitProducerFastMC* ecalHitProd = new CbmEcalHitProducerFastMC(
  		"ECAL Hitproducer");
  run->AddTask(ecalHitProd);
  // -------------------------------------------------------------------------

  // ===                      End of ECAL reconstruction                   ===
  // =========================================================================

*/

  // =========================================================================
  // ===                    Matching to Monte-carlo                        ===
  // =========================================================================
  CbmMatchRecoToMC* matchTask = new CbmMatchRecoToMC();
  run->AddTask(matchTask);
  // ===                  End of matching to Monte-Carlo                   ===
  // =========================================================================

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

  //  delete run;

  cout << " Test passed" << endl;
  cout << " All ok " << endl;

  // Function needed for CTest runtime dependency
  Generate_CTest_Dependency_File(depFile);
  
	Int_t analyseMode = mode;
	ofstream outputfile("log.txt", std::ofstream::app);
	if(!outputfile) {
		cout << "Error!" << endl;
	}
	else {
		TTimeStamp testtime;
		outputfile << "########## run_reco.C ##########" << endl;
		outputfile << "Date (of end): " << testtime.GetDate() << "\t Time (of end): " << testtime.GetTime() << " +2" << endl;
		outputfile << "Output file is "    << outFile << endl;
		outputfile << "Parameter file is " << parFile << endl;
		outputfile << "Number of events: " << nEvents << "\t mode: " << analyseMode << endl;
		outputfile << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl << endl;
		outputfile.close();
	}
}
