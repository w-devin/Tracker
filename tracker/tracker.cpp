// tracker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>
#include <opencv.hpp>
#include "tracker.h"
using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
	int count = sizeof(videos) / sizeof(videos[0]);
	//windowsInit(count);

	for (int i = 0; i < count; i++)
	{
		captures.push_back(VideoCapture(videos[i]));
		if (!captures.back().isOpened())
		{
			cerr << "Unable to open video file:" << videos[i] << endl;
			exit(EXIT_FAILURE);
		}
		captures.back().set(CAP_PROP_POS_FRAMES, 4100);
		//4100 simple
		//1500 complex
	}

	while(true)
	{
		Mat frame, fgMaskMOG2, gray;
		for (int i = 0; i < count; i++)
		{
			if (!captures[i].read(frame))
			{
				cerr << "no capture or file end" << endl;
				break;
			}

			GaussianBlur(frame, frame, Size(9, 9), 0.8);
			pMOG2->apply(frame, fgMaskMOG2);
			threshold(fgMaskMOG2, gray, 200, 255, THRESH_BINARY);

			vector<vector<Point> > contours;
			vector<Rect> boundRect;
			findConters(gray, contours, boundRect);

			reNew(boundRect, i);
			merge(i);

			for (vector<Targets>::iterator r = T[i].begin(); r != T[i].end(); r++)
			{
				rectangle(frame, Rect(r->x, r->y, r->width, r->height).tl(), Rect(r->x, r->y, r->width, r->height).br(), r->color, 5, 8, 0);
				for(vector<Point>::iterator t = r->path.begin(); t != r->path.end(); t++)
					circle(frame, *t, 2, r->color, -1, 8);
			}

			//get the frame number and write it on the current frame
			stringstream ss;
			rectangle(frame, cv::Point(10, 2), cv::Point(100, 20), cv::Scalar(255, 255, 255), -1);
			ss << captures[i].get(CAP_PROP_POS_FRAMES);
			string frameNumberString = ss.str();
			putText(frame, frameNumberString.c_str(), cv::Point(15, 15), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

			char winName[20];
			sprintf(winName, "Video[%d]", i);
			imshow(winName, frame);
			sprintf(winName, "Gray[%d]", i);
			imshow(winName, gray);
		}

		char c = waitKey(33);
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
	destroyAllWindows();
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
	if (x >= T.size())
	{
		vector<Targets> t;
		T.push_back(t);
	}

	for (vector<Targets>::iterator i = T[x].begin(); i != T[x].end(); i++)
	{
		vector<Rect>::iterator p = rect.end();
		bool updated = false;
		for (vector<Rect>::iterator r = rect.begin(); r != rect.end(); r++)
			if (p == rect.end() || Distance(Point(i->x, i->y), Point(p->x, p->y)) > Distance(Point(i->x, i->y), Point(r->x, r->y)))
				if (Distance(Point(i->x, i->y), Point(r->x, r->y)) < 100)
					p = r, updated = true;

		if (updated)	//update
		{
			i->x = p->x, i->y = p->y;
			i->path.push_back(Point(i->x + 25, i->y + 89));
			i->time = 10;
			rect.erase(p);
		}
		else { //delete
			i->time--;
			if (!i->time)
				T[x].erase(i), i--;
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

void merge(int x)
{
	for (vector<Targets>::iterator i = T[x].begin(); i != T[x].end(); i++)
	{
		for (int r = 0; r < x; r++)
		{
			bool merged = false;
			for (vector<Targets>::iterator j = T[r].begin(); j != T[r].end(); j++)
				if (Distance(Point(i->x, i->y), Point(j->x - offset[r][x].x, j->y - offset[r][x].y)) < 100)
				{
					merged = true;
					i->color = j->color;
					break;
				}
			if (merged) break;
		}
	}
}