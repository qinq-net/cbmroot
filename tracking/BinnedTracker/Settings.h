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
    CbmBinnedSettings() : FairParGenericSet("CbmBinnedSettings", "Binned tracker reconstruction parameters", "Default"),
            fIsConfiguring(false), fNofStsStations(0), fNofMuchStations(0), fNofTrdStations(0), fStationErrors()
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
        
        TArrayC detFlags(kLastModule);
        
        for (int i = 0; i < kLastModule; ++i)
            detFlags[i] = fUseModules[i];
        
        l->add("UseModules", detFlags);
        l->add("NofStsStations", fNofStsStations);
        l->add("NofMuchStations", fNofMuchStations);
        l->add("NofTrdStations", fNofTrdStations);
        
        Int_t nofStations = fStationErrors.size();
        l->add("NofStations", nofStations);
        TArrayD stationErrorArray(3 * nofStations);
        
        for (int i = 0; i < nofStations; ++i)
        {
            stationErrorArray[3 * i] = std::get<0> (fStationErrors[i]);
            stationErrorArray[3 * i + 1] = std::get<1> (fStationErrors[i]);
            stationErrorArray[3 * i + 2] = std::get<2> (fStationErrors[i]);
        }
        
        l->add("StationErrors", stationErrorArray);
    }
    
    Bool_t getParams(FairParamList* l)
    {
        if (0 == l)
            return kFALSE;

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
        
        Int_t nofStations;
        
        if (!l->fill("NofStations", &nofStations))
            return kFALSE;
        
        TArrayD stationErrorArray(3 * nofStations);
        
        if (!l->fill("StationErrors", &stationErrorArray))
            return kFALSE;
        
        fStationErrors.resize(nofStations);
        
        for (int i = 0; i < nofStations; ++i)
        {
            std::get<0> (fStationErrors[i]) = stationErrorArray[3 * i];
            std::get<1> (fStationErrors[i]) = stationErrorArray[3 * i + 1];
            std::get<2> (fStationErrors[i]) = stationErrorArray[3 * i + 2];
        }
        
        return kTRUE;
    }
    // ~Overridded virtual methods
    
    bool IsConfiguring() const { return fIsConfiguring; }
    void SetConfiguring() { fIsConfiguring = true; }
    bool Use(ECbmModuleId m) const { return fUseModules[m]; }
    void SetUse(ECbmModuleId m, bool v) { fUseModules[m] = v; }
    void SetUse(bool v) { std::fill_n(fUseModules, int(kLastModule), v); }
    Int_t GetNofStsStations() const { return fNofStsStations; }
    void SetNofStsStations(Int_t v) { fNofStsStations = v; }
    Int_t GetNofMuchStations() const { return fNofMuchStations; }
    void SetNofMuchStations(Int_t v) { fNofMuchStations = v; }
    Int_t GetNofTrdStations() const { return fNofTrdStations; }
    void SetNofTrdStations(Int_t v) { fNofTrdStations = v; }
    void AddStationErrors(Double_t xErr, Double_t yErr, Double_t tErr) { fStationErrors.push_back(std::make_tuple(xErr, yErr, tErr)); }
    Int_t GetNofStations() const { return fStationErrors.size(); }
    Double_t GetXError(int stationNumber) const { return std::get<0> (fStationErrors[stationNumber]); }
    Double_t GetYError(int stationNumber) const { return std::get<1> (fStationErrors[stationNumber]); }
    Double_t GetTError(int stationNumber) const { return std::get<2> (fStationErrors[stationNumber]); }
    
    
private:
    bool fIsConfiguring;// Not persistent flag. Used to know if we are in configuration mode.
    bool fUseModules[kLastModule];
    Int_t fNofStsStations;
    Int_t fNofMuchStations;
    Int_t fNofTrdStations;
    std::vector<std::tuple<Double_t, Double_t, Double_t> > fStationErrors;
    
    ClassDef(CbmBinnedSettings, 2)
};

#endif /* CBM_BINNED_SETTINGS_H */
