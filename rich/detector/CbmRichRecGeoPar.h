/**
 * \file CbmRichRecGeoPar.h
 *
 * \brief RICH geometry parameters for the reconstruction. This class is used for convinient storing
 * of the basic RICH geometry parameters like: PMT and mirrors position, dimensions and tilting.
 *
 * \author Lebedev Semen
 * \date 2013
 **/

#ifndef CBM_RICH_REC_GEO_PAR
#define CBM_RICH_REC_GEO_PAR

#include <iostream>
#include "TMath.h"

using namespace std;

/**
 * \class CbmRichMatchRings
 *
 * \brief RICH geometry parameters for the reconstruction. This class is used for convinient storing
 * of the basic RICH geometry parameters like: PMT and mirrors position, dimensions and tilting.
 *
 * \author Lebedev Semen
 * \date 2013
 **/
class CbmRichRecGeoPar
{
    
public:
    
    /**
     * \brief Default constructor.
     */
    CbmRichRecGeoPar()
    :fPmtTheta(0.),
    fPmtPhi(0.),
    fPmtX(0.),
    fPmtY(0.),
    fPmtZ(0.),
    fPmtPlaneX(0.),
    fPmtPlaneY(0.),
    fPmtPlaneZ(0.),
    fNRefrac(0.),
    fMirrorX(0.),
    fMirrorY(0.),
    fMirrorZ(0.),
    fMirrorR(0.),
    fMirrorTheta(0.)
    {;}
    
    /**
     * \brief Destructor.
     */
    ~CbmRichRecGeoPar(){;}
    
    /**
     * \brief Print geometry parameters
     */
    void Print() {
        cout << endl << "-I- RICH geometry parameters" << endl;
        cout << "PMT position in (x,y,z) [cm]: " << fPmtX << "  " << fPmtY << "  " << fPmtZ << endl;
        cout << "PMT plane position in (x,y,z) [cm]: " << fPmtPlaneX << "  " << fPmtPlaneY << "  " << fPmtPlaneZ << endl;
        cout << "PMT was rotated around x by " << fPmtTheta*180./TMath::Pi() << " degrees" << endl;
        cout << "PMT was rotated around y by " << fPmtPhi*180./TMath::Pi() << " degrees" << endl;
        cout << "Refractive index for lowest photon energies: "<< fNRefrac << ", (n-1)*10000: " << (fNRefrac-1.0)*10000.0 << endl;
        cout << "Mirror center (x,y,z): " << fMirrorX << " " << fMirrorY << " " << fMirrorZ << endl;
        cout << "Mirror radius: " << fMirrorR << endl;
        cout << "Mirror rotation angle: " << fMirrorTheta*180./TMath::Pi() << " degrees" << endl << endl;
    }
    
    
public:
    Double_t fPmtTheta; // angle by which photodetector was tilted around X-axis
    Double_t fPmtPhi; // angle by which photodetector was tilted around Y-axis
    
    //PMT position is used for rotation method
    Double_t fPmtX; // X-coordinate of photodetector
    Double_t fPmtY; // Y-coordinate of photodetector
    Double_t fPmtZ; // Z-coordinate of photodetector
    
    // PMt plane positions is used in projection producer (analytical)
    Double_t fPmtPlaneX; // X-coordinate of photodetector plane
    Double_t fPmtPlaneY; // Y-coordinate of photodetector plane
    Double_t fPmtPlaneZ; // Z-coordinate of photodetector plane
    
    Double_t fNRefrac; // refraction index
    
    Double_t fMirrorX; // X-coordinate of mirror center
    Double_t fMirrorY; // Y-coordinate of mirror center
    Double_t fMirrorZ; // Z-coordinate of mirror center
    Double_t fMirrorR; // mirror radius
    
    Double_t fMirrorTheta; // mirror rotation angle around X-axis
};

#endif
