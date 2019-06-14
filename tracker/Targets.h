#pragma once
#include <opencv.hpp>
using namespace std;
using namespace cv;

const int lifeTime = 50;	//Ŀ����Ϣ��������
const int roiCount = 100;	//Ŀ��ͼ����Ϣ�洢����

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
	deque<Mat> roi;

	//int lifeTime = 100; //Ŀ����Ϣ��������
public:
	Targets();
	Targets(Point, Scalar);
	Targets(Targets, Point);
	~Targets();

	Targets& operator=(const Targets&);
};