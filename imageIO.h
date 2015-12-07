#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#include "lodepng.h"
#include <iostream>
#include <math.h>
#include <algorithm>

class imageIO {

private:
	unsigned int imageWidth, imageHeight, size, imageBytes;

public:
	int loadPNG(const char * filename, std::vector<unsigned char>& imageBuffer);
	int savePNG(const char * filename, std::vector<unsigned char>& imageBuffer);
	
	unsigned char * readBMP(const char * filename);
	void saveBMP(const char * filename, int WIDTH, int HEIGHT, unsigned char dataArray[]);
	
	unsigned char * openImage(const char * filename);
	void saveImage(const char * filename, unsigned char dataArray[], int size);
	
	unsigned int getImageBytes();
	unsigned int getImageWidth();
	unsigned int getImageHeight();
	unsigned int getSize();

	void setImageBytes(int x);
	void setImageWidth(int width);
	void setImageHeight(int height);
	
	unsigned int getIntLE(const unsigned char *p);
	
	int isPowerOfTwo (unsigned int x);
	 
	unsigned char * squareImage(unsigned char origImage[], int imageWidth, int imageHeight, int imageBytes);

};
