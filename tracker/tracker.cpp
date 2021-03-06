// tracker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <cmath>
#include <opencv.hpp>
#include <opencv2/ml/ml.hpp>
#include "tracker.h"
using namespace cv;
using namespace cv::ml;
using namespace std;


int main(int argc, char* argv[])
{
	//Train BP
	Train();
	
	exit(0);

	//windowsInit(count);
	int count = sizeof(videos) / sizeof(videos[0]);

	for (int i = 0; i < count; i++)
	{
		captures.push_back(VideoCapture(videos[i]));
		if (!captures.back().isOpened())
		{
			cerr << "Unable to open video file:" << videos[i] << endl;
			exit(EXIT_FAILURE);
		}
		captures.back().set(CAP_PROP_POS_FRAMES, 2900);
		//4100 两人无接触反向移动
		//1500 两人接触	+ 速度方向
		//2900 两人相反方向移动  + 速度方向
		//5500 两人同向 近距离行走 !!!!!! + 颜色
	}

	vector<Mat> frame, pframe, fgMaskMOG2, gray;
	int iPosFrame = 0;
	char winName[45];

	for (int i = 0; i < count; i++)
	{
		frame.push_back(*(new Mat()));
		pframe.push_back(*(new Mat()));
		fgMaskMOG2.push_back(*(new Mat()));
		gray.push_back(*(new Mat()));
		T.push_back(*(new vector<Targets>));
	}

	while(true)
	{
		for (int i = 0; i < count; i++)
		{
			if (!captures[i].read(frame[i]))
			{
				cerr << "no capture or file end" << endl;
				break;
			}

			//get current frame position
			iPosFrame = captures[i].get(CAP_PROP_POS_FRAMES);

			//show original image
			//sprintf(winName, "Original[%d]", i);
			//imshow(winName, frame[i]);

			//init pframe
			if (!pframe[i].data)
				frame[i].copyTo(pframe[i]);

			//Gaussian background modeling
			Mat cur_frame;
			GaussianBlur(frame[i], cur_frame, Size(9, 9), 0.8);
			pMOG2->apply(cur_frame, fgMaskMOG2[i]);
			threshold(fgMaskMOG2[i], gray[i], 200, 255, THRESH_BINARY);

			vector<vector<Point> > contours;
			vector<Rect> boundRect;
			findConters(gray[i], contours, boundRect);
			//reNew(boundRect, i);

			vector<Point2f> contoursToTrack;
			goodFeaturesToTrack(gray[i], contoursToTrack, 20, 0.01, 1);

			//refresh contours per 10 frame
			{
				for (auto t = T[i].begin(); t != T[i].end(); t++)
					t->contours.clear();

				for (auto contour = contoursToTrack.begin(); contour != contoursToTrack.end(); contour++)
					for (auto t = T[i].begin(); t != T[i].end(); t++)
						if (contour->x >= t->x && contour->y >= t->y && contour->x <= t->x + t->width && contour->y <= t->y + t->height)
							t->contours.push_back(*contour);
			}

			//Optical flow tracking
			for (auto t = T[i].begin(); t != T[i].end(); t++)
			{
				vector<Point2f> new_contour;
				vector<uchar> status;
				vector<float> err;

				Mat cur_gray, pre_gray;
				cvtColor(frame[i], cur_gray, CV_RGB2GRAY);
				cvtColor(pframe[i], pre_gray, CV_RGB2GRAY);

				calcOpticalFlowPyrLK(pre_gray, cur_gray, t->contours, new_contour, status, err);

				//find the new rect contours in
				double rate = 0;
				int num = 0;
				auto new_rect = boundRect.begin();

				for (auto rect = boundRect.begin(); rect != boundRect.end(); rect++)
				{
					for (auto contour = new_contour.begin(); contour != new_contour.end(); contour++)
						if (contour->x >= rect->x && contour->y >= rect->y
							&& contour->x <= rect->x + rect->width && contour->y <= rect->y + rect->height)
							num++;

						if (1.0 * num / new_contour.size() > rate)
							rate = 1.0 * num / new_contour.size();
						new_rect = rect;
				}

				//if rate != 0, means the new rect is found
				if (rate)
				{
					t->time = lifeTime;
					t->x = new_rect->x;
					t->y = new_rect->y;
					t->width = new_rect->width;
					t->height = new_rect->height;
					t->path.push_back(Point(t->x + 25, t->y + 89));

					boundRect.erase(new_rect);
				}
				else --t->time;
			}
			for (auto r = boundRect.begin(); r != boundRect.end(); r++)
				T[i].push_back(Targets(Point(r->x, r->y), Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255))));

			//merge(i);


			//save image for every object
			cout << "object number in cam" << i << ": " << T[i].size() << endl;
			int _ = 0;
			for (vector<Targets>::iterator r = T[i].begin(); r != T[i].end(); r++)
			{
				Rect rect(r->x, r->y, min(r->width, cur_frame.cols - r->x), min(r->height, cur_frame.rows - r->y));

				Mat ROI;  
				cur_frame(rect).copyTo(ROI);

				if (ROI.data)
				{ 
					r->roi.push_back(ROI);

					//若图像存储数量过大，则删除部分
					while (roiCount < r->roi.size())
						r->roi.pop_front();

					char tmp[100];
					sprintf(tmp, "Object[%d]", _++);
					//imshow(tmp, ROI);
				}

				//draw rectangles and contours
				if (r->time > 0)
				{
					rectangle(cur_frame, Rect(r->x, r->y, r->width, r->height).tl(), Rect(r->x, r->y, r->width, r->height).br(), r->color, 5, 8, 0);
					for (auto con = r->contours.begin(); con != r->contours.end(); con++)
						circle(cur_frame, *con, 2, r->color, -1, 8);

					for (auto con = r->path.begin(); con != r->path.end(); con++)
						circle(cur_frame, *con, 2, r->color, -1, 8);

					sprintf(winName, "Feature contours[%d]", i);
					imshow(winName, cur_frame);
				}
				//update pframe
				pframe[i] = frame[i];
			}

			imshow("test", frame[i]);
				

			//get the frame number and write it on the current frame
			stringstream ss;
			rectangle(frame[i], cv::Point(10, 2), cv::Point(100, 20), cv::Scalar(255, 255, 255), -1);
			ss << iPosFrame;
			//ss << captures[i].get(CAP_PROP_POS_FRAMES);
			string frameNumberString = ss.str();
			putText(frame[i], frameNumberString.c_str(), cv::Point(15, 15), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

			sprintf(winName, "Video[%d]", i);
			imshow(winName, cur_frame);
			sprintf(winName, "Gray[%d]", i);
			imshow(winName, gray[i]);
		}

		//if (iPosFrame == 3131)
		//	waitKey(0);

		char c = waitKey(1);
		switch (c)
		{
		case 27:
			return EXIT_SUCCESS;
		case 'p':
			waitKey(0);
			break;
		case ' ':
			break;

		}
	}
	while (!captures.empty()) 
		captures.back().release(), captures.pop_back();
	cv::destroyAllWindows();
	return EXIT_SUCCESS;
}

void windowsInit(int x)
{
	for (int i = 0; i < x; i++)
	{
		char winName[20];
		sprintf(winName, "Video[%d]", i);
		namedWindow(winName, WINDOW_NORMAL);
		sprintf(winName, "Gray[%d]", i);
		namedWindow(winName, WINDOW_NORMAL);
	}
}

int Distance(Point a, Point b)
{
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

bool cmp(Rect &a, Rect &b)
{
	if (a.height == b.height) return a.width >= b.width;
	return a.height > b.height;
}

bool isMan(Rect &x)
{
	int width = 50, height = 100;
	if (abs(x.height - height) <= 25 && abs(x.width - width) <= 50) return true;
	return false;
}

void reNew(vector<Rect> &rect, int x)
{
	//若第一次处理改视频流, 则添加对应的目标数组
	if (x >= T.size())
	{
		vector<Targets> t;
		T.push_back(t);
	}

	for (vector<Targets>::iterator i = T[x].begin(); i != T[x].end(); i++)
	{
		/*
			注释添加太晚(期间相隔一个寒假...), 忘记p的作用了 囧 ( ╯□╰ )
			目前的猜测用途是, 当一个视频流中的目标为空时, 新目标无从匹配， 故可通过这种写法添加

			新的猜测用途为: 确定以哪个新的矩形来更新目标

		*/
		vector<Rect>::iterator p = rect.end();
		bool updated = false;
		for (vector<Rect>::iterator r = rect.begin(); r != rect.end(); r++)
			if (p == rect.end() || Distance(Point(i->x, i->y), Point(p->x, p->y)) > Distance(Point(i->x, i->y), Point(r->x, r->y)))
				if (isOne(*i, Targets(Point(r->x, r->y), Scalar(0, 0, 0)), 0))
					p = r, updated = true;

		if (updated)	//update
		{
			i->x = p->x, i->y = p->y;
			i->path.push_back(Point(i->x + 25, i->y + 89));
			i->time = lifeTime;
			rect.erase(p);
		}
		else { 
			//delete
			i->time--;
			//if (!i->time) T[x].erase(i), i--;
		}
	}

	for (vector<Rect>::iterator r = rect.begin(); r != rect.end(); r++)	//add
		T[x].push_back(Targets(Point(r->x, r->y), Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255))));
}

void findConters(Mat &img, vector<vector<Point> > &cs, vector<Rect> &bRect)
{
	vector<Vec4i> hierarchy;
	findContours(img, cs, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<Rect> boundRect(cs.size());

	for (int i = 0; i < cs.size(); i++)
	{
		approxPolyDP(Mat(cs[i]), cs[i], 3, true);
		boundRect[i] = boundingRect(Mat(cs[i]));
	}

	sort(boundRect.begin(), boundRect.end(), cmp);
	for (vector<Rect>::iterator i = boundRect.begin(); i != boundRect.end(); )
		if (!isMan(*i)) i = boundRect.erase(i);
		else i++;

	bRect = boundRect;
}

//匹配不同视频流中的目标, 若判断为同一目标, 则打上同样标记(标记框采用相同颜色)
void merge(int x)
{
	for (vector<Targets>::iterator i = T[x].begin(); i != T[x].end(); i++)
	{
		for (int r = 0; r < x; r++)
		{
			bool merged = false;
			for (vector<Targets>::iterator j = T[r].begin(); j != T[r].end(); j++)
				//if(isOne(*i, *(new Targets(Point(j->x - offset[r][x].x, j->y - offset[r][x].y), Scalar(0, 0, 0))), 1))
				if(isOne(*i, Targets(*j, offset[r][x]), 1))
				{
					merged = true;
					i->color = j->color;
					break;
				}
			if (merged) break;
		}
	}
}

/*
param = 0: 判断是否为同一视频流中的同一目标
param = 1: 判断是否为不同视频流中的同一目标
*/

/*
bool isOne(Targets i, Targets r, int param)
{
	double sigma = 0;

	//弥补同一视频流中同一目标判断时缺少运动方向信息的评估参数损失
	if (param == 0) sigma += 0.2; 

	//距离
	if (Distance(Point(i.x, i.y), Point(r.x, r.y)) < 100) sigma += 0.3;

	//运动方向
	int len = i.path.size();
	Point A, B;
	if (len >= 2)
		A = Point(i.path[len - 2].x - i.path[len - 1].x, i.path[len - 2].y - i.path[len - 1].y);
	len = r.path.size();
	if (len >= 2)
		B = Point(r.path[len - 2].x - r.path[len - 1].x, r.path[len - 2].y - r.path[len - 1].y);

	double cos_theta = 0.0;
	if (A != Point(0, 0) && B != Point(0, 0))
	{
		cos_theta = (A.x * B.x + A.y * B.y) / (sqrt(A.x * A.x + A.y * A.y) * sqrt(B.x * B.x + B.y * B.y));

		double alpha = 1.0 / sqrt(2);
		if (cos_theta >= alpha) sigma += 0.2;
	}
		
	//颜色信息	

	//xxxxxx

	if (sigma >= 0.5) return true;
	return false;
}
*/
bool isOne(Targets i, Targets r, int param)
{
	double sigma = 0;

	//弥补同一视频流中同一目标判断时缺少运动方向信息的评估参数损失
	if (param == 0) sigma += 0.4;

	//距离
	if (Distance(Point(i.x, i.y), Point(r.x, r.y)) < 100)
		sigma += 0.2 * (100 - Distance(Point(i.x, i.y), Point(r.x, r.y))) / 100;

	//运动方向
	int len = i.path.size();
	Point A, B;
	if (len >= 2)
	A = Point(i.path[len - 2].x - i.path[len - 1].x, i.path[len - 2].y - i.path[len - 1].y);
	len = r.path.size();
	if (len >= 2)
	B = Point(r.path[len - 2].x - r.path[len - 1].x, r.path[len - 2].y - r.path[len - 1].y);

	double cos_theta = 0.0;
	if (A != Point(0, 0) && B != Point(0, 0))
	{
	cos_theta = (A.x * B.x + A.y * B.y) / (sqrt(A.x * A.x + A.y * A.y) * sqrt(B.x * B.x + B.y * B.y));

	double alpha = 1.0 / sqrt(2);
	if (cos_theta >= alpha) sigma += 0.40 * (cos_theta - alpha) / (1.0 - alpha);
	}

	//颜色信息	

	if (sigma >= 0.5) return true;
	return false;
}

void Train()
{
	char filePath[45] = "Training//";

	Mat_<int> data;
	Mat_<float> responses(100 * 50, 2);


	Mat_<int> layerSizes(1, 4);
	layerSizes(0, 0) = 100 * 50;
	layerSizes(0, 1) = 10;
	layerSizes(0, 2) = 10;
	layerSizes(0, 3) = 2;

	Ptr<ANN_MLP> network = ANN_MLP::create();
	network->setLayerSizes(layerSizes);
	network->setActivationFunction(ANN_MLP::SIGMOID_SYM, 0.1, 0.1);
	network->setTrainMethod(ANN_MLP::BACKPROP, 0.1, 0.1);
	Ptr<TrainData> trainData = TrainData::create(data, ROW_SAMPLE, responses);

	network->train(trainData);
	if (network->isTrained())
	{
		network->save("bp.yml");
	}
}