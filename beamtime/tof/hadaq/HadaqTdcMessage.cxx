// -----------------------------------------------------------------------------------
// ----- HadaqTdcMessage source file                                             -----
// -----                                                                         -----
// ----- created by C. Simon on 2014-04-02                                       -----
// -----                                                                         -----
// ----- based on stream by S. Linev                                             -----
// ----- https://subversion.gsi.de/go4/app/stream/framework/hadaq/TdcMessage.cxx -----
// ----- revision 1046, 2013-11-20                                               -----
// -----------------------------------------------------------------------------------

#include "HadaqTdcMessage.h"

#include "TString.h"

#include "FairLogger.h"

UInt_t hadaq::TdcMessage::gFineMinValue = trbtdc::kuSimpleFineTimeMinValue;
UInt_t hadaq::TdcMessage::gFineMaxValue = trbtdc::kuSimpleFineTimeMaxValue;

void hadaq::TdcMessage::print(Double_t tm)
{
   switch (getKind()) {
      case trbtdc::message_Reserved:
         LOG(DEBUG)<<Form("     tdc reserv 0x%08x", (UInt_t) fData)<<FairLogger::endl;
         break;

      case trbtdc::message_Header:
         LOG(DEBUG)<<Form("     tdc head   0x%08x", (UInt_t) fData)<<FairLogger::endl;
         break;
      case trbtdc::message_Debug:
         LOG(DEBUG)<<Form("     tdc debug  0x%08x", (UInt_t) fData)<<FairLogger::endl;
         break;
      case trbtdc::message_Epoch:
         LOG(DEBUG)<<Form("     tdc epoch  0x%08x", (UInt_t) fData);
         if (tm>=0) LOG(DEBUG)<<Form("  tm:%9.2f", tm*1e9);
         LOG(DEBUG)<<Form("   epoch 0x%x", (UInt_t) getEpochValue())<<FairLogger::endl;
         break;
      case trbtdc::message_Time:
      case trbtdc::message_Time1:
      case trbtdc::message_Time2:
      case trbtdc::message_Time3:
         LOG(DEBUG)<<Form("     tdc hit    0x%08x", (UInt_t) fData);
         if (tm>=0) LOG(DEBUG)<<Form("  tm:%9.2f", tm*1e9);
         LOG(DEBUG)<<Form("   ch %3u isrising:%u tc 0x%03x tf 0x%03x",
                 (UInt_t) getTimeChannel(), (UInt_t)getTimeEdge(),
                 (UInt_t)getTimeTmCoarse(), (UInt_t)getTimeTmFine())<<FairLogger::endl;
         break;
      default:
         LOG(DEBUG)<<Form("     tdc unkn   0x%08x   kind %u", (UInt_t) fData, (UInt_t) getKind())<<FairLogger::endl;
         break;
   }
}
