// -----------------------------------------------------------------------------
// ----- dis_trks.C                                                 -----
// -----                                                                   -----
// ----- adapted by C. Simon on 2018-06-20                                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

void dis_trks(const TString& tCalibDir, Int_t iTrackingSetup, Int_t iBRef,
              Int_t iCorMode, Double_t dRange1, Double_t dRange2)
{
  // ---------------------------------------------------------------------------

  TString tLogLevel = "DEBUG2";
  TString tLogVerbosity = "LOW";
  gErrorIgnoreLevel = kWarning;
  gDebug = 0;

  TTree::SetMaxTreeSize(1000000000000LL); // [B] here: 1 TB

  // ---------------------------------------------------------------------------

  FairLogger::GetLogger()->SetLogScreenLevel(tLogLevel.Data());
  FairLogger::GetLogger()->SetLogVerbosityLevel(tLogVerbosity.Data());

  // ---------------------------------------------------------------------------

  TString tSrcDir   = gSystem->Getenv("VMCWORKDIR");
  TString tParamDir = tSrcDir + "/parameters";
  TString tGeoDir   = tSrcDir + "/geometry";

  TString tTofGeoVersion = "v15e";

  TString tCalibInputFile       = tCalibDir  + "/data/digi.out.root";
  TString tOutputFile           = "./dummy.out.root";
  TString tInputTrackCalibFile  = "./calib_track.cor_in.root";
  TString tOutputTrackCalibFile = "./calib_track.cor_out.root";

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
    std::cout << "-E- dis_trks: FAIRGeom not found in geometry file!" << std::endl;
    return;
  }

  // ---------------------------------------------------------------------------

  FairFileSource* tFileSource = new FairFileSource(tCalibInputFile);

  // ---------------------------------------------------------------------------

  FairRunAna* tRun= new FairRunAna();
  tRun->SetContainerStatic(kTRUE);
  tRun->SetSource(tFileSource);
  tRun->SetOutputFile(tOutputFile);

  // ---------------------------------------------------------------------------

  Int_t iBRefCounter = iBRef%10;
  Int_t iBRefModule  = ((iBRef - iBRefCounter)/10)%10;
  Int_t iBRefType    = (iBRef - iBRefCounter - 10*iBRefModule)/100;

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
  tTofFindTracks->SetStationMaxHMul(20);
  tTofFindTracks->SetT0MAX(dRange1);
  tTofFindTracks->SetSIGT(0.08);
  tTofFindTracks->SetSIGX(0.4);
  tTofFindTracks->SetSIGY(0.9);
  tTofFindTracks->SetSIGZ(0.05);

  tTofFindTracks->SetCalParFileName(tInputTrackCalibFile);
//  tTofFindTracks->SetCalOutFileName(tOutputTrackCalibFile);

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
      std::cout << "-E- dis_trks: tracking setup not known!" << std::endl;
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

  tRun->AddTask(tTofFindTracks);

  // ---------------------------------------------------------------------------

  FairRuntimeDb* tRuntimeDb = tRun->GetRuntimeDb();
  FairParAsciiFileIo* tParIo1 = new FairParAsciiFileIo();
  tParIo1->open(tParFileList, "in");
  tRuntimeDb->setFirstInput(tParIo1);

  // ---------------------------------------------------------------------------

  FairEventManager* tEventManager = new FairEventManager();

  // ---------------------------------------------------------------------------

  CbmPixelHitSetDraw* tTofHitDrawer = new CbmPixelHitSetDraw("TofHit", kRed, kOpenCircle);

  // ---------------------------------------------------------------------------

  CbmPixelHitSetDraw* tTofUHitDrawer = new CbmPixelHitSetDraw("TofUHit", kRed, kOpenCross);

  // ---------------------------------------------------------------------------

  CbmEvDisTracks* tTofTrackDrawer = new CbmEvDisTracks("Tof Tracks", 1);

  // ---------------------------------------------------------------------------

  tEventManager->AddTask(tTofHitDrawer);
  tEventManager->AddTask(tTofUHitDrawer);
  tEventManager->AddTask(tTofTrackDrawer);

  // ---------------------------------------------------------------------------

  gGeoManager->SetVisOption(1);
  gGeoManager->SetVisLevel(5);

  TObjArray* tVolumeList = gGeoManager->GetListOfVolumes();

  for(Int_t iVolume = 0; iVolume < tVolumeList->GetEntries(); iVolume++)
  {
    dynamic_cast<TGeoVolume*>(tVolumeList->At(iVolume))->SetTransparency(96);
  }

  // ---------------------------------------------------------------------------

  std::cout << "-I- dis_trks: initializing event manager..." << std::endl;
  tEventManager->Init(1, 5);

  // ---------------------------------------------------------------------------

  gEve->GetDefaultGLViewer()->SetClearColor(kYellow - 10);
  gEve->GetDefaultGLViewer()->SetDrawCameraCenter(kTRUE);
  gEve->GetDefaultGLViewer()->SetGuideState(TGLUtil::kAxesOrigin, kTRUE, kFALSE, NULL);
  gEve->GetDefaultGLViewer()->DrawGuides();

  // ---------------------------------------------------------------------------
}
