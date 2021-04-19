#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "bitmapHeaders.h"

#define RED 0
#define GREEN 1
#define BLUE 2

int main(int argc, char *argv[]){

    char image1[] = "lion.bmp";
    char image2[] = "flowers.bmp";
    double blendRatio = 0.5;

    //initialize image and file headers for both images
    BITMAPFILEHEADER fileHeader;
    BITMAPIMAGEHEADER imageHeader;
    BITMAPFILEHEADER secondFileHeader;
    BITMAPIMAGEHEADER secondImageHeader;

    //holds size of the image data
    int secondImageSize;
    int imageSize;

    //open both images in binary read mode
    FILE *fp = fopen(image1, "rb");
    FILE *fp2 = fopen(image2, "rb");

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
    unsigned char *secondFullHeader = malloc(secondFileHeader.bfOffBits);
    getImageData(&secondFileHeader, fp2, secondImageData, secondImageSize);
    getImageHeader(&secondFileHeader, fp2, secondFullHeader);
    fclose(fp2);

    //TODO: replace this with a method that reads the header directly from the origional file into the new output file
    unsigned char *fullHeader = malloc(secondFileHeader.bfOffBits);
    getImageHeader(&fileHeader, fp, fullHeader);
    fclose(fp);

    //Determines which image has the larger dimensions and then calls the method to interpolate
    if(imageSize == secondImageSize){
        interpolateSameSize(imageData, secondImageData, &imageHeader, &secondImageHeader, blendRatio, imageSize);
    }else if (imageSize > secondImageSize){
        interpolate(imageData, secondImageData, &imageHeader, &secondImageHeader, blendRatio);
    }else{
        interpolate(secondImageData, imageData, &secondImageHeader, &imageHeader, 1 - blendRatio);
    }

    char firstOutput[] = "output.bmp";
    FILE *outfp = fopen(firstOutput, "wb");
    fwrite(fullHeader, fileHeader.bfOffBits, 1, outfp);    
    fwrite(imageData, imageSize, 1, outfp);
    fclose(outfp);
    
    free(secondImageData);
    free(secondFullHeader);
    free(imageData);
    free(fullHeader);
    return 0;
}

void interpolateSameSize(unsigned char *firstData, unsigned char *secondData, BITMAPIMAGEHEADER *firstHeader, BITMAPIMAGEHEADER *secondHeader, double blendRatio, int imageSize){
    for(int i = 0; i < imageSize; i++){
        firstData[i] = (unsigned char)((int)(blendRatio * (int)firstData[i] + (1 - blendRatio) * (int)secondData[i]));
    }
}

void interpolate(unsigned char *largerData, unsigned char *smallerData, BITMAPIMAGEHEADER *largerHeader, BITMAPIMAGEHEADER *smallerHeader, double blendRatio){
    //adjusts width for null pixels
    int largerWidth = largerHeader->biWidth;// + (largerHeader->biWidth % sizeof(int));
    int smallerWidth = smallerHeader->biWidth;// + (smallerHeader->biWidth % sizeof(int));

    double heightRatio = (double)(smallerHeader->biHeight) / (double)(largerHeader->biHeight);
    double widthRatio = (double)(smallerWidth) / (double)(largerWidth);

    for(int y = 0; y < largerHeader->biHeight; y++){
        for(int x = 0; x < largerWidth; x++){
            int index = ((y * largerWidth) + x) * 3;
            /*
            double smallerY = heightRatio * y;
            double smallerX = widthRatio * x;
            int adjY = (int)smallerY;
            int adjX = (int)smallerX - ((int)smallerX % 3);
            */

            double smallY = y * heightRatio;
            double smallX = x * widthRatio;

            int adjY = (int)smallY;
            int adjX = (int)smallX - ((int)smallX % 3);

            double cornerX = (adjX + 1) / widthRatio;
            double cornerY = (adjY + 1) / heightRatio;

            double dx = cornerX - x;
            double dy = cornerY - y;

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
            
            int redul = getColorIntensity(smallerData, x, nextY, smallerWidth, RED, heightRatio, widthRatio);
            int redbl = getColorIntensity(smallerData, x, y, smallerWidth, RED, heightRatio, widthRatio);
            int redur = getColorIntensity(smallerData, nextX, nextY, smallerWidth, RED, heightRatio, widthRatio);
            int redbr = getColorIntensity(smallerData, nextX, y, smallerWidth, RED, heightRatio, widthRatio);

            int redLeft = (int)(redul * (1-dy) + redbl * dy);
            int redRight = (int)(redur * (1 - dy) + redbl * dy);
            int finalRed = (int)(redLeft * dx + redRight * (1 - dx));
            largerData[index + RED] = (unsigned char)((int)(finalRed * (1 - blendRatio) + (int)largerData[index + RED] * blendRatio));

            int greenul = getColorIntensity(smallerData, x, nextY, smallerWidth, GREEN, heightRatio, widthRatio);
            int greenbl = getColorIntensity(smallerData, x, y, smallerWidth, GREEN, heightRatio, widthRatio);
            int greenur = getColorIntensity(smallerData, nextX, nextY, smallerWidth, GREEN, heightRatio, widthRatio);
            int greenbr = getColorIntensity(smallerData, nextX, y, smallerWidth, GREEN, heightRatio, widthRatio);

            int greenLeft = (int)(greenul * (1 - dy) + greenbl * dy);
            int greenRight = (int)(greenur * (1 - dy) + greenbl * dy);
            int finalGreen = (int)(greenLeft * dx + greenRight * (1 - dx));
            largerData[index + GREEN] = (unsigned char)((int)(finalGreen * (1 - blendRatio) + (int)largerData[index + GREEN] * blendRatio));

            int blueul = getColorIntensity(smallerData, x, nextY, smallerWidth, BLUE, heightRatio, widthRatio);
            int bluebl = getColorIntensity(smallerData, x, y, smallerWidth, BLUE, heightRatio, widthRatio);
            int blueur = getColorIntensity(smallerData, nextX, nextY, smallerWidth, BLUE, heightRatio, widthRatio);
            int bluebr = getColorIntensity(smallerData, nextX, y, smallerWidth, BLUE, heightRatio, widthRatio);

            int blueLeft = (int)(blueul * (1-dy) + bluebl * dy);
            int blueRight = (int)(blueur * (1 - dy) + bluebl * dy);
            int finalBlue = (int)(blueLeft * dx + blueRight * (1 - dx));
            largerData[index + BLUE] = (unsigned char)((int)(finalBlue * (1 - blendRatio) + (int)largerData[index + BLUE] * blendRatio));
        }
    }

}

int getColorIntensity(unsigned char *imageData, int x, int y, int width, int colorOffset, double heightRatio, double widthRatio){
    double smallY = y * heightRatio;
    double smallX = x * widthRatio;

    int adjY = (int)smallY;
    int adjX = (int)smallX - ((int)smallX % 3);

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

void getImageData(BITMAPFILEHEADER *fileHeader, FILE *fp, unsigned char *imageData, int imageSize){
    fseek(fp, fileHeader->bfOffBits, SEEK_SET);
    fread(imageData, imageSize, 1, fp);
}

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


