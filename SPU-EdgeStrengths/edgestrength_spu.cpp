#include <stdio.h>
#include <spu_mfcio.h>
#include <stdlib.h>
#include <math.h>

struct nmsData {
	void * inputDataX;
	void * inputDataY;
	void * outputData;
	int imageWidth;
	char padding[112];
};

int main (unsigned long long spe_id, unsigned long long argp, unsigned long long envp){
	printf("%s \n", "NMS: Calculating Edge Strengths");
	
	nmsData input __attribute__((aligned(128)));	
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
	unsigned char lsbuffer2[chunkSize]__attribute__((aligned(128)));
	unsigned char edgeStrength[chunkSize]__attribute__((algined(128)));
	
	int currentChunk = spuID * input.imageWidth/4 * input.imageWidth;
	
	int iterations = (input.imageWidth * input.imageWidth/4) / 16384;
	
	for(int iter = 0; iter < iterations; iter++){
		mfc_get(lsbuffer, (unsigned int)(input.inputDataX)+currentChunk, sizeof(lsbuffer), tagID, 0, 0);
		mfc_get(lsbuffer2, (unsigned int)(input.inputDataY)+currentChunk, sizeof(lsbuffer2), tagID, 0, 0);
		mfc_read_tag_status_all();

		for (int i = 0; i < chunkSize; i++) {
			//Calculate the gradient direction
			if (lsbuffer[i] == 0) {
				if (lsbuffer2[i] == 0) {
					edgeStrength[i] = 0;
				}
				else {
					edgeStrength[i] = 90;
				}
			}
			else {
				edgeStrength[i] = (float)atan(abs(lsbuffer2[i]) / (float)abs(lsbuffer[i]));
			}

			//Round the gradient direction to nearest 45 degree
			if (edgeStrength[i] >= 0.0f && edgeStrength[i] < 22.5f) { edgeStrength[i] = 0.0f; }
			else if (edgeStrength[i] >= 22.5f && edgeStrength[i] < 67.5f) { edgeStrength[i] = 45.0f; }
			else if (edgeStrength[i] >= 67.5f && edgeStrength[i] <= 112.5f) { edgeStrength[i] = 90.0f; }
			else if (edgeStrength[i] > 112.5f && edgeStrength[i] <= 157.5f) { edgeStrength[i] = 135.0f; }

		}
		
		mfc_put(edgeStrength, (unsigned int)(input.outputData)+currentChunk, sizeof(edgeStrength), tagID, 0, 0);
		mfc_read_tag_status_all();
		currentChunk += chunkSize;		
	}

	return 0;
}