#include <stdio.h>
#include <spu_mfcio.h>
#include <stdlib.h> 

int strongEdge = 255;

struct dmaData {
	void * inputData;
	void * outputData;
	int imageWidth;
	char padding[116];
};

int main (unsigned long long spe_id, unsigned long long argp, unsigned long long envp){
	printf("%s \n", "Applying Hysterisis Edge Tracking");
	dmaData input __attribute__((aligned(128)));	
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
	unsigned char hyst[chunkSize/3]__attribute__((aligned(128)));
	
	int currentChunk = spuID * input.imageWidth/4 * input.imageWidth;
	
	int iterations = input.imageWidth/4;
	if(spuID == 3){
		iterations = iterations - 2;
	}
	
	for(int iter = 0; iter < iterations; iter++){
		mfc_get(lsbuffer, (unsigned int)(input.inputData)+currentChunk, chunkSize, tagID, 0, 0);
		mfc_read_tag_status_all();

		for (int y = 1; y < chunkHeight-1; y++) {
			for (int x = 1; x < chunkWidth-1; x++) {

				int middleIndex = ((y * chunkWidth) + (x));

				if (lsbuffer[middleIndex] != strongEdge) {
					
					bool strongEdge = false;

					for (int xOff = -1; xOff <= 1; xOff++) {
						for (int yOff = -1; yOff <= 1; yOff++) {

							int outIndex = ((y + yOff * chunkWidth) + (x + xOff));

							if (outIndex > 0 && outIndex < chunkSize) {
								if (lsbuffer[outIndex] == strongEdge) {
									strongEdge = true;
								}
							}
						}
					}
					if (strongEdge) {
						hyst[x] = lsbuffer[middleIndex];
					}
					else {
						hyst[x] = 0;
					}
				}
				else {
					hyst[x] = lsbuffer[middleIndex];
				}
			}
		}
		
		mfc_put(hyst, (unsigned int)(input.outputData)+currentChunk, sizeof(hyst), tagID, 0, 0);
		mfc_read_tag_status_all();
		currentChunk += chunkWidth;		
	}

	return 0;
}