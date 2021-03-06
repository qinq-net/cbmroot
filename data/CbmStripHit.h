/**
 * \file CbmStripHit.h
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2009
 *
 * Base class for strip-like hits used for tracking in CBM.
 * Derives from CbmBaseHit.
 * Additional members are u coordinate, phi angle and du, dphi measurement errors.
 **/
#ifndef CBMSTRIPHIT_H_
#define CBMSTRIPHIT_H_

#include "CbmHit.h"

class TVector3;

class CbmStripHit :public CbmHit
{
public:
	/**
	 * \brief Default constructor.
	 */
	CbmStripHit();

	/**
	 * \brief Standard constructor.
	 * \param[in] address detector unique identifier
	 * \param[in] u coordinate in the rotated c.s. [cm]
	 * \param[in] phi strip rotation angle [rad]
	 * \param[in] z Z position of the hit [cm]
	 * \param[in] du U measurement error [cm]
	 * \param[in] dphi PHI measurement error [rad]
	 * \param[in] z Z position of the hit [cm]
	 * \param[in] refId some reference ID
	 * \param[in] time Hit time [ns].
	 * \param[in] timeError Error of hit time [ns].

	 **/
	CbmStripHit(
			Int_t address,
			Double_t u,
			Double_t phi,
			Double_t z,
			Double_t du,
			Double_t dphi,
			Double_t dz,
			Int_t refId,
			Double_t time = -1.,
			Double_t timeError = -1.);

	/**
	 * \brief Standard constructor.
	 * \param[in] address Detector unique identifier.
	 * \param[in] pos Position of the hit as TVector3 (u, phi, z) [cm].
	 * \param[in] err Position errors of the hit as TVector3 (du, dphi, dz) [cm].
	 * \param[in] refId Some reference ID.
	 * \param[in] time Hit time [ns].
	 * \param[in] timeError Error of hit time [ns].
	 **/
	CbmStripHit(
			Int_t address,
			const TVector3& pos,
			const TVector3& err,
			Int_t refId,
			Double_t time = -1.,
			Double_t timeError = -1.);

	/**
	 * \brief Destructor.
	 */
	virtual ~CbmStripHit();

	/**
	 * \brief Inherited from CbmBaseHit.
	 **/
	virtual std::string ToString() const;

	/* Accessors */
	Double_t GetU() const { return fU; }
	Double_t GetPhi() const { return fPhi; }
	Double_t GetDu() const { return fDu; }
	Double_t GetDphi() const { return fDphi; }

	/* Setters */
	void SetU(Double_t u) { fU = u; }
	void SetPhi(Double_t phi) { fPhi = phi; }
	void SetDu(Double_t du) { fDu = du; }
	void SetDphi(Double_t dphi) { fDphi = dphi; }

private:
	Double_t fU; ///< U coordinate in the rotated c.s [cm]
	Double_t fDu; ///< U error [cm]
	Double_t fPhi; ///< strip rotation angle [rad]
	Double_t fDphi; ///< strip rotation error [rad]

	ClassDef(CbmStripHit, 1);
};

#endif /* CBMSTRIPHIT_H_ */
