
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
    vector<vector<int> > data = ReadTiffFile(fTiffFileName);

    int width = data.size();
    int height = data[0].size();

    TH2D* hist = new TH2D("image", "image", width, -.5, width - 0.5, height, -0.5, height - 0.5);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            hist->SetBinContent(x, y, data[x][y]);
        }
    }
    TCanvas* c = new TCanvas("c", "c", 1000, 1000);
    DrawH2(hist);

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
    for (int x = 0; x < width; ++x) {
        auto it = view.col_begin(x);
        data[x].resize(height);
        for (int y = 0; y < height; ++y) {
            int r = boost::gil::at_c<0>(it[y]);
            //int g = boost::gil::at_c<1>(it[y]);
            //int b = boost::gil::at_c<2>(it[y]);
            //int a = boost::gil::at_c<3>(it[y]);
            data[x][y] = r;
        }
    }
    return data;
}
