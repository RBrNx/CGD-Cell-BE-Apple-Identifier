#include <stdio.h>
#include <spu_mfcio.h>
#include <stdlib.h> 

float gaussianFilter[25] = {	1, 4, 7, 4, 1,
								4, 16, 26, 16, 4,
								7, 26, 41, 26, 7,
								4, 16, 26, 16, 4,
								1, 4, 7, 4, 1		};


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

struct dmaData {
	void * inputData;
	void * outputData;
	int imageWidth;
	char padding[116];
};

int main (unsigned long long spe_id, unsigned long long argp, unsigned long long envp){
	printf("%s \n", "Applying Gaussian Filter");
	
	for (int i = 0; i < 25; i++) {
		gaussianFilter[i] *= 1 / 273.0f;
	}
	float gaussImageSample[25];
	
	dmaData input __attribute__((aligned(128)));	
	unsigned int spuID = spu_read_in_mbox();
	int tagID = 1;
	
	printf("     Getting Struct for DMA \n");
	mfc_get((void *)&input, argp, envp, tagID, 0, 0);
	mfc_write_tag_mask(1<<tagID);
	mfc_read_tag_status_all();
	int chunkWidth = input.imageWidth;
	int chunkHeight = 5;
	int chunkSize = chunkWidth * chunkHeight;
	unsigned char lsbuffer[chunkSize]__attribute__((aligned(128)));
	unsigned char gaussbuffer[chunkSize/5]__attribute__((aligned(128)));
	
	int iterations = input.imageWidth/4 + 1;
	
	int currentChunk = spuID * input.imageWidth/4 * input.imageWidth;
	
	if(spuID == 3){
		iterations = iterations - 4;
	}
	
	for(int iter = 0; iter < iterations; iter++){
		mfc_get(lsbuffer, (unsigned int)(input.inputData)+currentChunk, sizeof(lsbuffer), tagID, 0, 0);
		mfc_read_tag_status_all();
		
		for (int y = 2; y < chunkHeight - 2; y++) {
			for (int x = 2; x < chunkWidth - 2; x++) {

				int index = (y * chunkWidth) + (x);

				calculateImageSample(lsbuffer, index, 1, chunkWidth, gaussImageSample, 5, 5);

				int result = applyFilter(gaussImageSample, gaussianFilter, 5, 5);

				result = result > 255 ? 255 : result;

			
				gaussbuffer[x] = result;
			}
		}
		
		mfc_put(gaussbuffer, (unsigned int)(input.outputData)+currentChunk, sizeof(gaussbuffer), tagID, 0, 0);
		mfc_read_tag_status_all();
		currentChunk += chunkWidth;		
	}

	return 0;
}