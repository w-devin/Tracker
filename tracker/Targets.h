#pragma once
#include <opencv.hpp>
using namespace std;
using namespace cv;

class Targets
{
public:
	int x;
	int y;
	int width = 50;
	int height = 100;
	int time = 10;
	Scalar color;
	vector<Point> path;
public:
	Targets();
	Targets(Point, Scalar);
	~Targets();
};