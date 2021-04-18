#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bitmapHeaders.h"

int main(int argc, char *argv[]){

    char image1[] = "lion.bmp";
    char image2[] = "wolf.bmp";
    BITMAPFILEHEADER fileHeader;
    BITMAPIMAGEHEADER imageHeader;
    BITMAPFILEHEADER secondFileHeader;
    BITMAPIMAGEHEADER secondImageHeader;
    int secondImageSize;
    int imageSize;
    FILE *fp = fopen(image1, "rb");
    FILE *fp2 = fopen(image2, "rb");
    populateFileHeader(&fileHeader, fp);
    populateImageHeader(&imageHeader, fp);
    populateFileHeader(&secondFileHeader, fp2);
    populateImageHeader(&secondFileHeader, fp2);
    
    if(imageHeader.biSizeImage == 0){
        imageSize = imageHeader.biWidth * imageHeader.biHeight * ((int)((double)imageHeader.biBitCount) / 8);
    }else{
        imageSize = imageHeader.biSizeImage;
    }
    if(secondImageHeader.biSizeImage == 0){
        secondImageSize = secondImageHeader.biWidth * secondImageHeader.biHeight * ((int)((double)secondImageHeader.biBitCount) / 8);
    }else{
        secondImageSize = secondImageHeader.biSizeImage;
    }

    char *imageData = malloc(imageSize);
    char *fullHeader = malloc(secondFileHeader.bfOffBits);
    getImageData(&fileHeader, fp, imageData, imageSize);
    getImageHeader(&fileHeader, fp, fullHeader);
    fclose(fp);

    char *secondImageData = malloc(secondImageSize);
    char *secondFullHeader = malloc(secondFileHeader.bfOffBits);
    getImageData(&secondFileHeader, fp2, secondImageData, secondImageSize);
    getImageHeader(&secondFileHeader, fp2, secondFullHeader);
    fclose(fp2);

    char firstOutput[] = "output.bmp";
    FILE *outfp = fopen(firstOutput, "wb");
    fwrite(fullHeader, fileHeader.bfOffBits, 1, outfp);    
    fwrite(imageData, imageSize, 1, outfp);
    fclose(outfp);
    
    free(imageData);
    free(fullHeader);
    return 0;
}

void getImageData(BITMAPFILEHEADER *fileHeader, FILE *fp, char *imageData, int imageSize){
    fseek(fp, fileHeader->bfOffBits, SEEK_SET);
    fread(imageData, imageSize, 1, fp);
}

void getImageHeader(BITMAPFILEHEADER *fileHeader, FILE *fp, char *fullHeader){
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

