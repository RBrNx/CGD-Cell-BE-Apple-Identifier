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
	printf("%s \n", "Performing NMS");
	
	nmsData input __attribute__((aligned(128)));	
	unsigned int spuID = spu_read_in_mbox();
	int tagID = 1;
	
	printf("     Getting Struct for DMA \n");
	mfc_get((void *)&input, argp, envp, tagID, 0, 0);
	mfc_write_tag_mask(1<<tagID);
	mfc_read_tag_status_all();
	int chunkWidth = input.imageWidth;
	int chunkHeight = 3;
	int chunkSize = chunkWidth * chunkHeight;
	unsigned char sobel[chunkSize]__attribute__((aligned(128)));
	unsigned char edgestrengths[chunkSize]__attribute__((aligned(128)));
	unsigned char NMS[chunkSize/3]__attribute__((aligned(128)));
	
	int currentChunk = spuID * input.imageWidth/4 * input.imageWidth;
	
	int iterations = input.imageWidth/4;
	if(spuID == 3){
		iterations = iterations - 2;
	}
	
	for(int iter = 0; iter < iterations; iter++){
		mfc_get(sobel, (unsigned int)(input.inputDataX)+currentChunk, sizeof(sobel), tagID, 0, 0);
		mfc_get(edgestrengths, (unsigned int)(input.inputDataY)+currentChunk, sizeof(edgestrengths), tagID, 0, 0);
		mfc_read_tag_status_all();

		for (int y = 1; y < chunkHeight-1; y++) {
			for (int x = 1; x < chunkWidth-1; x++) {

				int index = ((y * chunkWidth) + (x));

				if (edgestrengths[index] == 0.0f) {
					int rightPixel = ((y * chunkWidth) + (x + 1));
					int leftPixel = ((y * chunkWidth) + (x - 1));

					if ((sobel[x] < sobel[leftPixel] || sobel[x] < sobel[rightPixel])) { NMS[x] = 0; } else { NMS[x] = sobel[x]; }
				}
				else if (edgestrengths[index] == 45.0f) {
					int diagUpRightPixel = ((y - 1 * chunkWidth) + (x + 1));
					int diagDownLeftPixel = ((y + 1 * chunkWidth) + (x - 1));

					if ((sobel[x] < sobel[diagDownLeftPixel] || sobel[x] < sobel[diagUpRightPixel])) { NMS[x] = 0; } else { NMS[x] = sobel[x]; }
				}
				else if (edgestrengths[index] == 90.0f) {
					int abovePixel = ((y - 1 * chunkWidth) + (x));
					int belowPixel = ((y + 1 * chunkWidth) + (x));

					if ((sobel[x] < sobel[belowPixel] || sobel[x] < sobel[abovePixel])) { NMS[x] = 0; } else { NMS[x] = sobel[x]; }
				}
				else if (edgestrengths[index] == 135.0f) {
					int diagUpLeftPixel = ((y - 1 * chunkWidth) + (x - 1));
					int diagDownRightPixel = ((y + 1 * chunkWidth) + (x + 1));

					if ((sobel[x] < sobel[diagDownRightPixel] || sobel[x] < sobel[diagUpLeftPixel])) { NMS[x] = 0; } else { NMS[x] = sobel[x]; }
				}
			}
		}
		
		mfc_put(NMS, (unsigned int)(input.outputData)+currentChunk, sizeof(NMS), tagID, 0, 0);
		mfc_read_tag_status_all();
		currentChunk += chunkWidth;		
	}
	printf("SPU ID: %d Finished \n", spuID);
	return 0;
}