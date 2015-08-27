// ------------------------------------------------------------------
// -----                     TTofVftxUnpacker                   -----
// -----              Created 14/05/2013 by P.-A. Loizeau       -----
// ------------------------------------------------------------------ 

#include "TTofVftxUnpacker.h"

// TOF headers
#include "TofDef.h"
#include "TMbsUnpackTofPar.h"
#include "TofTdcDef.h"
#include "TofVftxDef.h"
#include "TTofVftxData.h"
#include "TTofVftxBoard.h"

// FAIR headers
#include "FairRootManager.h"
#include "FairLogger.h"
#include "TString.h"

// ROOT headers
#include "TClonesArray.h"
#include "TH1.h"
#include "TROOT.h"
#include "TDirectory.h"

TTofVftxUnpacker::TTofVftxUnpacker():
   fParUnpack(0),
   fuNbTdc(0),
   fVftxBoardCollection(NULL),
   fiLastFpgaTdcCoarse(),
   fh1VftxRawChMap(),
   fh1VftxChFt()
{
}
TTofVftxUnpacker::TTofVftxUnpacker( TMbsUnpackTofPar * parIn ):
   fParUnpack( parIn ),
   fuNbTdc( parIn->GetNbActiveBoards( tofMbs::vftx ) ),
   fVftxBoardCollection(NULL),
   fiLastFpgaTdcCoarse(),
   fh1VftxRawChMap(),
   fh1VftxChFt()
{
   // Recover first the VFTX board objects created in general unpacker class
   FairRootManager* rootMgr = FairRootManager::Instance();
   
   fVftxBoardCollection = (TClonesArray*) rootMgr->GetObject("TofVftxTdc");
   if(NULL == fVftxBoardCollection) 
   {
      LOG(WARNING)<<"TTofVftxUnpacker::TTofVftxUnpacker : no TOF VFTX TDC array! "<<FairLogger::endl;
      fuNbTdc = 0;
   }
    
   if( 0 < fuNbTdc )
      fiLastFpgaTdcCoarse.resize( fuNbTdc * vftxtdc::kuNbChan, 0);
}
TTofVftxUnpacker::~TTofVftxUnpacker()
{
   // not sure if it will not make problems for seeing them
//   DeleteHistos();
}

void TTofVftxUnpacker::Clear(Option_t *option)
{
   fParUnpack = NULL;
   fuNbTdc    = 0;
   fVftxBoardCollection = NULL;
   fiLastFpgaTdcCoarse.clear();
}

void TTofVftxUnpacker::ProcessVFTX( Int_t iTdcIndex, UInt_t* pMbsData, UInt_t uLength )
{
//   if( (iTdcIndex<0) || (fuNbTdc <= iTdcIndex) || 
//       (fVftxBoardCollection->GetEntriesFast() <= iTdcIndex )) 
   if( (iTdcIndex<0) || (fuNbTdc <= iTdcIndex) ) 
   {
      LOG(ERROR)<<"Error VFTX number "<<iTdcIndex<<" out of bound (max "<<fuNbTdc<<") "<<FairLogger::endl;
      return;
   }
   
   TTofVftxBoard * fVftxBoard = (TTofVftxBoard*) fVftxBoardCollection->ConstructedAt(iTdcIndex);

   for(Int_t ch=0; ch < vftxtdc::kuNbChan; ch++)
      fiLastFpgaTdcCoarse[iTdcIndex*vftxtdc::kuNbChan + ch] = -1;

   UInt_t l_dat = *pMbsData++; // module header
   if ( (l_dat & vftxtdc::kiHeaderKeyMask)>>vftxtdc::kiHeaderKeyShift != vftxtdc::kiHeaderKeyword ) 
   {
      LOG(WARNING)<<"This is not a vftx #"<< fParUnpack->GetActiveToAllTypeInd(iTdcIndex, tofMbs::vftx)
                  <<" header, jumping this sub-event..."<<FairLogger::endl;
      return;
   } // if ( (l_dat & vftxtdc::kiHeaderKeyMask)>>vftxtdc::kiHeaderKeyShift != vftxtdc::kiHeaderKeyword ) 
   
   UInt_t uMbsNbData = (l_dat & vftxtdc::kiHeaderNbMask);
   if (uMbsNbData+2 != uLength && uMbsNbData+2 + 256 != uLength)
   {
      LOG(WARNING)<<"Wrong length in vftx #"<<fParUnpack->GetActiveToAllTypeInd(iTdcIndex, tofMbs::vftx)
                  <<" header, jumping this sub-event... "<<uMbsNbData+2<<" vs "<<uLength<<FairLogger::endl;
     // Return commented: MBS vftxlib.c for gsi aug 12 not compatible with too many data in event: 
     // only 8 bit where used to code event size => effective limitation to 256 hits then loop
      //return;
   } // if (uMbsNbData+2 != uLength && uMbsNbData+2 + 256 != uLength)
   
   UInt_t uModIndex  = ((l_dat & vftxtdc::kiHeaderModMask) >> vftxtdc::kiHeaderModShift); 
/*
   // Not true when first VFTX are disabled!!!
   if( uModIndex != fParUnpack->GetActiveToAllTypeInd(iTdcIndex, tofMbs::vftx) )
      LOG(WARNING)<<"Wrong VFTX index in header of vftx #"<<fParUnpack->GetActiveToAllTypeInd(iTdcIndex, tofMbs::vftx)
                  <<": "<<uModIndex<<FairLogger::endl;
*/
   
   l_dat = *pMbsData++;  // vulom tdc fifo header
   UInt_t uFifoNbData;
   if( l_dat & vftxtdc::kiFifoMessageType ) 
   {
      // Found proper TDC FIFO header => unpack it
      fVftxBoard->SetTriggerType( (l_dat & vftxtdc::kiFifoHeadTrigType) >> vftxtdc::kiFifoHeadTrigTypeShift );
      fVftxBoard->SetTriggerTime( (l_dat & vftxtdc::kiFifoHeadTrigTime) >> vftxtdc::kiFifoHeadTrigTimeShift );
      uFifoNbData = (l_dat & vftxtdc::kiFifoHeadDataCnt)  >> vftxtdc::kiFifoHeadDataCntShift;
   } // if( l_dat & vftxtdc::kiFifoMessageType ) 
      else LOG(WARNING)<<"Vftx #"<<fParUnpack->GetActiveToAllTypeInd(iTdcIndex, tofMbs::vftx)<<" fifo header missing... "<<FairLogger::endl;
   
   if (uMbsNbData != uFifoNbData)
   {
   // uMbsNbData is always 0xAA in current versions of VFTX!!!    
/*
      LOG(WARNING)<<"Wrong length in vftx #"<<fParUnpack->GetActiveToAllTypeInd(iTdcIndex, tofMbs::vftx)
                  <<" FIFO header compared to its MBS header, jumping this sub-event... "
                  <<uFifoNbData<<" vs "<<uMbsNbData<<FairLogger::endl;
      return;
*/
   } // if (uMbsNbData-2 != uFifoNbData)
   
   while (uMbsNbData-- > 0) 
   {
      UInt_t l_da0 = *pMbsData++;

      TTofVftxData hit;
      if ((l_da0 & vftxtdc::kiFifoMessageType) == vftxtdc::kiFifoMessageType) 
      {
         TString sTemp = Form("Wrong data item in vftx #%d: type %d (message %08x, header %08x), jumping this sub-event...", 
                       fParUnpack->GetActiveToAllTypeInd(iTdcIndex, tofMbs::vftx), 
                       (l_da0 & vftxtdc::kiFifoMessageType) >> vftxtdc::kiFifoMessageTypeShift, l_da0, l_dat);
         LOG(WARNING)<<sTemp<<FairLogger::endl;
         sTemp = Form("Message: tt %01u ct %5u nd %3u", 
                      (l_da0 & vftxtdc::kiFifoHeadTrigType) >> vftxtdc::kiFifoHeadTrigTypeShift,
                      (l_da0 & vftxtdc::kiFifoHeadTrigTime) >> vftxtdc::kiFifoHeadTrigTimeShift,
                      (l_da0 & vftxtdc::kiFifoHeadDataCnt)  >> vftxtdc::kiFifoHeadDataCntShift );
         LOG(WARNING)<<sTemp<<FairLogger::endl;
         sTemp = Form("Header : tt %01u ct %5u nd %3u", 
                      (l_dat & vftxtdc::kiFifoHeadTrigType) >> vftxtdc::kiFifoHeadTrigTypeShift,
                      (l_dat & vftxtdc::kiFifoHeadTrigTime) >> vftxtdc::kiFifoHeadTrigTimeShift,
                      (l_dat & vftxtdc::kiFifoHeadDataCnt)  >> vftxtdc::kiFifoHeadDataCntShift );
         LOG(WARNING)<<sTemp<<FairLogger::endl;
         return;
      } // if ((l_da0 & vftxtdc::kiFifoMessageType) == vftxtdc::kiFifoMessageType) 
      
      hit.SetChannel( (l_da0 & vftxtdc::kiChannel) >> vftxtdc::kiChannelShift ); //4-5 bit
      if ( hit.GetChannel() > vftxtdc::kuNbChan - 1)
         LOG(WARNING)<<"Channel number larger than "<<vftxtdc::kuNbChan -1
                     <<" found in vftx #"<<fParUnpack->GetActiveToAllTypeInd(iTdcIndex, tofMbs::vftx)<<FairLogger::endl;
         
      hit.SetFutureBit(  (l_da0 & vftxtdc::kiFifoFutureBit)  >> vftxtdc::kiFiFoFutBitShift ); //  1 bit
      hit.SetCoarseTime( (l_da0 & vftxtdc::kiFifoCoarseTime) >> vftxtdc::kiFifoCtShift );     // 13-15 bits
      hit.SetFineTime(   (l_da0 & vftxtdc::kiFifoFineTime)   >> vftxtdc::kiFifoFtShift );     // 10-11 bits 

      if( kTRUE == fParUnpack->IsDebug() )
      {
         if( (Int_t)( hit.GetCoarseTime() ) - fiLastFpgaTdcCoarse[iTdcIndex*vftxtdc::kuNbChan + hit.GetChannel()] < 6 &&
               -6 < ( hit.GetCoarseTime() ) - fiLastFpgaTdcCoarse[iTdcIndex*vftxtdc::kuNbChan + hit.GetChannel()] &&
               -1 < fiLastFpgaTdcCoarse[iTdcIndex*vftxtdc::kuNbChan + hit.GetChannel()] )
         {
            TString sTemp = Form("Too close hits in vftx #%d channel %d: old coarse %d new coarse %d diff %d ft %3x",
                  fParUnpack->GetActiveToAllTypeInd(iTdcIndex, tofMbs::vftx), hit.GetChannel(), 
                  fiLastFpgaTdcCoarse[iTdcIndex*vftxtdc::kuNbChan + hit.GetChannel()], hit.GetCoarseTime(),
                  (Int_t)hit.GetCoarseTime() - fiLastFpgaTdcCoarse[iTdcIndex*vftxtdc::kuNbChan + hit.GetChannel()],
                  hit.GetFineTime());
            LOG(WARNING)<<sTemp<<FairLogger::endl;
         } // if hits on same channel closer than channel dead time

         fiLastFpgaTdcCoarse[iTdcIndex*vftxtdc::kuNbChan + hit.GetChannel()] = hit.GetCoarseTime();
      } // if( kTRUE == fParUnpack->IsDebug() )

      fVftxBoard->AddData( hit );
      
   } // while (uMbsNbData-- > 0)

   // Attempt at sorting hits according to their times
   fVftxBoard->SortData();

   LOG(DEBUG2)<<" VFTX TDC #"<<fParUnpack->GetActiveToAllTypeInd(iTdcIndex, tofMbs::vftx)<<" finished"<<FairLogger::endl;
   return;
} 

// Histogram management
// TODO: maybe add some options to control which histograms are 
// created and filed (memory management)
void TTofVftxUnpacker::CreateHistos()
{
   LOG(DEBUG)<<" TTofVftxUnpacker::CreateHistos Nb boards: "<<fuNbTdc<<" Directory "<<FairLogger::endl;
   
   TDirectory * oldir = gDirectory; // <= To prevent histos from being sucked in by the param file of the TRootManager!
   gROOT->cd(); // <= To prevent histos from being sucked in by the param file of the TRootManager !
   
   TH1* hTemp = 0;
   
   // 2D vector of histograms
   fh1VftxChFt.resize( fuNbTdc );
   for( Int_t iBoardIndex = 0; iBoardIndex < fuNbTdc; iBoardIndex++)
   {
      // Board specific histograms
      hTemp = new TH1I( Form("tof_%s_ch_%03d", toftdc::ksTdcHistName[ toftdc::vftx ].Data(), iBoardIndex),
                        Form("Counts per TDC channel for vftx #%03d", iBoardIndex),
                        vftxtdc::kuNbChan, 0.0, vftxtdc::kuNbChan );
      fh1VftxRawChMap.push_back( hTemp );
      
      for( Int_t iChannelIndex = 0; iChannelIndex < vftxtdc::kuNbChan; iChannelIndex++)
      {
         // Channel specific histograms
         hTemp = new TH1I( Form("tof_%s_ft_b%03d_ch%03d", 
                                    toftdc::ksTdcHistName[ toftdc::vftx ].Data(), 
                                    iBoardIndex, iChannelIndex),
                           Form("Counts per Finetime bin for TDC channel %3d on vftx #%03d", 
                                 iChannelIndex, iBoardIndex),
                           vftxtdc::kiFifoFineTime + 1, -0.5, vftxtdc::kiFifoFineTime + 1 - 0.5 );
         (fh1VftxChFt[iBoardIndex]).push_back( hTemp );
         
         LOG(DEBUG)<<" TTofVftxUnpacker::CreateHistos => Create FT histo for "
                                  <<toftdc::ksTdcHistName[ toftdc::vftx ]<<" #"
                                  <<iBoardIndex<<" ch "<<iChannelIndex<<" from the unpack step: 0x"
                                  <<fh1VftxChFt[iBoardIndex][iChannelIndex]<<" "
                                  <<fh1VftxChFt[iBoardIndex][iChannelIndex]->GetEntries()
                                  <<FairLogger::endl;
      } // for( Int_t iChannelIndex = 0; iChannelIndex < vftxtdc::kuNbChan; iChannelIndex++)
      
   } // for( Int_t iBoardIndex = 0; iBoardIndex < fuNbTdc; iBoardIndex++)
   
   gDirectory->cd( oldir->GetPath() ); // <= To prevent histos from being sucked in by the param file of the TRootManager!
}
void TTofVftxUnpacker::FillHistos()
{
   // loop over TDC boards
   TTofVftxBoard * fVftxBoard;
   for( Int_t iBoardIndex = 0; iBoardIndex < fuNbTdc; iBoardIndex++)
   { 
      fVftxBoard = (TTofVftxBoard*) fVftxBoardCollection->ConstructedAt(iBoardIndex);
      // Loop over hits data
      for( Int_t iDataIndex = 0; iDataIndex < fVftxBoard->GetDataNb() ; iDataIndex++ )
      {
         TTofVftxData data = fVftxBoard->GetData( iDataIndex);
         TString sTemp = Form( " TTofVftxUnpacker::FillHistos: Board #%03d Data #%04d Chan %3d CT %7d FT %7d",
                  iBoardIndex, iDataIndex, data.GetChannel(), 
                  data.GetCoarseTime(), data.GetFineTime() );
         LOG(DEBUG)<<sTemp<<FairLogger::endl;
         
         fh1VftxRawChMap[iBoardIndex]->Fill(  data.GetChannel() );
         fh1VftxChFt[iBoardIndex][ data.GetChannel() ]->Fill( data.GetFineTime() );
         
      } // for( Int_t iDataIndex = 0; iDataIndex < fVftxBoard[iBoardIndex]->GetDataNb() ; iDataIndex++ )
   } // for( Int_t iBoardIndex = 0; iBoardIndex < fuNbTdc; IBoardIndex++)
}
void TTofVftxUnpacker::WriteHistos( TDirectory* inDir)
{
   TDirectory * oldir = gDirectory;
   TDirectory *cdVftxUnp[fuNbTdc];
   
   for( Int_t iBoardIndex = 0; iBoardIndex < fuNbTdc; iBoardIndex++)
   { 
      cdVftxUnp[iBoardIndex] = inDir->mkdir( Form( "Unp_%s_%03d", toftdc::ksTdcHistName[ toftdc::vftx ].Data(), iBoardIndex) );
      cdVftxUnp[iBoardIndex]->cd();    // make the "Unp_triglog" directory the current directory
      
      fh1VftxRawChMap[iBoardIndex]->Write();
      
      for( Int_t iChannelIndex = 0; iChannelIndex < vftxtdc::kuNbChan; iChannelIndex++)
      {
         fh1VftxChFt[iBoardIndex][iChannelIndex]->Write();
      } // for( Int_t iChannelIndex = 0; iChannelIndex < vftxtdc::kuNbChan; iChannelIndex++)
      
   } // for( Int_t iBoardIndex = 0; iBoardIndex < fuNbTdc; iBoardIndex++)
}
void TTofVftxUnpacker::DeleteHistos()
{
   fh1VftxRawChMap.clear();
   for( Int_t iBoardIndex = 0; iBoardIndex < fuNbTdc; iBoardIndex++)
      (fh1VftxChFt[iBoardIndex]).clear();
   fh1VftxChFt.clear();
}
