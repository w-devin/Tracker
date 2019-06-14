#include "stdafx.h"
#include "Targets.h"

Targets::Targets(){}

Targets::Targets(Point point, Scalar color)
{
	path.clear();
	this->color = color;
	this->x = point.x, this->y = point.y;
	path.push_back(*new Point(point.x + 25, point.y + 89));
}

Targets::Targets(Targets t, Point offset)
{
	this->x = t.x - offset.x;
	this->y = t.y - offset.y;
	this->time = t.time;
	this->color = t.color;
	this->path = t.path;
}


Targets::~Targets()
{
	path.clear();
}

Targets& Targets::operator=(const Targets &x)
{
	this->x = x.x;
	this->y = x.y;
	this->width = x.width;
	this->height = x.height;
	this->color = x.color;
	this->path = x.path;
	this->time = x.time;
	this->roi = x.roi;
	
	return *this;
}