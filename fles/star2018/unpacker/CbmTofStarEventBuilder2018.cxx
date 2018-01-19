// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTofStarEventBuilder2018                 -----
// -----               Created 08.12.2017 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTofStarEventBuilder2018.h"
#include "CbmTofStar2018Par.h"
#include "CbmTofDigiExp.h"
#include "CbmTbEvent.h"

#include "CbmTbDaqBuffer.h"

//#include "CbmFiberHodoAddress.h"
#include "CbmHistManager.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

#include "TClonesArray.h"
#include "TString.h"

#include <iostream>
#include <stdint.h>
#include <iomanip>

static Int_t iMess=0;
const  Int_t DetMask = 0x0001FFFF;


CbmTofStarEventBuilder2018::CbmTofStarEventBuilder2018( UInt_t uNbGdpb )
  : CbmTSUnpack(),
    fuMsAcceptsPercent(100),
    fuTotalMsNb(0),
    fuOverlapMsNb(0),
    fdMsSizeInNs(0.0),
    fuMinNbGdpb( uNbGdpb ),
    fuCurrNbGdpb( 0 ),
    fuNrOfGdpbs(0),
    fuNrOfFebsPerGdpb(0),
    fuNrOfGet4PerFeb(0),
    fuNrOfChannelsPerGet4(0),
    fuNrOfChannelsPerFeet(0),
    fuNrOfGet4(0),
    fuNrOfGet4PerGdpb(0),
    fuNrOfChannelsPerGdpb(0),
    fMsgCounter(11,0), // length of enum MessageTypes initialized with 0
    fGdpbIdIndexMap(),
    fdStarTriggerDeadtime(),
    fdStarTriggerDelay(),
    fdStarTriggerWinSize(),
    fuGdpbId(0),
    fuGdpbNr(0),
    fuGet4Id(0),
    fuGet4Nr(0),
    fHM(new CbmHistManager()),
    fvulCurrentEpoch(),
    fvbFirstEpochSeen(),
    fNofEpochs(0),
    fulCurrentEpochTime(0.),
    fEquipmentId(0),
    fdMsIndex(0.),
    fdTShiftRef(0.),
    fTofDigi(),
    fDigi(NULL),
    fBuffer(CbmTbDaqBuffer::Instance()),
    fUnpackPar(NULL),
    fdRefTime(0.),
    fdLastDigiTime(0.),
    fdFirstDigiTimeDif(0.),
    fdEvTime0(0.),
    fhRawTDigEvT0( NULL ),
    fhRawTDigRef0( NULL ),
    fhRawTDigRef( NULL ),
    fhRawTRefDig0( NULL ),
    fhRawTRefDig1( NULL ),
    fhRawDigiLastDigi( NULL ),
    fhRawTotCh(),
    fhChCount(),
    fvbChanThere(),
    fhChanCoinc(),
    fulGdpbTsMsb(0),
    fulGdpbTsLsb(0),
    fulStarTsMsb(0),
    fulStarTsMid(0),
    fulGdpbTsFullLast(0),
    fulStarTsFullLast(0),
    fuStarTokenLast(0),
    fuStarDaqCmdLast(0),
    fuStarTrigCmdLast(0),
    fhTokenMsgType(NULL),
    fhTriggerRate(NULL),
    fhCmdDaqVsTrig(NULL),
    fhStarTokenEvo(NULL),
    fbEventBuilding( kFALSE ),
    fbTimeSortOutput( kFALSE ),
    fStarSubEvent(),
    fulNbBuiltSubEvent( 0 ),
    fulNbStarSubEvent( 0 ),
    fulNbBuiltSubEventLastPrintout( 0 ),
    fulNbStarSubEventLastPrintout( 0 ),
    fdCurrentMsStartTime( 0.0 ),
    fdCurrentMsEndTime( 0.0 ),
    fvmCurrentLinkBuffer(),
    fvtCurrentLinkBuffer(),
    fdCurrentTsStartTime( 0.0 ),
    fvmTsLinksBuffer(),
    fvtTsLinksBuffer(),
    fvmEpSupprBuffer()
{
}

CbmTofStarEventBuilder2018::~CbmTofStarEventBuilder2018()
{
}

Bool_t CbmTofStarEventBuilder2018::Init()
{
   LOG(INFO) << "Initializing flib Get4 unpacker" << FairLogger::endl;

   FairRootManager* ioman = FairRootManager::Instance();
   if( NULL == ioman )
   {
      LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
   }

   fTofDigi= new TClonesArray("CbmTofDigiExp", 10);
   if( NULL == fTofDigi )
   {
      LOG(FATAL) << "No Digi TClonesarray " << FairLogger::endl;
   }
   ioman->Register("CbmTofDigi", "Tof raw Digi", fTofDigi, kTRUE);

/*
   CbmTbEvent * fEventHeader = (CbmTbEvent *)ioman->GetObject("EventHeader.");
   if( NULL == fEventHeade r)
   {
      LOG(FATAL) << "No EventHeader TClonesarray " << FairLogger::endl;
   }
*/

   fUnpackPar = (CbmTofStar2018Par*)(FairRun::Instance());

   return kTRUE;
}

void CbmTofStarEventBuilder2018::SetParContainers()
{
  LOG(INFO) << "Setting parameter containers for " << GetName()
	    << FairLogger::endl;
  fUnpackPar = (CbmTofStar2018Par*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmTofStar2018Par"));

}

Bool_t CbmTofStarEventBuilder2018::InitContainers()
{
   LOG(INFO) << "Init parameter containers for " << GetName()
               << FairLogger::endl;
   Bool_t initOK = ReInitContainers();

   CreateHistograms();

   fvulCurrentEpoch.resize( fuNrOfGdpbs * fuNrOfGet4PerGdpb );
   fvbFirstEpochSeen.resize( fuNrOfGdpbs * fuNrOfGet4PerGdpb );
   fvbChanThere.resize( fUnpackPar->GetNumberOfChannels(), kFALSE );
   for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )
   {
      for( UInt_t j = 0; j < fuNrOfGet4PerGdpb; ++j )
      {
         fvulCurrentEpoch[GetArrayIndex(i, j)] = 0;
         fvbFirstEpochSeen[GetArrayIndex(i, j)] = kFALSE;
      } // for( UInt_t j = 0; j < fuNrOfGet4PerGdpb; ++j )
   } // for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )

   return initOK;
}

Bool_t CbmTofStarEventBuilder2018::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
             << FairLogger::endl;

   fuNrOfGdpbs = fUnpackPar->GetNrOfRocs();
   LOG(INFO) << "Nr. of Tof GDPBs: " << fuNrOfGdpbs << FairLogger::endl;
   fuMinNbGdpb = fuNrOfGdpbs;

   fuNrOfFebsPerGdpb = fUnpackPar->GetNrOfFebsPerGdpb();
   LOG(INFO) << "Nr. of FEBS per Tof GDPB: " << fuNrOfFebsPerGdpb
               << FairLogger::endl;

   fuNrOfGet4PerFeb = fUnpackPar->GetNrOfGet4PerFeb();
   LOG(INFO) << "Nr. of GET4 per Tof FEB: " << fuNrOfGet4PerFeb
               << FairLogger::endl;

   fuNrOfChannelsPerGet4 = fUnpackPar->GetNrOfChannelsPerGet4();
   LOG(INFO) << "Nr. of channels per GET4: " << fuNrOfChannelsPerGet4
               << FairLogger::endl;

   fuNrOfChannelsPerFeet = fuNrOfGet4PerFeb * fuNrOfChannelsPerGet4;
   LOG(INFO) << "Nr. of channels per FEET: " << fuNrOfChannelsPerFeet
               << FairLogger::endl;

   fuNrOfGet4 = fuNrOfGdpbs * fuNrOfFebsPerGdpb * fuNrOfGet4PerFeb;
   LOG(INFO) << "Nr. of GET4s: " << fuNrOfGet4 << FairLogger::endl;

   fuNrOfGet4PerGdpb = fuNrOfFebsPerGdpb * fuNrOfGet4PerFeb;
   LOG(INFO) << "Nr. of GET4s per GDPB: " << fuNrOfGet4PerGdpb
               << FairLogger::endl;

   fuNrOfChannelsPerGdpb = fuNrOfGet4PerGdpb * fuNrOfChannelsPerGet4;
   LOG(INFO) << "Nr. of channels per GDPB: " << fuNrOfChannelsPerGdpb
               << FairLogger::endl;

   fGdpbIdIndexMap.clear();
   for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )
   {
      fGdpbIdIndexMap[fUnpackPar->GetRocId(i)] = i;
      LOG(INFO) << "GDPB Id of TOF  " << i << " : " << std::hex << fUnpackPar->GetRocId(i)
                 << std::dec << FairLogger::endl;
   } // for( UInt_t i = 0; i < fuNrOfGdpbs; ++i )
   UInt_t uNrOfChannels = fUnpackPar->GetNumberOfChannels();
   LOG(INFO) << "Nr. of mapped Tof channels: " << uNrOfChannels;
   for( UInt_t i = 0; i < uNrOfChannels; ++i)
   {
      if (i % 8 == 0)
         LOG(INFO) << FairLogger::endl;
      LOG(INFO) << Form(" 0x%08x", fUnpackPar->GetChannelToDetUIdMap(i) );
   } // for( UInt_t i = 0; i < uNrOfChannels; ++i)
   LOG(INFO) << FairLogger::endl;

   fuTotalMsNb   = fUnpackPar->GetNbMsTot();
   fuOverlapMsNb = fUnpackPar->GetNbMsOverlap();
   fdMsSizeInNs  = fUnpackPar->GetSizeMsInNs();
   LOG(INFO) << "Timeslice parameters: "
             << fuTotalMsNb << " MS per link, of which "
             << fuOverlapMsNb << " overlap MS, each MS is "
             << fdMsSizeInNs << " ns"
             << FairLogger::endl;

   fdStarTriggerDeadtime.resize( fuNrOfGdpbs );
   fdStarTriggerDelay.resize(    fuNrOfGdpbs );
   fdStarTriggerWinSize.resize(  fuNrOfGdpbs );
   for (UInt_t iGdpb = 0; iGdpb < fuNrOfGdpbs; ++iGdpb)
   {
      fdStarTriggerDeadtime[ iGdpb ] = fUnpackPar->GetStarTriggDeadtime( iGdpb );
      fdStarTriggerDelay[ iGdpb ]    = fUnpackPar->GetStarTriggDelay( iGdpb );
      fdStarTriggerWinSize[ iGdpb ]  = fUnpackPar->GetStarTriggWinSize( iGdpb );
      LOG(INFO) << "STAR trigger par of gDPB  " << iGdpb << " are: "
                << " Deadtime "  <<  fdStarTriggerDeadtime[ iGdpb ]
                << " Delay "     << -fdStarTriggerDelay[ iGdpb ]
                << " and WinSz " <<  fdStarTriggerWinSize[ iGdpb ]
                << FairLogger::endl;
      if( fdStarTriggerDelay[ iGdpb ] < fdStarTriggerWinSize[ iGdpb ] )
         LOG(FATAL) << "STAR trigger window should end at latest on token itself!"
                    << " => Delay has to be greater or equal with window size!"
                    << FairLogger::endl;
   } // for (Int_t iGdpb = 0; iGdpb < fuNrOfGdpbs; ++iGdpb)

/// TODO clean epoch suppression in STAR 2018!
//   fvmEpSupprBuffer.resize( fuNrOfGet4 );

	return kTRUE;
}

void CbmTofStarEventBuilder2018::SetEventBuildingMode( Bool_t bEventBuildingMode )
{
   fbEventBuilding = bEventBuildingMode;
   if( fbEventBuilding )
      LOG(INFO) << "Event building mode = single link per subevent: " << FairLogger::endl
                << "test mode in 2018 S1, sector mode in 2018 S2"
                << FairLogger::endl;
      else LOG(INFO) << "Event building mode = all links in same subevent: " << FairLogger::endl
                     << "Sector mode in 2018 S1, Full eTOF mode in 2018 S2"
                     << FairLogger::endl;
}
void CbmTofStarEventBuilder2018::SetTimeSortOutput( Bool_t bTimeSort )
{
   fbTimeSortOutput = bTimeSort;
   if( fbTimeSortOutput )
      LOG(INFO) << "Output buffer will be time sorted before being sent to STAR DAQ " << FairLogger::endl
                << "=> This will have an effect (build time) only in eevent building mode with all links in one subevent "
                << " as data for single links are already time sorted"
                << FairLogger::endl;
      else LOG(INFO) << "Output buffer will NOT be time sorted before being sent to STAR DAQ " << FairLogger::endl
                     << "=> This will have an effect (build time) only in eevent building mode with all links in one subevent "
                     << " as data for single links are already time sorted"
                     << FairLogger::endl;
}

void CbmTofStarEventBuilder2018::CbmTofStarEventBuilder2018::CreateHistograms()
{
   LOG(INFO) << "create Histos for " << fuNrOfGdpbs <<" gDPBs "
	          << FairLogger::endl;

   fhRawTDigEvT0 = new TH1F( Form("Raw_TDig-EvT0"),
                             Form("Raw digi time difference to 1st digi ; time [ns]; cts"),
                             100, 0, 50.);
   fHM->Add( Form("Raw_TDig-EvT0"), fhRawTDigEvT0);

   fhRawTDigRef0 = new TH1F( Form("Raw_TDig-Ref0"),
                             Form("Raw digi time difference to Ref ; time [ns]; cts"),
                             5000, 0, 500000);
   fHM->Add( Form("Raw_TDig-Ref0"), fhRawTDigRef0);

   fhRawTDigRef = new TH1F( Form("Raw_TDig-Ref"),
                            Form("Raw digi time difference to Ref ; time [ns]; cts"),
                            5000, 0, 50000);
   fHM->Add( Form("Raw_TDig-Ref"), fhRawTDigRef);

   fhRawTRefDig0 = new TH1F( Form("Raw_TRef-Dig0"),
                             Form("Raw Ref time difference to last digi  ; time [ns]; cts"),
                             9999, -500000000, 500000000);
   fHM->Add( Form("Raw_TRef-Dig0"), fhRawTRefDig0);

   fhRawTRefDig1 = new TH1F( Form("Raw_TRef-Dig1"),
                             Form("Raw Ref time difference to last digi  ; time [ns]; cts"),
                             9999, -5000000, 5000000);
   fHM->Add( Form("Raw_TRef-Dig1"), fhRawTRefDig1);

   fhRawDigiLastDigi = new TH1F( Form("Raw_Digi-LastDigi"),
                                 Form("Raw Digi time difference to last digi  ; time [ns]; cts"),
                                 9999, -5000000, 5000000);
   fHM->Add( Form("Raw_Digi-LastDigi"), fhRawDigiLastDigi);

   fhRawTotCh.resize( fuNrOfGdpbs );
   fhChCount.resize( fuNrOfGdpbs );
   fhChanCoinc.resize( fuNrOfGdpbs * fuNrOfFebsPerGdpb / 2 );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
   {
      fhRawTotCh[ uGdpb ] = new TH2F( Form("Raw_Tot_gDPB_%02u", uGdpb),
                                      Form("Raw TOT gDPB %02u; channel; TOT [bin]", uGdpb),
                                      fuNrOfChannelsPerGdpb, 0., fuNrOfChannelsPerGdpb,
                                      256, 0., 256. );
      fHM->Add( Form("Raw_Tot_gDPB_%02u", uGdpb), fhRawTotCh[ uGdpb ]);

      fhChCount[ uGdpb ] = new TH1I( Form("ChCount_gDPB_%02u", uGdpb),
                                     Form("Channel counts gDPB %02u; channel; Hits", uGdpb),
                                     fuNrOfChannelsPerGdpb, 0., fuNrOfChannelsPerGdpb );
      fHM->Add( Form("ChCount_gDPB_%02u", uGdpb), fhChCount[ uGdpb ]);
/*
      for( UInt_t uLeftFeb = uGdpb*fuNrOfFebsPerGdpb / 2;
           uLeftFeb < (uGdpb + 1 )*fuNrOfFebsPerGdpb / 2;
           ++uLeftFeb )
      {
         fhChanCoinc[ uLeftFeb ] = new TH2F( Form("fhChanCoinc_%02u", uLeftFeb),
                                      Form("Channels Coincidence %02; Left; Right", uLeftFeb),
                                      fuNrOfChannelsPerFeet, 0., fuNrOfChannelsPerFeet,
                                      fuNrOfChannelsPerFeet, 0., fuNrOfChannelsPerFeet );
      } // for( UInt_t uLeftFeb = 0; uLeftFeb < fuNrOfFebsPerGdpb / 2; uLeftFeb ++ )
*/
         fhChanCoinc[ uGdpb ] = new TH2F( Form("fhChanCoinc_%02u", uGdpb),
                                      Form("Channels Coincidence %02u; Left; Right", uGdpb),
                                      fuNrOfChannelsPerGdpb, 0., fuNrOfChannelsPerGdpb,
                                      fuNrOfChannelsPerGdpb, 0., fuNrOfChannelsPerGdpb );
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)

//   fhDetChanCoinc = new TH2F( "fhDetChanCoinc",
//                              "Det Channels Coincidence; Left; Right",
//                              32, 0., 32,
//                              32, 0., 32 );
}

Bool_t CbmTofStarEventBuilder2018::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   LOG(DEBUG) << "Timeslice contains " << ts.num_microslices(component)
              << " microslices of component " << component << FairLogger::endl;

   // Loop over microslices
   Int_t iMessageType = -111;
   size_t numCompMsInTs = ts.num_microslices(component);
   for (size_t m = 0; m < numCompMsInTs; ++m)
   {
      // Jump some microslices if needed
//      if( fuMsAcceptsPercent < m)
//         continue;

      // Ignore overlap ms if number defined by user
      if( numCompMsInTs - fuOverlapMsNb <= m )
         continue;

      constexpr uint32_t kuBytesPerMessage = 8;

      auto msDescriptor = ts.descriptor(component, m);
      fEquipmentId = msDescriptor.eq_id;
      fdMsIndex = static_cast<double>(msDescriptor.idx);
      const uint8_t* msContent = reinterpret_cast< const uint8_t* >( ts.content(component, m) );

      uint32_t size = msDescriptor.size;
      if( 0 < size )
         LOG(DEBUG1) << "Microslice "<< m <<": " << fdMsIndex
                     << " has size: " << size << FairLogger::endl;

      // If not integer number of message in input buffer, print warning/error
      if( 0 != (size % kuBytesPerMessage) )
         LOG(ERROR) << "The input microslice buffer does NOT "
                    << "contain only complete nDPB messages!"
                    << FairLogger::endl;

      // If not integer number of message in input buffer, print warning/error
      if( 0 != (size % kuBytesPerMessage) )
         LOG(ERROR) << "The input microslice buffer does NOT "
                    << "contain only complete nDPB messages!"
                    << FairLogger::endl;

      // Compute the number of complete messages in the input microslice buffer
      uint32_t uNbMessages = (size - (size % kuBytesPerMessage) )
                            / kuBytesPerMessage;

      // Prepare variables for the loop on contents
      const uint64_t* pInBuff = reinterpret_cast<const uint64_t*>( msContent );
      for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)
      {
         // Fill message
         uint64_t ulData = static_cast<uint64_t>( pInBuff[uIdx] );
         gdpb::Message mess( ulData );

         if( gLogger->IsLogNeeded(DEBUG2) )
         {
            mess.printDataCout();
         } // if( gLogger->IsLogNeeded(DEBUG2) )

         // Increment counter for different message types
         iMessageType = mess.getMessageType();
         fMsgCounter[ iMessageType ]++;

         fuGdpbId = mess.getRocNumber();
         fuGdpbNr = fGdpbIdIndexMap[ fuGdpbId ];

         fuGet4Id = mess.getGdpbGenChipId();
         fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + fuGet4Id;

         if( fuNrOfGet4PerGdpb <= fuGet4Id &&
             gdpb::MSG_STAR_TRI != iMessageType &&
             get4v2x::kuChipIdMergedEpoch != fuGet4Id )
            LOG(WARNING) << "Message with Get4 ID too high: " << fuGet4Id
                         << " VS " << fuNrOfGet4PerGdpb << " set in parameters." << FairLogger::endl;

         /// First message in MS should ALWAYS be a merged epoch
         if( 0 == uIdx )
         {
            if( gdpb::MSG_EPOCH2 != iMessageType && get4v2x::kuChipIdMergedEpoch == fuGet4Id )
               LOG(WARNING) << "First message in MS is not a merged epoch!!!!!"
                                 << FairLogger::endl;
         } // if( 0 == uIdx )

         switch( iMessageType )
         {
            case gdpb::MSG_GET4:
            {
               //           FillEpochInfo(mess);
               LOG(ERROR) << "Message type " << iMessageType
                          << " not included in unpacker."
                          << FairLogger::endl;
               break;
            } // case old non tof messages
            case gdpb::MSG_EPOCH2:
            {
               if( get4v2x::kuChipIdMergedEpoch == fuGet4Id )
               {

                  for( UInt_t uChan = fuGdpbNr * fuNrOfChannelsPerGdpb;
                       uChan < (fuGdpbNr + 1 ) * fuNrOfChannelsPerGdpb;
                       ++uChan )
                     fvbChanThere[ uChan ] = kFALSE;
/*
                  if( 0 == fuGdpbNr )
                     for( UInt_t uDetChan = 0; uDetChan < 64; uDetChan ++)
                        fbDetChanThere[uDetChan] = kFALSE;
*/
                  /// Propagate for all ASICs corresponding to this merged epoch
                  for( uint32_t uGet4Index = 0; uGet4Index < fuNrOfGet4PerGdpb; uGet4Index ++ )
                  {
                     gdpb::Message tmpMess( mess );
                     tmpMess.setGdpbGenChipId( uGet4Index );
                     fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + uGet4Index;

                     FillEpochInfo( tmpMess );
                  } // for( uint32_t uGet4Index = 0; uGet4Index < fuNrOfGet4PerGdpb; uGetIndex ++ )

                  /// Keep track of extended epoch index for each gDPB
                  // TO BE DONE

                  ///
                  if( 0 == uIdx && kFALSE == fbEventBuilding )
                  {
                     gdpb::FullMessage fullMess( mess, mess.getGdpbEpEpochNb() );
                     fdCurrentMsStartTime = fullMess.GetFullTimeNs();
                     fdCurrentMsEndTime   = fdCurrentMsStartTime + fdMsSizeInNs;
                  } // if( 0 == uIdx && kFALSE == fbEventBuilding )
/*
                  for( UInt_t uChanA = fuGdpbNr * fuNrOfChannelsPerGdpb;
                       uChanA < (fuGdpbNr + 1 ) * fuNrOfChannelsPerGdpb;
                       ++uChanA )
                  {
                     if( kTRUE == fvbChanThere[ uChanA ] )
                     {
                        for( UInt_t uChanB = fuGdpbNr * fuNrOfChannelsPerGdpb;
                             uChanB < (fuGdpbNr + 1 ) * fuNrOfChannelsPerGdpb;
                             ++uChanB )
                        {
                           if( kTRUE == fvbChanThere[ uChanB ] )
                           {
                              fhChanCoinc[ fuGdpbNr  ]->Fill( uChanA - fuGdpbNr * fuNrOfChannelsPerGdpb,
                                                              uChanB - fuGdpbNr * fuNrOfChannelsPerGdpb );
                           } // if uChanB
                        } // for uChanB
                     } // if uChanA
                  } // for uChanA

                  if( 0 == fuGdpbNr )
                  {
                     for( UInt_t uDetChanLeft = 0; uDetChanLeft < 32; uDetChanLeft ++)
                        if( kTRUE == fbDetChanThere[ uDetChanLeft ] )
                           for( UInt_t uDetChanRight = 0; uDetChanRight < 32; uDetChanRight ++)
                              if( kTRUE == fbDetChanThere[ 32 + uDetChanRight ] )
                                 fhDetChanCoinc->Fill( uDetChanLeft, uDetChanRight );
                  } // if( 0 == fuGdpbNr )
*/
               } // if this epoch message is a merged one valiud for all chips
                  else
                  {
//                     fHistGet4MessType->Fill( fuGet4Nr, gdpb::GET4_32B_EPOCH );
                     FillEpochInfo(mess);
                  } // if single chip epoch message
               break;
            } // case gdpb::MSG_EPOCH2:
            case gdpb::MSG_GET4_32B:
            {
               FillHitInfo( mess );
/// TODO clean epoch suppression in STAR 2018!
//               fvmEpSupprBuffer[ fuGet4Nr ].push_back( mess );
               break;
            } // case gdpb::MSG_GET4_32B:
            case gdpb::MSG_GET4_SLC:
            {
               PrintSlcInfo(mess);
               break;
            } // case gdpb::MSG_GET4_SLC:
            case gdpb::MSG_GET4_SYS:
            {
               if(100 > iMess++)
                  PrintSysInfo(mess);
               break;
            } // case gdpb::MSG_GET4_SYS:
            case gdpb::MSG_STAR_TRI:
            {
               FillStarTrigInfo(mess);
               break;
            } // case gdpb::MSG_STAR_TRI:
            default:
            {
               if(100 > iMess++)
                  LOG(ERROR) << "Message ("<<iMess<<") type " << std::hex << std::setw(2)
                             << static_cast< uint16_t >( iMessageType )
                             << " not included in Get4 unpacker."
                             << FairLogger::endl;
               if(100 == iMess)
                  LOG(ERROR) << "Stop reporting MSG errors... "
                             << FairLogger::endl;
            } // default:
         } // switch( iMessageType )

      } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)

      /// Check if we are in single link per sub-event building mode
      if( kFALSE == fbEventBuilding )
      {
         /// Make sure that the current MS is not one of the overlap ones
         /// and build sub-events in it if that is the case
         if( m < fuTotalMsNb - fuOverlapMsNb )
            BuildStarEventsSingleLink();
      } // if( kFALSE == fbEventBuilding )

   } // for (size_t m = 0; m < numCompMsInTs; ++m)


  return kTRUE;
}

void CbmTofStarEventBuilder2018::FillHitInfo( gdpb::Message mess )
{
   // --- Get absolute time, GET4 ID and channel number
   UInt_t uGet4Id     = mess.getGdpbGenChipId();
   UInt_t uChannel    = mess.getGdpbHitChanId();
   UInt_t uTot        = mess.getGdpbHit32Tot();
   // In 32b mode the coarse counter is already computed back to 112 FTS bins
   // => need to hide its contribution from the Finetime
   // => FTS = Fullt TS modulo 112
   UInt_t uFts        = mess.getGdpbHitFullTs() % 112;

   ULong_t  ulCurEpochGdpbGet4 = fvulCurrentEpoch[ fuGet4Nr ];

   if( kTRUE == fvbFirstEpochSeen[ fuGet4Nr ] )
   {

/// TODO clean epoch suppression in STAR 2018!
/*
      // In Ep. Suppr. Mode, receive following epoch instead of previous
      if( 0 < ulCurEpochGdpbGet4 )
         ulCurEpochGdpbGet4 --;
         else ulCurEpochGdpbGet4 = get4v2x::kuEpochCounterSz; // Catch epoch cycle!
*/
      ULong_t  ulhitTime = mess.getMsgFullTime(  ulCurEpochGdpbGet4 );
      Double_t dHitTime  = mess.getMsgFullTimeD( ulCurEpochGdpbGet4 );
      Double_t dHitTot   = uTot;     // in bins


      UInt_t uFebIdx     = (uGet4Id / fuNrOfGet4PerFeb);
      UInt_t uFullFebIdx = (fuGdpbNr * fuNrOfFebsPerGdpb) + uFebIdx;

      UInt_t uChanInGdpb = uGet4Id * fuNrOfChannelsPerGet4 + uChannel;
      UInt_t uChanInSyst = fuGdpbNr * fuNrOfChannelsPerGdpb + uChanInGdpb;
      if( fUnpackPar->GetNumberOfChannels() < uChanInSyst )
      {
         LOG(ERROR) << "Invalid mapping index " << uChanInSyst
                    << " VS " << fUnpackPar->GetNumberOfChannels()
                    <<", from " << fuGdpbNr
                    <<", " << uGet4Id
                    <<", " << uChannel
                    << FairLogger::endl;
         return;
      } // if( fUnpackPar->GetNumberOfChannels() < uChanUId )

      fvbChanThere[ uChanInSyst ] = kTRUE;

      UInt_t uChanUId = fUnpackPar->GetChannelToDetUIdMap( uChanInSyst );
      if( 0 == uChanUId )
         return;   // Hit not mapped to digi

/*
      if( (uChanUId & DetMask) == 0x00001006 )
      {
         UInt_t uDetChan = (uChanUId & 0xFF000000) >> 24;
         if( (uChanUId & 0x00800000) == 0x00800000 )
            uDetChan += 32;

         fbDetChanThere[ uDetChan ] = kTRUE;
      } // if( (uChanUId & DetMask) == 0x00001006 )
*/

      fhRawDigiLastDigi->Fill( dHitTime - fdLastDigiTime );
      fdLastDigiTime = dHitTime;

      if( (uChanUId & DetMask) == 0x00001006 )
         dHitTime += fdTShiftRef;

      LOG(DEBUG) << Form("Insert 0x%08x digi with time ", uChanUId ) << dHitTime << Form(", Tot %4.0f",dHitTot)
                 << " into buffer with " << fBuffer->GetSize() << " data from "
                 << Form("%11.1f to %11.1f ", fBuffer->GetTimeFirst(), fBuffer->GetTimeLast())
                 << " at epoch " << ulCurEpochGdpbGet4
                 << FairLogger::endl;
      fDigi = new CbmTofDigiExp(uChanUId, dHitTime, dHitTot);

      fBuffer->InsertData(fDigi);

      // Histograms filling
      fhRawTotCh[ fuGdpbNr ]->Fill( uChanInGdpb, dHitTot);
      fhChCount[ fuGdpbNr ] ->Fill( uChanInGdpb );

      gdpb::FullMessage fullMess( mess, ulCurEpochGdpbGet4 );
      if( fbEventBuilding )
         fvmTsLinksBuffer[fuGdpbNr].push_back( fullMess );
         else
            fvmCurrentLinkBuffer.push_back( fullMess );

   } // if( kTRUE == fvbFirstEpochSeen[ fuGet4Nr ] )
}

void CbmTofStarEventBuilder2018::FillEpochInfo( gdpb::Message mess )
{
   ULong64_t ulEpochNr = mess.getGdpbEpEpochNb();

   fvulCurrentEpoch[ fuGet4Nr ] = ulEpochNr;

   if( kFALSE == fvbFirstEpochSeen[ fuGet4Nr ] )
      fvbFirstEpochSeen[ fuGet4Nr ] = kTRUE;

   fulCurrentEpochTime = mess.getMsgFullTime(ulEpochNr);
   fNofEpochs++;

   /// In Ep. Suppr. Mode, receive following epoch instead of previous
   /// Re-align the epoch number of the message in case it will be used later:
   /// We received the epoch after the data instead of the one before!
   /// TODO FIXME: commented out for the time being so that the order in the STAR
   ///             date is the same as in the CBM data
/*
   if( 0 < ulEpochNr )
      mess.setEpoch2Number( ulEpochNr - 1 );
   else mess.setEpoch2Number( get4v2x::kuEpochCounterSz );
*/

/// TODO clean epoch suppression in STAR 2018!
/*
   Int_t iBufferSize = fvmEpSupprBuffer[ fuGet4Nr ].size();
   if( 0 < iBufferSize )
   {
      LOG(DEBUG) << "Now processing stored messages for for get4 " << fuGet4Nr << " with epoch number "
                 << (fvulCurrentEpoch[ fuGet4Nr ] - 1) << FairLogger::endl;

      for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
      {
         FillHitInfo( fvmEpSupprBuffer[ fuGet4Nr ][ iMsgIdx ] );
      } // for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )

      fvmEpSupprBuffer[fuGet4Nr].clear();
   } // if( 0 < fvmEpSupprBuffer[fGet4Nr] )
*/
   gdpb::FullMessage fullMess( mess, ulEpochNr );
   if( fbEventBuilding )
      fvmTsLinksBuffer[fuGdpbNr].push_back( fullMess );
      else
         fvmCurrentLinkBuffer.push_back( fullMess );
}

void CbmTofStarEventBuilder2018::PrintSlcInfo(gdpb::Message mess)
{
   /// Nothing to do, maybe later use it to trakc parameter changes like treshold?
/*
  if( fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find( rocId ) )
     LOG(INFO) << "GET4 Slow Control message, epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][get4Id])
                << ", time " << std::setprecision(9) << std::fixed
                << Double_t(fulCurrentEpochTime) * 1.e-9 << " s "
                << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
                << FairLogger::endl
                << " +++++++ > Chip = " << std::setw(2) << mess.getGdpbGenChipId()
                << ", Chan = " << std::setw(1) << mess.getGdpbSlcChan()
                << ", Edge = " << std::setw(1) << mess.getGdpbSlcEdge()
                << ", Type = " << std::setw(1) << mess.getGdpbSlcType()
                << ", Data = " << std::hex << std::setw(6) << mess.getGdpbSlcData() << std::dec
                << ", Type = " << mess.getGdpbSlcCrc()
                << FairLogger::endl;
*/
   gdpb::FullMessage fullMess( mess, fulCurrentEpochTime );
   if( fbEventBuilding )
      fvmTsLinksBuffer[fuGdpbNr].push_back( fullMess );
      else
         fvmCurrentLinkBuffer.push_back( fullMess );

   /// TODO FIXME: Add the error messages to the buffer for inclusion in the STAR event
   ///             NEEDS the proper "<" operator in FullMessage or Message to allow time sorting
}

void CbmTofStarEventBuilder2018::PrintGenInfo(gdpb::Message mess)
{
   Int_t mType        = mess.getMessageType();
   Int_t rocId          = mess.getRocNumber();
   Int_t get4Id     = mess.getGdpbGenChipId();
   Int_t channel    = mess.getGdpbHitChanId();
   uint64_t            uData = mess.getData();
   if(100 > iMess++)
      LOG(INFO) << "Get4 MSG type "<<mType<<" from rocId "<<rocId<<", getId "<<get4Id
                << ", (hit channel) "<<channel<<Form(" hex data %0lx ",uData)
                << FairLogger::endl;
}

void CbmTofStarEventBuilder2018::PrintSysInfo(gdpb::Message mess)
{
   LOG(INFO) << "GET4 System message,       epoch " << (fvulCurrentEpoch[ fuGet4Nr])
                << ", time " << std::setprecision(9) << std::fixed
                << Double_t(fulCurrentEpochTime) * 1.e-9 << " s "
                << " for board ID " << std::hex << std::setw(4) << fuGdpbId << std::dec
                << FairLogger::endl;

   switch( mess.getGdpbSysSubType() )
   {
      case gdpb::SYSMSG_GET4_EVENT:
      {
         LOG(INFO) << " +++++++ > Chip = " << std::setw(2) << mess.getGdpbGenChipId()
                   << ", Chan = " << std::setw(1) << mess.getGdpbSysErrChanId()
                   << ", Edge = " << std::setw(1) << mess.getGdpbSysErrEdge()
                   << ", Empt = " << std::setw(1) << mess.getGdpbSysErrUnused()
                   << ", Data = " << std::hex << std::setw(2) << mess.getGdpbSysErrData() << std::dec
                   << " -- GET4 V1 Error Event"
                   << FairLogger::endl;

         /// TODO FIXME: Add the error messages to the buffer for inclusion in the STAR event
         ///             NEEDS the proper "<" operator in FullMessage or Message to allow time sorting
         break;
      } //
      case gdpb::SYSMSG_CLOSYSYNC_ERROR:
         LOG(INFO) << "Closy synchronization error" << FairLogger::endl;
         break;
      case gdpb::SYSMSG_TS156_SYNC:
         LOG(INFO) << "156.25MHz timestamp reset" << FairLogger::endl;
         break;
      case gdpb::SYSMSG_GDPB_UNKWN:
         LOG(INFO) << "Unknown GET4 message, data: " << std::hex << std::setw(8)
                   << mess.getGdpbSysUnkwData() << std::dec
                   << FairLogger::endl;
         break;
   } // switch( getGdpbSysSubType() )
}

void CbmTofStarEventBuilder2018::FillStarTrigInfo(gdpb::Message mess)
{
   Int_t iMsgIndex = mess.getStarTrigMsgIndex();

   switch( iMsgIndex )
   {
      case 0:
         fulGdpbTsMsb = mess.getGdpbTsMsbStarA();
         break;
      case 1:
         fulGdpbTsLsb = mess.getGdpbTsLsbStarB();
         fulStarTsMsb = mess.getStarTsMsbStarB();
         break;
      case 2:
         fulStarTsMid = mess.getStarTsMidStarC();
         break;
      case 3:
      {
         ULong64_t ulNewGdpbTsFull = ( fulGdpbTsMsb << 24 )
                           + ( fulGdpbTsLsb       );
         ULong64_t ulNewStarTsFull = ( fulStarTsMsb << 48 )
                           + ( fulStarTsMid <<  8 )
                           + mess.getStarTsLsbStarD();
         UInt_t uNewToken  = mess.getStarTokenStarD();
         UInt_t uNewDaqCmd  = mess.getStarDaqCmdStarD();
         UInt_t uNewTrigCmd = mess.getStarTrigCmdStarD();
         if( ( uNewToken == fuStarTokenLast ) && ( ulNewGdpbTsFull == fulGdpbTsFullLast ) &&
             ( ulNewStarTsFull == fulStarTsFullLast ) && ( uNewDaqCmd == fuStarDaqCmdLast ) &&
             ( uNewTrigCmd == fuStarTrigCmdLast ) )
         {
            LOG(DEBUG) << "Possible error: identical STAR tokens found twice in a row => ignore 2nd! "
                         << Form("token = %5u ", fuStarTokenLast )
                         << Form("gDPB ts  = %12llu ", fulGdpbTsFullLast )
                         << Form("STAR ts = %12llu ", fulStarTsFullLast )
                         << Form("DAQ cmd = %2u ", fuStarDaqCmdLast )
                         << Form("TRG cmd = %2u ", fuStarTrigCmdLast )
                         << FairLogger::endl;
            return;
         } // if exactly same message repeated

         if( (uNewToken != fuStarTokenLast + 1) &&
             0 < fulGdpbTsFullLast && 0 < fulStarTsFullLast &&
             ( 4095 != fuStarTokenLast || 1 != uNewToken)  )
            LOG(WARNING) << "Possible error: STAR token did not increase by exactly 1! "
                         << Form("old = %5u vs new = %5u ", fuStarTokenLast,   uNewToken)
                         << Form("old = %12llu vs new = %12llu ", fulGdpbTsFullLast, ulNewGdpbTsFull)
                         << Form("old = %12llu vs new = %12llu ", fulStarTsFullLast, ulNewStarTsFull)
                         << Form("old = %2u vs new = %2u ", fuStarDaqCmdLast,  uNewDaqCmd)
                         << Form("old = %2u vs new = %2u ", fuStarTrigCmdLast, uNewTrigCmd)
                         << FairLogger::endl;

         fulGdpbTsFullLast = ulNewGdpbTsFull;
         fulStarTsFullLast = ulNewStarTsFull;
         fuStarTokenLast   = uNewToken;
         fuStarDaqCmdLast  = uNewDaqCmd;
         fuStarTrigCmdLast = uNewTrigCmd;

         /// Generate Fake digi for NH analysis framework -----------///
         Double_t dTot = 1.;
         Double_t dTime = fulGdpbTsFullLast * 6.25;
         if( 0. == fdFirstDigiTimeDif && 0. != fdLastDigiTime )
         {
            fdFirstDigiTimeDif = dTime - fdLastDigiTime;
            LOG(INFO) << "Default fake digi time shift initialized to " << fdFirstDigiTimeDif
                      <<FairLogger::endl;
         } // if( 0. == fdFirstDigiTimeDif && 0. != fdLastDigiTime )

         dTime -= fdFirstDigiTimeDif;
         dTime += fdTShiftRef;

         LOG(DEBUG) << "Insert fake digi with time " << dTime << ", Tot " << dTot
                    << FairLogger::endl;
         fhRawTRefDig0->Fill( dTime - fdLastDigiTime);
         fhRawTRefDig1->Fill( dTime - fdLastDigiTime);

         fDigi = new CbmTofDigiExp(0x00005006, dTime, dTot); // fake start counter signal
         fBuffer->InsertData(fDigi);
         ///---------------------------------------------------------///

         /// Generate Trigger object and store it for event building ///
         CbmTofStarTrigger newTrig( fulGdpbTsFullLast, fulStarTsFullLast, fuStarTokenLast,
                                    fuStarDaqCmdLast, fuStarTrigCmdLast );
         if( fbEventBuilding )
            fvtTsLinksBuffer[fuGdpbNr].push_back( newTrig );
            else
               fvtCurrentLinkBuffer.push_back( newTrig );
         ///---------------------------------------------------------///

         break;
      } // case 3
      default:
         LOG(ERROR) << "Unknown Star Trigger messageindex: " << iMsgIndex << FairLogger::endl;
   } // switch( iMsgIndex )
}

void CbmTofStarEventBuilder2018::Reset()
{
  //  fFiberHodoRaw->Clear();
  fTofDigi->Clear();
}

void CbmTofStarEventBuilder2018::Finish()
{
   TString message_type;

   for( UInt_t uType = 0; uType < fMsgCounter.size(); ++uType)
   {
      switch(uType)
      {
         case 0:   message_type ="NOP"; break;
         case 1:   message_type ="HIT"; break;
         case 2:   message_type ="EPOCH"; break;
         case 3:   message_type ="SYNC"; break;
         case 4:   message_type ="AUX"; break;
         case 5:   message_type ="EPOCH2"; break;
         case 6:   message_type ="GET4"; break;
         case 7:   message_type ="SYS"; break;
         case 8:   message_type ="GET4_SLC"; break;
         case 9:   message_type ="GET4_32B"; break;
         case 10:  message_type ="GET4_SYS"; break;
         default:  message_type ="UNKNOWN"; break;
      } // switch(uType)
      LOG(INFO) << message_type << " messages: "
                << fMsgCounter[uType] << FairLogger::endl;
   } // for( UInt_t uType = 0; uType < fMsgCounter.size(); ++uType)

   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb )
      for( UInt_t uGet4 = 0; uGet4 < fuNrOfGet4PerGdpb; ++uGet4 )
         LOG(INFO) << "Last epoch for gDPB: "<< std::setw(4) << uGdpb
                   << " , GET4  " << std::setw(4) << uGet4
                   << " => " << fvulCurrentEpoch[GetArrayIndex(uGdpb, uGet4)]
                   << FairLogger::endl;
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;


   gDirectory->mkdir("Tof_Raw_gDPB");
   gDirectory->cd("Tof_Raw_gDPB");
   fHM->H1( Form("Raw_TDig-Ref0") )->Write();
   fHM->H1( Form("Raw_TDig-Ref") )->Write();
   fHM->H1( Form("Raw_TRef-Dig0") )->Write();
   fHM->H1( Form("Raw_TRef-Dig1") )->Write();
   fHM->H1( Form("Raw_Digi-LastDigi") )->Write();
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
   {
      fHM->H2( Form("Raw_Tot_gDPB_%02u", uGdpb) )->Write();
      fHM->H1( Form("ChCount_gDPB_%02u", uGdpb) )->Write();
/*
      for( UInt_t uLeftFeb = uGdpb*fuNrOfFebsPerGdpb / 2;
           uLeftFeb < (uGdpb + 1 )*fuNrOfFebsPerGdpb / 2;
           ++uLeftFeb )
         fhChanCoinc[ uLeftFeb ]->Write();
*/
      fhChanCoinc[ uGdpb ]->Write();
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
//   fhDetChanCoinc->Write();
   gDirectory->cd("..");

}


void CbmTofStarEventBuilder2018::FillOutput(CbmDigi* digi)
{
   if( 100 > iMess++ )
      LOG(DEBUG) << "Fill digi TClonesarray with "
                 << Form("0x%08x", digi->GetAddress())
                 << " at " << static_cast<Int_t>( fTofDigi->GetEntriesFast() )
                 << FairLogger::endl;


   new( (*fTofDigi)[ fTofDigi->GetEntriesFast() ] )
      CbmTofDigiExp( *( dynamic_cast<CbmTofDigiExp*>(digi) ) );
   //CbmTofDigiExp((CbmTofDigiExp *)digi);

   if( 0 == fTofDigi->GetEntriesFast())
      fdEvTime0=digi->GetTime();
      else fhRawTDigEvT0->Fill( digi->GetTime() - fdEvTime0 );

   if( (digi->GetAddress() & DetMask) != 0x00001006 )
   {
      fhRawTDigRef0->Fill( digi->GetTime() - fdRefTime);
      fhRawTDigRef->Fill( digi->GetTime() - fdRefTime);
   } // if( (digi->GetAddress() & DetMask) != 0x00001006 )
      else  fdRefTime = digi->GetTime();

   digi->Delete();
}

void CbmTofStarEventBuilder2018::BuildStarEventsSingleLink()
{

   std::vector< gdpb::FullMessage >::iterator itFirstMessageNextEvent   = fvmCurrentLinkBuffer.begin();
   Double_t dPrevEventEnd = 0.0;

   for( UInt_t uTriggIdx = 0; uTriggIdx < fvtCurrentLinkBuffer.size(); uTriggIdx++ )
   {
      Double_t dTriggerWinBeg = get4v2x::kdClockCycleSizeNs * fvtCurrentLinkBuffer[uTriggIdx].GetFullGdpbTs()
                                - fdStarTriggerDelay[ fuGdpbNr ];
      Double_t dTriggerWinEnd = get4v2x::kdClockCycleSizeNs * fvtCurrentLinkBuffer[uTriggIdx].GetFullGdpbTs()
                                - fdStarTriggerDelay[ fuGdpbNr ]
                                + fdStarTriggerWinSize[ fuGdpbNr ];
      Double_t dClosestTriggerWinStart = get4v2x::kdClockCycleSizeNs * fvtCurrentLinkBuffer[uTriggIdx].GetFullGdpbTs()
                                         - fdStarTriggerDelay[ fuGdpbNr ]
                                         + fdStarTriggerDeadtime[ fuGdpbNr ];
      Bool_t bFirstMessClosestEventFound = kFALSE;

      /// Associate this trigger to its subevent
      fStarSubEvent.SetTrigger( fvtCurrentLinkBuffer[uTriggIdx] );
      /// Set the source index of the subevent
      fStarSubEvent.SetSource( fuGdpbNr );

      /// Check if this trigger may correspond to an event with data in previous MS
      if( dTriggerWinBeg < fdCurrentMsStartTime )
         fStarSubEvent.SetStartBorderEventFlag();

      /// Check if this trigger may correspond to an event with data in next MS
      if( fdCurrentMsEndTime < dTriggerWinEnd )
         fStarSubEvent.SetEndBorderEventFlag();

      /// Check if this trigger leads to an event overlapping with the previous one
      if( 0 < dPrevEventEnd && dTriggerWinBeg < dPrevEventEnd )
         fStarSubEvent.SetOverlapEventFlag();

      for( std::vector< gdpb::FullMessage >::iterator itMess = itFirstMessageNextEvent;
           itMess != fvmCurrentLinkBuffer.end();
           ++ itMess )
      {
         Double_t dMessageFullTime = (*itMess).GetFullTimeNs();
         if( kFALSE == bFirstMessClosestEventFound &&
             dClosestTriggerWinStart < dMessageFullTime )
         {
            itFirstMessageNextEvent = itMess;
            bFirstMessClosestEventFound = kTRUE;
         } // If first possible hit of closest event not found and current hit fits

         if( dTriggerWinBeg   <= dMessageFullTime && dMessageFullTime <= dTriggerWinEnd )
         {
            /// Message belongs to this event
            fStarSubEvent.AddMsg( (*itMess) );
         } // if( dTriggerWinBeg   <= dMessageFullTime && dMessageFullTime <= dTriggerWinEnd )
            else if( dTriggerWinEnd < dMessageFullTime )
               /// First Message out of the window for this event => Stop there and go to the next
               break;
      } // Loop on message from first allowed by last event + deadtime to end

      /** TODO: clarify how we deal with multiple sub-events (eg one for each gDPB) **/
      /// Send the sub-event to the STAR systems
      Int_t  iBuffSzByte = 0;
      void * pDataBuff = fStarSubEvent.BuildOutput( iBuffSzByte );
      if( NULL != pDataBuff )
      {
         /// Valid output, do stuff with it!
         fulNbBuiltSubEvent++;

#ifdef STAR_SUBEVT_BUILDER
         /*
          ** Function to send sub-event block to the STAR DAQ system
          *       trg_word received is packed as:
          *
          *       trg_cmd|daq_cmd|tkn_hi|tkn_mid|tkn_lo
          */
         star_rhicf_write( fStarSubEvent.GetTrigger().GetStarTrigerWord(),
                           pDataBuff, iBuffSzByte );
         fulNbStarSubEvent++;
/*
         LOG(INFO) << "Sent STAR event with size " << iBuffSzByte << " Bytes"
                   << " and token " << fStarSubEvent.GetTrigger().GetStarToken()
                   << FairLogger::endl;
*/
#endif // STAR_SUBEVT_BUILDER
      } // if( NULL != pDataBuff )
         else LOG(ERROR) << "Invalid STAR SubEvent Output, can only happen if trigger "
                         << " object was not set => Do Nothing more with it!!! "
                         << FairLogger::endl;

      /// Fill plot of event size
//      fhStarEventSize_gDPB[ iGdpbIdx ]->Fill( iBuffSzByte );
      /// Fill plot of event size as function of trigger time
//      if( 0 < fdStartTime )
//         fhStarEventSizeTime_gDPB[ iGdpbIdx ]->Fill( 1e-9 *(dTriggerTime - fdStartTime), iBuffSzByte );

      /// Now clear the sub-event
      fStarSubEvent.ClearSubEvent();
   } // for( UInt_t uTriggIdx = 0; uTriggIdx < fvtCurrentLinkBuffer.size(); uTriggIdx++ )

   /// Remove message until the first which could fit in an event in next MS
   fvmCurrentLinkBuffer.erase( fvmCurrentLinkBuffer.begin(), itFirstMessageNextEvent);
}
void CbmTofStarEventBuilder2018::BuildStarEventsAllLinks()
{
}

ClassImp(CbmTofStarEventBuilder2018)
