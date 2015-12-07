#include "imageIO.h"

int imageIO::loadPNG(const char* filename, std::vector<unsigned char> &imageBuffer)
{
	std::vector<unsigned char> rawPixels;
	std::string tmp = filename;
	//load and decode
	unsigned int error = 0;
	lodepng::load_file(rawPixels, filename);
	error = lodepng::decode(imageBuffer, imageWidth, imageHeight, rawPixels);

	if (error)
	{
		std::cout << "Error: " << lodepng_error_text(error) << std::endl;
	}
	else {
		printf("%s \n", "File Read Successful");
	}

	imageBytes = 4;
	size = imageWidth * imageHeight * imageBytes;
	return 0;
}

int imageIO::savePNG(const char* filename, std::vector<unsigned char> &imageBuffer)
{

	std::vector<unsigned char> rawPixels;
	//encode and save
	unsigned int error = 0;

	error = lodepng::encode(rawPixels, imageBuffer, imageWidth, imageHeight);

	if (error)
	{
		std::cout << "Encoding error: " << lodepng_error_text(error) << std::endl;
	}
	else {
		printf("%s", "Saved to ");
		printf("%s \n", filename);
	}

	lodepng::save_file(rawPixels, filename);
	return 0;
}

unsigned char* imageIO::readBMP(const char* filename) {
	FILE* f = fopen(filename, "rb");
	if (f == NULL)
	{
		printf("%s \n", "File Not Loaded");
	}

	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); //read the 54-byte header

	//extract image height and width from header
	imageWidth = getIntLE(&info[18]);
	imageHeight = getIntLE(&info[22]);
	imageBytes = 3;
	size = imageWidth * imageHeight * imageBytes;

	unsigned char* readData = new unsigned char[size];	//allocate 3 byte per pixel
	fread(readData, sizeof(unsigned char), size, f); //read the rest of the data at once
	if (readData == NULL){
		printf("%s \n", "File Not Read");
	}
	else ("%s \n", "File Read Successfully");
	fclose(f);
	
	return readData;
}

void imageIO::saveBMP(const char* filename, int WIDTH, int HEIGHT, unsigned char dataArray[]) {

	unsigned int headers[13];
	FILE * outfile;
	int extrabytes;
	int paddedsize;
	int x; int y; int n;
	int red, green, blue;

	extrabytes = 4 - ((WIDTH * 3) % 4);                 // How many bytes of padding to add to each
														// horizontal line - the size of which must
														// be a multiple of 4 bytes.
	if (extrabytes == 4)
		extrabytes = 0;

	paddedsize = ((WIDTH * 3) + extrabytes) * HEIGHT;

	// Headers...
	// Note that the "BM" identifier in bytes 0 and 1 is NOT included in these "headers".

	headers[0] = paddedsize + 54;      // bfSize (whole file size)
	headers[1] = 0;                    // bfReserved (both)
	headers[2] = 54;                   // bfOffbits
	headers[3] = 40;                   // biSize
	headers[4] = WIDTH;  // biWidth
	headers[5] = HEIGHT; // biHeight

						 // Would have biPlanes and biBitCount in position 6, but they're shorts.
						 // It's easier to write them out separately (see below) than pretend
						 // they're a single int, especially with endian issues...

	headers[7] = 0;                    // biCompression
	headers[8] = paddedsize;           // biSizeImage
	headers[9] = 0;                    // biXPelsPerMeter
	headers[10] = 0;                    // biYPelsPerMeter
	headers[11] = 0;                    // biClrUsed
	headers[12] = 0;                    // biClrImportant

	outfile = fopen(filename, "wb");

	//
	// Headers begin...
	// When printing ints and shorts, we write out 1 character at a time to avoid endian issues.
	//

	fprintf(outfile, "BM");

	for (n = 0; n <= 5; n++)
	{
		fprintf(outfile, "%c", headers[n] & 0x000000FF);
		fprintf(outfile, "%c", (headers[n] & 0x0000FF00) >> 8);
		fprintf(outfile, "%c", (headers[n] & 0x00FF0000) >> 16);
		fprintf(outfile, "%c", (headers[n] & (unsigned int)0xFF000000) >> 24);
	}

	// These next 4 characters are for the biPlanes and biBitCount fields.

	fprintf(outfile, "%c", 1);
	fprintf(outfile, "%c", 0);
	fprintf(outfile, "%c", 24);
	fprintf(outfile, "%c", 0);

	for (n = 7; n <= 12; n++)
	{
		fprintf(outfile, "%c", headers[n] & 0x000000FF);
		fprintf(outfile, "%c", (headers[n] & 0x0000FF00) >> 8);
		fprintf(outfile, "%c", (headers[n] & 0x00FF0000) >> 16);
		fprintf(outfile, "%c", (headers[n] & (unsigned int)0xFF000000) >> 24);
	}

	//
	// Headers done, now write the data...
	//

	for (int i = 0; i <= HEIGHT -1; i++){
		for (int j = 0; j <= WIDTH -1; j++){

			red = dataArray[i * WIDTH + j];
			green = dataArray[i * WIDTH + j];
			blue = dataArray[i * WIDTH + j];

			// Also, it's written in (b,g,r) format...

			fprintf(outfile, "%c", blue);
			fprintf(outfile, "%c", green);
			fprintf(outfile, "%c", red);
		}
		if (extrabytes)      // See above - BMP lines must be of lengths divisible by 4.
		{
			for (n = 1; n <= extrabytes; n++)
			{
				fprintf(outfile, "%c", 0);
			}
		}
	}

	fclose(outfile);
	printf("%s", "Saved to ");
	printf("%s \n", filename);

}

unsigned char* imageIO::openImage(const char* filename) {
	std::string strFilename = std::string(filename);
	printf("%s", "Opening File: ");
	printf("%s \n", filename);

	if (strFilename.substr(strFilename.length() - 4) == ".bmp") {
		unsigned char* readData = readBMP(filename);
		return readData;
	}
	else if (strFilename.substr(strFilename.length() - 4) == ".png") {
		std::vector <unsigned char> buffer;
		loadPNG(filename, buffer);

		unsigned char* readData = new unsigned char[buffer.size()];
		memcpy(readData, buffer.data(), buffer.size());
		return readData;
	}
}

void imageIO::saveImage(const char* filename, unsigned char dataArray[], int size) {
	std::string strFilename = std::string(filename);
	std::vector <unsigned char> buffer;

	printf("%s", "Saving File as: ");
	printf("%s \n", filename);

	if (strFilename.substr(strFilename.length() - 4) == ".bmp") {
		saveBMP(filename, imageWidth, imageHeight, dataArray);
	}
	else if (strFilename.substr(strFilename.length() - 4) == ".png") {

		buffer.insert(buffer.begin(), dataArray, dataArray + size);
		savePNG(filename, buffer);
	}
}

unsigned int imageIO::getImageBytes() {
	return imageBytes;
}

unsigned int imageIO::getImageWidth() {
	return imageWidth;
}

unsigned int imageIO::getImageHeight() {
	return imageHeight;
}

unsigned int imageIO::getSize() {
	return size;
}

void imageIO::setImageBytes(int x) {
	imageBytes = x;
}

void imageIO::setImageWidth(int width) {
	imageWidth = width;
}

void imageIO::setImageHeight(int height) {
	imageHeight = height;
}

unsigned int imageIO::getIntLE(const unsigned char *p) {
//#if LITTLE_ENDIAN
    //return *(unsigned int *)p;
//#else
    return ((unsigned int)p[3] << 24) | 
           ((unsigned int)p[2] << 16) | 
           ((unsigned int)p[1] << 8) | 
           p[0];
//#endif
}

int imageIO::isPowerOfTwo (unsigned int x)
{
  return ((x != 0) && !(x & (x - 1)));
}

unsigned char* imageIO::squareImage(unsigned char origImage[], int imageWidth, int imageHeight, int imageBytes) {
	int newImageWidth = 0;
	int newImageHeight = 0;

	if (std::max(imageWidth, imageHeight) == imageWidth) {
		if (!isPowerOfTwo(imageWidth)) {
			newImageWidth = pow(2, ceil(log(imageWidth) / log(2)));
			newImageHeight = newImageWidth;
		}
		else {
			newImageWidth = imageWidth;
			newImageHeight = imageWidth;
		}
	}
	else {
		if (!isPowerOfTwo(imageHeight)) {
			newImageHeight = pow(2, ceil(log(imageHeight) / log(2)));;
			newImageWidth = newImageHeight;
		}
		else {
			newImageHeight = imageHeight;
			newImageWidth = imageHeight;
		}
	}
	
	unsigned char *squareImage = (unsigned char*)memalign(128,newImageWidth*newImageHeight*imageBytes);

	for (int y = 0; y < newImageHeight; y++) {
		for (int x = 0; x < newImageWidth; x++) {
			int oldIndex = ((y * imageWidth * imageBytes) + (x * imageBytes));
			int newIndex = ((y * newImageWidth * imageBytes) + (x * imageBytes));

			if (x < imageWidth && y < imageHeight) {
				squareImage[newIndex] = origImage[oldIndex];
				squareImage[newIndex + 1] = origImage[oldIndex + 1];
				squareImage[newIndex + 2] = origImage[oldIndex + 2];
				squareImage[newIndex + 3] = origImage[oldIndex + 3];
			}
			else {
				squareImage[newIndex] = 255;
				squareImage[newIndex + 1] = 255;
				squareImage[newIndex + 2] = 255;
				squareImage[newIndex + 3] = 255;
			}
		}
	}

	setImageWidth(newImageWidth);
	setImageHeight(newImageHeight);
	
	return squareImage;
}