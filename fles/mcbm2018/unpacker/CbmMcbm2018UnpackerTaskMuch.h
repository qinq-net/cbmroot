// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmMcbm2018UnpackerTaskMuch                   -----
// -----               Created 02.02.2019 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmMcbm2018UnpackerTaskMuch_H
#define CbmMcbm2018UnpackerTaskMuch_H

#include "Timeslice.hpp"

#include "CbmMcbmUnpack.h"

class CbmMcbm2018UnpackerAlgoMuch;
class CbmDigi;
class CbmTbDaqBuffer;
class TClonesArray;

class CbmMcbm2018UnpackerTaskMuch : public CbmMcbmUnpack
{
   public:

      CbmMcbm2018UnpackerTaskMuch( UInt_t uNbSdpb = 1 );
      virtual ~CbmMcbm2018UnpackerTaskMuch();

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

      inline void SetMonitorMode( Bool_t bFlagIn = kTRUE ) { fbMonitorMode = bFlagIn; }
      void SetIgnoreOverlapMs( Bool_t bFlagIn = kTRUE );

      void SetTimeOffsetNs( Double_t dOffsetIn = 0.0 );

      void UseDaqBuffer (Bool_t val) {fUseDaqBuffer = val;}
   private:
      /// Control flags
      Bool_t fbMonitorMode;  //! Switch ON the filling of a minimal set of histograms
      Bool_t fbDebugMonitorMode; //! Switch ON the filling of a additional set of histograms
      Bool_t fUseDaqBuffer; //! Switch OFF the usage of the DaqBuffer

      /// Parameters management
      TList* fParCList;

      /// Statistics & first TS rejection
      uint64_t fulTsCounter;

      /// Time Based buffer
      CbmTbDaqBuffer* fBuffer;

      /// Output ClonesArray
      TClonesArray* fMuchDigiCloneArray;

      /// Processing algo
      CbmMcbm2018UnpackerAlgoMuch * fUnpackerAlgo;

      CbmMcbm2018UnpackerTaskMuch(const CbmMcbm2018UnpackerTaskMuch&);
      CbmMcbm2018UnpackerTaskMuch operator=(const CbmMcbm2018UnpackerTaskMuch&);

      ClassDef(CbmMcbm2018UnpackerTaskMuch, 2)
};

#endif
