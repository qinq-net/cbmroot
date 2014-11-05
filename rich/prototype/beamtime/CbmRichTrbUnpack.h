#ifndef CBMRICHTRBUNPACK_H
#define CBMRICHTRBUNPACK_H


#include "TObject.h"
#include <string>
#include <vector>

using namespace std;

class CbmRawSubEvent;
class CbmTrbRawHit;


class CbmRichTrbUnpack : public TObject
{
public:
	CbmRichTrbUnpack();
    virtual ~CbmRichTrbUnpack();
    void Run(const string& hldFileName);

private:

    vector<CbmTrbRawHit*> fRawTrbHits;

    void ReadEvents(void* data, int size);
    void ProcessTdc(CbmRawSubEvent* subEvent);

    CbmRichTrbUnpack(const CbmRichTrbUnpack&){;}
    CbmRichTrbUnpack operator=(const CbmRichTrbUnpack&){;}

    ClassDef(CbmRichTrbUnpack,1)
};

#endif
