
#ifndef RICH_CbmRichDraw
#define RICH_CbmRichDraw

#include "TObject.h"
#include "TH2.h"
#include "TCanvas.h"
#include <string>

using namespace std;

class CbmRichDraw {
    
public:
    
    static void DrawPmtH2(
                          TH2* h,
                          TCanvas* c)
    {
        if (c == NULL) return;
        c->Divide(1, 2);
        c->cd(1);
        TH2D* hUp = (TH2D*)h->Clone();
        DrawH2(hUp);
        hUp->GetYaxis()->SetRangeUser(120, 210);
        hUp->GetYaxis()->SetTitleOffset(0.75);
        hUp->GetZaxis()->SetTitleOffset(0.87);
        gPad->SetLeftMargin(0.1);
        gPad->SetRightMargin(0.15);
        c->cd(2);
        TH2D* hDown = (TH2D*)h->Clone();
        hDown->GetYaxis()->SetRangeUser(-210, -120);
        DrawH2(hDown);
        hDown->GetYaxis()->SetTitleOffset(0.75);
        hDown->GetZaxis()->SetTitleOffset(0.87);
        gPad->SetLeftMargin(0.1);
        gPad->SetRightMargin(0.15);
    }
    
};

#endif