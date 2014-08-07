// -------------------------------------------------------------------------
// -----                     CbmMvdMimosa26AHR source file             -----
// -----                  Created 31/01/11  by M. Deveaux              -----
// -------------------------------------------------------------------------

#include "CbmMvdMimosa26AHR.h"
#include "TMath.h"

#include <iostream>


using std::cout;
using std::endl;



// -----   Default constructor   -------------------------------------------
CbmMvdMimosa26AHR::CbmMvdMimosa26AHR()  {
  
  fMimosaName="Mimosa-26AHR";      // Clear name of the simulated sensor
  fPixelPitchX=0.0025;	  	    // Pixel pitch in x of this sensor
  fPixelPitchY=0.0025;              // Pixel pitch in y of this sensor
  fNPixelsX=1152;    		    // Number of pixels in row
  fNPixelsY=576;  		    // Number of pixels in col
  fNPixels=fNPixelsX*fNPixelsY;	    // Number of pixels in sensor
  fPixelSignX=1;       		    // Direction of the pixel count X, if true, Pixel x=0 is at
				    // left corner, else at right corner
  fPixelSignY=1;       		    // Direction of the pixel count Y, if true, Pixel x=0 is at
				    // the lower corner, else at upper corner
  fShutterSign=1;				    
  fIntegrationTime=115.2e3;  	    // Integration time of the pixels in ns
  fEpiTh=14e-4;		   	    // Thickness of the epitaxial layer
  
  /** Description of the sensor for the digitizer //mesaured by melisa for Mimosa26AHR**/
       fNoise	     = 13;
       fLorentzPar0 = 520.;
       fLorentzPar1 = 0.34;
       fLorentzPar2 = -1.2;
       fLandauMPV   = 877.4;
       fLandauSigma = 204.93;
       fLandauGain= 3.3;
  
  /** ADC description **/
  
  fAdcDynamic = 150;
  fAdcOffset  = 0;
  fAdcBits    = 1;
  fAdcSteps= TMath::Power(2,fAdcBits);
  fAdcStepSize=fAdcDynamic/fAdcSteps;
  fStatesPerBank=6;
  fStatesPerLine=9;
  fStatesPerFrame=570;
  fPixelsPerBank=64;
  
  
  /** Self-organizsation **/
   
  fValidData=kTRUE;
  
  
}

// -----   Destructor   ----------------------------------------------------
CbmMvdMimosa26AHR::~CbmMvdMimosa26AHR() {
}
// -------------------------------------------------------------------------

ClassImp(CbmMvdMimosa26AHR)
