// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmMcbm2018MuchUnpacker                    -----
// -----               Modified 13.12.2018 by V. Singhal                   -----
// -----               Modified 26.06.2018 by V. Singhal and A. Kumar      -----
// -----               Created 11.11.2016 by V. Singhal and A. Kumar       -----
// -----                                                                   -----
// -----------------------------------------------------------------------------
#include "CbmMcbm2018MuchUnpacker.h"
#include "CbmMcbm2018MuchPar.h"
#include "CbmTbDaqBuffer.h"
#include "CbmMuchAddress.h"
#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "TClonesArray.h"
#include "TString.h"

#include <iostream>
#include <stdint.h>
#include <iomanip>

// C/C++
#include <vector>
#include <set>
#include <map>


class CbmMuchAddress;
using std::hex;
using std::dec;
using namespace std;

CbmMcbm2018MuchUnpacker::CbmMcbm2018MuchUnpacker()
  : CbmTSUnpack(),
   fuOverlapMsNb(0),
   fuNrOfDpbs(0),
   fDpbIdIndexMap(),
   fuNbElinksPerDpb(0),
   fuNbStsXyters(0),
   fuNbChanPerAsic(0),
   fvuElinkToAsic(),
   fbPrintMessages( kFALSE ),
   fPrintMessCtrl( stsxyter::MessagePrintMask::msg_print_Human ),
   fulCurrentTsIdx( 0 ),
   fulCurrentMsIdx( 0 ),
   fmMsgCounter(),
   fuCurrentEquipmentId(0),
   fuCurrDpbId(0),
   fuCurrDpbIdx(0),
   fvulCurrentTsMsb(),
   fvuCurrentTsMsbCycle(),
   fvulChanLastHitTime(),
   fvdChanLastHitTime(),
   fvdMsTime(),
   fvuChanNbHitsInMs(),
   fvdChanLastHitTimeInMs(),
   fvusChanLastHitAdcInMs(),
//   fvmChanHitsInTs(),
   fdStartTime(-1.0),
   fdStartTimeMsSz(-1.0),
   fuMaxNbMicroslices(100),
    //fMsgCounter(11,0), // length of enum MessageTypes initialized with 0
    //fNdpbIdIndexMapA(),
    //fNdpbIdIndexMapB(),
    //fMuchStationMapX(),
    //fMuchStationMapY(),
    //fCurrentEpoch(),
    //fNofEpochs(0),
    //fCurrentEpochTime(0.),
    //fdStartTime( -1 ),
    //fEquipmentId(0),
    fMuchRaw(new TClonesArray("CbmNxyterRawMessageForMUCH", 10)),
    fMuchBeamDigi(new TClonesArray("CbmMuchBeamTimeDigi", 10)),
    fMuchDigi(new TClonesArray("CbmMuchDigi", 10)),
    //fRawMessage(NULL),  
    fDigi(NULL),
    fBuffer(CbmTbDaqBuffer::Instance()),
    fCreateRawMessage(kFALSE),
    fUnpackPar(NULL)
{
}

CbmMcbm2018MuchUnpacker::~CbmMcbm2018MuchUnpacker()
{
}

Bool_t CbmMcbm2018MuchUnpacker::Init()
{
  LOG(INFO) << "Initializing flib nxyter unpacker for MUCH" << FairLogger::endl;
  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }
  //Commented due to (Error in <TTree::Bronch>: TClonesArray with no class defined in branch: cbmout.MUCH raw data.MuchRawMessage)
  
  if (fCreateRawMessage) {
    ioman->Register("MuchRawMessage", "MUCH raw data", fMuchRaw, kTRUE);
  }
  ioman->Register("MuchBeamDigi", "Stsxyter based Much Beam digi", fMuchBeamDigi, kTRUE);
  ioman->Register("MuchDigi", "Global Much digi", fMuchDigi, kTRUE);
  
  //Bellow line is for creating ASCII file
  //std::cout << "AFCK Nu" << "   " << "FEB Nu" 
  //	    << "  " << "Channel"  << "   " << "ADC Value" <<"  " << "Absolute Time" << "  " << " Sector X " << "  Channel Y" << std::endl;
  
  //  CreateHistograms();
  return kTRUE;
}

void CbmMcbm2018MuchUnpacker::SetParContainers()
{
  LOG(INFO) << "Setting parameter containers for " << GetName()
	    << FairLogger::endl;
  fUnpackPar = (CbmMcbm2018MuchPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmMcbm2018MuchPar"));
}


Bool_t CbmMcbm2018MuchUnpacker::InitContainers()
{
  LOG(INFO) << "Init parameter containers for " << GetName()
	    << FairLogger::endl;
  return ReInitContainers();
  
}

Bool_t CbmMcbm2018MuchUnpacker::ReInitContainers()
{
   LOG(INFO) << "ReInit parameter containers for " << GetName()
         << FairLogger::endl;
   fuNrOfDpbs       = fUnpackPar->GetNrOfDpbs();
   fuNbElinksPerDpb = fUnpackPar->GetNbElinksPerDpb();
   fuNbStsXyters    = fUnpackPar->GetNbStsXyters();
   fuNbChanPerAsic  = fUnpackPar->GetNbChanPerAsic();

   LOG(INFO) << "Nr. of GEM DPBs:       " << fuNrOfDpbs
             << FairLogger::endl;

   fDpbIdIndexMap.clear();
   fvuElinkToAsic.resize( fuNrOfDpbs );
   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      fDpbIdIndexMap[ fUnpackPar->GetDpbId( uDpb )  ] = uDpb;
      LOG(INFO) << "Eq. ID for DPB #" << std::setw(2) << uDpb << " = "
                << std::setw(4) << std::hex << fUnpackPar->GetDpbId( uDpb )
                << std::dec
                << " => " << fDpbIdIndexMap[ fUnpackPar->GetDpbId( uDpb )  ]
                << FairLogger::endl;
      fvuElinkToAsic[uDpb].resize( fuNbElinksPerDpb );
      for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
         fvuElinkToAsic[uDpb][uElink] = fUnpackPar->GetElinkToAsicIdx( uDpb * fuNbElinksPerDpb + uElink );
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
  LOG(INFO) << "Nr. eLinks per DPB:    " << fuNbElinksPerDpb
             << FairLogger::endl;
   LOG(INFO) << "Nr. of StsXyter ASICs: " << fuNbStsXyters
             << FairLogger::endl;
   LOG(INFO) << "Nb. channels per ASIC: " << fuNbChanPerAsic
             << FairLogger::endl;

   for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   {
      LOG(INFO) << "ASIC ID for eLinks in DPB #" << std::setw(2) << uDpb << ": ";

      for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
      {
         if( 0 == uElink % 10 )
            LOG(INFO) << FairLogger::endl
                      << "------> ";

         LOG(INFO) << std::setw( 5 ) << fvuElinkToAsic[uDpb][uElink] << " ";
      } // for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
     LOG(INFO) << FairLogger::endl;
   } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )
   
   
   // Check below Variables 
   // Internal status initialization
   fvulCurrentTsMsb.resize( fuNrOfDpbs ); //<Vector of Dpbs < vector of elinks>>
   fvuCurrentTsMsbCycle.resize( fuNrOfDpbs );//TsMsbCycle 
   //fvuCurrentTsMsbOver.resize( fuNrOfDpbs ); // Overlab bits 
   //fvuElinkLastTsHit.resize( fuNrOfDpbs ); // TS from last hit <Vector of Dpbs<vector of elinks>>
   /*for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb ) //Looping over all DPBs
   {
     fvulCurrentTsMsb[uDpb].resize( fuNbElinksPerDpb );
     fvuCurrentTsMsbCycle[uDpb].resize( fuNbElinksPerDpb );
      //fvuCurrentTsMsbOver[uDpb].resize( fuNbElinksPerDpb );
      //fvuElinkLastTsHit[uDpb].resize( fuNbElinksPerDpb );
      for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink) // Looping over all elinks in DPBs
      {
         fvulCurrentTsMsb[uDpb][uElink]      = 0;
         fvuCurrentTsMsbCycle[uDpb][uElink] = 0;
         //fvuCurrentTsMsbOver[uDpb][uElink]  = 0;
         //fvuElinkLastTsHit[uDpb][uElink]  = 0;
      } // for( UInt_t uElink = 0; uElink < fuNbElinksPerDpb; ++uElink )
      } // for( UInt_t uDpb = 0; uDpb < fuNrOfDpbs; ++uDpb )*/
   
   fvulChanLastHitTime.resize( fuNbStsXyters );
   fvdChanLastHitTime.resize( fuNbStsXyters );
   fvuChanNbHitsInMs.resize( fuNbStsXyters );
   fvdChanLastHitTimeInMs.resize( fuNbStsXyters );
   fvusChanLastHitAdcInMs.resize( fuNbStsXyters );
   //fvmChanHitsInTs.resize( fuNbStsXyters );
   for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   {
      fvulChanLastHitTime[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvdChanLastHitTime[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvuChanNbHitsInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvdChanLastHitTimeInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
      fvusChanLastHitAdcInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
      //fvmChanHitsInTs[ uXyterIdx ].resize( fuNbChanPerAsic );
      for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
      {
         fvulChanLastHitTime[ uXyterIdx ][ uChan ] = 0;
         fvdChanLastHitTime[ uXyterIdx ][ uChan ] = -1.0;

         //fvuChanNbHitsInMs[ uXyterIdx ][ uChan ].resize( fuPulserMaxNbMicroslices );
         //fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ].resize( fuPulserMaxNbMicroslices );
         //fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ].resize( fuPulserMaxNbMicroslices );
         //fvmChanHitsInTs[ uXyterIdx ][ uChan ].clear();
	 /* for( UInt_t uMsIdx = 0; uMsIdx < fuPulserMaxNbMicroslices; ++uMsIdx )
         {
            fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
            fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = -1.0;
            fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
	    } */// for( UInt_t uMsIdx = 0; uMsIdx < fuPulserMaxNbMicroslices; ++uMsIdx )
      } // for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
   } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
   LOG(INFO) << "CbmMcbm2018MuchUnpacker::ReInitContainers => Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                << FairLogger::endl;
   LOG(INFO) << "CbmMcbm2018MuchUnpacker::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
                << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fuNbChanPerAsic
                << FairLogger::endl;
   // LOG(INFO) << "CbmMcbm2018MuchUnpacker::ReInitContainers =>  Changed fvuChanNbHitsInMs size "
   //             << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuPulserMaxNbMicroslices
   //             << FairLogger::endl;
   
        for (Int_t febId = 0 ; febId<fuNbStsXyters; febId++){// looping on all the FEB IDs
                for (Int_t channelId=0; channelId<fuNbChanPerAsic; channelId++){
                        //Have to check GetPadX and GetPadY values.
                        LOG(INFO) << "Value of GetPadX " <<         fUnpackPar->GetPadX(febId,channelId) << FairLogger::endl;
                        fMuchStationMapX[febId][channelId] = fUnpackPar->GetPadX(febId,channelId);
                        LOG(INFO) << "Value of GetPadY " <<         fUnpackPar->GetPadY(febId,channelId) <<FairLogger::endl;
                        fMuchStationMapY[febId][channelId] = fUnpackPar->GetPadY(febId,channelId);
                }
        }

   return kTRUE;

}
//----------------------------------------------------------------//

Bool_t CbmMcbm2018MuchUnpacker::DoUnpack(const fles::Timeslice& ts, size_t component)
{
  LOG(DEBUG) << "Timeslice " << ts.index() <<" contains " << ts.num_microslices(component)
             << " microslices and components " << ts.num_components() << FairLogger::endl;
  LOG(DEBUG) << "Processing component number " << component << FairLogger::endl;


  //Int_t messageType = -111;
  // Loop over microslices
  size_t numCompMsInTs = ts.num_microslices(component);//Number of Microslices in TimeSlice
  // Below code copied from CbmVeccSetupMonitorGem

  fulCurrentTsIdx = ts.index();
  //Check purpose Bellow if loop
  /* if( fuMaxNbMicroslices < numCompMsInTs )
   {
      fuMaxNbMicroslices = numCompMsInTs;

      fvdMsTime.resize( fuMaxNbMicroslices );
      fvuChanNbHitsInMs.resize( fuNbStsXyters );
      fvdChanLastHitTimeInMs.resize( fuNbStsXyters );
      fvusChanLastHitAdcInMs.resize( fuNbStsXyters );
      for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      {
         fvuChanNbHitsInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
         fvdChanLastHitTimeInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
         fvusChanLastHitAdcInMs[ uXyterIdx ].resize( fuNbChanPerAsic );
         for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
         {
            fvuChanNbHitsInMs[ uXyterIdx ][ uChan ].resize( fuMaxNbMicroslices );
            fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ].resize( fuMaxNbMicroslices );
            fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ].resize( fuMaxNbMicroslices );
            for( UInt_t uMsIdx = 0; uMsIdx < fuMaxNbMicroslices; ++uMsIdx )
            {
               fvuChanNbHitsInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
               fvdChanLastHitTimeInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = -1.0;
               fvusChanLastHitAdcInMs[ uXyterIdx ][ uChan ][ uMsIdx ] = 0;
            } // for( UInt_t uMsIdx = 0; uMsIdx < fuMaxNbMicroslices; ++uMsIdx )
         } // for( UInt_t uChan = 0; uChan < fuNbChanPerAsic; ++uChan )
      } // for( UInt_t uXyterIdx = 0; uXyterIdx < fuNbStsXyters; ++uXyterIdx )
      LOG(INFO) << "CbmMcbm2018MuchUnpacker::DoUnpack => Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs.size() << " VS " << fuNbStsXyters
                   << FairLogger::endl;
      LOG(INFO) << "CbmMcbm2018MuchUnpacker::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ].size() << " VS " << fuNbChanPerAsic
                   << FairLogger::endl;
      LOG(INFO) << "CbmMcbm2018MuchUnpacker::DoUnpack =>  Changed fvuChanNbHitsInMs size "
                   << fvuChanNbHitsInMs[ 0 ][ 0 ].size() << " VS " << fuMaxNbMicroslices
                   << FairLogger::endl;
   } // if( fuMaxNbMicroslices < numCompMsInTs )
  */
  for( size_t m = 0; m < numCompMsInTs; ++m )
    {
      // Ignore overlap ms if number defined by user
      if( numCompMsInTs - fuOverlapMsNb <= m ){
	LOG(DEBUG) << "Timeslice " << ts.index() <<" contain " << ts.num_microslices(component)
		   << " microslices and skiping overlap microslice number is "<< m << FairLogger::endl;
	continue;}
      
      auto msDescriptor = ts.descriptor(component, m);
      fuCurrentEquipmentId = msDescriptor.eq_id; //Equipment Identifier (16 bit Unsigned Int)
      const uint8_t* msContent = reinterpret_cast<const uint8_t*>(ts.content(component, m));
      
      uint32_t size = msDescriptor.size;//Content Size
      fulCurrentMsIdx = msDescriptor.idx;//Microslice Idx is microslice start time 
      LOG(DEBUG) << "Microslice number (time in ns): " << fulCurrentMsIdx
                 << " from EqId " << std::hex << fuCurrentEquipmentId << std::dec
                 << " has content (size in bytes): " << size << FairLogger::endl;
      Double_t dMsTime = (1e-9) * static_cast<double>(fulCurrentMsIdx);//Time in Second
      
      fuCurrDpbId  = static_cast< uint32_t >( fuCurrentEquipmentId & 0xFFFF ); //Converted into 32 bit AFCK id
      fuCurrDpbIdx = fDpbIdIndexMap[ fuCurrDpbId ]; //taken from *.PAR file
      
      if( component < kiMaxNbFlibLinks )
	{
	  if( fdStartTimeMsSz < 0 )
            fdStartTimeMsSz = dMsTime; //Microslice start time in second.
	  //fhMsSz[ component ]->Fill( size );
	  //fhMsSzTime[ component ]->Fill( (1e-9) * static_cast<double>( msDescriptor.idx) - fdStartTimeMsSz, size);
	} // if( component < kiMaxNbFlibLinks )
      
      // If not integer number of message in input buffer, print warning/error
      if( 0 != (size % kuBytesPerMessage) )
	LOG(ERROR) << "The input microslice buffer does NOT "
		   << "contain complete nDPB messages! (only complete messages allowed.) "
		   << size % kuBytesPerMessage << " bytes are extra in this microslice. "
		   << FairLogger::endl;
      // Compute the number of complete messages in the input microslice buffer
      uint32_t uNbMessages = (size - (size % kuBytesPerMessage) )
	/ kuBytesPerMessage;
      
      // Prepare variables for the loop on contents
      const uint32_t* pInBuff = reinterpret_cast<const uint32_t*>( msContent );//Converted 8 bit content into 32 bit
      for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )
	{
	  // Fill message
	  uint32_t ulData = static_cast<uint32_t>(pInBuff[uIdx]);
	  stsxyter::Message mess( static_cast< uint32_t >( ulData & 0xFFFFFFFF ) );//Message is from FEE stsxyter chip.
	  // Print message if requested
	  if( fbPrintMessages )
            mess.PrintMess( std::cout, fPrintMessCtrl );
	  stsxyter::MessType typeMess = mess.GetMessType();
	  fmMsgCounter[ typeMess ] ++;
	  //fhStsMessType->Fill( static_cast< uint16_t > (typeMess) );
	  //fhStsMessTypePerDpb->Fill( fuCurrDpbIdx, static_cast< uint16_t > (typeMess) );
	  //fhStsMessTypePerElink->Fill( fuCurrDpbIdx * fuNbElinksPerDpb + usElinkIdx,
	  //                             static_cast< uint16_t > (typeMess) );
	  
	  switch( typeMess )
	    {
            case stsxyter::MessType::Hit :
	      {
         	// Extract the eLink and Asic indices 
		UShort_t usElinkIdx = mess.GetLinkIndex();//if Hit message 9 bit for link Index
	        if( fuNbElinksPerDpb <= usElinkIdx )//Checking LinkIndex value with written in .par file
		  {
	            LOG(FATAL) << "CbmMcbm2018MuchUnpacker::DoUnpack => "
			       << "eLink index out of bounds! " 
			       << "eLink in Par file "<< fuNbElinksPerDpb 
			       <<" and in Message :" << usElinkIdx 
			       << FairLogger::endl;
		  } // if( fuNbElinksPerDpb <= usElinkIdx )
	        UInt_t   uAsicIdx   = fvuElinkToAsic[fuCurrDpbIdx][usElinkIdx];//DPB id from elink
		LOG(DEBUG) << " FillHitInfo() called " << FairLogger::endl;
	        FillHitInfo( mess, usElinkIdx, uAsicIdx, m );
		break;
	      } // case stsxyter::MessType::Hit :
            case stsxyter::MessType::TsMsb :
	      {
		FillTsMsbInfo( mess, uIdx, m );
		break;
	      } // case stsxyter::MessType::TsMsb :
	      
	    case stsxyter::MessType::Epoch :
	      {
		// The first message in the TS is a special ones: EPOCH
		FillEpochInfo( mess );
		if( 0 < uIdx )
                  LOG(INFO) << "CbmMcbm2018MuchUnpacker::DoUnpack => "
                            << "EPOCH message at unexpected position in MS: message "
                            << uIdx << " VS message 0 expected!"
                            << FairLogger::endl;
		break;
	      } // case stsxyter::MessType::Epoch :
            case stsxyter::MessType::Empty :
	      {
		//               FillTsMsbInfo( mess );
		break;
	      } // case stsxyter::MessType::Empty :
	      
	    case stsxyter::MessType::Dummy :
	      {
		/*if( kTRUE == fbBetaFormat )
		  {
                  // In beta data format, a dummy hit indicates a TS overflow => TS_MSB increase by 1
                  fvuCurrentTsMsb[fuCurrDpbIdx][0] ++;
                  fvuElinkLastTsHit[fuCurrDpbIdx][0] = 0;
		  } // if( kTRUE == fbBetaFormat )*/
		break;
	      } // case stsxyter::MessType::Dummy / ReadDataAck / Ack :
            default:
	      {
		fulUnknownMessage++;
		LOG(ERROR) << "CbmVeccSetupMonitorGem::DoUnpack => "
			   << "Unknown message type, should never happen, Error!"
			   << "Till now " << fulUnknownMessage <<" Unknown Message "	
			   << FairLogger::endl;
	      }
	    } // switch( mess.GetMessType() )
	} // for( uint32_t uIdx = 0; uIdx < uNbMessages; ++uIdx )
    } // for( size_t m = 0; m < numCompMsInTs; ++m )
  
  
  return kTRUE;
}
//--------------------------------------------------------------------//

//void CbmTSUnpackMuch::FillHitInfo(ngdpb::Message mess){
void CbmMcbm2018MuchUnpacker::FillHitInfo( stsxyter::Message mess, const UShort_t & usElinkIdx, const UInt_t & uAsicIdx, const UInt_t & uMsIdx )
{
 
   // nxyterId of CERN2016 --> uAsicIdx
   UShort_t usChan   = mess.GetHitChannel();    //nxChannel of CERN2016
   UShort_t usRawAdc = mess.GetHitAdc();	// charge of CERN2016
   ULong64_t ulFullTs = mess.GetHitTimeFull();   // hitTime of CERN2016
   UShort_t usTsOver = mess.GetHitTimeOver();   
   UShort_t usRawTs  = mess.GetHitTime();       // 
   // For generating MUCH ADDRESS 
   Short_t station = 0; // for mCBM setup
   Short_t layer = 0; // for time being 0 but will have 3 layer in mCBM
   Short_t layerside = 0; // 0 in mCBM
   Short_t module = 0; // 0 in mCBM
   Short_t sector  = fUnpackPar->GetPadX(uAsicIdx, usChan);
   Short_t channel = fUnpackPar->GetPadY(uAsicIdx, usChan);
   if(sector<0 || channel<0)
   {
      LOG(DEBUG) << "Mapping: FebId " << uAsicIdx << " and Channel Id "
                 << usChan << " corresponding sector " << sector <<" channel " << channel
                 << FairLogger::endl;
   }
   //Creating Unique address of the particular channel of GEM  
   UInt_t address = CbmMuchAddress::GetAddress(station, layer, layerside, module, sector, channel);
   LOG(DEBUG) << " Generated MUCH address " << address <<" for sector number " <<sector << " and channel " << channel << FairLogger::endl;

    // Compute the Full time stamp
   //   Long64_t ulOldHitTime = fvulChanLastHitTime[ uAsicIdx ][ usChan ];
   

   // Use TS w/o overlap bits as they will anyway come from the TS_MSB
   //fvulChanLastHitTime[ uAsicIdx ][ usChan ] = usRawTs;
   LOG(DEBUG) << " fvulChanLastHitTime[ uAsicIdx ][ usChan ] "<<fvulChanLastHitTime[ uAsicIdx ][ usChan ] << FairLogger::endl;
   fvulChanLastHitTime[ uAsicIdx ][ usChan ] = static_cast<ULong64_t>(usRawTs) +
               static_cast<ULong64_t>( stsxyter::kuHitNbTsBins )
             * static_cast<ULong64_t>( fvulCurrentTsMsb[fuCurrDpbIdx])
             + static_cast<ULong64_t>( stsxyter::kuTsCycleNbBins )
             * static_cast<ULong64_t>( fvuCurrentTsMsbCycle[fuCurrDpbIdx] )
             ;
   LOG(DEBUG) << " stsxyter::kuHitNbTsBins "<<stsxyter::kuHitNbTsBins<<" fvulCurrentTsMsb[fuCurrDpbIdx] "<<fvulCurrentTsMsb[fuCurrDpbIdx]<<" stsxyter::kuTsCycleNbBins "<<stsxyter::kuTsCycleNbBins <<" fvuCurrentTsMsbCycle[fuCurrDpbIdx] "<<fvuCurrentTsMsbCycle[fuCurrDpbIdx]<<FairLogger::endl;
   
   LOG(DEBUG) << " fvulChanLastHitTime[ uAsicIdx ][ usChan ] "<<fvulChanLastHitTime[ uAsicIdx ][ usChan ] << FairLogger::endl;

   //   fvuElinkLastTsHit[fuCurrDpbIdx] = usRawTs;
   // Convert the Hit time in bins to Hit time in ns
   Double_t dHitTimeNs = fvulChanLastHitTime[ uAsicIdx ][ usChan ] * stsxyter::kdClockCycleNs;
   fvdChanLastHitTime[ uAsicIdx ][ usChan ] = fvulChanLastHitTime[ uAsicIdx ][ usChan ] * stsxyter::kdClockCycleNs;
   LOG(DEBUG) << " fvdChanLastHitTime[ uAsicIdx ][ usChan ] "<<fvdChanLastHitTime[ uAsicIdx ][ usChan ] << FairLogger::endl;

   Double_t FullHitTime  = fvdChanLastHitTime[ uAsicIdx ][ usChan ];
  //LOG(DEBUG) << "Create digi with time " << hitTime
   //	       << " at epoch " << fCurrentEpoch[rocId][nxyterId] << FairLogger::endl;

   LOG(DEBUG) << " Last MUCH Hit Time (global) " <<  FullHitTime << "ns stored in Double." <<FairLogger::endl;
   LOG(DEBUG) << " Current MUCH Hit Time " <<  fvulChanLastHitTime[uAsicIdx][usChan] << "in clock cycle stored unsigned long." <<FairLogger::endl;
   LOG(DEBUG) << " Generated MUCH Hit Time " <<  dHitTimeNs << "ns (stored unsigned long multiplied with clock)." <<FairLogger::endl;
   LOG(DEBUG) << " MUCH Hit Time storing in MuchDigi " <<  FullHitTime  << "ns." <<FairLogger::endl;
  
   // --- Create digi
   fDigi = new CbmMuchBeamTimeDigi(address, usRawAdc, FullHitTime);
   fDigi->SetPadX(sector);
   fDigi->SetPadY(channel);
   fDigi->SetRocId(uAsicIdx);// need to check Fill AFCK Hex Number 
   fDigi->SetNxId(uAsicIdx);
   fDigi->SetNxCh(usChan);
   
   
   //LOG(DEBUG) << "MUCH message: rocId " << rocId << " NXYTER " << nxyterId
   //	       << " channel " << nxChannel << " charge " << charge << " time "
   //	     << hitTime << FairLogger::endl;
   
   //Creating digi in CbmMuchBeamtimeDigi format
   //CbmMuchBeamTimeDigi::CbmMuchBeamTimeDigi()
   //			  : CbmDigi(), fData(0), fTime(0), fMatch(0), fPadX(-1), fPadY(-1), fRocId(-1), fNxId(-1), fNxCh(-1)
   
   //if(fDigi = new CbmMuchBeamTimeDigi(0,address, charge, hitTime);
   //Commented due to (Error in <TTree::Bronch>: TClonesArray with no class defined in branch: cbmout.MUCH raw data.MuchRawMessage)
   
   if(!fBuffer) cout << "------------- NULL pointer ------------- " << endl;
   if(!fBuffer) cout << "------------- NULL pointer ------------- " << endl;
   if(!fBuffer) {cout << "------------- NULL pointer ------------- " << endl;return;}
   
   fBuffer->InsertData(fDigi);
   //fBuffer->PrintStatus();
   //( !(fNdpbIdIndexMapA.end() == fNdpbIdIndexMapA.find( rocId )) 
   //LOG(DEBUG) << "Number of Digis stored in fBuffer " << fBuffer->GetEntriesFast() << endl;
   LOG(DEBUG) << "Number of Much Beam Digis created till  now " << fMuchBeamDigi->GetEntriesFast() << endl;
   LOG(DEBUG) << "Number of Much Digis created till  now " << fMuchDigi->GetEntriesFast() << endl;
}//void CbmMcbm2018MuchUnpacker::FillHitInfo(ngdpb::Message mess){
//-----------------------------------------------------------------------//

void CbmMcbm2018MuchUnpacker::FillTsMsbInfo( stsxyter::Message mess, UInt_t uMessIdx, UInt_t uMsIdx )
{
   UInt_t uVal    = mess.GetTsMsbVal();
/*
   if( 0 == fuCurrDpbIdx )
      LOG(INFO) << " TS " << std::setw( 12 ) << fulCurrentTsIdx
                << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                << " MsInTs " << std::setw( 3 ) << uMsIdx
                << " DPB "    << std::setw( 2 ) << fuCurrDpbIdx
                << " Mess "   << std::setw( 5 ) << uMessIdx
                << " TsMsb "  << std::setw( 5 ) << uVal
                << FairLogger::endl;
*/
/*
   if( (uVal != fvulCurrentTsMsb[fuCurrDpbIdx] + 1) && 0 < uVal  &&
       !( 1 == uMessIdx && usVal == fvulCurrentTsMsb[fuCurrDpbIdx] ) ) // 1st TS_MSB in MS is always a repeat of the last one in previous MS!
   {
      LOG(INFO) << "TS MSB not increasing by 1!  TS " << std::setw( 12 ) << fulCurrentTsIdx
                << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                << " MsInTs " << std::setw( 3 ) << uMsIdx
                << " DPB " << std::setw( 2 ) << fuCurrDpbIdx
                << " Mess " << std::setw( 5 ) << uMessIdx
                << " Old TsMsb " << std::setw( 5 ) << fvulCurrentTsMsb[fuCurrDpbIdx]
                << " new TsMsb " << std::setw( 5 ) << uVal
                << " Diff " << std::setw( 5 ) << uVal - fvulCurrentTsMsb[fuCurrDpbIdx]
                << " Old MsbCy " << std::setw( 5 ) << fvuCurrentTsMsbCycle[fuCurrDpbIdx]
                << FairLogger::endl;
   } // if( (uVal != fvulCurrentTsMsb[fuCurrDpbIdx] + 1) && 0 < uVal )
*/

   // Update Status counters
   if( uVal != fvulCurrentTsMsb[fuCurrDpbIdx] ) //Not understood
     fuNbTsMsbSinceLastHit++;


   
   if( uVal < fvulCurrentTsMsb[fuCurrDpbIdx] )// It shows one TS_MSB cycle completed and TS_MSB Cycle counter need to be increamented.
   {

      LOG(INFO) << " TS " << std::setw( 12 ) << fulCurrentTsIdx
                << " MS " << std::setw( 12 ) << fulCurrentMsIdx
                << " DPB " << std::setw( 2 ) << fuCurrDpbIdx
                << " Old TsMsb " << std::setw( 5 ) << fvulCurrentTsMsb[fuCurrDpbIdx]
                << " Old MsbCy " << std::setw( 5 ) << fvuCurrentTsMsbCycle[fuCurrDpbIdx]
                << " new TsMsb " << std::setw ( 5 ) << uVal
                << FairLogger::endl;

      fvuCurrentTsMsbCycle[fuCurrDpbIdx]++;
   } // if( uVal < fvulCurrentTsMsb[fuCurrDpbIdx] )
   fvulCurrentTsMsb[fuCurrDpbIdx] = uVal;
   /*
     if( 1 < uMessIdx )
     {
     fhStsDpbRawTsMsb->Fill( fuCurrDpbIdx,      fvulCurrentTsMsb[fuCurrDpbIdx] );
     fhStsDpbRawTsMsbSx->Fill( fuCurrDpbIdx,  ( fvulCurrentTsMsb[fuCurrDpbIdx] & 0x1F ) );
     fhStsDpbRawTsMsbDpb->Fill( fuCurrDpbIdx, ( fvulCurrentTsMsb[fuCurrDpbIdx] >> 5 ) );
     } // if( 0 < uMessIdx )
   
   //   fhStsAsicTsMsb->Fill( fvulCurrentTsMsb[fuCurrDpbIdx], uAsicIdx );
   
   ULong64_t ulNewTsMsbTime =  static_cast< ULong64_t >( stsxyter::kuHitNbTsBins )
     * static_cast< ULong64_t >( fvulCurrentTsMsb[fuCurrDpbIdx])
     + static_cast< ULong64_t >( stsxyter::kuTsCycleNbBins )
     * static_cast< ULong64_t >( fvuCurrentTsMsbCycle[fuCurrDpbIdx] );
     */
}

/*
  
  Int_t CbmMcbm2018MuchUnpacker::CreateAddress(Int_t rocId, Int_t febId, Int_t stationId, Int_t layerId, Int_t sideId, Int_t moduleId, Int_t channelId)
  {
  
  Int_t SysFebNum = fNdpbIdIndexMap[rocId]*fUnpackPar->GetNrOfFebsPerNdpb() + febId;
  //Below if condition is due to FEB connected in reverse direction due to FLEX cable.
	if (SysFebNum==7||SysFebNum==8||SysFebNum==9||SysFebNum==10||SysFebNum==11){
		channelId=127-channelId;
	}
		
	//converted SysFebNum into the febNr as per the MuchUnpackPar.par file
	Int_t febNr = fUnpackPar->GetFebsIdsFromArray(SysFebNum);
	Int_t sector  = fUnpackPar->GetPadX(febNr, channelId);
	Int_t channel = fUnpackPar->GetPadY(febNr, channelId);
	//Bellow line is for creating ASCII file
	//cout << sector << "  " << channel << std::endl;
	Int_t address = CbmMuchAddress::GetAddress(stationId, layerId, sideId, moduleId, sector, channel);
	return address;
}
*/

void CbmMcbm2018MuchUnpacker::FillEpochInfo( stsxyter::Message mess )
{
}


/* need to check
void CbmMcbm2018MuchUnpacker::FillEpochInfo(ngdpb::Message mess)
{
  Int_t rocId          = mess.getRocNumber();
  Int_t nxyterId       = mess.getEpochNxNum();
  
  // First check if nDPB is mapped
  if( fNdpbIdIndexMapA.end() == fNdpbIdIndexMapA.find( rocId ) || fNdpbIdIndexMapB.end() == fNdpbIdIndexMapB.find( rocId ))
    {
      //bellow should be FATAL error, converting it into INFO
      LOG(DEBUG) << "Unmapped nDPB Id " << std::hex << rocId << std::dec << FairLogger::endl;
    } // if( fNdpbIdIndexMap.end() == fNdpbIdIndexMap.find( rocId ) )
  
  fCurrentEpoch[rocId][nxyterId] = mess.getEpochNumber();
  
  //  LOG(INFO) << "Epoch message for ROC " << rocId << " with epoch number "
  //            << fCurrentEpoch[rocId] << FairLogger::endl;
  fCurrentEpochTime = mess.getMsgFullTime(fCurrentEpoch[rocId][nxyterId]);
  fNofEpochs++;
  LOG(DEBUG) << "Epoch message "
             << fNofEpochs << ", epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][nxyterId])
             << ", time " << std::setprecision(9) << std::fixed
             << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
             << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
             << FairLogger::endl;
  
}
*/
void CbmMcbm2018MuchUnpacker::Reset()
{
  if (fCreateRawMessage) {
    fMuchRaw->Clear();
  }
  fMuchBeamDigi->Clear();
  fMuchDigi->Clear();
}

void CbmMcbm2018MuchUnpacker::Finish()
{


  LOG(INFO) << "-------------------------------------" << FairLogger::endl;
  LOG(INFO) << "CbmMcbm2018MuchUnpacker statistics are " << FairLogger::endl;
  LOG(INFO) << " Hit      messages: " << fmMsgCounter[ stsxyter::MessType::Hit   ] << FairLogger::endl
	    << " Ts MSB   messages: " << fmMsgCounter[ stsxyter::MessType::TsMsb ] << FairLogger::endl
	    << " Dummy    messages: " << fmMsgCounter[ stsxyter::MessType::Dummy ] << FairLogger::endl
	    << " Epoch    messages: " << fmMsgCounter[ stsxyter::MessType::Epoch ] << FairLogger::endl
	    << " Empty    messages: " << fmMsgCounter[ stsxyter::MessType::Empty ] << FairLogger::endl
	    << " Unknown  messages: " << fulUnknownMessage << FairLogger::endl;
  
  LOG(INFO) << "-------------------------------------" << FairLogger::endl;
  
  //SaveAllHistos( fsHistoFileFullname );
  //LOG(INFO) << "CbmCosy2018MonitorDupli done saving histos " << FairLogger::endl;
  //LOG(INFO) << "-------------------------------------" << FairLogger::endl;
  
  //   LOG(INFO) << message_type << " messages: " 
  //          << fMsgCounter[i] << FairLogger::endl;

    /*  gDirectory->mkdir("Much_Raw");
	gDirectory->cd("Much_Raw");
	fHM->H2("Raw_ADC_Much")->Write();        
	fHM->H2("Pad_Distribution")->Write();        
	gDirectory->cd("..");
    */
}


void CbmMcbm2018MuchUnpacker::FillOutput(CbmDigi* digi)
{
  
  LOG(DEBUG) << "VS: FillOutput Called and should create a digi array:-" << FairLogger::endl;

  new( (*fMuchBeamDigi)[fMuchBeamDigi->GetEntriesFast()] ) CbmMuchBeamTimeDigi(*(dynamic_cast<CbmMuchBeamTimeDigi*>(digi)));
  new( (*fMuchDigi)[fMuchDigi->GetEntriesFast()] ) CbmMuchDigi(*(dynamic_cast<CbmMuchDigi*>(digi)));
  
  // Delete object pointed by digi after using it
  digi->Delete();
}

ClassImp(CbmMcbm2018MuchUnpacker)
