/** @file CbmStsDigitizeParameters.cxx
 ** @author Florian Uhlig <f.uhlig@gsi.de>
 ** @date 12.06.2017
 **/

#include "CbmStsDigitizeParameters.h"

#include "FairParamList.h"
#include "FairDetParIo.h"
#include "FairParIo.h"

#include <sstream>

using std::fixed;
using std::setprecision;
using std::setw;
using std::right;

ClassImp(CbmStsDigitizeParameters)


// -----   Constructor   ----------------------------------------------------
CbmStsDigitizeParameters::CbmStsDigitizeParameters(const char* name, 
                                               const char* title,
                                               const char* context)
  : FairParGenericSet(name, title, context), 
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
std::string CbmStsDigitizeParameters::ToString() const {
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


// -----   Public method clear   -------------------------------------------
void CbmStsDigitizeParameters::clear() 
{
  status = kFALSE;
  resetInputVersions();
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------

void CbmStsDigitizeParameters::putParams(FairParamList* l) 
{

    if (!l) return;

    l->add("ELossModel", fELossModel);
    l->add("UseLorentzShift", fUseLorentzShift);
    l->add("UseDiffusion", fUseDiffusion);
    l->add("UseCrossTalk", fUseCrossTalk);
    l->add("GenerateNoise", fGenerateNoise);
    l->add("Vdep", fVdep);
    l->add("Vbias", fVbias);
    l->add("Temperature", fTemperature);
    l->add("Ccoup", fCcoup);
    l->add("Cis", fCis);
    l->add("DynRange", fDynRange);
    l->add("Threshold", fThreshold);
    l->add("NofAdc", fNofAdc);
    l->add("TimeResolution", fTimeResolution);
    l->add("DeadTime", fDeadTime);
    l->add("Noise", fNoise);
    l->add("ZeroNoiseRate", fZeroNoiseRate);
    l->add("StripPitch", fStripPitch);
    l->add("DiscardSecondaries", fDiscardSecondaries);

}

Bool_t CbmStsDigitizeParameters::getParams(FairParamList* l) 
{

    if (!l) return kFALSE;

    if ( ! l->fill("ELossModel", &fELossModel) ) return kFALSE;
    if ( ! l->fill("UseLorentzShift", &fUseLorentzShift) ) return kFALSE;
    if ( ! l->fill("UseDiffusion", &fUseDiffusion) ) return kFALSE;
    if ( ! l->fill("UseCrossTalk", &fUseCrossTalk) ) return kFALSE;
    if ( ! l->fill("GenerateNoise", &fGenerateNoise) ) return kFALSE;
    if ( ! l->fill("Vdep", &fVdep) ) return kFALSE;
    if ( ! l->fill("Vbias", &fVbias) ) return kFALSE;
    if ( ! l->fill("Temperature", &fTemperature) ) return kFALSE;
    if ( ! l->fill("Ccoup", &fCcoup) ) return kFALSE;
    if ( ! l->fill("Cis", &fCis) ) return kFALSE;
    if ( ! l->fill("DynRange", &fDynRange) ) return kFALSE;
    if ( ! l->fill("Threshold", &fThreshold) ) return kFALSE;
    if ( ! l->fill("NofAdc", &fNofAdc) ) return kFALSE;
    if ( ! l->fill("TimeResolution", &fTimeResolution) ) return kFALSE;
    if ( ! l->fill("DeadTime", &fDeadTime) ) return kFALSE;
    if ( ! l->fill("Noise", &fNoise) ) return kFALSE;
    if ( ! l->fill("ZeroNoiseRate", &fZeroNoiseRate) ) return kFALSE;
    if ( ! l->fill("StripPitch", &fStripPitch) ) return kFALSE;
    if ( ! l->fill("DiscardSecondaries", &fDiscardSecondaries) ) return kFALSE;

    return kTRUE;
}
