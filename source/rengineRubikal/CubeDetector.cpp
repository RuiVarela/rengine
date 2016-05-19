#include "CubeDetector.h"

#include <limits>
#include <iostream>
#include <cassert>

#include <opencv/highgui.h>
#include <opencv/cv.hpp>

#include <rengine/CoreEngine.h>
#include <rengine/outputstream/OutputStream.h>
#include <rengine/image/Image.h>
#include <rengine/resource/ResourceManager.h>
#include <rengine/state/Texture.h>
#include <rengine/state/Program.h>
#include <rengine/string/String.h>
#include <rengine/RenderEngine.h>

using namespace rengine;

Vector4D text_color(1.0f, 0.0f, 0.0f, 1.0f);


int CubeDetector::histogram_bins = 16;
float CubeDetector::histogram_ranges_array[2] = { 0.0f, 180.0f }; /* hue varies from 0 (~0°red) to 180 (~360°red again) */
float* CubeDetector::histogram_ranges = histogram_ranges_array;


double const similar_point_max_distance = 15.0f; // minimum pixel distance between points

double const duplicate_center_max_distance = 6.0f; // minimum pixel distance between square centers
double const invalid_max_area_factor = 4.0f;	// max square area is (1/invalid_max_area_factor of the screen width)^2
double const invalid_min_area_factor = 15.0f;   // min square area is (1/invalid_min_area_factor of the screen width)^2

double const width_min_sd = 5.0;
double const heigth_min_sd = 5.0;
double const width_sd_threshold_factor = 1.3; // standard_deviation * width_sd_threshold_factor is the maximum variance allowed
double const height_sd_threshold_factor = 1.3; // standard_deviation * height_sd_threshold_factor is the maximum variance allowed

int const hmin = 0;
int const hmax = 180;
int const smin = 60;
int const smax = 256;
int const vmin = 10;
int const vmax = 256;

CubeDetector::Square::Square()
{
	color = cvScalar(0.0);
}

CubeDetector::Square::~Square()
{

}

CubeDetector::CalibrationColor::CalibrationColor()
{
	histogram = cvCreateHist(1, &histogram_bins, CV_HIST_ARRAY, &histogram_ranges, 1);
	color = cvScalar(0.0);

	hsv_min = cvScalar(hmin, smin, vmin);
	hsv_max = cvScalar(hmax, smax, vmax);
}

CubeDetector::CalibrationColor::~CalibrationColor()
{
	if (histogram)
	{
		cvReleaseHist(&histogram);
	}
}


CubeDetector::CubeDetector()
	:storage(0), pyramid(0), hsv(0), mask(0), mask_view(false), active_face(0), history_marker(-1), history_count(-1)
{
	//
	// BGR
	//
	CalibrationColor* calibration = 0;

	calibration = new CalibrationColor();
	calibration->color = CV_RGB(255.0, 0.0, 0.0); // Red = 0
	color_calibration.push_back(calibration);

	calibration = new CalibrationColor();
	calibration->color = CV_RGB(255.0, 255.0, 0.0); // Yellow = 1
	color_calibration.push_back(calibration);

	calibration = new CalibrationColor();
	calibration->color = CV_RGB(255.0, 140.0, 0.0); // Orange = 2
	color_calibration.push_back(calibration);

	calibration = new CalibrationColor();
	calibration->color = CV_RGB(255.0, 255.0, 255.0); // White	= 3
	color_calibration.push_back(calibration);

	calibration = new CalibrationColor();
	calibration->color = CV_RGB(0.0, 0.0, 255.0); // Blue = 4
	color_calibration.push_back(calibration);

	calibration = new CalibrationColor();
	calibration->color = CV_RGB(0.0, 255.0, 0.0); // Green = 5
	color_calibration.push_back(calibration);
}


CubeDetector::~CubeDetector()
{
	for (ColorCalibrations::size_type i = 0; i != color_calibration.size(); ++i)
	{
		delete(color_calibration[i]);
	}

	if (pyramid)
	{
		cvReleaseImage(&pyramid);
		pyramid = 0;
	}

	if (hsv)
	{
		cvReleaseImage(&hsv);
		hsv = 0;
	}

	if (mask)
	{
		cvReleaseImage(&mask);
		mask = 0;
	}


	if (storage)
	{
		cvClearMemStorage(storage);
		cvReleaseMemStorage(&storage);
		storage = 0;
	}
}

void CubeDetector::setMaskView(bool active)
{
	mask_view = active;
}

bool CubeDetector::maskView() const
{
	return mask_view;
}

void CubeDetector::setActiveFace(unsigned int face)
{
	active_face = face;
}

unsigned int CubeDetector::activeFace() const
{
	return active_face;
}

void CubeDetector::setColorCalibrationHistogram(FaceColor const& color, CvHistogram* value)
{
	cvCopyHist(value, &color_calibration[(unsigned int)(color)]->histogram);
}

void CubeDetector::setColorCalibrationHsv(FaceColor const& color, CvScalar const& min, CvScalar const& max)
{
	color_calibration[(unsigned int)(color)]->hsv_min = min;
	color_calibration[(unsigned int)(color)]->hsv_max = max;
}

CubeDetector::CalibrationColor const& CubeDetector::getColorCalibration(FaceColor const& color) const
{
	return *color_calibration[(unsigned int)(color)];
}

void CubeDetector::saveCalibrationElement(CvFileStorage* file_storage, unsigned int const index, std::string const& name)
{
	cvWrite(file_storage, (name + "_Histogram").c_str(), color_calibration[index]->histogram);

	cvStartWriteStruct(file_storage, (name + "_Color").c_str(), CV_NODE_SEQ);
	cvWriteReal(file_storage, 0, color_calibration[index]->color.val[0]);
	cvWriteReal(file_storage, 0, color_calibration[index]->color.val[1]);
	cvWriteReal(file_storage, 0, color_calibration[index]->color.val[2]);
	cvEndWriteStruct(file_storage);

	cvStartWriteStruct(file_storage, (name + "_HSV_Min").c_str(), CV_NODE_SEQ);
	cvWriteReal(file_storage, 0, color_calibration[index]->hsv_min.val[0]);
	cvWriteReal(file_storage, 0, color_calibration[index]->hsv_min.val[1]);
	cvWriteReal(file_storage, 0, color_calibration[index]->hsv_min.val[2]);
	cvEndWriteStruct(file_storage);

	cvStartWriteStruct(file_storage, (name + "_HSV_Max").c_str(), CV_NODE_SEQ);
	cvWriteReal(file_storage, 0, color_calibration[index]->hsv_max.val[0]);
	cvWriteReal(file_storage, 0, color_calibration[index]->hsv_max.val[1]);
	cvWriteReal(file_storage, 0, color_calibration[index]->hsv_max.val[2]);
	cvEndWriteStruct(file_storage);
}

void CubeDetector::loadCalibrationElement(CvFileStorage* file_storage, unsigned int const index, std::string const& name)
{
	CvHistogram* histogram = (CvHistogram*) cvReadByName(file_storage, 0, (name + "_Histogram").c_str());
	setColorCalibrationHistogram(FaceColor(index), histogram);
	cvReleaseHist(&histogram);

	CvSeq* sequence = 0;
	sequence = cvGetFileNodeByName(file_storage, 0, (name + "_Color").c_str())->data.seq;
	color_calibration[index]->color.val[0] = cvReadReal((CvFileNode*)cvGetSeqElem(sequence, 0));
	color_calibration[index]->color.val[1] = cvReadReal((CvFileNode*)cvGetSeqElem(sequence, 1));
	color_calibration[index]->color.val[2] = cvReadReal((CvFileNode*)cvGetSeqElem(sequence, 2));

	sequence = cvGetFileNodeByName(file_storage, 0, (name + "_HSV_Min").c_str())->data.seq;
	color_calibration[index]->hsv_min.val[0] = cvReadReal((CvFileNode*)cvGetSeqElem(sequence, 0));
	color_calibration[index]->hsv_min.val[1] = cvReadReal((CvFileNode*)cvGetSeqElem(sequence, 1));
	color_calibration[index]->hsv_min.val[2] = cvReadReal((CvFileNode*)cvGetSeqElem(sequence, 2));

	sequence = cvGetFileNodeByName(file_storage, 0, (name + "_HSV_Max").c_str())->data.seq;
	color_calibration[index]->hsv_max.val[0] = cvReadReal((CvFileNode*)cvGetSeqElem(sequence, 0));
	color_calibration[index]->hsv_max.val[1] = cvReadReal((CvFileNode*)cvGetSeqElem(sequence, 1));
	color_calibration[index]->hsv_max.val[2] = cvReadReal((CvFileNode*)cvGetSeqElem(sequence, 2));

}

void CubeDetector::saveCalibration(std::string const& file)
{
	CvFileStorage* file_storage = cvOpenFileStorage(file.c_str(), 0, CV_STORAGE_WRITE);

	saveCalibrationElement(file_storage, Red, "Red");
	saveCalibrationElement(file_storage, Yellow, "Yellow");
	saveCalibrationElement(file_storage, Orange, "Orange");
	saveCalibrationElement(file_storage, White, "White");
	saveCalibrationElement(file_storage, Blue, "Blue");
	saveCalibrationElement(file_storage, Green, "Green");

	cvReleaseFileStorage(&file_storage);
}

bool CubeDetector::loadCalibration(std::string const& file)
{
	bool ok = false;

	CvFileStorage* file_storage = cvOpenFileStorage(file.c_str(), 0, CV_STORAGE_READ);

	if (file_storage)
	{
		loadCalibrationElement(file_storage, Red, "Red");
		loadCalibrationElement(file_storage, Yellow, "Yellow");
		loadCalibrationElement(file_storage, Orange, "Orange");
		loadCalibrationElement(file_storage, White, "White");
		loadCalibrationElement(file_storage, Blue, "Blue");
		loadCalibrationElement(file_storage, Green, "Green");

		cvReleaseFileStorage(&file_storage);
		ok = true;
	}

	return ok;
}

template<typename T>
double pointDistance(T const& rhs, T const& lhs)
{
	return sqrt( pow( double(rhs.x) - double(lhs.x), 2.0) +
				 pow( double(rhs.y) - double(lhs.y), 2.0) );
}

double CubeDetector::angle(CvPoint* pt0, CvPoint* pt1, CvPoint* pt2)
{
	double dx1 = pt1->x - pt0->x;
	double dy1 = pt1->y - pt0->y;

	double dx2 = pt2->x - pt0->x;
	double dy2 = pt2->y - pt0->y;

	return (dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
}

void CubeDetector::reduceNoise(IplImage* src)
{
	CvSize size = cvSize(src->width & -2, src->height & -2);

	if (!pyramid)
	{
		pyramid = cvCreateImage(cvSize(size.width / 2, size.height / 2), src->depth, src->nChannels);
	}

	// select the maximum ROI in the image
	// with the width and height divisible by 2
	cvSetImageROI(src, cvRect(0, 0, size.width, size.height));

	cvPyrDown(src, pyramid, IPL_GAUSSIAN_5x5);
	cvPyrUp(pyramid, src, IPL_GAUSSIAN_5x5);
}

bool CubeDetector::isDuplicate(Square const& one, Square const& another)
{
	double distance = sqrt( pow(one.box.center.x - another.box.center.x, 2) +
						    pow(one.box.center.y - another.box.center.y, 2) );

	if (distance < duplicate_center_max_distance)
	{
		return true;
	}

	return false;
}

unsigned int CubeDetector::historyMarker(int offset)
{
	unsigned int marker = history_marker;

	if (offset > 0)
	{
		marker = ( (history_marker + (unsigned int)(offset)) % object_history_size );
	}
	else if (offset < 0)
	{
		while (int(object_history_size) < -offset)
		{
			offset += int(object_history_size);
		}

		marker = ( (history_marker + object_history_size + (unsigned int)(offset)) % object_history_size );
	}

	return marker;
}


unsigned int CubeDetector::nearestCorner(CvPoint const& point, CvPoint const *const corners) const
{
		//
		// Find the nearest corner.. they can be unordered
		//
		double nearest = std::numeric_limits<double>::max();
		unsigned int index = 0;

		for (unsigned int j = 0; j != 4; ++j)
		{
			double distance = pointDistance(point, corners[j]);
			if (distance < nearest)
			{
				nearest = distance;
				index = j;
			}
		}

		return index;
}

bool CubeDetector::similarSquare(Square const& lhs, Square const& rhs) const
{
	if (lhs.square_color != rhs.square_color)
	{
		return false;
	}

	if (pointDistance(lhs.box.center, rhs.box.center) > similar_point_max_distance)
	{
		return false;
	}

	for (unsigned int i = 0; i != 4; ++i)
	{
		//
		// Find the nearest corner.. they can be unordered
		//
		double nearest = std::numeric_limits<double>::max();
		for (unsigned int j = 0; j != 4; ++j)
		{
			double distance = pointDistance(lhs.corner[i], rhs.corner[j]);

			if (distance < nearest)
			{
				nearest = distance;
			}
		}

		if (nearest > similar_point_max_distance)
		{
			return false;
		}
	}


	return true;
}

bool CubeDetector::historyFind(Object const& object, int& index, int& elapsed, Square const& square)
{
	bool found = false;
	index = 0;

	//
	// Test history items
	//
	if (!found)
	{
		for (int i = 0; ((i != int(object_history_size)) && !found); ++i)
		{
			index = int(historyMarker(-i -1));

			if (similarSquare(square, object.history[index]))
			{
				found = true;
				elapsed = i;
			}
		}
	}

	if (!found)
	{
		if (similarSquare(square, object.square))
		{
			found = true;
			elapsed = -1;
			index = -1;
		}
	}

	return found;
}

CubeDetector::Square CubeDetector::mix(Square const& old_square, Square const& new_square) const
{
	float mix_value = 0.2f;

	Square out = new_square;

	for (unsigned int i = 0; i != 4; ++i)
	{
		unsigned int nearest = nearestCorner(new_square.corner[i], old_square.corner);

		out.corner[i].x = int( float(new_square.corner[i].x) * mix_value + float(old_square.corner[nearest].x) * (1.0f - mix_value) );
		out.corner[i].y = int( float(new_square.corner[i].y) * mix_value + float(old_square.corner[nearest].y) * (1.0f - mix_value) );
	}

	out.area = new_square.area * mix_value + old_square.area * (1.0f - mix_value);

	out.box.center.x = new_square.box.center.x * mix_value + old_square.box.center.x  * (1.0f - mix_value);
	out.box.center.y = new_square.box.center.y * mix_value + old_square.box.center.y  * (1.0f - mix_value);

	out.box.size.height = new_square.box.size.height * mix_value + old_square.box.size.height * (1.0f - mix_value);
	out.box.size.width  = new_square.box.size.width  * mix_value + old_square.box.size.width  * (1.0f - mix_value);

	return out;
}

void CubeDetector::historyUpdate(Object& object, int const& index, int const& elapsed, Square const& square)
{
	assert(index >= 0);
	assert((unsigned int)(index) < object_history_size);

	object.square = mix(object.square, square);
	object.history[index] = object.square;
	object.detected[index] = history_count;
	object.last_detection = history_count;
}

void CubeDetector::historyAddNew(Square const& square)
{
	Object object;

	for (unsigned int i = 0; i != object_history_size; ++i)
	{
		object.detected[i] = not_detected;
	}

	object.square = square;
	object.history[history_marker] = object.square;
	object.detected[history_marker] = history_count;
	object.last_detection = history_count;

	objects.push_back(object);
}

void CubeDetector::calculateHistory()
{
	if (history_count < 0) //starting
	{
		history_marker = 0;
		history_count = 0;
	}
	else
	{
		history_marker = historyMarker(1);
		++history_count;
	}

	unsigned int total_found_on_history = 0;
	std::vector<unsigned int> squares_to_add;

	for (unsigned int square_index = 0; square_index != squares.size(); ++square_index)
	{
		bool found = false;
		Objects::iterator object_iterator = objects.begin();
		int object_history_index = -1;
		int object_history_elapsed = -1;

		while (!found && (object_iterator != objects.end()))
		{
			found = historyFind(*object_iterator, object_history_index, object_history_elapsed, squares[square_index]);
			if (!found)
			{
				++object_iterator;
			}
		}

		if (found)
		{
			historyUpdate(*object_iterator, object_history_index, object_history_elapsed, squares[square_index]);
			++total_found_on_history;
		}
		else
		{
			squares_to_add.push_back(square_index);
		}
	}

	//
	// If nothing  was found on history, the cube must have moved
	//
	if (total_found_on_history == 0)
	{
		objects.clear();
	}

	//
	// Add the new squares
	//

	for (std::vector<unsigned int>::size_type i = 0; i != squares_to_add.size(); ++i)
	{
		historyAddNew(squares[ squares_to_add[i] ]);
	}

	//
	// Cleanup
	//
	if (history_count > object_history_size)
	{
		Objects::iterator object_iterator = objects.begin();
		while (object_iterator != objects.end())
		{
			if (history_count > (object_iterator->last_detection + object_history_size))
			{
				object_iterator = objects.erase(object_iterator);
			}
			else
			{
				++object_iterator;
			}
		}
	}
}

void CubeDetector::clearFace()
{
	objects.clear();
	detected_objects.clear();
}

bool CubeDetector::getFace(FaceDetected& face)
{

	float const angle_deadzone = 14.0f;

	if (objects.size() != 9)
	{
		objects.clear();
		return false;
	}

	detected_objects = objects;


	double angle = 0.0f;
	double max_angle = std::numeric_limits<double>::min();
	double min_angle = std::numeric_limits<double>::max();
	//
	// calculate the base rotation angle
	//
	for (Objects::iterator object = detected_objects.begin(); object != detected_objects.end(); ++object)
	{
		angle += object->square.box.angle;

		if (object->square.box.angle > max_angle)
		{
			max_angle = object->square.box.angle;
		}

		if (object->square.box.angle < min_angle)
		{
			min_angle = object->square.box.angle;
		}
	}

	if (fabs(max_angle - min_angle) > angle_deadzone)
	{
		angle = 0.0f;
	}


	//
	// Calculate rotation corner
	//
	CvPoint2D32f point_min = cvPoint2D32f(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	CvPoint2D32f point_max = cvPoint2D32f(0.0f, 0.0f);

	for (Objects::iterator object = detected_objects.begin(); object != detected_objects.end(); ++object)
	{
		for (unsigned int i = 0; i != 4; ++i)
		{
			if (float(object->square.corner[i].x) < point_min.x)
			{
				point_min.x = float(object->square.corner[i].x);
				point_min.y = float(object->square.corner[i].y);
			}
		}
	}


	angle /= 9.0f;
	//std::cout << "Estimated Angle: " << angle << std::endl;
	if (angle > 0.0f)
	{
		angle = 90.0f - angle;

		//
		// Align the squares with the main axis;
		//
		CvMat* input_point = cvCreateMat(3, 1, CV_32FC1);
		CvMat* rotation_matrix = cvCreateMat(2, 3, CV_32FC1);
		CvMat* output_point = cvCreateMat(2, 1, CV_32FC1);

		CvPoint2D32f center = cvPoint2D32f(0.0, 0.0);

		//angle = 0.0;
		cv2DRotationMatrix(point_min, angle, 1.0, rotation_matrix);

		for (Objects::iterator object = detected_objects.begin(); object != detected_objects.end(); ++object)
		{
			//
			// Align the box center
			//
			*((float*)CV_MAT_ELEM_PTR(*input_point, 0, 0)) = object->square.box.center.x;
			*((float*)CV_MAT_ELEM_PTR(*input_point, 1, 0)) = object->square.box.center.y;
			*((float*)CV_MAT_ELEM_PTR(*input_point, 2, 0)) = 1.0f;

			cvMatMul(rotation_matrix, input_point, output_point);

			object->square.box.center.x = *((float*)CV_MAT_ELEM_PTR(*output_point, 0, 0));
			object->square.box.center.y = *((float*)CV_MAT_ELEM_PTR(*output_point, 1, 0));

			//
			// Align the corners
			//
			for (unsigned int i = 0; i != 4; ++i)
			{
				*((float*)CV_MAT_ELEM_PTR(*input_point, 0, 0)) = float(object->square.corner[i].x);
				*((float*)CV_MAT_ELEM_PTR(*input_point, 1, 0)) = float(object->square.corner[i].y);
				*((float*)CV_MAT_ELEM_PTR(*input_point, 2, 0)) = 1.0f;

				cvMatMul(rotation_matrix, input_point, output_point);

				object->square.corner[i].x = int( *((float*)CV_MAT_ELEM_PTR(*output_point, 0, 0)) );
				object->square.corner[i].y = int( *((float*)CV_MAT_ELEM_PTR(*output_point, 1, 0)) );
			}
		}

		cvReleaseMat(&rotation_matrix);
		cvReleaseMat(&input_point);
		cvReleaseMat(&output_point);
	}

	point_min.x = std::numeric_limits<float>::max();
	point_min.y = std::numeric_limits<float>::max();
	point_max.x = std::numeric_limits<float>::min();
	point_max.y = std::numeric_limits<float>::min();

	//
	// Calculate bounding box
	//
	for (Objects::iterator object = detected_objects.begin(); object != detected_objects.end(); ++object)
	{
		for (unsigned int i = 0; i != 4; ++i)
		{
			if (float(object->square.corner[i].x) < point_min.x)
			{
				point_min.x = float(object->square.corner[i].x);
			}

			if (float(object->square.corner[i].y) < point_min.y)
			{
				point_min.y = float(object->square.corner[i].y);
			}

			if (float(object->square.corner[i].x) > point_max.x)
			{
				point_max.x = float(object->square.corner[i].x);
			}

			if (float(object->square.corner[i].y) > point_max.y)
			{
				point_max.y = float(object->square.corner[i].y);
			}
		}
	}

	//
	// Spatialize Square Centers
	//

	unsigned int count[3][3];
	for (int i = 0; i != 3; ++i)
	{
		for (int j = 0; j != 3; ++j)
		{
			count[i][j] = 0;
		}
	}

	float const x_size = (point_max.x - point_min.x) / 3.0f;
	float const y_size = (point_max.y - point_min.y) / 3.0f;

	assert(x_size > 0.0f);
	assert(y_size > 0.0f);

	for (Objects::iterator object = detected_objects.begin(); object != detected_objects.end(); ++object)
	{
		float delta_x = object->square.box.center.x - point_min.x;
		float delta_y = object->square.box.center.y - point_min.y;

		int x_index = int( floor(delta_x / x_size) );
		int y_index = int( floor(delta_y / y_size) );

		assert(x_index >= 0);
		assert(x_index < 3);

		assert(y_index >= 0);
		assert(y_index < 3);

		count[x_index][y_index]++;
		face.square[x_index][y_index] = object->square.square_color;


		//Debug
		object->square.box.center.x -= int(point_min.x);
		object->square.box.center.y -= int(point_min.y);
		for (unsigned int i = 0; i != 4; ++i)
		{
			object->square.corner[i].x -= int(point_min.x);
			object->square.corner[i].y -= int(point_min.y);
		}
	}

	bool objects_valid = true;

	for (int i = 0; i != 3; ++i)
	{
		for (int j = 0; j != 3; ++j)
		{
			if (count[i][j] != 1)
			{
				objects_valid = false;
			}
		}
	}

	objects.clear();

	//
	// Rescale for debug only
	//
	float const scale_factor = 0.35;
	for (Objects::iterator object = detected_objects.begin(); object != detected_objects.end(); ++object)
	{
		for (unsigned int i = 0; i != 4; ++i)
		{
			object->square.corner[i].x = int( float(object->square.corner[i].x) * scale_factor );
			object->square.corner[i].y = int( float(object->square.corner[i].y) * scale_factor );
		}

		object->square.box.center.x = int( float(object->square.box.center.x) * scale_factor );
		object->square.box.center.y = int( float(object->square.box.center.y) * scale_factor );
	}

//	std::cout << "Objects: " << objects.size() << std::endl;
//	std::cout << "Detected Objects: " << detected_objects.size() << std::endl;
//	std::cout << "Valid: " << objects_valid << std::endl;

	return objects_valid;
}

void CubeDetector::filterInvalid()
{
	//
	// This is not thread safe
	//
	unsigned int input_size = squares.size();


	//
	// Remove duplicates
	//
	filtered.clear();
	for (Squares::size_type i = 0; i != squares.size(); ++i)
	{
		bool duplicate_found = false;
		for (Squares::size_type j = i + 1; (j != squares.size()) && !duplicate_found; ++j)
		{
			if (isDuplicate(squares[i], squares[j]) )
			{
				duplicate_found = true;
			}
		}

		if (!duplicate_found)
		{
			filtered.push_back(squares[i]);
		}
	}
	filtered.swap(squares);


	//
	// Basic filter on size
	//
	double max_area = double(hsv->width / invalid_max_area_factor);
	max_area *= max_area;

	double min_area = double(hsv->width / invalid_min_area_factor);
	min_area *= min_area;

	filtered.clear();
	for (Squares::size_type i = 0; i != squares.size(); ++i)
	{
		if (squares[i].area > max_area || squares[i].area < min_area)
		{
			//std::cout << "OVERLIMIT MAX: " << max_area << " MIN: " <<  min_area << " Area: " << squares[i].area << std::endl;
			continue;
		}
		filtered.push_back(squares[i]);
	}
	filtered.swap(squares);



	// statistics
	float width_mean = 0.0;
	float width_standard_deviation = 0;

	float height_mean = 0.0;
	float height_standard_deviation = 0.0;

	float count = 0.0;
	float diference = 0.0;

	for (Squares::size_type i = 0; i != squares.size(); ++i)
	{
		width_mean += squares[i].box.size.width;
		height_mean += squares[i].box.size.height;
		count += 1.0;
	}

	if (count > 0.0)
	{
		width_mean /= count;
		height_mean /= count;

		for (Squares::size_type i = 0; i != squares.size(); ++i)
		{
			diference = squares[i].box.size.width - width_mean;
			width_standard_deviation += diference * diference;

			diference = squares[i].box.size.height - height_mean;
			height_standard_deviation += diference * diference;
		}
		width_standard_deviation = sqrt(width_standard_deviation / count);
		height_standard_deviation = sqrt(height_standard_deviation / count);
	}

	filtered.clear();


	for (Squares::size_type i = 0; i != squares.size(); ++i)
	{
		if (width_standard_deviation > width_min_sd)
		{
			if (fabs(squares[i].box.size.width - width_mean) > width_standard_deviation * width_sd_threshold_factor)
			{
//				std::cout << "Width " << width_mean
//						  << " " << fabs(squares[i].box.size.width - width_mean)
//						  << " " << width_standard_deviation
//						  << " " << squares[i].box.size.width << std::endl;
//				squares[i].color.val[0] = 0;
//				squares[i].color.val[1] = 0;
//				squares[i].color.val[2] = 0;
				continue;
			}
		}

		if (height_standard_deviation > heigth_min_sd)
		{
			if (fabs(squares[i].box.size.height - height_mean) > height_standard_deviation * height_sd_threshold_factor)
			{
//				std::cout << "Height " << height_mean
//						  << " " << fabs(squares[i].box.size.height - height_mean)
//						  << " " << height_standard_deviation
//						  << " " << squares[i].box.size.height << std::endl;
//				squares[i].color.val[0] = 255;
//				squares[i].color.val[1] = 0;
//				squares[i].color.val[2] = 0;
				continue;
			}
		}

		filtered.push_back(squares[i]);
	}
	filtered.swap(squares);




	unsigned int output_size = squares.size();
	if (input_size != output_size)
	{
		//std::cout << "Input: " << input_size << " Output: " << output_size << std::endl;
	}

//	if (squares.size() != 9)
//	{
//		for (Squares::size_type i = 0; i != squares.size(); ++i)
//		{
//			delete(squares[i]);
//		}
//		squares.clear();
//	}
}

// returns squares detected on the image.
CubeDetector::Squares const& CubeDetector::findSquares(IplImage* img)
{
	if (!hsv)
	{
		storage = cvCreateMemStorage(0);
		hsv = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 3);
		mask = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
	}

	cvCvtColor(img, hsv, CV_RGB2HSV);

	squares.clear();
	filtered.clear();


	for (int color = 0; color != int(color_calibration.size()); ++color)
	{
		if (mask_view && (color != int(active_face)))
		{
			continue;
		}

        cvInRangeS(hsv, cvScalar(color_calibration[color]->hsv_min.val[0], color_calibration[color]->hsv_min.val[1], color_calibration[color]->hsv_min.val[2], 0),
        		        cvScalar(color_calibration[color]->hsv_max.val[0], color_calibration[color]->hsv_max.val[1], color_calibration[color]->hsv_max.val[2], 0), mask);


		//cvSmooth(backprojection, backprojection, CV_BLUR, 5);
		reduceNoise(mask);
		cvThreshold(mask, mask, 40, 255, CV_THRESH_BINARY);

		//reduceNoise(backprojection);
        //cvDilate(backprojection, backprojection, 0, 1);

		//IplConvKernel* structuring_element = cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_RECT, NULL);
		//cvMorphologyEx(backprojection, backprojection, NULL, structuring_element, CV_MOP_CLOSE, 1);
		//cvReleaseStructuringElement(&structuring_element);


	    if (mask_view)
		{
			cvCvtColor(mask, img, CV_GRAY2BGR);
		}

		// find contours and store them all as a list
		CvSeq* all_contours;
		cvFindContours(mask, storage, &all_contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
		CvSeq* contours = all_contours;

		while (contours)
		{
			// approximate contour with accuracy proportional to the contour perimeter
			CvSeq* result = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, cvContourPerimeter(contours) * 0.03, 0);

			// square contours should have 4 vertices after approximation, relatively large area (to filter out noisy contours), and be convex.
			// Note: absolute value of an area is used because area may be positive or negative - in accordance with the contour orientation
			if ((result->total == 4) && (fabs(cvContourArea(result, CV_WHOLE_SEQ)) > 1000) && cvCheckContourConvexity(result))
			{
				double max_angle = 0.0f;

				for (unsigned int i = 0; i != 5; ++i)
				{
					// find minimum angle between joint edges (maximum of cosine)
					if (i >= 2)
					{
						double found_angle = angle( (CvPoint*) cvGetSeqElem(result, i - 1),
													(CvPoint*) cvGetSeqElem(result, i - 0),
													(CvPoint*) cvGetSeqElem(result, i - 2) );
						found_angle = fabs(found_angle);
						if (found_angle > max_angle)
						{
							max_angle = found_angle;
						}
					}
				}

				// if cosines of all angles are small (all angles are ~90 degree) then write quandrange vertices to resultant sequence
				if (max_angle < 0.15)
				{
					// Process the new square
					Square square;
					square.corner[0] = *((CvPoint*) cvGetSeqElem(result, 0));
					square.corner[1] = *((CvPoint*) cvGetSeqElem(result, 1));
					square.corner[2] = *((CvPoint*) cvGetSeqElem(result, 2));
					square.corner[3] = *((CvPoint*) cvGetSeqElem(result, 3));

					square.area = fabs(cvContourArea(result, CV_WHOLE_SEQ));
					square.box = cvMinAreaRect2(result, storage);
					square.color = color_calibration[color]->color;
					square.square_color = FaceColor(color);

					squares.push_back(square);
				}
			}
			cvClearSeq(result);

			// take the next contour
			contours = contours->h_next;
		}

		if (all_contours)
		{
			cvClearSeq(all_contours);
		}
		cvClearMemStorage(storage);
	}

	filterInvalid();

	calculateHistory();


	//std::cout << "Objects: " << objects.size() << std::endl;

    return squares;
}

void CubeDetector::draw(IplImage* dst)
{

	CvScalar rgb = cvScalar(0);

	for (Objects::iterator i = objects.begin(); i != objects.end(); ++i)
	{
		int count = 4;
		CvPoint* rect = i->square.corner;

		//cvFillPoly(dst, &rect, &count, 1, (*i)->color);
		rgb.val[0] = i->square.color.val[2];
		rgb.val[1] = i->square.color.val[1];
		rgb.val[2] = i->square.color.val[0];

		cvPolyLine(dst, &rect, &count, 1, 1,  rgb, 3, CV_AA, 0);

		//CvPoint center = cvPoint(int(i->square.box.center.x), int(i->square.box.center.y));
		//cvCircle(dst, center, 3, i->square.color, CV_FILLED);
	}

	for (Objects::iterator i = detected_objects.begin(); i != detected_objects.end(); ++i)
	{
		int count = 4;
		CvPoint* rect = i->square.corner;
		rgb.val[0] = i->square.color.val[2];
		rgb.val[1] = i->square.color.val[1];
		rgb.val[2] = i->square.color.val[0];
		cvPolyLine(dst, &rect, &count, 1, 1,  rgb, 3, CV_AA, 0);

		rgb.val[0] = i->square.color.val[2];
		rgb.val[1] = i->square.color.val[1];
		rgb.val[2] = i->square.color.val[0];
		CvPoint center = cvPoint(int(i->square.box.center.x), int(i->square.box.center.y));
		cvCircle(dst, center, 3, rgb, CV_FILLED);
	}

}

//
// Gui
//

CubeDetectorHook::CubeDetectorHook(rengine::SharedPointer<RubiksCube> rubics_cube) :
	m_rubics_cube(rubics_cube), m_state(Off)
{
	SharedPointer<SystemCommand> command;

	command = new SystemCommand("cubeMirrorStart", MirrorCubeStart, this);
	command->setDescription("Start real cube detection using a video source");
	CoreEngine::instance()->system().registerCommand(command);

	command = new SystemCommand("cubeMirrorStop", MirrorCubeStop, this);
	command->setDescription("Stop real cube detection process");
	CoreEngine::instance()->system().registerCommand(command);


	command = new SystemCommand("cubeMirrorLoadCalibration", MirrorCubeLoadCalibration, this);
	command->setDescription("Load cube mirror calibration file");
	CoreEngine::instance()->system().registerCommand(command);

	command = new SystemCommand("cubeMirrorGrabCurrentFace", MirrorCubeGrabCurrentFace, this);
	command->setDescription("Grabs the current detected face");
	CoreEngine::instance()->system().registerCommand(command);

	m_image = 0;

}

CubeDetectorHook::~CubeDetectorHook()
{
	if (m_image)
	{
		cvReleaseImageHeader(&m_image);
		m_image = 0;
	}
}

void CubeDetectorHook::SetOff()
{
	m_state = Off;
}

CubeDetector::FacesDetected const& CubeDetectorHook::detectedFaces() const
{
	return m_faces_detected;
}

bool CubeDetectorHook::hasDetectedCube() const
{
	return (m_state == Detected);
}

void CubeDetectorHook::update()
{
	if (m_video_capture.get() && m_video_capture->ready())
	{
		try
		{
			VideoCapture::FrameOptions options;
			m_frame = m_video_capture->grab(options);
			unsigned int const expected_size = m_video_capture->captureOptions().width * m_video_capture->captureOptions().height * 3;

			if (m_frame.get() && m_frame->data && (expected_size == m_frame->size))
			{



				cvSetData(m_image, m_frame->data, m_video_capture->captureOptions().width * 3);
				cvFlip(m_image, 0, 1);

				m_cubeDetector.findSquares(m_image);
				m_cubeDetector.draw(m_image);


//				cvCvtColor(m_image, m_image, CV_HSV2RGB);

				// Image object will destroy frame->data
				SharedPointer<Image> image = new Image(m_video_capture->captureOptions().width,
													   m_video_capture->captureOptions().height, 3, m_frame->data);
				//image->flip(Image::FlipVertical);
				m_texture->setImage(image);
			}
		}
		catch (VideoCaptureException caught)
		{
			CoreEngine::instance()->log() << "Unable to grab frame : " << caught << std::endl;
		}


		if (!m_label)
		{
			float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;
			std::string message = "Detected faces: " + lexical_cast<std::string>(m_faces_detected.size());

			Vector3D position(0.0f, height, 0.0f);

			m_label = new HudWriter();
			m_label->setColor(text_color);
			m_label->setText(message);

			Font::Glyph* reference_glyph = m_label->getFont()->referenceGlyph();
			if (reference_glyph)
			{
				position.y() -= reference_glyph->dimension().y();
			}
			m_label->setPosition(position);


			m_label->clear();
			m_label->write(message);
		}
	}
}

void CubeDetectorHook::render(RenderEngine& render_engine)
{
	float const width = (float) CoreEngine::instance()->mainWindow()->contextOptions().width;
	float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;
	render_engine.setViewport(0, 0, width, height);


	Matrix projection = Matrix::ortho2D(0, width, 0, height);
	render_engine.pushDrawStates();

	if (m_video_capture.get() && m_video_capture->ready())
	{
		m_quadrilateral->states()->getProgram()->uniform("mvp").set( Matrix::ortho2D(0, width, 0, height) );
		render_engine.draw( *m_quadrilateral );

		if (m_label.get())
		{
			render_engine.draw( *m_label );
		}
	}

	render_engine.popDrawStates();


}

void CubeDetectorHook::mirrorCubeStart(rengine::SystemCommand::Arguments const& arguments)
{
	m_faces_detected.clear();

	if ((m_state == Off) || (m_state == Detected))
	{
		m_faces_detected.clear();

		bool do_cleanup = false;
		std::string error_message;

		try
		{
			VideoCapture::CaptureOptions options;
			m_video_capture = VideoCapture::create();

			if (true)
			{
				SharedPointer<ThreadedVideoCapture> threaded_video_capture = new ThreadedVideoCapture(m_video_capture);
				m_video_capture = threaded_video_capture;
			}

			m_video_capture->open(options);

			if (m_video_capture->ready())
			{
				float const width = (float) CoreEngine::instance()->mainWindow()->contextOptions().width;
				float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;

				SharedPointer<Image> image = new Image(m_video_capture->captureOptions().width,
													   m_video_capture->captureOptions().height, 3);
				image->zeroImage();

				Vector3D window_size(width, height, 0.0f);
				Vector3D image_size(Real(m_video_capture->captureOptions().width), Real(m_video_capture->captureOptions().height), 0.0f);

				Vector3D corner = (window_size - image_size) / 2.0;

				m_texture = new Texture2D(image);
				m_quadrilateral = new Quadrilateral();
				m_quadrilateral->setCornersVertex(corner, window_size - corner);
				m_quadrilateral->states()->setTexture(m_texture);

				SharedPointer<Program> decal_program = CoreEngine::instance()->resourceManager().load<Program>("data/shaders/core/DecalTexture.eff");

				m_quadrilateral->states()->setCapability(DrawStates::DepthTest, DrawStates::Off);
				m_quadrilateral->states()->setCapability(DrawStates::Blend, DrawStates::Off);
				m_quadrilateral->states()->setCapability(DrawStates::CullFace, DrawStates::Off);
				m_quadrilateral->states()->setProgram(decal_program);

				if (!decal_program)
				{
					do_cleanup = true;
					error_message = "Unable to load decal effect";
				}

				m_image = cvCreateImageHeader(cvSize(m_video_capture->captureOptions().width,
											   m_video_capture->captureOptions().height), IPL_DEPTH_8U, 3);

			}

			if (m_label.get())
			{
				std::string message = "Detected faces: " + lexical_cast<std::string>(m_faces_detected.size());
				m_label->clear();
				m_label->write(message);
			}

			m_state = Detecting;
		}
		catch (VideoCaptureException caught)
		{
			do_cleanup = true;
			error_message = "Unable to start mirror cube : " + lexical_cast<std::string>(caught);
		}

		if (do_cleanup)
		{
			m_video_capture = 0;
			m_quadrilateral = 0;
			m_texture = 0;
			m_state = Off;

			if (m_image)
			{
				cvReleaseImageHeader(&m_image);
				m_image = 0;
			}

			CoreEngine::instance()->log() << error_message << std::endl;
		}

	}
}

void CubeDetectorHook::mirrorCubeStop(rengine::SystemCommand::Arguments const& arguments)
{
	m_cubeDetector.clearFace();

	m_video_capture = 0;
	m_quadrilateral = 0;
	m_texture = 0;
	m_state = Off;

	if (m_image)
	{
		cvReleaseImageHeader(&m_image);
		m_image = 0;
	}
}

void CubeDetectorHook::mirrorCubeLoadCalibration(rengine::SystemCommand::Arguments const& arguments)
{
	std::string syntax_error;
	bool show_syntax = false;


	if ((arguments.size() == 1) && (arguments[0]->type() == Variable::StringType))
	{
		if ( !m_cubeDetector.loadCalibration(arguments[0]->asString()) )
		{
			CoreEngine::instance()->log() << "Unable to load calibration file : " << arguments[0]->asString() << std::endl;
		}
		else
		{
			CoreEngine::instance()->log() << "Loaded calibration file : " << arguments[0]->asString() << std::endl;
		}
	}
	else
	{
		show_syntax = true;
		syntax_error = "Invalid filename";
	}

	if (show_syntax)
	{
		CoreEngine::instance()->system().echo(arguments);
		CoreEngine::instance()->log() << "Bad Syntax : " << syntax_error << std::endl;
		CoreEngine::instance()->log() << "Arguments : <configuration_file.xml>" << std::endl;
	}
}

void CubeDetectorHook::mirrorCubeGrabCurrentFace(rengine::SystemCommand::Arguments const& arguments)
{

	if (m_video_capture.get() && m_video_capture->ready() && (m_state == Detecting))
	{
		CubeDetector::FaceDetected face;

		if (m_cubeDetector.getFace(face))
		{
			m_faces_detected.push_back(face);
		}

		if (m_label.get())
		{
			std::string message = "Detected faces: " + lexical_cast<std::string>(m_faces_detected.size());
			m_label->clear();
			m_label->write(message);
		}


		if (m_faces_detected.size() == 6)
		{
			SystemCommand::Arguments none;
			mirrorCubeStop(none);
			m_state = Detected;
		}

	}
}

void CubeDetectorHook::operator()(SystemCommand::CommandId const command, SystemCommand::Arguments const& arguments)
{
	if (command == MirrorCubeStart)
	{
		mirrorCubeStart(arguments);
	}
	else if (command == MirrorCubeStop)
	{
		mirrorCubeStop(arguments);
	}
	else if (command == MirrorCubeLoadCalibration)
	{
		mirrorCubeLoadCalibration(arguments);
	}
	else if (command == MirrorCubeGrabCurrentFace)
	{
		mirrorCubeGrabCurrentFace(arguments);
	}
}


