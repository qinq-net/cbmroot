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
   fTrbIterator(NULL),
   fTrbTdcUnpacker(NULL),
   fTrbTdcBoardCollection(NULL),
   fbSaveRawTdcBoards(kFALSE),
   fTdcUnpackMap(),
   fuInDataTrbSebNb(0),
   fuActiveTrbTdcNb(0),
   fTrbTriggerPattern(NULL),
   fTrbTriggerType(NULL),
   fTrbEventNumberJump(NULL),
   fTrbSubeventSize(),
   fTrbSubeventStatus(),
   fTrbTdcWords(),
   fTrbTdcProcessStatus()
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
   LOG(DEBUG)<<"First word in MBS subevent: "<<Form("0x%.8x",*data)<<FairLogger::endl;
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

   LOG(DEBUG2)<<"First   word in HADAQ event: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent))<<FairLogger::endl;
   LOG(DEBUG2)<<"Second  word in HADAQ event: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent+1))<<FairLogger::endl;
   LOG(DEBUG2)<<"Third   word in HADAQ event: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent+2))<<FairLogger::endl;
   LOG(DEBUG2)<<"Fourth  word in HADAQ event: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent+3))<<FairLogger::endl;
   LOG(DEBUG2)<<"Fifth   word in HADAQ event: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent+4))<<FairLogger::endl;
   LOG(DEBUG2)<<"Sixth   word in HADAQ event: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent+5))<<FairLogger::endl;
   LOG(DEBUG2)<<"Seventh word in HADAQ event: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent+6))<<FairLogger::endl;
   LOG(DEBUG2)<<"Eighth  word in HADAQ event: "<<Form("0x%.8x",*((Int_t*)tCurrentEvent+7))<<FairLogger::endl;

   LOG(DEBUG2)<<"First   word in memory: "<<Form("0x%.8x",tCurrentEvent->tuSize)<<FairLogger::endl;
   LOG(DEBUG2)<<"Second  word in memory: "<<Form("0x%.8x",tCurrentEvent->tuDecoding)<<FairLogger::endl;
   LOG(DEBUG2)<<"Third   word in memory: "<<Form("0x%.8x",tCurrentEvent->tuId)<<FairLogger::endl;
   LOG(DEBUG2)<<"Fourth  word in memory: "<<Form("0x%.8x",tCurrentEvent->evtSeqNr)<<FairLogger::endl;
   LOG(DEBUG2)<<"Fifth   word in memory: "<<Form("0x%.8x",tCurrentEvent->evtDate)<<FairLogger::endl;
   LOG(DEBUG2)<<"Sixth   word in memory: "<<Form("0x%.8x",tCurrentEvent->evtTime)<<FairLogger::endl;
   LOG(DEBUG2)<<"Seventh word in memory: "<<Form("0x%.8x",tCurrentEvent->evtRunNr)<<FairLogger::endl;
   LOG(DEBUG2)<<"Eighth  word in memory: "<<Form("0x%.8x",tCurrentEvent->evtPad)<<FairLogger::endl;

   if( 0 == tCurrentEvent )
   {
     LOG(ERROR)<<"Bad HADAQ raw event. Skip whole MBS subevent."<<FairLogger::endl;
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

         for(UInt_t uChannel = 0; uChannel < 16; uChannel++)
         {
           if( uTriggerPattern & (0x1 << uChannel) )
           {
             fTrbTriggerPattern->Fill(TMath::Log2( uTriggerPattern & (0x1 << uChannel) ));
           }
         }

         fTrbTriggerType->Fill(uTriggerType);

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
   if( fMbsUnpackPar->IsTrbEventUnpacked(uTriggerPattern) )
   {
	 for ( std::map<Int_t,std::pair<hadaq::RawSubevent*,UInt_t> >::iterator it = fTdcUnpackMap.begin();
	       it != fTdcUnpackMap.end();
	       ++it)
	 {
       fTrbTdcProcessStatus[ it->first ]->Fill( fTrbTdcUnpacker->ProcessData( (it->second).first, (it->second).second ) );
	 }

   }

   LOG(DEBUG)<<FairLogger::endl;

   LOG(DEBUG)<<Form("Total number of subevents in HADAQ raw event: %u", uNbSubevents)<<FairLogger::endl;
   LOG(DEBUG)<<Form("The parameter file specifies %u active TRB subevent builders.", fMbsUnpackPar->GetActiveTrbSebNb())<<FairLogger::endl;
   LOG(DEBUG)<<FairLogger::endl;
   LOG(DEBUG)<<FairLogger::endl;
   
   fiNbEvents++;
   fiPreviousEventNumber = fiCurrentEventNumber;
   
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
    LOG(DEBUG)<<"TrbNet network status: node replied to network request"<<FairLogger::endl;
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
