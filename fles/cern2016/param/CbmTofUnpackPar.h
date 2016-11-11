// -------------------------------------------------------------------------
// -----                 CbmFHodoUnpackPar header file                 -----
// -----                 Created 02/11/16  by F. Uhlig                 -----
// -------------------------------------------------------------------------

#ifndef CBMTOFUNPACKPAR_H
#define CBMTOFUNPACKPAR_H

#include "FairParGenericSet.h"

#include "TArrayI.h"

class FairParIo;
class FairParamList;


class CbmTofUnpackPar : public FairParGenericSet 
{

 public:

  /** Standard constructor **/
  CbmTofUnpackPar(const char* name      = "CbmTofUnpackPar",
		    const char* title   = "Tof unpacker parameters",
		    const char* context = "Default");


  /** Destructor **/
  virtual ~CbmTofUnpackPar();
  
  /** Reset all parameters **/
  virtual void clear();

  void putParams(FairParamList*);
  Bool_t getParams(FairParamList*);

  inline Int_t GetNrOfRocs() { return fNrOfRocs; }
  inline Int_t GetRocId(Int_t i) { return fRocIdArray[i]; }
  inline Int_t GetNumberOfChannels() {return fNrOfChannels;}
  inline Int_t GetChannelToDetUIdMap(Int_t channel) {return fChannelToDetUIdMap[channel];}
  inline Bool_t IsChannelRateEnabled() {return (1 == fPlotChannelRate);}

 private:

  Int_t   fNrOfRocs; // Total number of Rocs
  TArrayI fRocIdArray; // Array to hold the unique IDs for all Tof Rocs

  Int_t   fNrOfChannels;
  TArrayI fChannelToDetUIdMap;// Array which stores the corresponding UIDs for each channel
  
  Int_t   fPlotChannelRate;

  ClassDef(CbmTofUnpackPar,1);
};
#endif
