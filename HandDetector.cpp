#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/background_segm.hpp"
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

enum HDChannels
{
	HD_ChY,
	HD_ChCb,
	HD_ChCr
};
enum Thresholds
{
	TSH_LOW,
	TSH_HIGH
};

Mat currVideoFrame;
Mat foregroundMask;

// Thresholding values, calibraeted values are stored here
uchar currentThershold[2][3] = {{112, 132, 114}, {211, 156, 133}};


/**
 * Makes thresholding on each YCC channel
 * @param {Mat} imageYCrCb  YCC image
 */
Mat thresholdSkinColor(Mat imageYCrCb, uchar threshold[2][3]) {
	Mat dstImage;
	vector<Mat> imageYCrCb3;
	split(imageYCrCb, imageYCrCb3);

	// Does computation for each channel separately to control threshold individually.
	for (int chIdx = 0; chIdx < imageYCrCb.channels(); chIdx++) {
		Mat lookUpTable(1, 256, CV_8U); // Matrix [1,256] initializer
		// Be sure that "pointer" knows type he`s pointing to :) yeah c++ stuff
		uchar* pointer = lookUpTable.data;
		for (int j = 0; j < 256; ++j) {
			pointer[j] = j <= threshold[TSH_LOW][chIdx] || j >= threshold[TSH_HIGH][chIdx] ?
						 (uchar) 0 : (uchar) j;
		}
		// LUT juggles chars from lookup table into matrix very efficiently  (based on Intels TBB)
		LUT(imageYCrCb3[chIdx], lookUpTable, imageYCrCb3[chIdx]);
	}
	merge(imageYCrCb3, dstImage);
	return dstImage;
}

/**
 * On mouse click on pixel calibration
 */
void handleMouseEvent(int evt, int x, int y, int flags, void* param) {
	if (evt == CV_EVENT_LBUTTONDOWN) {
		printf("Sample at (%d,%d): ", x, y);

		// Get pixel sample at this point
		Vec3b sample = currVideoFrame.at<Vec3b>(Point(x, y));
		Scalar pix = Scalar(sample.val[0], sample.val[1], sample.val[2]);
		// and a tiny matrix to be converted to CYY
		Mat matSample(1, 1, CV_8UC3, pix);
		cvtColor(matSample, matSample, CV_YCrCb2BGR);

		std::cout <<  "Original YCC " << matSample;
		Mat result = thresholdSkinColor(currVideoFrame, currentThershold);

		sample = result.at<Vec3b>(Point(x, y));
		pix = Scalar(sample.val[0], sample.val[1], sample.val[2]);

		std::cout <<  " Segmented YCC: " << pix << endl;

		cvtColor(result, result, CV_YCrCb2BGR);
		imshow("Calibrated YCC threshold", result);
	}
}



/**
 * @function handleTrackbarY
 * @brief Callback for trackbar
 */
void onTrackbar(int value, void *b/* pointer to any type */) {
	Mat dstImage, result;
	vector<int> *tshPtr = static_cast<vector<int>*>(b);
	vector<int> tshLoc = *tshPtr;

	std::cout << " Lo/Hi " << tshLoc[0] << " CH " << tshLoc[1] << endl;
	std::cout << " new TSH: " << value << " old TSH: " <<
		(int) currentThershold[tshLoc[0]][tshLoc[1]];
	currentThershold[tshLoc[0]][tshLoc[1]] = (uchar) value;

	// Do the thresholding on new parameters
	result = thresholdSkinColor(currVideoFrame, currentThershold);
	cvtColor(result, result, CV_YCrCb2BGR);

	// Applies foreground mask by skipping "0" pixels at copy time
	result.copyTo(dstImage, foregroundMask);
	imshow("Final image", dstImage);
}



// TODO: binariziraj sliko za vsak kanal posebaj
// TODO: implementiraj noise removal: opening = morphologyEx(img, cv2.MORPH_OPEN, kernel)
// TODO: nato sestavi vse kanale skupaj
// TODO: vzposobi kamero da ne bo dropala slik

int main(int argc, char **argv) {
	Mat imgBgOnlyYCC;
	const char *strOriginalFrame = "Original picture for calibration";

    // Load the source image
	Mat imgBgOnly = imread(argv[1], 1);
	vector<Mat> imgBgMeCombined {imread(argv[2], 1), imread(argv[3], 1), imread(argv[4], 1)};

	namedWindow(strOriginalFrame, 1);
	imshow(strOriginalFrame, imgBgMeCombined.at(0));

	// Convert to YCC with channels separated
	cvtColor(imgBgOnly, imgBgOnlyYCC, CV_BGR2YCrCb);
	cvtColor(imgBgMeCombined.at(0), imgBgMeCombined.at(0), CV_BGR2YCrCb);
	// Share complete YCC frame with other code
	currVideoFrame = imgBgMeCombined.at(0);

	// Background removal. Current set for quick learning from two static frames.
	Ptr<BackgroundSubtractorMOG2> bgSubtractor = createBackgroundSubtractorMOG2(1, 75, false);
	bgSubtractor->apply(imgBgOnlyYCC, foregroundMask);
	bgSubtractor->apply(imgBgMeCombined.at(0), foregroundMask);

//	Mat imageSkinColored = thresholdSkinColor(s);
	imshow("Foreground mask", foregroundMask);

	// cvSetMouseCallback(strOriginalFrame, handleMouseEvent, 0);


	int val0 = (int) currentThershold[TSH_LOW][HD_ChY];
	vector<int> chDsc1 = {TSH_LOW, HD_ChY};
	createTrackbar("LowY", strOriginalFrame, &val0, 255, onTrackbar, &chDsc1);

	int val1 = (int) currentThershold[TSH_HIGH][HD_ChY];
	vector<int> chDsc2 = {TSH_HIGH, HD_ChY};
	createTrackbar("HightY", strOriginalFrame, &val1, 255, onTrackbar, &chDsc2);

	int val2 = (int) currentThershold[TSH_LOW][HD_ChCb];
	vector<int> chDsc3 = {TSH_LOW, HD_ChCb};
	createTrackbar("LowCb", strOriginalFrame, &val2, 255, onTrackbar, &chDsc3);

	int val3 = (int) currentThershold[TSH_HIGH][HD_ChCb];
	vector<int> chDsc4 = {TSH_HIGH, HD_ChCb};
	createTrackbar("HighCb", strOriginalFrame, &val3, 255, onTrackbar, &chDsc4);

	int val4 = (int) currentThershold[TSH_LOW][HD_ChCr];
	vector<int> chDsc5 = {TSH_LOW, HD_ChCr};
	createTrackbar("LowCr", strOriginalFrame, &val4, 255, onTrackbar, &chDsc5);

	int val5 = (int) currentThershold[TSH_HIGH][HD_ChCr];
	vector<int> chDsc6 = {TSH_HIGH, HD_ChCr};
	createTrackbar("HighCr", strOriginalFrame, &val5, 255, onTrackbar, &chDsc6);


//    VideoCapture cap(0); // open the default camera
//    if (!cap.isOpened())  // check if we succeeded
//        return -1;
//
//    for (;;) {
//        cap >> currVideoFrame;
//		outImage = thresholdSkinColor(currVideoFrame, currentThershold);
//		cvtColor(outImage, outImage, CV_YCrCb2BGR);
//		imshow(strOriginalFrame, outImage);
//
//        if (waitKey() >= 0) break;
//    }

    // the camera will be deinitialized automatically in VideoCapture destructor

	waitKey();
	destroyAllWindows();


    return 0;
}



