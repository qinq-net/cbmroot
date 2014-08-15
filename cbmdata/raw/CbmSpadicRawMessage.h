/** @file CbmSpadicRawMessage.h
 ** @author Florian Uhlig <f.uhlig@gsi.de>
 ** @date 15.08.2014
 **/


#ifndef CBMSPADICRAWMESSAGE_H
#define CBMSPADICRAWMESSAGE_H 1


#include "CbmRawMessage.h"


/** @class CbmSpadicRawMessage
 ** @brief Class for representation of raw spadic information, derrived from CbmRawMessage.
 ** @author Florian Uhlig <f.uhlig@gsi.de>
 ** @version 1.0
 **
 ** CbmSpadicRawMessage is a base class for the ROOT representation of
 ** the smallest information unit delivered by the spadic front-ends.
 ** The CbmSpadicRawMessage extends the CbmRawMessage by the spadic sample data and the number of samples. 
 ** a single electronics channel and should be context free. an
 **
 **/
class CbmSpadicRawMessage : public CbmRawMessage
{

 public:

  /** Default constructor  **/
  CbmSpadicRawMessage();

  /** Constructor  **/
  CbmSpadicRawMessage(Int_t FlibLink, Int_t FebId, Int_t ChannelId,
		Int_t EpochMarker, Int_t Time, Int_t NrSamples, Int_t* Samples);

	
  /** Destructor  **/
  virtual ~CbmSpadicRawMessage() { };

	
 private:

  Int_t fNrSamples;
  Int_t fSamples[32];

  ClassDef(CbmSpadicRawMessage,1);

};

#endif
