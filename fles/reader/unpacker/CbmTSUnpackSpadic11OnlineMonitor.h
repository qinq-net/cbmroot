// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmTSUnpackSpadic11OnlineMonitor                       -----
// -----                    Created 07.11.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMTSUNPACKSPADIC11ONLINEMONITOR_H
#define CBMTSUNPACKSPADIC11ONLINEMONITOR_H

#ifndef __CINT__
  #include "Timeslice.hpp"
  #include "Message.hpp"
#endif


#include "CbmTSUnpack.h"
#include "CbmBeamDefaults.h"

#include "CbmHistManager.h"
#include "TClonesArray.h"

class CbmTSUnpackSpadic11OnlineMonitor : public CbmTSUnpack
{
 public:
  
  CbmTSUnpackSpadic11OnlineMonitor();
  virtual ~CbmTSUnpackSpadic11OnlineMonitor();
    
  virtual Bool_t Init();
#ifndef __CINT__
  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
  void print_message(const spadic::Message& m);
#endif
  virtual void Reset();

  virtual void Finish();
  virtual void FinishEvent();
  void SetParContainers() {;}

  // protected:
  //  virtual void Register();

  virtual void FillOutput(CbmDigi*){;}
 private:

  TClonesArray* fSpadicRaw;

  Int_t fEpochMarkerArray[NrOfSyscores][NrOfHalfSpadics];
  //Array to store the previous Epoch counter
  Int_t fPreviousEpochMarkerArray[NrOfSyscores][NrOfHalfSpadics];
  //Suppress multiple Epoch Messages in duplicated Microslices. NOTE:Currently Buggy
  const Bool_t SuppressMultipliedEpochMessages = false;
  Int_t fSuperEpochArray[NrOfSyscores][NrOfHalfSpadics];
  
  static Int_t   GetSpadicID(Int_t address);
  static Int_t   GetSyscoreID(Int_t link);
  inline TString GetSpadicName(Int_t link,Int_t address);
  Int_t GetChannelOnPadPlane(Int_t SpadicChannel, Int_t groupId);
  Int_t fEpochMarker;
  Int_t fSuperEpoch;
  Int_t fNrExtraneousSamples;
  void GetEpochInfo(Int_t link, Int_t addr);
  void FillEpochInfo(Int_t link, Int_t addr, Int_t epoch_count);
  CbmHistManager* fHM;
  TCanvas* fcB;
  TCanvas* fcM;//[(iLink)*(NrOfHalfSpadics/2)];
  TCanvas* fcH;
  TCanvas* fcL;
  TCanvas* fcE;
  TCanvas* fcO;
  TCanvas* fcS;
  TCanvas* fcI;
  TCanvas* fcTS;
  void InitHistos();
  void InitCanvas();
  void UpdateCanvas();
  TH2I* fBaseline[(NrOfSyscores)*(NrOfSpadics)];
  TH2I* fmaxADCmaxTimeBin[(NrOfSyscores)*(NrOfSpadics)];
  TH2I* fHit[(NrOfSyscores)*(NrOfSpadics)];
  TH2I* fLost[(NrOfSyscores)*(NrOfSpadics)];
  TH2I* fEpoch[(NrOfSyscores)*(NrOfSpadics)];
  TH2I* fOutOfSync[(NrOfSyscores)*(NrOfSpadics)];
  TH2I* fStrange[(NrOfSyscores)*(NrOfSpadics)];
  TH2I* fInfo[(NrOfSyscores)*(NrOfSpadics)];
  TH1I* fHitTime[(NrOfSyscores)*(NrOfSpadics)];
  TGraph* fTSGraph[(NrOfSyscores)*(NrOfSpadics)];
  Int_t fLastSuperEpoch[(NrOfSyscores)*(NrOfSpadics)];
  //Int_t fSumHitMessages[(NrOfSyscores)*(NrOfSpadics)];
  CbmTSUnpackSpadic11OnlineMonitor(const CbmTSUnpackSpadic11OnlineMonitor&);
  CbmTSUnpackSpadic11OnlineMonitor operator=(const CbmTSUnpackSpadic11OnlineMonitor&);

  ClassDef(CbmTSUnpackSpadic11OnlineMonitor, 2)
    };

#endif

