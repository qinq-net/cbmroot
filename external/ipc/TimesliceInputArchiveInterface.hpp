// Copyright 2019 Florian Uhligd <f.uhlig@gsi.de>
#pragma once

#include "TimesliceSource.hpp"
#include "log.hpp"
#include <chrono>
#include <memory>
#include <vector>
#include <string>

class TimesliceInputArchiveInterface {
public:
  TimesliceInputArchiveInterface(std::string);

  TimesliceInputArchiveInterface(const TimesliceInputArchiveInterface&) = delete;
  void operator=(const TimesliceInputArchiveInterface&) = delete;

  ~TimesliceInputArchiveInterface();

  std::unique_ptr<const fles::Timeslice> GetNextTimeslice();

private:

  void InitTimesliceArchiveInterface();
  void CreateInputFileList(std::string);
  bool OpenNextFile(int);

  void run();

  std::vector<std::unique_ptr<fles::TimesliceSource>> source_;


  std::vector<std::vector<std::string>> InputFileList;

  std::vector<std::unique_ptr<const fles::Timeslice>> timesliceCont;

  std::set<std::pair<uint64_t,int>> sortedSource_;
  
  logging::OstreamLog status_log_{status};
  logging::OstreamLog debug_log_{debug};

  std::chrono::high_resolution_clock::time_point time_begin_;

};
