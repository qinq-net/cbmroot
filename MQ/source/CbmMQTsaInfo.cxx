/**
 *  CbmMQTsaInfo.cpp
 *
 * @since 2017-11-17
 * @author F. Uhlig
 */


#include <thread> // this_thread::sleep_for
#include <chrono>

#include "CbmMQTsaInfo.h"
#include "FairMQLogger.h"
#include "FairMQProgOptions.h" // device->fConfig

#include "TimesliceSubscriber.hpp"
#include "TimesliceInputArchive.hpp"

#include <boost/archive/binary_oarchive.hpp>

#include <stdio.h>
#include <ctime>

using namespace std;

#include <stdexcept>

struct InitTaskError : std::runtime_error { using std::runtime_error::runtime_error; };


CbmMQTsaInfo::CbmMQTsaInfo()
    : FairMQDevice()
    , fMaxTimeslices(0)
    , fFileName("")
    , fInputFileList()
    , fFileCounter(0)
    , fHost("")
    , fPort(0)
    , fTSNumber(0)
    , fTSCounter(0)
    , fMessageCounter(0)
    , fTime()
{
}

void CbmMQTsaInfo::InitTask()
try
{
    // Get the values from the command line options (via fConfig)
    fFileName = fConfig->GetValue<string>("filename");
    fHost = fConfig->GetValue<string>("flib-host");
    fPort = fConfig->GetValue<uint64_t>("flib-port");
    fMaxTimeslices = fConfig->GetValue<uint64_t>("max-timeslices");


    LOG(INFO) << "Filename: " << fFileName;
    LOG(INFO) << "Host: " << fHost;
    LOG(INFO) << "Port: " << fPort;

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

  if ( 0 == fFileName.size() && 0 != fHost.size() ) {
    std::string connector = "tcp://" + fHost + ":" + std::to_string(fPort);
    LOG(INFO) << "Open TSPublisher at " << connector;
    fSource = new fles::TimesliceSubscriber(connector);
    if ( !fSource) {
      throw InitTaskError("Could not connect to publisher.");
    }
  } else {
    LOG(INFO) << "Open the Flib input file " << fFileName;
    // Check if the input file exist
    FILE* inputFile = fopen(fFileName.c_str(), "r");
    if ( ! inputFile )  {
      throw InitTaskError("Input file doesn't exist.");
    }
    fclose(inputFile);
    fSource = new fles::TimesliceInputArchive(fFileName);
    if ( !fSource) {
      throw InitTaskError("Could not open input file.");
    }

  }
  fTime = std::chrono::steady_clock::now();
} catch (InitTaskError& e) {
 LOG(ERROR) << e.what();
 ChangeState(ERROR_FOUND);
}

bool CbmMQTsaInfo::IsChannelNameAllowed(std::string channelName)
{
  if ( std::find(fAllowedChannels.begin(), fAllowedChannels.end(),
       channelName) != fAllowedChannels.end() ) {
    LOG(INFO) << "Channel name " << channelName
              << " found in list of allowed channel names.";
    return true;
  } else {
    LOG(INFO) << "Channel name " << channelName
              << " not found in list of allowed channel names.";
    LOG(ERROR) << "Stop device.";
    return false;
  }
}

bool CbmMQTsaInfo::ConditionalRun()
{


  auto timeslice = fSource->get();
  if (timeslice) {
    fTSCounter++;
    if (fTSCounter % 10000 == 0)  LOG(INFO) << "Analyse Event " << fTSCounter;


    const fles::Timeslice& ts = *timeslice;
    auto tsIndex = ts.index();


    LOG(INFO) << "Found " << ts.num_components()
              << " different components in timeslice";

    CheckTimeslice(ts);

    if (fTSCounter < fMaxTimeslices) {
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


CbmMQTsaInfo::~ CbmMQTsaInfo()
{
}

void CbmMQTsaInfo::CalcRuntime()
{
  std::chrono::duration<double> run_time =  
     std::chrono::steady_clock::now() - fTime;

  LOG(INFO) << "Runtime: " << run_time.count();
  LOG(INFO) << "No more input data";
}



void CbmMQTsaInfo::PrintMicroSliceDescriptor(const fles::MicrosliceDescriptor& mdsc)
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

bool CbmMQTsaInfo::CheckTimeslice(const fles::Timeslice& ts)
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

/*
    for (size_t m = 0; m < ts.num_microslices(c); ++m) {
      PrintMicroSliceDescriptor(ts.descriptor(c,m));
    }
*/
  }

  return true;
}
