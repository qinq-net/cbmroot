/**
 * CbmDevNullSink.cxx
 *
 * @since 2017-11-17
 * @author F. Uhlig
 */

#include "CbmDevNullSink.h"
#include "FairMQLogger.h"
#include "FairMQProgOptions.h" // device->fConfig

using namespace std;

CbmDevNullSink::CbmDevNullSink()
    : FairMQDevice{}
    , fNumMessages{0}
{
}

void CbmDevNullSink::Init()
{

}

void CbmDevNullSink::InitTask()
{
  // register a handler for data arriving on any channel
  int noChannel = fChannels.size();
  LOG(INFO) << "Number of defined input channels: " << noChannel;
  for(auto const &entry : fChannels) {
    LOG(INFO) << "Channel name: " << entry.first;
    OnData(entry.first, &CbmDevNullSink::HandleData);
  }
}

// handler is called whenever a message arrives on "data", with a reference to the message and a sub-channel index (here 0)
bool CbmDevNullSink::HandleData(FairMQMessagePtr& msg, int /*index*/)
{
// Don't do anything with the data
// Maybe add an message counter which counts the incomming messages and add
// an output
    fNumMessages++;
    LOG(INFO) << "Received message number "<<  fNumMessages 
              << " with size " << msg->GetSize(); 
    return true;
}

CbmDevNullSink::~CbmDevNullSink()
{
}
