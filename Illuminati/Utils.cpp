#include "Illuminati.hpp"

void findTriangles(Mat image, vector<vector<Point2f>> &triangles) {
	triangles.clear();

	Mat processingMat;
	bilateralFilter(image, processingMat, 2, 100, 2);

	vector<Mat> channels;
	split(processingMat, channels);
	
	Mat grayMat = Mat::zeros(image.rows, image.cols, CV_8UC1);

	for (int i = 0; i < 3; i++) {
		Mat cannyMat = Mat::zeros(image.rows, image.cols, CV_8UC1);
		Canny(channels[i], cannyMat, 230, 250);
		grayMat |= cannyMat;
	}
	
	vector<vector<Point>> contours;
	findContours(grayMat, contours, RETR_TREE, CHAIN_APPROX_TC89_KCOS);

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

			oldPoly |= polyMat;
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

void takeWindowScreenshot(HWND window, HDC out) {
	RECT rekt;
	GetWindowRect(window, &rekt);

	HDC windowScreenshotDC = CreateCompatibleDC(out);
	HBITMAP windowBitmap = CreateCompatibleBitmap(out, rekt.right - rekt.left, rekt.bottom - rekt.top);
	SelectObject(windowScreenshotDC, windowBitmap);

	PrintWindow(window, windowScreenshotDC, 0);

	BitBlt(out, 0, 0, w, h, NULL, 0, 0, WHITENESS);
	BitBlt(out, rekt.left, rekt.top, rekt.right - rekt.left, rekt.bottom - rekt.top, windowScreenshotDC, 0, 0, SRCCOPY);

	DeleteDC(windowScreenshotDC);
	DeleteObject(windowBitmap);
}