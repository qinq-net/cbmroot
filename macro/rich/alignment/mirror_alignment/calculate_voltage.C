#include <cmath>
#include <iostream>

int calculate_voltage(double alpha_mrad = 5.0) //, double distance = 0.15)
{
	// From angle to voltage
	double alpha_deg = alpha_mrad*pow(10.0, -3.0)*TMath::RadToDeg();
	std::cout << "Tilt in deg: " << alpha_deg << endl;

	double voltage_1 = ((5*25*sin(pow(10.0, -3.0) * alpha_mrad))/3);
	std::cout << "New Voltage 1: " << voltage_1 << endl;

	double voltage_2 = ((5*26*sin(pow(10.0, -3.0) * alpha_mrad))/3);
	std::cout << "New Voltage 2: " << voltage_2 << endl;

	// From distance to voltage
	// double voltage_bis = ((5*distance)/30);
	// std::cout << "New Voltage 2: " << voltage_bis << endl;
}
