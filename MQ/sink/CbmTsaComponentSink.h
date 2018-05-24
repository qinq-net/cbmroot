/**
 * CbmTsaComponentSink.h
 *
 * @since 2018-04-24
 * @author F. Uhlig
 */

#ifndef CBMTSACOMPONENTSINK_H_
#define CBMTSACOMPONENTSINK_H_

#include "FairMQDevice.h"

#include "Timeslice.hpp"
#include "MicrosliceDescriptor.hpp"

class CbmTsaComponentSink: public FairMQDevice
{
  public:
    CbmTsaComponentSink();
    virtual ~CbmTsaComponentSink();

  protected:
    virtual void InitTask();
    bool HandleData(FairMQMessagePtr&, int);

  private:
    uint64_t fNumMessages;

    std::vector<std::string> fAllowedChannels
          = {"stscomponent","tofcomponent","trdcomponent"};

    bool CheckTimeslice(const fles::Timeslice& ts);
    void PrintMicroSliceDescriptor(const fles::MicrosliceDescriptor& mdsc);
    bool IsChannelNameAllowed(std::string channelName);
};

#endif /* CBMTSACOMPONENTSINK_H_ */
