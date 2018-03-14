#pragma once
#include <opencv.hpp>
#include "Targets.h"
using namespace std;
using namespace cv;

//视频流路径
string videos[] = {
	"D:\\Backup\\Desktop\\L.avi"
	,"D:\\Backup\\Desktop\\R.avi"
	//, "D:\\Backup\\Desktop\\test.mp4"
};

/*
映射矩阵
offset[i][r]: 第r个视频(0, 0)点在第i个视频中的坐标
*/
Point offset[2][2] = {
	{Point(0, 0), Point(120, 0)}, 
	{Point(120, 0), Point(0, 0)}
};

//各种同一目标判断依据的权值
const double theta[] = {
	0.30,	//位置
	0.2		//运动方向
};

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
bool isOne(Targets , Targets , int);