// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSMonitorMuch                            -----
// -----                 Created 11.11.2016 by V. Singhal and A. Kumar     -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmTSMonitorMuch_H
#define CbmTSMonitorMuch_H

#ifndef __CINT__
  #include "Timeslice.hpp"
  #include "rocMess_wGet4v1.h"
#endif

#include "CbmTSUnpack.h"
#include "CbmHistManager.h"

#include <vector>
#include <map>

class CbmDigi;
class CbmMuchUnpackPar; // Unpaker Class which will read the MuchUnpackPar.par

class CbmTSMonitorMuch: public CbmTSUnpack
{
public:
  
  CbmTSMonitorMuch();
  virtual ~CbmTSMonitorMuch();
    
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

    void SetRunStart( Int_t dateIn, Int_t timeIn, Int_t iBinSize = 5 );

    void ResetAllHistos();

private:
   Int_t fNrOfNdpbs;           // Total number of NDPBs in the system
   Int_t fNrOfNdpbsA;           // Total number of NDPBs for GEM A
   Int_t fNrOfNdpbsB;           // Total number of NDPBs for GEM B
   Int_t fNrOfFebsPerNdpb;     // Number of FEBs per NDPB
   
  std::vector<int> fMsgCounter;
   std::map<UInt_t, UInt_t> fNdpbIdIndexMap;

  Int_t fMuchStationMapX[16][128]; //fMuchStationMap for map of row number via getX
  Int_t fMuchStationMapY[16][128]; // For map of column number via getY>

  CbmHistManager* fHM;  ///< Histogram manager

  std::map<Int_t, std::map<Int_t, UInt_t> > fCurrentEpoch; // Current epoch (first epoch in the stream initialises the map item)
  Int_t fNofEpochs; /** Current epoch marker for each ROC **/
  ULong_t fCurrentEpochTime;     /** Time stamp of current epoch **/
  Double_t fdStartTime; /** Time of first valid hit (epoch available), used as reference for evolution plots**/
  Double_t fdStartTimeMsSz; /** Time of first microslice, used as reference for evolution plots**/
  TCanvas* fcMsSizeAll;

  Int_t fEquipmentId;

  CbmMuchUnpackPar* fUnpackPar;    //!
    
  std::vector<TH1*> fFebRateDate_nDPB;
  std::vector<TH2*> fHitDtDate_nDPB;
  std::vector<Double_t> fdLastHitTime_nDPB;
  Int_t             fiRunStartDateTimeSec;
  Int_t             fiBinSizeDatePlots;

  void CreateHistograms();

#ifndef __CINT__
  void FillHitInfo(ngdpb::Message, std::vector<TH1*>, std::vector<TH2*>,
		  std::vector<TH1*>, TH2*);
  void FillEpochInfo(ngdpb::Message, std::vector<TH1*>);
#endif

  Int_t CreateAddress(Int_t rocId, Int_t febId, Int_t stationId, Int_t layerId, Int_t sideId,
		  Int_t moduleId, Int_t channelId, TH2*);

  CbmTSMonitorMuch(const CbmTSMonitorMuch&);
  CbmTSMonitorMuch operator=(const CbmTSMonitorMuch&);

  ClassDef(CbmTSMonitorMuch, 1)
};

#endif
