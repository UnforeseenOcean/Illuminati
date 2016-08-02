#include "Illuminati.hpp"

void findTriangles(Mat image, vector<vector<Point2f>> &triangles) {
	triangles.clear();

	Mat processingMat;
	image.copyTo(processingMat);

	blur(processingMat, processingMat, Size(2, 2));

	vector<Mat> channels;
	split(processingMat, channels);
	
	Mat grayMat = Mat::zeros(image.rows, image.cols, CV_8UC1);

	for (int i = 0; i < channels.size(); i++) {
		Mat cannyMat = Mat::zeros(image.rows, image.cols, CV_8UC1);
		Canny(channels[i], cannyMat, 100, 200);
		grayMat |= cannyMat;
	}

	vector<vector<Point>> contours;
	findContours(grayMat, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

	Mat oldPoly = Mat::zeros(image.rows, image.cols, CV_8UC1);

	for (int i = 0; i < contours.size(); i++) {
		vector<Point> poly;
		approxPolyDP(contours[i], poly, 0.2*arcLength(contours[i], true), true);

		if (poly.size() == 3 && contourArea(poly) > 1200) {
			Mat polyMat = Mat::zeros(image.rows, image.cols, CV_8UC1);
			fillConvexPoly(polyMat, poly, Scalar(255));
			Mat and = polyMat & oldPoly;

			if (countNonZero(and) == 0) {
				vector<Point2f> floatPoly;
				Mat(poly).copyTo(floatPoly);

				triangles.push_back(floatPoly);
			}

			oldPoly = polyMat;
		}
	}
}

unsigned int countChanges(char* x, char* y, size_t size) {
	unsigned int changes = 0;

	for (int i = 0; i < size; i++) {
		if (x[i] != y[i]) {
			changes++;
		}
	}

	return changes;
}