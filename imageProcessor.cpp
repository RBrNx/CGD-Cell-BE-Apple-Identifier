#include "imageProcessor.h"

imageProcessor::imageProcessor(void){
	lowThresh = 70;
	highThresh = 150;
	strongEdge = 255;
	weakEdge = 100;
	
	float sobelFilterX[9] = {	-1, 0, 1,
								-2, 0, 2,
								-1, 0, 1	};

	float sobelFilterY[9] = {	-1, -2, -1,
								0, 0, 0,
								1, 2, 1		};

	float gaussianFilter[25] = {	1, 4, 7, 4, 1,
									4, 16, 26, 16, 4,
									7, 26, 41, 26, 7,
									4, 16, 26, 16, 4,
									1, 4, 7, 4, 1		};
}

unsigned char* imageProcessor::RGBtoGreyscale(unsigned char image[], int imageWidth, int imageHeight, int imageBytes) {
	printf("%s \n", "Converting RGB to Greyscale");

	int greySize = imageWidth * imageHeight;
	unsigned char* greyscaleData = new unsigned char[greySize];

	for (int y = 0; y < imageHeight; y++) {
		for (int x = 0; x < imageWidth; x++) {

			int imageIndex = ((y * imageWidth * imageBytes) + (x * imageBytes));
			int greyIndex = ((y * imageWidth) + (x));
			float greyscale = 0;
			greyscale += image[imageIndex] * 0.21;
			greyscale += image[imageIndex + 1] * 0.71;
			greyscale += image[imageIndex + 2] * 0.07;
			greyscaleData[greyIndex] = greyscale;
		}
	}

	return greyscaleData;
}

void imageProcessor::calculateImageSample(unsigned char image[], int index, int imageBytes, int imageWidth, float outData[], int kernelWidth, int kernelHeight) {

	for (int x = 0; x < kernelWidth; x++) {
		for (int y = 0; y < kernelHeight; y++) {
			int sampleIndex = x * kernelHeight + y;

			int offsetX = x - (kernelWidth / 2);
			int offsetY = y - (kernelHeight / 2);
			int imageIndex = index + (offsetY*imageWidth*imageBytes + (offsetX*imageBytes));
			outData[sampleIndex] = image[imageIndex];
		}
	}
}

int imageProcessor::applyFilter(float imageSample[], float kernel[], int kernelWidth, int kernelHeight) {

	int filteredValue = 0;

	for (int x = 0; x < kernelWidth; x++) {
		for (int y = 0; y < kernelHeight; y++) {

			int index = x * kernelHeight + y;
			filteredValue += imageSample[index] * kernel[index];
		}
	}

	return filteredValue;

}

unsigned char* imageProcessor::padOutImage(unsigned char image[], int imageWidth, int imageHeight, int imageBytes) {
	printf("%s \n", "Coverting image back to original size");
	int paddedSize = imageWidth * imageHeight * imageBytes;
	unsigned char* paddedImage = new unsigned char[paddedSize];

	for (int y = 0; y < imageHeight; y++) {
		for (int x = 0; x < imageWidth; x++) {
			int index = (y * imageWidth + x);
			int paddedIndex = ((y * imageWidth * imageBytes) + (x * imageBytes));

			paddedImage[paddedIndex] = image[index];
			paddedImage[paddedIndex + 1] = image[index];
			paddedImage[paddedIndex + 2] = image[index];
			paddedImage[paddedIndex + 3] = 255;
		}
	}
	
	printf("%s \n", "Padding edges of image");
	for (int x = 0; x < imageWidth; x += 4) {
		paddedImage[x] = 0;
		paddedImage[x + 1] = 0;
		paddedImage[x + 2] = 0;
		if (imageBytes == 4) paddedImage[x + 3] = 255;

		paddedImage[(imageHeight - 1) + x] = 0;
		paddedImage[(imageHeight - 1) + x + 1] = 0;
		paddedImage[(imageHeight - 1) + x + 2] = 0;
		if (imageBytes == 4) paddedImage[(imageHeight - 1) + x + 3] = 255;
	}

	for (int y = 0; y < imageHeight; y++) {
		paddedImage[0 + y * (imageWidth * imageBytes)] = 0;
		paddedImage[1 + y * (imageWidth * imageBytes)] = 0;
		paddedImage[2 + y * (imageWidth * imageBytes)] = 0;
		if (imageBytes == 4) paddedImage[3 + y * (imageWidth * imageBytes)] = 255;

		paddedImage[((imageWidth - 1) * imageBytes) + (y * (imageWidth * imageBytes))] = 0;
		paddedImage[(((imageWidth - 1) * imageBytes) + 1) + (y * (imageWidth * imageBytes))] = 0;
		paddedImage[(((imageWidth - 1) * imageBytes) + 2) + (y * (imageWidth * imageBytes))] = 0;
		if (imageBytes == 4) paddedImage[(((imageWidth - 1) * imageBytes) + 3) + (y * (imageWidth * imageBytes))] = 255;
	}

	return paddedImage;
}

void imageProcessor::colourHistogram(unsigned char image[], int imageWidth, int imageHeight, int imageBytes, unsigned char mask[], float redHist[], float greenHist[], float blueHist[]) {

	printf("%s \n", "Performing Colour Histogram with Mask");
	for (int y = 0; y < imageHeight; y++) {
		for (int x = 0; x < imageWidth; x++) {

			int index = ((y * imageWidth * imageBytes) + (x * imageBytes));
			int maskIndex = ((y * imageWidth) + (x));

			if (mask[maskIndex] != 0) {
				if (image[index + 3] != 0) {

					if (image[index] > 110) {
						if (image[index + 1] < 100 && image[index + 2] < 100) {
							redHist[image[index]]++;
						}
					}
					else if (image[index + 1] > 110) {
						if (image[index] < 100 && image[index + 2] < 100) {
							greenHist[image[index]]++;
						}
					}
					else if (image[index + 2] > 110) {
						if (image[index] < 100 && image[index + 1] < 100) {
							blueHist[image[index]]++;
						}
					}
				}
			}
		}
	}
}

void imageProcessor::colourHistogram(unsigned char image[], int imageWidth, int imageHeight, int imageBytes, float redHist[], float greenHist[], float blueHist[]) {

	printf("%s \n", "Performing Colour Histogram with no Mask");
	for (int y = 0; y < imageHeight; y++) {
		for (int x = 0; x < imageWidth; x++) {

			int index = ((y * imageWidth * imageBytes) + (x * imageBytes));
			int maskIndex = ((y * imageWidth) + (x));

			if (image[index + 3] != 0) {

				if (image[index] > 150) {
					if (image[index + 1] < 100 && image[index + 2] < 100) {
						redBucket++;
					}
				}
				else if (image[index + 1] > 150) {
					if (image[index] < 100 && image[index + 2] < 100) {
						greenBucket++;
					}
				}
				else if (image[index + 2] > 150) {
					if (image[index] < 100 && image[index + 1] < 100) {
						blueBucket++;
					}
				}
			}
		}
	}
}

unsigned char* imageProcessor::NonMaxSuppress(unsigned char* sobelX, unsigned char* sobelY, unsigned char*  combinedSobel, int imageWidth, int imageHeight, int imageBytes) {
	//Calculate the Edge Strengths
	printf("%s \n", "NMS: Calculating Edge Strengths");
	int size = imageWidth * imageHeight * imageBytes;
	unsigned char* edgeStrength = new unsigned char[size];

	printf("%s \n", "NMS: Calculating Gradient Directions");
	for (int i = 0; i < size; i++) {
		//Calculate the gradient direction
		if (sobelX[i] == 0) {
			if (sobelY[i] == 0) {
				edgeStrength[i] = 0;
			}
			else {
				edgeStrength[i] = 90;
			}
		}
		else {
			edgeStrength[i] = (float)atan(abs(sobelY[i]) / (float)abs(sobelX[i]));
		}

		//Round the gradient direction to nearest 45 degree
		if (edgeStrength[i] >= 0.0f && edgeStrength[i] < 22.5f) { edgeStrength[i] = 0.0f; }
		else if (edgeStrength[i] >= 22.5f && edgeStrength[i] < 67.5f) { edgeStrength[i] = 45.0f; }
		else if (edgeStrength[i] >= 67.5f && edgeStrength[i] <= 112.5f) { edgeStrength[i] = 90.0f; }
		else if (edgeStrength[i] > 112.5f && edgeStrength[i] <= 157.5f) { edgeStrength[i] = 135.0f; }

	}

	printf("%s \n", "Performing NMS");
	//Use edge strengths in NMS
	for (int y = 0; y < imageHeight; y++) {
		for (int x = 0; x < imageWidth; x++) {

			int index = ((y * imageWidth * imageBytes) + (x * imageBytes));

			if (edgeStrength[index] == 0.0f) {
				int rightPixel = ((y * imageWidth * imageBytes) + (x + 1 * imageBytes));
				int leftPixel = ((y * imageWidth * imageBytes) + (x - 1 * imageBytes));

				if ((leftPixel > 0 && rightPixel < size) && (combinedSobel[index] < combinedSobel[leftPixel] || combinedSobel[index] < combinedSobel[rightPixel])) { combinedSobel[index] = 0; }
			}
			else if (edgeStrength[index] == 45.0f) {
				int diagUpRightPixel = ((y - 1 * imageWidth * imageBytes) + (x + 1 * imageBytes));
				int diagDownLeftPixel = ((y + 1 * imageWidth * imageBytes) + (x - 1 * imageBytes));

				if ((diagUpRightPixel > 0 && diagUpRightPixel < size && diagDownLeftPixel > 0 && diagDownLeftPixel < size) && (combinedSobel[index] < combinedSobel[diagDownLeftPixel] || combinedSobel[index] < combinedSobel[diagUpRightPixel])) { combinedSobel[index] = 0; }
			}
			else if (edgeStrength[index] == 90.0f) {
				int abovePixel = ((y - 1 * imageWidth * imageBytes) + (x * imageBytes));
				int belowPixel = ((y + 1 * imageWidth * imageBytes) + (x * imageBytes));

				if ((abovePixel > 0 && belowPixel < size) && (combinedSobel[index] < combinedSobel[belowPixel] || combinedSobel[index] < combinedSobel[abovePixel])) { combinedSobel[index] = 0; }
			}
			else if (edgeStrength[index] == 135.0f) {
				int diagUpLeftPixel = ((y - 1 * imageWidth * imageBytes) + (x - 1 * imageBytes));
				int diagDownRightPixel = ((y + 1 * imageWidth * imageBytes) + (x + 1 * imageBytes));

				if ((diagUpLeftPixel > 0 && diagUpLeftPixel < size && diagDownRightPixel > 0 && diagDownRightPixel < size) && (combinedSobel[index] < combinedSobel[diagDownRightPixel] || combinedSobel[index] < combinedSobel[diagUpLeftPixel])) { combinedSobel[index] = 0; }
			}

		}
	}

	return combinedSobel;
}

unsigned char* imageProcessor::doubleThresholding(unsigned char image[], int imageWidth, int imageHeight, int imageBytes) {
	int size = imageWidth * imageHeight * imageBytes;

	printf("%s \n", "Performing Double-Thresholding");
	unsigned char* treshData = new unsigned char[size];

	for (int i = 0; i < size; i++) {
		if (image[i] < lowThresh) { treshData[i] = 0; }
		else if (image[i] > highThresh) { treshData[i] = strongEdge; }
		else if (image[i] >= lowThresh && image[i] <= highThresh) { treshData[i] = weakEdge; }
	}
	return treshData;
}

unsigned char* imageProcessor::hystTracking(unsigned char image[], int imageWidth, int imageHeight, int imageBytes) {
	printf("%s \n", "Applying Hysterisis Edge Tracking");
	int size = imageWidth * imageHeight * imageBytes;
	unsigned char* hystData = new unsigned char[size];

	for (int y = 0; y < imageHeight; y++) {
		for (int x = 0; x < imageWidth; x++) {

			int middleIndex = ((y * imageWidth * imageBytes) + (x * imageBytes));

			if (image[middleIndex] != strongEdge) {
				
				bool strongEdge = false;

				for (int xOff = -1; xOff <= 1; xOff++) {
					for (int yOff = -1; yOff <= 1; yOff++) {

						int outIndex = ((y + yOff * imageWidth * imageBytes) + (x + xOff * imageBytes));

						if (outIndex > 0 && outIndex < size) {
							if (image[outIndex] == strongEdge) {
								strongEdge = true;
							}
						}
					}
				}
				if (strongEdge) {
					hystData[middleIndex] = image[middleIndex];
				}
				else {
					hystData[middleIndex] = 0;
				}
			}
			else {
				hystData[middleIndex] = image[middleIndex];
			}
		}
	}

	return hystData;
}

unsigned char* imageProcessor::fillFromEdges(unsigned char image[], int imageWidth, int imageHeight, int imageBytes) {
	printf("%s \n", "Filling out from the Edges");
	int size = imageWidth * imageHeight * imageBytes;
	unsigned char* filledData = new unsigned char[size];
	int outerOffset = 10;

	for (int y = 0; y < imageHeight; y++) {
		int firstPixelIndex = 0;
		int lastPixelIndex = 0;

		for (int x = 0; x < imageWidth; x++) {

			int index = ((y * imageWidth * imageBytes) + (x * imageBytes));

			if (y > outerOffset && y < imageHeight - outerOffset && x > outerOffset && x < imageWidth - outerOffset) {
				if (image[index] == strongEdge || image[index] == weakEdge) {
					if (firstPixelIndex == 0) {
						firstPixelIndex = index;
					}
					else {
						lastPixelIndex = index;
					}
				}
				else {
					filledData[index] = 0;
				}
			}
			else {
				filledData[index] = 0;
			}
		}
		if (firstPixelIndex != 0) {
			for (int i = 0; i < lastPixelIndex - firstPixelIndex; i++) {
				filledData[firstPixelIndex + i] = 255;
			}
		}
	}

	return filledData;
}

void imageProcessor::loadHistogram(char* filename, float redHist[], float greenHist[], float blueHist[]) {
	std::ifstream file (filename);
	std::string redString;
	std::string greenString;
	std::string blueString;

	if (file.is_open()) {
		std::getline(file, redString);
		std::getline(file, greenString);
		std::getline(file, blueString);
	}
	file.close();

	size_t pos1 = 0;
	size_t pos2;

	for (int i = 0; i < 256; i++) {
		pos2 = redString.find(",", pos1);
		std::string subString = redString.substr(pos1, (pos2 - pos1));
		redHist[i] = atoi(subString.c_str());
		pos1 = pos2 + 1;
	}

	pos1 = 0;
	for (int i = 0; i < 256; i++) {
		pos2 = greenString.find(",", pos1);
		std::string subString = greenString.substr(pos1, (pos2 - pos1));
		greenHist[i] = atoi(subString.c_str());

		pos1 = pos2 + 1;
	}

	pos1 = 0;
	for (int i = 0; i < 256; i++) {
		pos2 = blueString.find(",", pos1);
		std::string subString = blueString.substr(pos1, (pos2 - pos1));
		blueHist[i] = atoi(subString.c_str());

		pos1 = pos2 + 1;	
	}
}

void imageProcessor::saveHistogram(char* filename, float redHist[], float greenHist[], float blueHist[]) {
	printf("%s", "Saving Colour Histogram to file: ");
	printf("%s \n", filename);
	std::ofstream file (filename);
	std::string redString;
	std::string greenString;
	std::string blueString;

	for (int i = 0; i < 256; i++) {
		std::stringstream stream;
		stream << redHist[i];
		redString += stream.str() + std::string(",");
	}
	for (int i = 0; i < 256; i++) {
		std::stringstream stream;
		stream << greenHist[i];
		greenString += stream.str() + std::string(",");
	}
	for (int i = 0; i < 256; i++) {
		std::stringstream stream;
		stream << blueHist[i];
		blueString += stream.str() + std::string(",");
	}


	if (file.is_open()) {
		file << redString + std::string("\n");
		file << greenString + std::string("\n");
		file << blueString + std::string("\n");
	}
	file.close();
}

void imageProcessor::compareHistogram(float redHist[], float greenHist[], float blueHist[], std::string imageArray[]) {
	float loadedRedHist[256];
	float loadedBlueHist[256];
	float loadedGreenHist[256];

	float currentRedHist[256];
	float currentGreenHist[256];
	float currentBlueHist[256];

	double redChi = 0;
	double greenChi = 0;
	double blueChi = 0;

	double lowestRedChi = 100;
	double lowestGreenChi = 100;
	double lowestBlueChi = 100;

	int currentClosestRed;
	int currentClosestGreen;
	int currentClosestBlue;

	int closestArray[15];
	int redChiArray[15];
	int greenChiArray[15];
	int blueChiArray[15];
	int j = 0;


	for (int i = 0; i < 14; i++) {

		for (int i = 0; i < 256; i++) {
			currentRedHist[i] = redHist[i];
			currentGreenHist[i] = greenHist[i];
			currentBlueHist[i] = blueHist[i];
		}

		imageArray[i].append(".txt");
		char* filename = new char[imageArray[i].length() + 1];
		strcpy(filename, imageArray[i].c_str());

		loadHistogram(filename, loadedRedHist, loadedGreenHist, loadedBlueHist);
		normaliseColourHistogram(currentRedHist, currentGreenHist, currentBlueHist);
		normaliseColourHistogram(loadedRedHist, loadedGreenHist, loadedBlueHist);

		for (int i = 0; i < 256; i++) {
			if (loadedRedHist[i] != 0 || currentRedHist[i] != 0) {
				float difference = currentRedHist[i] - loadedRedHist[i];
				redChi += pow(difference, 2) / (currentRedHist[i] + loadedRedHist[i]);
			}
			if (loadedGreenHist[i] != 0 || currentGreenHist[i] != 0) {
				float difference = currentGreenHist[i] - loadedGreenHist[i];
				greenChi += pow(difference, 2) / (currentGreenHist[i] + loadedGreenHist[i]);
			}
			if (loadedBlueHist[i] != 0 || currentBlueHist[i] != 0) {
				float difference = currentBlueHist[i] - loadedBlueHist[i];
				blueChi += pow(difference, 2) / (currentBlueHist[i] + loadedBlueHist[i]);
			}
		}

		if (redChi <= 5 && greenChi <= 5 && blueChi <= 5) {
			closestArray[j+1] = i;
			redChiArray[j+1] = redChi;
			greenChiArray[j+1] = greenChi;
			blueChiArray[j+1] = blueChi;
			j++;
		}
		else {
			closestArray[j+1] = NULL;
			redChiArray[j+1] = NULL;
			greenChiArray[j+1] = NULL;
			blueChiArray[j+1] = NULL;
			j++;
		}

		redChi = 0;
		greenChi = 0;
		blueChi = 0;

	}

	printf("%s \n", "This Apple is Similar to: ");
	for (int i = 1; i < 15; i++) {
		if (closestArray[i] != NULL) {
			std::string apple = imageArray[i-1].substr(7, imageArray[i].length() - 3);
			printf("%s", apple.c_str());
			printf("  Red Chi: %d, Green Chi: %d, Blue Chi: %d \n", redChiArray[i], greenChiArray[i], blueChiArray[i]);
		}
	}
}

void imageProcessor::normaliseColourHistogram(float redHist[], float greenHist[], float blueHist[]) {
	for (int i = 0; i < 256; i++) {
		redHist[i] = redHist[i] / 256;
		greenHist[i] = greenHist[i] / 256;
		blueHist[i] = blueHist[i] / 256;
	}
}