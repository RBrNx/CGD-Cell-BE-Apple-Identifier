#include <stdio.h>
#include <spu_mfcio.h>
#include <stdlib.h> 

void RGBtoGreyscale(unsigned char inputImage[], int imageWidth, int imageHeight, int imageBytes, unsigned char outputImage[]) {
	//printf("%s \n", "Converting RGB to Greyscale");

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
	mfc_get((void *)&input, argp, envp, tagID, 0 ,0);
	mfc_write_tag_mask(1<<tagID);
	mfc_read_tag_status_all();
	int chunkSize = 128 * 128;
	unsigned char lsbuffer[chunkSize]__attribute__((aligned(128)));
	unsigned char greybuffer[chunkSize/4]__attribute__((aligned(128)));
	
	
	int iterations = ((input.imageWidth * input.imageWidth * 4) / 65536) / 4; //imageSize in bytes / 64KB / Number of SPUs
	
	int currentChunk = spuID * iterations * 4 * chunkSize;
	
	for(int iter = 0; iter < iterations; iter++){
		for(int i = 0; i < 4; i++){
			mfc_get(lsbuffer, (unsigned int)(input.inputData)+currentChunk, chunkSize, tagID, 0, 0);
			mfc_read_tag_status_all();
			
			RGBtoGreyscale(lsbuffer, 128, 128, 4, greybuffer);
			
			mfc_put(greybuffer, (unsigned int)(input.outputData)+currentChunk/4, chunkSize/4, tagID, 0, 0);
			mfc_read_tag_status_all();
			currentChunk += chunkSize;		
		}
	}

	return 0;
}