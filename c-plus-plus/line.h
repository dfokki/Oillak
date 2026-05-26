#pragma once
class line
{
public:
	float x1;
	float x2;
	float y1;
	float y2;
	float k;
	float length;
	line(float x1, float y1, float x2, float y2);
	line(float k, float x, float y);
};

