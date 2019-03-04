// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmMcbm2018UnpackerTaskTof                    -----
// -----               Created 10.02.2019 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmMcbm2018UnpackerTaskTof_H
#define CbmMcbm2018UnpackerTaskTof_H

#include "Timeslice.hpp"

#include "CbmMcbmUnpack.h"

class CbmMcbm2018UnpackerAlgoTof;
class CbmDigi;
class CbmTbDaqBuffer;
class TClonesArray;

class CbmMcbm2018UnpackerTaskTof : public CbmMcbmUnpack
{
   public:

      CbmMcbm2018UnpackerTaskTof( UInt_t uNbSdpb = 1 );
      virtual ~CbmMcbm2018UnpackerTaskTof();

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
      void SetTimeOffsetNs( Double_t dOffsetIn = 0.0 );
      void SetDiamondDpbIdx( UInt_t uIdx = 2 );

      /// Task settings
      void SetSeparateArrayT0( Bool_t bFlagIn = kTRUE ) { fbSeparateArrayT0 = bFlagIn; }
      void SetDigiMaskAndIdT0( UInt_t uMask, UInt_t uMaskedIdT0 )
            { fuDigiMaskId = uMask; uMaskedIdT0 = fuDigiMaskedIdT0; }

      void UseDaqBuffer(Bool_t val) {fUseDaqBuffer = val;}
   private:
      /// Control flags
      Bool_t fbMonitorMode;  //! Switch ON the filling of a minimal set of histograms
      Bool_t fbDebugMonitorMode; //! Switch ON the filling of a additional set of histograms
      Bool_t fbSeparateArrayT0; //! If ON, T0 digis are saved in separate TClonesArray
      Bool_t fUseDaqBuffer; //! Switch OFF the usage of the DaqBuffer

      /// Parameters
      UInt_t fuDigiMaskedIdT0;
      UInt_t fuDigiMaskId;

      /// Parameters management
      TList* fParCList;

      /// Statistics & first TS rejection
      uint64_t fulTsCounter;

      /// Time Based buffer
      CbmTbDaqBuffer* fBuffer;

      /// Output ClonesArray
      TClonesArray* fTofDigiCloneArray;
      TClonesArray* fT0DigiCloneArray;

      /// Processing algo
      CbmMcbm2018UnpackerAlgoTof * fUnpackerAlgo;

      CbmMcbm2018UnpackerTaskTof(const CbmMcbm2018UnpackerTaskTof&);
      CbmMcbm2018UnpackerTaskTof operator=(const CbmMcbm2018UnpackerTaskTof&);

      ClassDef(CbmMcbm2018UnpackerTaskTof, 1)
};

#endif
