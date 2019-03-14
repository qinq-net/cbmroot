/**
 *  CbmMQTsaMultiSampler.cpp
 *
 * @since 2017-11-17
 * @author F. Uhlig
 */


#include "CbmMQTsaMultiSampler.h"
#include "FairMQLogger.h"
#include "FairMQProgOptions.h" // device->fConfig

#include "TimesliceSubscriber.hpp"
#include "TimesliceInputArchive.hpp"
#include "TimesliceMultiInputArchive.hpp"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

namespace filesys = boost::filesystem;

#include <stdio.h>
#include <ctime>
#include <thread> // this_thread::sleep_for
#include <chrono>
#include <algorithm>
#include <string>

using namespace std;

#include <stdexcept>

struct InitTaskError : std::runtime_error { using std::runtime_error::runtime_error; };

CbmMQTsaMultiSampler::CbmMQTsaMultiSampler()
    : FairMQDevice()
    , fMaxTimeslices(0)
    , fFileName("")
    , fDirName("")
    , fInputFileList()
    , fFileCounter(0)
    , fHost("")
    , fPort(0)
    , fTSNumber(0)
    , fTSCounter(0)
    , fMessageCounter(0)
    , fSource(nullptr)
    , fTime()
{
}

void CbmMQTsaMultiSampler::InitTask()
try
{
    // Get the values from the command line options (via fConfig)
    fFileName = fConfig->GetValue<string>("filename");
    fDirName = fConfig->GetValue<string>("dirname");
    fHost = fConfig->GetValue<string>("flib-host");
    fPort = fConfig->GetValue<uint64_t>("flib-port");
    fMaxTimeslices = fConfig->GetValue<uint64_t>("max-timeslices");

    // Check which input is defined 
    // Posibilities
    // filename && ! dirname : single file
    // filename with wildcards && diranme : all files with filename regex in the directory
    // host && port : connect to the flim server

    bool isGoodInputCombi{false};
    if ( 0 != fFileName.size() &&  0 == fDirName.size() && 0 == fHost.size() && 0 == fPort ) {
      isGoodInputCombi=true;  
      fInputFileList.push_back(fFileName);
    } else if ( 0 == fFileName.size() &&  0 == fDirName.size() && 0 != fHost.size() && 0!= fPort) {
      isGoodInputCombi=true;
      LOG(INFO) << "Host: " << fHost;
      LOG(INFO) << "Port: " << fPort;
    } else {
      isGoodInputCombi=false;
    }

 
    if (!isGoodInputCombi) {
      throw InitTaskError("Wrong combination of inputs. Either file or wildcard file + directory or host + port are allowed combination.");
    }


    LOG(INFO) << "MaxTimeslices: " << fMaxTimeslices;

    // Get the information about created channels from the device
    // Check if the defined channels from the topology (by name)
    // are in the list of channels which are possible/allowed
    // for the device
    // The idea is to check at initilization if the devices are
    // properly connected. For the time beeing this is done with a
    // nameing convention. It is not avoided that someone sends other
    // data on this channel.
    int noChannel = fChannels.size();
    LOG(INFO) << "Number of defined output channels: " << noChannel;
    for(auto const &entry : fChannels) {
      LOG(INFO) << "Channel name: " << entry.first;
      if (!IsChannelNameAllowed(entry.first)) throw InitTaskError("Channel name does not match.");
    }

    for(auto const& value: fComponentsToSend) {
      LOG(INFO) << "Value : " << value;
      if (value > 1) {
        throw InitTaskError("Sending same data to more than one output channel not implemented yet.");
      }
    }


  if ( 0 == fFileName.size() && 0 != fHost.size() ) {
    std::string connector = "tcp://" + fHost + ":" + std::to_string(fPort);
    LOG(INFO) << "Open TSPublisher at " << connector;
    fSource = new fles::TimesliceSubscriber(connector);
  } else {
    // Create a ";" separated string with all file names 
    std::string fileList{""};
    for(const auto obj: fInputFileList) {
      std::string fileName = obj;
      fileList += fileName; 
      fileList += ";";
    }
    fileList.pop_back(); // Remove the last ;
    LOG(INFO) << "Input File String: " << fileList;
    fSource = new fles::TimesliceMultiInputArchive(fileList);
    if ( !fSource) {
      throw InitTaskError("Could open files from file list.");
    }
  }

  fTime = std::chrono::steady_clock::now();
} catch (InitTaskError& e) {
 LOG(ERROR) << e.what();
 ChangeState(ERROR_FOUND);
}

/*
bool CbmMQTsaMultiSampler::OpenNextFile() 
{ 
  // First Close and delete existing source
  if( nullptr != fSource )
    delete fSource;
    
  if (fInputFileList.size() > 0) {
    fFileName = fInputFileList[0];
    fInputFileList.erase(fInputFileList.begin());
    LOG(INFO) << "Open the Flib input file " << fFileName;
    filesys::path pathObj(fFileName);
    if ( ! filesys::is_regular_file(pathObj) ) {
      LOG(ERROR) << "Input file " << fFileName << " doesn't exist.";
      return false;
    }
    fSource = new fles::TimesliceInputArchive(fFileName);
    if ( !fSource) {
      LOG(ERROR) << "Could not open input file.";
      return false;
    }
  } else {
    LOG(INFO) << "End of files list reached.";
    return false;
  } 
  return true;
}
*/

bool CbmMQTsaMultiSampler::IsChannelNameAllowed(std::string channelName)
{

  for(auto const &entry : fAllowedChannels) {
    std::size_t pos1 = channelName.find(entry);
    if (pos1!=std::string::npos) {
      const vector<std::string>::const_iterator pos =
         std::find(fAllowedChannels.begin(), fAllowedChannels.end(), entry);
      const vector<std::string>::size_type idx = pos-fAllowedChannels.begin();
      LOG(INFO) << "Found " << entry << " in " << channelName;
      LOG(INFO) << "Channel name " << channelName
              << " found in list of allowed channel names at position " << idx;
      fComponentsToSend[idx]++;
      fChannelsToSend[idx].push_back(channelName);
      return true;
    }
  }
  LOG(INFO) << "Channel name " << channelName
            << " not found in list of allowed channel names.";
  LOG(ERROR) << "Stop device.";
  return false;
}

bool CbmMQTsaMultiSampler::ConditionalRun()
{


  auto timeslice = fSource->get();
  if (timeslice) {
    if (fTSCounter < fMaxTimeslices) {
      fTSCounter++;
      if (fTSCounter % 10000 == 0)  {
        LOG(INFO) << "Analyse Event " << fTSCounter;
      }

      const fles::Timeslice& ts = *timeslice;
//      auto tsIndex = ts.index();


      LOG(INFO) << "Found " << ts.num_components()
                << " different components in timeslice";


      CheckTimeslice(ts);

      for (unsigned int nrComp = 0; nrComp < ts.num_components(); ++nrComp) {
        CreateAndSendComponent(ts, nrComp);
      }
      return true;
    } else {
      CalcRuntime();
      return false;
    }
  } else {
    CalcRuntime();
    return false;
  }

}

bool CbmMQTsaMultiSampler::CreateAndSendComponent(const fles::Timeslice& ts, int nrComp)
{

  // Check if component has to be send. If the corresponding channel
  // is connected create the new timeslice and send it to the
  // correct channel

  LOG(INFO) << "SysID: " << static_cast<int>(ts.descriptor(nrComp,0).sys_id);
  const vector<int>::const_iterator pos =
     std::find(fSysId.begin(), fSysId.end(), static_cast<int>(ts.descriptor(nrComp,0).sys_id));
  if (pos != fSysId.end() ) {
    const vector<std::string>::size_type idx = pos-fSysId.begin();
    if (fComponentsToSend[idx]>0) {
      LOG(INFO) << "Create timeslice component for link " << nrComp;

      fles::StorableTimeslice component{static_cast<uint32_t>(ts.num_microslices(nrComp), ts.index())};
      component.append_component(ts.num_microslices(0));

      for (size_t m = 0; m < ts.num_microslices(nrComp); ++m) {
        component.append_microslice( 0, m, ts.descriptor(nrComp, m), ts.content(nrComp, m) );
      }
      if ( ! SendData(component, idx) ) return false;
      return true;
    }
  }
  return true;
}

bool CbmMQTsaMultiSampler::SendData(const fles::StorableTimeslice& component, int idx)
{ 
  // serialize the timeslice and create the message
  std::stringstream oss;
  boost::archive::binary_oarchive oa(oss);
  oa << component;
  std::string* strMsg = new std::string(oss.str());

  FairMQMessagePtr msg(NewMessage(const_cast<char*>(strMsg->c_str()), // data
                                                    strMsg->length(), // size
                                                    [](void* /*data*/, void* object){ delete static_cast<std::string*>(object); },
                                                    strMsg)); // object that manages the data

  // TODO: Implement sending same data to more than one channel
  // Need to create new message (copy message??)
  if (fComponentsToSend[idx]>1) {
    LOG(INFO) << "Need to copy FairMessage";
  }

  // in case of error or transfer interruption,
  // return false to go to IDLE state
  // successfull transfer will return number of bytes
  // transfered (can be 0 if sending an empty message).

  LOG(INFO) << "Send data to channel " << fChannelsToSend[idx][0];
  if (Send(msg, fChannelsToSend[idx][0]) < 0) {
    LOG(ERROR) << "Problem sending data";
    return false;
  }

  fMessageCounter++;
  LOG(INFO) << "Send message " << fMessageCounter << " with a size of "
            << msg->GetSize();

  return true;
}


CbmMQTsaMultiSampler::~ CbmMQTsaMultiSampler()
{
}

void CbmMQTsaMultiSampler::CalcRuntime()
{
  std::chrono::duration<double> run_time =
     std::chrono::steady_clock::now() - fTime;

  LOG(INFO) << "Runtime: " << run_time.count();
  LOG(INFO) << "No more input data";
}



void CbmMQTsaMultiSampler::PrintMicroSliceDescriptor(const fles::MicrosliceDescriptor& mdsc)
{
  LOG(INFO) << "Header ID: Ox" << std::hex << static_cast<int>(mdsc.hdr_id)
            << std::dec;
  LOG(INFO) << "Header version: Ox" << std::hex << static_cast<int>(mdsc.hdr_ver)
            << std::dec;
  LOG(INFO) << "Equipement ID: " << mdsc.eq_id;
  LOG(INFO) << "Flags: " << mdsc.flags;
  LOG(INFO) << "Sys ID: Ox" << std::hex << static_cast<int>(mdsc.sys_id)
            << std::dec;
  LOG(INFO) << "Sys version: Ox" << std::hex << static_cast<int>(mdsc.sys_ver)
            << std::dec;
  LOG(INFO) << "Microslice Idx: " << mdsc.idx;
  LOG(INFO) << "Checksum: " << mdsc.crc;
  LOG(INFO) << "Size: " << mdsc.size;
  LOG(INFO) << "Offset: " << mdsc.offset;
}

bool CbmMQTsaMultiSampler::CheckTimeslice(const fles::Timeslice& ts)
{
  if ( 0 == ts.num_components() ) {
    LOG(ERROR) << "No Component in TS " << ts.index();
    return 1;
  }
  LOG(INFO) << "Found " << ts.num_components()
            << " different components in timeslice";
  
  for (size_t c = 0; c < ts.num_components(); ++c) {
    LOG(INFO) << "Found " << ts.num_microslices(c) 
              << " microslices in component " << c;
    LOG(INFO) << "Component " << c << " has a size of "
              << ts.size_component(c) << " bytes";
    LOG(INFO) << "Component " << c << " has the system id 0x"
              << std::hex << static_cast<int>(ts.descriptor(c,0).sys_id)
              << std::dec;
    LOG(INFO) << "Component " << c << " has the system id 0x"
              << static_cast<int>(ts.descriptor(c,0).sys_id);

/*
    for (size_t m = 0; m < ts.num_microslices(c); ++m) {
      PrintMicroSliceDescriptor(ts.descriptor(c,m));
    }
*/
  }

  return true;
}
