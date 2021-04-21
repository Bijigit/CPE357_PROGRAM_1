#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "bitmapHeaders.h"

#define RED 0
#define GREEN 1
#define BLUE 2

int main(int argc, char *argv[]){

    //checks for the correct number of arguments
    if(argc != 5){
        printf("Incorrect number of arguments.  Please follow the format:\n[programname] [imagefile1] [imagefile2] [ratio] [outputfile]\n");
        return 0;
    }
    //checks to make sure the ratio is in the correct range, and that a number was entered
    double blendRatio = atof(argv[3]);
    if(blendRatio <= 0 || blendRatio >= 1){
        printf("Please use a valid ratio such that 0 < ratio < 1\n");
        return 0;
    }
    
    //open both images in binary read mode
    FILE *fp = fopen(argv[1], "rb");
    FILE *fp2 = fopen(argv[2], "rb");
    FILE *outfp = fopen(argv[4], "wb");

    //checks to see if actual files were provided
    if(fp == NULL || fp2 == NULL){
        printf("Invalid argument. Please make sure your image file names are correct.\n");
        return 0;
    }

    //Checks the first two bytes of each input file to confirm that they are both *.BMP files.
    char firstFileType[3];
    char secondFileType[3];
    unsigned char correctType[] = "BM";
    fread(firstFileType, sizeof(unsigned char), 2, fp);
    fread(secondFileType, sizeof(unsigned char), 2, fp2);
    fseek(fp, 0, SEEK_SET);
    fseek(fp2, 0, SEEK_SET);
    if(strcmp(firstFileType, correctType) + strcmp(secondFileType, correctType) != 0){
        printf("This program only accepts *.BMP files. Please revise your arguments.\n");
        return 0;
    }

    //initialize image and file headers for both images
    BITMAPFILEHEADER fileHeader;
    BITMAPIMAGEHEADER imageHeader;
    BITMAPFILEHEADER secondFileHeader;
    BITMAPIMAGEHEADER secondImageHeader;

    //holds size of the image data
    int secondImageSize;
    int imageSize;

    //loads header data into its respective structure
    populateFileHeader(&fileHeader, fp);
    populateImageHeader(&imageHeader, fp);
    populateFileHeader(&secondFileHeader, fp2);
    populateImageHeader(&secondImageHeader, fp2);

    //get the size of the image data for each picture
    imageSize = getImageSize(&imageHeader);
    secondImageSize = getImageSize(&secondImageHeader);

    //loads image data into a byte array
    unsigned char *imageData = malloc(imageSize);
    getImageData(&fileHeader, fp, imageData, imageSize);
    unsigned char *secondImageData = malloc(secondImageSize);
    getImageData(&secondFileHeader, fp2, secondImageData, secondImageSize);
    
    //initialize a character array to hold the full header to be written to the output file
    //header is determined by which image has a larger size, and defaults to the first input if they are equal sizes
    unsigned char *fullHeader;

    //Determines which image has the larger dimensions and then calls the method to interpolate
    //fills the fullHeader array with the correct header
    if(imageSize == secondImageSize){
        interpolateSameSize(imageData, secondImageData, &imageHeader, &secondImageHeader, blendRatio, imageSize);
        fullHeader = malloc(fileHeader.bfOffBits);
        getImageHeader(&fileHeader, fp, fullHeader);
    }else if (imageSize > secondImageSize){
        interpolate(imageData, secondImageData, &imageHeader, &secondImageHeader, blendRatio);
        fullHeader = malloc(fileHeader.bfOffBits);
        getImageHeader(&fileHeader, fp, fullHeader);
    }else{
        interpolate(secondImageData, imageData, &secondImageHeader, &imageHeader, 1 - blendRatio);
        fullHeader = malloc(secondFileHeader.bfOffBits);
        getImageHeader(&secondFileHeader, fp2, fullHeader);
    }

    //close and free all resources
    fwrite(fullHeader, fileHeader.bfOffBits, 1, outfp);    
    fwrite(imageData, imageSize, 1, outfp);
    fclose(outfp);
    fclose(fp2);
    fclose(fp);
    free(secondImageData);
    free(imageData);
    free(fullHeader);
    return 0;
}

//combines images of the same dimensions
void interpolateSameSize(unsigned char *firstData, unsigned char *secondData, BITMAPIMAGEHEADER *firstHeader, BITMAPIMAGEHEADER *secondHeader, double blendRatio, int imageSize){
    for(int i = 0; i < imageSize; i++){
        firstData[i] = (unsigned char)((int)(blendRatio * (int)firstData[i] + (1 - blendRatio) * (int)secondData[i]));
    }
}

//combines images of different dimensions
void interpolate(unsigned char *largerData, unsigned char *smallerData, BITMAPIMAGEHEADER *largerHeader, BITMAPIMAGEHEADER *smallerHeader, double blendRatio){
    
    int largerWidth = largerHeader->biWidth;
    int smallerWidth = smallerHeader->biWidth;

    double heightRatio = (double)(smallerHeader->biHeight) / (double)(largerHeader->biHeight);
    double widthRatio = (double)(smallerWidth) / (double)(largerWidth);

    //combines the pixels from the two images left to right, bottom to top
    for(int y = 0; y < largerHeader->biHeight; y++){
        for(int x = 0; x < largerWidth; x++){
            int index = ((y * largerWidth) + x) * 3;

            //finds the larger picture coordinates of the corner of a pixel from the smaller picture
            double smallY = y * heightRatio;
            double smallX = x * widthRatio;
            int adjY = (int)smallY;
            int adjX = (int)smallX - ((int)smallX % 3);
            double cornerX = (adjX + 1) / widthRatio;
            double cornerY = (adjY + 1) / heightRatio;

            //calculates the weights for each color to be interpolated
            double dx = cornerX - x;
            double dy = cornerY - y;

            //handles the case where we are at the right or upper boundary of the image and need the next position's color to interpolate
            //the next position's color will always be the same as the previous position's color when on the edge of an image in the direction of whatever edge you are on
            //ex: you are on the upper boundary of the larger image.  The last vertical position, and the second to last vertical position will alway have the same color
            //on the smaller image since the smaller image's pixels scale up to meet the larger dimensions
            int nextX;
            int nextY;
            if(x + 1 == largerWidth){
                nextX = x;
            }else{
                nextX = x + 1;
            }
            if(y + 1 == largerHeader->biHeight){
                nextY = y;
            }else{
                nextY = y + 1;
            }

            //actually changes the colors in the larger image's data
            int finalRed = getFinalIntensity(smallerData, x, y, smallerWidth, RED, heightRatio, widthRatio, dx, dy, nextX, nextY);
            int finalGreen = getFinalIntensity(smallerData, x, y, smallerWidth, GREEN, heightRatio, widthRatio, dx, dy, nextX, nextY);
            int finalBlue = getFinalIntensity(smallerData, x, y, smallerWidth, BLUE, heightRatio, widthRatio, dx, dy, nextX, nextY);
            largerData[index + RED] = (unsigned char)((int)(finalRed * (1 - blendRatio) + (int)largerData[index + RED] * blendRatio));
            largerData[index + GREEN] = (unsigned char)((int)(finalGreen * (1 - blendRatio) + (int)largerData[index + GREEN] * blendRatio));
            largerData[index + BLUE] = (unsigned char)((int)(finalBlue * (1 - blendRatio) + (int)largerData[index + BLUE] * blendRatio));
        }
    }

}

//gets the bilinearly interpolated intensity of either R, G, or B from the smaller image given an x, y coordinate from the larger image
int getFinalIntensity(unsigned char *imageData, int x, int y, int width, int colorOffset, double heightRatio, double widthRatio, double dx, double dy, int nextX, int nextY){
    //gets the color intensity of an R, G , or B value at all 4 corners of the current pixel's position
    int ul = getColorIntensity(imageData, x, nextY, width, colorOffset, heightRatio, widthRatio);
    int bl = getColorIntensity(imageData, x, y, width, colorOffset, heightRatio, widthRatio);
    int ur = getColorIntensity(imageData, nextX, nextY, width, colorOffset, heightRatio, widthRatio);
    int br = getColorIntensity(imageData, nextX, y, width, colorOffset, heightRatio, widthRatio);

    //interpolates using the ratio calculated by the caller returns the final value
    int left = (int)(ul * (1-dy) + bl * dy);
    int right = (int)(ur * (1 - dy) + bl * dy);
    return (int)(left * dx + right * (1 - dx));
}

//gets the intensity of an R, G, or B value of a pixel on the smaller image given a corresponding x, y coordinate on the larger image
int getColorIntensity(unsigned char *imageData, int x, int y, int width, int colorOffset, double heightRatio, double widthRatio){
    //adjusts x, y coordinates from the larger image to x, y coordinates on the smaller image
    double smallY = y * heightRatio;
    double smallX = x * widthRatio;
    int adjY = (int)smallY;
    int adjX = (int)smallX - ((int)smallX % 3);

    //number of padding bytes added to the end of each line
    int widthCorrection = width % sizeof(int);

    int index = ((adjY * width) + adjX) * 3 + (adjY * widthCorrection) + colorOffset;
    return (int)(imageData[index]);
}

int getImageSize(BITMAPIMAGEHEADER *imageHeader){
    if(imageHeader->biSizeImage == 0){
        return imageHeader->biWidth * imageHeader->biHeight * ((int)(((double)imageHeader->biBitCount) / 8));
    }else{
        return imageHeader->biSizeImage;
    }
}

//reads the image data into an unsigned char array
void getImageData(BITMAPFILEHEADER *fileHeader, FILE *fp, unsigned char *imageData, int imageSize){
    fseek(fp, fileHeader->bfOffBits, SEEK_SET);
    fread(imageData, imageSize, 1, fp);
}

//reads an imageHeader into an unsigned char array
void getImageHeader(BITMAPFILEHEADER *fileHeader, FILE *fp, unsigned char *fullHeader){
    fseek(fp, 0, SEEK_SET);
    fread(fullHeader, fileHeader->bfOffBits, 1, fp);
}

//populates the file header fields individually to handle padding issues
void populateFileHeader(BITMAPFILEHEADER *header, FILE *fp){
    fread(&(header->bfType), sizeof(header->bfType), 1, fp);
    fread(&(header->bfSize), sizeof(header->bfSize), 1, fp);
    fread(&(header->bfReserved1), sizeof(header->bfReserved1), 1, fp);
    fread(&(header->bfReserved2), sizeof(header->bfReserved2), 1, fp);
    fread(&(header->bfOffBits), sizeof(header->bfOffBits), 1, fp);
}

//populates the file header fields individually to handle padding issues
void populateImageHeader(BITMAPIMAGEHEADER *imageHeader, FILE *fp){
    fread(&(imageHeader->biSize), sizeof(imageHeader->biSize), 1, fp);
    fread(&(imageHeader->biWidth), sizeof(imageHeader->biWidth), 1, fp);
    fread(&(imageHeader->biHeight), sizeof(imageHeader->biHeight), 1, fp);
    fread(&(imageHeader->biPlanes), sizeof(imageHeader->biPlanes), 1, fp);
    fread(&(imageHeader->biBitCount), sizeof(imageHeader->biBitCount), 1, fp);
    fread(&(imageHeader->biCompression), sizeof(imageHeader->biCompression), 1, fp);
    fread(&(imageHeader->biSizeImage), sizeof(imageHeader->biSizeImage), 1, fp);
    fread(&(imageHeader->biXPelsPerMeter), sizeof(imageHeader->biXPelsPerMeter), 1, fp);
    fread(&(imageHeader->biYPelsPerMeter), sizeof(imageHeader->biYPelsPerMeter), 1, fp);
    fread(&(imageHeader->biClrUsed), sizeof(imageHeader->biClrUsed), 1, fp);
    fread(&(imageHeader->biClrImportant), sizeof(imageHeader->biClrImportant), 1, fp);
}


