// Copyright 2019 Florian Uhlig <f.uhlig@gsi.de>
/// \file
/// \brief Defines the fles::TimesliceMultiInputArchive class.
#pragma once


#include "TimesliceSource.hpp"
#include "StorableTimeslice.hpp"
#include "log.hpp"
#include <chrono>
#include <memory>
#include <vector>
#include <string>

namespace fles {
/**
 * \brief The TimesliceMultiInputArchive class reads timeslice data from
 * several TimesliceInputArchives and returns the timslice with the 
 * smallest index.
 */
  class TimesliceMultiInputArchive : public TimesliceSource {
public:
  /// Construct timeslice subscriber receiving from given ZMQ address.
  explicit TimesliceMultiInputArchive(const std::string&);

  /// Delete copy constructor (non-copyable).
  TimesliceMultiInputArchive(const TimesliceMultiInputArchive&) = delete;
  /// Delete assignment operator (non-copyable).
  void operator=(const TimesliceMultiInputArchive&) = delete;

  ~TimesliceMultiInputArchive() override = default;

  /**
   * \brief Retrieve the next item.
   *
   * \return pointer to the item, or nullptr if no more
   * timeslices available in the input archives
   */
  std::unique_ptr<Timeslice> get() {
    return (GetNextTimeslice());
  };

  bool eos() const override { return sortedSource_.size() == 0; }

private:
  Timeslice* do_get() override;

  void InitTimesliceArchive();
  void CreateInputFileList(std::string);
  bool OpenNextFile(int);
  std::unique_ptr<Timeslice> GetNextTimeslice();
  
  std::vector<std::unique_ptr<TimesliceSource>> source_;

  std::vector<std::vector<std::string>> InputFileList;

  std::vector<std::unique_ptr<Timeslice>> timesliceCont;

  std::set<std::pair<uint64_t,int>> sortedSource_;
  
  logging::OstreamLog status_log_{status};
  logging::OstreamLog debug_log_{debug};

};

} // namespace fles
