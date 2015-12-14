// -----------------------------------------------------------------------------
// ----- TTofTrbTdcUnpacker source file                                    -----
// -----                                                                   -----
// ----- created by C. Simon on 2014-04-03                                 -----
// -----                                                                   -----
// ----- based on TTofVftxUnpacker by P.-A. Loizeau                      -----
// ----- https://subversion.gsi.de/fairroot/cbmroot/development/ploizeau/  -----
// -----   main/unpack/tof/tdc/vftx/TTofVftxUnpacker.cxx                 -----
// ----- revision 20754, 2013-07-17                                        -----
// -----------------------------------------------------------------------------

#include "TTofTrbTdcUnpacker.h"

// Parameter header
#include "TMbsUnpackTofPar.h"

// Output data objects
#include "TTofTrbTdcBoard.h"

// Defines
#include "TofDef.h"
#include "TofTdcDef.h"

// TDC iterator
#include "HadaqTdcIterator.h"

// ROOT
#include "TClonesArray.h"
#include "TH1.h"
#include "TROOT.h"

// FairRoot
#include "FairRootManager.h"
#include "FairLogger.h"

ClassImp(TTofTrbTdcUnpacker)

std::map<Int_t,Bool_t> TTofTrbTdcUnpacker::fmRingBufferOverflow;

TTofTrbTdcUnpacker::TTofTrbTdcUnpacker( TMbsUnpackTofPar * parIn ):
   fParUnpack( parIn ),
   fuNbActiveTrbTdc( parIn->GetNbActiveBoards( tofMbs::trbtdc ) ),
   fbCalibTrigger(kFALSE),
   fbJointEdgesChannel( parIn->IsTrbTdcJointEdgesChannel() ),
   fTrbTdcBoardCollection(0),
   fTrbTdcRingBufferOverflow(NULL),
   fTrbTdcChannelOccupancy(),
   fTrbTdcChannelFineTimeOvershoot(),
   fTrbTdcChannelUnprocessedHits(),
   fTrbTdcChannelFineTime(),
   fTrbTdcChannelCoarseTime(),
   fuEventIndex(0)
{
   LOG(INFO)<<"**** TTofTrbTdcUnpacker: Call TTofTrbTdcUnpacker()..."<<FairLogger::endl;

   // Recover first the TRB-TDC board objects created in TTrbUnpackTof
   FairRootManager* rootMgr = FairRootManager::Instance();
   
   fTrbTdcBoardCollection = (TClonesArray*) rootMgr->GetObject("TofTrbTdc");
   if(0 == fTrbTdcBoardCollection) 
   {
      LOG(WARNING)<<"TTofTrbTdcUnpacker::TTofTrbTdcUnpacker : no TOF TRB-TDC array! "<<FairLogger::endl;
      fuNbActiveTrbTdc = 0;
   }
    
}

TTofTrbTdcUnpacker::~TTofTrbTdcUnpacker()
{
   LOG(INFO)<<"**** TTofTrbTdcUnpacker: Call ~TTofTrbTdcUnpacker()..."<<FairLogger::endl;
  // Creation and clean-up of member histogram vectors done by TMbsUnpTofMonitor
}

void TTofTrbTdcUnpacker::Clear(Option_t */*option*/)
{
   LOG(INFO)<<"**** TTofTrbTdcUnpacker: Call Clear()..."<<FairLogger::endl;

   fParUnpack = 0;
   fuNbActiveTrbTdc = 0;
   fTrbTdcBoardCollection = 0;
}

Int_t TTofTrbTdcUnpacker::ProcessData( hadaq::RawSubevent* tSubevent, UInt_t uStartIndex )
{
/* unclear points:
   - EPOCH reset compensation in stream
*/
  LOG(DEBUG)<<"**** TTofTrbTdcUnpacker: Call ProcessData()..."<<FairLogger::endl;

  UInt_t uTdcDataIndex = uStartIndex;

  UInt_t uTdcData = tSubevent->Data(uTdcDataIndex);
  UInt_t uTdcAddress = uTdcData & 0xffff;
  UInt_t uNbTdcDataWords = (uTdcData >> 16) & 0xffff;
  Int_t iTdcBoardIndex = fParUnpack->GetActiveTrbTdcIndex( uTdcAddress );

  TTofTrbTdcBoard* tTrbTdcBoard = (TTofTrbTdcBoard*) fTrbTdcBoardCollection->ConstructedAt( iTdcBoardIndex );
  tTrbTdcBoard->SetInvalid();
  UInt_t uTdcWordCount = 0;

  // PAL 23/07/2015: Use direct constructor call instead of copy constructor
//  hadaq::TdcIterator tTrbTdcIterator = hadaq::TdcIterator();
  hadaq::TdcIterator tTrbTdcIterator;
  tTrbTdcIterator.assign(tSubevent, uTdcDataIndex+1, uNbTdcDataWords);

  hadaq::TdcMessage& tTrbTdcMessage = tTrbTdcIterator.msg();

  Bool_t bPreviousIsEpoch = kFALSE;
  UInt_t uPreviousChannel = 0;

  while ( tTrbTdcIterator.next() )
  {
    uTdcWordCount++;

    if( 1 == uTdcWordCount )
    {
      if ( tTrbTdcMessage.isHeaderMsg() )
      {
        if( tTrbTdcMessage.getHeaderErr() )
        {
          tTrbTdcBoard->SetChannelBufferIssue();
          fmRingBufferOverflow[iTdcBoardIndex] = kTRUE;

//          LOG(ERROR)<<Form("At least one channel ring buffer of TDC 0x%.4x has been overwritten. Skip this TDC subsubevent!",uTdcAddress)<<FairLogger::endl;
//          return trbtdc::process_RingBufferOverwrite;
        }
      }
      else
      {
        LOG(ERROR)<<Form("First data word from TDC 0x%.4x is not a TDC HEADER word. Skip this TDC subsubevent!",uTdcAddress)<<FairLogger::endl;
        return trbtdc::process_FirstNoHeader;
      }
      
      continue;
    }

    if( 2 == uTdcWordCount )
    {
      if ( tTrbTdcMessage.isEpochMsg() )
      {
        tTrbTdcIterator.setRefEpoch( tTrbTdcIterator.getCurEpoch() );
      }
      else
      {
        LOG(ERROR)<<Form("Second data word from TDC 0x%.4x is not a TDC EPOCH word. Skip this TDC subsubevent!",uTdcAddress)<<FairLogger::endl;
        return trbtdc::process_SecondNoEpoch;
      }
      
      continue;
    }

    if( 3 == uTdcWordCount )
    {
      if ( tTrbTdcMessage.isTimeMsg() )
      {
        if( tTrbTdcMessage.getTimeChannel() == 0 )
        {
          if( trbtdc::kuHighestReasonableFineTimeBin >= tTrbTdcMessage.getTimeTmFine() )
          {
            ( tTrbTdcBoard->GetRefChannelData() ).SetData( 
              tTrbTdcMessage.getTimeChannel(),
              tTrbTdcMessage.getTimeTmFine(),
              ( UInt_t )( tTrbTdcIterator.getMsgStamp() & ( trbtdc::kiCoarseCounterSize-1 ) ),
              0,
              tTrbTdcMessage.getTimeEdge() );
            ( tTrbTdcBoard->GetRefChannelData() ).SetEpoch( tTrbTdcIterator.getCurEpoch() );
            ( tTrbTdcBoard->GetRefChannelData() ).SetFullCoarseTime( tTrbTdcIterator.getMsgStamp() );

            tTrbTdcBoard->SetTriggerTime( ( tTrbTdcBoard->GetRefChannelData() ).GetCoarseTime() );

          }
          else
          {
            LOG(INFO)<<Form("Event %7d: Third data word from TDC 0x%.4x is a TDC TIME word from channel 0 but with an unreasonable fine time of 0x%.3x. Skip this TDC subsubevent!",fuEventIndex, uTdcAddress,tTrbTdcMessage.getTimeTmFine())<<FairLogger::endl;
            return trbtdc::process_ThirdBadRefTime;
          }
        }
        else
        {
          LOG(ERROR)<<Form("Third data word from TDC 0x%.4x is a TDC TIME word but not from channel 0. Skip this TDC subsubevent!",uTdcAddress)<<FairLogger::endl;
          return trbtdc::process_ThirdNoRefTime;
        }
      }
      else
      {
        LOG(ERROR)<<Form("Third data word from TDC 0x%.4x is not a TDC TIME word. Skip this TDC subsubevent!",uTdcAddress)<<FairLogger::endl;
        return trbtdc::process_ThirdNoTime;
      }
      
      continue;
    }

    if( 4 == uTdcWordCount )
    {
      if ( !tTrbTdcMessage.isEpochMsg() && !fbCalibTrigger )
      {
        LOG(ERROR)<<Form("Fourth data word from TDC 0x%.4x is not a TDC EPOCH word. Stop unpacking this TDC subsubevent! Left words: %d",
			 uTdcAddress,uNbTdcDataWords-uTdcWordCount)
		  <<FairLogger::endl;
        return trbtdc::process_FourthNoEpoch;
      }

      bPreviousIsEpoch = kTRUE;
      continue;
    }

    if( kTRUE == bPreviousIsEpoch )
    {
      switch ( tTrbTdcMessage.getKind() ) 
      {
        case trbtdc::message_Time:
        case trbtdc::message_Time1:
        case trbtdc::message_Time2:
        case trbtdc::message_Time3:
        {
          bPreviousIsEpoch = kFALSE;

          UInt_t uChNumber         = tTrbTdcMessage.getTimeChannel();
          UInt_t uChFineTime       = tTrbTdcMessage.getTimeTmFine();
          UInt_t uChCoarseTime     = ( UInt_t )( tTrbTdcIterator.getMsgStamp() & ( trbtdc::kiCoarseCounterSize-1 ) );
          UInt_t uChEdge           = tTrbTdcMessage.getTimeEdge();
          UInt_t uChEpoch          = tTrbTdcIterator.getCurEpoch();
          ULong64_t uChFullCoarseTime = tTrbTdcIterator.getMsgStamp();

          uPreviousChannel = uChNumber;

          if ( ( 0 == uChNumber && !fbCalibTrigger ) || ( tTrbTdcBoard->GetChannelNb() < uChNumber ) )
          {
            LOG(ERROR)<<Form("Invalid channel number %u in TDC TIME word from TDC 0x%.4x. Stop unpacking this TDC subsubevent!", uChNumber, uTdcAddress)<<FairLogger::endl;
            return trbtdc::process_BadChannelNumber;
          }

          if ( trbtdc::time_FineTimeBitMask == uChFineTime )
          {
            LOG(DEBUG2)<<Form("Hit detected in channel %u but not properly processed by TDC 0x%.4x. Skip it.", uChNumber, uTdcAddress)<<FairLogger::endl;
            continue;
          }
          else
          {
        	if( 0 == uChNumber )
            {
              LOG(DEBUG2)<<Form("Additional calibration hit detected in channel %u of TDC 0x%.4x. Ignore it.", uChNumber, uTdcAddress)<<FairLogger::endl;
              continue;
        	}
        	else
        	{
              LOG(DEBUG2)<<Form("Hit in channel %u properly processed by TDC 0x%.4x. Store it.", uChNumber, uTdcAddress)<<FairLogger::endl;

              // Remove reference channel index
              UInt_t uFinalChNumber = uChNumber - 1;

              if( fbJointEdgesChannel )
              {
                uFinalChNumber = 2*uFinalChNumber + (uChEdge & 0x1);
              }

              TTofTrbTdcData tValidHit( uFinalChNumber, uChFineTime, uChCoarseTime, 0, uChEdge, uChEpoch, uChFullCoarseTime );
              tTrbTdcBoard->AddData( tValidHit );
              continue;
        	}
          }
       
          break;
        }
        case trbtdc::message_Epoch:

            LOG(ERROR)<<Form("Two consecutive TDC EPOCH words from TDC 0x%.4x. Stop unpacking this TDC subsubevent!", uTdcAddress)<<FairLogger::endl;
            return trbtdc::process_TwoConsecutiveEpochs;

          break;

        default:

          continue;

          break;
      }

    }
    else
    {
      switch ( tTrbTdcMessage.getKind() ) 
      {
        case trbtdc::message_Time:
        case trbtdc::message_Time1:
        case trbtdc::message_Time2:
        case trbtdc::message_Time3:
        {
          UInt_t uChNumber         = tTrbTdcMessage.getTimeChannel();
          UInt_t uChFineTime       = tTrbTdcMessage.getTimeTmFine();
          UInt_t uChCoarseTime     = ( UInt_t )( tTrbTdcIterator.getMsgStamp() & ( trbtdc::kiCoarseCounterSize-1 ) );
          UInt_t uChEdge           = tTrbTdcMessage.getTimeEdge();
          UInt_t uChEpoch          = tTrbTdcIterator.getCurEpoch();
          ULong64_t uChFullCoarseTime = tTrbTdcIterator.getMsgStamp();

          if( ( 0 == uChNumber && !fbCalibTrigger ) || ( tTrbTdcBoard->GetChannelNb() < uChNumber ) )
          {
            LOG(ERROR)<<Form("Invalid channel number %u in TDC TIME word from TDC 0x%.4x. Stop unpacking this TDC subsubevent!", uChNumber, uTdcAddress)<<FairLogger::endl;
            return trbtdc::process_BadChannelNumber;
          }

          if( uPreviousChannel != uChNumber )
          {
            LOG(ERROR)<<Form("Change in hit channel number from %u to %u without a TDC EPOCH word in between in TDC 0x%.4x. Stop unpacking this TDC subsubevent!", uPreviousChannel, uChNumber, uTdcAddress)<<FairLogger::endl;
            return trbtdc::process_ChannelButNoEpochChange;
          }

          if( trbtdc::time_FineTimeBitMask == uChFineTime )
          {
            LOG(DEBUG2)<<Form("Hit detected in channel %u but not properly processed by TDC 0x%.4x. Skip it.", uChNumber, uTdcAddress)<<FairLogger::endl;
            continue;
          }
          else
          {
          	if( 0 == uChNumber )
              {
                LOG(DEBUG2)<<Form("Additional calibration hit detected in channel %u of TDC 0x%.4x. Ignore it.", uChNumber, uTdcAddress)<<FairLogger::endl;
                continue;
          	}
          	else
          	{
                LOG(DEBUG2)<<Form("Hit in channel %u properly processed by TDC 0x%.4x. Store it.", uChNumber, uTdcAddress)<<FairLogger::endl;

                // Remove reference channel index
                UInt_t uFinalChNumber = uChNumber - 1;

                if( fbJointEdgesChannel )
                {
                  uFinalChNumber = 2*uFinalChNumber + (uChEdge & 0x1);
                }

                TTofTrbTdcData tValidHit( uFinalChNumber, uChFineTime, uChCoarseTime, 0, uChEdge, uChEpoch, uChFullCoarseTime );
                tTrbTdcBoard->AddData( tValidHit );
                continue;
          	}
          }

          break;
        }
        case trbtdc::message_Epoch:

          bPreviousIsEpoch = kTRUE;
          continue;

          break;

        default:

          continue;

          break;
      }

    }



  }

   tTrbTdcBoard->SortData();

   tTrbTdcBoard->SetCalibData(fbCalibTrigger);
   tTrbTdcBoard->SetValid();

   if( 0 == iTdcBoardIndex )
      fuEventIndex++;

   LOG(DEBUG)<<Form("TRB-TDC subsubevent from FPGA 0x%.4x fully unpacked. Success!",uTdcAddress)<<FairLogger::endl;
   return trbtdc::process_Success;

}

// Histogram management
// TODO: maybe add some options to control which histograms are 
// created and filed (memory management)
void TTofTrbTdcUnpacker::CreateHistos()
{
   LOG(INFO)<<"**** TTofTrbTdcUnpacker: Call CreateHistos()..."<<FairLogger::endl;
  
   TDirectory * oldir = gDirectory; // <= To prevent histos from being sucked in by the param file of the TRootManager!
   gROOT->cd(); // <= To prevent histos from being sucked in by the param file of the TRootManager !
   
   TH1* hTemp = 0;

   fTrbTdcChannelFineTime.resize( fuNbActiveTrbTdc );
   fTrbTdcChannelCoarseTime.resize( fuNbActiveTrbTdc );

   for( UInt_t uBoardIndex = 0; uBoardIndex < fuNbActiveTrbTdc; uBoardIndex++)
   {
      UInt_t uTrbNetAddress = fParUnpack->GetActiveTrbTdcAddr(uBoardIndex);

      // Board specific histograms
      hTemp = new TH1I( Form("tof_%s_ch_occ_%03u", toftdc::ksTdcHistName[ toftdc::trb ].Data(), uBoardIndex),
                        Form("Channel occupancy in TRB-TDC #%03u (0x%04x)", uBoardIndex, uTrbNetAddress),
                        trbtdc::kuNbChan+1, 0.0, trbtdc::kuNbChan+1 );
      fTrbTdcChannelOccupancy.push_back( hTemp );

      hTemp = new TH1I( Form("tof_%s_ch_highft_%03u", toftdc::ksTdcHistName[ toftdc::trb ].Data(), uBoardIndex),
                        Form("Fine time overshoot in TRB-TDC #%03u (0x%04x)", uBoardIndex, uTrbNetAddress),
                        trbtdc::kuNbChan+1, 0.0, trbtdc::kuNbChan+1 );
      fTrbTdcChannelFineTimeOvershoot.push_back( hTemp );


      hTemp = new TH1I( Form("tof_%s_ch_unproc_%03u", toftdc::ksTdcHistName[ toftdc::trb ].Data(), uBoardIndex),
                        Form("Unprocessed hits in TRB-TDC #%03u (0x%04x)", uBoardIndex, uTrbNetAddress),
                        trbtdc::kuNbChan+1, 0.0, trbtdc::kuNbChan+1 );
      fTrbTdcChannelUnprocessedHits.push_back( hTemp );
      
      // Reference Channel specific histograms
      hTemp = new TH1I( Form("tof_%s_ft_b%03u_ref", 
                                 toftdc::ksTdcHistName[ toftdc::trb ].Data(), 
                                 uBoardIndex),
                        Form("Counts per fine-time bin for reference channel on TRB-TDC #%03u (0x%04x)", uBoardIndex, uTrbNetAddress),
                        trbtdc::kiFineCounterSize, -0.5, trbtdc::kiFineCounterSize - 0.5 );

      (fTrbTdcChannelFineTime[uBoardIndex]).push_back( hTemp );

      hTemp = new TH1I( Form("tof_%s_ct_b%03u_ref", 
                                 toftdc::ksTdcHistName[ toftdc::trb ].Data(), 
                                 uBoardIndex),
                        Form("Counts per coarse-time bin for reference channel on TRB-TDC #%03u (0x%04x)", uBoardIndex, uTrbNetAddress),
                           trbtdc::kiCoarseCounterSize/1024/512, -0.5, trbtdc::kiCoarseCounterSize/1024/512 - 0.5 );
      // TODO: remove hard coding
      (fTrbTdcChannelCoarseTime[uBoardIndex]).push_back( hTemp );

      for( UInt_t uChannelIndex = 0; uChannelIndex < trbtdc::kuNbChan; uChannelIndex++)
      {
         // Channel specific histograms
         hTemp = new TH1I( Form("tof_%s_ft_b%03u_ch%03u", 
                                    toftdc::ksTdcHistName[ toftdc::trb ].Data(), 
                                    uBoardIndex, uChannelIndex),
                           Form("Counts per fine-time bin for channel %3u on TRB-TDC #%03u (0x%04x)", uChannelIndex, uBoardIndex, uTrbNetAddress),
                           trbtdc::kiFineCounterSize, -0.5, trbtdc::kiFineCounterSize - 0.5 );

         (fTrbTdcChannelFineTime[uBoardIndex]).push_back( hTemp );

         hTemp = new TH1I( Form("tof_%s_ct_b%03u_ch%03u", 
                                    toftdc::ksTdcHistName[ toftdc::trb ].Data(), 
                                    uBoardIndex, uChannelIndex),
                           Form("Counts per coarse-time bin for channel %3u on TRB-TDC #%03u (0x%04x)", uChannelIndex, uBoardIndex, uTrbNetAddress),
                           trbtdc::kiCoarseCounterSize/1024/512, -0.5, trbtdc::kiCoarseCounterSize/1024/512 - 0.5 );
// TODO: remove hard coding
         (fTrbTdcChannelCoarseTime[uBoardIndex]).push_back( hTemp );

         
      }
      
   }
   
   fTrbTdcRingBufferOverflow = new TH1I( Form("tof_%s_buffer_overflow", toftdc::ksTdcHistName[ toftdc::trb ].Data()),
		                                 "TRB-TDC ring buffer overflows per board",
		                                 fuNbActiveTrbTdc, 0, fuNbActiveTrbTdc);

   gDirectory->cd( oldir->GetPath() ); // <= To prevent histos from being sucked in by the param file of the TRootManager!
}

void TTofTrbTdcUnpacker::FillHistos()
{
   LOG(DEBUG)<<"**** TTofTrbTdcUnpacker: Call FillHistos()..."<<FairLogger::endl;

   // loop over TDC boards
   TTofTrbTdcBoard * tTrbTdcBoard;

   for( UInt_t uBoardIndex = 0; uBoardIndex < fuNbActiveTrbTdc; uBoardIndex++)
   { 
      UInt_t uTrbNetAddress = fParUnpack->GetActiveTrbTdcAddr(uBoardIndex);

      tTrbTdcBoard = (TTofTrbTdcBoard*) fTrbTdcBoardCollection->ConstructedAt(uBoardIndex);

      // Loop over regular channels
      for( UInt_t uDataIndex = 0; uDataIndex < tTrbTdcBoard->GetDataNb() ; uDataIndex++ )
      {
         TTofTrbTdcData data = tTrbTdcBoard->GetData( uDataIndex );
         TString sTemp = Form( " TTofTrbTdcUnpacker::FillHistos: Addr 0x%04x Board #%03u Data #%04u Chan %3d CT %7d FT %7d Edge %1u",
                  uTrbNetAddress, uBoardIndex, uDataIndex, data.GetChannel()+1, 
                  data.GetCoarseTime(), data.GetFineTime(), data.GetEdge() );
         LOG(DEBUG2)<<sTemp<<FairLogger::endl;
         
         fTrbTdcChannelOccupancy[uBoardIndex]->Fill( data.GetChannel() + 1 );

         if( trbtdc::time_FineTimeBitMask == data.GetFineTime() )
         {
           fTrbTdcChannelUnprocessedHits[uBoardIndex]->Fill( data.GetChannel() + 1 );
         }

         if( trbtdc::kuHighestReasonableFineTimeBin < data.GetFineTime() )
         {
           fTrbTdcChannelFineTimeOvershoot[uBoardIndex]->Fill( data.GetChannel() + 1 );           
         }

         fTrbTdcChannelFineTime[uBoardIndex][ data.GetChannel() +1 ]->Fill( data.GetFineTime() );
         fTrbTdcChannelCoarseTime[uBoardIndex][ data.GetChannel() +1 ]->Fill( data.GetCoarseTime()%2048 );
      }

      // Consider the respective reference channel
      fTrbTdcChannelOccupancy[uBoardIndex]->Fill(0);

      if( trbtdc::time_FineTimeBitMask == ( tTrbTdcBoard->GetRefChannelData() ).GetFineTime() )
      {
        fTrbTdcChannelUnprocessedHits[uBoardIndex]->Fill(0);
      }

      if( trbtdc::kuHighestReasonableFineTimeBin < ( tTrbTdcBoard->GetRefChannelData() ).GetFineTime() )
      {
        fTrbTdcChannelFineTimeOvershoot[uBoardIndex]->Fill(0);           
      }

      fTrbTdcChannelFineTime[uBoardIndex][0]->Fill( ( tTrbTdcBoard->GetRefChannelData() ).GetFineTime() );
      fTrbTdcChannelCoarseTime[uBoardIndex][0]->Fill( ( tTrbTdcBoard->GetRefChannelData() ).GetCoarseTime()%2048 );

   }

   for ( std::map<Int_t,Bool_t>::iterator it = fmRingBufferOverflow.begin();
	     it != fmRingBufferOverflow.end(); ++it)
   {
	 if( it->second )
	 {
	   fTrbTdcRingBufferOverflow->Fill( it->first );
	 }
   }

   fmRingBufferOverflow.clear();
}

void TTofTrbTdcUnpacker::WriteHistos( TDirectory* inDir)
{
   LOG(INFO)<<"**** TTofTrbTdcUnpacker: Call WriteHistos()..."<<FairLogger::endl;

   TDirectory * oldDir = gDirectory;
   TDirectory * cdTrbTdcUnp[fuNbActiveTrbTdc];
   
   for( UInt_t uBoardIndex = 0; uBoardIndex < fuNbActiveTrbTdc; uBoardIndex++)
   { 
      cdTrbTdcUnp[uBoardIndex] = inDir->mkdir( Form( "Unp_%s_%03u", toftdc::ksTdcHistName[ toftdc::trb ].Data(), uBoardIndex) );
      cdTrbTdcUnp[uBoardIndex]->cd();    // make the "Unp..." directory the current directory
      
      fTrbTdcChannelOccupancy[uBoardIndex]->Write();
      fTrbTdcChannelFineTimeOvershoot[uBoardIndex]->Write();
      fTrbTdcChannelUnprocessedHits[uBoardIndex]->Write();
      
      for( UInt_t uChannelIndex = 0; uChannelIndex <= trbtdc::kuNbChan; uChannelIndex++)
      {
         fTrbTdcChannelFineTime[uBoardIndex][uChannelIndex]->Write();
         fTrbTdcChannelCoarseTime[uBoardIndex][uChannelIndex]->Write();
      }
      
   }

   TDirectory * tAllHistoDir = inDir->mkdir( Form( "Unp_%s_all", toftdc::ksTdcHistName[ toftdc::trb ].Data()) );
   tAllHistoDir->cd();
   fTrbTdcRingBufferOverflow->Write();

   gDirectory->Cd(oldDir->GetPath());
}

void TTofTrbTdcUnpacker::DeleteHistos()
{
   LOG(INFO)<<"**** TTofTrbTdcUnpacker: Call DeleteHistos()..."<<FairLogger::endl;

   fTrbTdcChannelOccupancy.clear();
   fTrbTdcChannelFineTimeOvershoot.clear();
   fTrbTdcChannelUnprocessedHits.clear();

   for( UInt_t uBoardIndex = 0; uBoardIndex < fuNbActiveTrbTdc; uBoardIndex++)
   {
      (fTrbTdcChannelFineTime[uBoardIndex]).clear();
      (fTrbTdcChannelCoarseTime[uBoardIndex]).clear();
   }

   fTrbTdcChannelFineTime.clear();
   fTrbTdcChannelCoarseTime.clear();

   delete fTrbTdcRingBufferOverflow;
   fmRingBufferOverflow.clear();
}
