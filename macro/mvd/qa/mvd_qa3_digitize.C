// --------------------------------------------------------------------------
//
// Macro for local MVD reconstruction from MC data
//
// Tasks:  CbmMvdHitProducer
// 
//
// V. Friese   06/02/2007
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
Double_t fieldScale=0.;
Int_t    fieldSymType=0;

TString defaultInputFile="";


void mvd_qa3_digitize( const char* setup = "sis100_electron")
{

    // ========================================================================
    //          Adjust this part according to your requirements

    TString inDir   = gSystem->Getenv("VMCWORKDIR");

    TString outDir="data/";
    // Input file (MC events)
    TString inFile = outDir+ "mvd.mcQA.root";

    // Parameter file name
    TString parFile =outDir+ "params.root";

    // Output file
    TString outFile =outDir+ "mvd.recoQA.root";

    // Background file (MC events, for pile-up)
    TString bgFile = inFile;

    // Delta file (Au Ions)
    TString deltaFile =outDir+ "mvd.mcDelta.root";

    // Number of events to process
    Int_t nEvents = 5;

    // Verbosity level (0=quiet, 1=event level, 2=track level, 3=debug)
    Int_t iVerbose = 0;

     FairLogger* logger = FairLogger::GetLogger();
  logger->SetLogScreenLevel("INFO");
  logger->SetLogVerbosityLevel("LOW");

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
    FairRunAna *fRun= new FairRunAna();
    fRun->SetInputFile(inFile);
    fRun->SetOutputFile(outFile);
    // ------------------------------------------------------------------------

      // ----- Mc Data Manager   ------------------------------------------------
  CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
  mcManager->AddFile(inFile);
  fRun->AddTask(mcManager);
  // ------------------------------------------------------------------------
  

    // -------   MVD Digitiser   ----------------------------------------------
    CbmMvdDigitizer* digi = new CbmMvdDigitizer("MVDDigitiser", 0, iVerbose);
    cout << "Adding Task:  CbmMvdDigitiser... " << endl;
    fRun->AddTask(digi);
    
    //--- Pile Up -------
    
    Int_t pileUpInMVD=2; // use 1 or higher
    
    digi->SetBgFileName(bgFile);
    digi->SetBgBufferSize(5); //for simulation this buffer should contain > 2 * pileUpInMVD mBias events.
			      //default = few 100 (increase for high pile-up, reduce to limit memory consumption)
    digi->SetPileUp(pileUpInMVD-1);


    //--- Delta electrons -------
    digi->SetDeltaName(deltaFile);
    digi->SetDeltaBufferSize(50); //for simulation, this buffer must contain at least pileUpInMVD*200 delta electrons
    digi->SetDeltaEvents(2); //valid for QA-purposes only, use next line for simulations
    //digi->SetDeltaEvents(pileUpInMVD*100); // for simulation assumes 1% target

    //digi->ShowDebugHistograms();
    

  // -----   MVD Clusterfinder   ---------------------------------------------
  CbmMvdClusterfinder* mvdCluster = new CbmMvdClusterfinder("MVD Clusterfinder", 0, iVerbose);
  fRun->AddTask(mvdCluster);
  // -------------------------------------------------------------------------

    
    CbmMvdHitfinder* mvd_hit   = new CbmMvdHitfinder("MVDFindHits", 0, iVerbose);
    mvd_hit->UseClusterfinder(kTRUE);
    fRun->AddTask(mvd_hit);

    //----------------------------------------------------------------------------
    // -----  Parameter database   -----------------------------------------------
    FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
    FairParRootFileIo*  parIo1 = new FairParRootFileIo();
    parIo1->open(parFile.Data());

    rtdb->setFirstInput(parIo1);
    rtdb->setOutput(parIo1);
    rtdb->saveOutput();
    rtdb->print();
    // ---------------------------------------------------------------------------



    // -----   Run initialisation   ----------------------------------------------
    fRun->Init();
    // ---------------------------------------------------------------------------



    // -----   Start run   -------------------------------------------------------
    fRun->Run(0,nEvents);
    // ---------------------------------------------------------------------------



    // -----   Finish   ----------------------------------------------------------
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    cout << endl << endl;
    cout << "Macro finished succesfully." << endl;
    cout << "Output file is "    << outFile << endl;
    cout << "Parameter file is " << parFile << endl;
    cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
    cout << endl;
    // ---------------------------------------------------------------------------

  cout << " Test passed" << endl;
  cout << " All ok " << endl;
}
