// -----------------------------------------------------------------------------
// ----- TTrbUnpackTof source file                                         -----
// -----                                                                   -----
// ----- created by C. Simon on 2014-03-08                                 -----
// -----                                                                   -----
// ----- based on TMbsUnpackTof by P.-A. Loizeau                           -----
// ----- https://subversion.gsi.de/fairroot/cbmroot/development/ploizeau/  -----
// -----   main/unpack/tof/TMbsUnpackTof.cxx                               -----
// ----- revision 21787, 2013-09-20                                        -----
// -----------------------------------------------------------------------------

#include "TTrbUnpackTof.h"

// Parameter header
#include "TMbsUnpackTofPar.h"
#include "TofDef.h"
#include "TofTrbTdcDef.h"
#include "TTrbHeader.h"

// Iterator
#include "HadaqTrbIterator.h"

// Subunpacker header
#include "TTofTrbTdcUnpacker.h"

// Output object
#include "TTofTrbTdcBoard.h"

// ROOT headers
#include "TClonesArray.h"
#include "TROOT.h"
#include "TH1.h"
#include "TMath.h"

// FairRoot headers
#include "FairRunOnline.h"
#include "FairLogger.h"
#include "FairRuntimeDb.h"

ClassImp(TTrbUnpackTof)

TTrbUnpackTof::TTrbUnpackTof( Short_t type, Short_t subType, Short_t procId, Short_t subCrate, Short_t control) : 
   FairUnpack( type, subType, procId, subCrate, control),
   fMbsUnpackPar(0),
   fiNbEvents(0),
   fiCurrentEventNumber(0),
   fiPreviousEventNumber(0),
   fbFineSpillTiming(kTRUE),
   fTrbIterator(NULL),
   fTrbTdcUnpacker(NULL),
   fTrbTdcBoardCollection(NULL),
   fbSaveRawTdcBoards(kFALSE),
   fTdcUnpackMap(),
   fuInDataTrbSebNb(0),
   fuActiveTrbTdcNb(0),
   fTrbTriggerPattern(NULL),
   fTrbTriggerType(NULL),
   fCtsBusyTime(NULL),
   fCtsIdleTime(NULL),
   fCtsIdleTimeSpill(NULL),
   fItcAssertions(NULL),
   fItcEvents(NULL),
   fHadaqEventTime(NULL),
   fTrbEventNumberJump(NULL),
   fCtsTriggerCycles(NULL),
   fCtsTriggerAccepted(NULL),
   fHadaqTimeInSpill(NULL),
   fCtsTimeInSpill(NULL),
   fTrbHeader(NULL),
   fTrbSubeventSize(),
   fTrbSubeventStatus(),
   fTrbTdcWords(),
   fTrbTdcProcessStatus(),
   fuTrigChanEdgeCounter(new UInt_t[16]),
   fuTrigChanClockCounter(new UInt_t[16]),
   fuTrigInputEdgeCounter(new UInt_t[16]),
   fuTrigInputClockCounter(new UInt_t[16]),
   fiAllPossibleTriggers(0),
   fiAcceptedTriggers(0),
   fiHadaqLastEventTime(0),
   fiHadaqFirstEventTime(0),
   fiHadaqSpillStartTime(0),
   fiHadaqCoarseSpillStartTime(0),
   fiCtsLastEventTime(0),
   fiCtsFirstEventTime(0),
   fbNextSpillToStart(kFALSE),
   fbHadaqBufferDelay(kFALSE)
{
   LOG(INFO)<<"**** TTrbUnpackTof: Call TTrbUnpackTof()..."<<FairLogger::endl;
}
      
TTrbUnpackTof::~TTrbUnpackTof()
{
   LOG(INFO)<<"**** TTrbUnpackTof: Call ~TTrbUnpackTof()..."<<FairLogger::endl;

   LOG(INFO)<<"**** TTrbUnpackTof: Delete subunpacker objects..."<<FairLogger::endl;

   if( 0 < fuActiveTrbTdcNb )
      delete fTrbTdcUnpacker;

   fTdcUnpackMap.clear();

   delete[] fuTrigChanEdgeCounter;
   delete[] fuTrigChanClockCounter;
   delete[] fuTrigInputEdgeCounter;
   delete[] fuTrigInputClockCounter;

//   DeleteHistograms();
}

Bool_t TTrbUnpackTof::Init()
{
   LOG(INFO)<<"**** TTrbUnpackTof: Call Init()..."<<FairLogger::endl;

   if( kFALSE == InitParameters() )
   {
      LOG(ERROR)<<"**** TTrbUnpackTof: Failed to initialize parameters!"<<FairLogger::endl;
      return kFALSE;
   }
   LOG(INFO)<<"**** TTrbUnpackTof: Succeeded to initialize parameters."<<FairLogger::endl;
   
   if( kFALSE == RegisterOutput() )
   {
      LOG(ERROR)<<"**** TTrbUnpackTof: Failed to register output objects with FairRootManager!"<<FairLogger::endl;
      return kFALSE;
   }
   LOG(INFO)<<"**** TTrbUnpackTof: Succeeded to register output objects with FairRootManager."<<FairLogger::endl;

   if( kFALSE == CreateSubunpackers() )
   {
      LOG(ERROR)<<"**** TTrbUnpackTof: Failed to create subunpacker objects!"<<FairLogger::endl;
      return kFALSE;
   }
   LOG(INFO)<<"**** TTrbUnpackTof: Succeeded to create subunpacker objects."<<FairLogger::endl;

   CreateHistograms();

   return kTRUE;
}

Bool_t TTrbUnpackTof::DoUnpack(Int_t* data, Int_t size)
{
   LOG(DEBUG)<<"**** TTrbUnpackTof: Call DoUnpack()... "<<FairLogger::endl;

   Int_t  * pData   = data;
//   UInt_t  uNb4ByteWords = size/2 - 1;
// Changed in git commit 0c0bd037c201d3496f9d5d7c133874382e885677 to fairroot LMD source
// TODO: Make sure the same change is applied to FairMbsStreamSource !!!!
   UInt_t  uNb4ByteWords = size;
   UInt_t  uNb1ByteWords = uNb4ByteWords*4;

   LOG(DEBUG)<<FairLogger::endl;
   LOG(DEBUG)<<"Unpacking TRB-type MBS subevent..."<<FairLogger::endl;
   LOG(DEBUG)<<FairLogger::endl;
   LOG(DEBUG)<<"====================================="<<FairLogger::endl;
   LOG(DEBUG)<<"=     MBS subevent                  ="<<FairLogger::endl;
   LOG(DEBUG)<<"====================================="<<FairLogger::endl;
   LOG(DEBUG)<<"size:     "<<uNb1ByteWords<<" B"<<FairLogger::endl;
   LOG(DEBUG)<<"type:     "<<GetType()<<FairLogger::endl;
   LOG(DEBUG)<<"subtype:  "<<GetSubType()<<FairLogger::endl;
   LOG(DEBUG)<<"procid:   "<<GetProcId()<<FairLogger::endl;
   LOG(DEBUG)<<"subcrate: "<<GetSubCrate()<<FairLogger::endl;
   LOG(DEBUG)<<"control:  "<<GetControl()<<FairLogger::endl;
   LOG(DEBUG)<<"====================================="<<FairLogger::endl;
   LOG(DEBUG)<<FairLogger::endl;

   fTrbIterator = new hadaq::TrbIterator(pData, uNb1ByteWords);

   LOG(DEBUG)<<"Getting single HADAQ raw event in MBS subevent..."<<FairLogger::endl;
   hadaq::RawEvent* tCurrentEvent = fTrbIterator->nextEvent();

   LOG(DEBUG2)<<"First   word in memory: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent))<<FairLogger::endl;
   LOG(DEBUG2)<<"Second  word in memory: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent+1))<<FairLogger::endl;
   LOG(DEBUG2)<<"Third   word in memory: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent+2))<<FairLogger::endl;
   LOG(DEBUG2)<<"Fourth  word in memory: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent+3))<<FairLogger::endl;
   LOG(DEBUG2)<<"Fifth   word in memory: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent+4))<<FairLogger::endl;
   LOG(DEBUG2)<<"Sixth   word in memory: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent+5))<<FairLogger::endl;
   LOG(DEBUG2)<<"Seventh word in memory: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent+6))<<FairLogger::endl;
   LOG(DEBUG2)<<"Eighth  word in memory: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent+7))<<FairLogger::endl;

   LOG(DEBUG2)<<"First   word in HADAQ event: "<<Form("0x%.8x",tCurrentEvent->tuSize)<<FairLogger::endl;
   LOG(DEBUG2)<<"Second  word in HADAQ event: "<<Form("0x%.8x",tCurrentEvent->tuDecoding)<<FairLogger::endl;
   LOG(DEBUG2)<<"Third   word in HADAQ event: "<<Form("0x%.8x",tCurrentEvent->tuId)<<FairLogger::endl;
   LOG(DEBUG2)<<"Fourth  word in HADAQ event: "<<Form("0x%.8x",tCurrentEvent->evtSeqNr)<<FairLogger::endl;
   LOG(DEBUG2)<<"Fifth   word in HADAQ event: "<<Form("0x%.8x",tCurrentEvent->evtDate)<<FairLogger::endl;
   LOG(DEBUG2)<<"Sixth   word in HADAQ event: "<<Form("0x%.8x",tCurrentEvent->evtTime)<<FairLogger::endl;
   LOG(DEBUG2)<<"Seventh word in HADAQ event: "<<Form("0x%.8x",tCurrentEvent->evtRunNr)<<FairLogger::endl;
   LOG(DEBUG2)<<"Eighth  word in HADAQ event: "<<Form("0x%.8x",tCurrentEvent->evtPad)<<FairLogger::endl;

   if( 0 == tCurrentEvent )
   {
     LOG(ERROR)<<"Bad HADAQ raw event. Skip whole MBS subevent."<<FairLogger::endl;
     return kFALSE;   
   }

   if( 32 == uNb1ByteWords )
   {
     LOG(ERROR)<<"Empty HADAQ raw event. Skip it."<<FairLogger::endl;
     LOG(ERROR)<<"event number: "<<tCurrentEvent->GetSeqNr()<<FairLogger::endl;
     return kFALSE;
   }

   if( tCurrentEvent->GetDataError() )
   {
     LOG(WARNING)<<"Error bit set in at least one HADAQ raw subevent!"<<FairLogger::endl;
   }

   UInt_t uTriggerPattern = 0;
   UInt_t uTriggerType    = tCurrentEvent->GetTrigType();
   fTrbTdcUnpacker->SetCalibTrigger(uTriggerType);

   LOG(DEBUG)<<FairLogger::endl;
   LOG(DEBUG)<<"====================================="<<FairLogger::endl;
   LOG(DEBUG)<<"=     HADAQ raw event               ="<<FairLogger::endl;
   LOG(DEBUG)<<"====================================="<<FairLogger::endl;
   LOG(DEBUG)<<"size:         "<<tCurrentEvent->GetSize()<<" B"<<FairLogger::endl;
   LOG(DEBUG)<<"decoding:     "<<Form("0x%.8x",tCurrentEvent->GetDecoding())<<FairLogger::endl;
   LOG(DEBUG)<<"id:           "<<Form("0x%.8x",tCurrentEvent->GetId())<<FairLogger::endl;
   LOG(DEBUG)<<"event number: "<<tCurrentEvent->GetSeqNr()<<FairLogger::endl;
   LOG(DEBUG)<<"trigger type: "<<Form("0x%.1x",uTriggerType)<<FairLogger::endl;
   LOG(DEBUG)<<"date:         "<<Form("%.4u-%.2u-%.2u (yyyy-mm-dd)",1900+(tCurrentEvent->GetDate()>>16),
                                                                    1+((tCurrentEvent->GetDate()&0xff00)>>8),
                                                                    ((tCurrentEvent->GetDate()&0xff))
                                     )<<FairLogger::endl;
   LOG(DEBUG)<<"time:         "<<Form("%.2u:%.2u:%.2u UTC (hh:mm:ss)",(tCurrentEvent->GetTime()>>16),
                                                                      ((tCurrentEvent->GetTime()&0xff00)>>8),
                                                                      ((tCurrentEvent->GetTime()&0xff))
                                     )<<FairLogger::endl;
   LOG(DEBUG)<<"file number:  "<<tCurrentEvent->GetRunNr()<<FairLogger::endl;
   LOG(DEBUG)<<"padding:      "<<tCurrentEvent->GetPaddedSize()-tCurrentEvent->GetSize()
                               <<" B"<<FairLogger::endl;
   LOG(DEBUG)<<"====================================="<<FairLogger::endl;
   LOG(DEBUG)<<FairLogger::endl;

   if( uNb1ByteWords != tCurrentEvent->GetPaddedSize() )
   {
//     LOG(WARNING)<<Form("HADAQ raw event of padded size %u does not fully occupy MBS Subevent data field of size %u!",
     LOG(WARNING)<<Form("HADAQ raw event of padded size %u does not fully occupy MBS Subevent data field of size %u! Ratio: %f, size %d",
                        tCurrentEvent->GetPaddedSize(), uNb1ByteWords,
                        (Float_t)(tCurrentEvent->GetPaddedSize())/(Float_t)uNb1ByteWords, size )
                 <<FairLogger::endl;
   }

   Int_t iHadaqCurrentEventTime = (tCurrentEvent->GetTime()&0xff)
                                 +60*((tCurrentEvent->GetTime()&0xff00)>>8)
                                 +3600*(tCurrentEvent->GetTime()>>16);

   // time offset of the first valid event in a run
   if( 0 == fiNbEvents )
   {
     fiHadaqFirstEventTime = iHadaqCurrentEventTime;
   }

   // allow for up to 24h of data taking per run (1 overflow from 23:59:59 to 00:00:00)
   if( 0 > (iHadaqCurrentEventTime - fiHadaqLastEventTime) )
   {
     iHadaqCurrentEventTime += 86400;
   }

   fHadaqEventTime->Fill(iHadaqCurrentEventTime - fiHadaqFirstEventTime);

   // start of a new HADAQ spill: at least 2 seconds time difference to the preceding event
   if( 1 < (iHadaqCurrentEventTime - fiHadaqLastEventTime) )
   {
     fiHadaqCoarseSpillStartTime = iHadaqCurrentEventTime;
   }

   fTrbHeader->SetTimeInSpill(((UInt_t)(iHadaqCurrentEventTime-fiHadaqCoarseSpillStartTime))*100000000);

   fHadaqTimeInSpill->Fill(fTrbHeader->GetTimeInSpill()/100000000);

   hadaq::RawSubevent* tCurrentSubevent;
   UInt_t uNbSubevents = 0;
   UInt_t uNbGoodSubevents = 0;

   while( 0 != fTrbIterator->nextSubevent() )
   {
     LOG(DEBUG)<<"Getting HADAQ raw subevent in HADAQ raw event..."<<FairLogger::endl;

     tCurrentSubevent = fTrbIterator->currSubevent();
     UInt_t uSubeventId = tCurrentSubevent->GetId() & 0xffff;

     uNbSubevents++;

     Bool_t bKnownSubevent = ( -1 < fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) );
     UInt_t uNbRegisteredFpgas = 0;
     UInt_t uNbActiveFpgas = 0;

     if( bKnownSubevent )
     {
       uNbRegisteredFpgas = fMbsUnpackPar->GetInDataFpgaNbPerTrbSeb( uSubeventId );
       uNbActiveFpgas = fMbsUnpackPar->GetActiveTdcNbPerTrbSep( uSubeventId );

       fTrbSubeventSize[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill( tCurrentSubevent->GetSize()/1000 );
     }

     DataErrorHandling(uSubeventId, tCurrentSubevent->GetErrBits());

     if( tCurrentSubevent->GetDataError() || (tCurrentSubevent->GetErrBits() != 0x00000001) )
     {
       LOG(ERROR)<<Form("Skipping HADAQ raw subevent from source 0x%.4x. Contains broken data!", uSubeventId)<<FairLogger::endl;
       continue;
     }

     uNbGoodSubevents++;


     LOG(DEBUG)<<FairLogger::endl;
     LOG(DEBUG)<<"====================================="<<FairLogger::endl;
     if( bKnownSubevent )
     {
       LOG(DEBUG)<<Form("=     HADAQ raw subevent 0x%.4x     =",uSubeventId)<<FairLogger::endl;
     }
     else
     {
       LOG(DEBUG)<<Form("=     Unknown subevent 0x%.4x       =",uSubeventId)<<FairLogger::endl;
     }
     LOG(DEBUG)<<"====================================="<<FairLogger::endl;
     LOG(DEBUG)<<"size:                "<<tCurrentSubevent->GetSize()<<" B"<<FairLogger::endl;
     LOG(DEBUG)<<"decoding:            "<<Form("0x%.8x",tCurrentSubevent->GetDecoding())<<FairLogger::endl;
     LOG(DEBUG)<<"CTS trigger counter: "<<Form("0x%.6x",tCurrentSubevent->GetTrigNr()>>8)<<FairLogger::endl;
     LOG(DEBUG)<<"trigger type:        "<<Form("0x%.2x",tCurrentSubevent->GetTrigType())<<FairLogger::endl;
     LOG(DEBUG)<<"next trigger tag:    "<<Form("0x%.2x",tCurrentSubevent->GetTrigNr()&0xff)<<FairLogger::endl;
     LOG(DEBUG)<<"padding:             "<<tCurrentSubevent->GetPaddedSize()-tCurrentSubevent->GetSize()
                                        <<" B"<<FairLogger::endl;
     LOG(DEBUG)<<"====================================="<<FairLogger::endl;
     if( bKnownSubevent )
     {
       LOG(DEBUG)<<Form("FPGAs registered: %u", uNbRegisteredFpgas)<<FairLogger::endl;
       LOG(DEBUG)<<Form("TDCs read out   : %u", uNbActiveFpgas)<<FairLogger::endl;
       LOG(DEBUG)<<"====================================="<<FairLogger::endl;
     }
     LOG(DEBUG)<<FairLogger::endl;

     LOG(DEBUG)<<"Getting HADAQ raw subsubevents in HADAQ raw subevent..."<<FairLogger::endl;

     UInt_t uNbSubsubevents = 0;

     UInt_t uNbSubsubeventDataWords = tCurrentSubevent->GetNrOfDataWords();

     if( 0 == uNbSubsubeventDataWords )
     {
       LOG(WARNING)<<Form("No data in HADAQ raw subevent 0x%.4x.. Skip!", uSubeventId)<<FairLogger::endl;
       continue;
     }

     if( bKnownSubevent && ( 0xc000 == uSubeventId ) )
     {
       fiCurrentEventNumber = (tCurrentSubevent->GetTrigNr()) >> 8;
       if( 0 < fiNbEvents )
       {
         fTrbEventNumberJump->Fill(fiCurrentEventNumber - fiPreviousEventNumber);
       }
     }

     UInt_t uSubsubeventDataIndex = 0;
     UInt_t uSubsubeventData = 0;
     UInt_t uSubsubeventSource = 0xffff;

     while ( uSubsubeventSource != 0x5555 )
     {
       uSubsubeventData = tCurrentSubevent->Data(uSubsubeventDataIndex);
       uSubsubeventSource = uSubsubeventData & 0xffff;

       UInt_t uSubsubeventLength = (uSubsubeventData >> 16) & 0xffff;
       Bool_t bKnownSubsubevent = fMbsUnpackPar->IsTrbFpgaInData(uSubsubeventSource);

       UInt_t uMotherBoardId = 0xffff;
       Bool_t bUnpack = kFALSE;

       if( bKnownSubsubevent )
       {
         uMotherBoardId = fMbsUnpackPar->GetTrbSebAddrForFpga(uSubsubeventSource);
         bUnpack = ( -1 < fMbsUnpackPar->GetActiveTrbTdcIndex(uSubsubeventSource) );
       }

       LOG(DEBUG)<<FairLogger::endl;

       if( 0x0 == (uSubsubeventSource>>12) )
       {
         uNbSubsubevents++;

         LOG(DEBUG)<<"====================================="<<FairLogger::endl;
         LOG(DEBUG)<<Form("=  HADAQ raw TDC subsubevent 0x%.4x =",uSubsubeventSource)<<FairLogger::endl;
         LOG(DEBUG)<<"====================================="<<FairLogger::endl;
         LOG(DEBUG)<<"size:        "<<Form("%u B",(uSubsubeventLength+1)*4)<<FairLogger::endl;
         LOG(DEBUG)<<"data words:  "<<Form("%u",uSubsubeventLength)<<FairLogger::endl;
         LOG(DEBUG)<<"trigger tag: "<<Form("0x%.2x",(tCurrentSubevent->Data(uSubsubeventDataIndex+1)>>16)&0xff)<<FairLogger::endl;
         LOG(DEBUG)<<"====================================="<<FairLogger::endl;
         if( bKnownSubsubevent && bKnownSubevent)
         {
           LOG(DEBUG)<<Form("registered FPGA on TRB 0x%.4x", uMotherBoardId)<<FairLogger::endl;
           LOG(DEBUG)<<"====================================="<<FairLogger::endl;
           LOG(DEBUG)<<FairLogger::endl;

           if(gLogger->IsLogNeeded(DEBUG2))
           {
             for(UInt_t uWord = uSubsubeventDataIndex; uWord <= uSubsubeventDataIndex+uSubsubeventLength; uWord++)
             {
               LOG(DEBUG2)<<Form("0x%.8x",tCurrentSubevent->Data(uWord))<<FairLogger::endl;
             }

             LOG(DEBUG2)<<FairLogger::endl;
           }

           if( uSubeventId != uMotherBoardId )
           {
             LOG(FATAL)<<Form("Severe error in parameter file. FPGA 0x%.4x is the data transmitter of FPGA 0x%.4x, not FPGA 0x%.4x. Abort program execution!",
                              uSubeventId, uSubsubeventSource, uMotherBoardId)<<FairLogger::endl;
           }
           if( bUnpack )
           {
        	 Int_t iActiveTdcIndex = fMbsUnpackPar->GetActiveTrbTdcIndex( uSubsubeventSource );
        	 fTdcUnpackMap[iActiveTdcIndex] = std::pair<hadaq::RawSubevent*,UInt_t>(tCurrentSubevent, uSubsubeventDataIndex);
        	 fTrbTdcWords[iActiveTdcIndex]->Fill(uSubsubeventLength);

//             fTrbTdcProcessStatus[ iActiveTdcIndex ]->Fill( fTrbTdcUnpacker->ProcessData( tCurrentSubevent, uSubsubeventDataIndex ) );
           }
         }
         else
         {
           LOG(DEBUG)<<Form("unregistered FPGA")<<FairLogger::endl;
           LOG(DEBUG)<<"====================================="<<FairLogger::endl;
           LOG(DEBUG)<<FairLogger::endl;

           if(gLogger->IsLogNeeded(DEBUG2))
           {
             for(UInt_t uWord = uSubsubeventDataIndex; uWord <= uSubsubeventDataIndex+uSubsubeventLength; uWord++)
             {
               LOG(DEBUG2)<<Form("0x%.8x",tCurrentSubevent->Data(uWord))<<FairLogger::endl;
             }

             LOG(DEBUG2)<<FairLogger::endl;
           }

         }
       }

       else if( 0xb == (uSubsubeventSource>>12) )
       {
         uNbSubsubevents++;

         LOG(DEBUG)<<"====================================="<<FairLogger::endl;
         LOG(DEBUG)<<Form("=  HADAQ raw Hub subsubevent 0x%.4x =",uSubsubeventSource)<<FairLogger::endl;
         LOG(DEBUG)<<"====================================="<<FairLogger::endl;
         LOG(DEBUG)<<"size:        "<<Form("%u B",(uSubsubeventLength+1)*4)<<FairLogger::endl;
         LOG(DEBUG)<<"data words:  "<<Form("%u",uSubsubeventLength)<<FairLogger::endl;
         LOG(DEBUG)<<"====================================="<<FairLogger::endl;
         if( bKnownSubsubevent && bKnownSubevent)
         {
           LOG(DEBUG)<<Form("registered FPGA on TRB 0x%.4x", uMotherBoardId)<<FairLogger::endl;
           LOG(DEBUG)<<"====================================="<<FairLogger::endl;
           LOG(DEBUG)<<FairLogger::endl;
           if( uSubeventId != uMotherBoardId )
           {
             LOG(FATAL)<<Form("Severe error in parameter file. FPGA 0x%.4x is the data transmitter of FPGA 0x%.4x, not FPGA 0x%.4x. Abort program execution!",
                              uSubeventId, uSubsubeventSource, uMotherBoardId)<<FairLogger::endl;
           }
         }
         else
         {
           LOG(DEBUG)<<Form("unregistered FPGA")<<FairLogger::endl;
           LOG(DEBUG)<<"====================================="<<FairLogger::endl;
           LOG(DEBUG)<<FairLogger::endl;
         }

         LOG(DEBUG2)<<Form("0x%.8x",tCurrentSubevent->Data(uSubsubeventDataIndex))<<FairLogger::endl;

         UInt_t uHubSubeventDataIndex = uSubsubeventDataIndex+1;
         UInt_t uHubEventLength = uSubsubeventLength;

         while ( uHubEventLength )
         {
           UInt_t uHubSubeventData = tCurrentSubevent->Data(uHubSubeventDataIndex);
           UInt_t uHubSubeventSource = uHubSubeventData & 0xffff;
           UInt_t uHubSubeventLength = (uHubSubeventData >> 16) & 0xffff;

           Bool_t bKnownHubSubevent = fMbsUnpackPar->IsTrbFpgaInData(uHubSubeventSource);

           if( bKnownHubSubevent && bKnownSubsubevent )
           {
             uMotherBoardId = fMbsUnpackPar->GetTrbSebAddrForFpga(uHubSubeventSource);
             bUnpack = ( -1 < fMbsUnpackPar->GetActiveTrbTdcIndex(uHubSubeventSource) );
           }

           LOG(DEBUG)<<FairLogger::endl;

           if( 0x0 == (uHubSubeventSource>>12) )
           {
             uNbSubsubevents++;
    
             LOG(DEBUG)<<"====================================="<<FairLogger::endl;
             LOG(DEBUG)<<Form("=  HADAQ raw TDC hubsubevent 0x%.4x =",uHubSubeventSource)<<FairLogger::endl;
             LOG(DEBUG)<<"====================================="<<FairLogger::endl;
             LOG(DEBUG)<<"size:        "<<Form("%u B",(uHubSubeventLength+1)*4)<<FairLogger::endl;
             LOG(DEBUG)<<"data words:  "<<Form("%u",uHubSubeventLength)<<FairLogger::endl;
             LOG(DEBUG)<<"trigger tag: "<<Form("0x%.2x",(tCurrentSubevent->Data(uHubSubeventDataIndex+1)>>16)&0xff)<<FairLogger::endl;
             LOG(DEBUG)<<"====================================="<<FairLogger::endl;
             if( bKnownHubSubevent && bKnownSubsubevent )
             {
               LOG(DEBUG)<<Form("registered FPGA on TRB 0x%.4x", uMotherBoardId)<<FairLogger::endl;
               LOG(DEBUG)<<"====================================="<<FairLogger::endl;
               LOG(DEBUG)<<FairLogger::endl;

               if(gLogger->IsLogNeeded(DEBUG2))
               {
                 for(UInt_t uWord = uHubSubeventDataIndex; uWord <= uHubSubeventDataIndex+uHubSubeventLength; uWord++)
                 {
                   LOG(DEBUG2)<<Form("0x%.8x",tCurrentSubevent->Data(uWord))<<FairLogger::endl;
                 }

                 LOG(DEBUG2)<<FairLogger::endl;
               }

               if( uSubeventId != uMotherBoardId )
               {
                 LOG(FATAL)<<Form("Severe error in parameter file. FPGA 0x%.4x is the data transmitter of FPGA 0x%.4x, not FPGA 0x%.4x. Abort program execution!",
                                  uSubeventId, uHubSubeventSource, uMotherBoardId)<<FairLogger::endl;
               }
               if( bUnpack )
               {
              	 Int_t iActiveTdcIndex = fMbsUnpackPar->GetActiveTrbTdcIndex( uHubSubeventSource );
              	 fTdcUnpackMap[iActiveTdcIndex] = std::pair<hadaq::RawSubevent*,UInt_t>(tCurrentSubevent, uHubSubeventDataIndex);
              	 fTrbTdcWords[iActiveTdcIndex]->Fill(uHubSubeventLength);

//                 fTrbTdcProcessStatus[ iActiveTdcIndex ]->Fill( fTrbTdcUnpacker->ProcessData( tCurrentSubevent, uHubSubeventDataIndex ) );
               }
             }
             else
             {
               LOG(DEBUG)<<Form("unregistered FPGA")<<FairLogger::endl;
               LOG(DEBUG)<<"====================================="<<FairLogger::endl;
               LOG(DEBUG)<<FairLogger::endl;

               if(gLogger->IsLogNeeded(DEBUG2))
               {
                 for(UInt_t uWord = uHubSubeventDataIndex; uWord <= uHubSubeventDataIndex+uHubSubeventLength; uWord++)
                 {
                   LOG(DEBUG2)<<Form("0x%.8x",tCurrentSubevent->Data(uWord))<<FairLogger::endl;
                 }

                 LOG(DEBUG2)<<FairLogger::endl;
               }

             }
 
           }
           else if( 0x8 == (uHubSubeventSource>>12) )
           {
             uNbSubsubevents++;
    
             LOG(DEBUG)<<"====================================="<<FairLogger::endl;
             LOG(DEBUG)<<Form("=  HADAQ raw Hub hubsubevent 0x%.4x =",uHubSubeventSource)<<FairLogger::endl;
             LOG(DEBUG)<<"====================================="<<FairLogger::endl;
             LOG(DEBUG)<<"size:        "<<Form("%u B",(uHubSubeventLength+1)*4)<<FairLogger::endl;
             LOG(DEBUG)<<"data words:  "<<Form("%u",uHubSubeventLength)<<FairLogger::endl;
             LOG(DEBUG)<<"====================================="<<FairLogger::endl;
             if( bKnownHubSubevent && bKnownSubsubevent )
             {
               LOG(DEBUG)<<Form("registered central hub on slave TRB")<<FairLogger::endl;
               LOG(DEBUG)<<"====================================="<<FairLogger::endl;
               LOG(DEBUG)<<FairLogger::endl;

               if(gLogger->IsLogNeeded(DEBUG2))
               {
                 for(UInt_t uWord = uHubSubeventDataIndex; uWord <= uHubSubeventDataIndex+uHubSubeventLength; uWord++)
                 {
                   LOG(DEBUG2)<<Form("0x%.8x",tCurrentSubevent->Data(uWord))<<FairLogger::endl;
                 }
               }

               if( uSubeventId != uMotherBoardId )
               {
                 LOG(FATAL)<<Form("Severe error in parameter file. FPGA 0x%.4x is the data transmitter of FPGA 0x%.4x, not FPGA 0x%.4x. Abort program execution!",
                                  uSubeventId, uHubSubeventSource, uMotherBoardId)<<FairLogger::endl;
               }
             }
             else
             {
               LOG(DEBUG)<<Form("unregistered central hub on slave TRB")<<FairLogger::endl;
               LOG(DEBUG)<<"====================================="<<FairLogger::endl;
               LOG(DEBUG)<<FairLogger::endl;

               if(gLogger->IsLogNeeded(DEBUG2))
               {
                 for(UInt_t uWord = uHubSubeventDataIndex; uWord <= uHubSubeventDataIndex+uHubSubeventLength; uWord++)
                 {
                   LOG(DEBUG2)<<Form("0x%.8x",tCurrentSubevent->Data(uWord))<<FairLogger::endl;
                 }
               }
             }
  
           }
           else
           {
             uNbSubsubevents++;

             LOG(WARNING)<<"====================================="<<FairLogger::endl;
             LOG(WARNING)<<Form("=  Unknown hubsubevent type 0x%.4x  =",uHubSubeventSource)<<FairLogger::endl;
             LOG(WARNING)<<"====================================="<<FairLogger::endl;
             LOG(DEBUG)<<"size:       "<<Form("%u B",(uHubSubeventLength+1)*4)<<FairLogger::endl;
             LOG(DEBUG)<<"data words: "<<Form("%u",uHubSubeventLength)<<FairLogger::endl;
             LOG(DEBUG)<<"====================================="<<FairLogger::endl;

             if(gLogger->IsLogNeeded(DEBUG2))
             {
               for(UInt_t uWord = uHubSubeventDataIndex; uWord <= uHubSubeventDataIndex+uHubSubeventLength; uWord++)
               {
                 LOG(DEBUG2)<<Form("0x%.8x",tCurrentSubevent->Data(uWord))<<FairLogger::endl;
               }
             }

           }

           uHubSubeventDataIndex += uHubSubeventLength+1;
           uHubEventLength -= uHubSubeventLength+1;

         }   
       }

       else if( 0xc == (uSubsubeventSource>>12) )
       {
         uNbSubsubevents++;

         UInt_t uTrigStatus = tCurrentSubevent->Data(uSubsubeventDataIndex+1) & 0xffff;
         uTriggerPattern = uTrigStatus;

         if( !fMbsUnpackPar->IsTrbEventUnpacked(uTriggerPattern) )
         {
           LOG(DEBUG)<<"HADAQ raw event does not meet the trigger selection criteria. Skip it."<<FairLogger::endl;
           return kFALSE;
         }

         for(UInt_t uChannel = 0; uChannel < 16; uChannel++)
         {
           if( uTriggerPattern & (0x1 << uChannel) )
           {
             fTrbTriggerPattern->Fill(TMath::Log2( uTriggerPattern & (0x1 << uChannel) ));
           }
         }

         fTrbTriggerType->Fill(uTriggerType);

	       fTrbHeader->SetTriggerPattern(uTriggerPattern);
	       fTrbHeader->SetTriggerType(uTriggerType);
	       fTrbHeader->SetTimeInSpill((UInt_t)1);  // FIXME, TBD

         UInt_t uNbInputCh = (tCurrentSubevent->Data(uSubsubeventDataIndex+1) >> 16) & 0xf;
         UInt_t uNbTrigCh = (tCurrentSubevent->Data(uSubsubeventDataIndex+1) >> 20) & 0x1f;
         Bool_t bIncludeLastIdle = (tCurrentSubevent->Data(uSubsubeventDataIndex+1) >> 25) & 0x1;
         Bool_t bIncludeCounters = (tCurrentSubevent->Data(uSubsubeventDataIndex+1) >> 26) & 0x1;
         Bool_t bIncludeTimestamp = (tCurrentSubevent->Data(uSubsubeventDataIndex+1) >> 27) & 0x1;
         UInt_t uExtTrigFlag = (tCurrentSubevent->Data(uSubsubeventDataIndex+1) >> 28) & 0x3;

         Bool_t bIncludeMbsSync = kFALSE;
         UInt_t uMbsSync = 0xffffff;
         Bool_t bMbsSyncError = kFALSE;

         UInt_t uNbCtsWords = uNbInputCh*2
                             +uNbTrigCh*2
                             +bIncludeLastIdle*2
                             +bIncludeCounters*3
                             +bIncludeTimestamp*1;

         UInt_t uBusyIdleOffset = uSubsubeventDataIndex + 2 + uNbInputCh*2 + uNbTrigCh*2;
         UInt_t uStatsOffset = uBusyIdleOffset + bIncludeLastIdle*2;
         UInt_t uTimeStampOffset = uStatsOffset + bIncludeCounters*3;


         if( bIncludeLastIdle )
         {
           // time to complete the TrbNet trigger process in the last event
           fCtsBusyTime->Fill( tCurrentSubevent->Data(uBusyIdleOffset+1)/100. );

           UInt_t uIdleTime = tCurrentSubevent->Data(uBusyIdleOffset);

           // time passed since the last trigger procession
           fCtsIdleTime->Fill( uIdleTime/100. );

           if( 1. < uIdleTime/100000000. )
           {
             // time passed since the last trigger procession on inter-spill scales
             fCtsIdleTimeSpill->Fill( uIdleTime/100000000. );

             LOG(INFO)<<Form("CTS idle time of %.2f s observed in event %u.",uIdleTime/100000000.,fiNbEvents)<<FairLogger::endl;
           }
         }

         Long64_t iAllTriggersDiff(0);
         Long64_t iAllEventsDiff(0);

         if( bIncludeCounters )
         {
           Long64_t iAllTriggersCounter = (Long64_t)tCurrentSubevent->Data(uStatsOffset+1);
           Long64_t iAllEventsCounter = (Long64_t)tCurrentSubevent->Data(uStatsOffset+2);

           iAllTriggersDiff = iAllTriggersCounter - fiAllPossibleTriggers;
           if( 0 > iAllTriggersDiff )
           {
             iAllTriggersDiff += 4294967296;
           }

           iAllEventsDiff = iAllEventsCounter - fiAcceptedTriggers;
           if( 0 > iAllEventsDiff )
           {
             iAllEventsDiff += 4294967296;
           }

           LOG(DEBUG)<<Form("In total, %lld trigger opportunities (10 ns sampling) have been counted since the last event.", iAllTriggersDiff)<<FairLogger::endl;
           LOG(DEBUG)<<Form("In total, %lld events have been triggered since the last event.", iAllEventsDiff)<<FairLogger::endl;
           LOG(DEBUG)<<FairLogger::endl;

           fItcAssertions->Fill(iAllTriggersDiff);
           fItcEvents->Fill(iAllEventsDiff);

           fiAllPossibleTriggers = iAllTriggersCounter;
           fiAcceptedTriggers = iAllEventsCounter;
         }

         if( bIncludeTimestamp )
         {
           Long64_t iCurrentTimeStamp = (Long64_t)tCurrentSubevent->Data(uTimeStampOffset);

/* #############################################################################
// works reliably only for nov15 data
// more robust than the version covering both feb15 and nov15

           if( fbNextSpillToStart || (0 == fiNbEvents) )
           {
             fiCtsFirstEventTime = iCurrentTimeStamp;
             fiCtsLastEventTime = 0;
             fiHadaqSpillStartTime = iHadaqCurrentEventTime - fiHadaqFirstEventTime;
             iAllTriggersDiff = 0;
             iAllEventsDiff = 0;

             fbNextSpillToStart = kFALSE;
           }

           if( 10 < (iHadaqCurrentEventTime - fiHadaqLastEventTime) )
           {
             fbNextSpillToStart = kTRUE;
           }
  ########################################################################### */
// seems to work for both feb15 and nov15 data
// more assumptions necessary, less robust

           if( 0 == fiNbEvents )
           {
             fiCtsFirstEventTime = iCurrentTimeStamp;
             fiCtsLastEventTime = 0;
             fiHadaqSpillStartTime = iHadaqCurrentEventTime - fiHadaqFirstEventTime;
             iAllTriggersDiff = 0;
             iAllEventsDiff = 0;

             // seemingly a HADAQ buffer delay can already exist at the beginning of a run!
             fbHadaqBufferDelay = kTRUE;
           }
           else
           {
             // start of a new HADAQ spill: at least 2 seconds time difference to the preceding one
             if( 1 < (iHadaqCurrentEventTime - fiHadaqLastEventTime) )
             {
               // no HADAQ buffer delay has been observed
               if( !fbHadaqBufferDelay )
               {
                 // compare the current CTS time to the value in the previous event
                 Long64_t iTimeInSpill = iCurrentTimeStamp - fiCtsFirstEventTime;

                 if( 0 > iTimeInSpill )
                 {
                   iTimeInSpill += 4294967296;
                 }

                 // event most probably belongs to the preceding spill
                 if( 1000*100000 > (iTimeInSpill-fiCtsLastEventTime) )
                 {
                   fbHadaqBufferDelay = kTRUE;
                 }
                 // event most probably starts a new spill
                 else
                 {
                   fiCtsFirstEventTime = iCurrentTimeStamp;
                   fiCtsLastEventTime = 0;
                   fiHadaqSpillStartTime = iHadaqCurrentEventTime - fiHadaqFirstEventTime;
                   iAllTriggersDiff = 0;
                   iAllEventsDiff = 0;
                 }
               }
               // a HADAQ buffer delay is propagating
               else
               {
                 fiCtsFirstEventTime = iCurrentTimeStamp;
                 fiCtsLastEventTime = 0;
                 fiHadaqSpillStartTime = fiHadaqLastEventTime - fiHadaqFirstEventTime;
                 iAllTriggersDiff = 0;
                 iAllEventsDiff = 0;
               }
             }
             // somewhere inside a HADAQ spill
             else
             {
               // first event following a series of delayed HADAQ buffers
               if( fbHadaqBufferDelay )
               {
                 fiCtsFirstEventTime = iCurrentTimeStamp;
                 fiCtsLastEventTime = 0;
                 // correct spill start time w.r.t. the HADAQ buffer delay
                 fiHadaqSpillStartTime = fiHadaqLastEventTime - fiHadaqFirstEventTime;
                 iAllTriggersDiff = 0;
                 iAllEventsDiff = 0;

                 // HADAQ buffer delay propagation should stop here
                 fbHadaqBufferDelay = kFALSE;
               }
             }
           }
// #############################################################################

           Long64_t iTimeInSpill = iCurrentTimeStamp - fiCtsFirstEventTime;

           if( 0 > iTimeInSpill )
           {
             iTimeInSpill += 4294967296;
           }

           if( bIncludeCounters )
           {
             if( 0 == fiCtsLastEventTime )
             {
               LOG(DEBUG)<<Form("time to origin: %f. event: %u",fiHadaqSpillStartTime*1. + (Double_t)(fiCtsLastEventTime + iTimeInSpill)/2./100000000.,fiNbEvents)<<FairLogger::endl;
               LOG(DEBUG)<<Form("timestamp value: %lld",iCurrentTimeStamp)<<FairLogger::endl;
               LOG(DEBUG)<<Form("idle time: %u",tCurrentSubevent->Data(uBusyIdleOffset))<<FairLogger::endl; 
             }

             fCtsTriggerCycles->Fill(fiHadaqSpillStartTime*1. + (Double_t)(fiCtsLastEventTime + iTimeInSpill)/2./100000000., iAllTriggersDiff);
             fCtsTriggerAccepted->Fill(fiHadaqSpillStartTime*1. + (Double_t)(fiCtsLastEventTime + iTimeInSpill)/2./100000000., iAllEventsDiff);
           }

           if(fbFineSpillTiming)
           {
             fTrbHeader->SetTimeInSpill((UInt_t)iTimeInSpill);
           }
           
           fCtsTimeInSpill->Fill((Double_t)iTimeInSpill/100000.);

           fiCtsLastEventTime = iTimeInSpill;
         }


         /* Uncomment the following block of code to display [DEBUG2] the difference of assertion cycle and rising edge
            counters of the trigger input modules (IM) and the iternal trigger channels (ITC) compared to the previous event.
            As of 2016-01-22, most counter values written into the CTS subsubevent are not meaningful due to a firmware bug.
            In the CTS GUI, the counter values are correct (polled via the slow-control channel of TrbNet).
          */

/*
         UInt_t uTrigInpOffset = uSubsubeventDataIndex + 2;
         UInt_t uTrigCntOffset = uTrigInpOffset + uNbInputCh*2;

         UInt_t uTrigInpEdgeDiffsSum(0);
         UInt_t uTrigInpClockDiffsSum(0);

         for( UInt_t uInput = 0; uInput < uNbInputCh; uInput++ )
         {

           UInt_t uTrigInpClockDiff = tCurrentSubevent->Data( uTrigInpOffset + 2*uInput ) - fuTrigInputClockCounter[uInput];
           uTrigInpClockDiffsSum += uTrigInpClockDiff;
           fuTrigInputClockCounter[uInput] = tCurrentSubevent->Data( uTrigInpOffset + 2*uInput );

           LOG(DEBUG2)<<Form("Input Module %u: %u clock cycles since the last event.",uInput,uTrigInpClockDiff)<<FairLogger::endl;
           
         }

         for( UInt_t uInput = 0; uInput < uNbInputCh; uInput++ )
         {
           UInt_t uTrigInpEdgeDiff = tCurrentSubevent->Data( uTrigInpOffset + 1 + 2*uInput ) - fuTrigInputEdgeCounter[uInput];
           uTrigInpEdgeDiffsSum += uTrigInpEdgeDiff;
           fuTrigInputEdgeCounter[uInput] = tCurrentSubevent->Data( uTrigInpOffset + 1 + 2*uInput );

           LOG(DEBUG2)<<Form("Input Module %u: %u rising edges since the last event.",uInput,uTrigInpEdgeDiff)<<FairLogger::endl;

         }

         UInt_t uTrigChanEdgeDiffsSum(0);
         UInt_t uTrigChanClockDiffsSum(0);

         for( UInt_t uChannel = 0; uChannel < uNbTrigCh; uChannel++ )
         {

           UInt_t uTrigChanClockDiff = tCurrentSubevent->Data( uTrigCntOffset + 2*uChannel ) - fuTrigChanClockCounter[uChannel];
           uTrigChanClockDiffsSum += uTrigChanClockDiff;
           fuTrigChanClockCounter[uChannel] = tCurrentSubevent->Data( uTrigCntOffset + 2*uChannel );

           LOG(DEBUG2)<<Form("ITC %u: %u clock cycles since the last event.",uChannel,uTrigChanClockDiff)<<FairLogger::endl;
           
         }

         for( UInt_t uChannel = 0; uChannel < uNbTrigCh; uChannel++ )
         {
           UInt_t uTrigChanEdgeDiff = tCurrentSubevent->Data( uTrigCntOffset + 1 + 2*uChannel ) - fuTrigChanEdgeCounter[uChannel];
           uTrigChanEdgeDiffsSum += uTrigChanEdgeDiff;
           fuTrigChanEdgeCounter[uChannel] = tCurrentSubevent->Data( uTrigCntOffset + 1 + 2*uChannel );

           LOG(DEBUG2)<<Form("ITC %u: %u rising edges since the last event.",uChannel,uTrigChanEdgeDiff)<<FairLogger::endl;

         }

*/

         if( 0x1 == uExtTrigFlag )
         {
           bIncludeMbsSync = kTRUE;
           uMbsSync = tCurrentSubevent->Data(uSubsubeventDataIndex+2+uNbCtsWords) & 0xffffff;
           bMbsSyncError = tCurrentSubevent->Data(uSubsubeventDataIndex+2+uNbCtsWords) & 0x80000000;           
         }

         LOG(DEBUG)<<"====================================="<<FairLogger::endl;
         LOG(DEBUG)<<Form("=  HADAQ raw CTS subsubevent 0x%.4x =",uSubsubeventSource)<<FairLogger::endl;
         LOG(DEBUG)<<"====================================="<<FairLogger::endl;
         LOG(DEBUG)<<"size:           "<<Form("%u B",(uSubsubeventLength+1)*4)<<FairLogger::endl;
         LOG(DEBUG)<<"data words:     "<<Form("%u",uSubsubeventLength)<<FairLogger::endl;
         LOG(DEBUG)<<"ITC status:     "<<Form("%.4x",uTrigStatus)<<FairLogger::endl;
         LOG(DEBUG)<<"input channels: "<<Form("%u",uNbInputCh)<<FairLogger::endl;
         LOG(DEBUG)<<"ITC counters:   "<<Form("%u",uNbTrigCh)<<FairLogger::endl;
         if( bIncludeMbsSync )
         {
           LOG(DEBUG)<<"CBM/MBS sync:   "<<Form("%.6x",uMbsSync)<<FairLogger::endl;
           if( bMbsSyncError )
           {
             LOG(DEBUG)<<"sync error!!!   "<<FairLogger::endl;
           }
         }
         LOG(DEBUG)<<"====================================="<<FairLogger::endl;
         if( bKnownSubsubevent && bKnownSubevent )
         {
           LOG(DEBUG)<<Form("registered FPGA on TRB 0x%.4x", uMotherBoardId)<<FairLogger::endl;
           LOG(DEBUG)<<"====================================="<<FairLogger::endl;
           LOG(DEBUG)<<FairLogger::endl;

           if(gLogger->IsLogNeeded(DEBUG2))
           {
             for(UInt_t uWord = uSubsubeventDataIndex; uWord <= uSubsubeventDataIndex+uSubsubeventLength; uWord++)
             {
               LOG(DEBUG2)<<Form("0x%.8x",tCurrentSubevent->Data(uWord))<<FairLogger::endl;
             }
           }

           if( uSubeventId != uMotherBoardId )
           {
             LOG(FATAL)<<Form("Severe error in parameter file. FPGA 0x%.4x is the data transmitter of FPGA 0x%.4x, not FPGA 0x%.4x. Abort program execution!",
                              uSubeventId, uSubsubeventSource, uMotherBoardId)<<FairLogger::endl;
           }
         }
         else
         {
           LOG(DEBUG)<<Form("unregistered FPGA")<<FairLogger::endl;
           LOG(DEBUG)<<"====================================="<<FairLogger::endl;
           LOG(DEBUG)<<FairLogger::endl;

           if(gLogger->IsLogNeeded(DEBUG2))
           {
             for(UInt_t uWord = uSubsubeventDataIndex; uWord <= uSubsubeventDataIndex+uSubsubeventLength; uWord++)
             {
               LOG(DEBUG2)<<Form("0x%.8x",tCurrentSubevent->Data(uWord))<<FairLogger::endl;
             }
           }

         }

       }

       else if(0x5555 == uSubsubeventSource)
       {
         LOG(DEBUG)<<"====================================="<<FairLogger::endl;
         LOG(DEBUG)<<Form("=  HADAQ raw END subsubevent 0x%.4x =",uSubsubeventSource)<<FairLogger::endl;
         LOG(DEBUG)<<"====================================="<<FairLogger::endl;
         LOG(DEBUG)<<"size:       "<<Form("%u B",(uSubsubeventLength+1)*4)<<FairLogger::endl;
         LOG(DEBUG)<<"data words: "<<Form("%u",uSubsubeventLength)<<FairLogger::endl;
         LOG(DEBUG)<<"====================================="<<FairLogger::endl;
       }

       else
       {
         uNbSubsubevents++;

         LOG(DEBUG)<<"====================================="<<FairLogger::endl;
         LOG(DEBUG)<<Form("=  Unknown subsubevent type 0x%.4x  =",uSubsubeventSource)<<FairLogger::endl;
         LOG(DEBUG)<<"====================================="<<FairLogger::endl;
         LOG(DEBUG)<<"size:       "<<Form("%u B",(uSubsubeventLength+1)*4)<<FairLogger::endl;
         LOG(DEBUG)<<"data words: "<<Form("%u",uSubsubeventLength)<<FairLogger::endl;
         LOG(DEBUG)<<"====================================="<<FairLogger::endl;

         if(gLogger->IsLogNeeded(DEBUG2))
         {
           for(UInt_t uWord = uSubsubeventDataIndex; uWord <= uSubsubeventDataIndex+uSubsubeventLength; uWord++)
           {
             LOG(DEBUG2)<<Form("0x%.8x",tCurrentSubevent->Data(uWord))<<FairLogger::endl;
           }
         }

       }

       uSubsubeventDataIndex += uSubsubeventLength+1;
       uNbSubsubeventDataWords -= uSubsubeventLength+1;
     }

     LOG(DEBUG)<<FairLogger::endl;
     LOG(DEBUG)<<Form("%u subsubevents identified in subevent 0x%.4x.", uNbSubsubevents, uSubeventId)<<FairLogger::endl;
     if( bKnownSubevent )
     {
       LOG(DEBUG)<<Form("%u subsubevents expected according to the parameter file.", uNbRegisteredFpgas)<<FairLogger::endl;
     }

     LOG(DEBUG)<<FairLogger::endl;

   }

   // TDC data unpacking only in case of a positive trigger pattern evaluation
	 for ( std::map<Int_t,std::pair<hadaq::RawSubevent*,UInt_t> >::iterator it = fTdcUnpackMap.begin();
	       it != fTdcUnpackMap.end();
	       ++it)
   {
     fTrbTdcProcessStatus[ it->first ]->Fill( fTrbTdcUnpacker->ProcessData( (it->second).first, (it->second).second ) );
	 }

   LOG(DEBUG)<<FairLogger::endl;

   LOG(DEBUG)<<Form("Total number of subevents in HADAQ raw event: %u", uNbSubevents)<<FairLogger::endl;
   LOG(DEBUG)<<Form("The parameter file specifies %u active TRB subevent builders.", fMbsUnpackPar->GetActiveTrbSebNb())<<FairLogger::endl;
   LOG(DEBUG)<<FairLogger::endl;
   LOG(DEBUG)<<FairLogger::endl;
   
   fiNbEvents++;
   fiPreviousEventNumber = fiCurrentEventNumber;
   
   fiHadaqLastEventTime = iHadaqCurrentEventTime;

   delete fTrbIterator;

   return kTRUE;
}

void TTrbUnpackTof::Reset()
{
   LOG(DEBUG)<<"**** TTrbUnpackTof: Call Reset()... "<<FairLogger::endl;

   ClearOutput();
}

void TTrbUnpackTof::Register()
{
   LOG(INFO)<<"**** TTrbUnpackTof: Call Register()..."<<FairLogger::endl;
}

Bool_t TTrbUnpackTof::InitParameters()
{
   LOG(INFO)<<"**** TTrbUnpackTof: Call InitParameters()..."<<FairLogger::endl;

   FairRun * ana = FairRun::Instance();
   FairRuntimeDb * rtdb = ana->GetRuntimeDb();

   fMbsUnpackPar = (TMbsUnpackTofPar *) (rtdb->getContainer("TMbsUnpackTofPar"));

   if( 0 == fMbsUnpackPar )
      return kFALSE;

   fMbsUnpackPar->printParams();

   fuInDataTrbSebNb = fMbsUnpackPar->GetActiveTrbSebNb();
   fuActiveTrbTdcNb = fMbsUnpackPar->GetNbActiveBoards( tofMbs::trbtdc );

   return kTRUE;
}

Bool_t TTrbUnpackTof::CreateSubunpackers()
{
   LOG(INFO)<<"**** TTrbUnpackTof: Call CreateSubunpackers()..."<<FairLogger::endl;

   if( 0 == fuActiveTrbTdcNb )
   {
      LOG(ERROR)<<"**** TTrbUnpackTof: No active FPGA-TDCs!"<<FairLogger::endl;
      return kFALSE;
   }
   else
   {
     fTrbTdcUnpacker = new TTofTrbTdcUnpacker( fMbsUnpackPar );
   }

   return kTRUE;
}

Bool_t TTrbUnpackTof::RegisterOutput()
{
   LOG(INFO)<<"**** TTrbUnpackTof: Call RegisterOutput()..."<<FairLogger::endl;

   if( 0 == fuActiveTrbTdcNb )
   {
      LOG(ERROR)<<"**** TTrbUnpackTof: No active FPGA-TDCs!"<<FairLogger::endl;
      return kFALSE;
   }

   // PAL:
   // Not sure here what is the best way to group the TDC objects......
   // The vector of data in the board objects kill probably the TClonesArray purpose?
   // Maybe better to have big fixed size array and TTofVftxData::Clear calls in TTofVftxBoard combined
   // with ConstructedAt access in TTofVftxUnpacker

   FairRootManager * manager = FairRootManager::Instance();

   fTrbTdcBoardCollection = new TClonesArray( "TTofTrbTdcBoard", fuActiveTrbTdcNb );
   manager->Register( "TofTrbTdc","TofUnpack", fTrbTdcBoardCollection, 
                      fMbsUnpackPar->WriteDataInCbmOut() || fbSaveRawTdcBoards );

   fTrbHeader = new TTrbHeader();
   manager->Register( "TofTrbHeader","TofUnpack", fTrbHeader, kTRUE );

   return kTRUE;
}
void TTrbUnpackTof::SetSaveRawData( Bool_t bSaveRaw )
{
   fbSaveRawTdcBoards = bSaveRaw;
   LOG(INFO)<<"TTrbUnpackTof => Enable the saving of raw trb data in analysis output file"
            <<FairLogger::endl;

}

Bool_t TTrbUnpackTof::ClearOutput()
{
   LOG(DEBUG)<<"**** TTrbUnpackTof: Call ClearOutput()..."<<FairLogger::endl;

   if( 0 < fuActiveTrbTdcNb )
      fTrbTdcBoardCollection->Clear("C");

   fTdcUnpackMap.clear();

   return kTRUE;
}

void TTrbUnpackTof::CreateHistograms()
{
   LOG(DEBUG)<<"**** TTrbUnpackTof: Call CreateHistograms()..."<<FairLogger::endl;

   TDirectory* oldir = gDirectory;
   gROOT->cd();

   fTrbTriggerPattern = new TH1I("tof_trb_trigger_pattern", "CTS trigger pattern", 16, 0, 16);
   fTrbTriggerType = new TH1I("tof_trb_trigger_types", "CTS trigger types", 16, 0, 16);
   fTrbEventNumberJump = new TH1I("tof_trb_event_jump", "CTS event number jumps", 2500, 0, 2500);
   fCtsBusyTime = new TH1I("tof_trb_cts_busy", "CTS busy time", 100, 0, 100);
   fCtsBusyTime->GetXaxis()->SetTitle("#mus");
   fCtsIdleTime = new TH1I("tof_trb_cts_idle", "CTS idle time", 200, 0, 200);
   fCtsIdleTime->GetXaxis()->SetTitle("#mus");
   fCtsIdleTimeSpill = new TH1I("tof_trb_cts_idle_spill", "CTS inter-spill idle time", 50, 0, 50);
   fCtsIdleTimeSpill->GetXaxis()->SetTitle("s");
   fItcAssertions = new TH1I("tof_trb_itc_assert", "All trigger cycles since last event", 50, 0, 50);
   fItcEvents = new TH1I("tof_trb_cts_events", "Accepted cycles since last event", 50, 0, 50);

   fHadaqEventTime = new TH1I("tof_trb_hadaq_time", "HADAQ/DABC event time", 300, 0, 300);
   fHadaqEventTime->GetXaxis()->SetTitle("s");

   fCtsTriggerCycles = new TH1I("tof_trb_trigger_rate", "CTS trigger/event rate", 300, 0, 300);
   fCtsTriggerCycles->GetXaxis()->SetTitle("s");
   fCtsTriggerCycles->SetMarkerStyle(34);
   fCtsTriggerCycles->SetMarkerColor(kRed);
   fCtsTriggerCycles->SetLineColor(kRed);

   fCtsTriggerAccepted = new TH1I("tof_trb_event_rate", "CTS trigger/event rate", 300, 0, 300);
   fCtsTriggerAccepted->GetXaxis()->SetTitle("ms");
   fCtsTriggerAccepted->SetMarkerStyle(34);
   fCtsTriggerAccepted->SetMarkerColor(kGreen);
   fCtsTriggerAccepted->SetLineColor(kGreen);

   fHadaqTimeInSpill = new TH1I("tof_hadaq_time_spill", "HADAQ coarse time in spill", 20, 0, 20);
   fHadaqTimeInSpill->GetXaxis()->SetTitle("s");

   fCtsTimeInSpill = new TH1I("tof_cts_time_spill", "CTS fine time in spill", 20000, 0, 20000);
   fCtsTimeInSpill->GetXaxis()->SetTitle("ms");

   TH1* hTemp = 0;

   for( UInt_t uTrbSeb = 0; uTrbSeb < fuInDataTrbSebNb; uTrbSeb++ )
   {
     UInt_t uTrbNetAddress = fMbsUnpackPar->GetTrbSebAddr(uTrbSeb);

     hTemp = new TH1I( Form("tof_trb_size_subevent_%03u", uTrbSeb),
                       Form("data sent by TRB-SEB 0x%04x", uTrbNetAddress),
                       65, 0, 65);

     fTrbSubeventSize.push_back( hTemp );

     hTemp = new TH1I( Form("tof_trb_status_subevent_%03u", uTrbSeb),
                       Form("status bits of TRB-SEB 0x%04x", uTrbNetAddress),
    		           32, 0, 32);

     fTrbSubeventStatus.push_back( hTemp );
   }

   for( UInt_t uTrbTdc = 0; uTrbTdc < fuActiveTrbTdcNb; uTrbTdc++)
   {
     UInt_t uTrbNetAddress = fMbsUnpackPar->GetActiveTrbTdcAddr(uTrbTdc);

     hTemp = new TH1I( Form("tof_trb_words_tdc_%03u", uTrbTdc),
                       Form("words sent by TRB-TDC 0x%04x", uTrbNetAddress),
                       800, 0, 800);

     fTrbTdcWords.push_back( hTemp );

     hTemp = new TH1I( Form("tof_trb_process_status_tdc_%03u", uTrbTdc),
                       Form("data processing status of TRB-TDC 0x%04x", uTrbNetAddress),
                       trbtdc::process_StatusMessages, 0, trbtdc::process_StatusMessages);


     fTrbTdcProcessStatus.push_back( hTemp );
   }

   gDirectory->cd( oldir->GetPath() );
}

void TTrbUnpackTof::WriteHistograms()
{
   LOG(DEBUG)<<"**** TTrbUnpackTof: Call WriteHistograms()..."<<FairLogger::endl;

   TDirectory* oldir = gDirectory;

   TFile* tHist = new TFile("./tofTrbUnp.hst.root","RECREATE");

   fTrbTriggerPattern->Write();
   fTrbTriggerType->Write();
   fTrbEventNumberJump->Write();
   fCtsBusyTime->Write();
   fCtsIdleTime->Write();
   fCtsIdleTimeSpill->Write();
   fItcAssertions->Write();
   fItcEvents->Write();
   fHadaqEventTime->Write();
   fCtsTriggerCycles->Write();
   fCtsTriggerAccepted->Write();
   fHadaqTimeInSpill->Write();
   fCtsTimeInSpill->Write();

   for( UInt_t uTrbSeb = 0; uTrbSeb < fuInDataTrbSebNb; uTrbSeb++ )
   {
     fTrbSubeventSize[uTrbSeb]->Write();
     fTrbSubeventStatus[uTrbSeb]->Write();
   }

   for( UInt_t uTrbTdc = 0; uTrbTdc < fuActiveTrbTdcNb; uTrbTdc++)
   {
     fTrbTdcWords[uTrbTdc]->Write();
     fTrbTdcProcessStatus[uTrbTdc]->Write();
   }

   gDirectory->cd( oldir->GetPath() );
   tHist->Close();
   delete tHist;
}

void TTrbUnpackTof::DataErrorHandling(UInt_t uSubeventId, UInt_t uErrorPattern)
{
  // TrbNet network error and status bits
  if (tofTrb::status_network_0 & uErrorPattern)
  {
    LOG(DEBUG)<<Form("TrbNet network status: node replied to network request (%d) ",tofTrb::status_network_0)<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_network_0));
  }
  if (tofTrb::status_network_1 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet network status: endpoint data collision"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_network_1));
  }
  if (tofTrb::status_network_2 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet network status: incomplete data transfer between nodes"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_network_2));
  }
  if (tofTrb::status_network_3 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet network status: check-sum mismatch on point-to-point link"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_network_3));
  }
  if (tofTrb::status_network_4 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet network status: network request not understood by node"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_network_4));
  }
  if (tofTrb::status_network_5 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet network status: I/O buffer packet count mismatch"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_network_5));
  }
  if (tofTrb::status_network_6 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet network status: network transaction timeout"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_network_6));
  }

  // TrbNet readout error and status bits
  if (tofTrb::status_readout_16 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet readout status: trigger number mismatch"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_readout_16));
  }
  if (tofTrb::status_readout_17 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet readout status: trigger code mismatch"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_readout_17));
  }
  if (tofTrb::status_readout_18 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet readout status: wrong data stream length"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_readout_18));
  }
  if (tofTrb::status_readout_19 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet readout status: front-end failed to deliver any data"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_readout_19));
  }
  if (tofTrb::status_readout_20 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet readout status: requested CTS trigger number not in front-end event data buffer"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_readout_20));
  }
  if (tofTrb::status_readout_21 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet readout status: front-end data only partially sent"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_readout_21));
  }
  if (tofTrb::status_readout_22 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet readout status: severe readout problem (TrbNet reset required)"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_readout_22));
  }
  if (tofTrb::status_readout_23 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet readout status: single broken event"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_readout_23));
  }
  if (tofTrb::status_readout_24 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet readout status: GbE link down"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_readout_24));
  }
  if (tofTrb::status_readout_25 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet readout status: subevent buffer almost full"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_readout_25));
  }
  if (tofTrb::status_readout_26 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet readout status: corrupted data detected by the subevent builder"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_readout_26));
  }
  if (tofTrb::status_readout_27 & uErrorPattern)
  {
    LOG(ERROR)<<"TrbNet readout status: reference time problem"<<FairLogger::endl;
    fTrbSubeventStatus[ fMbsUnpackPar->GetTrbSebIndex( uSubeventId ) ]->Fill(TMath::Log2(tofTrb::status_readout_27));
  }

  return;
}
