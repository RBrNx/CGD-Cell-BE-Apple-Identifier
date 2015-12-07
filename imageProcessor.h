#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <math.h>
#include <stdlib.h> 

class imageProcessor {

private:
	int redBucket;
	int blueBucket;
	int greenBucket;
	
	int lowThresh;
	int highThresh;
	int strongEdge;
	int weakEdge;
public:

									
	
	imageProcessor(void);

	unsigned char * RGBtoGreyscale(unsigned char image[], int imageWidth, int imageHeight, int imageBytes);

	void calculateImageSample(unsigned char image[], int index, int imageBytes, int imageWidth, float outData[], int kernelWidth, int kernelHeight);

	int applyFilter(float imageSample[], float kernel[], int kernelWidth, int kernelHeight);

	unsigned char* padOutImage(unsigned char image[], int imageWidth, int imageHeight, int imageBytes);

	void colourHistogram(unsigned char image[], int imageWidth, int imageHeight, int imageBytes, unsigned char mask[], float redHist[], float greenHist[], float blueHist[]);

	void colourHistogram(unsigned char image[], int imageWidth, int imageHeight, int imageBytes, float redHist[], float greenHist[], float blueHist[]);

	unsigned char * NonMaxSuppress(unsigned char * sobelX, unsigned char * sobelY, unsigned char * combinedSobel, int imageWidth, int imageHeight, int imageBytes);

	unsigned char * doubleThresholding(unsigned char data[], int imageWidth, int imageHeight, int imageBytes);

	unsigned char * hystTracking(unsigned char image[], int imageWidth, int imageHeight, int imageBytes);

	unsigned char * fillFromEdges(unsigned char image[], int imageWidth, int imageHeight, int imageBytes);

	void loadHistogram(char * filename, float redHist[], float greenHist[], float blueHist[]);

	void saveHistogram(char * filename, float redHist[], float greenHist[], float blueHist[]);

	std::string compareHistogram(float redHist[], float greenHist[], float blueHist[], std::string imageArray[]);

	void normaliseColourHistogram(float redHist[], float greenHist[], float blueHist[]);

};