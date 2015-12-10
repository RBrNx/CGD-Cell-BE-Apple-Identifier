#include <stdio.h>
#include <spu_mfcio.h>
#include <stdlib.h>
#include <math.h>

float sobelFilterX[9] = {	-1, 0, 1,
							-2, 0, 2,
							-1, 0, 1	};

float sobelFilterY[9] = {	-1, -2, -1,
							0, 0, 0,
							1, 2, 1		};


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

struct sobelData {
	void * inputData;
	void * outputX;
	void * outputY;
	int imageWidth;
	char padding[112];
};

int main (unsigned long long spe_id, unsigned long long argp, unsigned long long envp){
	printf("%s \n", "Applying Sobel Filter");
	
	float imageSample[9];
	
	sobelData input __attribute__((aligned(128)));	
	unsigned int spuID = spu_read_in_mbox();
	int tagID = 1;
	
	printf("     Getting Struct for DMA \n");
	mfc_get((void *)&input, argp, envp, tagID, 0, 0);
	mfc_write_tag_mask(1<<tagID);
	mfc_read_tag_status_all();
	int chunkWidth = input.imageWidth;
	int chunkHeight = 3;
	int chunkSize = chunkWidth * chunkHeight;
	unsigned char lsbuffer[chunkSize]__attribute__((aligned(128)));
	//unsigned char sobelX[chunkSize/3]__attribute__((aligned(128)));
	//unsigned char sobelY[chunkSize/3]__attribute__((aligned(128)));
	unsigned char sobelComb[chunkSize/3]__attribute__((algined(128)));
	
	int currentChunk = spuID * input.imageWidth/4 * input.imageWidth;
	
	int iterations = input.imageWidth/4;
	if(spuID == 3){
		iterations = iterations - 2;
	}
	
	printf("%s \n", "Running Sobel Filter");
	for(int iter = 0; iter < iterations; iter++){
		mfc_get(lsbuffer, (unsigned int)(input.inputData)+currentChunk, sizeof(lsbuffer), tagID, 0, 0);
		mfc_read_tag_status_all();
		
		for (int y = 1; y < chunkHeight - 1; y++) {
			for (int x = 1; x < chunkWidth - 1; x++) {

				int index = (y * chunkWidth) + (x);

				calculateImageSample(lsbuffer, index, 1, chunkWidth, imageSample, 3, 3);

				int resultX = applyFilter(imageSample, sobelFilterX, 3, 3);
				int resultY = applyFilter(imageSample, sobelFilterY, 3, 3);

				int fin = sqrt((resultX * resultX * 3) + (resultY  *resultY * 3));

				fin = fin > 255 ? 255 : fin;
				//resultX = resultX > 255 ? 255 : resultX;
				//resultY = resultY > 255 ? 255 : resultY;

				unsigned char finalChar = fin;
				//unsigned char finalXChar = resultX;
				//unsigned char finalYChar = resultY;

				sobelComb[x] = finalChar;
				//sobelX[x] = finalXChar;
				//sobelY[x] = finalYChar;
			}
		}
		
		mfc_put(sobelComb, (unsigned int)(input.outputX)+currentChunk, sizeof(sobelComb), tagID, 0, 0);
		//mfc_put(sobelY, (unsigned int)(input.outputY)+currentChunk, sizeof(sobelY), tagID, 0, 0);
		mfc_read_tag_status_all();
		currentChunk += chunkWidth;		
	}

	return 0;
}