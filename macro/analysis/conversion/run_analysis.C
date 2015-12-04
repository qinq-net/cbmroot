

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




void run_analysis(Int_t nEvents = 2, Int_t mode = 0, Int_t file_nr = 1, const char* inputsetup = "", TString output = "")
	// mode 1 = tomography
	// mode 2 = urqmd
	// mode 3 = pluto
{
	if(mode == 0) {
		cout << "ERROR: No mode specified! Exiting..." << endl;
		exit();
	}

	Int_t iVerbose = 0;
	//FairLogger* logger = FairLogger::GetLogger();
	//logger->SetLogScreenLevel("INFO");
	//logger->SetLogVerbosityLevel("LOW");

	Char_t filenr[5];
	sprintf(filenr,"%05d",file_nr);
	printf("Filenr: %s\n", filenr);
	TString temp = filenr;
//	TString temp = "00003";

	TString script = TString(gSystem->Getenv("SCRIPT"));
	TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters");
//	TString stsMatBudgetFileName = parDir + "/sts/sts_matbudget_v13d.root"; // Material budget file for L1 STS tracking

	gRandom->SetSeed(10);

	TString dir = 			"";
	TString mcFile = 		"";
	TString parFile = 		"";
	TString recoFile =		"";
	TString analysisFile =	"";


	TString outName;
	if(output == "") {
		outName = "sis300electron_test";
	}
	else {
		outName = output;
	}
	if(mode == 1) {	// tomography
		dir =	"/common/home/reinecke/CBM-Simulationen/outputs/tomography/" + outName;
		mcFile =		dir + "/tomography." + outName + ".mc.0001.root";
		parFile =		dir + "/tomography." + outName + ".param.0001.root";
		recoFile =		dir + "/tomography." + outName + ".reco.0001.root";
		analysisFile =	dir + "/tomography." + outName + ".analysis.0001.root";
	}
	if(mode == 2) {	// urqmd
		dir = "/hera/cbm/users/sreinecke/outputs/urqmd/" + outName;
		//TString dir =	"/common/home/reinecke/CBM-Simulationen/outputs/urqmd/" + outName;
		//dir = "/common/cbmb/Users/reinecke/simulations/outputs/urqmd/" + outName;
		mcFile =		dir + "/urqmd." + outName + ".mc." + temp + ".root";
		parFile =		dir + "/urqmd." + outName + ".param." + temp + ".root";
		recoFile =		dir + "/urqmd." + outName + ".reco." + temp + ".root";
		analysisFile =	dir + "/urqmd." + outName + ".analysis." + temp + ".root";
	}
	if(mode == 3) {	// pluto
		dir =	"/common/home/reinecke/CBM-Simulationen/outputs/pluto/" + outName;
		mcFile =		dir + "/pluto." + outName + ".mc.0001.root";
		parFile =		dir + "/pluto." + outName + ".param.0001.root";
		recoFile =		dir + "/pluto." + outName + ".reco.0001.root";
		analysisFile =	dir + "/pluto." + outName + ".analysis.0001.root";
	}
	if(mode == 4) {	// pi0only
		TString dir		= "/common/cbmb/Users/reinecke/simulations/outputs/pi0only/" + outName;
		parFile			= dir + "/pi0only." + outName + ".param." + temp + ".root";
		mcFile			= dir + "/pi0only." + outName + ".mc." + temp + ".root";
		recoFile		= dir + "/pi0only." + outName + ".reco." + temp + ".root";
		analysisFile	= dir + "/pi0only." + outName + ".analysis." + temp + ".root";
	}



	TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters");
	TList *parFileList = new TList();
/*	TObjString stsDigiFile = parDir + "/sts/sts_v13d_std.digi.par"; // STS digi file
	TObjString trdDigiFile = parDir + "/trd/trd_v13g.digi.par"; // TRD digi file
	TObjString tofDigiFile = parDir + "/tof/tof_v13b.digi.par"; // TOF digi file

	parFileList->Add(&stsDigiFile);
	parFileList->Add(&trdDigiFile);
	parFileList->Add(&tofDigiFile);
*/




	const char* setup;
	if(inputsetup == "") {
		setup = "sis300_electron";
	}
	else {
		setup = inputsetup;
	}
//	TString inDir = "../../../";
	TString inDir = "/common/home/reinecke/cbma-software/simu_apr14/cbmroot_jul14";
	TString paramDir = inDir + "/parameters/";
	TString setupFile = dir + "/" + setup + "_setup.C";
	TString setupFunct = setup;
	setupFunct += "_setup()";

	gROOT->LoadMacro(setupFile);
	gInterpreter->ProcessLine(setupFunct);

	//TObjString stsDigiFile = paramDir + stsDigi;
	//parFileList->Add(&stsDigiFile);
	//cout << "macro/analysis/conversion/run_analysis.C using: " << stsDigi << endl;

	TObjString trdDigiFile = paramDir + trdDigi;
	parFileList->Add(&trdDigiFile);
	cout << "macro/analysis/conversion/run_analysis.C using: " << trdDigi << endl;

	TObjString tofDigiFile = paramDir + tofDigi;
	parFileList->Add(&tofDigiFile);
	cout << "macro/analysis/conversion/run_analysis.C using: " << tofDigi << endl;



	gDebug = 0;

	TStopwatch timer;
	timer.Start();

	// ----  Load libraries   -------------------------------------------------
//	gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
//	loadlibs();
//	gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/determine_setup.C");

	// -----   Reconstruction run   -------------------------------------------
	FairRunAna *run= new FairRunAna();
	run->SetInputFile(mcFile);
	run->SetOutputFile(analysisFile);
	run->SetGenerateRunInfo(kTRUE);       // Create FairRunInfo file
	run->AddFriend(recoFile);



  CbmKF* kalman = new CbmKF();
  run->AddTask(kalman);
  CbmL1* l1 = new CbmL1();
  TString mvdMatBudgetFileName = paramDir + mvdMatBudget;
  TString stsMatBudgetFileName = paramDir + stsMatBudget;
  l1->SetStsMaterialBudgetFileName(stsMatBudgetFileName.Data());
  l1->SetMvdMaterialBudgetFileName(mvdMatBudgetFileName.Data());
  run->AddTask(l1);


// ##########################################################################################################
	// ----- PID for KF Particle Finder --------------------------------------------
	Int_t kf_useMC = 1;
	CbmKFParticleFinderPID* kfParticleFinderPID = new CbmKFParticleFinderPID();
	if(kf_useMC == 1) {
		kfParticleFinderPID->SetPIDMode(1); 	// use MC-true PID
	}
	else {
		kfParticleFinderPID->SetPIDMode(2); 	// use reco PID
		kfParticleFinderPID->UseRICHRvspPID();	// use RICH PID
		kfParticleFinderPID->DoNotUseTRD();		// do not use TRD PID
		kfParticleFinderPID->SetSIS100();		// use SIS100 mode
	}
	run->AddTask(kfParticleFinderPID);
  
	// ----- KF Particle Finder --------------------------------------------
	CbmKFParticleFinder* kfParticleFinder = new CbmKFParticleFinder();
	kfParticleFinder->SetPIDInformation(kfParticleFinderPID);
	run->AddTask(kfParticleFinder);

	// ----- KF Particle Finder QA --------------------------------------------
	CbmKFParticleFinderQA* kfParticleFinderQA = new CbmKFParticleFinderQA("CbmKFParticleFinderQA", 0, kfParticleFinder->GetTopoReconstructor(),"");
	kfParticleFinderQA->SetPrintEffFrequency(200);
	TString effFileName = analysisFile;
	effFileName += ".eff_mc.txt";
	kfParticleFinderQA->SetEffFileName(effFileName.Data());
	kfParticleFinderQA->SaveMCParticles(true);
	run->AddTask(kfParticleFinderQA);
// ##########################################################################################################






// ##########################################################################################################
	// ----- PID for KF Particle Finder --------------------------------------------
/*	kf_useMC = 0;
	CbmKFParticleFinderPID* kfParticleFinderPID_reco = new CbmKFParticleFinderPID("CbmKFParticleFinderPID_reco");
	if(kf_useMC == 1) {
		kfParticleFinderPID_reco->SetPIDMode(1); 	// use MC-true PID
	}
	else {
		kfParticleFinderPID_reco->SetPIDMode(2); 	// use reco PID
		kfParticleFinderPID_reco->UseRICHRvspPID();	// use RICH PID
		kfParticleFinderPID_reco->DoNotUseTRD();		// do not use TRD PID
		kfParticleFinderPID_reco->SetSIS100();		// use SIS100 mode
	}
	run->AddTask(kfParticleFinderPID_reco);
  
	// ----- KF Particle Finder --------------------------------------------
	CbmKFParticleFinder* kfParticleFinder_reco = new CbmKFParticleFinder("CbmKFParticleFinder_reco");
	kfParticleFinder_reco->SetPIDInformation(kfParticleFinderPID_reco);
	run->AddTask(kfParticleFinder_reco);

	// ----- KF Particle Finder QA --------------------------------------------
	CbmKFParticleFinderQA* kfParticleFinderQA_reco = new CbmKFParticleFinderQA("CbmKFParticleFinderQA_reco", 0, kfParticleFinder_reco->GetTopoReconstructor(),"");
	kfParticleFinderQA_reco->SetPrintEffFrequency(200);
	TString effFileName_reco = analysisFile;
	effFileName_reco += ".eff_mc_reco.txt";
	kfParticleFinderQA_reco->SetEffFileName(effFileName_reco.Data());
	kfParticleFinderQA_reco->SaveMCParticles(true);
	run->AddTask(kfParticleFinderQA_reco);
*/
// ##########################################################################################################




 
   
 
	// ----- Conversion Analysis --------------------------------------------   
	CbmAnaConversion* conversionAna = new CbmAnaConversion();
	conversionAna->SetMode(mode);
	conversionAna->SetKF(kfParticleFinder, kfParticleFinderQA);
	run->AddTask(conversionAna);



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

	run->Init();
	cout << "\n\n" << endl;
	cout << "######### START ##########" << endl;
	cout << "Starting run" << endl;
	run->Run(0,nEvents);
    
	cout << "\n\n" << endl;
	cout << "######## RESULTS #########" << endl;
//	cout << "Test: " << conversionAna->GetTest() << endl;
//	cout << "Nof Events: " << conversionAna->GetNofEvents() << endl;
//	cout << "Fraction: " <<  1.0 * conversionAna->GetTest() / conversionAna->GetNofEvents() << endl;

	// -----   Finish   -------------------------------------------------------
	timer.Stop();
	Double_t rtime = timer.RealTime();
	Double_t ctime = timer.CpuTime();
	cout << endl << endl;
	cout << "######## Finishing run_analysis script ########" << endl;
	cout << "Macro finished successfully." << endl;
	cout << "Output file is "    << analysisFile << endl;
	cout << "Parameter file is " << parFile << endl;
	cout << std::fixed;
	//cout << std::setprecision(1);
	cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
	cout << endl;

	cout << " Test passed" << endl;
	cout << " All ok " << endl;

	Int_t analyseMode = mode;
	ofstream outputfile("log.txt", mode = ios_base::app);
	if(!outputfile) {
		cout << "Error!" << endl;
	}
	else {
		TTimeStamp testtime;
		outputfile << "########## run_analysis.C ##########" << endl;
		outputfile << "Date (of end): " << testtime.GetDate() << "\t Time (of end): " << testtime.GetTime() << " +2" << endl;
		outputfile << "Output file is "    << analysisFile << endl;
		outputfile << "Parameter file is " << parFile << endl;
		outputfile << "Number of events: " << nEvents << "\t mode: " << analyseMode << endl;
		outputfile << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl << endl;
		outputfile.close();
	}
}
