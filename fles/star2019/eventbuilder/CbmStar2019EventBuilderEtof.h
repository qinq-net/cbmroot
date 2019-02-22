// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                     CbmStar2019EventBuilderEtof                   -----
// -----               Created 14.11.2018 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmStar2019EventBuilderEtof_H
#define CbmStar2019EventBuilderEtof_H

#include "Timeslice.hpp"

#include "CbmMcbmUnpack.h"
#include "TStopwatch.h"

class CbmStar2019EventBuilderEtofAlgo;

/*
 ** Function to send sub-event block to the STAR DAQ system
 *       trg_word received is packed as:
 *
 *       trg_cmd|daq_cmd|tkn_hi|tkn_mid|tkn_lo
*/
extern "C" int star_rhicf_write(unsigned int trg_word, void *dta, int bytes);

class CbmDigi;

class CbmStar2019EventBuilderEtof : public CbmMcbmUnpack
{
   public:

      CbmStar2019EventBuilderEtof( UInt_t uNbGdpb = 1 );
      virtual ~CbmStar2019EventBuilderEtof();

      virtual Bool_t Init();
      virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
      virtual void Reset();

      virtual void Finish();

      void SetParContainers();

      Bool_t InitContainers();

      Bool_t ReInitContainers();

      void FillOutput(CbmDigi* digi);

      void SetSandboxMode( Bool_t bSandboxMode = kTRUE ){ fbSandboxMode = bSandboxMode; }
      void SetEventDumpEnable( Bool_t bDumpEna = kTRUE );

      /// Temp until we change from CbmMcbmUnpack to something else
      void AddMsComponentToList( size_t component, UShort_t usDetectorId );
      void SetNbMsInTs( size_t uCoreMsNb, size_t uOverlapMsNb ){};

      void UseDaqBuffer(Bool_t) {};
   private:
      /// Control flags
      Bool_t fbMonitorMode;  //! Switch ON the filling of a minimal set of histograms
      Bool_t fbDebugMonitorMode; //! Switch ON the filling of a additional set of histograms
      Bool_t fbSandboxMode;  //! Switch OFF the emission of data toward the STAR DAQ
      Bool_t fbEventDumpEna; //! Switch ON the dumping of the events to a binary file

      /// Parameters management
      TList* fParCList;

      /// Statistics & first TS rejection
      uint64_t fulTsCounter;

      /// Processing algo
      CbmStar2019EventBuilderEtofAlgo * fEventBuilderAlgo;

      TStopwatch fTimer;

      /// Event dump to binary file
      std::fstream * fpBinDumpFile;
      const UInt_t   kuBinDumpBegWord = 0xFEEDBEAF;
      const UInt_t   kuBinDumpEndWord = 0xFAEBDEEF;

      CbmStar2019EventBuilderEtof(const CbmStar2019EventBuilderEtof&);
      CbmStar2019EventBuilderEtof operator=(const CbmStar2019EventBuilderEtof&);

      ClassDef(CbmStar2019EventBuilderEtof, 1)
};

#endif
