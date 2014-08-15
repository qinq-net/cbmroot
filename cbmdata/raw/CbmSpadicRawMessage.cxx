/** @file CbmSpadicRawMessage.cxx
 ** @author Florian Uhlig <f.uhlig@gsi.de>
 ** @date 15.08.2014
 **/

#include "CbmSpadicRawMessage.h"

// -----   Default constructor   -------------------------------------------
CbmSpadicRawMessage::CbmSpadicRawMessage() 
 : CbmRawMessage(), 
   fNrSamples(-1),
   fSamples()  
{
}

CbmSpadicRawMessage::CbmSpadicRawMessage(Int_t FlibLink, Int_t FebId, Int_t ChannelId,
			     Int_t EpochMarker, Int_t Time, Int_t NrSamples,
                             Int_t* Samples)
 : CbmRawMessage(FlibLink, FebId, ChannelId, EpochMarker, Time),
   fNrSamples(NrSamples),
   fSamples()     
{
  for (Int_t i = 0; i < NrSamples; ++i) {
    fSamples[i] = Samples[i];
  }

}

// -------------------------------------------------------------------------



// -----   Copy constructor   ----------------------------------------------
//CbmSpadicRawMessage::CbmSpadicRawMessage(const CbmSpadicRawMessage& digi) : fMatch(NULL) { } 
// -------------------------------------------------------------------------



// -----   Assignment operator   -------------------------------------------
//CbmSpadicRawMessage& CbmSpadicRawMessage::operator=(const CbmSpadicRawMessage& digi) {
//	fMatch = NULL;
//	return *this;
//}
// -------------------------------------------------------------------------




ClassImp(CbmSpadicRawMessage)
