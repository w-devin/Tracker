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


Targets::~Targets()
{
	path.clear();
}
