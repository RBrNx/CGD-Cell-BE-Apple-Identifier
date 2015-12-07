#include <stdio.h>
#include <spu_mfcio.h>
#include <stdlib.h> 

int lowThresh = 70;
int highThresh = 150;
int strongEdge = 255;
int weakEdge = 100;

int redBucket;
int blueBucket;
int greenBucket;

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


void RGBtoGreyscale(unsigned char inputImage[], int imageWidth, int imageHeight, int imageBytes, unsigned char outputImage[]) {
	printf("%s \n", "Converting RGB to Greyscale");

	for (int y = 0; y < imageHeight; y++) {
		for (int x = 0; x < imageWidth; x++) {

			int imageIndex = ((y * imageWidth * imageBytes) + (x * imageBytes));
			int greyIndex = ((y * imageWidth) + (x));
			float greyscale = 0;
			greyscale += inputImage[imageIndex] * 0.21;
			greyscale += inputImage[imageIndex + 1] * 0.71;
			greyscale += inputImage[imageIndex + 2] * 0.07;
			outputImage[greyIndex] = greyscale;
		}
	}
}

void calculateImageSample(unsigned char image[], int index, int imageBytes, int imageWidth, float outData[], int kernelWidth, int kernelHeight) {

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

int applyFilter(float imageSample[], float kernel[], int kernelWidth, int kernelHeight) {

	int filteredValue = 0;

	for (int x = 0; x < kernelWidth; x++) {
		for (int y = 0; y < kernelHeight; y++) {

			int index = x * kernelHeight + y;
			filteredValue += imageSample[index] * kernel[index];
		}
	}

	return filteredValue;

}

unsigned char* doubleThresholding(unsigned char image[], int imageWidth, int imageHeight, int imageBytes) {
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

unsigned char* hystTracking(unsigned char image[], int imageWidth, int imageHeight, int imageBytes) {
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

struct dmaData {
	void * inputData;
	void * outputData;
	char padding[120];
};

int main (unsigned long long spe_id, unsigned long long argp, unsigned long long envp){

	int currentChunk = 0;
	int chunkSize = 128 * 128;
	dmaData input __attribute__((aligned(128)));
	unsigned char lsbuffer[chunkSize]__attribute__((aligned(128)));
	unsigned char greybuffer[chunkSize/4]__attribute__((aligned(128)));
	int tagID = 1;
	
	printf("     Getting Struct for DMA \n");
	mfc_get((void *)&input, argp, envp, tagID, 0 ,0);
	mfc_write_tag_mask(1<<tagID);
	mfc_read_tag_status_all();
	
	printf("     InputImage Pointer: %p\n",input.inputData);
	printf("     outputImage Pointer: %p\n",input.outputData);

	
	for(int i = 0; i < 4; i++){
		mfc_get(lsbuffer, (unsigned int)(input.inputData)+currentChunk, chunkSize, tagID, 0, 0);
		mfc_read_tag_status_all();
		
		RGBtoGreyscale(lsbuffer, 128, 128, 4, greybuffer);
		//for(int j = 0; j < chunkSize; j++){
			//greybuffer[i] = lsbuffer[i];
		//}
		
		mfc_put(greybuffer, (unsigned int)(input.outputData)+currentChunk/4, chunkSize/4, tagID, 0, 0);
		mfc_read_tag_status_all();
		currentChunk += chunkSize;		
	}

	return 0;
}