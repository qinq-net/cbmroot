
#include "CbmRichRonchiAna.h"

#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/tiff_dynamic_io.hpp>
//#include <boost/gil/extension/io/png_dynamic_io.hpp>

#include <iostream>
#include "TH2D.h"
#include "TCanvas.h"
#include "CbmDrawHist.h"
#include "TEllipse.h"
#include <math.h>

using namespace boost::gil;
using namespace std;

// For Ubuntu one needs dev version of libtiff
// sudo apt-get install libtiff-dev

CbmRichRonchiAna::CbmRichRonchiAna()
{

}

CbmRichRonchiAna::~CbmRichRonchiAna()
{

}

void CbmRichRonchiAna::Run()
{
    // Initialization
    vector<vector<int> > dataV;
    vector<vector<int> > dataH;
        
    if ( fTiffFileNameV == "" || fTiffFileNameH == "" ) {
        Fatal("CbmRichRonchiAna::Run:", "No FileNameV or FileNameH!");
    }
    else { cout << "FileNameV: " << fTiffFileNameV << " and FileNameH: " << fTiffFileNameH << endl;
        dataV = ReadTiffFile(fTiffFileNameV);
        dataH = ReadTiffFile(fTiffFileNameH);
    }

    int width = dataV.size();
    int height = dataV[0].size();

    TH2D* hInitH = new TH2D("hInitH", "hInitH;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hMeanIntensityH = new TH2D("hMeanIntensityH", "hMeanIntensityH;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hPeakH = new TH2D("hPeakH", "hPeakH;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hMeanYH = new TH2D("hMeanYH", "hMeanYH;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);

    TH2D* hInitV = new TH2D("hInitV", "hInitV;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hMeanIntensityV = new TH2D("hMeanIntensityV", "hMeanIntensityV;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hPeakV = new TH2D("hPeakV", "hPeakV;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hMeanYV = new TH2D("hMeanYV", "hMeanYV;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    
    TH2D* hSuperpose = new TH2D("hSuperpose", "hSuperpose;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    
    TH2D* hIntersections = new TH2D("hIntersections", "hIntersections;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);


    // vertical image
    FillH2WithVector(hInitV, dataV);
    DoRotation(dataV);
    DoMeanIntensityY(dataV);
    FillH2WithVector(hMeanIntensityV, dataV);
    DoPeakFinderY(dataV);
    FillH2WithVector(hPeakV, dataV);
    DoMeanY(dataV);
    DoRotation(dataV);
    FillH2WithVector(hMeanYV, dataV);

    // horizontal image
    FillH2WithVector(hInitH, dataH);
    DoMeanIntensityY(dataH);
    FillH2WithVector(hMeanIntensityH, dataH);
    DoPeakFinderY(dataH);
    FillH2WithVector(hPeakH, dataH);
    DoMeanY(dataH);
    FillH2WithVector(hMeanYH, dataH);

    // finding intersections
    vector<pair<int,int> > intersectionXY = DoIntersection(dataH, dataV);
    vector<vector<int> > dataSup = DoSuperpose(dataH, dataV);
    FillH2WithVector(hSuperpose, dataSup);
    
    // numbering the intersections
    vector<pair<int,int> > intNumberXY = DoNumInt(intersectionXY, dataSup);
    
 
    {
        TCanvas* c = new TCanvas("ronchi_2d_horizontal", "ronchi_2d_horizontal", 2000, 500);
        c->Divide(4,1);
        c->cd(1);
        DrawH2(hInitH);
        c->cd(2);
        DrawH2(hMeanIntensityH);
        c->cd(3);
        DrawH2(hPeakH);
        c->cd(4);
        DrawH2(hMeanYH);
    }

    {
        TCanvas* c = new TCanvas("ronchi_2d_vertical", "ronchi_2d_vertical", 2000, 500);
        c->Divide(4,1);
        c->cd(1);
        DrawH2(hInitV);
        c->cd(2);
        DrawH2(hMeanIntensityV);
        c->cd(3);
        DrawH2(hPeakV);
        c->cd(4);
        DrawH2(hMeanYV);
    }
    
    {
        TCanvas* c = new TCanvas("Superpose", "Superpose", 1000, 1000);
        DrawH2(hSuperpose);
    }
    
    {
        TH1D* h1 = hInitH->ProjectionY("_py1", 100, 100);
        TH1D* h2 = hInitH->ProjectionY("_py2", 200, 200);
        TH1D* h3 = hInitH->ProjectionY("_py3", 300, 300);
        TH1D* h4 = hInitH->ProjectionY("_py4", 500, 500);

        TH1D* hM1 = hMeanIntensityH->ProjectionY("_pyM1", 100, 100);
        TH1D* hM2 = hMeanIntensityH->ProjectionY("_pyM2", 200, 200);
        TH1D* hM3 = hMeanIntensityH->ProjectionY("_pyM3", 300, 300);
        TH1D* hM4 = hMeanIntensityH->ProjectionY("_pyM4", 400, 400);

        TH1D* hP1 = hPeakH->ProjectionY("_pyP1", 100, 100);
        TH1D* hP2 = hPeakH->ProjectionY("_pyP2", 200, 200);
        TH1D* hP3 = hPeakH->ProjectionY("_pyP3", 300, 300);
        TH1D* hP4 = hPeakH->ProjectionY("_pyP4", 400, 400);

        TCanvas* c2 = new TCanvas("ronchi_1d_slices_horizontal", "ronchi_1d_slices_horizontal", 1000, 1000);
        c2->Divide(2,2);
        c2->cd(1);
        DrawH1({h1,hM1,hP1}, {"Init", "Mean", "Peak"});
        c2->cd(2);
        DrawH1({h2,hM2,hP2}, {"Init", "Mean", "Peak"});
        c2->cd(3);
        DrawH1({h3,hM3,hP3}, {"Init", "Mean", "Peak"});
        c2->cd(4);
        DrawH1({h4,hM4,hP4}, {"Init", "Mean", "Peak"});
    }
    
    {
        TCanvas* c = new TCanvas("ronchi_2d_intersection", "ronchi_2d_intersection", 1000, 1000);
        DrawH2(hSuperpose);
        
        for (int i = 0; i < intersectionXY.size(); i++) {
            pair<int,int> xy = intersectionXY[i];
            TEllipse* center = new TEllipse(xy.first, xy.second, 3);
            center->Draw();
        }
    }
}

// /*
vector<pair<int,int>> CbmRichRonchiAna::DoNumInt(vector<pair<int,int>>& intersectionXY, vector<vector<int>>& dataSup)
{
    
    int nX = intersectionXY.size();
    int intDistance = 20; // approx. distance of intersections
    int counterLine = 0;
    int currentY = 0;
    int intersectionX = 0;
    int intersectionY = 0;
    
    bool line = false;
    bool intersection = false;
  
    vector<pair<int,int>> intNumberXY;
    
    for (int y0 = 20; y0 < nX; y0++) {
        if (dataSup[511][y0] > 0) currentY = y0;
        for (int x = 510; x > 20; x--) {
            for (int y = y0-1; y <= y0+1; y++) {
                if (dataSup[x][y] > 0) {
                    line = true;
                    currentY = y0;
                }
                for (int i = 0; i <= nX; i++) {
                    if (intersectionXY[i].first == intersectionX && intersectionXY[i].second == intersectionY );  // nach Eintrag aus intersectionXY suchen, der den aktuellen x- und y-Werten entspricht
                    
                }
            } 
        }        
    }
    
    return intNumberXY;

} // */


void CbmRichRonchiAna::DoMeanY(vector<vector<int> >& data)
{

    // calculating mean position in Y

    int meanHalfLength = 5;
    int meanHalfHeight = 3;
    int nX = data.size();
    int nY = data[0].size();
    
    for (int x = meanHalfLength; x < nX-meanHalfLength; x++) {
        for (int y = meanHalfHeight; y < nY-meanHalfHeight; y++) {
            if (data[x][y] == 0) continue;
            int curData = data[x][y];
            int sumY = 0;
            int divider = 0;
            for (int x2 = -meanHalfLength; x2 <= meanHalfLength; x2++) {
                for (int y2 = -meanHalfHeight; y2 <= meanHalfHeight; y2++) {
                    if (data[x+x2][y+y2] > 0) {
                        sumY += y+y2;
                        divider++;
                    }
                }
            }
            data[x][y] = 0;
            y=(int) sumY/divider;   // might be calculated more precisely by rounding
            data[x][y] = curData;
            curData = 0;
            sumY = 0;
        }
    }
}

void CbmRichRonchiAna::DoRotation(vector<vector<int> >& data)
{
    int nX = data.size();
    int nY = data[0].size();
    for (int x = 0; x < nX; x++) {
        for (int y = x+1; y < nY; y++) {
            swap(data[x][y], data[y][x]);
        }
    }
}

void CbmRichRonchiAna::DoPeakFinderY(vector<vector<int> >& data)
{
    int nX = data.size();
    int nY = data[0].size();
    int halfWindow = 10;

    for (int x = 0; x < nX; x++) {
        for (int y = 0 + halfWindow; y < nY - halfWindow; y++){
            bool isPeak = (data[x][y] >= data[x][y - 1]) && (data[x][y] >= data[x][y + 1]);
            if (!isPeak) {
                data[x][y] = 0;
                continue;
            }
            bool isBiggest = true;
            for (int iW = -halfWindow; iW <= halfWindow; iW++) {
                if (iW == 0) continue;
                if (data[x][y + iW] >= data[x][y]) {
                    isBiggest = false;
                    break;
                }
            }
            data[x][y] = (isBiggest && isPeak)? data[x][y] : 0;
        }
    }
}

void CbmRichRonchiAna::DoMeanIntensityY(vector<vector<int> >& data)
{
    int nX = data.size();
    int nY = data[0].size();

    int halfAvWindow = 6;
    int threshold = 23;
    vector<int> weights = {100, 80, 50, 20, 10, 5};

    for (int x = 0; x < nX; x++) {
        for (int y = 0; y < nY; y++){
            int total = 0;
            int weightSum = 0;
            for (int iW = -halfAvWindow; iW <= halfAvWindow; iW++) {
                int iWAbs = std::abs(iW);
                int weight = (iWAbs < weights.size() )? weights[iWAbs] : weights[weights.size() - 1];
                weightSum += weight;
                int ind = y + iW;
                if (ind < 0) ind = 0;
                if (ind >= nY) ind = nY - 1;
                total += data[x][ind] * weight;
            }
            data[x][y] = total / weightSum;
            if (data[x][y] <= threshold) data[x][y] = 0;
        }
    }
}

void CbmRichRonchiAna::FillH2WithVector(TH2* hist, const vector<vector<int> >& data)
{
    int nX = data.size();
    int nY = data[0].size();

    for (int x = 0; x < nX; x++) {
        for (int y = 0; y < nY; y++){
            if (data[x][y] != 0) {
                hist->SetBinContent(x, y, data[x][y]);
            }
        }
    }
}

vector<pair<int,int> > CbmRichRonchiAna::DoIntersection(const vector<vector<int> >& dataH, const vector<vector<int> >& dataV)
{
    int nX = dataV.size();
    int nY = dataV[0].size();
    
    vector<pair<int,int> > intersectionXY;
    
    for (int y = 0; y < nY; y++) {
        for (int x = 0; x < nX; x++) {
            if (dataH[x][y] > 0 && dataV[x][y] > 0) {
                intersectionXY.push_back(make_pair(x,y));
            }
        }
    }
    cout << "Number of intersections: " << intersectionXY.size() << endl;
    for (int i = 0; i < intersectionXY.size(); i++) {
        cout << "[" << intersectionXY[i].first << "," << intersectionXY[i].second << "]" << endl;
    }    
    return intersectionXY;
}


vector<vector<int> > CbmRichRonchiAna::DoSuperpose(const vector<vector<int> >& dataH, const vector<vector<int> >& dataV)
{
    int nX = dataV.size();
    int nY = dataV[0].size();
    
    vector<vector<int> > dataSup;
    dataSup.resize(nX);
    for (int x = 0; x < nX; x++) {
        dataSup[x].resize(nY);
        for (int y = 0; y < nY; y++) {
            dataSup[x][y] = dataH[x][y] + dataV[x][y];
        }
    }
    return dataSup;
}


vector<vector<int> > CbmRichRonchiAna::ReadTiffFile(const string& fileName)
{
    vector<vector<int> > data;
    rgba8_image_t img;
    tiff_read_and_convert_image(fileName,img);

    int height = img.height();
    int width = img.width();

    data.resize(width);

    auto view = const_view(img);
    for (int x = 0; x < width; ++x) 
    {
        auto it = view.col_begin(x);
        data[x].resize(height);
        for (int y = 0; y < height; ++y) 
        {
            int r = boost::gil::at_c<0>(it[y]);
            //int g = boost::gil::at_c<1>(it[y]);
            //int b = boost::gil::at_c<2>(it[y]);
            //int a = boost::gil::at_c<3>(it[y]);
            data[x][y] = r;
        }
    }
    return data;
}


ClassImp(CbmRichRonchiAna)
