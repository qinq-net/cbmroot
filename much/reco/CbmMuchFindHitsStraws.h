/**
 * \file CbmMuchFindHitsStraws.h

 *@author  A.Zinchenko <Alexander.Zinchenko@jinr.ru>
 *@version 2.0
 *@since   15.04.14
 * Added 2-dim. hits

 * Hit producer for straw detectors
 **/

#ifndef CBMMUCHFINDHITSSTRAWS_H
#define CBMMUCHFINDHITSSTRAWS_H 1

#include "CbmMuchGeoScheme.h"
#include "FairTask.h"
#include "TString.h"

class TClonesArray;

class CbmMuchFindHitsStraws : public FairTask
{
public:
    /** Constructor with task name **/
    CbmMuchFindHitsStraws(const char* digiFileName);

    /** Destructor **/
    virtual ~CbmMuchFindHitsStraws();

    /** Execution **/
    virtual void Exec(Option_t* opt);

    /** Initialization **/
    virtual InitStatus Init();

    Int_t GetEff() const
    {
        return fEffic;
    }
    Int_t GetMerge() const
    {
        return fMerge;
    }
    Int_t GetMirror() const
    {
        return fMirror;
    }
    Int_t GetDimens() const
    {
        return fDimens;
    }
    Double_t GetPhi(Int_t i) const
    {
        return fPhis[i];
    }
    void SetEff(Int_t eff)
    {
        fEffic = eff;
    }
    void SetMerge(Int_t merge)
    {
        fMerge = merge;
    }
    void SetMirror(Int_t mirror)
    {
        fMirror = mirror;
    }
    void SetBinary()
    {
        fBinary = 1;
        fMirror = 0;
    }
    void SetDimens(Int_t dim)
    {
        fDimens = dim;
    }
    void SetPhi(Int_t i, Double_t phi)
    {
        fPhis[i] = phi;
    } // set doublets rotation angles (degs)
    void SetPhis(Double_t ph0, Double_t ph1, Double_t ph2)
    {
        fPhis[0] = ph0;
        fPhis[1] = ph1;
        fPhis[2] = ph2;
    } // set doublets rotation angles (degs)

private:
    void ComputeErrors(Double_t phi,
                       Double_t cosPhi,
                       Double_t sinPhi,
                       Double_t sigX,
                       Double_t sigmaY,
                       Double_t& dx,
                       Double_t& dy,
                       Double_t& dxy); // compute errors for 2-D hits
    void Effic(Double_t* diam);        // apply inefficiency (currently inside straw tube walls)
    void Merge();                      // merge hits inside the same straw tube
    void Mirror();                     // add mirror hits (left/right ambiguity)
    void StorePixels();                // store pixel hits

    CbmMuchGeoScheme* fGeoScheme; // Geometry scheme
    TString fDigiFile;            // Digitization file
    TClonesArray* fPoints;        // Input array of CbmMuchPoint
    TClonesArray* fDigis;         // Input array of CbmMuchDigi
    TClonesArray* fDigiMatches;   // Input/output array of CbmMuchDigiMatch
    TClonesArray* fClusters;      // Output array of CbmMuchCluster objects
    TClonesArray* fHits;          // Output array of CbmMuchHit
    Int_t fEffic;                 // Efficiency correction flag
    Int_t fMerge;                 // merging flag
    Int_t fMirror;                // mirror flag (left-right ambiguity)
    Int_t fBinary;                // binary flag (binary info - tube center as coord.)
    Int_t fDimens;                // 1- or 2-dimensional hits
    Double_t fPhis[3];            // rotation angles of doublets

    CbmMuchFindHitsStraws(const CbmMuchFindHitsStraws&);
    CbmMuchFindHitsStraws& operator=(const CbmMuchFindHitsStraws&);

    ClassDef(CbmMuchFindHitsStraws, 1);
};

#endif
