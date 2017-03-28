// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSUnpackTof                             -----
// -----               Created 27.10.2016 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTSUnpackTof.h"
#include "CbmTofUnpackPar.h"
#include "CbmTofDigiExp.h"

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

const  UInt_t kuNbChanGet4 =  4;
static UInt_t kuNbChanAfck = 128; //96;  // FIXME - should be read from parameter file 
static Int_t iMess=0;
const  Double_t FineTimeConvFactor=0.048828;
static Double_t RefTime=0.;
static Double_t LastDigiTime=0.; 
static Double_t FirstDigiTimeDif=0.; 
const  Int_t DetMask = 0x0001FFFF;


CbmTSUnpackTof::CbmTSUnpackTof( UInt_t uNbGdpb )
  : CbmTSUnpack(),
    fuMsAcceptsPercent(100),
    fuOverlapMsNb(0),
    fuMinNbGdpb( uNbGdpb ),
    fuCurrNbGdpb( 0 ),
    fMsgCounter(11,0), // length of enum MessageTypes initialized with 0
    fGdpbIdIndexMap(),
    fHM(new CbmHistManager()),
    fCurrentEpoch(),
    fNofEpochs(0),
    fCurrentEpochTime(0.),
    fEquipmentId(0),
    fdTShiftRef(0.),
//    fFiberHodoRaw(new TClonesArray("CbmNxyterRawMessage", 10)),
    fTofDigi(),
//    fRawMessage(NULL),  
    fDigi(NULL),
    fBuffer(CbmTbDaqBuffer::Instance()),
    fUnpackPar(NULL),
    fbEpochSuppModeOn(kFALSE),
    fvmEpSupprBuffer()
{
}

CbmTSUnpackTof::~CbmTSUnpackTof()
{
}

Bool_t CbmTSUnpackTof::Init()
{
  LOG(INFO) << "Initializing flib Get4 unpacker" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }

  fTofDigi= new TClonesArray("CbmTofDigiExp", 10);
  if (fTofDigi == NULL) {
    LOG(FATAL) << "No Digi TClonesarray " << FairLogger::endl;
  }
  //  ioman->Register("FiberHodoRawMessage", "fiberhodo raw data", fFiberHodoRaw, kTRUE);
  ioman->Register("CbmTofDigi", "Tof raw Digi", fTofDigi, kTRUE);

  fUnpackPar = (CbmTofUnpackPar*)(FairRun::Instance());

  CreateHistograms();

  return kTRUE;
}

void CbmTSUnpackTof::SetParContainers()
{
  LOG(INFO) << "Setting parameter containers for " << GetName()
	    << FairLogger::endl;
  fUnpackPar = (CbmTofUnpackPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmTofUnpackPar"));

}

Bool_t CbmTSUnpackTof::InitContainers()
{
	LOG(INFO) << "Init parameter containers for " << GetName()
			<< FairLogger::endl;
	return ReInitContainers();
}

Bool_t CbmTSUnpackTof::ReInitContainers()
{
	LOG(INFO) << "ReInit parameter containers for " << GetName()
			<< FairLogger::endl;
	Int_t nrOfRocs = fUnpackPar->GetNrOfRocs();
        kuNbChanAfck = fUnpackPar->GetNrOfFebsPerGdpb()
	             * fUnpackPar->GetNrOfGet4PerFeb()
                     * fUnpackPar->GetNrOfChannelsPerGet4();  
	LOG(INFO) << "Nr. of Tof Rocs: " << nrOfRocs
		  << ", Nr of Channel/Roc: "<< kuNbChanAfck
    		  << FairLogger::endl;
	
	fGdpbIdIndexMap.clear();
	for (Int_t i = 0; i< nrOfRocs; ++i) {
	  fGdpbIdIndexMap[fUnpackPar->GetRocId(i)] = i;
	  LOG(INFO) << "Roc Id of TOF  " << i
			  << " : " << fUnpackPar->GetRocId(i)
			  << FairLogger::endl;
	}
	Int_t NrOfChannels = fUnpackPar->GetNumberOfChannels();
	//Int_t fNrOfGet4    = NrOfChannels/4;
	LOG(INFO) << "Nr. of mapped Tof channels: " << NrOfChannels;
	for (Int_t i = 0; i< NrOfChannels; ++i) {
	  if(i%8 == 0)  LOG(INFO) << FairLogger::endl;
	  LOG(INFO) << Form(" 0x%08x",fUnpackPar->GetChannelToDetUIdMap(i));
	}
	LOG(INFO)  << FairLogger::endl;
	if( fbEpochSuppModeOn )
	{
	  fvmEpSupprBuffer.resize( nrOfRocs );	
	  for (Int_t i = 0; i< nrOfRocs; ++i) {
	    Int_t nrOfGet4 = fUnpackPar->GetNrOfFebsPerGdpb()*fUnpackPar->GetNrOfGet4PerFeb();
	    fvmEpSupprBuffer[i].resize( nrOfGet4 );	
	  }
	}
	return kTRUE;
}

void CbmTSUnpackTof::CreateHistograms()
{
   LOG(INFO) << "create Histos for " << fuMinNbGdpb <<" Rocs "
	    << FairLogger::endl;

     fHM->Add( Form("Raw_TDig-Ref"),
           new TH1F( Form("Raw_TDig-Ref"),
                     Form("Raw digi time difference to Ref ; time [ns]; cts"),
                     5000, 0, 5000) );  

     fHM->Add( Form("Raw_TRef-Dig0"),
           new TH1F( Form("Raw_TRef-Dig0"),
                     Form("Raw Ref time difference to Last digi  ; time [ns]; cts"),
                     9999, -500000000, 500000000) );   
     fHM->Add( Form("Raw_TRef-Dig1"),
           new TH1F( Form("Raw_TRef-Dig1"),
                     Form("Raw Ref time difference to Last digi  ; time [ns]; cts"),
                     9999, -5000000, 5000000) );   

   for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   {
      fHM->Add( Form("Raw_Tot_gDPB_%02u", uGdpb),
           new TH2F( Form("Raw_Tot_gDPB_%02u", uGdpb),
                     Form("Raw TOT gDPB %02u; channel; TOT [bin]", uGdpb),
                     kuNbChanAfck, 0, kuNbChanAfck, 256, 0, 255) );
      fHM->Add( Form("ChCount_gDPB_%02u", uGdpb),
           new TH1I( Form("ChCount_gDPB_%02u", uGdpb),
                     Form("Channel counts gDPB %02u; channel; Hits", uGdpb),
                     kuNbChanAfck, 0, kuNbChanAfck ) );
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
}

Bool_t CbmTSUnpackTof::DoUnpack(const fles::Timeslice& ts, size_t component)
{

  LOG(DEBUG1) << "Timeslice contains " << ts.num_microslices(component)
              << " microslices." << FairLogger::endl;
  
  // Loop over microslices
  size_t numCompMsInTs = ts.num_microslices(component);
  for (size_t m = 0; m < numCompMsInTs; ++m)
    {
       if( fuMsAcceptsPercent < m )
         continue;

      // Ignore overlap ms if number defined by user
      if( numCompMsInTs - fuOverlapMsNb <= m )
        continue;

      constexpr uint32_t kuBytesPerMessage = 8;

      auto msDescriptor = ts.descriptor(component, m);
      fEquipmentId = msDescriptor.eq_id;
      const uint8_t* msContent = reinterpret_cast<const uint8_t*>(ts.content(component, m));

      uint32_t size = msDescriptor.size;
      if(size>0)
      LOG(DEBUG) << "Microslice: " << msDescriptor.idx 
                << " has size: " << size << FairLogger::endl; 

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
          ngdpb::Message mess( ulData );

          if(gLogger->IsLogNeeded(DEBUG1)) {
            mess.printDataCout();
          }

          // Increment counter for different message types 
          fMsgCounter[mess.getMessageType()]++;
          
          switch(mess.getMessageType()) {
          case ngdpb::MSG_HIT: 
 //           FillHitInfo(mess);
            LOG(ERROR) << "Message type " << mess.getMessageType() 
                       << " not yet included in unpacker."
                       << FairLogger::endl;
            break;
          case ngdpb::MSG_EPOCH:
 //           FillEpochInfo(mess);
            LOG(ERROR) << "Message type " << mess.getMessageType() 
                       << " not yet included in unpacker."
                       << FairLogger::endl;
            break;
          case ngdpb::MSG_EPOCH2:
            FillEpochInfo(mess);
            break;
          case ngdpb::MSG_GET4:
            PrintGenInfo(mess);
            break;
	  case ngdpb::MSG_GET4_32B:
	    if( fbEpochSuppModeOn )
	      {
		Int_t rocId      = mess.getRocNumber();
		Int_t get4Id     = mess.getGdpbGenChipId();
		//		Int_t fGet4Nr    = fGdpbIdIndexMap[rocId]*kuNbChanAfck + get4Id*kuNbChanGet4;
		fvmEpSupprBuffer[fGdpbIdIndexMap[rocId]][get4Id].push_back( mess );
	      }
	    else   FillHitInfo(mess);
            break;
          case ngdpb::MSG_GET4_SLC:
            PrintSlcInfo(mess);
            break;
          case ngdpb::MSG_GET4_SYS:
	    if(100 > iMess++)
            PrintSysInfo(mess);
            break;
	    
	  case ngdpb::MSG_STAR_TRI:
	    FillStarTrigInfo(mess);
	    break;
	    
          default:
	    if(100 > iMess++)
	      LOG(ERROR) << "Message ("<<iMess<<") type " << std::hex << std::setw(2) 
                       << static_cast< uint16_t >( mess.getMessageType() ) 
                       << " not yet included in Get4 unpacker."
                       << FairLogger::endl;
	    if(100 == iMess)
	      LOG(ERROR) << "Stop reporting MSG errors... "
                         << FairLogger::endl;
          }
          
        } // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)
      
    }


  return kTRUE;
}

void CbmTSUnpackTof::FillHitInfo(ngdpb::Message mess)
{
  // --- Get absolute time, NXYTER and channel number
  Int_t rocId      = mess.getRocNumber();
  Int_t get4Id     = mess.getGdpbGenChipId();
  Int_t channel    = mess.getGdpbHitChanId(); 
  Int_t tot        = mess.getGdpbHit32Tot();
  ULong_t hitTime  = mess.getMsgFullTime( 0 );
      
  if( fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find( rocId ) )
  {
    fHM->H2( Form("Raw_Tot_gDPB_%02u", fGdpbIdIndexMap[ rocId ]) )
       ->Fill( get4Id*kuNbChanGet4 + channel, tot);
    fHM->H1( Form("ChCount_gDPB_%02u", fGdpbIdIndexMap[ rocId ]) )
       ->Fill( get4Id*kuNbChanGet4 + channel );

    
    Int_t curEpochGdpbGet4 = fCurrentEpoch[rocId][get4Id];
    if( fbEpochSuppModeOn )
      curEpochGdpbGet4 --; // In Ep. Suppr. Mode, receive following epoch instead of previous
    hitTime  = mess.getMsgFullTime(curEpochGdpbGet4);
      
    Int_t Ft = mess.getGdpbHitFineTs();

    if(100 > iMess++)
    LOG(DEBUG) << "Hit: " << Form("0x%08x ",rocId) << ", " << get4Id 
            << ", " << channel << ", " << tot
	    << ", epoch " << fCurrentEpoch[rocId][get4Id]
	    << ", FullTime " << hitTime 
	    << ", FineTime " << Ft
            << FairLogger::endl;
  
    Int_t iChan = fGdpbIdIndexMap[rocId]*kuNbChanAfck + get4Id*kuNbChanGet4 + channel;
    if (iChan >  fUnpackPar->GetNumberOfChannels()){
      LOG(ERROR) << "Invalid mapping index "<<iChan
		 <<", from " << fGdpbIdIndexMap[rocId]
		 <<", " << get4Id
		 <<", " << channel
		 << FairLogger::endl;
      return;
    }
    Int_t iChanUId = fUnpackPar->GetChannelToDetUIdMap( iChan );
    if(0==iChanUId) return;   // Hit not mapped to digi
    
    Double_t dTime = mess.getMsgFullTimeD( curEpochGdpbGet4 );
    Double_t dTot  = tot;     // in ps ?

    LastDigiTime = dTime;

    if( (iChanUId & DetMask) == 0x00005006 ) dTime += fdTShiftRef;

    LOG(DEBUG) << Form("Insert 0x%08x digi with time ",iChanUId)<< dTime<<", Tot "<<dTot
      //<< " at epoch " << fCurrentEpoch[rocId]
	       << FairLogger::endl;
    fDigi = new CbmTofDigiExp(iChanUId, dTime, dTot);

    fBuffer->InsertData(fDigi);

  }else
    LOG(WARNING) << "found rocId: " << Form("0x%08x ",rocId) << FairLogger::endl;
      
}

void CbmTSUnpackTof::FillEpochInfo(ngdpb::Message mess)
{
  Int_t rocId      = mess.getRocNumber();
  Int_t get4Id     = mess.getGdpbGenChipId();
  //  Int_t fGet4Nr    = fGdpbIdIndexMap[rocId]*kuNbChanAfck + get4Id*kuNbChanGet4;
  fCurrentEpoch[rocId][get4Id] = mess.getEpoch2Number();
  if( fbEpochSuppModeOn )
  {
    Int_t iBufferSize = fvmEpSupprBuffer[fGdpbIdIndexMap[rocId]][get4Id].size();
    if( 0 < iBufferSize )
    { 
      LOG(DEBUG) << "Now processing stored messages for for get4 " <<  rocId <<", "<<get4Id<< " with epoch number "
                 << (fCurrentEpoch[fGdpbIdIndexMap[rocId]][get4Id] - 1) << FairLogger::endl;
      for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
      {
        FillHitInfo( fvmEpSupprBuffer[fGdpbIdIndexMap[rocId]][get4Id][ iMsgIdx ] );
      } // for( Int_t iMsgIdx = 0; iMsgIdx < iBufferSize; iMsgIdx++ )
      fvmEpSupprBuffer[fGdpbIdIndexMap[rocId]][get4Id].clear();
    } // if( 0 < fvmEpSupprBuffer[fGet4Nr] )
  } // if( fbEpochSuppModeOn )

  if( fGdpbIdIndexMap.end() == fGdpbIdIndexMap.find( rocId ) )
  {
     fGdpbIdIndexMap[ rocId ] = fuCurrNbGdpb;
     fuCurrNbGdpb ++;
     
     if( fuMinNbGdpb < fuCurrNbGdpb )
     {
        // Add new histo
        fHM->Add( Form("Raw_Tot_gDPB_%02u", fuMinNbGdpb),
           new TH2F( Form("Raw_Tot_gDPB_%02u", fuMinNbGdpb),
                     Form("Raw TOT gDPB %02u; channel; TOT [bin]", fuMinNbGdpb),
                     kuNbChanAfck, 0, kuNbChanAfck, 256, 0, 255) );
                     
         fHM->Add( Form("ChCount_gDPB_%02u", fuMinNbGdpb),
              new TH1I( Form("ChCount_gDPB_%02u", fuMinNbGdpb),
                        Form("Channel counts gDPB %02u; channel; Hits", fuMinNbGdpb),
                        kuNbChanAfck, 0, kuNbChanAfck ) );
	 LOG(INFO)<<" Add histos for gDPB "<<fuMinNbGdpb<<", rocID "<<rocId<<FairLogger::endl;
        // increase fuMinNbGdpb
        fuMinNbGdpb++;
     } // if( fuMinNbGdpb < fuCurrNbGdpb )
  } // if( std::map::end == fGdpbIdIndexMap.find( rocId ) )

  //  LOG(INFO) << "Epoch message for ROC " << rocId << " with epoch number "
  //            << fCurrentEpoch[rocId] << FairLogger::endl;

  fCurrentEpochTime = mess.getMsgFullTime(fCurrentEpoch[rocId][get4Id]);
  fNofEpochs++;
  LOG(DEBUG1) << "Epoch message "
              << fNofEpochs << ", epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][get4Id])
              << ", time " << std::setprecision(9) << std::fixed
              << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
              << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
              << " and chip " << mess.getEpoch2ChipNumber()
              << FairLogger::endl;

}

void CbmTSUnpackTof::PrintSlcInfo(ngdpb::Message mess)
{
  Int_t rocId          = mess.getRocNumber();
  Int_t get4Id     = mess.getGdpbGenChipId();

  if( fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find( rocId ) )
     LOG(INFO) << "GET4 Slow Control message, epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][get4Id])
                << ", time " << std::setprecision(9) << std::fixed
                << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
                << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
                << FairLogger::endl
                << " +++++++ > Chip = " << std::setw(2) << mess.getGdpbGenChipId()
                << ", Chan = " << std::setw(1) << mess.getGdpbSlcChan()
                << ", Edge = " << std::setw(1) << mess.getGdpbSlcEdge()
                << ", Type = " << std::setw(1) << mess.getGdpbSlcType()
                << ", Data = " << std::hex << std::setw(6) << mess.getGdpbSlcData() << std::dec
                << ", Type = " << mess.getGdpbSlcCrc()
                << FairLogger::endl;

}

void CbmTSUnpackTof::PrintGenInfo(ngdpb::Message mess)
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

void CbmTSUnpackTof::PrintSysInfo(ngdpb::Message mess)
{
  Int_t rocId          = mess.getRocNumber();
  Int_t get4Id     = mess.getGdpbGenChipId();

  if( fGdpbIdIndexMap.end() != fGdpbIdIndexMap.find( rocId ) )
     LOG(INFO) << "GET4 System message,       epoch " << static_cast<Int_t>(fCurrentEpoch[rocId][get4Id])
                << ", time " << std::setprecision(9) << std::fixed
                << Double_t(fCurrentEpochTime) * 1.e-9 << " s "
                << " for board ID " << std::hex << std::setw(4) << rocId << std::dec
                << FairLogger::endl;
             
   switch( mess.getGdpbSysSubType() )
   {
      case ngdpb::SYSMSG_GET4_EVENT:
      {
         LOG(INFO) << " +++++++ > Chip = " << std::setw(2) << mess.getGdpbGenChipId()
                   << ", Chan = " << std::setw(1) << mess.getGdpbSysErrChanId()
                   << ", Edge = " << std::setw(1) << mess.getGdpbSysErrEdge()
                   << ", Empt = " << std::setw(1) << mess.getGdpbSysErrUnused()
                   << ", Data = " << std::hex << std::setw(2) << mess.getGdpbSysErrData() << std::dec
                   << " -- GET4 V1 Error Event"
                   << FairLogger::endl;
         break;
      } //
      case ngdpb::SYSMSG_CLOSYSYNC_ERROR:
         LOG(INFO) << "Closy synchronization error" << FairLogger::endl;
         break;
      case ngdpb::SYSMSG_TS156_SYNC:
         LOG(INFO) << "156.25MHz timestamp reset" << FairLogger::endl;
         break;
      case ngdpb::SYSMSG_GDPB_UNKWN:
         LOG(INFO) << "Unknown GET4 message, data: " << std::hex << std::setw(8)
                   << mess.getGdpbSysUnkwData() << std::dec
                   << FairLogger::endl;
         break;
   } // switch( getGdpbSysSubType() )
}

void CbmTSUnpackTof::Reset()
{
  //  fFiberHodoRaw->Clear();
  fTofDigi->Clear();
}

void CbmTSUnpackTof::Finish()
{
  TString message_type;

  for (unsigned int i=0; i< fMsgCounter.size(); ++i) {
    switch(i) {
    case 0: message_type ="NOP"; break;
    case 1: message_type ="HIT"; break;
    case 2: message_type ="EPOCH"; break;
    case 3: message_type ="SYNC"; break;
    case 4: message_type ="AUX"; break;
    case 5: message_type ="EPOCH2"; break;
    case 6: message_type ="GET4"; break;
    case 7: message_type ="SYS"; break;
    case 8: message_type ="GET4_SLC"; break;
    case 9: message_type ="GET4_32B"; break;
    case 10: message_type ="GET4_SYS"; break;
    default:  message_type ="UNKNOWN"; break;
    }
    LOG(INFO) << message_type << " messages: " 
              << fMsgCounter[i] << FairLogger::endl;
  }
  
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   for( auto it = fCurrentEpoch.begin(); it != fCurrentEpoch.end(); ++it)
      for( auto itG = (it->second).begin(); itG != (it->second).end(); ++itG)
      LOG(INFO) << "Last epoch for gDPB: " 
                << std::hex << std::setw(4) << it->first 
                << " , GET4  " << std::setw(4) << itG->first 
                << " => " << itG->second 
                << FairLogger::endl;
   LOG(INFO) << "-------------------------------------" << FairLogger::endl;
   
   
   gDirectory->mkdir("Tof_Raw_gDPB");
   gDirectory->cd("Tof_Raw_gDPB");
   fHM->H1( Form("Raw_TDig-Ref") )->Write();
   fHM->H1( Form("Raw_TRef-Dig0") )->Write();
   fHM->H1( Form("Raw_TRef-Dig1") )->Write();
   for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   {
      fHM->H2( Form("Raw_Tot_gDPB_%02u", uGdpb) )->Write();
      fHM->H1( Form("ChCount_gDPB_%02u", uGdpb) )->Write();
   } // for( UInt_t uGdpb = 0; uGdpb < fuMinNbGdpb; uGdpb ++)
   gDirectory->cd("..");

}


void CbmTSUnpackTof::FillOutput(CbmDigi* digi)
{
 if(100 > iMess++)
 LOG(DEBUG) << "Fill digi TClonesarray with "
	    <<Form("0x%08x",digi->GetAddress())
	    <<" at " << (Int_t)fTofDigi->GetEntriesFast()
	    << FairLogger::endl;

 new( (*fTofDigi)[fTofDigi->GetEntriesFast()] )
    CbmTofDigiExp(*(dynamic_cast<CbmTofDigiExp*>(digi)));
    //CbmTofDigiExp((CbmTofDigiExp *)digi);
    
 if( (digi->GetAddress() & DetMask) != 0x00005006 )
       fHM->H1( Form("Raw_TDig-Ref") )
	  ->Fill( digi->GetTime() - RefTime);
 else  RefTime=digi->GetTime();

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

void CbmTSUnpackTof::FillStarTrigInfo(ngdpb::Message mess)
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
	 Double_t dTot = 1.;
	 Double_t dTime = fulGdpbTsFullLast * 6.25;    
	 if (FirstDigiTimeDif==0. && LastDigiTime!=0.) {
	   FirstDigiTimeDif=dTime-LastDigiTime; 
	   LOG(INFO) << "Default fake digi time shift initialized to " << FirstDigiTimeDif
		     <<FairLogger::endl;
	 }	 
	 dTime -= FirstDigiTimeDif; 
	 dTime += fdTShiftRef;
	 LOG(DEBUG) << "Insert fake digi with time " << dTime<<", Tot "<<dTot
		    <<FairLogger::endl;
         fHM->H1( Form("Raw_TRef-Dig0") )
	    ->Fill( dTime - LastDigiTime);
         fHM->H1( Form("Raw_TRef-Dig1") )
	    ->Fill( dTime - LastDigiTime);

	 fDigi = new CbmTofDigiExp(0x00005006, dTime, dTot); // fake start counter signal
	 fBuffer->InsertData(fDigi);
         break;
	  } // case 3
      default:
         LOG(FATAL) << "Unknown Star Trigger messageindex: " << iMsgIndex << FairLogger::endl;
  } // switch( iMsgIndex )
}

ClassImp(CbmTSUnpackTof)
