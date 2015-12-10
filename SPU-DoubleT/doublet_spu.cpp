#include <stdio.h>
#include <spu_mfcio.h>
#include <stdlib.h> 

int lowThresh = 70;
int highThresh = 150;
int strongEdge = 255;
int weakEdge = 100;

void doubleThresholding(unsigned char image[], int imageWidth, int imageHeight, int imageBytes) {
	int size = imageWidth * imageHeight * imageBytes;
	printf("%s \n", "Performing Double-Thresholding");

	for (int i = 0; i < size; i++) {
		if (image[i] < lowThresh) { image[i] = 0; }
		else if (image[i] > highThresh) { image[i] = strongEdge; }
		else if (image[i] >= lowThresh && image[i] <= highThresh) { image[i] = weakEdge; }
	}
}

struct dmaData {
	void * inputData;
	void * outputData;
	int spuIterations;
	char padding[116];
};

int main (unsigned long long spe_id, unsigned long long argp, unsigned long long envp){

	int chunkSize = 128 * 128;
	dmaData input __attribute__((aligned(128)));
	unsigned char lsbuffer[chunkSize]__attribute__((aligned(128)));
	int tagID = 2;
	
	unsigned int spuID = spu_read_in_mbox();
	
	printf("     Getting Struct for DMA \n");
	mfc_get((void *)&input, argp, 128, tagID, 0 ,0);
	mfc_write_tag_mask(1<<tagID);
	mfc_read_tag_status_all();
	
	int currentChunk = spuID * input.spuIterations * chunkSize;
	
	for(int iter = 0; iter < input.spuIterations; iter++){
		for(int i = 0; i < 4; i++){
			mfc_get(lsbuffer, (unsigned int)(input.inputData)+currentChunk, chunkSize, tagID, 0, 0);
			mfc_read_tag_status_all();
			
			doubleThresholding(lsbuffer, chunkSize, chunkSize, 1);
			
			mfc_put(lsbuffer, (unsigned int)(input.outputData)+currentChunk, chunkSize, tagID, 0, 0);
			mfc_read_tag_status_all();
			currentChunk += chunkSize;		
		}
	}

	return 0;
}