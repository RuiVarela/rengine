#ifndef __CUBE_DETECTOR_H__
#define __CUBE_DETECTOR_H__

#include <opencv/cv.h>

#include <vector>
#include <list>
#include <string>

#include <rengine/lang/Lang.h>
#include <rengine/system/System.h>
#include <rengine/windowing/Windowing.h>
#include <rengine/capture/VideoCapture.h>
#include <rengine/capture/ThreadedVideoCapture.h>
#include <rengine/geometry/BaseShapes.h>
#include <rengine/text/HudWriter.h>

#include "RubiksCube.h"

class CubeDetector
{
public:
	static int histogram_bins;
	static float histogram_ranges_array[2];
	static float* histogram_ranges;

	static rengine::Uint64 const object_history_size = 15;

	static rengine::Uint64 const not_detected = -2;

	enum FaceColor
	{
		Red		= 0,
		Yellow	= 1,
		Orange	= 2,
		White	= 3,
		Blue	= 4,
		Green	= 5
	};

	struct Square
	{
		Square();
		~Square();

		FaceColor square_color;
		CvScalar color;
		CvBox2D box;
		CvPoint corner[4];
		double area;
	};

	struct Object
	{
		Square square;
		rengine::Uint64 detected[object_history_size];
		Square history[object_history_size];
		rengine::Uint64 last_detection;
	};

	typedef std::vector<Square> Squares;
	typedef std::list<Object> Objects;

	struct CalibrationColor
	{
		CalibrationColor();
		~CalibrationColor();

		CvHistogram* histogram;
		CvScalar color;
		CvScalar hsv_min;
		CvScalar hsv_max;
	};

	typedef std::vector< CalibrationColor* > ColorCalibrations;

	CubeDetector();
	virtual ~CubeDetector();

	void setColorCalibrationHistogram(FaceColor const& color, CvHistogram* value);
	void setColorCalibrationHsv(FaceColor const& color, CvScalar const& min, CvScalar const& max);
	CalibrationColor const& getColorCalibration(FaceColor const& color) const;

	void saveCalibration(std::string const& file);
	bool loadCalibration(std::string const& file);

	// finds a cosine of angle between vectors, from pt0->pt1 and from pt0->pt2
	static double angle(CvPoint* pt0, CvPoint* pt1, CvPoint* pt2);

	// reduces noise by performing a pyrDown and pyrUp
	void reduceNoise(IplImage* src);

	// checks if the inputs squares are duplicates
	bool isDuplicate(Square const& one, Square const& another);


	struct FaceDetected
	{
		FaceColor square[3][3];
	};
	typedef std::vector<FaceDetected> FacesDetected;

	// Gets the current detected face from the objects
	bool getFace(FaceDetected& face);
	void clearFace();


	Squares const& findSquares(IplImage* img);

	//
	// Debug
	//
	void setMaskView(bool active = true);
	bool maskView() const;

	void setActiveFace(unsigned int face);
	unsigned int activeFace() const;

	void draw(IplImage* dst);
private:
	void saveCalibrationElement(CvFileStorage* file_storage, unsigned int const index, std::string const& name);
	void loadCalibrationElement(CvFileStorage* file_storage, unsigned int const index, std::string const& name);

	void filterInvalid();
	void calculateHistory();
	unsigned int historyMarker(int offset);
	bool similarSquare(Square const& lhs, Square const& rhs) const;
	bool historyFind(Object const& object, int& index, int& elapsed, Square const& square);
	void historyUpdate(Object& object, int const& index, int const& elapsed, Square const& square);
	void historyAddNew(Square const& square);
	unsigned int nearestCorner(CvPoint const& point, CvPoint const *const corners) const;
	Square mix(Square const& old_square, Square const& new_square) const;

	Squares filtered;
	Squares squares;

	ColorCalibrations color_calibration;

	CvMemStorage* storage;
	IplImage* pyramid;
	IplImage* hsv;
	IplImage* mask;

	//
	// Debug
	//
	bool mask_view;
	unsigned int active_face;

	Objects objects;
	Objects detected_objects;
	unsigned int history_marker;
	rengine::Uint64 history_count;
};


class CubeDetectorHook : public rengine::SystemCommand::Handler
{
public:
	CubeDetectorHook(rengine::SharedPointer<RubiksCube> rubics_cube);
	virtual ~CubeDetectorHook();

	enum Commands
	{
		MirrorCubeStart,
		MirrorCubeStop,
		MirrorCubeLoadCalibration,
		MirrorCubeGrabCurrentFace
	};

	enum State
	{
		Off,
		Detecting,
		Detected
	};

	virtual void operator()(rengine::SystemCommand::CommandId const command, rengine::SystemCommand::Arguments const& arguments);

	virtual void update();
	virtual void render(rengine::RenderEngine& render_engine);

	CubeDetector::FacesDetected const& detectedFaces() const;
	bool hasDetectedCube() const;
	void SetOff();

private:
	void mirrorCubeStart(rengine::SystemCommand::Arguments const& arguments);
	void mirrorCubeStop(rengine::SystemCommand::Arguments const& arguments);
	void mirrorCubeLoadCalibration(rengine::SystemCommand::Arguments const& arguments);
	void mirrorCubeGrabCurrentFace(rengine::SystemCommand::Arguments const& arguments);

	rengine::SharedPointer<RubiksCube> m_rubics_cube;
	State m_state;
	CubeDetector m_cubeDetector;


	rengine::SharedPointer<rengine::VideoCapture> m_video_capture;
	rengine::SharedPointer<rengine::VideoCapture::Frame> m_frame;
	rengine::SharedPointer<rengine::Quadrilateral> m_quadrilateral;
	rengine::SharedPointer<rengine::Texture2D> m_texture;

	IplImage* m_image;

	CubeDetector::FacesDetected m_faces_detected;

	rengine::SharedPointer<rengine::HudWriter> m_label;
};


#endif //__CUBE_DETECTOR_H__

