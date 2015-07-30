// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                      CbmGet4EventBuffer                           -----
// -----                    Created 24.02.2015 by                          -----
// -----                        P.-A. Loizeau                              -----
// -----                                                                   -----
// -----------------------------------------------------------------------------


#include <iostream>
#include <iomanip>

#include "TString.h"

#include "FairLogger.h"

#include "CbmGet4EventBuffer.h"

/*
 * TODO:
 * 1) Find proper default values for fuBufferMaxSize, fuBufferMaxEventSpread
 *    and fuBufferHighWater
 */

// -----   Initialization of static variables   ------------------------------
CbmGet4EventBuffer* CbmGet4EventBuffer::fgInstance = NULL;
// ---------------------------------------------------------------------------

// -----   Constructor   -----------------------------------------------------
CbmGet4EventBuffer::CbmGet4EventBuffer() :
      fData(),
      fuBufferMaxSize(500),
      fuBufferMaxEventSpread(100),
      fuBufferHighWater(400)
{
   fData.clear();
   LOG(INFO) << "CbmGet4EpochBuffer::CbmGet4EpochBuffer = Starting event buffer with: "
             << FairLogger::endl;
   LOG(INFO) << fuBufferHighWater << " stored events for buffer high water "
             << FairLogger::endl;
   LOG(INFO) << fuBufferMaxSize << " stored events for maximum buffer storage "
             << FairLogger::endl;
   LOG(INFO) << fuBufferMaxEventSpread << " for events spread rejection when maximum reached "
             << FairLogger::endl;
   PrintStatus();
}
// ---------------------------------------------------------------------------

// -----   Destructor   ------------------------------------------------------
CbmGet4EventBuffer::~CbmGet4EventBuffer()
{
}
// ---------------------------------------------------------------------------

// -----   Access to next Event   ---------------------------------------------
std::pair< CbmGet4EventHeader, std::vector< get4v1x::FullMessage >* >
   CbmGet4EventBuffer::GetNextEvent()
{
  // --- Check for empty buffer
  if( !fData.size() )
//     return std::make_pair( 0, NULL);
     return std::make_pair< CbmGet4EventHeader, std::vector< get4v1x::FullMessage >* >(0, NULL);

  // --- Get data from buffer
  std::pair< CbmGet4EventHeader, std::vector< get4v1x::FullMessage >* > event =
        fData.front();
  fData.pop_front();
  return event;

}
// ---------------------------------------------------------------------------

// -----   Insert Event into buffer   -----------------------------------------
Int_t CbmGet4EventBuffer::InsertEvent( ULong64_t uEventIdx,
      std::vector< get4v1x::FullMessage >* eventCont )
{

  if( !eventCont )
     LOG(FATAL)<<"CbmGet4EventBuffer::InsertEvent = invalid event vector pointer"
                           << FairLogger::endl;

  if( 0 < eventCont->size() )
  {
     LOG(DEBUG) << "CbmGet4EventBuffer::InsertEvent = Inserting event vector, event idx "
                 << uEventIdx << FairLogger::endl;
     CbmGet4EventHeader header(0);
     header.fuEventIndex = uEventIdx;
     std::pair< CbmGet4EventHeader, std::vector< get4v1x::FullMessage >* > event(header, eventCont);
     fData.push_back(event);

     if( fuBufferHighWater == fData.size() )
     {
        LOG(INFO) << "CbmGet4EventBuffer::InsertEvent = High water mark reached,  "
                    << fuBufferHighWater << "/" << fuBufferMaxSize
                    << " event stored in buffer."<< FairLogger::endl;
        LOG(INFO) << "                                  If MAX reached, event older than "
                  << fuBufferMaxEventSpread << " events from the new one will be dropped!"
                  << FairLogger::endl;
     } // if( fuBufferHighWater == fData.size() )
     if( fuBufferMaxSize <= fData.size() )
     {
        LOG(WARNING) << "CbmGet4EventBuffer::InsertEvent = Max buffer size reached. "
                     << "Now dropping all events more than " << fuBufferMaxEventSpread
                     << " events older than the newly inserted one!!!"
                     << FairLogger::endl;

        // now drop all events too old
        // WARNING: in case of event index cycle (should never happen but well...) the
        //          "older" epoch is also thrown out
        // Loop stop at least when reaching the newly inserted event
        while( ( ((fData.front()).first).fuEventIndex + fuBufferMaxEventSpread
                     < uEventIdx ) ||
               ( uEventIdx < ((fData.front()).first).fuEventIndex )
              )
        {
           // Throw out the event: clear event vector, delete event vector, pop the pair
           ((fData.front()).second)->clear();
           delete ((fData.front()).second);
           fData.pop_front();
        } // while first event has to be thrown out
     } // if( fuBufferMaxSize <= fData.size() )
  } // if( 0 < eventCont->size() )
     else
     {
        LOG(DEBUG2) << "CbmGet4EventBuffer: Trying to insert an empty event vector => do nothing "
                    << FairLogger::endl;
     } // else of if( 0 < eventCont->size() )

   return fData.size();
}

Int_t CbmGet4EventBuffer::InsertEvent( ULong64_t uEventIdx,
      Int_t iTrigType, ULong64_t uTrigExtEp, UInt_t uTrigTs,
      std::vector< get4v1x::FullMessage >* eventCont)
{

  if( !eventCont )
     LOG(FATAL)<<"CbmGet4EventBuffer::InsertEvent = invalid event vector pointer"
                           << FairLogger::endl;

  if( 0 < eventCont->size() )
  {
     LOG(DEBUG) << "CbmGet4EventBuffer::InsertEvent = Inserting event vector, event idx "
                 << uEventIdx << FairLogger::endl;
     CbmGet4EventHeader header( uEventIdx, iTrigType, uTrigExtEp, uTrigTs);
     std::pair< CbmGet4EventHeader, std::vector< get4v1x::FullMessage >* > event(header, eventCont);
     fData.push_back(event);

     if( fuBufferHighWater == fData.size() )
     {
        LOG(INFO) << "CbmGet4EventBuffer::InsertEvent = High water mark reached,  "
                    << fuBufferHighWater << "/" << fuBufferMaxSize
                    << " event stored in buffer."<< FairLogger::endl;
        LOG(INFO) << "                                  If MAX reached, event older than "
                  << fuBufferMaxEventSpread << " events from the new one will be dropped!"
                  << FairLogger::endl;
     } // if( fuBufferHighWater == fData.size() )
     if( fuBufferMaxSize <= fData.size() )
     {
        LOG(WARNING) << "CbmGet4EventBuffer::InsertEvent = Max buffer size reached. "
                     << "Now dropping all events more than " << fuBufferMaxEventSpread
                     << " events older than the newly inserted one!!!"
                     << FairLogger::endl;

        // now drop all events too old
        // WARNING: in case of event index cycle (should never happen but well...) the
        //          "older" epoch is also thrown out
        // Loop stop at least when reaching the newly inserted event
        while( ( ((fData.front()).first).fuEventIndex + fuBufferMaxEventSpread
                     < uEventIdx ) ||
               ( uEventIdx < ((fData.front()).first).fuEventIndex )
              )
        {
           // Throw out the event: clear event vector, delete event vector, pop the pair
           ((fData.front()).second)->clear();
           delete ((fData.front()).second);
           fData.pop_front();
        } // while first event has to be thrown out
     } // if( fuBufferMaxSize <= fData.size() )
  } // if( 0 < eventCont->size() )
     else
     {
        LOG(DEBUG2) << "CbmGet4EventBuffer: Trying to insert an empty event vector => do nothing "
                    << FairLogger::endl;
     } // else of if( 0 < eventCont->size() )

   return fData.size();
}

Int_t CbmGet4EventBuffer::InsertEvent( CbmGet4EventHeader eventHdr,
      std::vector< get4v1x::FullMessage >* eventCont)
{

  if( !eventCont )
     LOG(FATAL)<<"CbmGet4EventBuffer::InsertEvent = invalid event vector pointer"
                           << FairLogger::endl;

  if( 0 < eventCont->size() )
  {
     LOG(DEBUG) << "CbmGet4EventBuffer::InsertEvent = Inserting event vector, event idx "
                 << eventHdr.fuEventIndex << FairLogger::endl;

     std::pair< CbmGet4EventHeader, std::vector< get4v1x::FullMessage >* > event(eventHdr, eventCont);
     fData.push_back(event);

     if( fuBufferHighWater == fData.size() )
     {
        LOG(INFO) << "CbmGet4EventBuffer::InsertEvent = High water mark reached,  "
                    << fuBufferHighWater << "/" << fuBufferMaxSize
                    << " event stored in buffer."<< FairLogger::endl;
        LOG(INFO) << "                                  If MAX reached, event older than "
                  << fuBufferMaxEventSpread << " events from the new one will be dropped!"
                  << FairLogger::endl;
     } // if( fuBufferHighWater == fData.size() )
     if( fuBufferMaxSize <= fData.size() )
     {
        LOG(WARNING) << "CbmGet4EventBuffer::InsertEvent = Max buffer size reached. "
                     << "Now dropping all events more than " << fuBufferMaxEventSpread
                     << " events older than the newly inserted one!!!"
                     << FairLogger::endl;

        // now drop all events too old
        // WARNING: in case of event index cycle (should never happen but well...) the
        //          "older" epoch is also thrown out
        // Loop stop at least when reaching the newly inserted event
        while( ( ((fData.front()).first).fuEventIndex + fuBufferMaxEventSpread
                     < eventHdr.fuEventIndex ) ||
               ( eventHdr.fuEventIndex < ((fData.front()).first).fuEventIndex )
              )
        {
           // Throw out the event: clear event vector, delete event vector, pop the pair
           ((fData.front()).second)->clear();
           delete ((fData.front()).second);
           fData.pop_front();
        } // while first event has to be thrown out
     } // if( fuBufferMaxSize <= fData.size() )
  } // if( 0 < eventCont->size() )
     else
     {
        LOG(DEBUG2) << "CbmGet4EventBuffer: Trying to insert an empty event vector => do nothing "
                    << FairLogger::endl;
     } // else of if( 0 < eventCont->size() )

   return fData.size();
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
void CbmGet4EventBuffer::Clear()
{
   LOG(DEBUG)<< "CbmGet4EventBuffer::Clear " << FairLogger::endl;
   while( 0 < GetSize()  )
   {
      // Get next Event
      std::pair< CbmGet4EventHeader, std::vector< get4v1x::FullMessage >* > pEvtCont = GetNextEvent();
      if( 0 < (pEvtCont.second)->size() )
         (pEvtCont.second)->clear();
      delete (pEvtCont.second);
   } // while( 0 < fOutputBuffer->GetSize()  )
}
// ---------------------------------------------------------------------------



// -----   Instance   --------------------------------------------------------
CbmGet4EventBuffer* CbmGet4EventBuffer::Instance() {
  if ( ! fgInstance ) fgInstance = new CbmGet4EventBuffer();
  return fgInstance;
}
// ---------------------------------------------------------------------------



// -----   Print status   ----------------------------------------------------
void CbmGet4EventBuffer::PrintStatus() const {
  UInt_t size = GetSize();
  LOG(INFO) << "CbmGet4EventBuffer: Status ";
  if( !size )
  {
    LOG(INFO) << "empty" << FairLogger::endl;
    return;
  } // if( !size )
  LOG(INFO) << GetSize() << " event vectors from event "
            << Form("%9llu to event ", GetEventFirst() )
            << Form("%9llu ", GetEventLast() ) << FairLogger::endl;
}
// ---------------------------------------------------------------------------

