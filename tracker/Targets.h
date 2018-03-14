#pragma once
#include <opencv.hpp>
using namespace std;
using namespace cv;

const int lifeTime = 50;	//目标信息保留周期

class Targets
{
public:
	int x;
	int y;
	int width = 50;
	int height = 100;
	int time = lifeTime;
	Scalar color;
	vector<Point> path;
	//int lifeTime = 100; //目标信息保留周期
public:
	Targets();
	Targets(Point, Scalar);
	Targets(Targets, Point);
	~Targets();

	Targets& operator=(const Targets&);
};