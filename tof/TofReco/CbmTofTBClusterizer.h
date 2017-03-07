/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CbmTofTBClusterizer.h
 * Author: tablyaz
 *
 * Created on December 6, 2016, 11:29 AM
 */

#ifndef CBMTOFTBCLUSTERIZER_H
#define CBMTOFTBCLUSTERIZER_H

#include "FairTask.h"
#include "CbmTofDigiExp.h"
#include "CbmTofGeoHandler.h"
#include "CbmTofDetectorId_v12b.h"
#include "CbmTofDetectorId_v14a.h"
#include "CbmTofDigiPar.h"
#include "CbmTofCell.h"
#include "CbmTofDigiBdfPar.h"

class CbmTofTBClusterizer : public FairTask
{
public:
    struct ChannelDigis
    {
        struct DigiDesc
        {
            CbmTofDigiExp* pDigi;
            Int_t digiInd;
        };
        
        struct DigiPair
        {
            Double_t y;
            DigiDesc topDigi;
            DigiDesc bottomDigi;
        };
        
        std::map<Double_t, DigiDesc> topDigis;
        std::map<Double_t, DigiDesc> bottomDigis;
        std::map<Double_t, DigiPair> digiPairs;
    };
    
    CbmTofTBClusterizer();
    InitStatus Init();
    void SetParContainers();
    void Exec(Option_t* option);
    void Finish();
    
private:
    Bool_t InitCalibParameter();
    void GetEventInfo(Int_t& inputNr, Int_t& eventNr, Double_t& eventTime);

private:
    CbmTofGeoHandler* fGeoHandler;
    CbmTofDetectorId* fTofId;
    CbmTofDigiPar* fDigiPar;
    CbmTofCell* fChannelInfo;
    CbmTofDigiBdfPar* fDigiBdfPar;
    std::vector<std::vector<Double_t> > fvCPSigPropSpeed; //[nSMT][nRpc]
    std::vector< std::vector<std::vector<std::vector<Double_t> > > > fvCPDelTof; //[nSMT][nRpc][nbClDelTofBinX][nbTrg]
    std::vector< std::vector<std::vector<std::vector<Double_t> > > > fvCPTOff; //[nSMT][nRpc][nCh][nbSide]
    std::vector< std::vector<std::vector<std::vector<Double_t> > > > fvCPTotGain; //[nSMT][nRpc][nCh][nbSide]
    std::vector< std::vector<std::vector<std::vector<std::vector<Double_t> > > > > fvCPWalk; //[nSMT][nRpc][nCh][nbSide][nbWalkBins]
    TClonesArray* fTofDigis;
    TClonesArray* fTofPoints;
    TClonesArray* fTofHits;
    TClonesArray* fTofDigiMatchs;
    //std::vector<std::vector<std::vector<std::list<CbmTofDigiExp*> > > > fStorDigiExp; //[nbType][nbSm*nbRpc][nbCh]{nDigis}
#ifndef __CINT__
    
    
    std::vector<std::vector<std::vector<ChannelDigis> > > fStorDigiExp;
    
    std::vector<std::vector<std::vector<std::pair
     <std::pair
      <std::map
       <Double_t, std::pair
        <CbmTofDigiExp*, Int_t
        >
       >, std::map
       <Double_t, std::pair
        <CbmTofDigiExp*, Int_t
        >
       >
      >, std::map
      <Double_t, std::pair
       <Double_t, std::pair
        <std::pair
         <CbmTofDigiExp*, Int_t
         >, std::pair
         <CbmTofDigiExp*, Int_t
         >
        >
       >
      >
     >
    > > > fStorDigiExpOld;//[nbType][nbSm*nbRpc][nbCh]<[->|nTopDigis][->|nBottomDigis]>
#endif//__CINT__
    Double_t fOutTimeFactor;
    
    ClassDef(CbmTofTBClusterizer, 1);
};

#endif /* CBMTOFTBCLUSTERIZER_H */

