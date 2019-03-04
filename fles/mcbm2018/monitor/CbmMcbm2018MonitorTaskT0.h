// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmMcbm2018MonitorTaskT0                      -----
// -----               Created 10.02.2019 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmMcbm2018MonitorTaskT0_H
#define CbmMcbm2018MonitorTaskT0_H

#include "Timeslice.hpp"

#include "CbmMcbmUnpack.h"

class CbmMcbm2018MonitorAlgoT0;
class CbmDigi;
class CbmTbDaqBuffer;
class TClonesArray;

class CbmMcbm2018MonitorTaskT0 : public CbmMcbmUnpack
{
   public:

      CbmMcbm2018MonitorTaskT0();
      virtual ~CbmMcbm2018MonitorTaskT0();

      virtual Bool_t Init();
      virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
      virtual void Reset();

      virtual void Finish();

      void SetParContainers();

      Bool_t InitContainers();

      Bool_t ReInitContainers();

      void FillOutput(CbmDigi* digi);

      /// Temp until we change from CbmMcbmUnpack to something else
      void AddMsComponentToList( size_t component, UShort_t usDetectorId );
      void SetNbMsInTs( size_t uCoreMsNb, size_t uOverlapMsNb ){};

      /// Algo settings setters
      inline void SetMonitorMode( Bool_t bFlagIn = kTRUE ) { fbMonitorMode = bFlagIn; }
      void SetIgnoreOverlapMs( Bool_t bFlagIn = kTRUE );
      inline void SetHistoryHistoSize( UInt_t inHistorySizeSec = 1800 ) { fuHistoryHistoSize = inHistorySizeSec; }

      /// Task settings
      void UseDaqBuffer(Bool_t val) {}
   private:
      /// Control flags
      Bool_t fbMonitorMode;  //! Switch ON the filling of a minimal set of histograms
      Bool_t fbDebugMonitorMode; //! Switch ON the filling of a additional set of histograms

      /// Parameters
      UInt_t fuHistoryHistoSize;

      /// Parameters management
      TList* fParCList;

      /// Statistics & first TS rejection
      uint64_t fulTsCounter;

      /// Processing algo
      CbmMcbm2018MonitorAlgoT0 * fMonitorAlgo;

      CbmMcbm2018MonitorTaskT0(const CbmMcbm2018MonitorTaskT0&);
      CbmMcbm2018MonitorTaskT0 operator=(const CbmMcbm2018MonitorTaskT0&);

      ClassDef(CbmMcbm2018MonitorTaskT0, 1)
};

#endif
