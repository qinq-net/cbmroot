
#include "CbmRichRonchiAna.h"

#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/tiff_dynamic_io.hpp>
//#include <boost/gil/extension/io/png_dynamic_io.hpp>

#include <iostream>
#include "TH2D.h"
#include "TCanvas.h"
#include "CbmDrawHist.h"

using namespace boost::gil;
using namespace std;

// For Ubuntu one needs dev version of libtiff
//sudo apt-get install libtiff-dev

CbmRichRonchiAna::CbmRichRonchiAna()
{

}

CbmRichRonchiAna::~CbmRichRonchiAna()
{

}

void CbmRichRonchiAna::Run()
{
    vector<vector<int> > dataV = ReadTiffFile(fTiffFileNameV);
    vector<vector<int> > dataH = ReadTiffFile(fTiffFileNameH);

    int width = dataV.size();
    int height = dataV[0].size();

    TH2D* hInitH = new TH2D("hInitH", "hInitH;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hMeanIntensityH = new TH2D("hMeanIntensityH", "hMeanIntensityH;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hPeakH = new TH2D("hPeakH", "hPeakH;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);

    TH2D* hInitV = new TH2D("hInitV", "hInitV;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hMeanIntensityV = new TH2D("hMeanIntensityV", "hMeanIntensityV;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);
    TH2D* hPeakV = new TH2D("hPeakV", "hPeakV;X [pixel];Y [pixel];Intensity", width, -.5, width - 0.5, height, -0.5, height - 0.5);


    // vertical image
    FillH2WithVector(hInitV, dataV);
    DoRotation(dataV);
    DoMeanIntensityY(dataV);
    FillH2WithVector(hMeanIntensityV, dataV);
    DoPeakFinderY(dataV);
    DoRotation(dataV);
    FillH2WithVector(hPeakV, dataV);

    // horizontal image
    FillH2WithVector(hInitH, dataH);
    DoMeanIntensityY(dataH);
    FillH2WithVector(hMeanIntensityH, dataH);
    DoPeakFinderY(dataH);
    FillH2WithVector(hPeakH, dataH);


    {
        TCanvas* c = new TCanvas("ronchi_2d_horizontal", "ronchi_2d_horizontal", 1800, 600);
        c->Divide(3,1);
        c->cd(1);
        DrawH2(hInitH);
        c->cd(2);
        DrawH2(hMeanIntensityH);
        c->cd(3);
        DrawH2(hPeakH);
    }

    {
        TCanvas* c = new TCanvas("ronchi_2d_vertical", "ronchi_2d_vertical", 1800, 600);
        c->Divide(3,1);
        c->cd(1);
        DrawH2(hInitV);
        c->cd(2);
        DrawH2(hMeanIntensityV);
        c->cd(3);
        DrawH2(hPeakV);
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

}

void CbmRichRonchiAna::DoMeanY( vector<vector<int> >& data)
{

    // calculating mean position in Y

//    int meanHalfLength = 2;
//    int meanHalfHeight = 5;
//
//    for (int x = meanHalfLength; x < width-meanHalfLength; x++) {
//        for (int y = meanHalfHeight; y < height-meanHalfHeight; y++) {
//            if (image[x][y] == 0) continue;
//            int curData = image[x][y];
//            int sumY = 0;
//            int divider = 0;
//            for (int x2 = -meanHalfLength; x2 <= meanHalfLength; x2++) {
//                for (int y2 = -meanHalfHeight; y2 <= meanHalfHeight; y2++) {
//                    if (image[x+x2][y+y2] > 0) {
//                        sumY += y+y2;
//                        divider++;
//                    }
//                }
//            }
//            image[x][y] = 0;
//            y=(int) sumY/divider;   // might be calculated more precisely by rounding
//            image[x][y] = curData;
//            hMeanY->SetBinContent(x, y, image[x][y]);
//            curData = 0;
//            sumY = 0;
//        }
//    }
}

void CbmRichRonchiAna::DoRotation( vector<vector<int> >& data)
{
    int nX = data.size();
    int nY = data[0].size();
    for (int x = 0; x < nX; x++) {
        for (int y = x+1; y < nY; y++) {
            swap(data[x][y], data[y][x]);
        }
    }
}

void CbmRichRonchiAna::DoPeakFinderY( vector<vector<int> >& data)
{
    int nX = data.size();
    int nY = data[0].size();
    int halfWindow = 8;

    for (int x = 0; x < nX; x++) {
        for (int y = 0 + halfWindow; y < nY - halfWindow; y++){
            bool isPeak = (data[x][y] >= data[x][y - 1]) && (data[x][y] >= data[x][y + 1]);
            bool isBiggest = true;
            for (int iW = -halfWindow; iW <= halfWindow; iW++) {
                if (iW == 0) continue;
                if (data[x][y + iW] >= data[x][y]) {
                    isBiggest = false;
                    break;
                }
            }
            data[x][y] = (isBiggest && isPeak) ? data[x][y] : 0;
        }
    }
}

void CbmRichRonchiAna::DoMeanIntensityY(vector<vector<int> >& data)
{
    int nX = data.size();
    int nY = data[0].size();

    int halfAvWindow = 4;
    int threshold = 25;
    vector<int> weights = {100, 80, 50, 20, 10, 5};

    for (int x = 0; x < nX; x++) {
        for (int y = 0; y < nY; y++){
            int total = 0;
            int weightSum = 0;
            for (int iW = -halfAvWindow; iW <= halfAvWindow; iW++) {
                int iWAbs = std::abs(iW);
                int weight = (iWAbs < weights.size())?weights[iWAbs]:weights[weights.size() - 1];
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
