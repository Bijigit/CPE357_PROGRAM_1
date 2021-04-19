typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

typedef struct tagBITMAPFILEHEADER{
    WORD bfType; //specifies the file type
    DWORD bfSize; //specifies the size in bytes of the bitmap file
    WORD bfReserved1; //reserved; must be 0
    WORD bfReserved2; //reserved; must be 0
    DWORD bfOffBits; //species the offset in bytes from the bitmapfileheader to the bitmap bits
}BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
    DWORD biSize; //specifies the number of bytes required by the struct
    LONG biWidth; //specifies width in pixels
    LONG biHeight; //species height in pixels
    WORD biPlanes; //specifies the number of color planes, must be 1
    WORD biBitCount; //specifies the number of bit per pixel
    DWORD biCompression;//spcifies the type of compression
    DWORD biSizeImage; //size of image in bytes
    LONG biXPelsPerMeter; //number of pixels per meter in x axis
    LONG biYPelsPerMeter; //number of pixels per meter in y axis
    DWORD biClrUsed; //number of colors used by th ebitmap
    DWORD biClrImportant; //number of colors that are important
}BITMAPIMAGEHEADER;

void populateFileHeader(BITMAPFILEHEADER *header, FILE *fp);
void populateImageHeader(BITMAPIMAGEHEADER *imageHeader, FILE *fp);
void getImageData(BITMAPFILEHEADER *fileHeader, FILE *fp, unsigned char *imageData, int imageSize);
void getImageHeader(BITMAPFILEHEADER *fileHeader, FILE *fp, unsigned char *fullHeader);
int getImageSize(BITMAPIMAGEHEADER *imageHeader);
void interpolate(unsigned char *largerData, unsigned char *smallerData, BITMAPIMAGEHEADER *largerHeader, BITMAPIMAGEHEADER *smallerHeader, double blendRatio);
int getColorIntensity(unsigned char *imageData, int x, int y, int width, int colorOffset, double heightRatio, double widthRatio);
void interpolateSameSize(unsigned char *firstData, unsigned char *secondData, BITMAPIMAGEHEADER *firstHeader, BITMAPIMAGEHEADER *secondHeader, double blendRatio, int imageSize);