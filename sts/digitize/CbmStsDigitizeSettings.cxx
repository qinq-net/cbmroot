/** @file CbmStsDigitizeSettings.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 31.05.2017
 **/

#include "CbmStsDigitizeSettings.h"

#include <sstream>

using std::fixed;
using std::setprecision;
using std::setw;
using std::right;

ClassImp(CbmStsDigitizeSettings)


// -----   Constructor   ----------------------------------------------------
CbmStsDigitizeSettings::CbmStsDigitizeSettings() :
  TNamed("StsDigiSettings", "Settings for STS digitisation"),
  fELossModel(-1),
  fUseLorentzShift(kTRUE),
  fUseDiffusion(kTRUE),
  fUseCrossTalk(kTRUE),
  fGenerateNoise(kFALSE),
  fVdep(-1.),
  fVbias(-1.),
  fTemperature(0.),
  fCcoup(-1.),
  fCis(-1.),
  fDynRange(0.),
  fThreshold(0.),
  fNofAdc(0),
  fTimeResolution(0.),
  fDeadTime(0.),
  fNoise(0.),
  fZeroNoiseRate(0.),
  fDeadChannelFrac(0.),
  fStripPitch(-1.),
  fDiscardSecondaries(kFALSE)
{
}
// --------------------------------------------------------------------------



// ----- String output   ----------------------------------------------------
std::string CbmStsDigitizeSettings::ToString() const {
   std::stringstream ss;

   ss << GetTitle() << ":";
   ss << "\n\t  Energy loss model ";
   switch (fELossModel) {
     case 0: ss << "IDEAL"; break;
     case 1: ss << "UNIFORM"; break;
     case 2: ss << "NON_UNIFORM"; break;
     default: ss << "!!! UNKNOWN !!!"; break;
   }
   ss << "\n\t  Lorentz shift     " << (fUseLorentzShift ? "ON" : "OFF");
   ss << "\n\t  Diffusion         " << (fUseDiffusion ? "ON" : "OFF");
   ss << "\n\t  Cross-talk        " << (fUseCrossTalk ? "ON" : "OFF");
   ss << "\n\t  Noise             " << (fGenerateNoise ? "ON" : "OFF");

   ss << "\n\t  Sensor operation conditions :\n";
   ss << "\t\t Depletion voltage         "
       << fVdep << " V\n";
   ss << "\t\t Bias voltage              "
       << fVbias << " V\n";
   ss << "\t\t Temperature               "
       << fTemperature << " K\n";
   ss << "\t\t Coupling capacitance      "
       << fCcoup << " pF\n";
   ss << "\t\t Inter-strip capacitance   "
       << fCis << " pF\n";

   ss << "\t  ASIC parameters :\n";
   ss << "\t\t Dynamic range             "
       << fDynRange << " e\n";
   ss << "\t\t Threshold                 "
       << fThreshold << " e\n";
   ss << "\t\t ADC channels              "
       << fNofAdc << " \n";
   ss << "\t\t Time resolution           "
       << fTimeResolution << " ns\n";
   ss << "\t\t Dead time                 "
       << fDeadTime << " ns\n";
   ss << "\t\t Noise (RMS)               "
       << fNoise << " e\n";
   ss << "\t\t Zero noise rate           "
      << fZeroNoiseRate << " / ns\n";
   ss << "\t\t Fraction of dead channels "
       << fDeadChannelFrac;

   if ( fDiscardSecondaries ) ss << "\n\t!!! Secondaries will be discarded!!!";
   if ( fStripPitch > 0. ) ss << "\n\t!!! Overriding strip pitch with "
       << fStripPitch << " cm !!!";

   return ss.str();
}
// --------------------------------------------------------------------------



