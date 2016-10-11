/** @file CbmStsSensorTypeDssdOrtho.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 02.10.2015
 **/

#ifndef CBMSTSSENSORTYPEDSSDORTHO_H
#define CBMSTSSENSORTYPEDSSDORTHO_H 1

#include "TArrayD.h"
#include "CbmStsSensorTypeDssd.h"

class CbmStsPhysics;

/** @class CbmStsSensorTypeDssdOrtho
 ** @brief Class describing double-sided silicon strip sensors with orthogonal strips.
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** This class describes the response of double-sided silicon
 ** strip sensors in the STS. Its free parameters are the dimensions
 ** of the active area and the number of strips at the
 ** top (readout) edge. Strips are vertical on the front side
 ** and horizontal on the back side.
 **
 ** The active area does not necessarily coincide with the geometric
 ** dimensions of the sensor. It is, however, centred in the latter,
 ** meaning that the width of inactive regions (guard ring) are
 ** the same on the left and on the right side and also the same at
 ** the top and and the bottom.
 **
 ** The response to charged particles is modelled by a uniform charge
 ** distribution along the particle trajectory in the active volume,
 ** which is projected to the readout edge, where it is discretised
 ** on the active strips. The charge is then delivered to the corresponding
 ** channel of the readout module (CbmStsModule).
 **/
class CbmStsSensorTypeDssdOrtho : public CbmStsSensorTypeDssd
{

  public:

    /** Constructor  **/
    CbmStsSensorTypeDssdOrtho();


    /** Destructor  **/
    virtual ~CbmStsSensorTypeDssdOrtho() { };


    /** Set the parameters
     ** @param dx,dy,dz          Size in x,y,z [cm]
     ** @param pitchF,pitchB     Strip pitch foint and back side [cm]
     ** @param stereoF,stereoB   Strip stereo angle front and back side [degrees]
     **/
    virtual void SetParameters(Double_t dx, Double_t dy, Double_t dz,
                       Int_t nStripsF, Int_t nStripsB,
                       Double_t stereoF, Double_t stereoB);


  protected:

    /** Charge diffusion into adjacent strips
     ** @param[in] x      x coordinate of charge centre (local c.s.) [cm]
     ** @param[in] y      y coordinate of charge centre (local c.s.) [cm]
     ** @param[in] sigma  Diffusion width [cm]
     ** @param[in] side   0 = front (p) side, 1 = back (n) side
     ** @param[out] fracL  Fraction of charge in left neighbour strip
     ** @param[out] fracC  Fraction of charge in centre strip
     ** @param[out] fracR  Fraction of charge in right neighbour strip
     **
     ** Calculates the fraction of charge in the most significant (centre)
     ** strip and its left and right neighbours. The charge distribution is
     ** assumed to be a 2-d Gaussian (resulting from thermal diffusion)
     ** with centre (x,y) and width sigma in both dimensions. The integration
     ** is performed in the coordinate across the strips. For simplicity,
     ** all charge left (right) of the centre strip is accumulated in the left
     ** (right) neighbour; this is justified since typical values of the
     ** diffusion width are much smaller than the strip pitch. The charge in
     ** the neighbouring strip is neglected if it is more distant than 3 sigma
     ** from the charge centre.
     ** Edge effects are neglected, i.e. diffusion into the inactive area is
     ** allowed.
     **/
    virtual void Diffusion(Double_t x, Double_t y, Double_t sigma, Int_t side,
    		           Double_t& fracL, Double_t& fracC, Double_t& fracR);


    /** Get the readout channel in the module for a given strip and side
     ** @param strip     Strip number
     ** @param side      Side (0 = front, 1 = back)
     ** @param sensorId  Index of sensor within module
     ** @return  Channel number in module
     **
     ** Note: This encodes the mapping of sensor strip to module
     ** channel, i.e. defines the physical meaning of the latter.
     **/
    virtual Int_t GetModuleChannel(Int_t strip, Int_t side, Int_t sensorId) const;


    /** Get strip and side from module channel.
     ** @param[in] channel   Channel number in module
     ** @param[in] sensorId  Sensor index in module
     ** @param[out]  strip   Strip number in sensor
     ** @param[out]  side    Sensor side [0 = front, 1 = back]
     **
     ** Note: This must be the inverse of GetModuleChannel.
     **/
    virtual void GetStrip(Int_t channel, Int_t sensorId, Int_t& strip, Int_t& side);


    /** Get strip number from point coordinates
     ** @param x     x coordinate [cm]
     ** @param y     y coordinate [cm]
     ** @param side  0 = front side, 1 = back side
     ** @return strip number on selected side
     **/
    virtual Int_t GetStripNumber(Double_t x, Double_t y, Int_t side) const;


    /** Find the intersection points of two clusters.
     ** For each intersection point, a hit is created.
     ** @param clusterF    Pointer to cluster on front side
     ** @param clusterB    Pointer to cluster on back side
     ** @param sensor      Pointer to sensor object
     ** @return Number of intersection points inside active area
     **/
    virtual Int_t IntersectClusters(CbmStsCluster* clusterF,
    		                    CbmStsCluster* clusterB,
    		                    CbmStsSensor* sensor);


    /** Copy constructor (not implemented)  **/
    CbmStsSensorTypeDssdOrtho(CbmStsSensorTypeDssdOrtho& rhs);


    /** Assignment operator (not implemented)  **/
    CbmStsSensorTypeDssdOrtho& operator = (const CbmStsSensorTypeDssdOrtho& rhs);



    ClassDef(CbmStsSensorTypeDssdOrtho,1);

};


#endif
