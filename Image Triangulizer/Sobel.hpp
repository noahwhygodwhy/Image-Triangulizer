#pragma once

#ifndef SOBEL_H
#define SOBEL_H

#include <cmath>
#include <algorithm>

using namespace std;

float sobelX[3][3] = { {1.0, 0.0, -1.0},{2.0, 0.0, -2.0}, {1.0, 0.0, -1.0} };
float sobelY[3][3] = { {1.0, 2.0, 1.0},{0.0, 0.0, 0.0}, {-1.0, -2.0, -1.0} };
float gaussianBlurr[3][3] = { {1.0, 2.0, 1.0},{2.0, 4.0, 2.0}, {1.0, 2.0, 1.0} };




//gaussian blurs a specific channel 
void blurChannel(unsigned char* data, int width, int height, int nrChannels, int colorOffset)
{
    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width; x++)
        {
            int pixelOffset = ((y * width) + x) * nrChannels;
            float sum = 0;
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    int yAdjust = (i - 1) * (width * nrChannels);
                    int xAdjust = (j - 1) * (nrChannels);
                    sum += gaussianBlurr[i][j] * *(data + pixelOffset + yAdjust + xAdjust + colorOffset);
                }
            }
            *(data + pixelOffset + colorOffset) = sum / 16.0f;
        }
    }
}

//blurs all channels
void blur(unsigned char* data, int width, int height, int nrChannels)
{
    for (int i = 0; i < nrChannels; i++)
    {
        blurChannel(data, width, height, nrChannels, i);
    }
}

//applies a specific (x or y) sobel operator to the image data and returns the value
double sobel(float sobelMat[3][3], unsigned char* imageData, int width, int height, int nrChannels, int pixelOffset, int colorOffset)
{
    int sum = 0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int yAdjust = (i - 1) * (width * nrChannels);
            int xAdjust = (j - 1) * (nrChannels);
            sum += sobelMat[i][j] * *(imageData + pixelOffset + yAdjust + xAdjust + colorOffset);
        }
    }
    return (double)sum;
}


//returns the pixel data for an image that represents the change in color between pixels.
//the sharper the change in color, the more white the pixel int he sobal image.
//this is used as edge detection. The brigher the point, the more likely it's an edge in the image.
unsigned char* makeSobalImage(unsigned char* originalImage, int width, int height, int nrChannels)
{
    unsigned char* sobelImage = new unsigned char[(width * height * nrChannels) + 1];

    //applies a gaussian blur to reduce noise 
    blur(originalImage, width, height, nrChannels);
    

    int sumX = 0;
    int sumY = 0;
    int sum;
    int maxSobalVal = 0;



    //applies the sobel operator to the original image
    //and puts the result in the sobelImage
    for (int i = 0; i < width * height; i++)
    {
        int pixelOffset = i * nrChannels;
        int y = i / width;
        int x = i % height;


        if (x == 0 || y == 0 || x == width - 1 || y == height - 1)
        {
            *(sobelImage + pixelOffset + 0) = 0;
            *(sobelImage + pixelOffset + 1) = 0;
            *(sobelImage + pixelOffset + 2) = 0;
            *(sobelImage + pixelOffset + 3) = 255;
        }
        else
        {
            double rsx = sobel(sobelX, originalImage, width, height, nrChannels, pixelOffset, 0);
            double rsy = sobel(sobelY, originalImage, width, height, nrChannels, pixelOffset, 0);
            double rrealSum = sqrt((rsx * rsx) + (rsy * rsy));

            double gsx = sobel(sobelX, originalImage, width, height, nrChannels, pixelOffset, 1);
            double gsy = sobel(sobelY, originalImage, width, height, nrChannels, pixelOffset, 1);
            double grealSum = sqrt((gsx * gsx) + (gsy * gsy));

            double bsx = sobel(sobelX, originalImage, width, height, nrChannels, pixelOffset, 2);
            double bsy = sobel(sobelY, originalImage, width, height, nrChannels, pixelOffset, 2);
            double brealSum = sqrt((bsx * bsx) + (bsy * bsy));

            int avgSum = (int)((rrealSum + grealSum + brealSum) / 3);

            maxSobalVal = std::max(maxSobalVal, avgSum);

            for (int colorOffset = 0; colorOffset < 3; colorOffset++)
            {
                *(sobelImage + pixelOffset + colorOffset) = avgSum/4;
            }
            *(sobelImage + pixelOffset + 3) = 255;

        }
    }


    //these four for loops make all the edge pixels white so they are likely to get a vertex 
    for (int i = 0; i < width; i++)
    {
        int pixelOffset = i * nrChannels;
        *(sobelImage + pixelOffset + 0) = 255;
        *(sobelImage + pixelOffset + 1) = 255;
        *(sobelImage + pixelOffset + 2) = 255;
    }
    for (int i = width*(height-1); i < width*height; i++)
    {
        int pixelOffset = i * nrChannels;
        *(sobelImage + pixelOffset + 0) = 255;
        *(sobelImage + pixelOffset + 1) = 255;
        *(sobelImage + pixelOffset + 2) = 255;
    }
    for (int i = 0; i < width * (height-1); i+= width)
    {
        int pixelOffset = i * nrChannels;
        *(sobelImage + pixelOffset + 0) = 255;
        *(sobelImage + pixelOffset + 1) = 255;
        *(sobelImage + pixelOffset + 2) = 255;
    }
    for (int i = width-1; i < width * height; i += width)
    {
        int pixelOffset = i * nrChannels;
        *(sobelImage + pixelOffset + 0) = 255;
        *(sobelImage + pixelOffset + 1) = 255;
        *(sobelImage + pixelOffset + 2) = 255;
    }
    
    return sobelImage;
}
#endif