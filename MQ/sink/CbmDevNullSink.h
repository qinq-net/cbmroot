/**
 * CbmDevNullSink.h
 *
 * @since 2017-11-17
 * @author F. Uhlig
 */

#ifndef CBMDEVNULLSINK_H_
#define CBMDEVNULLSINK_H_

#include "FairMQDevice.h"

class CbmDevNullSink : public FairMQDevice
{
  public:
    CbmDevNullSink();
    virtual ~CbmDevNullSink();

  protected:
    virtual void InitTask();
    virtual void Init();
    bool HandleData(FairMQMessagePtr&, int);

  private:
    uint64_t fNumMessages;
};

#endif /* CBMDEVNULLSINK_H_ */
