// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmMcbm2018TofUnpacker                       -----
// -----               Created 08.12.2017 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018TofUnpacker.h"
#include "CbmTofStar2018Par.h"
//#include "CbmTofUnpackPar.h"
#include "CbmTofDigiExp.h"
#include "CbmTofAddress.h"
#include "CbmTofDetectorId_v14a.h" // in cbmdata/tof
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


CbmMcbm2018TofUnpacker::CbmMcbm2018TofUnpacker()
  : CbmTSUnpack(),
    fuMsAcceptsPercent(100),
    fuOverlapMsNb(0),
    fuCoreMs(0),
    fuMinNbGdpb( 0 ),
    fuCurrNbGdpb( 0 ),
    fUnpackPar(),
    fuNrOfGdpbs(0),
    fuNrOfFeetPerGdpb(0),
    fuNrOfGet4PerFeb(0),
    fuNrOfChannelsPerGet4(0),
    fuNrOfChannelsPerFeet(0),
    fuNrOfGet4(0),
    fuNrOfGet4PerGdpb(0),
    fuNrOfChannelsPerGdpb(0),
    fulCurrentTsIndex(0),
    fuCurrentMs(0),
    fdMsIndex(0),
    fuGdpbId(0),
    fuGdpbNr(0),
    fuGet4Id(0),
    fuGet4Nr(0),
    fiEquipmentId(0),
    fMsgCounter(11,0), // length of enum MessageTypes initialized with 0
    fGdpbIdIndexMap(),
    fvulGdpbTsMsb(),
    fvulGdpbTsLsb(),
    fvulStarTsMsb(),
    fvulStarTsMid(),
    fvulGdpbTsFullLast(),
    fvulStarTsFullLast(),
    fvuStarTokenLast(),
    fvuStarDaqCmdLast(),
    fvuStarTrigCmdLast(),
    fHM(new CbmHistManager()),
    fvulCurrentEpoch(),
    fvbFirstEpochSeen(),
    fNofEpochs(0),
    fulCurrentEpochTime(0.),
    fdTShiftRef(0.),
    fTofDigi(),
    fDigi(NULL),
    fBuffer(CbmTbDaqBuffer::Instance()),
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
    fhDetChanCoinc(nullptr),
    fvuPadiToGet4(),
    fvuGet4ToPadi(),
    fvuElinkToGet4(),
    fvuGet4ToElink(),
    fvmEpSupprBuffer()
{
}

CbmMcbm2018TofUnpacker::~CbmMcbm2018TofUnpacker()
{
}

Bool_t CbmMcbm2018TofUnpacker::Init()
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

   //fUnpackPar = (CbmTofStar2018Par*)(FairRun::Instance());

   return kTRUE;
}

void CbmMcbm2018TofUnpacker::SetParContainers()
{
  LOG(INFO) << "Setting parameter containers for " << GetName()
	    << FairLogger::endl;
  fUnpackPar = (CbmTofStar2018Par*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmTofStar2018Par"));

}

Bool_t CbmMcbm2018TofUnpacker::InitContainers()
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

Bool_t CbmMcbm2018TofUnpacker::ReInitContainers()
{
    LOG(INFO) << "ReInit parameter containers for " << GetName()
             << FairLogger::endl;

   fuNrOfGdpbs = fUnpackPar->GetNrOfRocs();
   LOG(INFO) << "Nr. of Tof GDPBs: " << fuNrOfGdpbs << FairLogger::endl;
   fuMinNbGdpb = fuNrOfGdpbs;

   fuNrOfFeetPerGdpb = fUnpackPar->GetNrOfFebsPerGdpb();
   LOG(INFO) << "Nr. of FEBS per Tof GDPB: " << fuNrOfFeetPerGdpb
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

   fuNrOfGet4 = fuNrOfGdpbs * fuNrOfFeetPerGdpb * fuNrOfGet4PerFeb;
   LOG(INFO) << "Nr. of GET4s: " << fuNrOfGet4 << FairLogger::endl;

   fuNrOfGet4PerGdpb = fuNrOfFeetPerGdpb * fuNrOfGet4PerFeb;
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
   for( UInt_t i = 0; i < uNrOfChannels; ++i)
   {
      if (i % 8 == 0)
         LOG(INFO) << FairLogger::endl;
      LOG(INFO) << Form(" 0x%08x", fUnpackPar->GetChannelToDetUIdMap(i) );
   } // for( UInt_t i = 0; i < uNrOfChannels; ++i)
   LOG(INFO) << FairLogger::endl;

   fuTotalMsNb   = fUnpackPar->GetNbMsTot();
   fuOverlapMsNb = fUnpackPar->GetNbMsOverlap();
   fuCoreMs      = fuTotalMsNb - fuOverlapMsNb;
   fdMsSizeInNs  = fUnpackPar->GetSizeMsInNs();
   fdTsCoreSizeInNs = fdMsSizeInNs * fuCoreMs;
   LOG(INFO) << "Timeslice parameters: "
             << fuTotalMsNb << " MS per link, of which "
             << fuOverlapMsNb << " overlap MS, each MS is "
             << fdMsSizeInNs << " ns"
             << FairLogger::endl;

   /// STAR Trigger decoding and monitoring
   fvulGdpbTsMsb.resize(  fuNrOfGdpbs );
   fvulGdpbTsLsb.resize(  fuNrOfGdpbs );
   fvulStarTsMsb.resize(  fuNrOfGdpbs );
   fvulStarTsMid.resize(  fuNrOfGdpbs );
   fvulGdpbTsFullLast.resize(  fuNrOfGdpbs );
   fvulStarTsFullLast.resize(  fuNrOfGdpbs );
   fvuStarTokenLast.resize(  fuNrOfGdpbs );
   fvuStarDaqCmdLast.resize(  fuNrOfGdpbs );
   fvuStarTrigCmdLast.resize(  fuNrOfGdpbs );
   for (UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; ++uGdpb)
   {
      fvulGdpbTsMsb[ uGdpb ] = 0;
      fvulGdpbTsLsb[ uGdpb ] = 0;
      fvulStarTsMsb[ uGdpb ] = 0;
      fvulStarTsMid[ uGdpb ] = 0;
      fvulGdpbTsFullLast[ uGdpb ] = 0;
      fvulStarTsFullLast[ uGdpb ] = 0;
      fvuStarTokenLast[ uGdpb ]   = 0;
      fvuStarDaqCmdLast[ uGdpb ]  = 0;
      fvuStarTrigCmdLast[ uGdpb ] = 0;
   } // for (Int_t iGdpb = 0; iGdpb < fuNrOfGdpbs; ++iGdpb)

   fvmEpSupprBuffer.resize( fuNrOfGet4 );

/// TODO: move these constants somewhere shared, e.g the parameter file
   fvuPadiToGet4.resize( fuNrOfChannelsPerFeet );
   fvuGet4ToPadi.resize( fuNrOfChannelsPerFeet );
   /* source: Monitor
   UInt_t uGet4topadi[32] = {
       4,  3,  2,  1,  // provided by Jochen
      24, 23, 22, 21,
       8,  7,  6,  5,
      28, 27, 26, 25,
      12, 11, 10,  9,
      32, 31, 30, 29,
      16, 15, 14, 13,
      20, 19, 18, 17 };
   */
   UInt_t uGet4topadi[32] = {
       4,  3,  2,  1,  // provided by Jochen
       8,  7,  6,  5,
      12, 11, 10,  9,
      16, 15, 14, 13,
      20, 19, 18, 17,
      24, 23, 22, 21,
      28, 27, 26, 25,
      32, 31, 30, 29
   };

  UInt_t uPaditoget4[32] = {
       4,  3,  2,  1,  // provided by Jochen
      12, 11, 10,  9,
      20, 19, 18, 17,
      28, 27, 26, 25,
      32, 31, 30, 29,
       8,  7,  6,  5,
      16, 15, 14, 13,
      24, 23, 22, 21 };

   for( UInt_t uChan = 0; uChan < fuNrOfChannelsPerFeet; ++uChan )
   {
      fvuPadiToGet4[ uChan ] = uPaditoget4[ uChan ] - 1;
      fvuGet4ToPadi[ uChan ] = uGet4topadi[ uChan ] - 1;
   } // for( UInt_t uChan = 0; uChan < fuNrOfChannelsPerFeet; ++uChan )


/// TODO: move these constants somewhere shared, e.g the parameter file
   fvuElinkToGet4.resize( kuNbGet4PerGbtx );
   fvuGet4ToElink.resize( kuNbGet4PerGbtx );
   UInt_t kuElinkToGet4[ kuNbGet4PerGbtx ] = { 27,  2,  7,  3, 31, 26, 30,  1,
                                               33, 37, 32, 13,  9, 14, 10, 15,
                                               17, 21, 16, 35, 34, 38, 25, 24,
                                                0,  6, 20, 23, 18, 22, 28,  4,
                                               29,  5, 19, 36, 39,  8, 12, 11
                                              };
   UInt_t kuGet4ToElink[ kuNbGet4PerGbtx ] = { 24,  7,  1,  3, 31, 33, 25,  2,
                                               37, 12, 14, 39, 38, 11, 13, 15,
                                               18, 16, 28, 34, 26, 17, 29, 27,
                                               23, 22,  5,  0, 30, 32,  6,  4,
                                               10,  8, 20, 19, 35,  9, 21, 36
                                              };
   for( UInt_t uLinkAsic = 0; uLinkAsic < kuNbGet4PerGbtx; ++uLinkAsic )
   {
      fvuElinkToGet4[ uLinkAsic ] = kuElinkToGet4[ uLinkAsic ];
      fvuGet4ToElink[ uLinkAsic ] = kuGet4ToElink[ uLinkAsic ];
   } // for( UInt_t uChan = 0; uChan < fuNrOfChannelsPerFeet; ++uChan )


   const UInt_t fuNrOfGbtxPerGdpb = 6;

// FIXME: move these inputs to the parameter file
   const UInt_t kuNrOfGbtx = 12;   // for 2 Gdpbs
   Int_t kiNrOfRpc  [ kuNrOfGbtx ] = { 5, 5, 5, 5,  5,  5,  5,  5,  5,  5,  5,  5};
   Int_t kiRpcType  [ kuNrOfGbtx ] = { kuNrOfGbtx*0    };
   Int_t kiModuleId [ kuNrOfGbtx ] = { 4, 4, 2, 2, -1, -1,  1,  1,  0,  0,  3,  3};
   Int_t kiRpcSide  [ kuNrOfGbtx ] = { 0, 1, 0, 1,  0,  1,  0,  1,  0,  1,  0,  1};

   UInt_t uNrOfGbtx  = kuNrOfGbtx;
   fviRpcType.resize(uNrOfGbtx);
   fviModuleId.resize(uNrOfGbtx);
   fviNrOfRpc.resize(uNrOfGbtx);
   fviRpcSide.resize(uNrOfGbtx);

   for (Int_t iGbtx = 0; iGbtx < uNrOfGbtx; ++iGbtx) 
   {
     fviNrOfRpc[ iGbtx ]  = kiNrOfRpc[ iGbtx ];   
     fviRpcType[ iGbtx ]  = kiRpcType[ iGbtx ];   
     fviModuleId[ iGbtx ] = kiModuleId[ iGbtx ];   
     fviRpcSide[ iGbtx ]  = kiRpcSide[ iGbtx ];   
   }

   uNrOfChannels = fuNrOfGet4 * fuNrOfChannelsPerGet4;
   LOG(INFO) << "Nr. of possible Tof channels: " << uNrOfChannels
	     << FairLogger::endl;

   CbmTofDetectorId* fTofId = new CbmTofDetectorId_v14a();
   fviRpcChUId.resize(uNrOfChannels);
   UInt_t iCh= 0; 
   for(Int_t iGbtx= 0; iGbtx < uNrOfGbtx; iGbtx++)  {
     for(Int_t iRpc= 0; iRpc < fviNrOfRpc[iGbtx]; iRpc++)  {
       for(Int_t iStr= 0; iStr < 32; iStr++)  {
	 Int_t iStrMap = iStr;
	 if( fviRpcSide[iGbtx] == 1) iStrMap=31-iStr; 
	 fviRpcChUId[iCh]=CbmTofAddress::GetUniqueAddress(fviModuleId[iGbtx],
							  iRpc,iStrMap,
							  fviRpcSide[iGbtx],
							  fviRpcType[iGbtx]);

	 LOG(DEBUG1)<<Form("Map Ch %d to Address 0x%08x",iCh,fviRpcChUId[iCh])
		   << FairLogger::endl;

	 iCh++;
       }
     }
   }

   for( UInt_t i = 0; i < uNrOfChannels; ++i)
   {
      if (i % 8 == 0)
	LOG(INFO) << FairLogger::endl;
      LOG(INFO) << Form(" 0x%08x", fviRpcChUId[i] );
   } // for( UInt_t i = 0; i < uNrOfChannels; ++i)

   LOG(INFO) << FairLogger::endl;

   // LOG(FATAL)<< "Stop here for debugging " << FairLogger::endl;

   return kTRUE;
}

void CbmMcbm2018TofUnpacker::CreateHistograms()
{
   LOG(INFO) << "create Histos for " << fuNrOfGdpbs <<" gDPBs "
	          << FairLogger::endl;

   fhRawTDigEvT0 = new TH1F( Form("Raw_TDig-EvT0"),
                             Form("Raw digi time difference to 1st digi ; time [ns]; cts"),
                             500, 0, 100.);
   fHM->Add( Form("Raw_TDig-EvT0"), fhRawTDigEvT0);

   fhRawTDigRef0 = new TH1F( Form("Raw_TDig-Ref0"),
                             Form("Raw digi time difference to Ref ; time [ns]; cts"),
                             6000, -10000, 50000);
   fHM->Add( Form("Raw_TDig-Ref0"), fhRawTDigRef0);

   fhRawTDigRef = new TH1F( Form("Raw_TDig-Ref"),
                            Form("Raw digi time difference to Ref ; time [ns]; cts"),
                            6000, -1000, 5000);
   fHM->Add( Form("Raw_TDig-Ref"), fhRawTDigRef);

   fhRawTRefDig0 = new TH1F( Form("Raw_TRef-Dig0"),
                             Form("Raw Ref time difference to last digi  ; time [ns]; cts"),
                             9999, -50000, 50000);
   fHM->Add( Form("Raw_TRef-Dig0"), fhRawTRefDig0);

   fhRawTRefDig1 = new TH1F( Form("Raw_TRef-Dig1"),
                             Form("Raw Ref time difference to last digi  ; time [ns]; cts"),
                             9999, -5000, 5000);
   fHM->Add( Form("Raw_TRef-Dig1"), fhRawTRefDig1);

   fhRawDigiLastDigi = new TH1F( Form("Raw_Digi-LastDigi"),
                                 Form("Raw Digi time difference to last digi  ; time [ns]; cts"),
                                 9999, -5000, 5000);
   //                                 9999, -5000000, 5000000);
   fHM->Add( Form("Raw_Digi-LastDigi"), fhRawDigiLastDigi);

   fhRawTotCh.resize( fuNrOfGdpbs );
   fhChCount.resize( fuNrOfGdpbs );
   fhChanCoinc.resize( fuNrOfGdpbs );
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
   {
      fhRawTotCh[ uGdpb ] = new TH2F( Form("Raw_Tot_gDPB_%02u", uGdpb),
                                      Form("Raw TOT gDPB %02u; channel; TOT [bin]", uGdpb),
                                      fuNrOfGet4PerGdpb*4, 0., fuNrOfGet4PerGdpb*4,
                                      256, 0., 256. );
      fHM->Add( Form("Raw_Tot_gDPB_%02u", uGdpb), fhRawTotCh[ uGdpb ]);

      fhChCount[ uGdpb ] = new TH1I( Form("ChCount_gDPB_%02u", uGdpb),
                                     Form("Channel counts gDPB %02u; channel; Hits", uGdpb),
                                     fuNrOfChannelsPerGdpb, 0., fuNrOfChannelsPerGdpb );
      fHM->Add( Form("ChCount_gDPB_%02u", uGdpb), fhChCount[ uGdpb ]);

      fhChanCoinc[ uGdpb ] = new TH2F( Form("fhChanCoinc_%02u", uGdpb),
                                      Form("Channels Coincidence %02u; Left; Right", uGdpb),
                                      fuNrOfChannelsPerGdpb, 0., fuNrOfChannelsPerGdpb,
                                      fuNrOfChannelsPerGdpb, 0., fuNrOfChannelsPerGdpb );
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
}

Bool_t CbmMcbm2018TofUnpacker::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   LOG(DEBUG) << "Timeslice contains " << ts.num_microslices(component)
              << " microslices of component " << component << FairLogger::endl;

   // Loop over microslices
   Int_t messageType = -111;
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
      fiEquipmentId = msDescriptor.eq_id;
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
         gdpbv100::Message mess( ulData );

         if( gLogger->IsLogNeeded(DEBUG2) )
         {
            mess.printDataCout();
         } // if( gLogger->IsLogNeeded(DEBUG2) )

////////////////////////////////////////////////////////////////////////
//                   TEMP SOLUTION                                    //
////////////////////////////////////////////////////////////////////////
         fuGdpbId = mess.getGdpbGenGdpbId();

         /// Check if this gDPB ID was declared in parameter file and stop there if not
         auto it = fGdpbIdIndexMap.find( fuGdpbId );
         if( it == fGdpbIdIndexMap.end() )
         {
            LOG(FATAL) << "Could not find the gDPB index for AFCK id 0x"
                      << std::hex << fuGdpbId << std::dec
                      << " in microslice " << fdMsIndex
                      << FairLogger::endl
                      << "If valid this index has to be added in the TOF parameter file in the RocIdArray field"
                      << FairLogger::endl;
            continue;
         } // if( it == fGdpbIdIndexMap.end() )
            else fuGdpbNr = fGdpbIdIndexMap[ fuGdpbId ];
////////////////////////////////////////////////////////////////////////
//                   TEMP SOLUTION                                    //
////////////////////////////////////////////////////////////////////////


         // Increment counter for different message types
         messageType = mess.getMessageType();
         fMsgCounter[ messageType ]++;

///         fuGet4Id = mess.getGdpbGenChipId();
         fuGet4Id = ConvertElinkToGet4( mess.getGdpbGenChipId() );
         fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + fuGet4Id;

         if( fuNrOfGet4PerGdpb <= fuGet4Id &&
             !mess.isStarTrigger()  &&
             ( gdpbv100::kuChipIdMergedEpoch != fuGet4Id ) )
	   {
            LOG(WARNING) << "Message with Get4 ID too high: " << fuGet4Id
                         << " VS " << fuNrOfGet4PerGdpb << " set in parameters, message ignored" 
			 << FairLogger::endl;
	    continue;
	   }

         switch( messageType )
         {
	    case gdpbv100::MSG_HIT:
            {
               if( mess.getGdpbHitIs24b() )
               {
                  PrintGenInfo(mess);
               } // if( getGdpbHitIs24b() )
                  else
                  {
                     fvmEpSupprBuffer[fuGet4Nr].push_back( mess );
                  } // else of if( getGdpbHitIs24b() )
	      break;
            } // case  gdpbv100::MSG_HIT:
            case gdpbv100::MSG_EPOCH:
            {
	      if( gdpbv100::kuChipIdMergedEpoch == fuGet4Id )
               {
                  for( uint32_t uGet4Index = 0; uGet4Index < fuNrOfGet4PerGdpb; uGet4Index ++ )
                  {
                     fuGet4Id = uGet4Index;
                     fuGet4Nr = (fuGdpbNr * fuNrOfGet4PerGdpb) + fuGet4Id;
                     gdpbv100::Message tmpMess(mess);
                     tmpMess.setGdpbGenChipId( uGet4Index );

		     //  fhGet4MessType->Fill(fuGet4Nr, 1);
                     FillEpochInfo(tmpMess);
                  } // for( uint32_t uGet4Index = 0; uGet4Index < fuNrOfGet4PerGdpb; uGetIndex ++ )
               } // if this epoch message is a merged one valiud for all chips
               else
               {
		 //     fhGet4MessType->Fill(fuGet4Nr, 1);
                  FillEpochInfo(mess);
               } // if single chip epoch message
	      break;
  	    } // case gdpbv100::MSG_EPOCH:
	    case gdpbv100::MSG_SLOWC:
            {
               PrintSlcInfo(mess);
               break;
            } // case gdpbv100::MSG_SLOWC:
            case gdpbv100::MSG_SYST:
            {
	       PrintSysInfo(mess);
               break;
            } // case gdpbv100::MSG_SYST:
            case gdpbv100::MSG_STAR_TRI_A:
            case gdpbv100::MSG_STAR_TRI_B:
            case gdpbv100::MSG_STAR_TRI_C:
            case gdpbv100::MSG_STAR_TRI_D:
               FillStarTrigInfo(mess);
               break;
    
            default:
            {
               if(100 > iMess++)
                  LOG(ERROR) << "Message ("<<iMess<<") type " << std::hex << std::setw(2)
                             << static_cast< uint16_t >( mess.getMessageType() )
                             << " not included in Get4 unpacker."
                             << FairLogger::endl;
               if(100 == iMess)
                  LOG(ERROR) << "Stop reporting MSG errors... "
                             << FairLogger::endl;
            } // default:
         } // switch( mess.getMessageType() )

      } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)

   } // for (size_t m = 0; m < numCompMsInTs; ++m)


  return kTRUE;
}

static Int_t iErrorMess=0;

void CbmMcbm2018TofUnpacker::FillHitInfo( gdpbv100::Message mess )
{
   UInt_t uChannel = mess.getGdpbHitChanId();  // Get4 channel nr
   UInt_t uTot     = mess.getGdpbHit32Tot();
   UInt_t uFts     = mess.getGdpbHitFineTs();

   ULong_t  ulCurEpochGdpbGet4 = fvulCurrentEpoch[ fuGet4Nr ];

   // In Ep. Suppr. Mode, receive following epoch instead of previous
   if( 0 < ulCurEpochGdpbGet4 )
      ulCurEpochGdpbGet4 --;
      else ulCurEpochGdpbGet4 = gdpbv100::kuEpochCounterSz; // Catch epoch cycle!

   UInt_t uChannelNr = fuGet4Id * fuNrOfChannelsPerGet4 + uChannel;
   UInt_t uChannelNrInFeet = (fuGet4Id % fuNrOfGet4PerFeb) * fuNrOfChannelsPerGet4 + uChannel;
   UInt_t uFeetNr   = (fuGet4Id / fuNrOfGet4PerFeb);
   UInt_t uFeetNrInSys = fuGdpbNr * fuNrOfFeetPerGdpb + uFeetNr;
   UInt_t uGbtxNr      = (uFeetNr / kuNbFeePerGbtx);
   UInt_t uFeetInGbtx  = (uFeetNr % kuNbFeePerGbtx);
   UInt_t uGbtxNrInSys = fuGdpbNr * kuNbGbtxPerGdpb + uGbtxNr;
   UInt_t uRemappedChannelNr = fuGdpbNr * fuNrOfChannelsPerGdpb + 
                               uFeetNr * fuNrOfChannelsPerFeet 
                             + fvuGet4ToPadi[ uChannelNrInFeet ];
   //   UInt_t uRemappedChannelNr = uFeetNr * fuNrOfChannelsPerFeet + uChannelNrInFeet;
    if( fuGdpbNr==2)
      LOG(INFO)<<" Fill Hit GdpbNr" << fuGdpbNr
	       <<", ChNr "<<uChannelNr<<", CIF "<< uChannelNrInFeet
	       <<", FNr "<<uFeetNr<<", FIS "<<uFeetNrInSys
	       <<", GbtxNr "<<uGbtxNr
	       <<", Remap "<<uRemappedChannelNr<<FairLogger::endl;

   ULong_t  ulHitTime = mess.getMsgFullTime(ulCurEpochGdpbGet4);
   Double_t dHitTime  = mess.getMsgFullTimeD(ulCurEpochGdpbGet4);

   uFts = mess.getGdpbHitFullTs() % 112;

   if( kTRUE == fvbFirstEpochSeen[ fuGet4Nr ] )
   {
      Double_t dHitTot   = uTot;     // in bins

      //if( fUnpackPar->GetNumberOfChannels() < uRemappedChannelNr )
      if( fviRpcChUId.size() < uRemappedChannelNr )
      {
	if(iErrorMess++ < 10000)
	{
	  LOG(ERROR) << "Invalid mapping index " << uRemappedChannelNr
		     << " VS " << fviRpcChUId.size()
		     << ", from GdpbNr " << fuGdpbNr
		     << ", Get4 " << fuGet4Id
		     << ", Ch " << uChannel
		     << ", ChNr " << uChannelNr
		     << ", ChNrIF " << uChannelNrInFeet
		     << ", FiS " << uFeetNrInSys
		     << FairLogger::endl;
	  return;
        } else 
	  LOG(FATAL) << "Max number of error messages reached "
		     << FairLogger::endl;

      } // if( fUnpackPar->GetNumberOfChannels() < uChanUId )

      fvbChanThere[ uRemappedChannelNr ] = kTRUE;

      // UInt_t uChanUId = fUnpackPar->GetChannelToDetUIdMap( uRemappedChannelNr );
      UInt_t uChanUId =  fviRpcChUId[ uRemappedChannelNr ];
      if( 0 == uChanUId )
         return;   // Hit not mapped to digi

      if( (uChanUId & DetMask) != 0x00005006 )  dHitTime += fdTShiftRef;
      fdLastDigiTime = dHitTime;

      LOG(DEBUG) << Form("Insert 0x%08x digi with time ", uChanUId ) << dHitTime << Form(", Tot %4.0f",dHitTot)
                 << " into buffer with " << fBuffer->GetSize() << " data from "
                 << Form("%11.1f to %11.1f ", fBuffer->GetTimeFirst(), fBuffer->GetTimeLast())
                 << " at epoch " << ulCurEpochGdpbGet4
                 << FairLogger::endl;

      fDigi = new CbmTofDigiExp(uChanUId, dHitTime, dHitTot);

      fBuffer->InsertData(fDigi);

      // Histograms filling
      fhRawTotCh[ fuGdpbNr ]->Fill( uRemappedChannelNr, dHitTot);
      fhChCount[ fuGdpbNr ] ->Fill( uRemappedChannelNr );

   } // if( kTRUE == fvbFirstEpochSeen[ fuGet4Nr ] )
}

void CbmMcbm2018TofUnpacker::FillEpochInfo( gdpbv100::Message mess )
{
   ULong64_t ulEpochNr = mess.getGdpbEpEpochNb();

   //   LOG(DEBUG1) << "Get4Nr "<<fuGet4Nr<< " in epoch "<<ulEpochNr<<FairLogger::endl;

   fvulCurrentEpoch[ fuGet4Nr ] = ulEpochNr;

   if( kFALSE == fvbFirstEpochSeen[ fuGet4Nr ] )
      fvbFirstEpochSeen[ fuGet4Nr ] = kTRUE;

   fulCurrentEpochTime = mess.getMsgFullTime(ulEpochNr);
   fNofEpochs++;

   /// Re-align the epoch number of the message in case it will be used later:
   /// We received the epoch after the data instead of the one before!
   if( 0 < ulEpochNr )
      mess.setGdpbEpEpochNb( ulEpochNr - 1 );
      else mess.setGdpbEpEpochNb( gdpbv100::kuEpochCounterSz );

   Int_t iBufferSize = fvmEpSupprBuffer[ fuGet4Nr ].size();
   if( 0 < iBufferSize )
   {
     LOG(DEBUG) << "Now processing "<<iBufferSize<<" stored messages for get4 " 
                << fuGet4Nr << " with epoch number "
                << (fvulCurrentEpoch[ fuGet4Nr ] - 1) << FairLogger::endl;

      /// Data are sorted between epochs, not inside => Epoch level ordering
      /// Sorting at lower bin precision level
      std::stable_sort( fvmEpSupprBuffer[fuGet4Nr].begin(), fvmEpSupprBuffer[fuGet4Nr].begin() );

      for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
      {
         FillHitInfo( fvmEpSupprBuffer[ fuGet4Nr ][ iMsgIdx ] );
	 LOG(DEBUG1) << "Done with Get4Nr "<<fuGet4Nr
		     << " MsgId "<<iMsgIdx<<" in epoch "<<ulEpochNr<<FairLogger::endl;
      } // for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )

      fvmEpSupprBuffer[fuGet4Nr].clear();
   } // if( 0 < fvmEpSupprBuffer[fGet4Nr] )

}

void CbmMcbm2018TofUnpacker::PrintSlcInfo(gdpbv100::Message mess)
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
}

void CbmMcbm2018TofUnpacker::PrintGenInfo(gdpbv100::Message mess)
{
   Int_t mType = mess.getMessageType();
   Int_t channel = mess.getGdpbHitChanId();
   uint64_t uData = mess.getData();

   LOG(DEBUG) << "Get4 MSG type " << mType << " from gdpbId " << fuGdpbId
              << ", getId " << fuGet4Id << ", (hit channel) " << channel
              << " data " << std::hex << uData
              << FairLogger::endl;
}

void CbmMcbm2018TofUnpacker::PrintSysInfo(gdpbv100::Message mess)
{
   if (fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find(fuGdpbId))
      LOG(DEBUG) << "GET4 System message,       epoch "
                 << static_cast<Int_t>(fvulCurrentEpoch[fuGet4Nr]) << ", time " << std::setprecision(9)
                 << std::fixed << Double_t(fulCurrentEpochTime) * 1.e-9 << " s "
                 << " for board ID " << std::hex << std::setw(4) << fuGdpbId
                 << std::dec << FairLogger::endl;

   switch( mess.getGdpbSysSubType() )
   {
      case gdpbv100::SYS_GET4_ERROR:
      {
         uint32_t uData = mess.getGdpbSysErrData();
         if( gdpbv100::GET4_V2X_ERR_TOT_OVERWRT == uData
          || gdpbv100::GET4_V2X_ERR_TOT_RANGE   == uData
          || gdpbv100::GET4_V2X_ERR_EVT_DISCARD == uData
          || gdpbv100::GET4_V2X_ERR_ADD_RIS_EDG == uData
          || gdpbv100::GET4_V2X_ERR_UNPAIR_FALL == uData
          || gdpbv100::GET4_V2X_ERR_SEQUENCE_ER == uData
           )
            LOG(DEBUG) << " +++++++ > gDPB: " << std::hex << std::setw(4) << fuGdpbId
                       << std::dec << ", Chip = " << std::setw(2)
                       << mess.getGdpbGenChipId() << ", Chan = " << std::setw(1)
                       << mess.getGdpbSysErrChanId() << ", Edge = "
                       << std::setw(1) << mess.getGdpbSysErrEdge() << ", Empt = "
                       << std::setw(1) << mess.getGdpbSysErrUnused()
                       << ", Data = " << std::hex << std::setw(2) << uData
                       << std::dec << " -- GET4 V1 Error Event"
                       << FairLogger::endl;
            else LOG(DEBUG) << " +++++++ >gDPB: " << std::hex << std::setw(4) << fuGdpbId
                            << std::dec << ", Chip = " << std::setw(2)
                            << mess.getGdpbGenChipId() << ", Chan = " << std::setw(1)
                            << mess.getGdpbSysErrChanId() << ", Edge = "
                            << std::setw(1) << mess.getGdpbSysErrEdge() << ", Empt = "
                            << std::setw(1) << mess.getGdpbSysErrUnused()
                            << ", Data = " << std::hex << std::setw(2) << uData
                            << std::dec << " -- GET4 V1 Error Event "
                            << FairLogger::endl;
         break;
  
      } // case gdpbv100::SYSMSG_GET4_EVENT
      case gdpbv100::SYS_GDPB_UNKWN:
      {
         LOG(DEBUG) << "Unknown GET4 message, data: " << std::hex << std::setw(8)
                    << mess.getGdpbSysUnkwData() << std::dec
                    <<" Full message: " << std::hex << std::setw(16)
                    << mess.getData() << std::dec
                    << FairLogger::endl;
         break;
      } // case gdpbv100::SYS_GDPB_UNKWN:
      case gdpbv100::SYS_SYNC_ERROR:
      {
         LOG(DEBUG) << "Closy synchronization error" << FairLogger::endl;
         break;
      } // case gdpbv100::SYS_SYNC_ERROR:
   } // switch( getGdpbSysSubType() )

}

void CbmMcbm2018TofUnpacker::Reset()
{
  //  fFiberHodoRaw->Clear();
  fTofDigi->Clear();
}

void CbmMcbm2018TofUnpacker::Finish()
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
   fhRawTDigEvT0->Write();
   fHM->H1( Form("Raw_TDig-Ref0") )->Write();
   fHM->H1( Form("Raw_TDig-Ref") )->Write();
   fHM->H1( Form("Raw_TRef-Dig0") )->Write();
   fHM->H1( Form("Raw_TRef-Dig1") )->Write();
   fHM->H1( Form("Raw_Digi-LastDigi") )->Write();
   for( UInt_t uGdpb = 0; uGdpb < fuNrOfGdpbs; uGdpb ++)
   {
      fHM->H2( Form("Raw_Tot_gDPB_%02u", uGdpb) )->Write();
      fHM->H1( Form("ChCount_gDPB_%02u", uGdpb) )->Write();
      fhChanCoinc[ uGdpb ]->Write();
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   gDirectory->cd("..");

}

static Double_t dLastDigiTime=0.;
void CbmMcbm2018TofUnpacker::FillOutput(CbmDigi* digi)
{
  //   if( 100 > iMess++ )
      LOG(DEBUG) << "Fill digi TClonesarray with "
                 << Form("0x%08x", digi->GetAddress())
                 << " at " << static_cast<Int_t>( fTofDigi->GetEntriesFast() )
                 << FairLogger::endl;


   new( (*fTofDigi)[ fTofDigi->GetEntriesFast() ] )
      CbmTofDigiExp( *( dynamic_cast<CbmTofDigiExp*>(digi) ) );
   //CbmTofDigiExp((CbmTofDigiExp *)digi);

   if( 1 == fTofDigi->GetEntriesFast())
      fdEvTime0=digi->GetTime();
      else fhRawTDigEvT0->Fill( digi->GetTime() - fdEvTime0 );

   if( (digi->GetAddress() & DetMask) != 0x00005006 )
   {
     fhRawTDigRef0->Fill( digi->GetTime() - fdRefTime);
     fhRawTDigRef->Fill( digi->GetTime() - fdRefTime);

     Double_t dDigiTime = digi->GetTime();
     fhRawDigiLastDigi->Fill( dDigiTime - dLastDigiTime );
     dLastDigiTime = dDigiTime;
   } // if( (digi->GetAddress() & DetMask) != 0x00005006 )
   else  fdRefTime = digi->GetTime();

   digi->Delete();
}

static    ULong64_t fulGdpbTsMsb;
static    ULong64_t fulGdpbTsLsb;
static    ULong64_t fulStarTsMsb;
static    ULong64_t fulStarTsMid;
static    ULong64_t fulGdpbTsFullLast;
static    ULong64_t fulStarTsFullLast;
static    UInt_t    fuStarTokenLast;
static    UInt_t    fuStarDaqCmdLast;
static    UInt_t    fuStarTrigCmdLast;

void CbmMcbm2018TofUnpacker::FillStarTrigInfo(gdpbv100::Message mess)
{
  Int_t iMsgIndex = mess.getStarTrigMsgIndex();

  //mess.printDataCout();

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
/*
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
*/
         fulGdpbTsFullLast = ulNewGdpbTsFull;
         fulStarTsFullLast = ulNewStarTsFull;
         fuStarTokenLast   = uNewToken;
         fuStarDaqCmdLast  = uNewDaqCmd;
         fuStarTrigCmdLast = uNewTrigCmd;

         Double_t dTot = 1.;
         Double_t dTime = fulGdpbTsFullLast * 6.25;
         if( 0. == fdFirstDigiTimeDif && 0. != fdLastDigiTime )
         {
            fdFirstDigiTimeDif = dTime - fdLastDigiTime;
            LOG(INFO) << "Reference fake digi time shift initialized to " << fdFirstDigiTimeDif
		      << ", default: " << fdTShiftRef
                      <<FairLogger::endl;
         } // if( 0. == fdFirstDigiTimeDif && 0. != fdLastDigiTime )

	 //         dTime -= fdFirstDigiTimeDif;
         // dTime += fdTShiftRef;

         LOG(DEBUG) << "Insert fake digi with time " << dTime << ", Tot " << dTot
                    << FairLogger::endl;
         fhRawTRefDig0->Fill( dTime - fdLastDigiTime);
         fhRawTRefDig1->Fill( dTime - fdLastDigiTime);

         fDigi = new CbmTofDigiExp(0x00005006, dTime, dTot); // fake start counter signal
         fBuffer->InsertData(fDigi);
         break;
	  } // case 3
      default:
         LOG(ERROR) << "Unknown Star Trigger messageindex: " << iMsgIndex << FairLogger::endl;
  } // switch( iMsgIndex )
}

ClassImp(CbmMcbm2018TofUnpacker)
