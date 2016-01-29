/*
 * CbmRichDigi.h
 *
 *  Created on: Dec 17, 2015
 *      Author: slebedev
 */

#include "CbmDetectorList.h"
#include "CbmDigi.h"

#ifndef DATA_RICH_CBMRICHDIGI_H_
#define DATA_RICH_CBMRICHDIGI_H_

class CbmRichDigi : public CbmDigi {
public:
	CbmRichDigi();

	virtual ~CbmRichDigi();

	/*
	 * \brief Inherited from CbmDigi
	 * @value Unique address of pixel channel
	 */
	virtual Int_t GetAddress() const { return fAddress; }

	/*
	 * \brief Inherited from CbmDigi
	 * @value System identifier (enum DetectorId)
	 */
	virtual Int_t GetSystemId() const { return kRICH; }

	/*
	 * \brief Inherited from CbmDigi
	 * @value Time [ns]
	 */
	virtual Double_t GetTime() const { return 0.0; }

	/*
	 * \brief Set pixel Address
	 */
	void SetAddress(Int_t address) { fAddress = address; }

private :
	  Int_t fAddress; // Unique pixel address


	  ClassDef(CbmRichDigi, 1);
};

#endif /* DATA_RICH_CBMRICHDIGI_H_ */
