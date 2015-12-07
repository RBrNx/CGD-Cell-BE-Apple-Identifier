#include "imageIO.h"
#include "imageProcessor.h"
#include <stdio.h>
#include <stdlib.h>
#include <libspe2.h>
#include <pthread.h>

imageIO imageLoader;
imageProcessor imagePro;

// Data to send to the pthread
typedef struct ppu_pthread_data { 
	spe_context_ptr_t context;
	unsigned int entry;
	void* argp;
	void* envp;
} ppu_pthread_data_t;

void* ppu_pthread_function(void* arg){
	ppu_pthread_data_t *data = (ppu_pthread_data_t *) arg;
	spe_context_run(data->context, &data->entry, 0 ,data->argp, data->envp, NULL);
	pthread_exit(NULL);
}

int findFactor(int num){
	int largest;
	
	for(int i = 1; i<= num; i++){
		if (!(num % i)) {
			if(i != num && i <= 128 && i % 4 == 0){
				largest = i;
			}
		}
	}
	
	return largest;
}

struct dmaData {
	void * inputData;
	void * outputData;
	char padding[120];
};

// SPU Init Data
extern spe_program_handle_t main_spu; //Program Handle

int main(){
	ppu_pthread_data_t ptdata[8];
	pthread_t pthread[8];
	unsigned int i;
	
	//MAIN CODE

	unsigned char* rawData = imageLoader.openImage("Apples/test.png");
	int imageWidth = imageLoader.getImageWidth();
	int imageHeight = imageLoader.getImageHeight();
	int imageBytes = imageLoader.getImageBytes();
	int imageSize = imageWidth * imageHeight * imageBytes;
	unsigned char* squareData = imageLoader.squareImage(rawData, imageWidth, imageHeight, imageBytes);
	delete[] rawData;
	unsigned char testData[imageSize]__attribute__((aligned(128)));
	unsigned char greyData[imageSize/4]__attribute__((aligned(128)));
	for(int i = 0; i < imageSize; i++){
		testData[i] = squareData[i];
	}
	imageWidth = imageLoader.getImageWidth();
	imageHeight = imageLoader.getImageHeight();
	imageSize = imageWidth * imageHeight * imageBytes;
	printf("Image Size: %d \n", imageWidth);
	
	dmaData DMA __attribute__((aligned(128)));
	
	DMA.inputData = (void *)testData;
	DMA.outputData = (void *)greyData;
		
	// Determine number of available SPU
	//printf("%s \n", "Determining Number of SPUs");
	//spus = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, 0);
	
	//Create a context and thread for each SPU
	for(int i = 0; i < 1; i++){
		printf("Create Context for SPU: %d \n", i);
		//Create Context
		ptdata[i].context = spe_context_create(0, NULL);
		
		//Load Program into Context
		spe_program_load(ptdata[i].context, &main_spu);
		
		ptdata[i].entry = SPE_DEFAULT_ENTRY;
		ptdata[i].argp = (void *)&(DMA);
		ptdata[i].envp = (void *) sizeof(dmaData);
		
		printf("Create Thread for SPU %d \n", i);
		//Create thread
		pthread_create(&pthread[i], NULL, &ppu_pthread_function, &ptdata[i]);
	}
	
	//Wait for threads to finish processing
	for(int i = 0; i < 1; i++){
		printf("Joining Thread for SPU %d \n", i);
		pthread_join(pthread[i], NULL);
		
		printf("Destroying Context for SPU %d \n", i);
		spe_context_destroy(ptdata[i].context);
		
	}
	
	unsigned char* paddedImage = new unsigned char[imageSize];
	paddedImage = imagePro.padOutImage(greyData, imageWidth, imageHeight, imageBytes);

	imageLoader.saveImage("Apples/test-grey.png", paddedImage, imageSize);
	
	return 0;
}