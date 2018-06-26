// -----------------------------------------------------------------------------
// ----- ana_trks.C                                                        -----
// -----                                                                   -----
// ----- adapted by C. Simon on 2018-06-11                                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

void ana_trks(Int_t iNEvents, Int_t iCorMode, const TString& tUnpackDir, const TString& tCalibDir,
              Int_t iTrackingSetup, Double_t dRange1, Double_t dRange2, Bool_t bFinalRun,
              Int_t iDut, Int_t iMRef, Int_t iBRef, Int_t iSel2, Double_t dScaleFactor)
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

  TString tUnpackInputFile      = tUnpackDir + "/data/unpack.out.root";
  TString tCalibInputFile       = tCalibDir  + "/data/digi.out.root";
  TString tOutputFile           = "./hits.out.root";
  TString tHistoFile            = "./ana_track.hst.root";
  TString tInputTrackCalibFile  = "./calib_track.cor_in.root";
  TString tOutputTrackCalibFile = "./calib_track.cor_out.root";
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
    std::cout << "-E- ana_trks: FAIRGeom not found in geometry file!" << std::endl;
    return;
  }

  // ---------------------------------------------------------------------------

  FairFileSource* tFileSource = new FairFileSource(tUnpackInputFile);
  tFileSource->AddFriend(tCalibInputFile);

  // ---------------------------------------------------------------------------

  FairRunAna* tRun= new FairRunAna();
  tRun->SetContainerStatic(kTRUE);
  tRun->SetSource(tFileSource);
  tRun->SetOutputFile(tOutputFile);

  // ---------------------------------------------------------------------------

  Int_t iSel = 1000*iDut + iMRef;

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

  CbmTofTrackFitter* tTofTrackFitter = new CbmTofTrackFitterKF(0, 211);

  CbmTofTrackFinder* tTofTrackFinder = new CbmTofTrackFinderNN();
  tTofTrackFinder->SetMaxTofTimeDifference(5.);
  tTofTrackFinder->SetTxLIM(0.05);
  tTofTrackFinder->SetTyLIM(0.05);
  tTofTrackFinder->SetTyMean(0.11);
  tTofTrackFinder->SetFitter(tTofTrackFitter);
  tTofTrackFinder->SetSIGLIM(dRange2*1.2);
  tTofTrackFinder->SetChiMaxAccept(dRange2);

  CbmTofFindTracks* tTofFindTracks = new CbmTofFindTracks("TOF Track Finder");
  tTofFindTracks->UseFinder(tTofTrackFinder);
  tTofFindTracks->UseFitter(tTofTrackFitter);
  tTofFindTracks->SetCorMode(iCorMode);
  tTofFindTracks->SetTtTarg(0.035);
//  tTofFindTracks->SetUseSigCalib(kFALSE);
  tTofFindTracks->SetBeamCounter(iBRefType, iBRefModule, iBRefCounter);
//  tTofFindTracks->SetStationMaxHMul(20); FIXME
  tTofFindTracks->SetStationMaxHMul(3);
  tTofFindTracks->SetT0MAX(dRange1);
  tTofFindTracks->SetSIGT(0.08);
  tTofFindTracks->SetSIGX(0.4);
  tTofFindTracks->SetSIGY(0.9);
  tTofFindTracks->SetSIGZ(0.05);

  tTofFindTracks->SetCalParFileName(tInputTrackCalibFile);
  tTofFindTracks->SetCalOutFileName(tOutputTrackCalibFile);

  Int_t iMinNofHits = -1;
  Int_t iNStations = 0;
  Int_t iNReqStations = 0;

  switch(iTrackingSetup)
  {
    case 0:
      iMinNofHits = -1;
      iNStations = 1;
      tTofFindTracks->SetStation(0, iBRefType, iBRefModule, iBRefCounter);
      break;

    case 1:
      iMinNofHits = 4;
      iNStations = 5;
      tTofFindTracks->SetStation(0, iBRefType, iBRefModule, iBRefCounter);
      tTofFindTracks->SetStation(1, 9, 2, 1);
      tTofFindTracks->SetStation(2, 4, 0, 0);
      tTofFindTracks->SetStation(3, 9, 0, 1);
      tTofFindTracks->SetStation(4, 9, 2, 0);
      break;

    default:
      std::cout << "-E- ana_trks: tracking setup not known!" << std::endl;
      return;
  }

  if(0 == iNReqStations)
  {
    iNReqStations = iNStations;
  }

  tTofFindTracks->SetMinNofHits(iMinNofHits);
  tTofFindTracks->SetNStations(iNStations);
  tTofFindTracks->SetNReqStations(iNReqStations);

  // ---------------------------------------------------------------------------

  CbmTofAnaTestbeam* tTofAnaTestbeam = new CbmTofAnaTestbeam("TOF TestBeam Analysis", 1);
  tTofAnaTestbeam->SetCorMode(1);

  tTofAnaTestbeam->SetDXMean(0.);
  tTofAnaTestbeam->SetDYMean(0.);
  tTofAnaTestbeam->SetDTMean(0.);
  tTofAnaTestbeam->SetDXWidth(0.5);
  tTofAnaTestbeam->SetDYWidth(1.0);
  tTofAnaTestbeam->SetDTWidth(0.1);

  tTofAnaTestbeam->SetPosY4Sel(0.5*dScaleFactor);
  tTofAnaTestbeam->SetDTDia(0.);
  tTofAnaTestbeam->SetMul0Max(20);
  tTofAnaTestbeam->SetMul4Max(30);
  tTofAnaTestbeam->SetMulDMax(3);
  tTofAnaTestbeam->SetTOffD4(10.);
  tTofAnaTestbeam->SetDTD4MAX(6.);
  tTofAnaTestbeam->SetHitDistMin(30.);

//  tTofAnaTestbeam->SetTShift(-28000.);
  tTofAnaTestbeam->SetPosYS2Sel(0.5);
  tTofAnaTestbeam->SetChS2Sel(0.);
  tTofAnaTestbeam->SetDChS2Sel(100.);
  tTofAnaTestbeam->SetSel2TOff(0.);
  tTofAnaTestbeam->SetChi2Lim(5.);
  tTofAnaTestbeam->SetChi2Lim2(2.);
  tTofAnaTestbeam->SetDutDX(12.);
  tTofAnaTestbeam->SetDutDY(12.);

  tTofAnaTestbeam->SetSIGLIM(3.);
  tTofAnaTestbeam->SetSIGT(0.08);
  tTofAnaTestbeam->SetSIGX(0.4);
  tTofAnaTestbeam->SetSIGY(1.0);

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


  switch(iSel)
  {
    case 901921:
    case 920921:
      switch(iBRefType)
      {
        case 5:
          tTofAnaTestbeam->SetTShift(2.5);
          tTofAnaTestbeam->SetTOffD4(13.);

          switch(iSel2Type)
          {
            case 4:
              tTofAnaTestbeam->SetTShift(-0.7);
              tTofAnaTestbeam->SetTOffD4(16.);
              tTofAnaTestbeam->SetSel2TOff(-0.7);
              break;

            default:
              ;   
          }
          break;

        default:
          ;
      }

      tTofAnaTestbeam->SetCh4Sel(15);
      tTofAnaTestbeam->SetDCh4Sel(15*dScaleFactor);
      break;

    default:
      std::cout << "-I- ana_tracks: analysis setup not defined! Running with default offset parameters..." << std::endl;
      ;
  }

  // ---------------------------------------------------------------------------

  tRun->AddTask(tTofFindTracks);

  if(bFinalRun)
  {
    tRun->AddTask(tTofAnaTestbeam);
  }

  // ---------------------------------------------------------------------------

  FairRuntimeDb* tRuntimeDb = tRun->GetRuntimeDb();
  FairParAsciiFileIo* tParIo1 = new FairParAsciiFileIo();
  tParIo1->open(tParFileList, "in");
  tRuntimeDb->setFirstInput(tParIo1);

  // ---------------------------------------------------------------------------

  std::cout << "-I- ana_trks: initializing run..." << std::endl;
  tRun->Init();

  std::cout << "-I- ana_trks: starting run..." << std::endl;
  tRun->Run(0, iNEvents);

  // ---------------------------------------------------------------------------

  gROOT->LoadMacro((tMacroDir + "/save_hst.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_over_MatD4sel.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_over_Mat04D4best.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_eff_XY.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_calib_trk.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_TIS.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_over_trk.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_XY_trk.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_pull_trk.C").Data());

  gInterpreter->ProcessLine("save_hst(\"" + tHistoFile + "\", kTRUE)");

  if(bFinalRun)
  {
    gInterpreter->ProcessLine("pl_over_MatD4sel()");
    gInterpreter->ProcessLine("pl_over_Mat04D4best(1)");
    gInterpreter->ProcessLine("pl_eff_XY()");
    gInterpreter->ProcessLine("pl_TIS()");
  }

  gInterpreter->ProcessLine("pl_calib_trk()");
  gInterpreter->ProcessLine(Form("pl_over_trk(%d)", iNStations));
  gInterpreter->ProcessLine(Form("pl_XY_trk(%d)", iNStations));
  gInterpreter->ProcessLine(Form("pl_pull_trk(%d, %d, 1)", iNStations, 0));
  gInterpreter->ProcessLine(Form("pl_pull_trk(%d, %d, 1)", iNStations, 1));
  gInterpreter->ProcessLine(Form("pl_pull_trk(%d, %d, 1)", iNStations, 3));
  gInterpreter->ProcessLine(Form("pl_pull_trk(%d, %d, 1)", iNStations, 4));

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
