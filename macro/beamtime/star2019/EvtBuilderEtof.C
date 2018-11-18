/** @file FHodoLabSetup
 ** @author Florian Uhlig <f.uhlig@gsi.de>
 ** @date 20.06.2016
 **
 ** ROOT macro to read tsa files which have been produced with the new data transport
 ** Convert data into cbmroot format.
 ** Uses CbmFlibTestSource as source task.
 */

// In order to call later Finish, we make this global
FairRunOnline *run = NULL;

void EvtBuilderEtof(TString inFile = "", TString sHostname = "localhost",
                 Int_t iStartFile = -1, Int_t iStopFile = -1,
                 Int_t iServerRefreshRate = 100, Int_t iServerHttpPort = 8080  )
{
  TString srcDir = gSystem->Getenv("VMCWORKDIR");
//  TString inDir  = srcDir + "/input/";
//  if( "" != inFile )
//   inFile = inDir + inFile;

  // --- Specify number of events to be produced.
  // --- -1 means run until the end of the input file.
  Int_t nEvents = -1;

  // --- Specify output file name (this is just an example)
  TString outFile = "data/test.root";
  TString parFile = "data/testparam.root";

  // --- Set log output levels
  FairLogger::GetLogger();
  gLogger->SetLogScreenLevel("INFO");
//  gLogger->SetLogScreenLevel("DEBUG");
//  gLogger->SetLogScreenLevel("DEBUG2"); // Print raw messages
  gLogger->SetLogVerbosityLevel("LOW");

  // --- Define parameter files
  TList *parFileList = new TList();
  TString paramDir = "./";
/*
  TString paramFileMuch = paramDir + "MuchUnpackPar.par";
  TObjString* tutDetDigiFileMuch = new TObjString(paramFileMuch);
  parFileList->Add(tutDetDigiFileMuch);
*/
  TString paramFileTof = paramDir + "etofEvtBuildPar.par";
  TObjString* tutDetDigiFileTof = new TObjString(paramFileTof);
  parFileList->Add(tutDetDigiFileTof);

  // --- Set debug level
  gDebug = 0;

  std::cout << std::endl;
  std::cout << ">>> EvtBuilderEtof: output file is " << outFile << std::endl;

  // ========================================================================
  // ========================================================================

  std::cout << std::endl;
  std::cout << ">>> EvtBuilderEtof: Initialising..." << std::endl;

  // Get4 Unpacker
  CbmStar2019EventBuilderEtof* etofEventBuilder = new CbmStar2019EventBuilderEtof();
  etofEventBuilder->SetSandboxMode();

  // --- Source task
  CbmMcbm2018Source* source = new CbmMcbm2018Source();
  if( "" != inFile )
  {
      if( 0 <= iStartFile && iStartFile < iStopFile )
      {
         for( Int_t iFileIdx = iStartFile; iFileIdx < iStopFile; ++iFileIdx )
         {
            TString sFilePath = Form( "%s_%04u.tsa", inFile.Data(), iFileIdx );
            source->AddFile( sFilePath  );
            std::cout << "Added " << sFilePath <<std::endl;
         } // for( Int_t iFileIdx = iStartFile; iFileIdx < iStopFile; ++iFileIdx )
      } // if( 0 < iStartFile && 0 < iStopFile )
         else source->SetFileName(inFile);
  } // if( "" != inFile )
      else
      {
         source->SetHostName( sHostname );
         source->SetPortNumber( 5556 );
      }

  source->AddUnpacker(etofEventBuilder,  0x60, 6); //gDPBs

  // --- Event header
  FairEventHeader* event = new CbmTbEvent();
  event->SetRunId(1);

  // --- Run
  run = new FairRunOnline(source);
  run->SetOutputFile(outFile);
  run->SetEventHeader(event);
  run->ActivateHttpServer( iServerRefreshRate, iServerHttpPort ); // refresh each 100 events
  run->SetAutoFinish(kFALSE);

  // -----   Runtime database   ---------------------------------------------
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  Bool_t kParameterMerged = kTRUE;
  FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
  FairParAsciiFileIo* parIn = new FairParAsciiFileIo();
  parOut->open(parFile.Data());
  parIn->open(parFileList, "in");
  rtdb->setFirstInput(parIn);
  rtdb->setOutput(parOut);

  run->Init();

  // --- Start run
  TStopwatch timer;
  timer.Start();
  std::cout << ">>> EvtBuilderEtof: Starting run..." << std::endl;
  run->Run(nEvents, 0); // run until end of input file
  timer.Stop();

  std::cout << "Processed " << std::dec << source->GetTsCount() << " timeslices" << std::endl;

  run->Finish();

  // --- End-of-run info
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  std::cout << std::endl << std::endl;
  std::cout << ">>> EvtBuilderEtof: Macro finished successfully." << std::endl;
  std::cout << ">>> EvtBuilderEtof: Output file is " << outFile << std::endl;
  std::cout << ">>> EvtBuilderEtof: Real time " << rtime << " s, CPU time "
	    << ctime << " s" << std::endl;
  std::cout << std::endl;
}
