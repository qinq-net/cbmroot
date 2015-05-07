#include "MultiscatteringModel.h"
#include <iostream>

namespace MultiscatteringModelNamespace {

#include "../NumericalIntegration.h"

}

MultiscatteringModel::MultiscatteringModel() {
	MultiscatteringModelNamespace::GetCoefsIntegrateLaguerre32(xlag32, wlag32);
	std::cout << "Size = " << xlag32.size() << " " << wlag32.size() << "\n";
}
