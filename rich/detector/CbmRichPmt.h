/*
 * CbmRichPmt.h
 *
 *  Created on: Dec 16, 2015
 *      Author: slebedev
 */

#ifndef RICH_DETECTOR_CBMRICHPMT_H_
#define RICH_DETECTOR_CBMRICHPMT_H_

#include "TObject.h"
#include <vector>
#include "CbmRichPmtType.h"

class CbmRichPmt {
public:
	CbmRichPmt();

	virtual ~CbmRichPmt();

	Bool_t isPhotonDetected(Double_t momentum);

	/**
	* \brief Set detector type
	*/
	void SetDetectorType(CbmRichPmtTypeEnum detType){
		fDetectorType = detType;
		InitQE();
	}

	/**
	* \brief Set collection efficiency for photoelectrons in PMT optics.
	*/
	void SetCollectionEfficiency(Double_t collEff){fCollectionEfficiency = collEff;}

private:
	Double_t fCollectionEfficiency; // collection efficiency. Final QE = QE * fCollectionEfficiency
	CbmRichPmtTypeEnum fDetectorType; // pmt type, See CbmRichPmtType.h for on details about each Pmt detector type
    Double_t fLambdaMin; // minimum wavwlength in QE table
    Double_t fLambdaMax; // maximum wavelength in QE table
    Double_t fLambdaStep; // waelangth in QE table
    std::vector<Double_t> fEfficiency; // Array of QE

    static const Double_t c; // speed of light
    static const Double_t h; // Planck constant
    static const Double_t e; // elementary charge
    Double_t fRefractiveIndex; // refractive index of the gas. Set in the constructor from CbmRichGeoManager

	void InitQE();


	/**
	 * \brief Copy constructor.
	 */
	CbmRichPmt(const CbmRichPmt&);

	/**
	 * \brief Assignment operator.
	 */
	CbmRichPmt& operator=(const CbmRichPmt&);
};

#endif /* RICH_DETECTOR_CBMRICHPMT_H_ */
