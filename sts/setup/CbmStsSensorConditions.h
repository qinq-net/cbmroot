/** @file CbmStsSensorConditions.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 28.11.2014
 **/

#ifndef CBMSTSSENSORCONDITIONS_H
#define CBMSTSSENSORCONDITIONS_H 1

#include "TMath.h"
#include "TObject.h"


/** @class CbmStsSensorConditions
 ** @brief Container for operating condition parameters of a sensor
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 28.11.2014
 ** @version 1.0
 **/
class CbmStsSensorConditions : public TObject {

	public:

		/** Default constructor
		 ** @param vFD           Full depletion voltage [V]
		 ** @param vBias         Bias voltage [V]
		 ** @param temperature   Temperature [degrees C]
		 ** @param cCoupling     Coupling capacitance [F]
		 ** @param cInterstrip   Inter-strip capacitance [F]
		 ** @param bX            Magn. field Bx at sensor centre [T]
		 ** @param bY            Magn. field By at sensor centre [T]
		 ** @param bZ            Magn. field Bz at sensor centre [T]
		 **/
		CbmStsSensorConditions(Double_t vFD = 0., Double_t vBias = 0.,
		                       Double_t temperature= 0,
		                       Double_t cCoupling = 0., Double_t cInterstrip = 0.,
		                       Double_t bX = 0., Double_t bY = 0.,
		                       Double_t bZ = 0.);


		/** Destructor **/
		virtual ~CbmStsSensorConditions();


		/** Magnetic field at sensor centre **/
		Double_t GetBx() const { return fBx; }
		Double_t GetBy() const { return fBy; }
		Double_t GetBz() const { return fBz; }
		Double_t GetB()  const {return TMath::Sqrt( fBx * fBx +
				                                         fBy * fBy +
				                                         fBz * fBz ); }


		/** Coupling capacitance
		 ** @return Coupling capacitance [F]
		 **/
		Double_t GetCcoupling() const { return fCcoupling; }


		/** Inter-strip capacitance
		 ** @return Inter-strip capacitance [F]
		 **/
		Double_t GetCinterstrip() const { return fCinterstrip; }


		/** Temperature
		 ** @return Temperature [degrees C]
	   **/
		Double_t GetTemperatur() const { return fTemperature; }


		/** Bias voltage
		 ** @return Bias voltage [V]
		 **/
		Double_t GetVbias() const { return fVbias; }


		/** Full depletion voltage
		 ** @return Full depletion voltage [V]
		 **/
		Double_t GetVfd() const { return fVfd; }




	private:

		Double_t fVfd;           ///< Full depletion voltage [V]
		Double_t fVbias;         ///< Bias voltage [V]
		Double_t fTemperature;   ///< Temperature [degrees C]
		Double_t fCcoupling;     ///< Coupling capacitance [F]
		Double_t fCinterstrip;   ///< Inter-strip capacitance [F]
		Double_t fBx;            ///< Mag. field (x comp.) at sensor centre [T]
		Double_t fBy;            ///< Mag. field (y comp.) at sensor centre [T]
		Double_t fBz;            ///< Mag. field (z comp.) at sensor centre [T]

		ClassDef(CbmStsSensorConditions, 1);
};

#endif /* CBMSTSSENSSORCONDITIONS_H */
