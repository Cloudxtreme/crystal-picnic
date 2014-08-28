#include "crystalpicnic.h"
#include "frame.h"

Bitmap *Frame::get_bitmap()
{
	return bitmap;
}

int Frame::get_delay()
{
	return delay;
}

int Frame::get_width()
{
	return bitmap->get_width();
}

int Frame::get_height()
{
	return bitmap->get_height();
}

Frame::Frame(Bitmap *bitmap, int delay) :
	bitmap(bitmap),
	delay(delay)
{
}

Frame::~Frame()
{
	delete bitmap;
}

