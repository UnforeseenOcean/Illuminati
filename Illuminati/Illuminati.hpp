#include <Windows.h>
#include <magnification.h>

#include <vector>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "resource.h"

using namespace cv;
using namespace std;

extern int w, h;
extern HWND mainWindow;

#ifdef _DEBUG
int main(void);
#else
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
#endif

DWORD WINAPI findTrianglesThread(LPVOID parameter);
DWORD WINAPI redrawThread(LPVOID parameter);

void findTriangles(Mat image, vector<vector<Point2f>> &triangles);
unsigned int countChanges(char* x, char* y, size_t size);
void takeWindowScreenshot(HWND window, HDC out);