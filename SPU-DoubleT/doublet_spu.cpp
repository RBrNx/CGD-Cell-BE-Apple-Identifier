#include <stdio.h>
#include <spu_mfcio.h>
#include <stdlib.h> 

int lowThresh = 50;
int highThresh = 150;
int strongEdge = 255;
int weakEdge = 100;

struct dmaData {
	void * inputData;
	void * outputData;
	int imageWidth;
	char padding[116];
};

int main (unsigned long long spe_id, unsigned long long argp, unsigned long long envp){

	dmaData input __attribute__((aligned(128)));	
	unsigned int spuID = spu_read_in_mbox();
	int tagID = 1;
	
	printf("     Getting Struct for DMA \n");
	mfc_get((void *)&input, argp, envp, tagID, 0, 0);
	mfc_write_tag_mask(1<<tagID);
	mfc_read_tag_status_all();
	int chunkWidth = 128;
	int chunkHeight = 128;
	int chunkSize = chunkWidth * chunkHeight;
	unsigned char lsbuffer[chunkSize]__attribute__((aligned(128)));
	
	int currentChunk = spuID * input.imageWidth/4 * input.imageWidth;
	
	int iterations = (input.imageWidth * input.imageWidth/4) / 16384;
	
	for(int iter = 0; iter < iterations; iter++){
		mfc_get(lsbuffer, (unsigned int)(input.inputData)+currentChunk, chunkSize, tagID, 0, 0);
		mfc_read_tag_status_all();
		
		for (int i = 0; i < chunkSize; i++) {
			if (lsbuffer[i] < lowThresh) { lsbuffer[i] = 0; }
			else if (lsbuffer[i] > highThresh) { lsbuffer[i] = strongEdge; }
			else if (lsbuffer[i] >= lowThresh && lsbuffer[i] <= highThresh) { lsbuffer[i] = weakEdge; }
		}
		
		mfc_put(lsbuffer, (unsigned int)(input.outputData)+currentChunk, chunkSize, tagID, 0, 0);
		mfc_read_tag_status_all();
		currentChunk += chunkSize;		
	}

	return 0;
}