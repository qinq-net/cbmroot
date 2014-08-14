//! \author Michael Krieger

#include "TimesliceReader.hpp"
#include <unordered_map>

namespace spadic {

struct TimesliceReader::TimesliceReader_ {
    std::unordered_map<uint16_t, spadic::MessageReader> _readers;
    //< one MessageReader per CBMnet source address

    void add_mc(const flib_dpb::MicrosliceContents& mc)
    {
        for (auto& dtm : mc.dtms()) {
            auto& reader = _readers[dtm.data[0]];
            reader.add_buffer(dtm.data+1, dtm.size-1);
        }
    }
};

TimesliceReader::TimesliceReader() : _t {new TimesliceReader_} {}
TimesliceReader::~TimesliceReader() {}

void TimesliceReader::add_timeslice(const fles::Timeslice& ts)
{
  for (size_t c {0}; c < ts.num_components(); c++) {
    for (size_t m {0}; m < ts.num_microslices(c); m++) {
      auto& desc = ts.descriptor(c, m);
      auto p = ts.content(c, m);
      // TODO check sys_id, sys_version
      // TODO check same source address from different components
      _t->add_mc({p, desc.size});
    }
  }
}
  
void TimesliceReader::add_timeslice_cbmroot(const fles::Timeslice& ts,
					    size_t c)
{
  for (size_t m {0}; m < ts.num_microslices(c); m++) {
    auto& desc = ts.descriptor(c, m);
    auto p = ts.content(c, m);
    // TODO check same source address from different components
    _t->add_mc({p, desc.size});
  }
}

std::unordered_set<uint16_t> TimesliceReader::sources() const
{
    std::unordered_set<uint16_t> result;
    for (auto& item : _t->_readers) {
        auto addr = item.first;
        result.insert(addr);
    }
    return result;
}

std::unique_ptr<spadic::Message>
TimesliceReader::get_message(uint16_t source_addr) const
{
    return _t->_readers[source_addr].get_message();
}

} // namespace
