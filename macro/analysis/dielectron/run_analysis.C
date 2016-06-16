/** run_analysis.C
 * @author Elena Lebedeva <e.lebedeva@gsi.de>
 * @since 2010
 * @version 1.0
 **/

void run_analysis(Int_t nEvents = 1000)
{
   TString script = TString(gSystem->Getenv("SCRIPT"));
   TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters");

   //gRandom->SetSeed(10);
/*	TString dir = "/hera/cbm/users/slebedev/mc/dielectron/sep13/25gev/trd/1.0field/nomvd/rho0/";
	TString mcFile = dir + "mc.auau.25gev.centr.00001.root";
	TString parFile = dir + "/params.auau.25gev.centr.00001.root";
	TString recoFile = dir + "/reco.auau.25gev.centr.00001.root";
	TString analysisFile = dir + "/test.analysis.test.auau.25gev.centr.00001.root";*/

   TString parFile = "/hera/cbm/users/slebedev/data/lmvm/apr16/8gev/geosetup_v1512_8gev/rho0/params.auau.8gev.centr.00002.root";
   TString mcFile = "/hera/cbm/users/slebedev/data/lmvm/apr16/8gev/geosetup_v1512_8gev/rho0/mc.auau.8gev.centr.00002.root";
   TString recoFile = "/hera/cbm/users/slebedev/data/lmvm/apr16/8gev/geosetup_v1512_8gev/rho0/reco.auau.8gev.centr.00002.root";
   TString analysisFile = "/hera/cbm/users/slebedev/data/lmvm/apr16/8gev/geosetup_v1512_8gev/rho0/analysis.auau.8gev.centr.00002.root";

	TString energy = "8gev";
	TString plutoParticle = "rho0";

   TString geoSetupFile = TString(gSystem->Getenv("VMCWORKDIR")) + "/macro/analysis/dielectron/geosetup/geosetup_v1512_8gev.C";

   if (script == "yes") {
      mcFile = TString(gSystem->Getenv("MC_FILE"));
      recoFile = TString(gSystem->Getenv("RECO_FILE"));
      parFile = TString(gSystem->Getenv("PAR_FILE"));
      analysisFile = TString(gSystem->Getenv("ANALYSIS_FILE"));
      energy = TString(gSystem->Getenv("ENERGY"));
      plutoParticle = TString(gSystem->Getenv("PLUTO_PARTICLE"));

      geoSetupFile = TString(gSystem->Getenv("VMCWORKDIR")) + "/macro/analysis/dielectron/geosetup/" + TString(gSystem->Getenv("GEO_SETUP_FILE"));
   }

    remove(analysisFile.Data());

   //setup all geometries from macro
   	cout << "geoSetupName:" << geoSetupFile << endl;
   	gROOT->LoadMacro(geoSetupFile);
   	init_geo_setup();

   	// digi parameters
   	TList *parFileList = new TList();
       // TObjString stsDigiFile = parDir + "/" + stsDigi;
   	TObjString trdDigiFile = parDir + "/" + trdDigi;
   	TObjString tofDigiFile = parDir + "/" + tofDigi;
       // parFileList->Add(&stsDigiFile);
   	if (trdDigiFile.GetString() != "") parFileList->Add(&trdDigiFile);
   	parFileList->Add(&tofDigiFile);

   	// material budget for STS and MVD
   	TString mvdMatBudgetFileName = "";
   	TString stsMatBudgetFileName = parDir + "/" + stsMatBudget;


   // load libraries
  // gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
  // loadlibs();
   //gSystem->Load("libAnalysis");
   gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/determine_setup.C");

   TStopwatch timer;
   timer.Start();

   FairRunAna* fRun = new FairRunAna();
   fRun->SetName("TGeant3");
   fRun->SetInputFile(mcFile);
   fRun->SetOutputFile(analysisFile);
   fRun->AddFriend(recoFile);

	CbmKF* kalman = new CbmKF();
	fRun->AddTask(kalman);
	CbmL1* l1 = new CbmL1();
	l1->SetStsMaterialBudgetFileName(stsMatBudgetFileName.Data());
	if (mvdMatBudgetFileName != "") l1->SetMvdMaterialBudgetFileName(mvdMatBudgetFileName.Data());
	fRun->AddTask(l1);


   CbmAnaDielectronTask *task = new CbmAnaDielectronTask();
   if (energy == "8gev" || energy == "10gev") {
      // weight rho0 = Multiplicity * Branching Ratio = 9 * 4.7e-5 for 10 AGeV beam energy
      if (plutoParticle == "rho0") task->SetWeight(9 * 4.7e-5);
      // weight omega = Multiplicity * Branching Ratio = 19 * 7.28e-5 for 10 AGeV beam energy
      if (plutoParticle == "omegaepem" ) task->SetWeight(19 * 7.28e-5);
      // weight omega = Multiplicity * Branching Ratio = 19 * 7.7e-4 for 10 AGeV beam energy
      if (plutoParticle == "omegadalitz") task->SetWeight(19 * 7.7e-4);
      // weight phi = Multipli0city * Branching Ratio = 0.12 * 2.97e-4 for 10 AGeV beam energy
      if (plutoParticle == "phi") task->SetWeight(0.12 * 2.97e-4);
      // weight in medium rho. 0.5 is a scaling factor for 8AGev from 25AGeV
      if (plutoParticle == "inmed") task->SetWeight(0.5 * 4.45e-2);
      // weight qgp radiation  0.5 is a scaling factor for 8AGev from 25AGeV
      if (plutoParticle == "qgp") task->SetWeight(0.5 * 1.15e-2);
   } else if (energy == "25gev") {
      // weight rho0 = Multiplicity * Branching Ratio = 23 * 4.7e-5 for 25 AGeV beam energy
      if (plutoParticle == "rho0") task->SetWeight(23 * 4.7e-5);
      // weight omega = Multiplicity * Branching Ratio = 38 * 7.28e-5 for 25 AGeV beam energy
      if (plutoParticle == "omegaepem" ) task->SetWeight(38 * 7.28e-5);
      // weight omega = Multiplicity * Branching Ratio = 38 * 7.7e-4 for 25 AGeV beam energy
      if (plutoParticle == "omegadalitz") task->SetWeight(38 * 7.7e-4);
      // weight phi = Multiplicity * Branching Ratio = 1.28 * 2.97e-4 for 25 AGeV beam energy
      if (plutoParticle == "phi") task->SetWeight(1.28 * 2.97e-4);
       // weight in medium rho.
      if (plutoParticle == "inmed") task->SetWeight(4.45e-2);
      // weight qgp radiation
      if (plutoParticle == "qgp") task->SetWeight(1.15e-2);
   } else if (energy == "3.5gev") {
      // weight rho0 = Multiplicity * Branching Ratio = 1.0 * 4.7e-5 for 25 AGeV beam energy
      if (plutoParticle == "rho0") task->SetWeight(1.0 * 4.7e-5);
      // weight omega = Multiplicity * Branching Ratio = 1.2 * 7.28e-5 for 25 AGeV beam energy
      if (plutoParticle == "omegaepem" ) task->SetWeight(1.2 * 7.28e-5);
      // weight omega = Multiplicity * Branching Ratio = 1.2 * 5.9e-4 for 25 AGeV beam energy
      if (plutoParticle == "omegadalitz") task->SetWeight(1.2 * 7.7e-5);
      // weight phi = Multiplicity * Branching Ratio = 0.1 * 2.97e-4 for 25 AGeV beam energy
      if (plutoParticle == "phi") task->SetWeight(0.1 * 2.97e-4);
   }
   //task->SetChiPrimCut(2.);
   //task->SetMomentumCut(momentumCut); // if cut < 0 then it is not used
   task->SetUseMvd(IsMvd(parFile));
   task->SetUseRich(true);
   task->SetUseTrd(IsTrd(parFile));
   task->SetUseTof(true);
   task->SetPionMisidLevel(pionMisidLevel);
   task->SetTrdAnnCut(trdAnnCut);

   fRun->AddTask(task);


	// -----  Parameter database   --------------------------------------------
	FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
	FairParRootFileIo* parIo1 = new FairParRootFileIo();
	FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
	parIo1->open(parFile.Data());
	parIo2->open(parFileList, "in");
	rtdb->setFirstInput(parIo1);
	rtdb->setSecondInput(parIo2);
	rtdb->setOutput(parIo1);
	rtdb->saveOutput();

	fRun->Init();
	fRun->Run(0, nEvents);

   timer.Stop();
   std::cout << "Macro finished succesfully." << std::endl;
   std::cout << "Output file is " << analysisFile << std::endl;
   std::cout << "Parameter file is " << parFile << std::endl;
   std::cout << "Real time " << timer.RealTime() << " s, CPU time " << timer.CpuTime() << " s" << std::endl;
   std::cout << " Test passed" << std::endl;
   std::cout << " All ok " << std::endl;
}
