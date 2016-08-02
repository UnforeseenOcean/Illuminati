#include "Illuminati.hpp"

int w, h;
HWND mainWindow;
const LPWSTR windowClass = L"illuminatiWindow";

vector<vector<Point2f>> triangles;
vector<vector<Point2f>> newTriangles;
vector<vector<Point2f>> drawnTriangles;

bool block = false;
bool changed = false;
bool removed = false;

#ifdef _DEBUG
int main(void) {
#else
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#endif
	WNDCLASSEX c;
	c.cbSize = sizeof(WNDCLASSEX);
	c.lpfnWndProc = DefWindowProcW;
	c.lpszClassName = windowClass;
	c.style = 0;
	c.cbClsExtra = 0;
	c.cbWndExtra = 0;
	c.hInstance = GetModuleHandle(NULL);
	c.hIcon = 0;
	c.hCursor = LoadCursor(NULL, IDC_ARROW);
	c.hbrBackground = NULL;
	c.lpszMenuName = NULL;
	c.hIconSm = 0;

	RegisterClassEx(&c);

	w = GetSystemMetrics(SM_CXSCREEN);
	h = GetSystemMetrics(SM_CYSCREEN);

	mainWindow = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, windowClass, L"Illuminati",
		WS_POPUP, 0, 0, w, h, NULL, NULL, GetModuleHandle(NULL), NULL);

	SetLayeredWindowAttributes(mainWindow, RGB(255, 255, 255), 0, LWA_COLORKEY);

	CreateThread(NULL, 0, &findTrianglesThread, NULL, 0, NULL);
	CreateThread(NULL, 0, &redrawThread, NULL, 0, NULL);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

DWORD WINAPI findTrianglesThread(LPVOID parameter) {
	HDC screenDC = GetWindowDC(GetDesktopWindow());

	BITMAPINFO drawBitmapInfo;
	ZeroMemory(&drawBitmapInfo, sizeof(BITMAPINFO));

	drawBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	drawBitmapInfo.bmiHeader.biWidth = w;
	drawBitmapInfo.bmiHeader.biHeight = -h;
	drawBitmapInfo.bmiHeader.biPlanes = 1;
	drawBitmapInfo.bmiHeader.biBitCount = 24;

	size_t rawDataSize = w*h * 3;
	size_t maxChanges = 400;

	PVOID screenshotPixelData;
	HBITMAP screenshotBitmap = CreateDIBSection(screenDC, &drawBitmapInfo, DIB_RGB_COLORS, &screenshotPixelData, NULL, 0);

	HDC screenshotDC = CreateCompatibleDC(screenDC);
	SelectObject(screenshotDC, screenshotBitmap);
	Mat screenshotMat = Mat(h, w, CV_8UC3, screenshotPixelData, 0);

	char *oldScreenshot = new char[rawDataSize];

	for (;;) {
		memcpy(oldScreenshot, screenshotPixelData, rawDataSize);
		takeWindowScreenshot(GetForegroundWindow(), screenshotDC);

		vector<vector<Point2f>> oldTriangles = triangles;
		findTriangles(screenshotMat, triangles);

		if (oldTriangles != triangles) {
			block = true;

			for (int i = 0; i < oldTriangles.size(); i++) {
				if (find(triangles.begin(), triangles.end(), oldTriangles[i]) == triangles.end()) {
					removed = removed || find(drawnTriangles.begin(), drawnTriangles.end(), oldTriangles[i]) != drawnTriangles.end();

					drawnTriangles.erase(remove(drawnTriangles.begin(), drawnTriangles.end(), oldTriangles[i]), drawnTriangles.end());
					newTriangles.erase(remove(newTriangles.begin(), newTriangles.end(), oldTriangles[i]), newTriangles.end());
				}
			}

			if (countChanges(oldScreenshot, (char*)screenshotPixelData, rawDataSize) <= maxChanges) {
				for (int i = 0; i < triangles.size(); i++) {
					if (find(oldTriangles.begin(), oldTriangles.end(), triangles[i]) == oldTriangles.end()) {
						newTriangles.push_back(triangles[i]);
					}
				}
			} else {
				triangles = oldTriangles;
			}

			block = false;
		}

		Sleep(100);
	}

	return 0;
}

DWORD WINAPI redrawThread(LPVOID parameter) {
	HRSRC imageResource = FindResource(GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDB_PNG1), L"PNG");

	char* imageData = (char*)LoadResource(GetModuleHandle(NULL), imageResource);

	vector<char> data;
	data.insert(data.end(), imageData, imageData + SizeofResource(GetModuleHandle(NULL), imageResource));

	Mat illuminatiImage = imdecode(data, IMREAD_COLOR);

	Point2f illuminatiTriangle[] = { Point2f(illuminatiImage.cols / 1.868, 0),
		Point2f(0, illuminatiImage.rows), Point2f(illuminatiImage.cols, illuminatiImage.rows) };
	vector<Point2f> illuminatiTriangleVector(begin(illuminatiTriangle),
		end(illuminatiTriangle));

	HDC windowDC = GetWindowDC(mainWindow);
	ShowWindow(mainWindow, SW_SHOW);

	BitBlt(windowDC, 0, 0, w, h, NULL, 0, 0, WHITENESS);

	BITMAPINFO drawBitmapInfo;
	ZeroMemory(&drawBitmapInfo, sizeof(BITMAPINFO));

	drawBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	drawBitmapInfo.bmiHeader.biWidth = w;
	drawBitmapInfo.bmiHeader.biHeight = -h;
	drawBitmapInfo.bmiHeader.biPlanes = 1;
	drawBitmapInfo.bmiHeader.biBitCount = 24;

	PVOID drawPixelData;
	HBITMAP drawBitmap = CreateDIBSection(windowDC, &drawBitmapInfo, DIB_RGB_COLORS, &drawPixelData, NULL, 0);
	Mat drawMat = Mat(h, w, CV_8UC3, drawPixelData, 0);

	HDC drawDC = CreateCompatibleDC(windowDC);
	SelectObject(drawDC, drawBitmap);

	HDC drawTempDC = CreateCompatibleDC(windowDC);
	SelectObject(drawTempDC, CreateCompatibleBitmap(windowDC, w, h));

	BitBlt(drawTempDC, 0, 0, w, h, NULL, 0, 0, WHITENESS);

	for (;;) {
		int time = GetTickCount();

		while (GetTickCount() - time < 1200) {
			if (!block && removed) {
				vector<vector<Point2f>> drawn = drawnTriangles;

				BitBlt(drawTempDC, 0, 0, w, h, NULL, 0, 0, WHITENESS);

				for (int i = 0; i < drawn.size(); i++) {
					BitBlt(drawDC, 0, 0, w, h, NULL, 0, 0, WHITENESS);

					Mat affineMat = getAffineTransform(illuminatiTriangleVector, drawn[i]);
					warpAffine(illuminatiImage, drawMat, affineMat, drawMat.size(), 1, BORDER_TRANSPARENT);

					TransparentBlt(drawTempDC, 0, 0, w, h, drawDC, 0, 0, w, h, RGB(255, 255, 255));
				}

				BitBlt(windowDC, 0, 0, w, h, drawTempDC, 0, 0, SRCCOPY);
				removed = false;

				if (drawn.size() == 0)
					PlaySound(NULL, NULL, SND_NODEFAULT);
			}

			Sleep(100);
		}

		if (newTriangles.size() > 0) {
			PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(NULL), SND_ASYNC | SND_RESOURCE);

			vector<Point2f> triangle = newTriangles.back();
			drawnTriangles.push_back(triangle);
			newTriangles.pop_back();

			BitBlt(drawDC, 0, 0, w, h, NULL, 0, 0, WHITENESS);

			Mat affineMat = getAffineTransform(illuminatiTriangleVector, triangle);
			warpAffine(illuminatiImage, drawMat, affineMat, drawMat.size(), 1, BORDER_TRANSPARENT);

			TransparentBlt(windowDC, 0, 0, w, h, drawDC, 0, 0, w, h, RGB(255, 255, 255));
		}
	}
}