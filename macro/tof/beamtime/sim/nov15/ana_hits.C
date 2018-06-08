// -----------------------------------------------------------------------------
// ----- ana_hits.C                                                        -----
// -----                                                                   -----
// ----- adapted by C. Simon on 2018-06-04                                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

void ana_hits(Int_t iNEvents, Int_t iGenCor, const TString& tUnpackDir, const TString& tCalibDir,
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
  TString tHistoFile            = "./ana_cluster.hst.root";
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
    std::cout << "-E- ana_hits: FAIRGeom not found in geometry file!" << std::endl;
    return;
  }

  // ---------------------------------------------------------------------------

  FairFileSource* tFileSource = new FairFileSource(tUnpackInputFile);
  tFileSource->AddFriend(tCalibInputFile);

  // ---------------------------------------------------------------------------

  FairRunAna* tRun= new FairRunAna();
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

  CbmTofAnaTestbeam* tTofAnaTestbeam = new CbmTofAnaTestbeam("TOF TestBeam Analysis", 1);
  tTofAnaTestbeam->SetDXMean(0.);
  tTofAnaTestbeam->SetDYMean(0.);
  tTofAnaTestbeam->SetDTMean(0.);
  tTofAnaTestbeam->SetDXWidth(0.7);
  tTofAnaTestbeam->SetDYWidth(1.0);
  tTofAnaTestbeam->SetDTWidth(0.1);

  tTofAnaTestbeam->SetPosY4Sel(0.5*dScaleFactor);
  tTofAnaTestbeam->SetDTDia(0.);
  tTofAnaTestbeam->SetCorMode(iGenCor);
  tTofAnaTestbeam->SetMul0Max(10);
  tTofAnaTestbeam->SetMul4Max(10);
  tTofAnaTestbeam->SetMulDMax(10);
  //tTofAnaTestbeam->SetMul0Max(200);
  //tTofAnaTestbeam->SetMul4Max(3);
  //tTofAnaTestbeam->SetMulDMax(20);
  tTofAnaTestbeam->SetHitDistMin(30.);
  tTofAnaTestbeam->SetDTD4MAX(6.);

  tTofAnaTestbeam->SetPosYS2Sel(0.5);
  tTofAnaTestbeam->SetChS2Sel(0.);
  tTofAnaTestbeam->SetDChS2Sel(100.);

  //tTofAnaTestbeam->SetTShift(-28.);
  tTofAnaTestbeam->SetSel2TOff(0.45);
  //tTofAnaTestbeam->SetSel2TOff(0.);
  tTofAnaTestbeam->SetTOffD4(10.);

  tTofAnaTestbeam->SetChi2Lim(5.);
  tTofAnaTestbeam->SetChi2Lim2(2.);

  tTofAnaTestbeam->SetSIGLIM(3.);
  tTofAnaTestbeam->SetSIGT(0.1);
  tTofAnaTestbeam->SetSIGX(1.);
  tTofAnaTestbeam->SetSIGY(1.);

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

  switch (iSel)
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
          tTofAnaTestbeam->SetTShift(0.1);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.5);
          break;

        default:
          ;
      }

      tTofAnaTestbeam->SetCh4Sel(15);
      tTofAnaTestbeam->SetDCh4Sel(15*dScaleFactor);
      break;

    case 900300:
    case 901300:
    case 910300:
    case 920300:
    case 921300:
      switch (iBRefType)
      {
        case 4:
          tTofAnaTestbeam->SetTShift(0.);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;
     
        case 5:
          tTofAnaTestbeam->SetTShift(-17.);
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

      tTofAnaTestbeam->SetCh4Sel(15);
      tTofAnaTestbeam->SetDCh4Sel(15*dScaleFactor);
      break;

    case 300920:  
    case 400920:  
    case 900920:  
    case 901920:  
    case 910920:  
    case 921920:  
      tTofAnaTestbeam->SetTOffD4(13.);
      // tTofAnaTestbeam->SetTShift(-2.);
      switch (iBRefType)
      {
        case 5:   
          switch(iBRefModule)
          {
            case 0:
              tTofAnaTestbeam->SetTShift(3.5);
              tTofAnaTestbeam->SetTOffD4(20.);
              break;

            case 1:
              tTofAnaTestbeam->SetTShift(2.05);
              tTofAnaTestbeam->SetTOffD4(12.);
              break;
          }

          switch(iSel2Type)
          {
            case 3:
              tTofAnaTestbeam->SetSel2TOff(-0.020);
              break;

            case 4:
              tTofAnaTestbeam->SetSel2TOff(-0.050);
              break;

            case 9:
              switch(iSel2Module)
              {
                case 2:
                  switch(iSel2Counter)
                  {
                    case 0:
                      tTofAnaTestbeam->SetSel2TOff(0.030);
                      break;

                    case 1:
                      tTofAnaTestbeam->SetSel2TOff(0.140);
                      break;

                    default:
                      std::cout << "-E- ana_hits: counter " << iSel2 << " not implemented as Sel2!" << std::endl;
                      return;
                  }
                  break;

                case 0:
                  switch(iSel2Counter)
                  {
                    case 0:
                      tTofAnaTestbeam->SetSel2TOff(0.);
                      break;

                    case 1:
                      tTofAnaTestbeam->SetSel2TOff(-0.010);
                      break;

                    default:
                      std::cout << "-E- ana_hits: counter " << iSel2 << " not implemented as Sel2!" << std::endl;
                      return;
                  }
                  break;

                default:
                  std::cout << "-E- ana_hits: counter " << iSel2 << " not implemented as Sel2!" << std::endl;
                  return; 
              }
              break;

            default:
              ;
          }
          break;

        case 3:
          tTofAnaTestbeam->SetTShift(1.95);
          tTofAnaTestbeam->SetTOffD4(13.);
          tTofAnaTestbeam->SetSel2TOff(2.25);
          break;

        case 9:
          switch(iSel2Module)
          {
            case 0: 
              tTofAnaTestbeam->SetTShift(-0.12);
              tTofAnaTestbeam->SetTOffD4(14.);
              tTofAnaTestbeam->SetSel2TOff(-0.14);
              break;
          }
          break;

        default:
          std::cout << "-E- ana_hits: setup not implemented!" << std::endl;
          return;
      }

      tTofAnaTestbeam->SetCh4Sel(15);
      tTofAnaTestbeam->SetDCh4Sel(15*dScaleFactor);
      break;


    case 300921:  
    case 400921:  
    case 900921:  
    case 901921:  
    case 910921:  
    case 911921:
    case 920921:  
      switch (iBRefType)
      {
        case 5:
          switch(iBRefModule)
          {
            case 0:
              tTofAnaTestbeam->SetTShift(3.);
              tTofAnaTestbeam->SetTOffD4(20.);
              break;

            case 1:
              tTofAnaTestbeam->SetTShift(1.8);
              tTofAnaTestbeam->SetTOffD4(17.);
              break;

            case 2: // XXX
              tTofAnaTestbeam->SetTShift(1.8);
              tTofAnaTestbeam->SetTOffD4(17.);
              break;
          }

          switch(iSel2Type)
          {
            case 3:
              tTofAnaTestbeam->SetSel2TOff(-0.1);
              break;

            case 4:
              tTofAnaTestbeam->SetSel2TOff(-0.1);
              break;

            case 9:
              switch(iSel2Module)
              {
                case 2:
                  tTofAnaTestbeam->SetSel2TOff(-0.17);
                  break;

                default:
                  std::cout << "-E- ana_hits: counter " << iSel2 << " not implemented as Sel2!" << std::endl;
                  return; 
              }
              break;

            default:
             ;
          }
          break;

        case 3:
          tTofAnaTestbeam->SetTShift(1.95);
          tTofAnaTestbeam->SetTOffD4(13.);
          tTofAnaTestbeam->SetSel2TOff(2.07);
          break;

        case 9:
          switch(iSel2Module)
          {
            case 0:
              tTofAnaTestbeam->SetTShift(-0.12);
              tTofAnaTestbeam->SetTOffD4(14.);
              tTofAnaTestbeam->SetSel2TOff(-0.14);
              break;
          }
          break;

        default:
          std::cout << "-E- ana_hits: setup not implemented!" << std::endl;
          return;
      }

      tTofAnaTestbeam->SetCh4Sel(15);
      tTofAnaTestbeam->SetDCh4Sel(15*dScaleFactor);
      break;

    case 300901:  
    case 400901:  
    case 920901: 
    case 921901: 
    case 910901: 
    case 900901:  
      switch (iBRefType)
      {
        case 5:
          tTofAnaTestbeam->SetTShift(3.5);
          tTofAnaTestbeam->SetTOffD4(20.);

          switch(iSel2Type)
          {
            case 3:
              tTofAnaTestbeam->SetSel2TOff(0.);
              break;

            case 4:
              tTofAnaTestbeam->SetSel2TOff(0.07);
              break;

            case 9:
              switch(iSel2Module)
              {
                case 0:
                  tTofAnaTestbeam->SetSel2TOff(0.02);
                  break;

                default:
                  std::cout << "-E- ana_hits: counter " << iSel2 << " not implemented as Sel2!" << std::endl;
                  return; 
              }
              break;

            default:
              ;
          }
          break;

        case 3:
          tTofAnaTestbeam->SetTShift(1.95);
          tTofAnaTestbeam->SetTOffD4(13.);
          tTofAnaTestbeam->SetSel2TOff(2.07);
          break;

        default:
          std::cout << "-E- ana_hits: setup not implemented!" << std::endl;
          return;
      }

      tTofAnaTestbeam->SetCh4Sel(15);
      tTofAnaTestbeam->SetDCh4Sel(15*dScaleFactor);
      break;

    case 300900:  
    case 400900:  
    case 920900:  
    case 921900:  
    case 910900:  
    case 901900:  
      switch (iBRefType)
      {
        case 5:
          tTofAnaTestbeam->SetTShift(3.5);
          tTofAnaTestbeam->SetTOffD4(20.);

          switch(iSel2Type)
          {
            case 3:
              tTofAnaTestbeam->SetSel2TOff(0.);
              break;

            case 4:
              tTofAnaTestbeam->SetSel2TOff(0.07);
              break;

            case 9:
              switch(iSel2Module)
              {
                case 0:
                  tTofAnaTestbeam->SetSel2TOff(0.02);
                  break;

                default:
                  std::cout << "-E- ana_hits: counter " << iSel2 << " not implemented as Sel2!" << std::endl;
                  return; 
              }
              break;

            default:
              ;
          }
          break;

        case 3:
          tTofAnaTestbeam->SetTShift(1.95);
          tTofAnaTestbeam->SetTOffD4(13.);
          tTofAnaTestbeam->SetSel2TOff(2.07);
          break;

        default:
          std::cout << "-E- ana_hits: setup not implemented!" << std::endl;
          return;
      }

      tTofAnaTestbeam->SetCh4Sel(15);
      tTofAnaTestbeam->SetDCh4Sel(15*dScaleFactor);
      break;

    case 300400:
    case 900400:
    case 901400:
    case 910400:
    case 920400:
    case 921400:
      switch (iBRefType)
      {
        case 3:
          tTofAnaTestbeam->SetTShift(2.5);
          tTofAnaTestbeam->SetTOffD4(0.);
          tTofAnaTestbeam->SetSel2TOff(2.76);
          break;
     
        case 5:
          tTofAnaTestbeam->SetTShift(3.5);
          tTofAnaTestbeam->SetTOffD4(17.);

          switch(iSel2Type)
          {
            case 3:
              tTofAnaTestbeam->SetSel2TOff(-0.045);
              break;

            case 9:
              switch(iSel2Module)
              {
                case 2:
                  switch(iSel2Counter)
                  {
                    case 0:
                      tTofAnaTestbeam->SetSel2TOff(0.03);
                      break;

                    case 1:
                      tTofAnaTestbeam->SetSel2TOff(0.124);
                      break;

                    default:
                      std::cout << "-E- ana_hits: counter " << iSel2 << " not implemented as Sel2!" << std::endl;
                      return; 
                  }
                  break;

                case 0:
                  switch(iSel2Counter)
                  {
                    case 0:
                      tTofAnaTestbeam->SetSel2TOff(-0.040);
                      break;

                    case 1:
                      tTofAnaTestbeam->SetSel2TOff(-0.045);
                      break;

                    default:
                      std::cout << "-E- ana_hits: counter " << iSel2 << " not implemented as Sel2!" << std::endl;
                      return; 
                  }
                  break;

                default:
                  std::cout << "-E- ana_hits: counter " << iSel2 << " not implemented as Sel2!" << std::endl;
                  return; 
              }
              break;

            default:
              std::cout << "-E- ana_hits: counter " << iSel2 << " not implemented as Sel2!" << std::endl;
              return;        
          }
          break;

        case 9:
          tTofAnaTestbeam->SetTShift(0.1);
          tTofAnaTestbeam->SetTOffD4(16.);
          tTofAnaTestbeam->SetSel2TOff(0.5);
          break;

        default:
          std::cout << "-E- ana_hits: setup not implemented!" << std::endl;
          return;
      }

      tTofAnaTestbeam->SetCh4Sel(8);
      tTofAnaTestbeam->SetDCh4Sel(8*dScaleFactor);
      break;

    case 100600:
    case 200600:
    case 210600:
    case 700600:
    case 701600:
    case 702600:
    case 703600:
    case 800600:
    case 801600:
    case 802600:
    case 803600:
    case 804600:
    case 805600:
    case 601600:
      switch (iBRefType)
      {     
        case 5:
          switch(iBRefModule)
          {
            case 0:
              tTofAnaTestbeam->SetTShift(-8.3);
              tTofAnaTestbeam->SetTOffD4(17.);
              break;

            case 1:
              tTofAnaTestbeam->SetTShift(0.2);
              tTofAnaTestbeam->SetTOffD4(17.);
              break;   
          }

          switch(iSel2Type)
          {
            case 0:
              tTofAnaTestbeam->SetSel2TOff(2.9);
              break;

            case 1:
              tTofAnaTestbeam->SetSel2TOff(-0.085);
              break;

            case 6:
              tTofAnaTestbeam->SetSel2TOff(0.085);
              break;

            default:
              ;
          }
          break;

        case 1:
          tTofAnaTestbeam->SetTShift(2.5);
          tTofAnaTestbeam->SetTOffD4(17.);
          tTofAnaTestbeam->SetSel2TOff(2.9);

          switch(iSel2Type)
          {
            case 0:
              tTofAnaTestbeam->SetSel2TOff(2.9);
              break;

            case 1:
              tTofAnaTestbeam->SetTShift(0.);
              tTofAnaTestbeam->SetSel2TOff(0.);
              break;

            case 6:
              tTofAnaTestbeam->SetSel2TOff(0.085);
              break;

            default:
              ;
          }
          break;

        case 6:
          tTofAnaTestbeam->SetTShift(-0.070);
          tTofAnaTestbeam->SetTOffD4(15.);
          tTofAnaTestbeam->SetSel2TOff(-0.050);
          break;

        case 7:
          switch(iSel2Counter)
          {
            case 0:
              tTofAnaTestbeam->SetTShift(-0.038);
              tTofAnaTestbeam->SetTOffD4(15.);
              tTofAnaTestbeam->SetSel2TOff(-0.042);
              break;
          }
          break;

        default:
          std::cout << "-E- ana_hits: setup not implemented!" << std::endl;
          return;
      }

      tTofAnaTestbeam->SetCh4Sel(16);
      tTofAnaTestbeam->SetDCh4Sel(16*dScaleFactor);
      break;

    case 100601:
    case 200601:
    case 210601:
    case 700601:
    case 701601:
    case 702601:
    case 703601:
    case 800601:
    case 801601:
    case 802601:
    case 803601:
    case 804601:
    case 805601:
    case 600601:
      switch (iBRefType)
      {
        case 5:
          switch(iBRefModule)
          {
            case 0:
              tTofAnaTestbeam->SetTShift(-5.3);
              tTofAnaTestbeam->SetTOffD4(17.);
              break;

            case 1:
              tTofAnaTestbeam->SetTShift(0.29);
              tTofAnaTestbeam->SetTOffD4(17.);
              break;   
          }

          switch(iSel2Type)
          {
            case 0:
              tTofAnaTestbeam->SetSel2TOff(2.9);
              break;

            case 1:
              tTofAnaTestbeam->SetSel2TOff(0.);
              break;

            case 6:
              tTofAnaTestbeam->SetSel2TOff(0.070);
              break;

            default:
              ;
          }
          break;

        case 1:
          tTofAnaTestbeam->SetTShift(2.5);
          tTofAnaTestbeam->SetTOffD4(17.);
          tTofAnaTestbeam->SetSel2TOff(2.9);
          break;

        case 6:
          tTofAnaTestbeam->SetTShift(-0.5);
          tTofAnaTestbeam->SetTOffD4(17.);
          tTofAnaTestbeam->SetSel2TOff(0.);
          break;

        default:
          std::cout << "-E- ana_hits: setup not implemented!" << std::endl;
          return;
      }

      tTofAnaTestbeam->SetCh4Sel(16);
      tTofAnaTestbeam->SetDCh4Sel(16*dScaleFactor);
      break;

    case 200100:
    case 210100:
    case 700100:
    case 701100:
    case 702100:
    case 703100:
    case 800100:
    case 801100:
    case 802100:
    case 803100:
    case 804100:
    case 805100:
    case 600100:
    case 601100:
      switch (iBRefType)
      {     
        case 5:
          tTofAnaTestbeam->SetTShift(-6.3);
          tTofAnaTestbeam->SetTOffD4(17.);
          tTofAnaTestbeam->SetSel2TOff(2.9);

          switch(iSel2Type)
          {
            case 0:
              break;

            case 6:
              switch(iSel2Counter)
              {
                case 0:
                  tTofAnaTestbeam->SetSel2TOff(2.);
                  break;

                case 1:
                  tTofAnaTestbeam->SetSel2TOff(0.);
                  break;
              }
              break;

            default:
              ;
          }
          break;

        case 6:
          tTofAnaTestbeam->SetTShift(-3.);
          tTofAnaTestbeam->SetTOffD4(17.);
          tTofAnaTestbeam->SetSel2TOff(-2.9);
          break;

        default:
          std::cout << "-E- ana_hits: setup not implemented!" << std::endl;
          return;
      }

      tTofAnaTestbeam->SetCh4Sel(32);
      tTofAnaTestbeam->SetDCh4Sel(32*dScaleFactor);
      break;

    default:
      std::cout << "-E- ana_hits: setup not implemented!" << std::endl;
      return;
  }

  // ---------------------------------------------------------------------------

  tRun->AddTask(tTofAnaTestbeam);

  // ---------------------------------------------------------------------------

  FairRuntimeDb* tRuntimeDb = tRun->GetRuntimeDb();
  FairParAsciiFileIo* tParIo1 = new FairParAsciiFileIo();
  tParIo1->open(tParFileList, "in");
  tRuntimeDb->setFirstInput(tParIo1);

  // ---------------------------------------------------------------------------

  std::cout << "-I- ana_hits: initializing run..." << std::endl;
  tRun->Init();

  std::cout << "-I- ana_hits: starting run..." << std::endl;
  tRun->Run(0, iNEvents);

  // ---------------------------------------------------------------------------

  gROOT->LoadMacro((tMacroDir + "/save_hst.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_over_MatD4sel.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_over_Mat04D4best.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_TIS.C").Data());
  gROOT->LoadMacro((tMacroDir + "/pl_eff_XY.C").Data());
  
  gInterpreter->ProcessLine("save_hst(\"" + tHistoFile + "\", kTRUE)");

  gInterpreter->ProcessLine("pl_over_MatD4sel()");
  gInterpreter->ProcessLine("pl_over_Mat04D4best(1)");
  gInterpreter->ProcessLine("pl_TIS()");
  gInterpreter->ProcessLine("pl_eff_XY()");

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
