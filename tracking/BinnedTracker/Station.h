/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Station.h
 * Author: tablyaz
 *
 * Created on August 7, 2017, 2:05 PM
 */

#ifndef STATION_H
#define STATION_H

#include "Bins.h"
#include <functional>
#include "HitReader.h"
#include <set>
#include <iostream>
#include "CbmTrackParam2.h"

const Double_t cbmBinnedSigma = 4;
const Double_t cbmBinnedSigmaSq = cbmBinnedSigma * cbmBinnedSigma;
extern Double_t cbmBinnedSOL;
const Double_t cbmBinnedCrazyChiSq = 1000000;

class CbmBinnedStation
{
public:
    struct Segment
    {
        CbmTBin::HitHolder* begin;
        CbmTBin::HitHolder* end;
        Double_t chiSq;
        std::list<Segment*> children;
        Segment* bestBranch;
        
        Segment(CbmTBin::HitHolder* beginHit, CbmTBin::HitHolder* endHit) : begin(beginHit), end(endHit), chiSq(cbmBinnedCrazyChiSq), children(), bestBranch(0) {}
        Segment(const Segment&) = default;
        Segment& operator=(const Segment&) = default;
    };
    
    struct SegmentComp
    {
        bool operator()(const Segment& s1, const Segment& s2)
        {
            if (&s1.begin < &s2.begin)
                return true;
            else if (&s1.end < &s2.end)
                return true;
            else
                return false;
        }
    };
    
    /*struct KFParamsCoord
    {
        Double_t coord, tg, C11, C12, C21, C22;
    };

    struct KFParams
    {
        KFParamsCoord xParams;
        KFParamsCoord yParams;
        Double_t chi2;
    };*/
    
    struct KFStation
    {
        struct Q
        {
            Double_t Q11, Q12, Q21, Q22;
        };
        
        Q qs[2];
    };
    
public:
    /*static KFParams Extrapolate(const KFParams& params, Double_t deltaZ)
    {
        KFParams result = params;
        
        result.xParams.coord += params.xParams.tg * deltaZ;
        result.xParams.C12 += params.xParams.C22 * deltaZ;
        result.xParams.C11 += params.xParams.C12 * deltaZ + result.xParams.C12 * deltaZ;
        
        result.yParams.coord += params.yParams.tg * deltaZ;
        result.yParams.C12 += params.yParams.C22 * deltaZ;
        result.yParams.C11 += params.yParams.C12 * deltaZ + result.yParams.C12 * deltaZ;
        
        return result;
    }*/
    
    /*static CbmTrackParam2 Extrapolate(const CbmTrackParam2& parIn, Double_t zOut)
    {
        CbmTrackParam2 parOut = parIn;
        
        Double_t X[6] = { parIn.GetX(), parIn.GetY(), parIn.GetTx(), parIn.GetTy(), parIn.GetQp(), parIn.GetTime() };
        Double_t dz = zOut - parIn.GetZ();

        //transport state vector F*X*F.T()
        X[0] = X[0] + dz * X[2];
        X[1] = X[1] + dz * X[3];

        parOut.SetX(X[0]);
        parOut.SetY(X[1]);

        Double_t C[21];
        parIn.CovMatrix(C);
        Double_t txSq = std::pow(X[2], 2);
        Double_t tySq = std::pow(X[3], 2);
        Double_t timeCoeff = dz / std::sqrt(1 + txSq + tySq) / cbmBinnedSOL;
        Double_t dttx = X[2] * timeCoeff;
        Double_t dttxz = dttx * dz;
        Double_t dtty = X[3] * timeCoeff;
        Double_t dttyz = dtty * dz;
        //transport covariance matrix F*C*F.T()
        Double_t t3 = C[2] + dz * C[11];
        Double_t t7 = dz * C[12];
        Double_t t8 = C[3] + t7;
        Double_t t19 = C[8] + dz * C[15];
        C[0] = C[0] + dz * C[2] + t3 * dz;
        C[1] = C[1] + dz * C[7] + t8 * dz;
        C[2] = t3;
        C[3] = t8;
        //C[4] = C[4] + dz * C[13];
        C[5] = C[5] + dz * C[14] + dttx * dttxz * C[11] + dttyz * C[12] + dttx * C[2] + dtty * C[3];
        C[6] = C[6] + dz * C[8] + t19 * dz;
        C[7] = C[7] + t7;
        C[8] = t19;
        //C[9] = C[9] + dz * C[16];
        C[10] = C[10] + dz * C[17] + dttxz * C[12] + dttyz * C[15] + dttx * C[7] + dtty * C[8];
        C[14] = C[14] + dttx * C[11] + dtty * C[12];
        C[17] = C[17] + dttx * C[12] + dtty * C[15];
        //C[19] = C[19] + dttx * C[13] + dtty * C[16];
        C[20] = C[20] + 2 * dttx * dtty * C[12] + dttx * C[14] + dtty * C[17] + dttx * dttx * C[11] + dtty * dtty * C[15];

        parOut.SetCovMatrix(C);
        parOut.SetZ(zOut);
        parOut.SetTime(parIn.GetTime() + std::sqrt(1 + std::pow(X[2], 2) + std::pow(X[3], 2)) * dz / cbmBinnedSOL);
   
        return parOut;
    }*/
    static CbmTrackParam2 Extrapolate(const CbmTrackParam2& parIn, Double_t zOut)
    {
        CbmTrackParam2 parOut = parIn;
        
        Double_t dz = zOut - parIn.GetZ();
        Double_t timeCoeff = dz / std::sqrt(1 + parIn.GetTx() * parIn.GetTx() + parIn.GetTy() * parIn.GetTy()) / cbmBinnedSOL;
        
        Double_t F_ini[5][5] = {
            { 1, 0, dz, 0, 0 },
            { 0, 1, 0, dz, 0 },
            { 0, 0, 1, 0, 0 },
            { 0, 0, 0, 1, 0 },
            { 0, 0, parIn.GetTx() * timeCoeff, parIn.GetTy() * timeCoeff, 1 }
        };
        
        TMatrixD F(5, 5);
        
        for (int i = 0; i < 5; ++i)
        {
            for (int j = 0; j < 5; ++j)
                F(i, j) = F_ini[i][j];
        }
        
        TMatrixD& X = parOut.GetVector();
        TMatrixD Xtmp = X;
        X.Mult(F, Xtmp);
        
        TMatrixD F_t(5, 5);
        F_t.Transpose(F);
        
        TMatrixD& P = parOut.GetCovMatrix();
        TMatrixD PF_t(5, 5);
        PF_t.Mult(P, F_t);
        P.Mult(F, PF_t);
        
        parOut.SetZ(zOut);
        
        return parOut;
    }
    
    /*static void Update(CbmTrackParam2& par, const CbmPixelHit* hit, Double_t& chiSq)
    {
        Double_t cIn[21];
        par.CovMatrix(cIn);

        static const Double_t ONE = 1., TWO = 2.;

        Double_t dxx = hit->GetDx() * hit->GetDx();
        Double_t dxy = hit->GetDxy();
        Double_t dyy = hit->GetDy() * hit->GetDy();
        Double_t dtt = hit->GetTimeError() * hit->GetTimeError();

        // calculate residuals
        Double_t dx = hit->GetX() - par.GetX();
        Double_t dy = hit->GetY() - par.GetY();
        Double_t dt = hit->GetTime() - par.GetTime();

        // Calculate and inverse residual covariance matrix
        Double_t t = ONE / (
                (cIn[0] + dxx) * ((cIn[6] + dyy) * (cIn[20] + dtt) - cIn[10] * cIn[10]) -
                (cIn[1] + dxy) * ((cIn[1] + dxy) * (cIn[20] + dtt) - cIn[5] * cIn[10]) +
                cIn[5] * ((cIn[1] + dxy) * cIn[10] - (cIn[6] + dyy) * cIn[5])
                );
        Double_t R00 = ((cIn[6] + dyy) * (cIn[20] + dtt) - cIn[10] * cIn[10]) * t;
        Double_t R01 = (cIn[5] * cIn[10] - (cIn[1] + dxy) * (cIn[20] + dtt)) * t;
        Double_t R02 = ((cIn[1] + dxy) * cIn[10] - (cIn[6] + dyy) * cIn[5]) * t;
        Double_t R11 = ((cIn[0] + dxx) * (cIn[20] + dtt) - cIn[5] * cIn[5]) * t;
        Double_t R12 = ((cIn[1] + dxy) * cIn[5] - (cIn[0] + dxx) * cIn[10]) * t;
        Double_t R22 = ((cIn[0] + dxx) * (cIn[6] + dyy) - (cIn[1] + dxy) * (cIn[1] + dxy)) * t;

        // Calculate Kalman gain matrix
        Double_t K00 = cIn[0] * R00 + cIn[1] * R01 + cIn[5] * R02;
        Double_t K01 = cIn[0] * R01 + cIn[1] * R11 + cIn[5] * R12;
        Double_t K02 = cIn[0] * R02 + cIn[1] * R12 + cIn[5] * R22;
        Double_t K10 = cIn[1] * R00 + cIn[6] * R01 + cIn[10] * R02;
        Double_t K11 = cIn[1] * R01 + cIn[6] * R11 + cIn[10] * R12;
        Double_t K12 = cIn[1] * R02 + cIn[6] * R12 + cIn[10] * R22;
        Double_t K20 = cIn[2] * R00 + cIn[7] * R01 + cIn[14] * R02;
        Double_t K21 = cIn[2] * R01 + cIn[7] * R11 + cIn[14] * R12;
        Double_t K22 = cIn[2] * R02 + cIn[7] * R12 + cIn[14] * R22;
        Double_t K30 = cIn[3] * R00 + cIn[8] * R01 + cIn[17] * R02;
        Double_t K31 = cIn[3] * R01 + cIn[8] * R11 + cIn[17] * R12;
        Double_t K32 = cIn[3] * R02 + cIn[8] * R12 + cIn[17] * R22;
        //Double_t K40 = cIn[4] * R00 + cIn[9] * R01 + cIn[19] * R02;
        //Double_t K41 = cIn[4] * R01 + cIn[9] * R11 + cIn[19] * R12;
        //Double_t K42 = cIn[4] * R02 + cIn[9] * R12 + cIn[19] * R22;
        Double_t K50 = cIn[5] * R00 + cIn[10] * R01 + cIn[20] * R02;
        Double_t K51 = cIn[5] * R01 + cIn[10] * R11 + cIn[20] * R12;
        Double_t K52 = cIn[5] * R02 + cIn[10] * R12 + cIn[20] * R22;

        // Calculate filtered state vector
        Double_t xOut[6] = { par.GetX(), par.GetY(), par.GetTx(), par.GetTy(), par.GetQp(), par.GetTime() };
        xOut[0] += K00 * dx + K01 * dy + K02 * dt;
        xOut[1] += K10 * dx + K11 * dy + K12 * dt;
        xOut[2] += K20 * dx + K21 * dy + K22 * dt;
        xOut[3] += K30 * dx + K31 * dy + K32 * dt;
        //xOut[4] += K40 * dx + K41 * dy + K42 * dt;
        xOut[5] += K50 * dx + K51 * dy + K52 * dt;

        // Calculate filtered covariance matrix
        Double_t cOut[21];
        std::copy(std::begin(cIn), std::end(cIn), std::begin(cOut));

        cOut[0] -= K00 * cIn[0] + K01 * cIn[1] + K02 * cIn[5];
        cOut[1] -= K00 * cIn[1] + K01 * cIn[6] + K02 * cIn[10];
        cOut[2] -= K00 * cIn[2] + K01 * cIn[7] + K02 * cIn[14];
        cOut[3] -= K00 * cIn[3] + K01 * cIn[8] + K02 * cIn[17];
        //cOut[4] -= K00 * cIn[4] + K01 * cIn[9] + K02 * cIn[19];
        cOut[5] -= K00 * cIn[5] + K01 * cIn[10] + K02 * cIn[20];

        cOut[6] -= K10 * cIn[1] + K11 * cIn[6] + K12 * cIn[10];
        cOut[7] -= K10 * cIn[2] + K11 * cIn[7] + K12 * cIn[14];
        cOut[8] -= K10 * cIn[3] + K11 * cIn[8] + K12 * cIn[17];
        //cOut[9] -= K10 * cIn[4] + K11 * cIn[9] + K12 * cIn[19];
        cOut[10] -= K10 * cIn[5] + K11 * cIn[10] + K12 * cIn[20];

        cOut[11] -= K20 * cIn[2] + K21 * cIn[7] + K22 * cIn[14];
        cOut[12] -= K20 * cIn[3] + K21 * cIn[8] + K22 * cIn[17];
        //cOut[13] -= K20 * cIn[4] + K21 * cIn[9] + K22 * cIn[19];
        cOut[14] -= K20 * cIn[5] + K21 * cIn[10] + K22 * cIn[20];

        cOut[15] -= K30 * cIn[3] + K31 * cIn[8] + K32 * cIn[17];
        //cOut[16] -= K30 * cIn[4] + K31 * cIn[9] + K32 * cIn[19];
        cOut[17] -= K30 * cIn[5] + K31 * cIn[10] + K32 * cIn[20];

        //cOut[18] -= K40 * cIn[4] + K41 * cIn[9] + K42 * cIn[19];
        //cOut[19] -= K40 * cIn[5] + K41 * cIn[10] + K42 * cIn[20];

        cOut[20] -= K50 * cIn[5] + K51 * cIn[10] + K52 * cIn[20];

        // Copy filtered state to output
        par.SetX(xOut[0]);
        par.SetY(xOut[1]);
        par.SetTx(xOut[2]);
        par.SetTy(xOut[3]);
        //par.SetQp(xOut[4]);
        par.SetTime(xOut[5]);
        par.SetCovMatrix(cOut);

        // Calculate chi-square
        Double_t xmx = hit->GetX() - par.GetX();
        Double_t ymy = hit->GetY() - par.GetY();
        Double_t tmt = hit->GetTime() - par.GetTime();
        Double_t C0 = cOut[0];
        Double_t C1 = cOut[1];
        Double_t C5 = cOut[6];
        
        Double_t norm = (dxx - cOut[0]) * ((dyy - cOut[6]) * (dtt - cOut[20]) - cOut[10] * cOut[10]) +
                (dxy - cOut[1]) * (cOut[5] * cOut[10] - (dxy - cOut[1]) * (dtt - cOut[20])) +
                cOut[5] * ((dxy - cOut[1]) * cOut[10] - (dyy - cOut[6]) * cOut[5]);

        if (norm == 0.)
            norm = 1e-10;

        // Mij is the (symmetric) inverse of the residual matrix
        Double_t M00 = ((dyy - cOut[6]) * (dtt - cOut[20]) - cOut[10] * cOut[10]) / norm;
        Double_t M01 = ((dxy - cOut[1]) * (dtt - cOut[20]) - cOut[5] * cOut[10]) / norm;
        Double_t M02 = ((dxy - cOut[1]) * cOut[10] - (dyy - cOut[6]) * cOut[5]) / norm;
        Double_t M11 = ((dxx - cOut[0]) * (dtt - cOut[20]) - cOut[5] * cOut[5]) / norm;
        Double_t M12 = ((dxx - cOut[0]) * cOut[10] - (dxy - cOut[1]) * cOut[5]) / norm;
        Double_t M22 = ((dxx - cOut[0]) * (dyy - cOut[6]) - (dxy - cOut[1]) * (dxy - cOut[1])) / norm;

        chiSq = xmx * (xmx * M00 + ymy * M01 + tmt * M02) + ymy * (xmx * M01 + ymy * M11 + tmt * M12) + tmt * (xmx * M02 + ymy * M12 + tmt * M22);
    }*/
    static void Update(CbmTrackParam2& par, const CbmPixelHit* hit, Double_t& chiSq)
    {
        TMatrixD& X = par.GetVector();
        TMatrixD& P = par.GetCovMatrix();
        TMatrixD H(3, 5);
        H(0, 0) = 1;
        H(1, 1) = 1;
        H(2, 4) = 1;
        
        TMatrixD Z(3, 1);
        Z(0, 0) = hit->GetX();
        Z(1, 0) = hit->GetY();
        Z(2, 0) = hit->GetTime();
        
        TMatrixD Y = Z;
        TMatrixD HX(3, 1);
        HX.Mult(H, X);
        Y.Minus(Z, HX);
        
        TMatrixD H_t(5, 3);
        H_t.Transpose(H);
        
        TMatrixD R(3, 3);
        R(0, 0) = hit->GetDx() * hit->GetDx();
        R(0, 1) = hit->GetDxy();
        R(1, 1) = hit->GetDy() * hit->GetDy();
        R(1, 0) = hit->GetDxy();
        R(2, 2) = hit->GetTimeError() * hit->GetTimeError();
        
        TMatrixD HP(3, 5);
        HP.Mult(H, P);
        
        TMatrixD HPH_t(3, 3);
        HPH_t.Mult(HP, H_t);
        
        TMatrixD S(3, 3);
        S.Plus(R, HPH_t);
        
        TMatrixD S_1 = S;
        S_1.InvertFast();
        
        TMatrixD H_tS_1(5, 3);
        H_tS_1.Mult(H_t, S_1);
        
        TMatrixD K(5, 3);// Optimal Kalman gain
        K.Mult(P, H_tS_1);
        
        TMatrixD KY(5, 1);
        KY.Mult(K, Y);
        
        X += KY;
        
        TMatrixD KHP(5, 5);
        KHP.Mult(K, HP);
        
        P -= KHP;
        
        //
        HX.Mult(H, X);
        Y.Minus(Z, HX);
        
        HP.Mult(H, P);
        HPH_t.Mult(HP, H_t);
        
        TMatrixD V = R;
        V -= HPH_t;
        
        TMatrixD V_1 = V;
        V_1.InvertFast();
        
        TMatrixD Y_t(1, 3);
        Y_t.Transpose(Y);
        
        TMatrixD V_1Y(3, 1);
        V_1Y.Mult(V_1, Y);
        
        TMatrixD Y_tV_1Y(1, 1);
        Y_tV_1Y.Mult(Y_t, V_1Y);
        
        chiSq += Y_tV_1Y(0, 0);
    }
    
public:
    CbmBinnedStation(Double_t minZ, Double_t maxZ, int nofYBins, int nofXBins, int nofTBins);
    CbmBinnedStation(const CbmBinnedStation&) = delete;
    CbmBinnedStation& operator=(const CbmBinnedStation&) = delete;
    
    virtual ~CbmBinnedStation() {}
    Double_t GetMinZ() const { return fMinZ; }
    Double_t GetMaxZ() const { return fMaxZ; }
    void SetMinZ(Double_t v) { fMinZ = v; }
    void SetMaxZ(Double_t v) { fMaxZ = v; }
    void SetMinY(Double_t v) { fMinY = v; }
    void SetMaxY(Double_t v) { fMaxY = v; }
    void SetMinX(Double_t v) { fMinX = v; }
    void SetMaxX(Double_t v) { fMaxX = v; }
    
    void SetTBinSize(Double_t v) { fTBinSize = v; }
    
    void SetMinT(Double_t v)
    {
        fMinT = v;
        fMaxT = fMinT + fNofTBins * fTBinSize;
    }
    
    void SetDefaultUse() { fDefaultUse = true; }
    
    Double_t GetDx() const { return fDx; }
    
    void SetDx(Double_t v)
    {
        if (v > fDx)
        {
            fDx = v;
            fDxSq = v * v;
        }
    }
    
    Double_t GetDy() const { return fDy; }
    
    void SetDy(Double_t v)
    {
        if (v > fDy)
        {
            fDy = v;
            fDySq = v * v;
        }
    }
    
    Double_t GetDt() const { return fDt; }
    
    void SetDt(Double_t v)
    {
        if (v > fDt)
        {
            fDt = v;
            fDtSq = v * v;
        }
    }
    
    Double_t GetScatX() const { return fScatX; }
    Double_t GetScatXSq() const { return fScatXSq; }
    
    void SetScatX(Double_t v)
    {
        fScatX = v;
        fScatXSq = v * v;
    }
    
    Double_t GetScatY() const { return fScatY; }
    Double_t GetScatYSq() const { return fScatYSq; }
    
    void SetScatY(Double_t v)
    {
        fScatY = v;
        fScatYSq = v * v;
    }
    
    Double_t GetNofSigmaX() const { return fNofSigmasX; }
    Double_t GetNofSigmaXSq() const { return fNofSigmasXSq; }
    
    void SetNofSigmaX(Double_t v)
    {
        fNofSigmasX = v;
        fNofSigmasXSq = v * v;
    }
    
    Double_t GetNofSigmaY() const { return fNofSigmasY; }
    Double_t GetNofSigmaYSq() const { return fNofSigmasYSq; }
    
    void SetNofSigmaY(Double_t v)
    {
        fNofSigmasY = v;
        fNofSigmasYSq = v * v;
    }
    
    int GetXInd(Double_t v) const
    {
        int ind = (v - fMinX) / fXBinSize;
        
        if (ind < 0)
            ind = 0;
        else if (ind >= fNofXBins)
            ind = fNofXBins - 1;
        
        return ind;
    }
    
    int GetYInd(Double_t v) const
    {
        int ind = (v - fMinY) / fYBinSize;
        
        if (ind < 0)
            ind = 0;
        else if (ind >= fNofYBins)
            ind = fNofYBins - 1;
        
        return ind;
    }
    
    int GetTInd(Double_t v) const
    {
        int ind = (v - fMinT) / fTBinSize;
        
        if (ind < 0)
            ind = 0;
        else if (ind >= fNofTBins)
            ind = fNofTBins - 1;
        
        return ind;
    }
    
    void SetCheckUsed(bool v) { fCheckUsed = v; }
    
    virtual void Init()
    {
        fYBinSize = (fMaxY - fMinY) / fNofYBins;
        fXBinSize = (fMaxX - fMinX) / fNofXBins;
    }
    
    virtual void Clear()
    {
        fDx = 0;
        fDxSq = 0;
        fDy = 0;
        fDySq = 0;
        fDt = 0;
        fDtSq = 0;
        fCheckUsed = false;
        fSegments.clear();
    }
    
    virtual void AddHit(const CbmPixelHit* hit, Int_t index) = 0;
    virtual void IterateHits(std::function<void(CbmTBin::HitHolder&)> handleHit) = 0;
    virtual void SearchHits(const CbmTrackParam2& stateVec, Double_t stateZ, std::function<void(CbmTBin::HitHolder&)> handleHit) = 0;
    virtual void SearchHits(Segment& segment, std::function<void(CbmTBin::HitHolder&)> handleHit) = 0;
    virtual void SearchHits(Double_t minZ, Double_t maxZ, Double_t minY, Double_t maxY, Double_t minX, Double_t maxX, Double_t minT, Double_t maxT,
        std::function<void(CbmTBin::HitHolder&)> handleHit) = 0;
    
    void IterateSegments(std::function<void(Segment&)> handleSegment)
    {
        for (std::set<Segment, SegmentComp>::iterator i = fSegments.begin(); i != fSegments.end(); ++i)
            handleSegment(const_cast<Segment&> (*i));
    }
    
    void CreateSegmentsFromHits()
    {
        IterateHits(
            [&](CbmTBin::HitHolder& hitHolder)->void
            {
                Segment segment(&fVertexHolder, &hitHolder);
                fSegments.insert(segment);
            }
        );
    }
    
    void NulifySegments()
    {
        for (std::set<Segment, SegmentComp>::iterator i = fSegments.begin(); i != fSegments.end(); ++i)
        {
            Segment& segment = const_cast<Segment&> (*i);
            segment.chiSq = 0;
        }
    }
    
protected:
    Double_t fMinZ;
    Double_t fMaxZ;
    int fNofYBins;
    int fNofXBins;
    int fNofTBins;
    Double_t fYBinSize;
    Double_t fXBinSize;
    Double_t fTBinSize;
    Double_t fMinY;
    Double_t fMaxY;
    Double_t fMinX;
    Double_t fMaxX;
    Double_t fMinT;
    Double_t fMaxT;
    Double_t fDx;
    Double_t fDxSq;
    Double_t fDy;
    Double_t fDySq;
    Double_t fDt;
    Double_t fDtSq;
    bool fDefaultUse;
    Double_t fScatX;
    Double_t fScatXSq;
    Double_t fScatY;
    Double_t fScatYSq;
    
public:
    std::set<Segment, SegmentComp> fSegments;
    CbmTBin fVertexBin;
    CbmTBin::HitHolder fVertexHolder;
    
protected:
    Double_t fNofSigmasX;
    Double_t fNofSigmasXSq;
    Double_t fNofSigmasY;
    Double_t fNofSigmasYSq;
    bool fCheckUsed;
};

#endif /* STATION_H */

