#include <stdio.h>
#include <spu_mfcio.h>
#include <stdlib.h> 

int strongEdge = 255;

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
	int spuIterations;
	char padding[116];
};

int main (unsigned long long spe_id, unsigned long long argp, unsigned long long envp){

	/*int chunkSize = 128 * 128;
	dmaData input __attribute__((aligned(128)));
	unsigned char lsbuffer[chunkSize]__attribute__((aligned(128)));
	unsigned char greybuffer[chunkSize/4]__attribute__((aligned(128)));
	int tagID = 1;
	
	unsigned int spuID = spu_read_in_mbox();
	
	printf("     Getting Struct for DMA \n");
	mfc_get((void *)&input, argp, envp, tagID, 0 ,0);
	mfc_write_tag_mask(1<<tagID);
	mfc_read_tag_status_all();
	
	int currentChunk = spuID * input.spuIterations * 4 * chunkSize;*/
	
	/*for(int iter = 0; iter < input.spuIterations; iter++){
		for(int i = 0; i < 4; i++){
			mfc_get(lsbuffer, (unsigned int)(input.inputData)+currentChunk, chunkSize, tagID, 0, 0);
			mfc_read_tag_status_all();
			
			RGBtoGreyscale(lsbuffer, 128, 128, 4, greybuffer);
			
			mfc_put(greybuffer, (unsigned int)(input.outputData)+currentChunk/4, chunkSize/4, tagID, 0, 0);
			mfc_read_tag_status_all();
			currentChunk += chunkSize;		
		}
	}*/

	return 0;
}