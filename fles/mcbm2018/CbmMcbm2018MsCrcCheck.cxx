// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                          CbmMcbm2018MsCrcCheck                    -----
// -----                    Created 02.02.2019 by P.-A. Loizeau            -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmMcbm2018MsCrcCheck.h"

#include "TimesliceInputArchive.hpp"
#include "Timeslice.hpp"
#include "TimesliceSubscriber.hpp"

#include "FairLogger.h"

#include <iostream>
#include <fstream>
#include <iomanip>

CbmMcbm2018MsCrcCheck::CbmMcbm2018MsCrcCheck()
  : fFileName(""),
    fInputFileList( new TObjString() ),
    fFileCounter(0),
    fTSNumber(0),
    fTSCounter(0),
    fSource( nullptr )
{
}

CbmMcbm2018MsCrcCheck::~CbmMcbm2018MsCrcCheck()
{
}

void CbmMcbm2018MsCrcCheck::Run()
{
   while( OpenNextFile() )
   {
      while( auto timeslice = fSource->get() )
      {
         const fles::Timeslice& ts = *timeslice;
         auto tsIndex = ts.index();

         if( 0 == tsIndex % 1000 )
         {
            LOG(INFO) << "Reading Timeslice " << tsIndex
                      << FairLogger::endl;
         } // if( 0 == tsIndex % 1000 )

         UInt_t fuNbCoreMsPerTs = ts.num_core_microslices();
         UInt_t fuNbComponents  = ts.num_components();
         /// Loop over core microslices
         for( UInt_t uMsIdx = 0; uMsIdx < fuNbCoreMsPerTs; uMsIdx ++ )
         {
            /// Loop over registered components
            for( UInt_t uMsCompIdx = 0; uMsCompIdx < fuNbComponents; ++uMsCompIdx )
            {
               bool bCrcOk = ts.get_microslice( uMsCompIdx, uMsIdx ).check_crc();

               if( !bCrcOk )
               {
                  auto msDescriptor = ts.descriptor( uMsCompIdx, uMsIdx );
                  uint32_t uSize  = msDescriptor.size;
                  const uint8_t* msContent = reinterpret_cast<const uint8_t*>( ts.content( uMsCompIdx, uMsIdx ) );
                  LOG(INFO) << "---------------------------------------------------------------"
                            << FairLogger::endl;
                  LOG(INFO) << Form( " CRC error for TS %6lu MS %3u Component %2u, dump following", tsIndex, uMsIdx, uMsCompIdx )
                            << FairLogger::endl;
                  LOG(INFO) << "---------------------------------------------------------------"
                            << FairLogger::endl;
                  LOG(INFO) << "hi hv eqid flag si sv idx/start        crc      size     offset"
                            << FairLogger::endl;
                  LOG(INFO) << Form( "%02x %02x %04x %04x %02x %02x %016lx %08x %08x %016lx",
                                    static_cast<unsigned int>(msDescriptor.hdr_id),
                                    static_cast<unsigned int>(msDescriptor.hdr_ver), msDescriptor.eq_id, msDescriptor.flags,
                                    static_cast<unsigned int>(msDescriptor.sys_id),
                                    static_cast<unsigned int>(msDescriptor.sys_ver), msDescriptor.idx, msDescriptor.crc,
                                    msDescriptor.size, msDescriptor.offset )
                            << FairLogger::endl;
                  for( UInt_t uByte = 0; uByte < uSize; ++uByte )
                  {
                     LOG(INFO) << Form( "%02x", msContent[ uByte ] );
                     if( 3 == uByte % 4 )
                        LOG(INFO) << " ";
                     if( 15 != uByte % 16 )
                        LOG(INFO) << FairLogger::endl;
                  } // for( UInt_t uByte = 0; uByte < uSize; ++uByte )
                  if( 0 == uSize % 16 )
                     LOG(INFO) << FairLogger::endl;
               } // if( !bCrcOk )
            } // for( UInt_t uMsCompIdx = 0; uMsCompIdx < fuNbComponents; ++uMsCompIdx )
         } // for( UInt_t uMsIdx = 0; uMsIdx < fuNbCoreMsPerTs; uMsIdx ++ )

         fTSCounter++;
         if( 0 == fTSCounter % 10000 )
         {
            LOG(INFO) << "Analysed " << fTSCounter << " TS "
                      << FairLogger::endl;
         } // if( 0 == fTSCounter % 10000 )
      } // while( auto timeslice = fSource->get() )

      /// If no more data and file mode, try to read next file in List
      if( fSource->eos() && 0 < fFileName.Length() )
      {
         fFileCounter ++; // Increment file counter to go to next item in List
      } // if( fSource->eos() && 0 < fFileName.Length() )
   } // while( OpenNextFile() )
}

Bool_t CbmMcbm2018MsCrcCheck::OpenNextFile()
{
   // First Close and delete existing source
   if( nullptr != fSource )
      delete fSource;

   if( fFileCounter < fInputFileList.GetSize() )
   {
      // --- Open current input file
      TObjString* tmp =
      dynamic_cast<TObjString*>(fInputFileList.At(fFileCounter));
      fFileName = tmp->GetString();

      LOG(INFO) << "Open the Flib input file " << fFileName << FairLogger::endl;
      // Check if the input file exist
      FILE* inputFile = fopen(fFileName.Data(), "r");
      if ( ! inputFile )  {
         LOG(ERROR) << "Input file " << fFileName << " doesn't exist." << FairLogger::endl;
         return kFALSE;
      }
      fclose(inputFile);
      fSource = new fles::TimesliceInputArchive(fFileName.Data());
      if ( !fSource) {
         LOG(ERROR) << "Could not open input file." << FairLogger::endl;
         return kFALSE;
      }
   } // if( fFileCounter < fInputFileList.GetSize() )
      else
      {
         LOG(INFO) << "End of files list reached: file counter is " << fFileCounter
                   << " for " << fInputFileList.GetSize() << " entries in the file list."
                   << FairLogger::endl;
         return kFALSE;
      } // else of if( fFileCounter < fInputFileList.GetSize() )

   return kTRUE;
}


ClassImp(CbmMcbm2018MsCrcCheck)
