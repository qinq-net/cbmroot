// -----------------------------------------------------------------------------
// ----- setup_analysis FairRoot macro                                     -----
// -----                                                                   -----
// ----- created by N. Herrmann on 2014-04-18                              -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

// Max nEvents: 198999999999
void setup_unpack(Int_t calMode=1,
                  const TString& tIdentifier="CbmTofSps_26Nov1715",
                  const TString& tDirectory="./data/",
                  Int_t iSet=0) 
{
   // Verbosity level (0=quiet, 1=event level, 2=track level, 3=debug, 4=raw debug)
  Int_t iVerbose = 2;
  // Specify log level (INFO, DEBUG, DEBUG1, ...)
  TString logLevel = "FATAL";
  //TString logLevel = "ERROR";
  TString logLevel = "INFO";
  //TString logLevel = "DEBUG";
  //TString logLevel = "DEBUG1";
  //TString logLevel = "DEBUG2";
  //TString logLevel = "DEBUG3";
  FairLogger* log;  
  Int_t nEvents = 10;
  Int_t start_run=0;
  Int_t end_run=11;
//  TString logVerbosity = "LOW";
  TString logVerbosity = "MEDIUM";
//  TString logVerbosity = "HIGH";
  char *cFileId=tIdentifier;
   //  Parameter files.
   // Add TObjectString containing the different file names to
   // a TList which is passed as input to the FairParAsciiFileIo.
   // The FairParAsciiFileIo will take care to create on the fly 
   // a concatenated input parameter file.
   TList *parFileList = new TList();
   
   TString workDir  = gSystem->Getenv("VMCWORKDIR");
   TString paramDir = workDir + "/macro/tof/beamtime/nov15";

   TObjString unpParFile = paramDir + "/parUnpCernNov2015.txt";					
   parFileList->Add(&unpParFile);

   TObjString calParFile;
   TString cOutfileId;
   //TString TofGeo="v14b";
   TString TofGeo="v15c";
   TObjString mapParFile;

   switch(iSet){
   case 0: 
     calParFile = paramDir + "/parCalib_basic.txt";										
     cOutfileId = Form("%s",tIdentifier.Data());
     mapParFile = paramDir + "/parMapCernNov2015.txt";						
     break;

   case 1:
   /*
     calParFile = paramDir + "/parCalib_basic.txt";
     cOutfileId = Form("%s%s",tIdentifier.Data(),"_nopla");
     mapParFile = paramDir + "/parMapCosmicsThuMay2015_nopla.txt";
 	int mapping =1;   
 	*/  
	break;

   default:
     ;
   }
   parFileList->Add(&calParFile);

   cout << " Output File tag "<< cOutfileId << endl;

   parFileList->Add(&mapParFile);
   //TObjString convParFile = paramDir + "/parConvFeb2015.txt";
   //parFileList->Add(&convParFile);

   TObjString tofDigiFile = workDir + "/parameters/tof/tof_" + TofGeo + ".digi.par"; // TOF digi file
   //parFileList->Add(&tofDigiFile);   

   TObjString tofDigiBdfFile = workDir  + "/parameters/tof/tof_" + TofGeo +".digibdf.par";
   //parFileList->Add(&tofDigiBdfFile);

   
   TString geoDir  = gSystem->Getenv("VMCWORKDIR");
   TString geoFile = geoDir + "/geometry/tof/geofile_tof_" + TofGeo + ".root";
   TFile* fgeo = new TFile(geoFile);

   TGeoManager *geoMan = (TGeoManager*) fgeo->Get("FAIRGeom");
   if (NULL == geoMan){
     cout << "<E> FAIRGeom not found in geoFile"<<endl;
     return;
   }
   

   if(0){
   TGeoVolume* master=geoMan->GetTopVolume();
   master->SetVisContainers(1); 
   master->Draw("ogl"); 
   }
   // Parameter file output
   TString parFile = paramDir + "/unpack_" + cOutfileId + ".params.root";
   
   // Output file
   TString outFile = paramDir + "/unpack_" + cOutfileId + ".out.root";
   // ----    Debug option   -------------------------------------------------
   gDebug = 0;
   // ------------------------------------------------------------------------

   // -----   Timer   --------------------------------------------------------
   TStopwatch timer;
   timer.Start();
   // ------------------------------------------------------------------------
   
   // =========================================================================
   // ===                           Unpacker                                ===
   // =========================================================================
   CbmHldSource* source = new CbmHldSource();
     source->AddPath(tDirectory, tIdentifier+"*");
     //     source->AddFile(hldfile); 
   /*   
   TTriglogUnpackTof* tofTriglogUnpacker = new TTriglogUnpackTof();
   //   tofTriglogUnpacker->SetSaveTriglog(kTRUE);
   tofTriglogUnpacker->SetSaveTriglog(kFALSE);
   source->AddUnpacker( tofTriglogUnpacker );
  
   TMbsUnpackTof* tofMbsDataUnpacker = new TMbsUnpackTof();
   source->AddUnpacker( tofMbsDataUnpacker );
   */
   TTrbUnpackTof* tofTrbDataUnpacker = new TTrbUnpackTof(10,1,31,0,0);
   source->AddUnpacker( tofTrbDataUnpacker );
   
   gLogger->SetLogScreenLevel(logLevel.Data());
   gLogger->SetLogVerbosityLevel(logVerbosity.Data());

   // -----   Online/Offline MBS run   -----------------------------------

   FairRunOnline *run = FairRunOnline::Instance();
   run->SetSource(source);
   run->SetOutputFile(outFile);
   run->SetAutoFinish(kFALSE);

   // --------------------------------------------------------------------
   // ===                        End of Unpacker                            ===
   // =========================================================================
   
   // =========================================================================
   // ===                     Unpacker monitoring                           ===
   // =========================================================================
   TMbsUnpTofMonitor* tofUnpMonitor = new TMbsUnpTofMonitor("Tof Unp Moni", iVerbose);
   run->AddTask(tofUnpMonitor);
   // ===                 End of Unpacker monitoring                        ===
   // =========================================================================
   
   // =========================================================================
   // ===                         Calibration                               ===
   // =========================================================================
   
   TMbsCalibTof* tofCalibration = new TMbsCalibTof("Tof Calibration", iVerbose);
   run->AddTask(tofCalibration);
   // ===                      End of Calibration                           ===
   // =========================================================================
   
   // =========================================================================
   // ===                         Mapping                                   ===
   // =========================================================================
   if(calMode>0){
   TMbsMappingTof* tofMapping = new TMbsMappingTof("Tof Mapping", iVerbose);
   tofMapping->SetSaveDigis(kTRUE);
   run->AddTask(tofMapping);
   }
   // ===                      End of Mapping                               ===
   // =========================================================================
   
   // =========================================================================
   // ===                  GO4 like output formatting                       ===
   // =========================================================================
//   TMbsConvTof* tofConversion = new TMbsConvTof("Tof Conversion", iVerbose);
//   run->AddTask(tofConversion);
   // ===                  End of output conversion                         ===
   // =========================================================================
   CbmTofOnlineDisplay* display = new CbmTofOnlineDisplay();
   display->MonitorTdcOccupancy(kTRUE);
   display->SetNumberOfTDC(38);
   // display->SetNumberOfSEB(5);
   //   display->SetResMonitorEna(kTRUE);
   display->SetUpdateInterval(1000);
   run->AddTask(display);   

   // -----  Parameter database   --------------------------------------------
   FairRuntimeDb* rtdb = run->GetRuntimeDb();
   
   Bool_t kParameterMerged = kTRUE;
   FairParRootFileIo* parIo2 = new FairParRootFileIo(kParameterMerged);
   parIo2->open(parFile.Data(), "UPDATE");
   parIo2->print();
   rtdb->setFirstInput(parIo2);
   
   FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo();
   parIo1->open(parFileList, "in");
   parIo1->print();
//   rtdb->setFirstInput(parIo1);
   rtdb->setSecondInput(parIo1);
   
   rtdb->print();
   rtdb->printParamContexts();
   // ------------------------------------------------------------------------

   // -----   Intialise and run   --------------------------------------------
   run->Init();

   cout << "Starting run" << endl;
   // ------------------------------------------------------------------------

   // -----   Finish   -------------------------------------------------------
/*
   timer.Stop();
   Double_t rtime = timer.RealTime();
   Double_t ctime = timer.CpuTime();
   cout << endl << endl;
   cout << "Macro finished successfully." << endl;
   cout << "Output file is " << outFile << endl;
   cout << "Parameter file is " << parFile << endl;
   cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
   cout << endl;
*/
   // ------------------------------------------------------------------------

//   delete run;
}

