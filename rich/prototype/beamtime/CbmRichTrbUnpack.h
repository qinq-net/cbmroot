#ifndef CBMRICHTRBUNPACK_H
#define CBMRICHTRBUNPACK_H

#include "CbmRichTrbDefines.h"
#include "FairSource.h"
#include <string>
#include <vector>

using namespace std;

class CbmRawSubEvent;
class CbmTrbRawHit;
class CbmTrbOutputHit;
class TClonesArray;

class CbmRichTrbUnpack : public FairSource
{
public:
	/*
	 * Constructor.
	 * \param[in] hldFileName File name of input HLD file.
	 */
	CbmRichTrbUnpack(const string& hldFileName);

	/*
	 * Destructor.
	 */
    virtual ~CbmRichTrbUnpack();

    /*
     * Inherited from FairSource.
     */
    Bool_t Init();

    /*
     * Inherited from FairSource.
     */
    Int_t ReadEvent();

    /*
     * Inherited from FairSource.
     */
    void Close();

    /*
     * Inherited from FairSource.
     */
    void Reset();

private:
    string fHldFileName; // file name of HLD file

    TClonesArray* fRichHits; // output array of RICH hits


    void* fDataPointer; // pointer to data
    UInt_t fDataSize; // size of data
    UInt_t fEventNum; // current event number

    vector<CbmTrbRawHit*> fRawRichHits;
    vector<CbmTrbRawHit*> fRawReferenceHits;

    vector<CbmTrbOutputHit*> fOutputRichHits;
    vector<CbmTrbOutputHit*> fOutputReferenceHits;

    /*
     * Read full input file to memory.
     */
    void ReadInputFileToMemory();

    /*
     * Add CbmRichHit to hte output array.
     * \param x X coordinate of the hit
     * \param y Y coordinate of the hit
     */
    void AddRichHitToOutputArray(Double_t x, Double_t y);

    void ReadEvents();

    void ProcessTdc(CbmRawSubEvent* subEvent);

    void CreateOutputHits();

    void BuildEvent(Int_t refHitIndex);

    void FindMinMaxIndex(Double_t x0, Int_t *indmin, Int_t *indmax);

    CbmTrbOutputHit* CreateOutputHit(CbmTrbRawHit* rawHit);

    void ClearAllBuffers();

    /*
     * Return time in ns.
     */
    Double_t GetFullTime(UShort_t TRB, UShort_t TDC, UShort_t CH,
                         UInt_t epoch, UShort_t coarseTime, UShort_t fineTime);

    void DrawQa();
    
    void GenHistos();

    CbmRichTrbUnpack(const CbmRichTrbUnpack&){;}
    CbmRichTrbUnpack operator=(const CbmRichTrbUnpack&){;}

    ClassDef(CbmRichTrbUnpack,1)
};

#endif
