/** @file     calib_daniel.hpp
*  @ingroup   group_Calibration
*  @brief     The basic algorithm for calibration.
*  @copyright Multi-view_IpS
*/
#ifndef CHESSBOARD_HPP
#define CHESSBOARD_HPP

#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

#include <opencv2/core/types_c.h>
#include <reconstruction3d_global.h>
#include <common/debug.hpp>

#include "openCV2/features2d.hpp"
#include <vector>
#include <map>

//#define MODIFING

namespace cv {

namespace details{

// #define TELECENTRICCONSTANT 60000

/**
 * @brief Fast point systemic cross detector based on a localized radon transformation
 */
enum TestOp {
	TEST_CUSTOM = 0,
	TEST_EQ = 1,
	TEST_NE = 2,
	TEST_LE = 3,
	TEST_LT = 4,
	TEST_GE = 5,
	TEST_GT = 6,
	CV__LAST_TEST_OP
};

#define CV_CheckType(t, test_expr, msg)  CV__CHECK_CUSTOM_TEST(_, MatType, t, (test_expr), #t, #test_expr, msg)
#define CV_CheckTypeEQ(t1, t2, msg)  CV__CHECK(_, EQ, MatType, t1, t2, #t1, #t2, msg)

class FastX : public Feature2D
{
public:
	struct Parameters
	{
		float strength;       //!< minimal strength of a valid junction in dB
		float resolution;     //!< angle resolution in radians
		int branches;         //!< the number of branches
		int min_scale;        //!< scale level [0..8]
		int max_scale;        //!< scale level [0..8]
		bool filter;          //!< post filter feature map to improve impulse response
		bool super_resolution; //!< up-sample

		Parameters()
		{
			strength = 40;
			resolution = float(CV_PI*0.25);
			branches = 2;
			min_scale = 2;
			max_scale = 5;
			super_resolution = 1;
			filter = true;
		}
	};

public:
	FastX(const Parameters &config = Parameters());
	virtual ~FastX()  override {}

	void reconfigure(const Parameters &para);

	//declaration to be wrapped by rBind
	void detect(InputArray image,std::vector<KeyPoint>& keypoints, InputArray mask=Mat()) override
	{Feature2D::detect(image.getMat(),keypoints,mask.getMat());}

	virtual void detectAndCompute(
		InputArray image, InputArray mask, std::vector<KeyPoint>& keypoints, OutputArray descriptors,
		bool useProvidedKeyPoints = false) override;

	void detectImpl(const Mat& image,
					std::vector<KeyPoint>& keypoints,
					std::vector<Mat> &feature_maps,
					const Mat& mask=Mat())const;

	void detectImpl(const Mat& image,
					std::vector<Mat> &rotated_images,
					std::vector<Mat> &feature_maps,
					const Mat& mask=Mat())const;

	void findKeyPoints(const std::vector<Mat> &feature_map,
					   std::vector<KeyPoint>& keypoints,
					   const Mat& mask = Mat())const;

	std::vector<std::vector<float> > calcAngles(const std::vector<Mat> &rotated_images,
												std::vector<KeyPoint> &keypoints)const;
	// define pure virtual methods
	virtual int descriptorSize()const override{return 0;}
	virtual int descriptorType()const override{return 0;}
	virtual void operator()( InputArray image, InputArray mask, std::vector<KeyPoint>& keypoints, OutputArray descriptors, bool useProvidedKeypoints=false )const
	{
		descriptors.clear();
		std::vector<Mat> feature_maps;
		detectImpl(image.getMat(),keypoints,feature_maps,mask.getMat());
		if(!useProvidedKeypoints)        // suppress compiler warning
			return;
		return;
	}

protected:
	virtual void computeImpl( const Mat& image, std::vector<KeyPoint>& keypoints, Mat& descriptors)const
	{
		descriptors = Mat();
		std::vector<Mat> feature_maps;
		detectImpl(image,keypoints,feature_maps);
	}

private:
	void rotate(float angle,const Mat &img,Size size,Mat &out)const;
	void calcFeatureMap(const Mat &images,Mat& out)const;

private:
	Parameters parameters;
};

/**
 * @brief Ellipse class
 */
class Ellipse
{
public:
	Ellipse();
	Ellipse(const Point2f &center, const Size2f &axes, float angle);
	Ellipse(const Ellipse &other);


	void draw(InputOutputArray img,const Scalar &color = Scalar::all(120))const;
	bool contains(const Point2f &pt)const;
	Point2f getCenter()const;
	const Size2f &getAxes()const;

private:
	Point2f center;
	Size2f axes;
	float angle,cosf,sinf;
};

/**
 * @brief Chessboard corner detector
 *
 * The detectors tries to find all chessboard corners of an imaged
 * chessboard and returns them as an ordered vector of KeyPoints.
 * Thereby, the left top corner has index 0 and the bottom right
 * corner n*m-1.
 */
class Chessboard: public Feature2D
{
public:
	static const int DUMMY_FIELD_SIZE = 100;  // in pixel

	/**
	* @brief Configuration of a chessboard corner detector
	*/
	struct Parameters
	{
		Size chessboard_size; //!< size of the chessboard
		int min_scale;            //!< scale level [0..8]
		int max_scale;            //!< scale level [0..8]
		int max_points;           //!< maximal number of points regarded
		int max_tests;            //!< maximal number of tested hypothesis
		bool super_resolution;    //!< use super resolution for chessboard detection
		bool larger;              //!< indicates if larger boards should be returned

		Parameters()
		{
			chessboard_size = Size(9,6);
			min_scale = 2;
			max_scale = 4;
			super_resolution = true;
			max_points = 400;
			max_tests = 100;
			larger = false;
		}

		Parameters(int scale,int _max_points):
			min_scale(scale),
			max_scale(scale),
			max_points(_max_points)
		{
			chessboard_size = Size(9,6);
		}
	};


	/**
	 * @brief Gets the 3D objects points for the chessboard assuming the
	 * left top corner is located at the origin.
	 *
	 * @param[in] pattern_size Number of rows and cols of the pattern
	 * @param[in] cell_size Size of one cell
	 *
	 * \returns Returns the object points as CV_32FC3
	 */
	static Mat getObjectPoints(const Size &pattern_size,float cell_size);

	/**
	 * @brief Class for searching and storing chessboard corners.
	 *
	 * The search is based on a feature map having strong pixel
	 * values at positions where a chessboard corner is located.
	 *
	 * The board must be rectangular but supports empty cells
	 *
	 */
	class Board
	{
	public:

		// Estimates the position of the next point on a line using cross ratio constrain
		static bool estimatePoint(const Point2f &p0,const Point2f &p1,const Point2f &p2,Point2f &p3);

		// using 1D homography
		static bool estimatePoint(const Point2f &p0,const Point2f &p1,const Point2f &p2,const Point2f &p3, Point2f &p4);

		/**
		* @brief Checks if all points of a row or column have a valid cross ratio constraint
		*
		* cross ratio:
		* d12/d34 = d13/d24
		*
		* point order on the row/column:
		* pt1 --> pt2 --> pt3 --> pt4
		*
		* @param[in] points THe points of the row/column
		*
		*/
		static bool checkRowColumn(const std::vector<Point2f> &points);

		/**
		* @brief Estimates the search area for the next point on the line using cross ratio
		* @ingroup   group_Calibration
		*
		* point order on the line:
		* (p0) --> p1 --> p2 --> p3 --> search area
		*
		* @param[in] p1 First point coordinate
		* @param[in] p2 Second point coordinate
		* @param[in] p3 Third point coordinate
		* @param[in] p Percentage of d34 used for the search area width and height [0..1]
		* @param[out] ellipse The search area
		* @param[in] p0 optional point to improve accuracy
		*
		* @return Returns false if no search area can be calculated
		*/
		static bool estimateSearchArea(const Point2f &p1,const Point2f &p2,const Point2f &p3,float p,
									   Ellipse &ellipse,const Point2f *p0 =nullptr);

		// Estimates the search area for a specific point based on the given homography
		static Ellipse estimateSearchArea(Mat _H,int row, int col,float p,int field_size = DUMMY_FIELD_SIZE);

		//Searches for the maximum in a given search area
		static float findMaxPoint(flann::Index &index,const Mat &data,const Ellipse &ellipse,float white_angle,float black_angle,Point2f &pt);

		/**
		* @brief Searches for the next point using cross ratio constrain
		*
		* @param[in] index flann index
		* @param[in] data extended flann data
		* @param[in] pt1
		* @param[in] pt2
		* @param[in] pt3
		* @param[in] white_angle
		* @param[in] black_angle
		* @param[in] min_response
		* @param[out] point The resulting point
		*
		* @return Returns false if no point could be found
		*
		*/
		static bool findNextPoint(flann::Index &index,const Mat &data,
								  const Point2f &pt1,const Point2f &pt2, const Point2f &pt3,
								  float white_angle,float black_angle,float min_response,Point2f &point);

		/**
		* @brief Creates a new Board object
		*
		*/
		Board(float white_angle=0,float black_angle=0);
		Board(const Size &size, const std::vector<Point2f> &points,float white_angle=0,float black_angle=0);
		Board(const Chessboard::Board &other);
		virtual ~Board();

		Board& operator=(const Chessboard::Board &other);

		// Draws the corners into the given image
		void draw(InputArray m,OutputArray out,InputArray _H=Mat())const;

		/**
		* @brief Estimates the pose of the chessboard
		*
		*/
		bool estimatePose(const Size2f &real_size,InputArray _K,OutputArray rVec,OutputArray tVec)const;

		/**
		* @brief Clears all internal data of the object
		*
		*/
		void clear();

		/**
		* @brief Returns the angle of the black diagonal
		*
		*/
		float getBlackAngle()const;

		/**
		* @brief Returns the angle of the black diagonal
		*
		*/
		float getWhiteAngle()const;

		/**
		* @brief Initializes a 3x3 grid from 9 corner coordinates
		*
		* All points must be ordered:
		* p0 p1 p2
		* p3 p4 p5
		* p6 p7 p8
		*
		* @param[in] points vector of points
		*
		* @return Returns false if the grid could not be initialized
		*/
		bool init(const std::vector<Point2f> points);

		/**
		* @brief Returns true if the board is empty
		*
		*/
		bool isEmpty() const;

		/**
		* @brief Returns all board corners as ordered vector
		*
		* The left top corner has index 0 and the bottom right
		* corner rows*cols-1. All corners which only belong to
		* empty cells are returned as NaN.
		*/
		std::vector<Point2f> getCorners(bool ball=true) const;

		/**
		 * @brief Returns all board corners as ordered vector of KeyPoints
		 *
		 * The left top corner has index 0 and the bottom right
		 * corner rows*cols-1.
		 *
		 * @param[in] ball if set to false only non empty points are returned
		 *
		 */
		std::vector<KeyPoint> getKeyPoints(bool ball=true) const;

		/**
		 * @brief Returns the centers of the chessboard cells
		 *
		 * The left top corner has index 0 and the bottom right
		 * corner (rows-1)*(cols-1)-1.
		 *
		 */
		std::vector<Point2f> getCellCenters() const;

		/**
		 * @brief Estimates the homography between an ideal board
		 * and reality based on the already recovered points
		 *
		 * @param[in] rect selecting a subset of the already recovered points
		 * @param[in] field_size The field size of the ideal board
		 *
		 */
		Mat estimateHomography(Rect rect,int field_size = DUMMY_FIELD_SIZE)const;

		/**
		 * @brief Estimates the homography between an ideal board
		 * and reality based on the already recovered points
		 *
		 * @param[in] field_size The field size of the ideal board
		 *
		 */
		Mat estimateHomography(int field_size = DUMMY_FIELD_SIZE)const;

		/**
		 * @brief Returns the size of the board
		 *
		 */
		Size getSize() const;

		/**
		 * @brief Returns the number of cols
		 *
		 */
		size_t colCount() const;

		/**
		 * @brief Returns the number of rows
		 *
		 */
		size_t rowCount() const;

		/** @brief  Returns the inner contour of the board including only valid corners
		 *  @note   the contour might be non squared if not all points of the board are defined
		 */
		std::vector<Point2f> getContour()const;


		/**
		 * @brief Grows the board in all direction until no more corners are found in the feature map
		 *
		 * @param[in] data CV_32FC1 data of the flann index
		 * @param[in] flann_index flann index
		 *
		 * \returns the number of grows
		 */
		int grow(const Mat &data,flann::Index &flann_index);

		/**
		 * @brief Validates all corners using guided search based on the given homography
		 *
		 * @param[in] data CV_32FC1 data of the flann index
		 * @param[in] flann_index flann index
		 * @param[in] h Homography describing the transformation from ideal board to the real one
		 * @param[in] min_response Min response
		 *
		 * \returns the number of valid corners
		 */
		int validateCorners(const Mat &data,flann::Index &flann_index,const Mat &h,float min_response=0);

		/**
		 * @brief check that no corner is used more than once
		 *
		 * \returns Returns false if a corner is used more than once
		 */
		bool checkUnique()const;

		/**
		  * @brief Returns false if the angles of the contour are smaller than 35°
		  *
		  */
		bool validateContour()const;

		/** @brief  Grows the board to the left by adding one column.
		 *  @note   param[in] map CV_32FC1 feature map
		 *  @returns Returns false if the feature map has no maxima at the requested positions
		 */
		bool growLeft(const Mat &map,flann::Index &flann_index);
		void growLeft();

		/** @brief  Grows the board to the top by adding one row.
		 *  @note   param[in] map CV_32FC1 feature map
		 *  @returns Returns false if the feature map has no maxima at the requested positions
		 */
		bool growTop(const Mat &map,flann::Index &flann_index);
		void growTop();

		/** @brief  Grows the board to the right by adding one column.
		 *  @note   param[in] map CV_32FC1 feature map
		 *  @returns Returns false if the feature map has no maxima at the requested positions
		 */
		bool growRight(const Mat &map,flann::Index &flann_index);
		void growRight();

		/** @brief  Grows the board to the bottom by adding one row.
		 *  @note   param[in] map CV_32FC1 feature map
		 *  @returns Returns false if the feature map has no maxima at the requested positions
		 */
		bool growBottom(const Mat &map,flann::Index &flann_index);
		void growBottom();

		/**
		 * @brief Adds one column on the left side
		 *
		 * @param[in] points The corner coordinates
		 *
		 */
		void addColumnLeft(const std::vector<Point2f> &points);

		/**
		 * @brief Adds one column at the top
		 *
		 * @param[in] points The corner coordinates
		 *
		 */
		void addRowTop(const std::vector<Point2f> &points);

		/**
		 * @brief Adds one column on the right side
		 *
		 * @param[in] points The corner coordinates
		 *
		 */
		void addColumnRight(const std::vector<Point2f> &points);

		/**
		 * @brief Adds one row at the bottom
		 *
		 * @param[in] points The corner coordinates
		 *
		 */
		void addRowBottom(const std::vector<Point2f> &points);

		/**
		 * @brief Rotates the board 90° degrees to the left
		 */
		void rotateLeft();

		/**
		 * @brief Rotates the board 90° degrees to the right
		 */
		void rotateRight();

		/**
		 * @brief Flips the board along its local x(width) coordinate direction
		 */
		void flipVertical();

		/**
		 * @brief Flips the board along its local y(height) coordinate direction
		 */
		void flipHorizontal();

		/**
		 * @brief Flips and rotates the board so that the angle of
		 * either the black or white diagonal is bigger than the x
		 * and y axis of the board and from a right handed
		 * coordinate system
		 */
		void normalizeOrientation(bool bblack=true);

		/**
		 * @brief Exchanges the stored board with the board stored in other
		 */
		void swap(Chessboard::Board &other);

		bool operator==(const Chessboard::Board& other) const {return rows*cols == other.rows*other.cols;}
		bool operator< (const Chessboard::Board& other) const {return rows*cols < other.rows*other.cols;}
		bool operator> (const Chessboard::Board& other) const {return rows*cols > other.rows*other.cols;}
		bool operator>= (const Size& size)const { return rows*cols >= size.width*size.height; }

		/** @brief  Returns a specific corner
		 *  @note   raises runtime_error if row col does not exists
		 */
		Point2f& getCorner(int row,int col);

		/**
		 * @brief Returns true if the cell is empty meaning at least one corner is NaN
		 */
		bool isCellEmpty(int row,int col);

		/**
		 * @brief Returns the mapping from all corners index to only valid corners index
		 */
		std::map<int,int> getMapping()const;

		/**
		 * @brief Estimates rotation of the board around the camera axis
		 */
		double estimateRotZ()const;

		/**
		 * @brief Returns true if the cell is black
		 *
		 */
		bool isCellBlack(int row,int cola)const;

	private:
		// stores one cell
		// in general a cell is initialized by the Board so that:
		// * all corners are always pointing to a valid Point2f
		// * depending on the position left,top,right and bottom might be set to NaN
		// * A cell is empty if at least one corner is NaN
		struct Cell
		{
			Point2f *top_left,*top_right,*bottom_right,*bottom_left; // corners
			Cell *left,*top,*right,*bottom;         // neighboring cells
			bool black;                             // set to true if cell is black
			Cell();
			bool empty()const;                      // indicates if the cell is empty (one of its corners has NaN)
			int getRow()const;
			int getCol()const;
		};

		// corners
		enum CornerIndex
		{
			TOP_LEFT,
			TOP_RIGHT,
			BOTTOM_RIGHT,
			BOTTOM_LEFT
		};

		Cell* getCell(int row,int column); // returns a specific cell
		const Cell* getCell(int row,int column)const; // returns a specific cell
		void drawEllipses(const std::vector<Ellipse> &ellipses);

		// Iterator for iterating over board corners
		class PointIter
		{
		public:
			PointIter(Cell *cell,CornerIndex corner_index);
			PointIter(const PointIter &other);
			void operator=(const PointIter &other);
			bool valid() const;                   // returns if the pointer is pointing to a cell

			bool left(bool check_empty=false);    // moves one corner to the left or returns false
			bool right(bool check_empty=false);   // moves one corner to the right or returns false
			bool bottom(bool check_empty=false);  // moves one corner to the bottom or returns false
			bool top(bool check_empty=false);     // moves one corner to the top or returns false
			bool checkCorner()const;              // returns true if the current corner belongs to at least one
			// none empty cell
			bool isNaN()const;                    // returns true if the current corner is NaN

			const Point2f* operator*() const;  // current corner coordinate
			Point2f* operator*();              // current corner coordinate
			const Point2f* operator->() const; // current corner coordinate
			Point2f* operator->();             // current corner coordinate

			Cell *getCell();                 // current cell
		private:
			CornerIndex corner_index;
			Cell *cell;
		};

		std::vector<Cell*> cells;          // storage for all board cells
		std::vector<Point2f*> corners; // storage for all corners
		Cell *top_left;                    // pointer to the top left corner of the board in its local coordinate system
		int rows;                          // number of row cells
		int cols;                          // number of col cells
		float white_angle,black_angle;
	};
public:

	/**
		 * @brief Creates a chessboard corner detectors
		 *
		 * @param[in] config Configuration used to detect chessboard corners
		 *
		 */
	Chessboard(const Parameters &config = Parameters());
	virtual ~Chessboard() override;
	void reconfigure(const Parameters &config = Parameters());
	Parameters getPara()const;

	/*
	* @brief Detects chessboard corners in the given image.
	*
	* The detectors tries to find all chessboard corners of an imaged
	* chessboard and returns them as an ordered vector of KeyPoints.
	* Thereby, the left top corner has index 0 and the bottom right
	* corner n*m-1.
	*
	* @param[in] image The image
	* @param[out] keyPoints The detected corners as a vector of ordered KeyPoints
	* @param[in] mask Currently not supported
	*
	*/
	void detect(InputArray image,std::vector<KeyPoint>& keypoints, InputArray mask=Mat())override
	{Feature2D::detect(image.getMat(),keypoints,mask.getMat());}

	virtual void detectAndCompute(InputArray image,InputArray mask, std::vector<KeyPoint>& keypoints,OutputArray descriptors,
								  bool useProvidedKeyPoints = false)override;

	/*
	* @brief Detects chessboard corners in the given image.
	*
	* The detectors tries to find all chessboard corners of an imaged
	* chessboard and returns them as an ordered vector of KeyPoints.
	* Thereby, the left top corner has index 0 and the bottom right
	* corner n*m-1.
	*
	* @param[in] image The image
	* @param[out] keyPoints The detected corners as a vector of ordered KeyPoints
	* @param[out] feature_maps The feature map generated by LRJT and used to find the corners
	* @param[in] mask Currently not supported
	*
	*/
	Chessboard::Board detectImpl(const Mat& image,std::vector<Mat> &feature_maps,const Mat& mask)const;

	// define pure virtual methods
	virtual int descriptorSize()const override{return 0;}
	virtual int descriptorType()const override{return 0;}
	virtual void operator()( InputArray image, InputArray mask, std::vector<KeyPoint>& keypoints, OutputArray descriptors, bool useProvidedKeypoints=false )const
	{
		descriptors.clear();
		std::vector<Mat> maps;
		keypoints.clear();
		Board board = detectImpl(image.getMat(),maps,mask.getMat());
		keypoints = board.getKeyPoints();
		if(!useProvidedKeypoints)        // suppress compiler warning
			return;
		return;
	}

protected:
	virtual void computeImpl( const Mat& image, std::vector<KeyPoint>& keypoints, Mat& descriptors)const
	{
		descriptors = Mat();
		std::vector<Mat> maps;
		keypoints.clear();
		Board board = detectImpl(image,maps,Mat());
		keypoints = board.getKeyPoints();
	}

	// indicates why a board could not be initialized for a certain keyPoint
	enum BState
	{
		MISSING_POINTS = 0,       // at least 5 points are needed
		MISSING_PAIRS = 1,        // at least two pairs are needed
		WRONG_PAIR_ANGLE = 2,     // angle between pairs is too small
		WRONG_CONFIGURATION = 3,  // point configuration is wrong and does not belong to a board
		FOUND_BOARD = 4           // board was found
	};

	void findKeyPoints(const Mat& image, std::vector<KeyPoint>& keypoints,std::vector<Mat> &feature_maps,
					   std::vector<std::vector<float> > &angles ,const Mat& mask)const;
	Mat buildData(const std::vector<KeyPoint>& keypoints)const;
	std::vector<KeyPoint> getInitialPoints(flann::Index &flann_index,const Mat &data,const KeyPoint &center,float white_angle,float black_angle, float min_response = 0)const;
	BState generateBoards(flann::Index &flann_index,const Mat &data, const KeyPoint &center,
						  float white_angle,float black_angle,float min_response,const Mat &img,
						  std::vector<Chessboard::Board> &boards)const;

private:
	Parameters parameters; // storing the configuration of the detector
};
}

// The new method in OpenCV 4, and we extracted it from that library.
bool RECONSTRUCTION3D_EXPORT FindChessboardCornersSB(InputArray image_, Size pattern_size, OutputArray corners_, int flags);

namespace MultiCalib {

enum { CALIB_MULTI_USE_INTRINSIC_GUESS = 0x00001,
	   CALIB_MULTI_FIX_ASPECT_RATIO    = 0x00002,
	   CALIB_MULTI_FIX_PRINCIPAL_POINT = 0x00004,
	   CALIB_MULTI_ZERO_TANGENT_DIST   = 0x00008,       //<<4

	   CALIB_MULTI_FIX_FOCAL_LENGTH    = 0x00010,
	   CALIB_MULTI_FIX_K1              = 0x00020,
	   CALIB_MULTI_FIX_K2              = 0x00040,
	   CALIB_MULTI_FIX_K3              = 0x00080,       //<<8

	   CALIB_MULTI_FIX_INTRINSIC       = 0x00100,
	   CALIB_MULTI_SAME_FOCAL_LENGTH   = 0x00200,
	   CALIB_MULTI_ZERO_DISPARITY      = 0x00400,
	   CALIB_MULTI_FIX_K4              = 0x00800,       //<<12

	   CALIB_MULTI_FIX_K5              = 0x01000,
	   CALIB_MULTI_FIX_K6              = 0x02000,
	   CALIB_MULTI_RATIONAL_MODEL      = 0x04000,
	   CALIB_MULTI_THIN_PRISM_MODEL    = 0x08000,       //<<16

	   CALIB_MULTI_FIX_S1_S2_S3_S4     = 0x10000,
	   CALIB_MULTI_FIX_ANGAL_IN_PLANE  = 0x20000,
	   CALIB_MULTI_TILTED_MODEL        = 0x40000,
	   CALIB_MULTI_FIX_TAUX_TAUY       = 0x80000,       //<<20

	   CALIB_MULTI_USE_QR              = 0x100000, //!< use QR instead of SVD decomposition for solving. Faster but potentially less precise
	   CALIB_MULTI_FIX_TANGENT_DIST    = 0x200000,

	   CALIB_MULTI_USE_LU              = (1 << 17), //!< use LU instead of SVD decomposition for solving. much faster but potentially less precise
	   CALIB_MULTI_USE_EXTRINSIC_GUESS = (1 << 22), //!< for stereoCalibrate

	   CALIB_MULTI_FIX_ALL             = 0x1000000,     //<<25
	   CALIB_MULTI_TELECENTRIC_MODEL   = 0x2000000,
	 };

class LevMarq
{
public:
	LevMarq();
	LevMarq(int nparams, int nerrs, CvTermCriteria criteria =
			cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, DBL_EPSILON),
			bool completeSymmFlag = false);
	~LevMarq();
	void init(int nparams, int nerrs, CvTermCriteria criteria =
			cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, DBL_EPSILON),
			  bool completeSymmFlag = false);
	bool update(const Mat*& param, Mat*& J, Mat*& err);
	bool updateAlt(const Mat*& param, Mat*& JtJ, Mat*& JtErr, double*& errNorm);

	void clear();
	void step();
	enum { DONE = 0, STARTED = 1, CALC_J = 2, CHECK_ERR = 3 };

	Ptr<Mat> mask;
	Ptr<Mat> prevParam;
	Ptr<Mat> param;
	Ptr<Mat> J;
	Ptr<Mat> err;
	Ptr<Mat> JtJ;
	Ptr<Mat> JtJN;
	Ptr<Mat> JtErr;
	Ptr<Mat> JtJV;
	Ptr<Mat> JtJW;
	double prevErrNorm, errNorm;
	int lambdaLg10;
	CvTermCriteria criteria;
	int state;
	int iters;
	bool completeSymmFlag;
	int solveMethod;
};

void solvePnP_Telecentric(Mat	&objectPoints, Mat &imagePoints, Mat &cameraMatrix, Mat	&rVec, Mat &tVec, bool hasZ = false);

Mat prepareCameraMatrix(Mat	&cameraMatrix0, int rtype, int flags);
Mat prepareDistCoeff(Mat &srcDistCoeff, int rtype, int outputSize = 14);
// Fx != Fy in optimization and fixed translation a b.
void initIntrinsicParams2D(std::vector<Mat> &objectPoints, std::vector<Mat>  &imagePoints, Mat &cameraMatrix, double aspectRatio);
// Fx = Fy in optimization and fixed translation a b.
void initIntrinsicParams2D_manual(std::vector<Mat> &objectPoints, std::vector<Mat> &imagePoints, Mat &cameraMatrix);
void initIntrinsicParams2D_Cx_loop(
	std::vector<Mat> &objectPoints, std::vector<Mat> &imagePoints, Mat &cameraMatrix, Mat &cameraDist,
	Mat tiledIntrinsicMatrix, Mat tiledRotationMatrix, float focalLengthUm, float pixelSizeUm);
void initIntrinsicParams2D_Cy_loop(
	std::vector<Mat> &objectPoints, std::vector<Mat> &imagePoints, Mat &cameraMatrix, Mat &cameraDist,
	Mat tiledIntrinsicMatrix, Mat tiledRotationMatrix, float focalLengthUm, float pixelSizeUm);
// compute imaging distance by magnification.
void initIntrinsicParams2D(
	std::vector<Mat> &objectPoints, std::vector<Mat> &imagePoints, Mat &cameraMatrix, double focalLength, double pixelSize);
// Fx = Fy and Cx or Cy in optimization and fixed translation Cx or Cx.
void initIntrinsicParams2D_fixCx(std::vector<Mat> &objectPoints, std::vector<Mat> &imagePoints, Mat &cameraMatrix);

void initIntrinsicParams2D_fixCy(std::vector<Mat> &objectPoints, std::vector<Mat> &imagePoints, Mat &cameraMatrix);

void initIntrinsicParams2D_fixCy_test(
	std::vector<Mat> &objectPoints, std::vector<Mat> &imagePoints_1, std::vector<Mat> &imagePoints_2,
	Mat &cameraMatrix_1, Mat &cameraMatrix_2);

void initIntrinsicParams2D_fixFocalLengthCx(std::vector<Mat> &objectPoints, std::vector<Mat> &imagePoints, Mat &cameraMatrix);

void initIntrinsicParams2D_fixFocalLengthCy(
	std::vector<Mat> &objectPoints, std::vector<Mat> &imagePoints, Mat &cameraMatrix, Mat tiledIntrinsicMatrix,
	Mat tiledRotationMatrix, float focalLengthUm, float pixelSizeUm);

void collectCalibrationData(
	std::vector<std::vector<Point3f>> &objectPoints, std::vector<std::vector<std::vector<Point2f>>> &imagePoints,
	int iFixedPoint, Mat &objPtMat, std::vector<Mat> &imgPtMat, Mat &npoints);

void collectCalibrationData(
	std::vector<std::vector<Point3f>> &objectPoints, std::vector<std::vector<std::vector<Point2f>>> &imagePoints,
	Mat &objPtMat, std::vector<Mat> &imgPtMat, Mat &npoints);

void collectCalibrationData(
	std::vector<std::vector<Point3f>> &objectPoints, std::vector<std::vector<std::vector<Point2f>>> &imagePoints,
	std::vector<Mat> &objPtMat, std::vector<std::vector<Mat>> &imgPtMat);

void collectCalibrationData(
	std::vector<std::vector<Point3f>> &objectPoints, std::vector<std::vector<Point2f>> &imagePoints,
	std::vector<Mat> &objPtMat, std::vector<Mat> &imgPtMat);

int dbCmp(const void* _a, const void* _b);

double calibrateCameraImpl_Telecentric(
	std::vector<Mat> &_objectPoints, std::vector<Mat> &_imagePoints, Mat &_cameraMatrix, Mat &_distCoeff,
	float _focalLength_um, float _pixelSize_um, std::vector<Mat> &_rVecs, std::vector<Mat> &_tVecs, int _flags,
	Mat &_perViewErr, CvTermCriteria _termCrit, bool _output_temp = false, float *_angleX = nullptr, float *_angleY = nullptr);

double multiCameraCalibrateImpl(
	std::vector<Mat> &_objectPoints, std::vector<std::vector<Mat> > &_imagePoints, std::vector<Mat> &_cameraMatrix,
	std::vector<Mat> &_distCoeff, std::vector<float> &_focalLengthUm, std::vector<float> &_pixelSizeUm,
	std::vector<Mat> &_firstR, std::vector<Mat> &_firstT, std::vector<int> &_flags, Mat &_perViewErr, CvTermCriteria _termCrit);

/**
* @brief Forward projection for pinhole lens.
* @ingroup   group_Calibration
*
* @param[in] _oPoints			Input point set. N*3 points depth 64F
* @param[in] _rVec				Rotation matrix.
* @param[in] _tVec				Translation matrix.
* @param[in] _cameraMatrix		Intrinsic matrix.
* @param[in] _distCoeff        Distortion matrix.
* @param[out] _iPoints			Output projected points.
* @param[in] aspectRatio        Whether Fx equal to Fy.
* @param[in] _jacobian			The output Jacobian matrix.
*/
void RECONSTRUCTION3D_EXPORT ProjectPoints_Pinhole(
	Mat &_oPoints, Mat &_rVec, Mat &_tVec, Mat &_cameraMatrix, Mat &_distCoeff, Mat &_iPoints, double aspectRatio = 0, Mat *_jacobian=nullptr);
/**
* @brief Forward projection for telecentric lens.
* @ingroup   group_Calibration
*
* @param[in] _oPoints			Input point set. N*3 points depth 64F
* @param[in] _rVec				Rotation matrix.
* @param[in] _tVec				Translation matrix.
* @param[in] _cameraMatrix		Intrinsic matrix.
* @param[in] _distCoeff        Distortion matrix.
* @param[out] _iPoints			Output projected points.
* @param[in] aspectRatio        Whether Fx equal to Fy.
* @param[in] _jacobian			The output Jacobian matrix.
*/
void RECONSTRUCTION3D_EXPORT ProjectPoints_Telecentric(
	Mat &_oPoints, Mat &_rVec, Mat &_tVec, Mat &_cameraMatrix, Mat &_distCoeff,
	Mat &_iPoints, double aspectRatio = 0, Mat *_jacobian=nullptr);
/**
* @brief Overload of forward projection for telecentric lens.
* @ingroup   group_Calibration
*
* @param[in] _oPoints			Input point set. N*3 points depth 64F
* @param[in] _rVec				Rotation matrix.
* @param[in] _tVec				Translation matrix.
* @param[in] _cameraMatrix		Intrinsic matrix.
* @param[in] _distCoeff        Distortion matrix.
* @param[out] _iPoints			Output projected points.
*/
void RECONSTRUCTION3D_EXPORT ProjectPoints_Telecentric(
	InputArray &_oPoints, Mat &_rVec, Mat &_tVec, Mat &_cameraMatrix, Mat &_distCoeff, OutputArray &_iPoints);
/**
* @brief Nonlinear distortion removal for telecentric lens.
* @ingroup   group_Calibration
*
* @param[in] _src				Input point set. 32FC2
* @param[out] _dst				Output undistorted point set.
* @param[in] _cameraMatrix		Intrinsic matrix.
* @param[in] _distCoeff		Distortion matrix.
* @param[in] _RMat				Rotation matrix.
* @param[in] _PMat				The new intrinsic matrix.
*/
void RECONSTRUCTION3D_EXPORT UndistortPoints_Telecentric(
	InputArray _src, OutputArray _dst, InputArray _cameraMatrix, InputArray _distCoeff, InputArray _RMat, InputArray _PMat);
/**
* @brief Nonlinear distortion removal for telecentric lens.
* @ingroup   group_Calibration
*
* @param[in] _src				Input point set. 32FC2
* @param[out] _dst				Output undistorted point set.
* @param[in] _cameraMatrix		Intrinsic matrix.
* @param[in] _distCoeff		Distortion matrix.
* @param[in] _RMat				Rotation matrix.
* @param[in] _PMat				The new intrinsic matrix.
* @param[in] criteria			Parameters for iteration.
*/
void RECONSTRUCTION3D_EXPORT UndistortPoints_Telecentric(
	InputArray _src, OutputArray _dst, InputArray _cameraMatrix, InputArray _distCoeff, InputArray _RMat,
	InputArray _PMat, TermCriteria criteria);

/**
* @brief Calibration method for telecentric lens.
* @ingroup   group_Calibration
*
* @param[in] objectPoints   Object points coordinate in global coordinate system.
* @param[in] imagePoints    Image points coordinate in camera coordinate system.
* @param[out] cameraMatrix  Intrinsic matrix of cameras, you should input initial guess matrix.
* @param[out] distCoeff    Distortion matrix of cameras.
* @param[in] focalLength_um  The default focal length.
* @param[in] pixelSize_um    The default pixel size.
* @param[out] rVecs         Rotation matrix of cameras.
* @param[out] tVecs         Translation matrix of cameras.
* @param[in] flags          Calibration method flag vector.
* @param[in] criteria       Parameters for iteration.
*
* @return Returns the reprojection error.
*/
double RECONSTRUCTION3D_EXPORT CalibrateCamera_Telecentric(
	std::vector<std::vector<Point3f> > &objectPoints, std::vector<std::vector<Point2f> > &imagePoints,
	Mat &cameraMatrix, Mat &distCoeff, float focalLength_um, float pixelSize_um,
	std::vector<Mat> &rVecs, std::vector<Mat> &tVecs, int flags, CvTermCriteria criteria, bool outputTemp = false);
/**
* @brief Calibration method for telecentric lens.
* @ingroup   group_Calibration
*
* @param[in] objectPoints   Object points coordinate in global coordinate system.
* @param[in] imagePoints    Image points coordinate in camera coordinate system.
* @param[out] cameraMatrix  Intrinsic matrix of cameras, you should input initial guess matrix.
* @param[out] distCoeff    Distortion matrix of cameras.
* @param[in] focalLength_um  The default focal length.
* @param[in] pixelSize_um    The default pixel size.
* @param[out] rVecs         Rotation matrix of cameras.
* @param[out] tVecs         Translation matrix of cameras.
* @param[in] flags          Calibration method flag vector.
* @param[out] perViewErrors Reprojection error map for each image.
* @param[in] criteria       Parameters for iteration.
* @param[in] angleX         Pointer of the specified angle in X-axis where the iteration optimizes at.
* @param[in] angleY         Pointer of the specified angle in Y-axis where the iteration optimizes at.
*
* @return Returns the reprojection error.
*/
double RECONSTRUCTION3D_EXPORT CalibrateCamera_Telecentric(
	std::vector<std::vector<Point3f> >  &objectPoints, std::vector<std::vector<Point2f> >  &imagePoints,
	Mat &cameraMatrix, Mat &distCoeff, float focalLength_um, float pixelSize_um,
	std::vector<Mat> &rVecs, std::vector<Mat> &tVecs, int flags, Mat &perViewErrors,
	CvTermCriteria criteria, bool outputTemp = false, float *angleX = nullptr, float *angleY = nullptr);


/**
* @brief Calibration method for multi-view.
* @ingroup   group_Calibration
*
* @param[in] objectPoints   Object points coordinate in global coordinate system.
* @param[in] imagePoints    Image points coordinate in camera coordinate system.
* @param[out] cameraMatrix  Intrinsic matrix of cameras, you should input initial guess matrix.
* @param[out] distCoeff    Distortion matrix of cameras.
* @param[in] focalLengthUm  The default focal length.
* @param[in] pixelSizeUm    The default pixel size.
* @param[out] firstR        Rotation matrix of cameras.
* @param[out] firstT        Translation matrix of cameras.
* @param[in] flags          Calibration method flag vector.
* @param[in] criteria       Parameters for iteration.
*
* @return Returns the reprojection error.
*/
double RECONSTRUCTION3D_EXPORT MultiCameraCalibration(
	std::vector<std::vector<Point3f> > &objectPoints, std::vector<std::vector<std::vector<Point2f> > > &imagePoints,
	std::vector<Mat> &cameraMatrix, std::vector<Mat> &distCoeff, std::vector<float> &focalLengthUm, std::vector<float> &pixelSizeUm,
	std::vector<Mat> &firstR, std::vector<Mat> &firstT, std::vector<int> &flags, CvTermCriteria criteria);
/**
* @brief Calibration method for multi-view.
* @ingroup   group_Calibration
*
* @param[in] objectPoints   Object points coordinate in global coordinate system.
* @param[in] imagePoints    Image points coordinate in camera coordinate system.
* @param[out] cameraMatrix  Intrinsic matrix of cameras, you should input initial guess matrix.
* @param[out] distCoeff    Distortion matrix of cameras.
* @param[in] focalLengthUm  The default focal length.
* @param[in] pixelSizeUm    The default pixel size.
* @param[out] firstR        Rotation matrix of cameras.
* @param[out] firstT        Translation matrix of cameras.
* @param[in]  flags         Calibration method flag vector.
* @param[out] perViewErrors Reprojection error map for each image.
* @param[in]  criteria      Parameters for iteration.
*
* @return Returns the reprojection error.
*/
double RECONSTRUCTION3D_EXPORT MultiCameraCalibration(
	std::vector<std::vector<Point3f> > &objectPoints, std::vector<std::vector<std::vector<Point2f> > >  &imagePoints,
	std::vector<Mat> &cameraMatrix, std::vector<Mat> &distCoeff, std::vector<float> &focalLengthUm, std::vector<float> &pixelSizeUm,
	std::vector<Mat> &firstR, std::vector<Mat> &firstT, std::vector<int> &flags, Mat &perViewErrors, CvTermCriteria criteria);
/**
* @brief Rotation vector to rotation matrix(Rz-Ry-Rx)
* @ingroup   group_Calibration
*
* @param[in] srcRV  Source rotation vector.
* @param[out] rx    Rotation angle (radians not degree) in x axis.
* @param[out] ry    Rotation angle (radians not degree) in y axis.
* @param[out] rz    Rotation angle (radians not degree) in z axis.
*
*/
void RECONSTRUCTION3D_EXPORT RotationVector2RxRyRz(const Mat &srcRV,double &rx,double &ry,double &rz);
/**
* @brief Rotation matrix to Rz-Rx-Ry
* @ingroup   group_Calibration
*
* @param[in] srcRM  Source rotation matrix.
* @param[out] rx    Rotation angle (radians not degree) in x axis.
* @param[out] ry    Rotation angle (radians not degree) in y axis.
* @param[out] rz    Rotation angle (radians not degree) in z axis.
*
*/
void RECONSTRUCTION3D_EXPORT RotationMatrix2RxRyRz(const Mat &srcRM, double &rx, double &ry, double &rz);

void RECONSTRUCTION3D_EXPORT RxRyRz2RotationVector(const double &rx, const double &ry, const double &rz, Mat &dstRM);
void RECONSTRUCTION3D_EXPORT RxRyRz2RotationMatrix(const double &rx, const double &ry, const double &rz, Mat &dstRM);

/**
* @brief Used for computing relative transformation from multiple matrices.
* @ingroup   group_Calibration
*
* @param[in] inputR  Source rotation vectors/matrices.
* @param[in] inputT  Source translation vectors.
* @param[out] outputR    Computed relative rotation vectors
* @param[out] outputT    Computed relative translation vectors
*
*/
void RECONSTRUCTION3D_EXPORT CalculateRelativeTransformForCalib(
	std::vector<cv::Mat> &inputR, std::vector<cv::Mat> &inputT, std::vector<cv::Mat> &outputR, std::vector<cv::Mat> &outputT);		
void RECONSTRUCTION3D_EXPORT CalculateRelativeTransformForMultiCalib(
	std::vector<std::vector<cv::Mat> > &inputR, std::vector<std::vector<cv::Mat> > &inputT,
	std::vector<cv::Mat> &outputR, std::vector<cv::Mat> &outputT);
void RECONSTRUCTION3D_EXPORT CalculateRelativeTransformForMultiCalib_inv(
	std::vector<cv::Mat> &inputR, std::vector<cv::Mat> &inputT, std::vector<std::vector<cv::Mat> > &outputR,
	std::vector<std::vector<cv::Mat> > &outputT, uint32_t camNum);									
/**
* @brief Calculate the rotation matrix between two vector.
* @ingroup   group_Calibration
*
* @param[in] beforeVec  First rotation matrix.
* @param[in] afterVec   Second rotation matrix.
* @param[out] rotationM Rotation matrix between inputted two matrix.
*
*/
void RECONSTRUCTION3D_EXPORT RotationMatrixTwoVector(Point3d beforeVec, Point3d afterVec, Mat &rotationM);
//extracting main area mask
//type CV_8UC1,CV_32FC1 etc.
void RECONSTRUCTION3D_EXPORT ExtractMainArea(Mat &srcImage, Mat &maskImage, int type, int thresh, double backGroundValue, double foreGroundValue);
void RECONSTRUCTION3D_EXPORT GetThetaRhoFromStartEndPoints(Point &startPoint, Point &endPoint, float &theta, float &rho);
void RECONSTRUCTION3D_EXPORT GetUnitVectorUpDownFromStartEndPoints(Point &firstP, Point &endP, Vec2f &v);
void RECONSTRUCTION3D_EXPORT GetCrossPointFromTwoLines(float &theta1, float &rho1, float &theta2, float &rho2, Point2f &p);
//find the perspective transformation from arbitrary quadrilateral to square
//The vertex of square subject to the vertex of the image
void RECONSTRUCTION3D_EXPORT FindPerspective_Quadrilateral2Square(Mat &mask,Mat &perTransform);

extern dlp::Debug multiCalib_debug_;

} //MultiCalib

}  // end namespace details and cv

#endif


#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif