// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSUnpackMuch                            -----
// -----                 Created 11.11.2016 by V. Singhal and A. Kumar     -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMTSUNPACKMUCH_H
#define CBMTSUNPACKMUCH_H

#ifndef __CINT__
  #include "Timeslice.hpp"
  #include "rocMess_wGet4v1.h"
#endif

#include "CbmTSUnpack.h"
#include "CbmHistManager.h"
#include "CbmNxyterRawMessage.h"
//#include "CbmFiberHodoDigi.h"
//#include "CbmSpsMuchDigi.h" //Created a new Much Digi File for the SPS beamtime2016

#include "/home/pmd/daqsoft/cbmroot_trunk/data/much/CbmSpsMuchDigi.h" //Created a new Much Digi File for the SPS beamtime2016
#include "CbmTbDaqBuffer.h"

#include "TClonesArray.h"

#include <vector>
#include <map>

class CbmDigi;
//class CbmFHodoUnpackPar;
class CbmMuchUnpackPar; // Unpaker Class which will read the MuchUnpackPar.par
//class CbmSpsMuchDigi;
class CbmTSUnpackMuch: public CbmTSUnpack
{
public:
  
  CbmTSUnpackMuch();
  virtual ~CbmTSUnpackMuch();
    
  virtual Bool_t Init();
#ifndef __CINT__
  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
#endif
  virtual void Reset();

  virtual void Finish();

  void SetParContainers();

  Bool_t InitContainers();

  Bool_t ReInitContainers();

  void FillOutput(CbmDigi* digi);

  void CreateRawMessageOutput(Bool_t val) {fCreateRawMessage=val;}

  Int_t CreateAddress(Int_t rocId, Int_t febId, Int_t stationId, Int_t layerId, Int_t sideId, Int_t moduleId, Int_t channelId);

private:

  std::vector<int> fMsgCounter;

  Int_t fMuchStationMapX[16][128]; //fMuchStationMap for map of row number via getX
  Int_t fMuchStationMapY[16][128]; // For map of column number via getY>
  
//  Int_t fHodoFiber[128];  /** Mapping from fiber hodoscope feb channel to fiber number **/
//  Int_t fHodoPlane[128];  /** Mapping from fiber hodoscope feb channel to plane number 1=X, 2=Y **/
//  Int_t fHodoPixel[128];  /** Mapping from fiber hodoscope feb channel to pixel number **/

  CbmHistManager* fHM;  ///< Histogram manager

  std::map<Int_t, UInt_t> fCurrentEpoch; // Current epoch (first epoch in the stream initialises the map item)
  Int_t fNofEpochs; /** Current epoch marker for each ROC **/
  ULong_t fCurrentEpochTime;     /** Time stamp of current epoch **/

  Int_t fEquipmentId;

  TClonesArray* fMuchRaw;
  TClonesArray* fMuchDigi;
  CbmNxyterRawMessage* fRawMessage;  
  //CbmSpsMuchDigi* fDigi;  //If CbmSpsMuchDigi will not work then will use this.
  CbmSpsMuchDigi* fDigi;  
  CbmTbDaqBuffer* fBuffer;

  Bool_t fCreateRawMessage;

  CbmMuchUnpackPar* fUnpackPar;    //!

  void CreateHistograms();

#ifndef __CINT__
  void FillHitInfo(ngdpb::Message);
  void FillEpochInfo(ngdpb::Message);
#endif

  CbmTSUnpackMuch(const CbmTSUnpackMuch&);
  CbmTSUnpackMuch operator=(const CbmTSUnpackMuch&);

  ClassDef(CbmTSUnpackMuch, 1)
};

#endif
