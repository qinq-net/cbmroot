#ifndef CBMRICHTRBRECOQA_H
#define CBMRICHTRBRECOQA_H


#include "FairTask.h"

class TClonesArray;

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

private:
    TClonesArray* fRichHits;

    UInt_t fEventNum;

    CbmRichTrbRecoQa(const CbmRichTrbRecoQa&){;}
    CbmRichTrbRecoQa operator=(const CbmRichTrbRecoQa&){;}

    ClassDef(CbmRichTrbRecoQa, 1);
};

#endif
