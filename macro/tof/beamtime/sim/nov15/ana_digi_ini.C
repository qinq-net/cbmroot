// -----------------------------------------------------------------------------
// ----- ana_digi_ini.C                                                    -----
// -----                                                                   -----
// ----- adapted by C. Simon on 2018-06-02                                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

void ana_digi_ini(Int_t iNEvents, Int_t iCalMode, Int_t iCalSel, Int_t iCalRPC, Int_t iRefSel,
                  const TString& tUnpackDir, Int_t iDut, Int_t iMRef, Int_t iBRef, Double_t dDeadTime)
{
  // ---------------------------------------------------------------------------

  TString tLogLevel = "WARNING";
  TString tLogVerbosity = "LOW";
  gErrorIgnoreLevel = kWarning;
  gDebug = 0;

  TTree::SetMaxTreeSize(1000000000000LL); // [B] here: 1 TB

  // ---------------------------------------------------------------------------

  TStopwatch tTimer;
  tTimer.Start();

  Bool_t bHasFairMonitor = Has_Fair_Monitor();
  if(bHasFairMonitor)
  {
    FairMonitor::GetMonitor()->EnableMonitor(kTRUE);
  }

  // ---------------------------------------------------------------------------

  FairLogger::GetLogger()->SetLogScreenLevel(tLogLevel.Data());
  FairLogger::GetLogger()->SetLogVerbosityLevel(tLogVerbosity.Data());

  // ---------------------------------------------------------------------------

  TString tSrcDir   = gSystem->Getenv("VMCWORKDIR");
  TString tMacroDir = tSrcDir + "/macro/beamtime";
  TString tParamDir = tSrcDir + "/parameters";
  TString tGeoDir   = tSrcDir + "/geometry";

  TString tTofGeoVersion = "v15e";

  TString tInputFile            = tUnpackDir + "/data/unpack.out.root";
  TString tOutputFile           = "./digi_ini.out.root";
  TString tHistoFile            = "./calib_cluster_ini.hst.root";
  TString tInputClustCalibFile  = "./calib_cluster.cor_in.root";
  TString tOutputClustCalibFile = "./calib_cluster.cor_out.root";

  // ---------------------------------------------------------------------------

  TList* tParFileList = new TList();

	TObjString* tTofDigiParFile    = new TObjString(tParamDir + "/tof/tof_"+ tTofGeoVersion + ".digi.par");
	TObjString* tTofDigiBdfParFile = new TObjString(tParamDir + "/tof/tof_"+ tTofGeoVersion + ".digibdf.par");

	tParFileList->Add(tTofDigiParFile);
	tParFileList->Add(tTofDigiBdfParFile);

  // ---------------------------------------------------------------------------

  TFile::Open(tGeoDir + "/tof/geofile_tof_" + tTofGeoVersion + ".root");

  gFile->Get("FAIRGeom");
  if(!gGeoManager)
  {
    std::cout << "-E- ana_digi_ini: FAIRGeom not found in geometry file!" << std::endl;
    return;
  }

  // ---------------------------------------------------------------------------

  FairFileSource* tFileSource = new FairFileSource(tInputFile);

  // ---------------------------------------------------------------------------

  FairRunAna* tRun= new FairRunAna();
  tRun->SetSource(tFileSource);
  tRun->SetOutputFile(tOutputFile);

  // ---------------------------------------------------------------------------

  Int_t iSet = 1000*iDut + iMRef;

  Int_t iDutCounter = iDut%10;
  Int_t iDutModule  = ((iDut - iDutCounter)/10)%10;
  Int_t iDutType    = (iDut - iDutCounter - 10*iDutModule)/100;

  Int_t iMRefCounter = iMRef%10;
  Int_t iMRefModule  = ((iMRef - iMRefCounter)/10)%10;
  Int_t iMRefType    = (iMRef - iMRefCounter - 10*iMRefModule)/100;

  Int_t iBRefCounter = iBRef%10;
  Int_t iBRefModule  = ((iBRef - iBRefCounter)/10)%10;
  Int_t iBRefType    = (iBRef - iBRefCounter - 10*iBRefModule)/100;

  // ---------------------------------------------------------------------------

  CbmTofTestBeamClusterizer* tTofTestBeamClust = new CbmTofTestBeamClusterizer("TOF TestBeam Clusterizer", 1, kFALSE);
  tTofTestBeamClust->SetPs2Ns(kFALSE);
  tTofTestBeamClust->SetCalMode(iCalMode);
  tTofTestBeamClust->SetCalSel(iCalSel);
  tTofTestBeamClust->SetCaldXdYMax(5.);
  tTofTestBeamClust->SetCalCluMulMax(20.);  
  tTofTestBeamClust->SetCalRpc(iCalRPC); 
  tTofTestBeamClust->SetTRefId(iRefSel);
  tTofTestBeamClust->SetTotMax(10.);
  tTofTestBeamClust->SetTotMin(0.01);
  tTofTestBeamClust->SetTotPreRange(5.);
  tTofTestBeamClust->SetTotMean(2.);
  tTofTestBeamClust->SetMaxTimeDist(0.5);
  //tTofTestBeamClust->SetMaxTimeDist(0.);
  //tTofTestBeamClust->SetDelTofMax(3.);
  tTofTestBeamClust->SetBeamRefMulMax(4);
  tTofTestBeamClust->SetBeamAddRefMul(-1);
  tTofTestBeamClust->SetChannelDeadtime(dDeadTime);
  tTofTestBeamClust->SetYFitMin(1000.);
  tTofTestBeamClust->SetMemoryTime(0.);

  tTofTestBeamClust->SetDutId(iDutType);
  tTofTestBeamClust->SetDutSm(iDutModule);
  tTofTestBeamClust->SetDutRpc(iDutCounter);
  tTofTestBeamClust->SetSelId(iMRefType);
  tTofTestBeamClust->SetSelSm(iMRefModule);
  tTofTestBeamClust->SetSelRpc(iMRefCounter);
  tTofTestBeamClust->SetBeamRefId(iBRefType);
  tTofTestBeamClust->SetBeamRefSm(iBRefModule);
  tTofTestBeamClust->SetBeamRefDet(iBRefCounter);

  tTofTestBeamClust->SetCalParFileName(tInputClustCalibFile);
  tTofTestBeamClust->SetOutHstFileName(tOutputClustCalibFile);

  switch (iCalMode)
  {
    // initial calibration
    case 0:
      tTofTestBeamClust->SetTotMax(100.);
      //tTofTestBeamClust->SetTotMin(1.);
      //tTofTestBeamClust->SetTotOff(0.);
      //tTofTestBeamClust->SetTRefDifMax(2000000.);
      tTofTestBeamClust->SetTRefDifMax(3000000000.);
      tTofTestBeamClust->PosYMaxScal(2000.);
      tTofTestBeamClust->SetMaxTimeDist(0.);
      break;

    // save offsets, update walks
    case 1: 
      tTofTestBeamClust->SetTRefDifMax(5.);
      tTofTestBeamClust->PosYMaxScal(1.0);
      break;
    case 11:
      tTofTestBeamClust->SetTRefDifMax(4.);
      tTofTestBeamClust->PosYMaxScal(1.0);
      break;   
    case 21:
      tTofTestBeamClust->SetTRefDifMax(3.);
      tTofTestBeamClust->PosYMaxScal(1.0);
      break;
    case 31:
      tTofTestBeamClust->SetTRefDifMax(2.);
      tTofTestBeamClust->PosYMaxScal(1.0);
      break;
    case 41:
      tTofTestBeamClust->SetTRefDifMax(1.);
      tTofTestBeamClust->PosYMaxScal(0.8);
      break;   
    case 51:
      tTofTestBeamClust->SetTRefDifMax(0.7);
      tTofTestBeamClust->PosYMaxScal(0.7);
      break;
    case 61:
      tTofTestBeamClust->SetTRefDifMax(0.5);
      tTofTestBeamClust->PosYMaxScal(0.7);
      break;   
    case 71:
      tTofTestBeamClust->SetTRefDifMax(0.4);
      tTofTestBeamClust->PosYMaxScal(0.6);
      break;

    // time difference calibration
    case 2:
      tTofTestBeamClust->SetTRefDifMax(300.);
      tTofTestBeamClust->PosYMaxScal(1000.);
      break;

    // time offsets
    case 3:
      //tTofTestBeamClust->SetTRefDifMax(200000.);
      tTofTestBeamClust->SetTRefDifMax(300000000.);
      tTofTestBeamClust->PosYMaxScal(1000.);
      tTofTestBeamClust->SetMaxTimeDist(0.);
      break;
    case 12:
    case 13:
      //tTofTestBeamClust->SetTRefDifMax(100000.);
      tTofTestBeamClust->SetTRefDifMax(3000000.);
      tTofTestBeamClust->PosYMaxScal(100.);
      break;
    case 22:
    case 23:
      //tTofTestBeamClust->SetTRefDifMax(50000.);
      tTofTestBeamClust->SetTRefDifMax(300000.);
      tTofTestBeamClust->PosYMaxScal(50.);
      break;
    case 32:
    case 33:
      //tTofTestBeamClust->SetTRefDifMax(25000.);
      tTofTestBeamClust->SetTRefDifMax(30000.);
      tTofTestBeamClust->PosYMaxScal(20.);
      break;
    case 42:
    case 43:
      //tTofTestBeamClust->SetTRefDifMax(13000.);
      tTofTestBeamClust->SetTRefDifMax(3000.);
      tTofTestBeamClust->PosYMaxScal(10.);
      break;
    case 52:
    case 53:
      //tTofTestBeamClust->SetTRefDifMax(6000.);
      tTofTestBeamClust->SetTRefDifMax(400.);
      tTofTestBeamClust->PosYMaxScal(10.);
      break;
    case 62:
    case 63:
      tTofTestBeamClust->SetTRefDifMax(200.);
      tTofTestBeamClust->PosYMaxScal(10.);
      break;
    case 72:
    case 73:
      tTofTestBeamClust->SetTRefDifMax(2.);
      tTofTestBeamClust->PosYMaxScal(1.0);
      break;
    case 82:
    case 83:
      tTofTestBeamClust->SetTRefDifMax(1.);
      tTofTestBeamClust->PosYMaxScal(0.8);
      break;
    case 92:
    case 93:
      tTofTestBeamClust->SetTRefDifMax(0.6);
      tTofTestBeamClust->PosYMaxScal(0.8);
      break;

    // velocity dependence
    case 4:
      tTofTestBeamClust->SetTRefDifMax(6.);
      tTofTestBeamClust->PosYMaxScal(1.5);
      break;
    case 14:
      tTofTestBeamClust->SetTRefDifMax(2.5);
      tTofTestBeamClust->PosYMaxScal(1.);
      break;
    case 24:
      tTofTestBeamClust->SetTRefDifMax(1.);
      tTofTestBeamClust->PosYMaxScal(0.8);
      break;
    case 54:
      tTofTestBeamClust->SetTRefDifMax(0.7);
      tTofTestBeamClust->PosYMaxScal(0.7);
      break;
    case 64:
      tTofTestBeamClust->SetTRefDifMax(0.5);
      tTofTestBeamClust->PosYMaxScal(0.7);
      break;

    default:
      std::cout << "-E- ana_digi_ini: calibration mode not implemented!" << std::endl;
      return;
  }

  // ---------------------------------------------------------------------------

  tRun->AddTask(tTofTestBeamClust);

  // ---------------------------------------------------------------------------

  FairRuntimeDb* tRuntimeDb = tRun->GetRuntimeDb();
  FairParAsciiFileIo* tParIo1 = new FairParAsciiFileIo();
  tParIo1->open(tParFileList, "in");
  tRuntimeDb->setFirstInput(tParIo1);

  // ---------------------------------------------------------------------------

  std::cout << "-I- ana_digi_ini: initializing run..." << std::endl;
  tRun->Init();

  std::cout << "-I- ana_digi_ini: starting run..." << std::endl;
  tRun->Run(0, iNEvents);

  // ---------------------------------------------------------------------------

  gROOT->LoadMacro((tMacroDir + "/save_hst.C").Data());
  gROOT->LoadMacro((tMacroDir + "/fit_ybox.h").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_all_CluMul.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_all_CluRate.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_over_cluSel.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_over_clu.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_all_dTSel.C").Data());

  gInterpreter->ProcessLine("save_hst(\"" + tHistoFile + "\", kTRUE)");

  switch (iSet)
  {
    case 300400:
    case 300900:
    case 300921:
    case 400300:
    case 400900:
    case 900901:
    case 900921:
    case 901900:
    case 901921:
    case 910900:
    case 911921:
    case 920300:
    case 920921:
    case 921300:
    case 921920:
      gInterpreter->ProcessLine("pl_over_clu(3)");
      gInterpreter->ProcessLine("pl_over_clu(4)");
      gInterpreter->ProcessLine("pl_over_clu(5,0)");
      gInterpreter->ProcessLine("pl_over_clu(5,1)");
      gInterpreter->ProcessLine("pl_over_clu(5,2)");
      gInterpreter->ProcessLine("pl_over_clu(9,0,0)");
      gInterpreter->ProcessLine("pl_over_clu(9,0,1)");
      gInterpreter->ProcessLine("pl_over_clu(9,1,0)");
      gInterpreter->ProcessLine("pl_over_clu(9,1,1)");
      gInterpreter->ProcessLine("pl_over_clu(9,2,0)");
      gInterpreter->ProcessLine("pl_over_clu(9,2,1)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,3)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,4)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,5,0,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,5,1,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,5,2,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,9,0,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,9,0,1)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,9,1,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,9,1,1)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,9,2,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,9,2,1)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,3)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,4)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,5,0,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,5,1,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,5,2,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,9,0,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,9,0,1)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,9,1,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,9,1,1)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,9,2,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,9,2,1)");
      break;

    case 100600:
    case 100601:
    case 200600:
    case 200601:
    case 210200:
    case 600601:
    case 601600:
    case 700600:
    case 700601:
      gInterpreter->ProcessLine("pl_over_clu(1)");
      gInterpreter->ProcessLine("pl_over_clu(2,0,0)");
      gInterpreter->ProcessLine("pl_over_clu(2,0,1)");
      gInterpreter->ProcessLine("pl_over_clu(6,0,0)");
      gInterpreter->ProcessLine("pl_over_clu(6,0,1)");
      gInterpreter->ProcessLine("pl_over_clu(7,0,0)");
      gInterpreter->ProcessLine("pl_over_clu(7,0,1)");
      gInterpreter->ProcessLine("pl_over_clu(7,0,2)");
      gInterpreter->ProcessLine("pl_over_clu(7,0,3)");
      gInterpreter->ProcessLine("pl_over_clu(8,0,0)");
      gInterpreter->ProcessLine("pl_over_clu(8,0,1)");
      gInterpreter->ProcessLine("pl_over_clu(8,0,2)");
      gInterpreter->ProcessLine("pl_over_clu(8,0,3)");
      gInterpreter->ProcessLine("pl_over_clu(8,0,4)");
      gInterpreter->ProcessLine("pl_over_clu(8,0,5)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,1)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,2,0,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,2,1,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,5,0,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,5,1,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,5,2,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,6,0,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,6,0,1)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,7,0,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,7,0,1)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,7,0,2)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,7,0,3)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,8,0,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,8,0,1)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,8,0,2)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,8,0,3)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,8,0,4)");
      gInterpreter->ProcessLine("pl_over_cluSel(0,8,0,5)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,1)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,2,0,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,2,1,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,5,0,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,5,1,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,5,2,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,6,0,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,6,0,1)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,7,0,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,7,0,1)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,7,0,2)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,7,0,3)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,8,0,0)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,8,0,1)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,8,0,2)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,8,0,3)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,8,0,4)");
      gInterpreter->ProcessLine("pl_over_cluSel(1,8,0,5)");
      break;

    default:
      ;
  }

  gInterpreter->ProcessLine("pl_all_dTSel()");
  gInterpreter->ProcessLine("pl_all_CluMul()");
  gInterpreter->ProcessLine("pl_all_CluRate()");

  // ---------------------------------------------------------------------------

  tTimer.Stop();
  Double_t dRealTime = tTimer.RealTime();
  Double_t dCPUTime = tTimer.CpuTime();

  std::cout << std::endl << std::endl;
  std::cout << "Macro finished successfully." << std::endl;
  std::cout << "Real time " << dRealTime << " s, CPU time " << dCPUTime << " s" << std::endl;
  std::cout << std::endl;

  // ---------------------------------------------------------------------------

  if(bHasFairMonitor)
  {
    // Extract the maximal used memory an add is as Dart measurement
    // This line is filtered by CTest and the value send to CDash
    FairSystemInfo tSysInfo;
    Float_t fMaxMemory = tSysInfo.GetMaxMemory();
    std::cout << "<DartMeasurement name=\"MaxMemory\" type=\"numeric/double\">";
    std::cout << fMaxMemory;
    std::cout << "</DartMeasurement>" << endl;

    Float_t fCPUUsage = dCPUTime/dRealTime;
    std::cout << "<DartMeasurement name=\"CpuLoad\" type=\"numeric/double\">";
    std::cout << fCPUUsage;
    std::cout << "</DartMeasurement>" << endl;

    FairMonitor::GetMonitor()->Print();
  }

  // ---------------------------------------------------------------------------

  FairRootManager::Instance()->CloseOutFile();

  // ---------------------------------------------------------------------------

  RemoveGeoManager();

  // ---------------------------------------------------------------------------
}
