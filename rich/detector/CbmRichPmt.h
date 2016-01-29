/*
 * CbmRichPmt.h
 *
 *  Created on: Dec 16, 2015
 *      Author: slebedev
 *
 *
 *  0) det_type == 0
 *     Hamamatsu H8500 with constant QE=0.99
 *  1) det_type == 1
 *     Protvino-type PMT
 *  2) det_type == 2
 *     Hamamatsu H8500 from data sheet
 *  3) det_type == 3
 *     CsI photocathode, NIM A 433 (1999) 201 (HADES)
 *  4) det_type == 4
 *     Hamamatsu H8500-03 from data sheet, with UV window
 *  5) det_type == 5
 *     Hamamatsu H8500 + WLS film with UV window, estimation
 *  6) det_type == 6
 *     Hamamatsu H8500-03 posF in CernOct2011 setup (BUW measurement)
 *  10) det_type == 10
 *      Hamamatsu H8500-03 posH with dipcoated WLS film (BUW measurement)
 *  11) det_type == 11
 *      Hamamatsu H8500-03 posH with dipcoated WLS film -3.8 % due to inhomogeneity (BUW measurement)
 *  12) det_type == 12
 *      Hamamatsu H8500-03 posD in CernOct2011 setup (BUW measurement)
 *  13) det_type == 13
 *      Hamamatsu R11265, average from ZN0590, ZN0593, ZN0731, ZN0733 (BUW measurement)
 *  14) det_type == 14
 *      Hamamatsu R11265, average from ZN0590, ZN0593, ZN0731, ZN0733 with dipcoated WLS film (BUW measurement)
 *  15) det_type == 15
 *      Hamamatsu H8500D-03, posC in CernOct2012 setup, DA0141 ,DA0142, DA0147, DA0154 (BUW measurement)
 *  16) det_type == 16
 *      Hamamatsu H8500D-03, posC in CernOct2012 setup, DA0141, DA0142, DA0147, DA0154 with dipcoated WLS film (BUW measurement)
 *  17) det_type == 17
 *      Hamamatsu H10966A-103, posE in CernOct2012 setup, ZL0003 (BUW measurement)
 *  18) det_type == 18
 *      Hamamatsu H10966A-103, posE in CernOct2012 setup, ZL0003 with dipcoated WLS film (BUW measurement)
 */

#ifndef RICH_DETECTOR_CBMRICHPMT_H_
#define RICH_DETECTOR_CBMRICHPMT_H_

#include "TObject.h"

class CbmRichPmt {
public:
	CbmRichPmt();

	virtual ~CbmRichPmt();

	Bool_t isPhotonDetected(Double_t momentum);

	/**
	* \brief Set detector type
	*/
	void SetDetectorType(Int_t detType){
		fDetectorType = detType;
		InitQE();
	}

	/**
	* \brief Set collection efficiency for photoelectrons in PMT optics.
	*/
	void SetCollectionEfficiency(Double_t collEff){fCollectionEfficiency = collEff;}

private:
	Double_t fCollectionEfficiency; // collection efficiency. Final QE = QE * fCollectionEfficiency
	Int_t fDetectorType; // pmt type
    Double_t fLambdaMin; // minimum wavwlength in QE table
    Double_t fLambdaMax; // maximum wavelength in QE table
    Double_t fLambdaStep; // waelangth in QE table
    Double_t fEfficiency[200]; // Array of QE

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
