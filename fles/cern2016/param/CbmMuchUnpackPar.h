// -------------------------------------------------------------------------
// -----                 CbmMuchUnpackPar header file                 -----
// -----                 Created 11/11/16  by V. Singhal and A. Kumar -----
// -------------------------------------------------------------------------

#ifndef CBMMUCHUNPACKPAR_H
#define CBMMUCHUNPACKPAR_H

#include "FairParGenericSet.h"

#include "TArrayI.h"

class FairParIo;
class FairParamList;


class CbmMuchUnpackPar : public FairParGenericSet 
{

 public:

  /** Standard constructor **/
  CbmMuchUnpackPar(const char* name    = "CbmMuchUnpackPar",
		    const char* title   = "Much unpacker parameters",
		    const char* context = "Default");


  /** Destructor **/
  virtual ~CbmMuchUnpackPar();
  
  /** Reset all parameters **/
  virtual void clear();

  void putParams(FairParamList*);
  Bool_t getParams(FairParamList*);

  Int_t GetNrOfnDpbsModA() { return fNrOfnDpbsModA; }
  Int_t GetNrOfFebs() { return fNrOfFebs; }
  Int_t GetNrOfChannels() {return fNrOfChannels;}

  Int_t GetPadX(Int_t febid, Int_t channelid) {return fChannelsToPadX[(febid*fNrOfFebs)+channelid];}
  Int_t GetPadY(Int_t febid, Int_t channelid) {return fChannelsToPadY[(febid*fNrOfFebs)+channelid];}

//  Int_t GetChannelToPixelMap(Int_t channel) {return fChannelToPixelMap[channel];}
//  Int_t GetChannelToPlaneMap(Int_t channel) {return fChannelToPlaneMap[channel];}
 private:

  Int_t fNrOfnDpbsModA; // Total number of nDPBs in GEM Module 1
  TArrayI fnDbpsIdsArray; // Array to hold the unique IDs for all GEM nDPBs 
  Int_t fNrOfFebs; // Number of FEBs for a nDPB
  TArrayI fnFebsIdsArray; // Array to hold FEB IDs connected to 1 nDPB
  Int_t fNrOfChannels;
  TArrayI fChannelsToPadX; // Array which stores the corresponding x position of PAD of entire module A 
  TArrayI fChannelsToPadY; // Array which stores the corresponding y position of PAD of entire module A


  ClassDef(CbmMuchUnpackPar,1);
};
#endif
