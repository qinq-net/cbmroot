/** @file CbmRawMessage.cxx
 ** @author Florian Uhlig <f.uhlig@gsi.de>
 ** @date 15.08.2014
 **/

#include "CbmRawMessage.h"

#include "FairMultiLinkedData.h"

#include <memory>

// -----   Default constructor   -------------------------------------------
CbmRawMessage::CbmRawMessage() 
 : TObject(), 
   fFlibLink(-1),
   fFebId(-1),
   fChannelId(-1),
   fEpochMarker(-1),
   fTime(-1)
   
{
}

CbmRawMessage::CbmRawMessage(Int_t FlibLink, Int_t FebId, Int_t ChannelId,
			     Int_t EpochMarker, Int_t Time)
 : TObject(), 
   fFlibLink(FlibLink),
   fFebId(FebId),
   fChannelId(ChannelId),
   fEpochMarker(EpochMarker),
   fTime(Time)
   
{
}

// -------------------------------------------------------------------------



// -----   Copy constructor   ----------------------------------------------
//CbmRawMessage::CbmRawMessage(const CbmRawMessage& digi) : fMatch(NULL) { } 
// -------------------------------------------------------------------------



// -----   Assignment operator   -------------------------------------------
//CbmRawMessage& CbmRawMessage::operator=(const CbmRawMessage& digi) {
//	fMatch = NULL;
//	return *this;
//}
// -------------------------------------------------------------------------




ClassImp(CbmRawMessage)
