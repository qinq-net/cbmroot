/** @file CbmRawMessage.h
 ** @author Florian Uhlig <f.uhlig@gsi.de>
 ** @date 15.08.2014
 **/


#ifndef CBMRAWMESSAGE_H
#define CBMRAWMESSAGE_H 1


#include "TObject.h"


/** @class CbmDigi
 ** @brief Base class for representation of raw information which comes from the FLIB ind CbmNet messages.
 ** @author Florian Uhlig <f.uhlig@gsi.de>
 ** @version 1.0
 **
 ** CbmRawMessage is a base class for the ROOT representation of
 ** the smallest information unit delivered by the detector front-ends.
 ** This data is packed into CbmNet messages in the FLIB and either read
 ** from file or directly from memory. 
 ** The CbmRawMessage contains the complete information from a single hit of 
 *a single electronics channel and should be context free. 
 ** The absolute time and the unique channel number has to be 
 ** calculated in another data level, probably the Digi.
 ** The information content is the FLIB link, the feb id within this link, the channel ID within this feb,
 ** the corresponding epoch marker (0 if there was no previous one), and the
 ** time stamp from the last epoch marker. 
 **
 **/
class CbmRawMessage : public TObject
{

 public:

  /** Default constructor  **/
  CbmRawMessage();

  /** Constructor  **/
  CbmRawMessage(Int_t FlibLink, Int_t FebId, Int_t ChannelId,
		Int_t EpochMarker, Int_t Time);

	
  /** Copy constructor  **/
  //  CbmRawMessage(const CbmRawMessage&);



  /** Destructor  **/
  virtual ~CbmRawMessage() { };

	
  /** Assignment operator  **/	
  //  CbmRawMessage& operator=(const CbmRawMessage&);

 protected:

  Int_t fFlibLink;
  Int_t fFebId;
  Int_t fChannelId;
  Int_t fEpochMarker;
  Int_t fTime;

  ClassDef(CbmRawMessage,1);

};

#endif
