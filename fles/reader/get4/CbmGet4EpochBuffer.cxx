// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                      CbmGet4EpochBuffer                           -----
// -----                    Created 24.02.2015 by                          -----
// -----                        P.-A. Loizeau                              -----
// -----                                                                   -----
// -----------------------------------------------------------------------------


#include <iostream>
#include <iomanip>

#include "TString.h"

#include "FairLogger.h"

#include "CbmGet4EpochBuffer.h"

/*
 * TODO:
 * 1) Find proper default values for fuBufferMaxSize, fuBufferMaxEpochSpread
 *    and fuBufferHighWater
 */

// -----   Initialization of static variables   ------------------------------
CbmGet4EpochBuffer* CbmGet4EpochBuffer::fgInstance = NULL;
// ---------------------------------------------------------------------------

// -----   Constructor   -----------------------------------------------------
CbmGet4EpochBuffer::CbmGet4EpochBuffer() :
      fData(),
      fuBufferMaxSize(200),
      fuBufferMaxEpochSpread(100),
      fuBufferHighWater(160)
{
   LOG(INFO) << "CbmGet4EpochBuffer::CbmGet4EpochBuffer = Starting epoch buffer with: "
             << FairLogger::endl;
   LOG(INFO) << fuBufferHighWater << " stored epochs for buffer high water "
             << FairLogger::endl;
   LOG(INFO) << fuBufferMaxSize << " stored epochs for maximum buffer storage "
             << FairLogger::endl;
   LOG(INFO) << fuBufferMaxEpochSpread << " for epoch spread rejection when maximum reached "
             << FairLogger::endl;
   PrintStatus();
}
// ---------------------------------------------------------------------------

// -----   Destructor   ------------------------------------------------------
CbmGet4EpochBuffer::~CbmGet4EpochBuffer()
{
}
// ---------------------------------------------------------------------------

// -----   Access to next epoch   ---------------------------------------------
std::multiset< get4v1x::FullMessage >* CbmGet4EpochBuffer::GetNextEpoch()
{
  // --- Check for empty buffer
  if( !fData.size() )
     return NULL;

  // --- Get data from buffer
  std::multiset< get4v1x::FullMessage >* multiset = fData.front();
  fData.pop_front();
  return multiset;

}
// ---------------------------------------------------------------------------

// -----   Insert Epoch into buffer   -----------------------------------------
Int_t CbmGet4EpochBuffer::InsertEpoch( std::multiset< get4v1x::FullMessage >* epochCont )
{

  if( !epochCont )
     LOG(FATAL)<<"CbmGet4EpochBuffer:InsertEpoch = Invalid epoch multiset pointer"
                           << FairLogger::endl;
/*
  if( 100000 < epochCont->size() )
     LOG(INFO) << "CbmGet4EpochBuffer::InsertEpoch = Inserting epoch multiset, epoch idx "
                 << ( epochCont->begin() )->getExtendedEpoch()
                 << " New buffer size: " << fData.size()
                 << " First epoch: "<< GetEpochFirst()
                 << " Last epoch: "<< GetEpochLast()
                 << FairLogger::endl;
*/
  if( 0 < epochCont->size() )
  {
     fData.push_back(epochCont);
     LOG(DEBUG2) << "CbmGet4EpochBuffer::InsertEpoch = Inserting epoch multiset, epoch idx "
//     LOG(INFO) << "CbmGet4EpochBuffer::InsertEpoch = Inserting epoch multiset, epoch idx "
                 << ( epochCont->begin() )->getExtendedEpoch()
                 << " New buffer size: " << fData.size()
                 << " First epoch: "<< GetEpochFirst()
                 << " Last epoch: "<< GetEpochLast()
                 << FairLogger::endl;

     if( fuBufferHighWater == fData.size() )
     {
        LOG(INFO) << "CbmGet4EpochBuffer::InsertEpoch = High water mark reached,  "
                    << fuBufferHighWater << "/" << fuBufferMaxSize
                    << " epoch stored in buffer."<< FairLogger::endl;
        LOG(INFO) << "                                  If MAX reached, epoch older than "
                  << fuBufferMaxEpochSpread << " epoch from the new one will be dropped!"
                  << FairLogger::endl;
     } // if( fuBufferHighWater == fData.size() )
     if( fuBufferMaxSize <= fData.size() )
     {
        LOG(WARNING) << "CbmGet4EpochBuffer::InsertEpoch = Max buffer size reached. "
                     << "Now dropping all epochs more than " << fuBufferMaxEpochSpread
                     << " epochs older than the newly inserted one!!!"
                     << FairLogger::endl;

        // now drop all epochs too old
        // WARNING: in case of epoch index cycle (should never happen but well...) the
        //          "older" epoch is also thrown out
        // Loop stop at least when reaching the newly inserted epoch
        while( ( ((fData.front())->begin() )->getExtendedEpoch() + fuBufferMaxEpochSpread
                    < ( epochCont->begin() )->getExtendedEpoch() ) ||
               ( ( ( epochCont->begin() )->getExtendedEpoch() )
                     < ((fData.front())->begin() )->getExtendedEpoch() )
              )
        {
           // Throw out the epoch: clear multiset, delete multiset, pop multiset pointer
           (fData.front())->clear();
           delete (fData.front());
           fData.pop_front();
        } // while first epoch has to be thrown out
     } // if( fuBufferMaxSize <= fData.size() )
  } // if( 0 < epochCont->size() )
     else
     {
        LOG(DEBUG2) << "CbmGet4EpochBuffer: Trying to insert an empty epoch multiset => do nothing "
//        LOG(INFO) << "CbmGet4EpochBuffer: Trying to insert an empty epoch multiset => do nothing "
                    << FairLogger::endl;
        epochCont->clear();
        delete epochCont;
     } // else of if( 0 < epochCont->size() )

   return fData.size();
}
// ---------------------------------------------------------------------------



// -----   Instance   --------------------------------------------------------
CbmGet4EpochBuffer* CbmGet4EpochBuffer::Instance() {
  if ( ! fgInstance ) fgInstance = new CbmGet4EpochBuffer();
  return fgInstance;
}
// ---------------------------------------------------------------------------



// -----   Print status   ----------------------------------------------------
void CbmGet4EpochBuffer::PrintStatus() const {
  Int_t size = GetSize();
  LOG(INFO) << "CbmGet4EpochBuffer: Status ";
  if( !size )
  {
    LOG(INFO) << "empty" << FairLogger::endl;
    return;
  } // if( !size )
  LOG(INFO) << GetSize() << " epoch multisets from epoch "
            << Form("%9llu to epoch ", GetEpochFirst() )
            << Form("%9llu ", GetEpochLast() ) << FairLogger::endl;
}
// ---------------------------------------------------------------------------

