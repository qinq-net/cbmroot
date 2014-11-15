#ifndef CBMRICHTRBRECOQA_H
#define CBMRICHTRBRECOQA_H


#include "FairTask.h"

class TClonesArray;
class TH1D;
class TH2D;

class CbmRichTrbRecoQa : public FairTask
{
public:
	/*
	 * Constructor.
	 */
	CbmRichTrbRecoQa();

	/*
	 * Destractor.
	 */
    virtual ~CbmRichTrbRecoQa();

    /**
     * \brief Inherited from FairTask.
     */
    virtual InitStatus Init();

    /**
     * \brief Inherited from FairTask.
     */
    virtual void Exec(
         Option_t* option);

    /**
     * \brief Inherited from FairTask.
     */
    virtual void Finish();

    /*
     * Histogram initialization.
     */
    void InitHist();

    /*
     * Draw histograms.
     */
    void DrawHist();

    /*
     * Draw current event (event display)
     */
    void DrawEvent();

private:
    TClonesArray* fRichHits; // Array of RICH hits

    TH1D* fhNofHitsInEvent; // number of hits in event
    TH2D* fhHitsXY; // XY distribution of the hits in event


    UInt_t fEventNum; // Event counter
    UInt_t fNofDrawnEvents; // Number of drawn events

    CbmRichTrbRecoQa(const CbmRichTrbRecoQa&){;}
    CbmRichTrbRecoQa operator=(const CbmRichTrbRecoQa&){;}

    ClassDef(CbmRichTrbRecoQa, 1);
};

#endif
