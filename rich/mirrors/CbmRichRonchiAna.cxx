
#include "CbmRichRonchiAna.h"

#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/tiff_dynamic_io.hpp>

using namespace boost::gil;

CbmRichRonchiAna::CbmRichRonchiAna()
{
    rgb8_image_t img;
    tiff_read_image("test.tiff",img);

//    int h = img.height();
//    int w = img.width();
//    auto view = const_view(img);
//    for (int x = 0; x < w; ++x)
//    {
//        boost::gil::rgb8_view_t::y_iterator it = view.col_begin(x);
//        for (int y = 0; y < h; ++y)
//        {
////            boost::gil::at_c<0>(it[y]);
////            boost::gil::at_c<1>(it[y]);
////            boost::gil::at_c<2>(it[y]);
////            boost::gil::at_c<3>(it[y]);
//        }
//    }
}

CbmRichRonchiAna::~CbmRichRonchiAna() {

}

void CbmRichRonchiAna::Run() {


}
