/////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Analysis macro for MUCH									       //
//									                               //
//      Authors: Partha Pratim Bhaduri (partha.bhaduri@vecc.gov.in), Omveer Singh (omveer.rs@amu.ac.in)//
//      and Ekata Nandy (ekata@vecc.gov.in)			                                       //
//												       //
//												       //
/////////////////////////////////////////////////////////////////////////////////////////////////////////


void much_analysis(
     Int_t nEvents=2, 
     Int_t nSig=1,    //No of Signal Di-Moun 
     TString dataset = "test",
     TString setup = "sis100_muon_lmvm"
)
{
 

// --- Logger settings ----------------------------------------------------
  TString logLevel     = "INFO";
  TString logVerbosity = "LOW";
// ------------------------------------------------------------------------



// -----   In- and output file names   ------------------------------------
  TString mcFile  = dataset + ".tra.root";
  TString parFile = dataset + ".par.root";
  TString recFile = dataset + ".rec.root";
  TString outFile = dataset + ".analysis.root";
  
//-------------------------------------------------------------------------



// -----   Environment   --------------------------------------------------
  TString myName = "much_analysis";  // this macro's name for screen output
  TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory
  // ------------------------------------------------------------------------



 // -----   Load the geometry setup   -------------------------------------
  std::cout << std::endl;
  TString setupFile = srcDir + "/geometry/setup/setup_" + setup + ".C";
  TString setupFunct = "setup_";
  setupFunct = setupFunct + setup + "()";
  std::cout << "-I- " << myName << ": Loading macro " << setupFile << std::endl;
  gROOT->LoadMacro(setupFile);
  gROOT->ProcessLine(setupFunct);

//-------------------------------------------------------------------------



// ------   Parameter files as input to the runtime database   -------------
  std::cout << std::endl;
  std::cout << "-I- " << myName << ": Defining parameter files " << std::endl;
  TList *parFileList = new TList();
  TString geoTag, parFileMuch, parFileSts;
//--------------------------------------------------------------------------




// -----   Much digitisation parameters   ---------------------------------

 if ( CbmSetup::Instance()->GetGeoTag(kMuch, geoTag) ) {
                Int_t muchFlag=0;
		if (geoTag.Contains("mcbm")) muchFlag=1;
                std::cout << geoTag(0,4) << std::endl;
	        parFileMuch = gSystem->Getenv("VMCWORKDIR");
		parFileMuch = parFileMuch + "/parameters/much/much_" + geoTag(0,4)
				    + "_digi_sector.root";
		std::cout << "Using MUCH parameter file " << parFileMuch << std::endl;
}

//-------------------------------------------------------------------------





// -----   STS digitisation parameters   ---------------------------------

 if (CbmSetup::Instance()->GetGeoTag(kSts, geoTag) ) {
    parFileSts = gSystem->Getenv("VMCWORKDIR");
    parFileSts = parFileSts + "/parameters/sts/sts_matbudget_" + geoTag + ".root";
    std::cout << "Using  STS material budget file " << parFileSts << std::endl;
  }

//-------------------------------------------------------------------------


// -----   FairRunAna   --------------------------------------------------- 
  FairRunAna *fRun= new FairRunAna();
  fRun->SetInputFile(mcFile);
  fRun->AddFriend(recFile);
  fRun->SetOutputFile(outFile);


 // -----   Logger settings   ----------------------------------------------
  FairLogger::GetLogger()->SetLogScreenLevel(logLevel.Data());
  FairLogger::GetLogger()->SetLogVerbosityLevel(logVerbosity.Data());
  // ------------------------------------------------------------------------


// -----  Parameter database   -------------------------------------------- 
  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
  FairParRootFileIo*  parIo1 = new FairParRootFileIo();
  parIo1->open(parFile);
  rtdb->setFirstInput(parIo1);
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
// ------------------------------------------------------------------------



//----------------- Track Finding in STS----------------------------------
  CbmKF* kf = new CbmKF();
  CbmL1* L1 = new CbmL1();
  L1->SetStsMaterialBudgetFileName(parFileSts.Data()); 
//-------------------------------------------------------------------------


//--------------------Dimuon Analysis -------------------------------------
    CbmAnaDimuonAnalysis* ana = new CbmAnaDimuonAnalysis(parFileMuch,nSig);
   
    //ana->SetChi2MuChCut(100);
    //ana->SetChi2VertexCut(3.5);
    //ana->SetNofMuchCut(0);
    //ana->SetNofStsCut(7);
      ana->SetVerbose(0);
      ana->SetStsPointsAccQuota(4);
      ana->SetStsTrueHitQuota(0.7);
//-------------------------------------------------------------------------


  fRun->AddTask(kf);
  fRun->AddTask(L1);
  fRun->AddTask(ana);


  // -----   Run initialisation   -------------------------------------------
  std::cout << std::endl;
  std::cout << "-I- " << myName << ": Initialise run" << std::endl;
  fRun->Init();
  // ------------------------------------------------------------------------



  // -----   Start run   ----------------------------------------------------
  std::cout << std::endl << std::endl;
  std::cout << "-I- " << myName << ": Starting run" << std::endl;
  fRun->Run(0, nEvents);
  // ------------------------------------------------------------------------

  std::cout << std::endl << std::endl;
  cout << "Macro finished successfully." << endl;
  cout << "Output file is "    << outFile << endl;
}


