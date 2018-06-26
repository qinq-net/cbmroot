// -----------------------------------------------------------------------------
// ----- ana_digi.C                                                        -----
// -----                                                                   -----
// ----- adapted by C. Simon on 2018-06-03                                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

void ana_digi(Int_t iNEvents, Int_t iCalMode, Int_t iCalSel, Int_t iCalRPC, Int_t iRefSel,
              const TString& tUnpackDir, Int_t iDut, Int_t iMRef, Int_t iBRef, Int_t iSel2,
              Double_t dDeadTime, Bool_t bWriteHits)
{
  // ---------------------------------------------------------------------------

  TString tLogLevel = "ERROR";
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
  TString tOutputFile           = "./digi.out.root";
  TString tHistoFile            = "./calib_cluster.hst.root";
  TString tInputClustCalibFile  = "./calib_cluster.cor_in.root";
  TString tOutputClustCalibFile = "./calib_cluster.cor_out.root";
  TString tInputAnaCalibFile    = "./calib_ana.cor_in.root";
  TString tOutputAnaCalibFile   = "./calib_ana.cor_out.root";

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
    std::cout << "-E- ana_digi: FAIRGeom not found in geometry file!" << std::endl;
    return;
  }

  // ---------------------------------------------------------------------------

  FairFileSource* tFileSource = new FairFileSource(tInputFile);

  // ---------------------------------------------------------------------------

  FairRunAna* tRun= new FairRunAna();
  tRun->SetContainerStatic(kTRUE);
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

  Int_t iSel2Counter = iSel2%10;
  Int_t iSel2Module  = ((iSel2 - iSel2Counter)/10)%10;
  Int_t iSel2Type    = (iSel2 - iSel2Counter - 10*iSel2Module)/100;

  // ---------------------------------------------------------------------------

  CbmTofTestBeamClusterizer* tTofTestBeamClust = new CbmTofTestBeamClusterizer("TOF TestBeam Clusterizer", 1, bWriteHits);
  tTofTestBeamClust->SetPs2Ns(kFALSE);
  tTofTestBeamClust->SetCalMode(iCalMode);
  tTofTestBeamClust->SetCalSel(iCalSel);
  tTofTestBeamClust->SetCaldXdYMax(2.);
  tTofTestBeamClust->SetCalCluMulMax(10.);
  tTofTestBeamClust->SetCalRpc(iCalRPC);
  tTofTestBeamClust->SetTRefId(iRefSel);
  tTofTestBeamClust->SetTotMax(10.);
  tTofTestBeamClust->SetTotMin(0.01);
  tTofTestBeamClust->SetTotPreRange(5.);
  tTofTestBeamClust->SetTotMean(2.);
  tTofTestBeamClust->SetMaxTimeDist(0.5);
  //tTofTestBeamClust->SetMaxTimeDist(0.);
  tTofTestBeamClust->SetDelTofMax(6.);
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
  tTofTestBeamClust->SetSel2Id(iSel2Type); 
  tTofTestBeamClust->SetSel2Sm(iSel2Module); 
  tTofTestBeamClust->SetSel2Rpc(iSel2Counter);

  tTofTestBeamClust->SetCalParFileName(tInputClustCalibFile);
  tTofTestBeamClust->SetOutHstFileName(tOutputClustCalibFile);


  switch (iCalMode)
  {
    // initial calibration
    case 0:
      tTofTestBeamClust->SetTotMax(10.);
      //tTofTestBeamClust->SetTotMin(1.);
      tTofTestBeamClust->SetTRefDifMax(2000.);
      tTofTestBeamClust->PosYMaxScal(2000.);
      tTofTestBeamClust->SetMaxTimeDist(0.);
      break;

    // save offsets, update walks, for diamonds
    case 1:
      tTofTestBeamClust->SetTRefDifMax(50.);
      tTofTestBeamClust->PosYMaxScal(0.1);
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
      tTofTestBeamClust->SetTRefDifMax(200.);
      tTofTestBeamClust->PosYMaxScal(1000.);
      tTofTestBeamClust->SetMaxTimeDist(0.);
      break;
    case 12:
    case 13:
      tTofTestBeamClust->SetTRefDifMax(100.);
      tTofTestBeamClust->PosYMaxScal(100.);
      break;
    case 22:
    case 23:
      tTofTestBeamClust->SetTRefDifMax(50.);
      tTofTestBeamClust->PosYMaxScal(50.);
      break;
    case 32:
    case 33:
      tTofTestBeamClust->SetTRefDifMax(25.);
      tTofTestBeamClust->PosYMaxScal(20.);
      break;
    case 42:
    case 43:
      tTofTestBeamClust->SetTRefDifMax(12.);
      tTofTestBeamClust->PosYMaxScal(10.);
      break;
    case 52:
    case 53:
      tTofTestBeamClust->SetTRefDifMax(5.);
      tTofTestBeamClust->PosYMaxScal(4.);
      break;
    case 62:
    case 63:
      tTofTestBeamClust->SetTRefDifMax(2.5);
      tTofTestBeamClust->PosYMaxScal(2.);
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
      tTofTestBeamClust->SetTRefDifMax(3.);
      tTofTestBeamClust->PosYMaxScal(1.);
      break;
    case 24:
      tTofTestBeamClust->SetTRefDifMax(1.5);
      tTofTestBeamClust->PosYMaxScal(0.8);
      break;
    case 44:
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
      std::cout << "-E- ana_digi: calibration mode not implemented!" << std::endl;
      return;
  }

  // ---------------------------------------------------------------------------

  CbmTofAnaTestbeam* tTofAnaTestbeam = new CbmTofAnaTestbeam("TOF TestBeam Analysis", 1);
  tTofAnaTestbeam->SetDXMean(0.);
  tTofAnaTestbeam->SetDYMean(0.);
  tTofAnaTestbeam->SetDTMean(0.);
  tTofAnaTestbeam->SetDXWidth(0.4);
  tTofAnaTestbeam->SetDYWidth(0.4);
  tTofAnaTestbeam->SetDTWidth(0.08);

  tTofAnaTestbeam->SetPosY4Sel(0.5);
  tTofAnaTestbeam->SetDTDia(0.);
  tTofAnaTestbeam->SetCorMode(iRefSel);
  tTofAnaTestbeam->SetMul0Max(10);
  tTofAnaTestbeam->SetMul4Max(10);
  tTofAnaTestbeam->SetMulDMax(10);
  tTofAnaTestbeam->SetHitDistMin(30.);

  tTofAnaTestbeam->SetPosYS2Sel(0.5);
  tTofAnaTestbeam->SetChS2Sel(0.);
  tTofAnaTestbeam->SetDChS2Sel(100.);

  tTofAnaTestbeam->SetTShift(0.);
  tTofAnaTestbeam->SetSel2TOff(0.);
  tTofAnaTestbeam->SetTOffD4(13.);

  tTofAnaTestbeam->SetChi2Lim(10.);

  tTofAnaTestbeam->SetDut(iDutType);
  tTofAnaTestbeam->SetDutSm(iDutModule);
  tTofAnaTestbeam->SetDutRpc(iDutCounter);
  tTofAnaTestbeam->SetMrpcRef(iMRefType);
  tTofAnaTestbeam->SetMrpcRefSm(iMRefModule);
  tTofAnaTestbeam->SetMrpcRefRpc(iMRefCounter);
  tTofAnaTestbeam->SetMrpcSel2(iSel2Type);
  tTofAnaTestbeam->SetMrpcSel2Sm(iSel2Module);
  tTofAnaTestbeam->SetMrpcSel2Rpc(iSel2Counter);
  tTofAnaTestbeam->SetBeamRefSmType(iBRefType);
  tTofAnaTestbeam->SetBeamRefSmId(iBRefModule);
  tTofAnaTestbeam->SetBeamRefRpc(iBRefCounter);

  tTofAnaTestbeam->SetCalParFileName(tInputAnaCalibFile);
  tTofAnaTestbeam->SetCalOutFileName(tOutputAnaCalibFile);

  Bool_t bBEff = kFALSE;

  switch (iSet)
  {
    case 400300:
      switch (iBRefType)
      {
        case 4:
          tTofAnaTestbeam->SetTShift(0.);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;
     
        case 5:
          tTofAnaTestbeam->SetTShift(-3.);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;

        case 9:
          tTofAnaTestbeam->SetChi2Lim(100.);
          tTofAnaTestbeam->SetMulDMax(3);
          tTofAnaTestbeam->SetTShift(0.1);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.5);
          break;

        default:
          ;
      }

      tTofAnaTestbeam->SetChi2Lim(30.);
      tTofAnaTestbeam->SetDXWidth(1.5);
      tTofAnaTestbeam->SetDYWidth(1.5);
      tTofAnaTestbeam->SetDTWidth(0.12);
      tTofAnaTestbeam->SetCh4Sel(15);
      tTofAnaTestbeam->SetDCh4Sel(3);
      tTofAnaTestbeam->SetPosY4Sel(0.25);
      tTofAnaTestbeam->SetMulDMax(1);  
      tTofAnaTestbeam->SetMul0Max(4);
      tTofAnaTestbeam->SetMul4Max(4);
      break;

    case 920300:
    case 921300:
      switch (iBRefType)
      {
        case 5:
          tTofAnaTestbeam->SetMulDMax(1); 
          tTofAnaTestbeam->SetTShift(0.2);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;

        case 9:
          tTofAnaTestbeam->SetTShift(0.1);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.5);
          break;

        default:
          ;
      }

      tTofAnaTestbeam->SetChi2Lim(30.);
      tTofAnaTestbeam->SetDXWidth(2.);
      tTofAnaTestbeam->SetDYWidth(2.);
      tTofAnaTestbeam->SetDTWidth(0.2);
      tTofAnaTestbeam->SetCh4Sel(15);
      tTofAnaTestbeam->SetDCh4Sel(10);
      tTofAnaTestbeam->SetPosY4Sel(0.5);
      tTofAnaTestbeam->SetMulDMax(1);
      tTofAnaTestbeam->SetMul0Max(3);
      tTofAnaTestbeam->SetMul4Max(3);
      break; 

    case 920400:
    case 921400:
      switch (iBRefType)
      {
        case 5:
          tTofAnaTestbeam->SetMulDMax(1);
          tTofAnaTestbeam->SetTShift(0.2);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;

        case 9:
          tTofAnaTestbeam->SetTShift(0.1);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.5);
          break;

        default:
          ;
      }

      tTofAnaTestbeam->SetChi2Lim(30.);
      tTofAnaTestbeam->SetDXWidth(2.);
      tTofAnaTestbeam->SetDYWidth(2.);
      tTofAnaTestbeam->SetDTWidth(0.2);
      tTofAnaTestbeam->SetCh4Sel(8);
      tTofAnaTestbeam->SetDCh4Sel(10);
      tTofAnaTestbeam->SetPosY4Sel(0.5);
      tTofAnaTestbeam->SetMulDMax(1);
      tTofAnaTestbeam->SetMul0Max(30);
      tTofAnaTestbeam->SetMul4Max(30);
      break; 

    case 300920:
      switch (iBRefType)
      {
        case 9:
          tTofAnaTestbeam->SetTShift(-2.);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;

        case 5:
          tTofAnaTestbeam->SetTShift(0.);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;

        default:
          ;
      }

      tTofAnaTestbeam->SetChi2Lim(1000.);
      tTofAnaTestbeam->SetDXWidth(2.);
      tTofAnaTestbeam->SetDYWidth(2.);
      tTofAnaTestbeam->SetDTWidth(0.5);
      tTofAnaTestbeam->SetMulDMax(1); 
      tTofAnaTestbeam->SetMul0Max(5);
      tTofAnaTestbeam->SetMul4Max(5);
      break; 

    case 900920:
      switch (iBRefType)
      {
        case 4:
          tTofAnaTestbeam->SetTShift(-2.);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;

        case 5:
          tTofAnaTestbeam->SetTShift(0.);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;

        default:
          ;
      }

      tTofAnaTestbeam->SetChi2Lim(10.);
      tTofAnaTestbeam->SetDXWidth(1.);
      tTofAnaTestbeam->SetDYWidth(2.);
      tTofAnaTestbeam->SetDTWidth(0.1);
      tTofAnaTestbeam->SetMulDMax(1);
      tTofAnaTestbeam->SetMul0Max(15);
      tTofAnaTestbeam->SetMul4Max(15);
      break;

    case 300921:
    case 400921:
    case 900921:
    case 910921:
    case 920921:
      switch (iBRefType)
      {
        case 3:
          tTofAnaTestbeam->SetTShift(-8.);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(-0.15);
          break;

        case 5:
          tTofAnaTestbeam->SetTShift(-0.33);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.1);
          break;

        case 9:
          tTofAnaTestbeam->SetTShift(1.2);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(-0.15);
          break;

        default:
          ;
      }

      tTofAnaTestbeam->SetChi2Lim(10.);
      tTofAnaTestbeam->SetDXWidth(1.0);
      tTofAnaTestbeam->SetDYWidth(2.0);
      tTofAnaTestbeam->SetDTWidth(0.15);

      bBEff = kFALSE;
      //bBEff=kTRUE;
      if(bBEff)
      {
        tTofAnaTestbeam->SetCh4Sel(16);
        tTofAnaTestbeam->SetDCh4Sel(13);
        tTofAnaTestbeam->SetPosY4Sel(0.4);
      }
      break; 

    case 901900:
    case 400900:
    case 300900:
      switch (iBRefType)
      {
        case 4:
          tTofAnaTestbeam->SetTShift(-2.);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;

        case 5:
          tTofAnaTestbeam->SetTShift(-20.);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(-0.17);
          break;

        default:
          ;
      }

      tTofAnaTestbeam->SetChi2Lim(40.);
      tTofAnaTestbeam->SetDXWidth(1.0);
      tTofAnaTestbeam->SetDYWidth(2.0);
      tTofAnaTestbeam->SetDTWidth(0.15);
      break; 

    case 300400:
      switch (iBRefType)
      {
        case 9:
          tTofAnaTestbeam->SetTShift(-2.);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;

        default:
          ;
      }
      break;

    case 210200:
      switch (iBRefType)
      {
        case 9:
          tTofAnaTestbeam->SetTShift(-2.);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;

        default:
          ;
      }
      break;

    case 700600:
    case 701600:
    case 702600:
    case 703600:
    case 100600:
    case 200600:
    case 210600:
    case 601600:
      switch (iBRefType)
      {
        case 5:
          tTofAnaTestbeam->SetTShift(-11.);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;

        case 6:
          tTofAnaTestbeam->SetTShift(-6.);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;

        default:
          ;
      }

      tTofAnaTestbeam->SetChi2Lim(5.);
      tTofAnaTestbeam->SetDXWidth(0.5);
      tTofAnaTestbeam->SetDYWidth(1.);
      tTofAnaTestbeam->SetDTWidth(0.1);

      bBEff = kFALSE;
      if(bBEff)
      {
        tTofAnaTestbeam->SetCh4Sel(20);
        tTofAnaTestbeam->SetDCh4Sel(8);
        tTofAnaTestbeam->SetPosY4Sel(0.4);
      }

      tTofAnaTestbeam->SetMulDMax(10);
      tTofAnaTestbeam->SetMul0Max(10);
      tTofAnaTestbeam->SetMul4Max(10);
      break;

    default:
      std::cout << "-E- ana_digi: detector setup not known!" << std::endl;
    return;
  }

  // ---------------------------------------------------------------------------

  tRun->AddTask(tTofTestBeamClust);
  tRun->AddTask(tTofAnaTestbeam);

  // ---------------------------------------------------------------------------

  FairRuntimeDb* tRuntimeDb = tRun->GetRuntimeDb();
  FairParAsciiFileIo* tParIo1 = new FairParAsciiFileIo();
  tParIo1->open(tParFileList, "in");
  tRuntimeDb->setFirstInput(tParIo1);

  // ---------------------------------------------------------------------------

  std::cout << "-I- ana_digi: initializing run..." << std::endl;
  tRun->Init();

  std::cout << "-I- ana_digi: starting run..." << std::endl;
  tRun->Run(0, iNEvents);

  // ---------------------------------------------------------------------------

  gROOT->LoadMacro((tMacroDir + "/save_hst.C").Data());
  gROOT->LoadMacro((tMacroDir + "/fit_ybox.h").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_all_CluMul.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_all_CluRate.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_over_cluSel.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_over_clu.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_all_dTSel.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_over_MatD4sel.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_over_Mat04D4best.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_TIS.C").Data());

  gInterpreter->ProcessLine("save_hst(\"" + tHistoFile + "\", kTRUE)");

  switch(iSet)
  {
    case 0:
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

    case 1:
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
      gInterpreter->ProcessLine("pl_over_clu(2,1,0)");
      gInterpreter->ProcessLine("pl_over_clu(5,0,0)");
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
  gInterpreter->ProcessLine("pl_over_MatD4sel()");
  gInterpreter->ProcessLine("pl_over_Mat04D4best()");
  gInterpreter->ProcessLine("pl_all_CluMul()");
  gInterpreter->ProcessLine("pl_all_CluRate()");
  gInterpreter->ProcessLine("pl_TIS()");

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
