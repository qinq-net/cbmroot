// --------------------------------------------------------------------------
//
// Macro for testing the STS addressing scheme
//
// V. Friese   23/08/2017
//
// --------------------------------------------------------------------------



void test_address()
{

  // Dummy just to get the libraries loaded
  new CbmStsDigi();

  // Maximal numbers of allowed elements
  Int_t nUnits = 64;
  Int_t nLadders = 32;
  Int_t nHalfLadders = 2;
  Int_t nModules = 32;
  Int_t nSensors = 16;
  Int_t nSides = 2;

  nUnits = 16;
  nLadders = 15;
  nHalfLadders = 2;
  nModules = 8;
  nSensors = 4;
  nSides = 2;

  Int_t nTests = 0;
  Bool_t success = kTRUE;
  TStopwatch timer;
  timer.Start();


  // Loop over allowed elements
  for (UInt_t unit = 0; unit < nUnits; unit++) {
    for (UInt_t ladd = 0; ladd < nLadders; ladd++) {
      for (UInt_t hlad = 0; hlad < nHalfLadders; hlad++) {
        for (UInt_t modu = 0; modu < nModules; modu++) {
          for (UInt_t sens = 0; sens < nSensors; sens++) {
            for (UInt_t side = 0; side < nSides; side++) {

              Int_t address = CbmStsAddress::GetAddress(unit, ladd, hlad, modu, sens, side);

              UInt_t rSyst = CbmStsAddress::GetElementId(address, kStsSystem);
              UInt_t rUnit = CbmStsAddress::GetElementId(address, kStsUnit);
              UInt_t rLadd = CbmStsAddress::GetElementId(address, kStsLadder);
              UInt_t rHlad = CbmStsAddress::GetElementId(address, kStsHalfLadder);
              UInt_t rModu = CbmStsAddress::GetElementId(address, kStsModule);
              UInt_t rSens = CbmStsAddress::GetElementId(address, kStsSensor);
              UInt_t rSide = CbmStsAddress::GetElementId(address, kStsSide);


              if ( rSyst != kSts ) {
                std::cout << "System " << rSyst << " is not STS" << std::endl;
                success = kFALSE;
              }
              if ( CbmStsAddress::GetSystemId(address) != kSts ) {
                std::cout << "GetSystemId " << CbmStsAddress::GetSystemId(address)
                << " does not return STS" << std::endl;
                success = kFALSE;
              }
              if (rUnit != unit) {
                std::cout << "Different unit " << unit << " " << rUnit << std::endl;
                success = kFALSE;
              }
              if (rLadd != ladd) {
                std::cout << "Different ladder " << ladd << " " << rLadd << std::endl;
                success = kFALSE;
              }
              if (rHlad != hlad) {
                std::cout << "Different half-ladder " << hlad << " " << rHlad << std::endl;
                success = kFALSE;
              }
              if (rModu != modu) {
                std::cout << "Different module " << modu << " " << rModu << std::endl;
                success = kFALSE;
              }
              if (rSens != sens) {
                std::cout << "Different sensor " << sens << " " << rSens << std::endl;
                success = kFALSE;
              }
              if (rSide != side) {
                std::cout << "Different side " << side << " " << rSide << std::endl;
                success = kFALSE;
              }
              nTests++;


            } // #side
          } //# sensor
        } //# module
      } //# half-ladder
    } //# ladder
  } //# unit

  timer.Stop();
  if ( success ) std::cout << nTests << " addresses tested successfully." << std::endl;
  else std::cout << "Test ended with errors." << std::endl;
  std::cout << "Consumption: CPU " << timer.CpuTime() << " s, Real "
            << timer.RealTime() << std::endl;

} // end of script

