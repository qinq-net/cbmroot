#ifndef CBMRICHTRBUNPACK_H
#define CBMRICHTRBUNPACK_H

#include "CbmRichTrbDefines.h"

#include <TString.h>
#include "FairSource.h"
#include <vector>

using namespace std;

class CbmRichHitInfo;
class CbmRawSubEvent;
class CbmTrbRawHit;
class CbmTrbOutputHit;
class TClonesArray;
class TH1D;
class TH2D;

enum CbmRichAnaTypeEnum{
	kCbmRichBeamEvent,
	kCbmRichLaserPulserEvent,
	kCbmRichLedPulserEvent
};

class CbmRichTrbUnpack : public FairSource
{
public:
	/*
	 * Constructor.
	 * \param[in] hldFileName File name of input HLD file.
	 */
	CbmRichTrbUnpack(TString hldFileName);

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

    /*
     * Set what you want to analyze.
     */
    void SetAnaType(CbmRichAnaTypeEnum type){fAnaType = type;}

    /*
     * If TRUE histograms are drawn.
     */
    void SetDrawHist(Bool_t b){fDrawHist = b;}

private:
    TString fHldFileName; // file name of HLD file

    TClonesArray* fRichHits; // output array of RICH hits
    TClonesArray* fRichHitInfos; // output array of CbmRichHitInfo hits

    Char_t* fDataPointer; // pointer to data
    UInt_t fDataSize; // size of data
    UInt_t fEventNum; // current event number
    UInt_t fNofDoubleHits; // number of detected double hits

    vector<CbmTrbRawHit*> fRawRichHits; // raw hit from PMTs
    vector<CbmTrbRawHit*> fRawEventTimeHits; // raw hits from reference time TDC

    vector<CbmTrbOutputHit*> fOutputRichHits; // output hits from PMTs
    vector<CbmTrbOutputHit*> fOutputEventTimeHits; // output hits from reference time TDC

    // Debug histograms
    TH1D* fhChannelEntries[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC];
    TH1D* fhEpoch[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC];
    TH1D* fhCoarseTime[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC];
    TH1D* fhFineTime[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC];
    TH1D* fhDeltaT[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC];
    TH2D* fhNofRichHitsVsTrbNum; // Number of RICH hit per event vs. TRB number (only real event count)
    TH1D* fhDiffHitTimeEventTime; // Difference between reference event time and RICH hit time

    CbmRichAnaTypeEnum fAnaType; // What do you want to analyza beam events, laser events, led events

    Bool_t fDrawHist; // if TRUE histograms wre drawn

    /*
     * Read full input file to memory.
     */
    void ReadInputFileToMemory();

    /*
     * Add CbmRichHit to the output TClonesArray.
     * \param x X coordinate of the hit
     * \param y Y coordinate of the hit
     */
    void AddRichHitToOutputArray(UShort_t trbId, CbmRichHitInfo* hitInfo);

    /*
     * Read all events and create output hits
     */
    void ReadEvents();

    /*
     * Process TDC message for subevent
     * \param subEvent Poiner to subEvent
     */
    void ProcessTdc(
    		CbmRawSubEvent* subEvent);
    /*
     * Decode TDC data.
     * \param data Array of TDC data words.
     * \param size Number of words.
     * \param trbId TRB ID.
     * \param tdcId TDC ID.
     */
    void DecodeTdcData(
    		UInt_t* data,
    		UInt_t size,
    		UInt_t trbId,
    		UInt_t tdcId);

    /*
     * Create CbmTrbOutputHits from Raw hits
     */
    void CreateOutputHits();

    /*
     * Create CbmRichTrbOutputHit out of CbmTrbRawHit.
     */
    CbmTrbOutputHit* CreateOutputHit(
    		CbmTrbRawHit* rawHit);

    /*
     * Build event (create CbmRichHits) for the specified reference time hit.
     * \param refHitIndex Index of reference time hit.
     */
    void BuildEvent(
    		Int_t refHitIndex);

    /*
     * Find min and max indecies of the time corridor in the output CbmRichTrbOutputHit array sorted by time.
     * \param x0 Reference time.
     * \param windowT Time window in ns.
     * \param[out] indmin Min index.
     * \param[out] indmax Max index.
     */
    void FindMinMaxIndex(
    		Double_t x0,
    		Double_t windowT,
    		Int_t *indmin,
    		Int_t *indmax);

    /*
     * Clear all buffers and used memory.
     */
    void ClearAllBuffers();

    /*
     * Return time in ns.
     */
    Double_t GetFullTime(
    		UShort_t TRB,
    		UShort_t TDC,
    		UShort_t CH,
            UInt_t epoch,
            UShort_t coarseTime,
            UShort_t fineTime);

    /*
     * Create and draw debug histogramms.
     */
    void CreateAndDrawQa();

    /*
     * Fill histograms with CbmTrbRawHit data
     */
    void FillRawHitHist(CbmTrbRawHit* rh);

    /*
	 * Fill histograms with CbmTrbOutputHit data
	 */
	void FillOutputHitHist(CbmTrbOutputHit* outHit);

	/*
	 * Create and draw event display for event building.
	 */
	void CreateAndDrawEventBuildDisplay();


    CbmRichTrbUnpack(const CbmRichTrbUnpack&){;}
    CbmRichTrbUnpack operator=(const CbmRichTrbUnpack&){;}

    ClassDef(CbmRichTrbUnpack,1)
};

#endif
