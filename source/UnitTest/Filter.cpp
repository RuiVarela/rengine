#include "UnitTest/UnitTest.h"

#include <rengine/image/Filter.h>
#include <rengine/string/String.h>
#include <iostream>

using namespace rengine;


//
// UnitTestFilter
//

UNITT_TEST_BEGIN_CLASS(UnitTestFilter)

	virtual void run()
	{
		BoxFilter box_filter;
		UNITT_FAIL_NOT_EQUAL(1.0f, box_filter.sampleDelta(0.5f, 0.5f));
		UNITT_FAIL_NOT_EQUAL(1.0f, box_filter.sampleDelta(-0.5f, -0.5f));
		UNITT_FAIL_NOT_EQUAL(0.0f, box_filter.sampleDelta(-1.0f, -1.0f));
		UNITT_FAIL_NOT_EQUAL(0.0f, box_filter.sampleDelta(1.0f, 1.0f));

		UNITT_FAIL_NOT_EQUAL(1.0f, box_filter.sampleDelta(0.2f, 0.2f));
		UNITT_FAIL_NOT_EQUAL(1.0f, box_filter.sampleDelta(-0.2f, -0.2f));


		//BoxFilter filter0;
		//Kernel1D kernel0(filter0, 9, 1.0);

		//TriangleFilter filter1;
		//Kernel1D kernel1(filter1, 9, 1.0);

		//QuadraticFilter filter2;
		//Kernel1D kernel2(filter2, 9, 1.0);

		//CubicFilter filter3;
		//Kernel1D kernel3(filter3, 9, 1.0);

		//BsplineFilter filter4;
		//Kernel1D kernel4(filter4, 9, 1.0);

		//MitchellFilter filter5;
		//Kernel1D kernel5(filter5, 9, 1.0);

		//LanczosFilter filter6;
		//Kernel1D kernel6(filter6, 9, 1.0);

		//KaiserFilter filter7;
		//Kernel1D kernel7(filter7, 9, 1.0);


		//GaussianFilter filter8;
		//Kernel2D kernel8(filter8, 7, 1.0);

	//  std::cout << kernel8.toString() << std::endl;
	//	std::cout << "out " << filter.sampleDelta(0.15, 1.0f) << std::endl;
	//	std::cout << "out " << filter.sampleBox(0.15, 1.0f, 16) << std::endl;
	//	std::cout << "out " << filter.sampleTriangle(0.15, 1.0f, 16) << std::endl;

		//box_filter.sampleTriangle(5.0, 1.0f, 32);
	}

UNITT_TEST_END_CLASS(UnitTestFilter)
