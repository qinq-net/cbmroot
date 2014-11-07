#include "CbmNxyterRawMessage.h"

// -----   Default constructor   -------------------------------------------
CbmNxyterRawMessage::CbmNxyterRawMessage() 
 : CbmRawMessage()
{
}

// ------ Constructor -----------------------------------------------------
CbmNxyterRawMessage::CbmNxyterRawMessage(Int_t FlibLink, Int_t FebId, Int_t ChannelId, Int_t EpochMarker, Int_t Time,
                        Int_t ADCval)
 : CbmRawMessage(FlibLink, FebId, ChannelId, EpochMarker, Time),
   fADCvalue(ADCval)
{

}

// -------------------------------------------------------------------------



// -----   Copy constructor   ----------------------------------------------
//CbmNxyterRawMessage::CbmNxyterRawMessage(const CbmNxyterRawMessage& digi) { } 
// -------------------------------------------------------------------------



// -----   Assignment operator   -------------------------------------------
//CbmNxyterRawMessage& CbmNxyterRawMessage::operator=(const CbmNxyterRawMessage& digi) {
//	return *this;
//}
// -------------------------------------------------------------------------




ClassImp(CbmNxyterRawMessage)
