/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Settings.h
 * Author: tablyaz
 *
 * Created on October 4, 2017, 12:57 PM
 */

#ifndef CBM_BINNED_SETTINGS_H
#define CBM_BINNED_SETTINGS_H

#include "CbmDefs.h"
#include "FairParGenericSet.h"
#include "FairParamList.h"
#include "TArrayD.h"
#include <tuple>

class CbmBinnedSettings : public FairParGenericSet
{
public:
    static CbmBinnedSettings* Instance()
    {
        static CbmBinnedSettings* theInstance = 0;
        
        if (0 == theInstance)
            theInstance = new CbmBinnedSettings;
        
        return theInstance;
    }
    
public:
    CbmBinnedSettings() : FairParGenericSet("CbmBinnedSettings", "Binned tracker reconstruction parameters", "Default"), fIsConfiguring(false), fIsOnlyPrimary(true),
            fNofStsStations(0), fNofMuchStations(0), fNofTrdStations(0)/*, fStationErrors()*/, fNofStations(0), fXScats(), fYScats()
    {
        std::fill_n(fUseModules, int(kLastModule), true);
    }
    
    CbmBinnedSettings(const CbmBinnedSettings&) = delete;
    CbmBinnedSettings& operator=(const CbmBinnedSettings&) = delete;
    
    void clear() 
    {
        status = kFALSE;
        resetInputVersions();
    }
    
    // Overridden virtual methods
    void putParams(FairParamList* l)
    {
        if (0 == l)
            return;
        
        l->add("OnlyPrimary", fIsOnlyPrimary);
        
        TArrayC detFlags(kLastModule);
        
        for (int i = 0; i < kLastModule; ++i)
            detFlags[i] = fUseModules[i];
        
        l->add("UseModules", detFlags);
        l->add("NofStsStations", fNofStsStations);
        l->add("NofMuchStations", fNofMuchStations);
        l->add("NofTrdStations", fNofTrdStations);
        
        /*Int_t nofStations = fStationErrors.size();
        l->add("NofStations", nofStations);
        TArrayD stationErrorArray(3 * nofStations);
        
        for (int i = 0; i < nofStations; ++i)
        {
            stationErrorArray[3 * i] = std::get<0> (fStationErrors[i]);
            stationErrorArray[3 * i + 1] = std::get<1> (fStationErrors[i]);
            stationErrorArray[3 * i + 2] = std::get<2> (fStationErrors[i]);
        }
        
        l->add("StationErrors", stationErrorArray);*/
        
        l->add("NofStations", fNofStations);
        TArrayD stationXScatArray(fNofStations);
        TArrayD stationYScatArray(fNofStations);
        
        for (int i = 0; i < fNofStations; ++i)
        {
            stationXScatArray[i] = fXScats[i];
            stationYScatArray[i] = fYScats[i];
        }
        
        l->add("StationXScats", stationXScatArray);
        l->add("StationYScats", stationYScatArray);
    }
    
    Bool_t getParams(FairParamList* l)
    {
        if (0 == l)
            return kFALSE;
        
        Int_t op = 0;
        
        if (!l->fill("OnlyPrimary", &op))
            return kFALSE;
        
        fIsOnlyPrimary = op != 0;

        TArrayC detFlags(kLastModule);
        
        if (!l->fill("UseModules", &detFlags))
            return kFALSE;
        
        for (int i = 0; i < kLastModule; ++i)
            fUseModules[i] = detFlags[i];
        
        if (!l->fill("NofStsStations", &fNofStsStations))
            return kFALSE;
        
        if (!l->fill("NofMuchStations", &fNofMuchStations))
            return kFALSE;
        
        if (!l->fill("NofTrdStations", &fNofTrdStations))
            return kFALSE;
        
        if (!l->fill("NofStations", &fNofStations))
            return kFALSE;
        
        /*TArrayD stationErrorArray(3 * nofStations);
        
        if (!l->fill("StationErrors", &stationErrorArray))
            return kFALSE;
        
        fStationErrors.resize(nofStations);
        
        for (int i = 0; i < nofStations; ++i)
        {
            std::get<0> (fStationErrors[i]) = stationErrorArray[3 * i];
            std::get<1> (fStationErrors[i]) = stationErrorArray[3 * i + 1];
            std::get<2> (fStationErrors[i]) = stationErrorArray[3 * i + 2];
        }*/
        
        TArrayD stationXScats(fNofStations);
        
        if (!l->fill("StationXScats", &stationXScats))
            return kFALSE;
        
        TArrayD stationYScats(fNofStations);
        
        if (!l->fill("StationYScats", &stationYScats))
            return kFALSE;
        
        for (int i = 0; i < fNofStations; ++i)
        {
            fXScats[i] = stationXScats[i];
            fYScats[i] = stationYScats[i];
        }
        
        return kTRUE;
    }
    // ~Overridded virtual methods
    
    bool IsConfiguring() const { return fIsConfiguring; }
    void SetConfiguring(bool v) { fIsConfiguring = v; }
    bool IsOnlyPrimary() const { return fIsOnlyPrimary; }
    void SetOnlyPrimary(bool v) { fIsOnlyPrimary = v; }
    bool Use(ECbmModuleId m) const { return fUseModules[m]; }
    void SetUse(ECbmModuleId m, bool v) { fUseModules[m] = v; }
    void SetUse(bool v) { std::fill_n(fUseModules, int(kLastModule), v); }
    void SetUse(bool useModules[kLastModule]) { std::copy(useModules, useModules + kLastModule, fUseModules); }
    Int_t GetNofStsStations() const { return fNofStsStations; }
    void SetNofStsStations(Int_t v) { fNofStsStations = v; }
    Int_t GetNofMuchStations() const { return fNofMuchStations; }
    void SetNofMuchStations(Int_t v) { fNofMuchStations = v; }
    Int_t GetNofTrdStations() const { return fNofTrdStations; }
    void SetNofTrdStations(Int_t v) { fNofTrdStations = v; }
    Int_t GetNofStations() const { return fNofStations; }
    void SetNofStations(Int_t v) { fNofStations = v; }
    /*void AddStationErrors(Double_t xErr, Double_t yErr, Double_t tErr) { fStationErrors.push_back(std::make_tuple(xErr, yErr, tErr)); }
    Int_t GetNofStations() const { return fStationErrors.size(); }
    Double_t GetXError(int stationNumber) const { return std::get<0> (fStationErrors[stationNumber]); }
    Double_t GetYError(int stationNumber) const { return std::get<1> (fStationErrors[stationNumber]); }
    Double_t GetTError(int stationNumber) const { return std::get<2> (fStationErrors[stationNumber]); }*/
    
    void AddStationScats(Double_t x, Double_t y)
    {
        fXScats.push_back(x);
        fYScats.push_back(y);
        ++fNofStations;
    }
    
    Double_t GetXScat(int stationNumber) const { return fXScats[stationNumber]; }
    Double_t GetYScat(int stationNumber) const { return fYScats[stationNumber]; }
    
private:
    bool fIsConfiguring;
    bool fIsOnlyPrimary;
    bool fUseModules[kLastModule];
    Int_t fNofStsStations;
    Int_t fNofMuchStations;
    Int_t fNofTrdStations;
    //std::vector<std::tuple<Double_t, Double_t, Double_t> > fStationErrors;
    Int_t fNofStations;
    std::vector<Double_t> fXScats;
    std::vector<Double_t> fYScats;
    
    ClassDef(CbmBinnedSettings, 7)
};

#endif /* CBM_BINNED_SETTINGS_H */
