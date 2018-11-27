
#ifndef RICH_CbmRichDraw
#define RICH_CbmRichDraw

#include "TObject.h"
#include "TH2.h"
#include "TGraph2D.h"
#include "TCanvas.h"
#include "TVector.h"
#include <string>
#include <iomanip>
#include "detector/CbmRichDigiMapManager.h"
#include "detector/CbmRichGeoManager.h"

using namespace std;

class CbmRichDraw {
    
public:
    
    static void DrawPmtH2(
                          TH2* h,
                          TCanvas* c,
                          Bool_t usePmtBins = false)
    {
        if (c == NULL) return;
        c->Divide(1, 2);
        c->cd(1);
        TH2D* hUp = (TH2D*)h->Clone();
        DrawH2(hUp);
        if (usePmtBins){
            vector<Double_t> yPmtBins = CbmRichDraw::GetPmtHistYbins();
            hUp->GetYaxis()->SetRange(yPmtBins.size() / 2 + 1, yPmtBins.size());
        } else {
            hUp->GetYaxis()->SetRangeUser(120, 210);
        }
        hUp->GetYaxis()->SetTitleOffset(0.75);
        hUp->GetZaxis()->SetTitleOffset(0.87);
        gPad->SetLeftMargin(0.1);
        gPad->SetRightMargin(0.15);
        c->cd(2);
        TH2D* hDown = (TH2D*)h->Clone();
        if (usePmtBins){
            vector<Double_t> yPmtBins = CbmRichDraw::GetPmtHistYbins();
            hDown->GetYaxis()->SetRange(0, yPmtBins.size() / 2 - 1);
        } else {
            hDown->GetYaxis()->SetRangeUser(-210, -120);
        }
        DrawH2(hDown);
        hDown->GetYaxis()->SetTitleOffset(0.75);
        hDown->GetZaxis()->SetTitleOffset(0.87);
        gPad->SetLeftMargin(0.1);
        gPad->SetRightMargin(0.15);
    }
    
    static void DrawPmtGraph2D(
                          TGraph2D* gUp,
                          TGraph2D* gDown,
                          TCanvas* c)
    {
        if (c == NULL) return;
        c->Divide(1, 2);
        c->cd(1);
        DrawGraph2D(gUp);
        gUp->GetYaxis()->SetTitleOffset(0.75);
        gUp->GetZaxis()->SetTitleOffset(0.87);
        gPad->SetLeftMargin(0.1);
        gPad->SetRightMargin(0.15);
        c->cd(2);
        DrawGraph2D(gDown);
        gDown->GetYaxis()->SetTitleOffset(0.75);
        gDown->GetZaxis()->SetTitleOffset(0.87);
        gPad->SetLeftMargin(0.1);
        gPad->SetRightMargin(0.15);
    }

    static vector<Double_t> GetPmtHistXbins()
    {
        return CbmRichDraw::GetPmtHistBins(true);
    }

    static vector<Double_t> GetPmtHistYbins()
    {
        return CbmRichDraw::GetPmtHistBins(false);
    }

private:
    static vector<Double_t> GetPmtHistBins(Bool_t isX)
    {
        vector<Double_t> initVec;
        vector<Int_t> pmts = CbmRichDigiMapManager::GetInstance().GetPmtIds();
        for (Int_t pmtId : pmts) {
            CbmRichPmtData* pmtData = CbmRichDigiMapManager::GetInstance().GetPmtDataById(pmtId);
            TVector3 inPos(pmtData->fX, pmtData->fY, pmtData->fZ);
            TVector3 outPos;
            CbmRichGeoManager::GetInstance().RotatePoint(&inPos, &outPos);
            if (isX) {
                initVec.push_back(outPos.X() - 0.5 * pmtData->fWidth);
                initVec.push_back(outPos.X() + 0.5 * pmtData->fWidth);
            } else {
                initVec.push_back(outPos.Y() - 0.5 * pmtData->fHeight);
                initVec.push_back(outPos.Y() + 0.5 * pmtData->fHeight);
            }
        }
        sort( initVec.begin(), initVec.end() );

        vector<Double_t> uniVec;
        for (int i = 0; i < initVec.size(); i++) {
            if (i == 0) uniVec.push_back(initVec[i]);
            if (initVec[i] - uniVec[uniVec.size() - 1] > 0.000001) uniVec.push_back(initVec[i]);
        }

//        cout << "uniVec.size():" << uniVec.size() << endl;
//        for (int i = 0; i < uniVec.size(); i++) {
//            cout << std::setprecision(9);
//            cout << fixed;
//            cout << uniVec[i] << " " ;
//        }
//        cout << endl;

        return uniVec;
    }

};

#endif
