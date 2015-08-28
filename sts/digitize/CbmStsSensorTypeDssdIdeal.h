/** @file CbmStsSensorTypeDssdIdeal.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 02.05.2013
 **/

#ifndef CBMSTSSENSORTYPEDSSDIDEAL_H
#define CBMSTSSENSORTYPEDSSDIDEAL_H 1


#include "CbmStsSensorTypeDssd.h"


/** @class CbmStsSensorTypeDssdIdeal
 ** @brief Class describing double-sided silicon strip sensors with ideal response.
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** This class describes the ideal response of double-sided silicon
 ** strip sensors in the STS. All charge is collected on one strip on
 ** both the front and the back side. For more information, see
 ** the mother class CbmStsSensorTypeDssd.
 **
 **/



class CbmStsSensorTypeDssdIdeal : public CbmStsSensorTypeDssd
{

  public:

    /** Constructor  **/
    CbmStsSensorTypeDssdIdeal();


    /** Destructor  **/
    virtual ~CbmStsSensorTypeDssdIdeal() { };

  	/** Process one STS Point
  	 **
  	 ** @param point   Pointer to CbmStsSensorPoint with relevant parameters
  	 ** @param sensor  Pointer to CbmStsSensor object
  	 ** @return  1000* # signals on front side + # signals on back side
  	 **
  	 ** Perform the appropriate action for a particle trajectory in the
  	 ** sensor characterised by the CbmStsSensorPoint object.
  	 **/
  	virtual Int_t ProcessPoint(CbmStsSensorPoint* point,
  		const CbmStsSensor* sensor);



  private:

    /** Produce charge on front or back side from a CbmStsPoint
     ** @param point  Pointer to CbmStsSensorType object
     ** @param side   0 = front, 1 = back side
     ** @param sensor Pointer to sensor object
     ** @return  Number of generated charge signals (active strips)
     **
     ** Re-implements CbmStsSensorTypeDssd::ProduceCharge
     **/
    virtual Int_t ProduceCharge(CbmStsSensorPoint* point, Int_t side,
                                const CbmStsSensor* sensor);




    ClassDef(CbmStsSensorTypeDssdIdeal,1);

};


#endif
