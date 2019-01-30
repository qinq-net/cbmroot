// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmMcbm2018UnpackerTaskSts                   -----
// -----               Created 14.11.2018 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmMcbm2018UnpackerTaskSts_H
#define CbmMcbm2018UnpackerTaskSts_H

#include "Timeslice.hpp"

#include "CbmMcbmUnpack.h"

class CbmMcbm2018UnpackerAlgoSts;
class CbmDigi;
class CbmTbDaqBuffer;
class TClonesArray;

class CbmMcbm2018UnpackerTaskSts : public CbmMcbmUnpack
{
   public:

      CbmMcbm2018UnpackerTaskSts( UInt_t uNbSdpb = 1 );
      virtual ~CbmMcbm2018UnpackerTaskSts();

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

   private:
      /// Control flags
      Bool_t fbMonitorMode;  //! Switch ON the filling of a minimal set of histograms
      Bool_t fbDebugMonitorMode; //! Switch ON the filling of a additional set of histograms

      /// Parameters management
      TList* fParCList;

      /// Statistics & first TS rejection
      uint64_t fulTsCounter;

      /// Time Based buffer
      CbmTbDaqBuffer* fBuffer;

      /// Output ClonesArray
      TClonesArray* fStsDigiCloneArray;

      /// Processing algo
      CbmMcbm2018UnpackerAlgoSts * fUnpackerAlgo;

      CbmMcbm2018UnpackerTaskSts(const CbmMcbm2018UnpackerTaskSts&);
      CbmMcbm2018UnpackerTaskSts operator=(const CbmMcbm2018UnpackerTaskSts&);

      ClassDef(CbmMcbm2018UnpackerTaskSts, 1)
};

#endif
