// -------------------------------------------------------------------------------------
// ----- HadaqTrbIterator source file                                              -----
// -----                                                                           -----
// ----- created by C. Simon on 2014-03-07                                         -----
// -----                                                                           -----
// ----- based on stream by S. Linev                                               -----
// ----- https://subversion.gsi.de/go4/app/stream/framework/hadaq/TrbIterator.cxx  -----
// ----- revision 1046, 2013-11-20                                                 -----
// -------------------------------------------------------------------------------------

#include "HadaqTrbIterator.h"

#include "TString.h"

#include "FairLogger.h"

hadaq::TrbIterator::TrbIterator(void* data, UInt_t datalen) :
   fData(data),
   fDatalen(datalen),
   fEvCursor(0),
   fEvLen(0),
   fSubCursor(0),
   fSubLen(0),
   fSubsubCursor(0),
   fSubsubLen(0)
{
}

hadaq::RawEvent* hadaq::TrbIterator::nextEvent()
{
   if (fEvCursor == 0) {

      fEvCursor = fData;
      fEvLen = fDatalen;

   } else {
      hadaq::RawEvent* prev = (hadaq::RawEvent*) fEvCursor;

      UInt_t fulllen = prev->GetPaddedSize();

      LOG(DEBUG3)<<Form("nextEvent fEvLen = %u evlen = %u", fEvLen, fulllen)<<FairLogger::endl;

      if (fulllen >= fEvLen) {
         if (fulllen > fEvLen)
         {
            LOG(ERROR)<<Form("hadaq::RawEvent length mismatch %u %u", fulllen, fEvLen)<<FairLogger::endl;
         }
         fEvCursor = 0;
         fEvLen = 0;
      } else {
         fEvCursor = ((UChar_t*) fEvCursor) + fulllen;
         fEvLen -= fulllen;
      }
   }

   if ((fEvCursor!=0) && (fEvLen!=0) && (fEvLen < sizeof(hadaq::RawEvent))) {
      LOG(ERROR)<<Form("Strange hadaq::RawEvent length %u minumum %u", (UInt_t) fEvLen, (UInt_t) sizeof(hadaq::RawEvent))<<FairLogger::endl;
      fEvCursor = 0;
      fEvLen = 0;
   }

   fSubCursor = 0;
   fSubLen = 0;

   return (hadaq::RawEvent*) fEvCursor;
}

hadaq::RawSubevent* hadaq::TrbIterator::nextSubevent()
{
   RawEvent* ev = currEvent();

   if (ev==0) return 0;

   if (fSubCursor == 0) {
      fSubCursor = ((UChar_t*) ev) + sizeof(hadaq::RawEvent);

      fSubLen = ev->GetPaddedSize();

      if (fSubLen >= sizeof(hadaq::RawEvent)) {
         fSubLen -= sizeof(hadaq::RawEvent);
      } else {
         LOG(ERROR)<<Form("Wrong hadaq::RawEvent length %u header size %u", (UInt_t) ev->GetSize(), (UInt_t) sizeof(hadaq::RawEvent))<<FairLogger::endl;
         fSubLen = 0;
      }
   } else {

      hadaq::RawSubevent* sub = (hadaq::RawSubevent*) fSubCursor;

      UInt_t fulllen = sub->GetPaddedSize();

      LOG(DEBUG3)<<Form("Shift to next subevent size %u  fulllen %u", fulllen, fSubLen)<<FairLogger::endl;

      if (fulllen >= fSubLen) {
         if (fulllen > fSubLen)
         {
            LOG(ERROR)<<Form("Mismatch in subevent length %u %u", fulllen, fSubLen)<<FairLogger::endl;
         }
         fSubLen = 0;
      } else {
         fSubCursor = ((UChar_t*) fSubCursor) + fulllen;
         fSubLen -= fulllen;

         if (fSubLen < sizeof(hadaq::RawSubevent)) {
            fSubLen = 0;
         } else {

            UInt_t align = ((UChar_t*) fSubCursor - (UChar_t*) fEvCursor) % 8;

            if (align != 0) {
               LOG(ERROR)<<Form("Align problem %u != 0 of subevent relative to buffer begin", align)<<FairLogger::endl;
            }
         }
      }
   }

   if ((fSubLen==0) || ((fSubLen!=0) && (fSubLen < sizeof(hadaq::RawSubevent)))) {
//      LOG(ERROR)<<Form("Strange hadaq::Subevent length %u", fSubLen)<<FairLogger::endl;
      fSubCursor = 0;
      fSubLen = 0;
   }

   return (hadaq::RawSubevent*) fSubCursor;
}

