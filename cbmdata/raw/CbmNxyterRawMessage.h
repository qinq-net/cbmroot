#ifndef CBMNXYTERRAWMESSAGE_H
#define CBMNXYTERRAWMESSAGE_H 1


#include "CbmRawMessage.h"

class CbmNxyterRawMessage : public CbmRawMessage
{

 public:

  /** Default constructor  **/
  CbmNxyterRawMessage();

  /** Constructor  **/
  CbmNxyterRawMessage(Int_t FlibLink, Int_t FebId, Int_t ChannelId, Int_t EpochMarker, Int_t Time,
                        Int_t ADCval);

	
  /** Destructor  **/
  virtual ~CbmNxyterRawMessage() { };

	
 private:

  Int_t fADCvalue;

  ClassDef(CbmNxyterRawMessage,1);

};

#endif // CBMNXYTERRAWMESSAGE_H
