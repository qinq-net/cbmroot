#include "TTrbHeader.h"

TTrbHeader::TTrbHeader() : TNamed(),
    fiTriggerPattern(0),
    fiTriggerType(0),
    fiTimeInSpill(0)
    {}

TTrbHeader::~TTrbHeader() 
{}

Bool_t TTrbHeader::TriggerFired(Int_t iTrg){
  // check whether Trigger Pattern matches iTrg in any bit
  if( fiTriggerPattern & (0x1 << iTrg) ) {
    return kTRUE;
  }else{
    return kFALSE;
  }
  return kFALSE;
}

ClassImp(TTrbHeader)
