#ifndef CBMRICHTRBUNPACK_H
#define CBMRICHTRBUNPACK_H

#include "CbmRichTrbDefines.h"

#include "TObject.h"
#include <string>
#include <vector>

using namespace std;

class CbmRawSubEvent;
class CbmTrbRawHit;
class CbmTrbOutputHit;

class CbmRichTrbUnpack : public TObject
{
public:
	CbmRichTrbUnpack();
    virtual ~CbmRichTrbUnpack();
    void Run(const string& hldFileName);

private:

    vector<CbmTrbRawHit*> fRawRichHits;
    vector<CbmTrbRawHit*> fRawReferenceHits;

    vector<CbmTrbOutputHit*> fOutputRichHits;
    vector<CbmTrbOutputHit*> fOutputReferenceHits;

    void ReadEvents(void* data, int size);
    void ProcessTdc(CbmRawSubEvent* subEvent);
    void CreateOutputHits();
    void BuildEvent();
    void FindMinMaxIndex(Double_t x0, Int_t *indmin, Int_t *indmax);
    CbmTrbOutputHit* CreateOutputHit(CbmTrbRawHit* rawHit);
    void ClearAllBuffers();

    /*
     * Return time in ns.
     */
    Double_t GetFullTime(UInt_t epoch, UShort_t coarseTime, UShort_t fineTime);

    void DrawQa();
    
    void GenHistos();

    CbmRichTrbUnpack(const CbmRichTrbUnpack&){;}
    CbmRichTrbUnpack operator=(const CbmRichTrbUnpack&){;}

    ClassDef(CbmRichTrbUnpack,1)
};

#endif
