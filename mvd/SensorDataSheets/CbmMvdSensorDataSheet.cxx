// -------------------------------------------------------------------------
// -----                     CbmMvdSensorDataSheet source file                  -----
// -----                  Created 31/01/11  by M. Deveaux              -----
// -------------------------------------------------------------------------

#include "CbmMvdSensorDataSheet.h"
#include "TMath.h"

#include <iostream>


using std::cout;
using std::endl;



// -----   Default constructor   -------------------------------------------
CbmMvdSensorDataSheet::CbmMvdSensorDataSheet()  {
  
  fMimosaName="DefaulSensor";       // Clear name of the simulated sensor
  fPixelPitchX=18.4e-4;	  	    // Pixel pitch of this sensor
  fPixelPitchY=18.4e-4;             // Pixel pitch of this sensor
  fNPixelsX=0;	    		    // Number of pixels in row
  fNPixelsY=0;	  		    // Number of pixels in col
  fNPixels=0;	  		    // Number of pixels in sensor
  fPixelSignX=1;       		    // Direction of the pixel count X, if true, Pixel x=0 is at
				    // left corner, else at right corner
  fPixelSignY=1;       		    // Direction of the pixel count Y, if true, Pixel x=0 is at
				    // the lower corner, else at upper corner
  fShutterSign=1;      // if true, shutter runs parallel to pixel number
  fIntegrationTime=50e3;  	    // Integration time of the pixels in ns
  fEpiTh=14e-4;		   	    // Thickness of the epitaxial layer
  
  /** Description of the sensor for the digitizer **/
	fLorentzPar0 = 4.12073e+02;
	fLorentzPar1 = 0.8e+00;
	fLorentzPar2 = 0;
	fLandauMPV   = 8.62131e+02;
	fLandauSigma = 2.e+02;
        fLandauGain  = 1.56;
  
  /** ADC description **/
  
  fAdcDynamic = 150;
  fAdcOffset  = 0;
  fAdcBits    = 1;
  fAdcSteps= TMath::Power(2,fAdcBits);
  fAdcStepSize=fAdcDynamic/fAdcSteps;
  fStatesPerBank=0;
  fStatesPerLine=0;
  fStatesPerFrame=0;
  fPixelsPerBank=0;
  
  
  /** Self-organizsation **/
   
  fValidData=kFALSE;
  
  
}

// -----   Destructor   ----------------------------------------------------
CbmMvdSensorDataSheet::~CbmMvdSensorDataSheet() {
}
// -------------------------------------------------------------------------



// -----   Public method Print   -------------------------------------------
void CbmMvdSensorDataSheet::Print(Option_t* opt) {
  cout << " --- " << GetName() << "--------------------------------" << endl;
  cout << " Technical Data:" << endl;
  cout << "--------------------------" << endl;
  cout << "Name of the sensor: " <<  fMimosaName << endl;
  cout << "Pixel pitch (x/y) [µm]: " << fPixelPitchY*1e4 << "/"<<fPixelPitchY*1e-4 << endl;
  cout << "N Pixels (rows/cols): " << fNPixelsX <<"/"<<fNPixelsY << endl;
  cout << "Thickness of active volume[µm]: " <<fEpiTh * 1e4 << endl;
  cout << "Integration time [µs]: " << fIntegrationTime*1e6 << endl;
  cout << "ADC resolution: " << fAdcBits << "bit ADC (1 = discriminator)" <<endl;
  cout << "Default noise [e ENC]: " << fNoise << endl; 
  cout << "Matrix size (x/y) [cm]: " << fPixelPitchX*fNPixelsX << " / " << fPixelPitchY*fNPixelsY << endl;
  cout << endl;
  cout << "Readout system: " << endl;
  cout << "----------------" << endl;
  cout << "PixelNmb X from left to right?: " << fPixelSignX << endl;
  cout << "PixelNmb Y from up to down?: " << fPixelSignY << endl;
  cout << "Rolling shutter from up to down?: " << fShutterSign << endl;
  cout << endl;
  cout << "ADC / SUZE -Settings: " << endl;
  cout << "----------------" << endl;
  cout << "ADC Range [e]: " << fAdcDynamic << endl;
  cout << "ADC offset [e]: " << fAdcOffset << endl;
  cout << "ADC stepsize [e]: " << fAdcStepSize << endl;
  cout << "SUZE states per banks: " << fStatesPerBank << endl;
  cout << "SUZE states per line : " << fStatesPerLine << endl;
  cout << "SUZE states per frame: " << fStatesPerFrame << endl;
  cout << "SUZE pixels per bank: " << fPixelsPerBank << endl;
  cout << endl;
  cout << "Data card properly initialized: " << fValidData << endl;
  
}
// -------------------------------------------------------------------------



ClassImp(CbmMvdSensorDataSheet)
