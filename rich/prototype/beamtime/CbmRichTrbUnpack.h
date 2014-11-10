#ifndef CBMRICHTRBUNPACK_H
#define CBMRICHTRBUNPACK_H

#include "CbmRichTrbDefines.h"

#include "TObject.h"
#include "TH1I.h"
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

    //CbmTrbCalibrator* fCalibrator;

    ///// Histos for calibration
    TH1I* hLeadingFine[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];
    TH1I* hLeadingFineBuffer[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];
    TH1I* hTrailingFine[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];
    TH1I* hTrailingFineBuffer[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];
    ////////////////////////////

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
