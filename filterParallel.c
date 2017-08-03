#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "filterParallel.h"
#include "bmp.h"

Pixel calculate(BMPImage * image, int i, int j, BoxFilter f)
{
	Pixel newPixel;
	newPixel.alpha = 255;
	int blue = 0;
	int green = 0;
	int red = 0;

	int m, n;
	for(m = -1; m <= 1; m++)
	{
		for(n = -1; n <= 1; n++)
		{
			int p = i + m;
			int q = j + n;

			if(p < 0) 
			{p = 0;}
			
			if(q < 0) 
			{q = 0;}

			if(p >= image->norm_height) 
			{p = image->norm_height - 1;}

			if(q >= image->header.width_px) 
			{q = image->header.width_px - 1;}

			blue += (image->pixels[p][q].blue)*(f.filter[m+1][n+1]);
			green += (image->pixels[p][q].green) *(f.filter[m+1][n+1]);
			red += (image->pixels[p][q].red) * (f.filter[m+1][n+1]);
		}
	}

	blue = blue / f.norm;
	green = green / f.norm;
	red = red / f.norm; 

	newPixel.blue = ((blue >= 255) || (blue <= 0)) ? ((blue >= 255) ? 255 : 0) : blue;
	newPixel.green = ((green >= 255) || (green <= 0)) ? ((green >= 255) ? 255 : 0) : green;
	newPixel.red = ((red >= 255) || (red <= 0)) ? ((red >= 255) ? 255 : 0) : red;

return newPixel;
}

void applyThread(BMPImage * imageIn, BMPImage * imageOut, BoxFilter f, int tid, int numThreads) {
	
	//FILL IN. This is the work that thread #tid should do
	//You may find it useful to know the number of threads (numThreads) to
	//figure out what to do.

	int i, j;
	
	if(imageIn->norm_height % numThreads == 0)	//rows is a multiple of numThreads
	{
		int height = imageIn->norm_height / numThreads;
	
		for(i=tid*height; i < ((tid+1)*height); i++)
		{
			for(j=0; j < imageIn->header.width_px; j++)
			{
				imageOut->pixels[i][j] = calculate(imageIn, i, j, f);	
			}
		}
	}
	else		//rows is not a multiple of numThreads
	{
		int height = imageIn->norm_height / numThreads;	
		int rest = imageIn->norm_height % numThreads;
		
		if(tid == (numThreads-1))	//assigning last thread
		{
			for(i= imageIn->norm_height-rest; i < imageIn->norm_height; i++)
			{	
				for(j=0; j < imageIn->header.width_px; j++)
				{
				imageOut->pixels[i][j] = calculate(imageIn, i, j, f);	
				}
			}
		}
		else
		{	
			for(i=tid*height; i < ((tid+1)*height); i++)
			{
				for(j=0; j < imageIn->header.width_px; j++)
				{
				imageOut->pixels[i][j] = calculate(imageIn, i, j, f);	
				}
			}
		}
	}
}

void * applyThreadWrapper(void * args) 
{
	ApplyArgs * applyArgs = (ApplyArgs *) args;

	fprintf(stdout, "calling thread %d\n", applyArgs->tid);
	
	applyThread(applyArgs->in, applyArgs->out, applyArgs->filter, applyArgs->tid, applyArgs->numThreads);
	
	fprintf(stdout, "finished with thread %d\n", applyArgs->tid);
	
	return NULL;
}

void apply(BMPImage * imageIn, BMPImage * imageOut, BoxFilter f, int numThreads) {

	//FILL IN:
	
	//1. create numThreads threads, that each call applyThreadWrapper
	//2. wait for all threads to finish
	//3. clean up any memory allocation you did.
	
	pthread_t * threads = malloc(sizeof(pthread_t) * numThreads);

	ApplyArgs * args = malloc(sizeof(ApplyArgs) * numThreads);

	for (int i = 0; i < numThreads; i++) 
	{
		args[i].tid = i;
		args[i].numThreads = numThreads;
		args[i].in = imageIn;
		args[i].out = imageOut;
		args[i].filter = f;
	}

	for (int i = 0; i < numThreads; i++) 
	{
		pthread_create(&threads[i], NULL, applyThreadWrapper, &args[i]);
	}
	
	for (int i = 0; i < numThreads; i++) 
	{
		pthread_join(threads[i], NULL);
	}

	free(args);
	free(threads);
}
