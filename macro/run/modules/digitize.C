/** Macro module for registration of digitiser tasks to a run
 **
 ** The run instance can either be FairRunSim or FairRunAna.
 ** The macro assumes a CbmSetup instance. The digitisers for the
 ** active detectors within the setup are instantiated.
 **
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 10 February 2016
 **/


void digitize()
{


	// -----   Get the run instance   ------------------------------------------
	FairRun* run = FairRun::Instance();
	if ( ! run ) {
		std::cerr << "-E- digitize: No run instance!" << std::endl;
		return;
	}
	std::cout << std::endl;
	std::cout << "-I- Macro digitize.C called for run " << run->GetName()
			      << std::endl;
  // -------------------------------------------------------------------------


	// -----   Get the CBM setup instance   ------------------------------------
	CbmSetup* setup = CbmSetup::Instance();
	std::cout << std::endl;
	std::cout << "-I- digitize: Found setup " << setup->GetTitle() << std::endl;
  // -------------------------------------------------------------------------


  // -----   MVD Digitiser   -------------------------------------------------
	if ( setup->IsActive(kMvd) ) {
		FairTask* mvdDigi = new CbmMvdDigitizer("MVD Digitiser", 0);
		run->AddTask(mvdDigi);
		std::cout << "-I- digitize: Added task " << mvdDigi->GetName()
				      << std::endl;
	}
  // -------------------------------------------------------------------------

  
  // -----   STS Digitiser   -------------------------------------------------
	if ( setup->IsActive(kSts) ) {

	  CbmStsDigitize* stsDigi = new CbmStsDigitize();
	  run->AddTask(stsDigi);
      std::cout << "-I- digitize: Added task " << stsDigi->GetName()
				<< std::endl;
	}
  // -------------------------------------------------------------------------


  // -----   RICH Digitiser   ------------------------------------------------
	if ( setup->IsActive(kRich) ) {
		FairTask* richDigi = new CbmRichDigitizer();
		run->AddTask(richDigi);
		std::cout << "-I- digitize: Added task " << richDigi->GetName()
				      << std::endl;
	}
  // -------------------------------------------------------------------------


  // -----   MUCH Digitiser   ------------------------------------------------
	if ( setup->IsActive(kMuch) ) {

		// --- Parameter file name
		TString geoTag;
		setup->GetGeoTag(kMuch, geoTag);
		std::cout << geoTag(0,4) << std::endl;
		TString parFile = gSystem->Getenv("VMCWORKDIR");
		parFile = parFile + "/parameters/much/much_" + geoTag(0,4)
				    + "_digi_sector.root";
		std::cout << "Using parameter file " << parFile << std::endl;

		// --- Digitiser for GEM
		FairTask* gemDigi = new CbmMuchDigitizeGem(parFile.Data());
		run->AddTask(gemDigi);
		std::cout << "-I- digitize: Added task " << gemDigi->GetName()
				      << std::endl;

		// --- Digitiser for Straws
		FairTask* strawDigi = new CbmMuchDigitizeStraws(parFile.Data());
		run->AddTask(strawDigi);
		std::cout << "-I- digitize: Added task " << strawDigi->GetName()
				      << std::endl;
	}
  // -------------------------------------------------------------------------


  // -----   TRD Digitiser   -------------------------------------------------
	if ( setup->IsActive(kTrd) ) {
		CbmTrdRadiator *radiator = new CbmTrdRadiator(kTRUE,"K++");
		FairTask* trdDigi = new CbmTrdDigitizerPRF(radiator);
		run->AddTask(trdDigi);
		std::cout << "-I- digitize: Added task " << trdDigi->GetName()
				      << std::endl;
	}
  // -------------------------------------------------------------------------
  

  // -----   TOF Digitiser   -------------------------------------------------
	if ( setup->IsActive(kTof) ) {
      Int_t iVerbose = 0;
      CbmTofDigitizerBDF* tofDigi = new CbmTofDigitizerBDF("TOF Digitizer BDF",iVerbose);
      tofDigi->SetOutputBranchPersistent("TofDigi",            kFALSE);
      tofDigi->SetOutputBranchPersistent("TofDigiMatchPoints", kFALSE);
      TString paramDir = gSystem->Getenv("VMCWORKDIR");
      tofDigi->SetInputFileName( paramDir + "/parameters/tof/test_bdf_input.root"); // Required as input file name not read anymore by param class
//      tofDigi->SetHistoFileName( digiOutFile ); // Uncomment to save control histograms
      run->AddTask(tofDigi);
      
		std::cout << "-I- digitize: Added task " << tofDigi->GetName()
				      << std::endl;
	}
  // -------------------------------------------------------------------------

}
