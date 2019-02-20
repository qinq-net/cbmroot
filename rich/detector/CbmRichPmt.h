#/*
 * CbmRichPmt.h
 *
 *  Created on: Dec 16, 2015
 *      Author: slebedev
 */

#ifndef RICH_DETECTOR_CBMRICHPMT_H_
#define RICH_DETECTOR_CBMRICHPMT_H_

#include "TObject.h"
#include "TGraph.h"
#include <vector>
#include <map>
#include "CbmRichPmtType.h"
using namespace std;

class CbmRichPmtQEData {
public:
    CbmRichPmtQEData() = default;
/*
     : fDetectorType()
     , fLambdaMin(0.)
     , fLambdaMax(0.)
     , fLambdaStep(0.)
     , fEfficiency()
*/

    CbmRichPmtTypeEnum fDetectorType; // pmt type
    Double_t fLambdaMin; // minimum wavelength in QE table
    Double_t fLambdaMax; // maximum wavelength in QE table
    Double_t fLambdaStep; // wavelaength in QE table
    vector<Double_t> fEfficiency; // Array of QE

    CbmRichPmtQEData() : fDetectorType(), fLambdaMin(0.), fLambdaMax(0.), fLambdaStep(0.), fEfficiency() {}
};

class CbmRichPmt : public TObject {
public:
	CbmRichPmt();

	virtual ~CbmRichPmt();

	Bool_t isPhotonDetected(CbmRichPmtTypeEnum detType, Double_t momentum);

	/**
	* \brief Set collection efficiency for photoelectrons in PMT optics.
	*/
	void SetCollectionEfficiency(Double_t collEff){fCollectionEfficiency = collEff;}

	/*
	 * Get QE curve as TGraph for specified detector type
	 */
	TGraph* getQEGraph(CbmRichPmtTypeEnum detType);

private:
	Double_t fCollectionEfficiency; // collection efficiency. Final QE = QE * fCollectionEfficiency
	map<CbmRichPmtTypeEnum, CbmRichPmtQEData*> fPmtDataMap; // store QE for specific PMT type

    static const Double_t c; // speed of light
    static const Double_t h; // Planck constant
    static const Double_t e; // elementary charge
    Double_t fRefractiveIndex; // refractive index of the gas. Set in the constructor from CbmRichGeoManager

	void InitQE();

	void ClearMap();


	/**
	 * \brief Copy constructor.
	 */
	CbmRichPmt(const CbmRichPmt&);

	/**
	 * \brief Assignment operator.
	 */
	CbmRichPmt& operator=(const CbmRichPmt&);

	ClassDef(CbmRichPmt,1)
};

#endif /* RICH_DETECTOR_CBMRICHPMT_H_ */
