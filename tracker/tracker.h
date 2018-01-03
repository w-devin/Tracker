#pragma once
#include <opencv.hpp>
#include "Targets.h"
using namespace std;
using namespace cv;

string videos[] = {
	"D:\\Backup\\Desktop\\L.avi",
	"D:\\Backup\\Desktop\\R.avi"
	//, "D:\\Backup\\Desktop\\LL.avi"
};

Point offset[2][2] = {
	{Point(0, 0), Point(120, 0)}, 
	{Point(120, 0), Point(0, 0)}
};
/*
offset[i][r]: 第r个视频(0, 0)点在第i个视频中的坐标
*/

Ptr<BackgroundSubtractor> pMOG2 = createBackgroundSubtractorMOG2();
vector<VideoCapture> captures;
vector<vector<Targets> >T;
RNG rng(12345);

void windowsInit(int);
bool cmp(Rect &, Rect &);
bool isMan(Rect &);
void findConters(Mat &, vector<vector<Point> > &, vector<Rect> &);
int Distance(Point a, Point b);
void reNew(vector<Rect> &, int);
void merge(int);