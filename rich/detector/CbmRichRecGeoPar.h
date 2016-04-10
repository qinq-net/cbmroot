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
#include <map>

using namespace std;

enum CbmRichGeometryType { CbmRichGeometryTypeTwoWings, CbmRichGeometryTypeCylindrical };

/**
 * \class CbmRichRecGeoPar
 *
 * \brief PMT parameters for the RICH geometry.
 *
 * \author Lebedev Semen
 * \date 2016
 **/
class CbmRichRecGeoParPmt
{
public:
    /**
     * \brief Default constructor.
     */
    CbmRichRecGeoParPmt()
    :fTheta(0.),
    fPhi(0.),
    fX(0.),
    fY(0.),
    fZ(0.),
    fPlaneX(0.),
    fPlaneY(0.),
    fPlaneZ(0.),
    fWidth(0.),
    fHeight(0.)
    {;}
    
    
    Double_t fTheta; // angle by which photodetector was tilted around X-axis
    Double_t fPhi; // angle by which photodetector was tilted around Y-axis
    
    //PMT position is used for rotation method
    Double_t fX; // X-coordinate of photodetector
    Double_t fY; // Y-coordinate of photodetector
    Double_t fZ; // Z-coordinate of photodetector
    
    // PMt plane positions is used in projection producer (analytical)
    Double_t fPlaneX; // X-coordinate of photodetector plane
    Double_t fPlaneY; // Y-coordinate of photodetector plane
    Double_t fPlaneZ; // Z-coordinate of photodetector plane
    
    Double_t fWidth; // TGeoBBox->GetDX(), half of the camera quater
    Double_t fHeight; // TGeoBBox->GetDY(), half of the camera quater
};


/**
 * \class CbmRichRecGeoPar
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
    :fPmt(),
    fGeometryType(CbmRichGeometryTypeTwoWings),
    fPmtMap(),
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
        
        if (fGeometryType == CbmRichGeometryTypeTwoWings) {
            PrintTwoWings();
        } else if (fGeometryType == CbmRichGeometryTypeCylindrical) {
            PrintCylindrical();
        } else {
            cout << "ERROR, geometry type is not correct" << endl;
        }
        
        PrintMirror();
    }
    
    /**
     * \brief Print geometry parameters for two wings geometry
     */
    void PrintTwoWings() {
        cout << "PMT position in (x,y,z) [cm]: " << fPmt.fX << "  " << fPmt.fY << "  " << fPmt.fZ << endl;
        cout << "PMT plane position in (x,y,z) [cm]: " << fPmt.fPlaneX << "  " << fPmt.fPlaneY << "  " << fPmt.fPlaneZ << endl;
        cout << "PMT width and height [cm]: " << fPmt.fWidth << "  " << fPmt.fHeight << endl;
        cout << "PMT was rotated around x (theta) by " << fPmt.fTheta*180./TMath::Pi() << " degrees" << endl;
        cout << "PMT was rotated around y (phi) by " << fPmt.fPhi*180./TMath::Pi() << " degrees" << endl;
    }
    
    /**
     * \brief Print geometry parameters for cylindrical geometry
     */
    void PrintCylindrical() {
        
        typedef map<string, CbmRichRecGeoParPmt>::iterator it_type;
        for(it_type iterator = fPmtMap.begin(); iterator != fPmtMap.end(); iterator++) {
            cout << endl << "Geo path:" << iterator->first << endl;
            cout << "PMT position in (x,y,z) [cm]: " << iterator->second.fX << "  " << iterator->second.fY << "  " << iterator->second.fZ << endl;
           // cout << "PMT plane position in (x,y,z) [cm]: " << iterator->second.fPlaneX << "  " << iterator->second.fPlaneY << "  " << iterator->second.fPlaneZ << endl;
            cout << "PMT width and height [cm]: " << iterator->second.fWidth << "  " << iterator->second.fHeight << endl;
            cout << "PMT was rotated around x (theta) by " << iterator->second.fTheta*180./TMath::Pi() << " degrees" << endl;
            cout << "PMT was rotated around y (phi) by " << iterator->second.fPhi*180./TMath::Pi() << " degrees" << endl;
        }
    }
    
    void PrintMirror() {
        cout << "Refractive index for lowest photon energies: "<< fNRefrac << ", (n-1)*10000: " << (fNRefrac-1.0)*10000.0 << endl;
        cout << "Mirror center (x,y,z): " << fMirrorX << " " << fMirrorY << " " << fMirrorZ << endl;
        cout << "Mirror radius: " << fMirrorR << endl;
        cout << "Mirror rotation angle: " << fMirrorTheta*180./TMath::Pi() << " degrees" << endl << endl;
    }
    
    CbmRichRecGeoParPmt GetGeoRecPmtByBlockPath(const string& path) {
        typedef map<string, CbmRichRecGeoParPmt>::iterator it_type;
        for(it_type iterator = fPmtMap.begin(); iterator != fPmtMap.end(); iterator++) {
            if (path.find(iterator->first) != std::string::npos) {
                return iterator->second;
            }
        }
    }
    
    
public:
    
    CbmRichGeometryType fGeometryType;
    
    CbmRichRecGeoParPmt fPmt; // PMT parameters for 2-wings geometry CbmRichGeometryTypeTwoWings
    
    map<string, CbmRichRecGeoParPmt> fPmtMap; // PMT parameter map for CbmRichGeometryTypeCylindrical, string is geo path to PMT block
    
    
    Double_t fNRefrac; // refraction index
    
    Double_t fMirrorX; // X-coordinate of mirror center
    Double_t fMirrorY; // Y-coordinate of mirror center
    Double_t fMirrorZ; // Z-coordinate of mirror center
    Double_t fMirrorR; // mirror radius
    
    Double_t fMirrorTheta; // mirror rotation angle around X-axis
};

#endif
