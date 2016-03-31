/*
 * CbmRichGeoManager.cxx
 *
 *  Created on: Dec 16, 2015
 *      Author: slebedev
 */

#include "CbmRichGeoManager.h"
#include <iostream>
#include "FairLogger.h"

#include "TGeoNode.h"
#include "TGeoManager.h"
#include "TGeoSphere.h"
#include "FairLogger.h"

using namespace std;

CbmRichGeoManager::CbmRichGeoManager()
: fGP(NULL)
{
    InitGeometry();
}

void CbmRichGeoManager::InitGeometry() {
    
    LOG(INFO) << "CbmRichGeoManager::InitGeometry" << endl;
    
    fGP = new CbmRichRecGeoPar();
    //TODO: get refractive index from material
    fGP->fNRefrac = 1.000446242;
    
    CbmRichGeometryType geoType = CbmRichGeometryTypeTwoWings;
    
    fGP->fGeometryType = geoType;
    
    if (geoType == CbmRichGeometryTypeTwoWings) {
        InitPmt();
    } else if (geoType == CbmRichGeometryTypeCylindrical) {
        InitPmtCyl();
    }
    
    
    InitMirror();
    
    fGP->Print();
}

void CbmRichGeoManager::InitPmtCyl()
{
    TGeoIterator geoIterator(gGeoManager->GetTopNode()->GetVolume());
    geoIterator.SetTopName("/cave_1");
    TGeoNode* curNode;
    
    geoIterator.Reset();
    while ((curNode=geoIterator())) {
        TString nodeName(curNode->GetName());
        TString nodePath;
        if (curNode->GetVolume()->GetName() == TString("pmt_block_strip")) {
            
            geoIterator.GetPath(nodePath);
            const TGeoMatrix* curMatrix = geoIterator.GetCurrentMatrix();
            const Double_t* curNodeTr = curMatrix->GetTranslation();
            const Double_t* curNodeRot = curMatrix->GetRotationMatrix();
            
            
            double pmtX = curNodeTr[0];
            double pmtY = curNodeTr[1];
            double pmtZ = curNodeTr[2];
            
            double rotY = TMath::ASin(-curNodeRot[2]);          // around Y
            double rotZ = TMath::ASin(curNodeRot[1]/TMath::Cos(TMath::ASin(-curNodeRot[2]))); // around Z
            //double rotX = TMath::ASin(curNodeRot[5]/TMath::Cos(TMath::ASin(-curNodeRot[2]))); // around X
            double rotX = TMath::ACos(curNodeRot[8]/TMath::Cos(TMath::ASin(-curNodeRot[2]))); // around X
            
            
            fGP->fPmtMap[string(nodePath.Data())].fTheta = rotX;
            fGP->fPmtMap[string(nodePath.Data())].fPhi = rotY;
            const TGeoBBox* shape = (const TGeoBBox*)(curNode->GetVolume()->GetShape());
            fGP->fPmtMap[string(nodePath.Data())].fWidth = shape->GetDX();
            fGP->fPmtMap[string(nodePath.Data())].fHeight = shape->GetDY();
            fGP->fPmtMap[string(nodePath.Data())].fZ = pmtZ;
            fGP->fPmtMap[string(nodePath.Data())].fX = pmtX;
            fGP->fPmtMap[string(nodePath.Data())].fY = pmtY;
        }
    }
    
}

void CbmRichGeoManager::InitPmt()
{
    TGeoIterator geoIterator(gGeoManager->GetTopNode()->GetVolume());
    TGeoNode* curNode;
    
    // PMT plane position\rotation
    TString filterName_pixel("pmt_pixel");
    geoIterator.Reset();
    double minPmtX = 9999999., minPmtY = 9999999., minPmtZ = 9999999.;
    double maxPmtX, maxPmtY, maxPmtZ = 0.;
    while ((curNode=geoIterator())) {
        TString nodeName(curNode->GetName());
        TString nodePath;
        if (curNode->GetVolume()->GetName() == filterName_pixel) {
            
            geoIterator.GetPath(nodePath);
            const TGeoMatrix* curMatrix = geoIterator.GetCurrentMatrix();
            const Double_t* curNodeTr = curMatrix->GetTranslation();
            const Double_t* curNodeRot = curMatrix->GetRotationMatrix();
            
            double pmtX = curNodeTr[0];
            double pmtY = curNodeTr[1];
            double pmtZ = curNodeTr[2];
            
            if (pmtX > 0. && pmtY > 0) {
                //printf ("%08f\t%08f\t%08f\t\n", curNodeTranslation[0], curNodeTranslation[1], curNodeTranslation[2]);
                double rotY = TMath::ASin(-curNodeRot[2]);          // around Y
                double rotZ = TMath::ASin(curNodeRot[1]/TMath::Cos(TMath::ASin(-curNodeRot[2]))); // around Z
                //double rotX = TMath::ASin(curNodeRot[5]/TMath::Cos(TMath::ASin(-curNodeRot[2]))); // around X
                double rotX = TMath::ACos(curNodeRot[8]/TMath::Cos(TMath::ASin(-curNodeRot[2]))); // around X
                
                fGP->fPmt.fTheta = rotX;
                fGP->fPmt.fPhi = rotY;
                
                minPmtX = TMath::Min(minPmtX, pmtX);
                maxPmtX = TMath::Max(maxPmtX, pmtX);
                minPmtY = TMath::Min(minPmtY, pmtY);
                maxPmtY = TMath::Max(maxPmtY, pmtY);
                minPmtZ = TMath::Min(minPmtZ, pmtZ);
                maxPmtZ = TMath::Max(maxPmtZ, pmtZ);
            }
        }
    }
    
    // cout << "minPmtX = " << minPmtX << " maxPmtX = " << maxPmtX << endl;
    // cout << "minPmtY = " << minPmtY << " maxPmtY = " << maxPmtY << endl;
    fGP->fPmt.fPlaneX = (minPmtX + maxPmtX) / 2.;
    fGP->fPmt.fPlaneY = (minPmtY + maxPmtY) / 2.;
    fGP->fPmt.fPlaneZ = (minPmtZ + maxPmtZ) / 2.;
    
    geoIterator.Reset();
    while ((curNode=geoIterator())) {
        TString nodeName(curNode->GetName());
        TString nodePath;
        if (curNode->GetVolume()->GetName() == TString("camera_quarter")) {
            
            geoIterator.GetPath(nodePath);
            const TGeoMatrix* curMatrix = geoIterator.GetCurrentMatrix();
            const Double_t* curNodeTr = curMatrix->GetTranslation();
            //const Double_t* curNodeRot = curMatrix->GetRotationMatrix();
            
            
            double pmtX = curNodeTr[0];
            double pmtY = curNodeTr[1];
            double pmtZ = curNodeTr[2];
            
            if (pmtX > 0. && pmtY > 0) {
                const TGeoBBox* shape = (const TGeoBBox*)(curNode->GetVolume()->GetShape());
                fGP->fPmt.fWidth = shape->GetDX();
                fGP->fPmt.fHeight = shape->GetDY();
                fGP->fPmt.fZ = pmtZ;
                fGP->fPmt.fX = pmtX;
                fGP->fPmt.fY = pmtY;
            }
        }
    }
}

void CbmRichGeoManager::InitMirror()
{
    
    TGeoIterator geoIterator(gGeoManager->GetTopNode()->GetVolume());
    TGeoNode* curNode;
    geoIterator.Reset();
    
    //mirror position\rotation
    TString mirrorName0("mirror_tile_type0");
    TString mirrorName1("mirror_tile_type1");
    TString mirrorName2("mirror_tile_type2");
    TString mirrorName3("mirror_tile_type3");
    //TString mirrorName4("mirror_tile_type4");
    //TString mirrorName5("mirror_tile_type5");
    
    // these names are needed for misaligned geometry
    TString mirrorMisAlignName0("mirror_tile_0");
    TString mirrorMisAlignName1("mirror_tile_1");
    TString mirrorMisAlignName2("mirror_tile_2");
    TString mirrorMisAlignName3("mirror_tile_3");
    
    geoIterator.Reset();
    double maxTheta = 0.;
    double minTheta = 999999999.;
    double mirrorX = 0.;
    double mirrorY = 0.;
    double mirrorZ = 0.;
    double mirrorRadius = 0.;
    while ((curNode=geoIterator())) {
        TString nodeName(curNode->GetName());
        TString nodePath;
        
        
        if (nodeName.Contains(mirrorName0) || nodeName.Contains(mirrorName1) ||  nodeName.Contains(mirrorName2) ||  nodeName.Contains(mirrorName3) ||
            nodeName.Contains(mirrorMisAlignName0) || nodeName.Contains(mirrorMisAlignName1) ||  nodeName.Contains(mirrorMisAlignName2) ||  nodeName.Contains(mirrorMisAlignName3)) {
            geoIterator.GetPath(nodePath);
            const TGeoMatrix* curMatrix = geoIterator.GetCurrentMatrix();
            const Double_t* curNodeTr = curMatrix->GetTranslation();
            mirrorX = TMath::Abs(curNodeTr[0]);
            mirrorY = TMath::Abs(curNodeTr[1]);
            mirrorZ = TMath::Abs(curNodeTr[2]);
            
            const TGeoSphere* shape = dynamic_cast<const TGeoSphere*> (curNode->GetVolume()->GetShape());
            //const TGeoSphere* shape = (const TGeoSphere*)(curNode->GetVolume()->GetShape());
            if (shape != NULL) {
                mirrorRadius = shape->GetRmin();
                
                double theta1 = shape->GetTheta1() ;
                double theta2 = shape->GetTheta2() ;
                if (maxTheta < theta1 || maxTheta < theta2) {
                    maxTheta = TMath::Max(theta1, theta2);
                }
                if (minTheta > theta1 || minTheta > theta2) {
                    minTheta = TMath::Min(theta1, theta2);
                }
            }
        }
    }
    
    fGP->fMirrorTheta = -((maxTheta + minTheta)/2. - 90.) * TMath::DegToRad(); // rotation angle around x-axis
    fGP->fMirrorX = mirrorX;
    fGP->fMirrorY = mirrorY;
    fGP->fMirrorZ = mirrorZ;
    fGP->fMirrorR = mirrorRadius;
    
}


void CbmRichGeoManager::RotatePoint(
                                    TVector3 *inPos,
                                    TVector3 *outPos,
                                    Bool_t noTilting)
{
    if (fGP == nullptr) {
        LOG(ERROR) << "CbmRichGeoManager::RotatePoint RICH geometry is not initialized. fGP == NULL" << FairLogger::endl;
    }
    
    if (fGP->fGeometryType == CbmRichGeometryTypeTwoWings) {
        RotatePointTwoWings(inPos, outPos, noTilting);
    } else if (fGP->fGeometryType == CbmRichGeometryTypeCylindrical) {
        RotatePointCyl(inPos, outPos, noTilting);
    }
    
}

void CbmRichGeoManager::RotatePointTwoWings(
                                            TVector3 *inPos,
                                            TVector3 *outPos,
                                            Bool_t noTilting)
{
    if (noTilting == false){
        RotatePointImpl(inPos, outPos, fGP->fPmt.fPhi, fGP->fPmt.fTheta, fGP->fPmt.fX, fGP->fPmt.fY, fGP->fPmt.fZ);
    } else {
        outPos->SetXYZ(inPos->X(), inPos->Y(), inPos->Z());
    }
}


void CbmRichGeoManager::RotatePointCyl(
                                       TVector3 *inPos,
                                       TVector3 *outPos,
                                       Bool_t noTilting)
{
    if (noTilting == false){
        TGeoNode* node = gGeoManager->FindNode(inPos->X(), inPos->Y(), inPos->Z());
        string path(gGeoManager->GetPath());
        
        CbmRichRecGeoParPmt pmtPar = fGP->GetGeoRecPmtByBlockPath(path);

        RotatePointImpl(inPos, outPos, -TMath::Abs(pmtPar.fPhi), TMath::Abs(pmtPar.fTheta), TMath::Abs(pmtPar.fX), TMath::Abs(pmtPar.fY), TMath::Abs(pmtPar.fZ));
        
    }  else {
        outPos->SetXYZ(inPos->X(), inPos->Y(), inPos->Z());
    }
    
}


void CbmRichGeoManager::RotatePointImpl(
                                        TVector3 *inPos,
                                        TVector3 *outPos,
                                        Double_t phi,
                                        Double_t theta,
                                        Double_t pmtX,
                                        Double_t pmtY,
                                        Double_t pmtZ)
{
    Double_t xDet = 0.,yDet = 0.,zDet = 0.;
    Double_t x = inPos->X();
    Double_t y = inPos->Y();
    Double_t z = inPos->Z();
    
    Double_t sinTheta = TMath::Sin(theta);
    Double_t cosTheta = TMath::Cos(theta);
    Double_t sinPhi = TMath::Sin(phi);
    Double_t cosPhi = TMath::Cos(phi);
    
    if (x > 0 && y > 0) {
        y -= pmtY;
        x -= pmtX;
        z -= pmtZ;
        //xDet = x*cosPhi + z*sinPhi;// - detZOrig*sinPhi;
        //yDet = -x*sinTheta*sinPhi + y*cosTheta + z*sinTheta*cosPhi;
        //zDet = -x*cosTheta*sinPhi - y*sinTheta + z*cosTheta*cosPhi;
        
        xDet = x * cosPhi - y * sinPhi * sinTheta + z * cosTheta * sinPhi;
        yDet = y * cosTheta + z * sinTheta;
        zDet = - x * sinPhi - y * sinTheta * cosPhi + z * cosTheta * cosPhi;
        
        yDet += pmtY;
        xDet += pmtX;
        zDet += pmtZ;
        
    } else if (x > 0 && y < 0) {
        y += pmtY;
        x -= pmtX;
        z -= pmtZ;
        // xDet = x*cosPhi + z*sinPhi;// - detZOrig*sinPhi;
        //yDet = x*sinTheta*sinPhi + y*cosTheta - z*sinTheta*cosPhi;
        //zDet = -x*cosTheta*sinPhi + y*sinTheta + z*cosTheta*cosPhi;
        
        xDet = x * cosPhi + y * sinPhi * sinTheta + z * cosTheta * sinPhi;
        yDet = y * cosTheta - z * sinTheta;
        zDet = - x * sinPhi + y * sinTheta * cosPhi + z * cosTheta * cosPhi;
        
        yDet -= pmtY;
        xDet += pmtX;
        zDet += pmtZ;
    } else if (x < 0 && y < 0) {
        y += pmtY;
        x += pmtX;
        z -= pmtZ;
        // xDet = x*cosPhi - z*sinPhi;// + detZOrig*sinPhi;
        //yDet = -x*sinTheta*sinPhi + y*cosTheta - z*sinTheta*cosPhi;
        //zDet = x*cosTheta*sinPhi + y*sinTheta + z*cosTheta*cosPhi;
        
        xDet = x * cosPhi - y * sinPhi * sinTheta - z * cosTheta * sinPhi;
        yDet = y * cosTheta - z * sinTheta;
        zDet = x * sinPhi + y * sinTheta * cosPhi + z * cosTheta * cosPhi;
        
        yDet -= pmtY;
        xDet -= pmtX;
        zDet += pmtZ;
    } else if (x < 0 && y > 0) {
        y -= pmtY;
        x += pmtX;
        z -= pmtZ;
        //xDet = x*cosPhi - z*sinPhi;// + detZOrig*sinPhi;
        //yDet = x*sinTheta*sinPhi + y*cosTheta + z*sinTheta*cosPhi;
        //zDet = x*cosTheta*sinPhi - y*sinTheta + z*cosTheta*cosPhi;
        
        xDet = x * cosPhi + y * sinPhi * sinTheta - z * cosTheta * sinPhi;
        yDet = y * cosTheta + z * sinTheta;
        zDet = x * sinPhi - y * sinTheta * cosPhi + z * cosTheta * cosPhi;
        
        
        yDet += pmtY;
        xDet -= pmtX;
        zDet += pmtZ;
    } else {
        outPos->SetXYZ(x,y,z);
    }
    outPos->SetXYZ(xDet,yDet,zDet);
}
