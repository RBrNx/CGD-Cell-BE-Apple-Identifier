#include "imageIO.h"
#include "imageProcessor.h"
#include <stdio.h>
#include <stdlib.h>
#include <libspe2.h>
#include <pthread.h>
#include <math.h>

imageIO imageLoader;
imageProcessor imagePro;

float sobelFilterX[9] = {	-1, 0, 1,
							-2, 0, 2,
							-1, 0, 1	};

float sobelFilterY[9] = {	-1, -2, -1,
							0, 0, 0,
							1, 2, 1		};

float gaussianFilter[25] = {	1, 4, 7, 4, 1,
								4, 16, 26, 16, 4,
								7, 26, 41, 26, 7,
								4, 16, 26, 16, 4,
								1, 4, 7, 4, 1		};


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
	int imageWidth;
	char padding[116];
};

struct sobelData {
	void * inputData;
	void * outputX;
	void * outputY;
	int imageWidth;
	char padding[112];
};

void initSPUs(int spus, ppu_pthread_data_t ptdata[] ,pthread_t pthread[], struct dmaData dmaStruct, spe_program_handle_t program){
	//Create a context and thread for each SPU
	for(int i = 0; i < spus; i++){
		unsigned int spuID = i;
		printf("Create Context for SPU: %d \n", i);
		//Create Context
		ptdata[i].context = spe_context_create(0, NULL);
		
		//Load Program into Context
		spe_program_load(ptdata[i].context, &program);
		
		ptdata[i].entry = SPE_DEFAULT_ENTRY;
		ptdata[i].argp = (void *)&(dmaStruct);
		ptdata[i].envp = (void *) sizeof(dmaStruct);
		
		printf("Create Thread for SPU %d \n", i);
		//Create thread
		pthread_create(&pthread[i], NULL, &ppu_pthread_function, &ptdata[i]);
		//Send SPUID to each SPU 
		spe_in_mbox_write(ptdata[i].context,&spuID,1,SPE_MBOX_ANY_NONBLOCKING);
	}
}

void initSPUs(int spus, ppu_pthread_data_t ptdata[] ,pthread_t pthread[], struct sobelData dmaStruct, spe_program_handle_t program){
	//Create a context and thread for each SPU
	for(int i = 0; i < spus; i++){
		unsigned int spuID = i;
		printf("Create Context for SPU: %d \n", i);
		//Create Context
		ptdata[i].context = spe_context_create(0, NULL);
		
		//Load Program into Context
		spe_program_load(ptdata[i].context, &program);
		
		ptdata[i].entry = SPE_DEFAULT_ENTRY;
		ptdata[i].argp = (void *)&(dmaStruct);
		ptdata[i].envp = (void *) sizeof(dmaStruct);
		
		printf("Create Thread for SPU %d \n", i);
		//Create thread
		pthread_create(&pthread[i], NULL, &ppu_pthread_function, &ptdata[i]);
		//Send SPUID to each SPU 
		spe_in_mbox_write(ptdata[i].context,&spuID,1,SPE_MBOX_ANY_NONBLOCKING);
	}
}

void destroySPUs(int spus, ppu_pthread_data_t ptdata[], pthread_t pthread[]){
	//Wait for threads to finish processing
	for(int i = 0; i < spus; i++){
		printf("Joining Thread for SPU %d \n", i);
		pthread_join(pthread[i], NULL);
		
		printf("Destroying Context for SPU %d \n", i);
		spe_context_destroy(ptdata[i].context);
		
	}
}

// SPU Init Data
extern spe_program_handle_t greyscale_spu;
extern spe_program_handle_t gaussian_spu;
extern spe_program_handle_t sobel_spu;
extern spe_program_handle_t doublet_spu;
extern spe_program_handle_t hyst_spu;

int main(){
	ppu_pthread_data_t ptdata[8];
	pthread_t pthread[8];
	unsigned int i;

	//Load in Image
	unsigned char* rawData = imageLoader.openImage("Apples/Gala.png");
	int imageWidth = imageLoader.getImageWidth();
	int imageHeight = imageLoader.getImageHeight();
	int imageBytes = imageLoader.getImageBytes();
	int imageSize = imageWidth * imageHeight * imageBytes;

	//Make image Power of Two
	unsigned char *squareData = imageLoader.squareImage(rawData, imageWidth, imageHeight, imageBytes);
	delete[] rawData;

	imageWidth = imageLoader.getImageWidth();
	imageHeight = imageLoader.getImageHeight();
	imageSize = imageWidth * imageHeight * imageBytes;
	
	dmaData DMA __attribute__((aligned(128)));
	
	//RGB to Greyscale on SPUs
	unsigned char greyData[imageSize/4]__attribute__((aligned(128)));
	DMA.inputData = (void *)squareData;
	DMA.outputData = (void *)greyData;
	DMA.imageWidth = imageWidth;
	initSPUs(4, ptdata, pthread, DMA, greyscale_spu);
	destroySPUs(4, ptdata, pthread);
	delete[] squareData;
	imageSize = imageWidth * imageHeight;
	
	//Run Gaussian Filter on SPUs
	unsigned char *gaussData = (unsigned char*)memalign(128,imageSize);
	DMA.inputData = (void *)greyData;
	DMA.outputData = (void *)gaussData;
	DMA.imageWidth = imageWidth;
	initSPUs(4, ptdata, pthread, DMA, gaussian_spu);
	destroySPUs(4, ptdata, pthread);
	
	//Run Sobel Filter on SPUs
	sobelData sobelDMA __attribute__((aligned(128)));
	unsigned char *sobelX = (unsigned char*)memalign(128,imageSize); //Sobel SQRT is currently done on the SPU, so sobelX is actually the combined X and Y
	unsigned char *sobelY = (unsigned char*)memalign(128,imageSize);
	sobelDMA.inputData = (void *)gaussData;
	sobelDMA.outputX = (void *)sobelX;
	sobelDMA.outputY = (void *)sobelY;
	sobelDMA.imageWidth = imageWidth;
	initSPUs(4, ptdata, pthread, sobelDMA, sobel_spu);
	destroySPUs(4, ptdata, pthread);
	
	//Pad back to Original Size and fill the sides
	imageSize = imageWidth * imageHeight * imageBytes;
	unsigned char* paddedImage = new unsigned char[imageSize];
	paddedImage = imagePro.padOutImage(sobelX, imageWidth, imageHeight, imageBytes);

	imageLoader.saveImage("Apples/gala-grey-gauss2.png", paddedImage, imageSize);
	delete[] gaussData;
	//delete[] sobelData;
	//delete[] sobelX;
	//delete[] sobelY;
	
	return 0;
}