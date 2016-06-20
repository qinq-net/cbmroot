// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSUnpackTest                            -----
// -----               Created 07.11.2014 by F. Uhlig                      -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTSUnpackTest.h"
#include "rocMess_wGet4v1.h"

#include "FairLogger.h"
#include "FairRootManager.h"

#include "TClonesArray.h"


#include <iostream>
#include <stdint.h>

CbmTSUnpackTest::CbmTSUnpackTest()
  : CbmTSUnpack()
{
}

CbmTSUnpackTest::~CbmTSUnpackTest()
{
}

Bool_t CbmTSUnpackTest::Init()
{
  LOG(INFO) << "Initializing flib nxyter unpacker" << FairLogger::endl;

  return kTRUE;
}

Bool_t CbmTSUnpackTest::DoUnpack(const fles::Timeslice& ts, size_t component)
{

  LOG(INFO) << "Timeslice contains " << ts.num_microslices(component)
            << "microslices." << FairLogger::endl;
  
  // Loop over microslices
  for (size_t m = 0; m < ts.num_microslices(component); ++m)
    {

      constexpr uint32_t kuBytesPerMessage = 8;

      auto msDescriptor = ts.descriptor(component, m);
      const uint8_t* msContent = reinterpret_cast<const uint8_t*>(ts.content(component, m));
      //      uint8_t* msContent = reinterpret_cast<uint8_t*>(ts.content(component, m));

      uint32_t size = msDescriptor.size;
      LOG(INFO) << "Microslice: " << msDescriptor.idx 
		<< " has size: " << size << FairLogger::endl; 
      // If not integer number of message in input buffer, print warning/error
      if( 0 != (size % kuBytesPerMessage) )
	LOG(ERROR) << "The input microslice buffer does NOT contain only complete nDPB messages!" << FairLogger::endl;

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
	  mess.printDataCout();
	  // Do things with message
	  
	} // for (uint32_t uIdx = 0; uIdx < uNbMessages; uIdx ++)
      
    }


  return kTRUE;
}

void CbmTSUnpackTest::Reset()
{
}

ClassImp(CbmTSUnpackTest)
