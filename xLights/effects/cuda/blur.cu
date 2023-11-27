#include <cuda.h>
#include <cuda_runtime.h>
#include <string>
#include <iostream>
#include "blur.h"
#include "Color.h"

__global__ void separateChannels(const uchar4* const inputImageRGBA , int numRows , int numCols , unsigned char* const redChannel , 
                      unsigned char* const greenChannel , unsigned char* const blueChannel);

__global__
void recombineChannels(const unsigned char* const redChannel , const unsigned char* const greenChannel,
                       const unsigned char* const blueChannel,
                       uchar4* const outputImageRGBA,
                       int numRows,
                       int numCols);

__global__
void gaussianBlur(const unsigned char* const inputChannel , unsigned char* const outputChannel, int numRows , int numCols , const float* const filter, int filterWidth);

__global__ 
void boxBlur(unsigned char* input_image, unsigned char* output_image, int width, int height, int fsize);

Blurrer * Blurrer::instance = NULL;

Blurrer::Blurrer()
{
}

Blurrer * Blurrer::factory()
{
	if(Blurrer::instance == NULL)
	{
		Blurrer::instance = new Blurrer();
	}
	return Blurrer::instance;
	
}

void Blurrer::BlurPixels(xlColor *pixels,int rows, int cols, int filterWidth)
{
	numRows = rows;
	numCols = cols;
	allocateMemory(pixels);
	makeFilter(filterWidth);
	wrapperBlurrer();
	cudaMemcpy(pixels, d_outputImageRGBA , sizeof(uchar4) * numRows * numCols, cudaMemcpyDeviceToHost);
	resetBlurrer();
}

void Blurrer::allocateMemory(xlColor *pixels)
{
	const size_t numPixels = numRows * numCols;
	h_inputImageRGBA  = (uchar4 *)pixels;
	//h_outputImageRGBA = (uchar4 *)pixels;
	cudaMalloc(&d_inputImageRGBA, sizeof(uchar4) * numPixels);
	cudaMalloc(&d_outputImageRGBA, sizeof(uchar4) * numPixels);
	cudaMemset(d_outputImageRGBA, 0, numPixels * sizeof(uchar4));
	cudaMemcpy(d_inputImageRGBA, h_inputImageRGBA, sizeof(uchar4) * numPixels, cudaMemcpyHostToDevice);
	cudaMalloc(&d_redBlurred,    sizeof(unsigned char) * numPixels);
	cudaMalloc(&d_greenBlurred,  sizeof(unsigned char) * numPixels);
	cudaMalloc(&d_blueBlurred,   sizeof(unsigned char) * numPixels);
	cudaMemset(d_redBlurred,   0, sizeof(unsigned char) * numPixels);
	cudaMemset(d_greenBlurred, 0, sizeof(unsigned char) * numPixels);
	cudaMemset(d_blueBlurred,  0, sizeof(unsigned char) * numPixels);
	cudaMalloc(&d_red,   sizeof(unsigned char) * numRows * numCols);
	cudaMalloc(&d_green, sizeof(unsigned char) * numRows * numCols);
	cudaMalloc(&d_blue,  sizeof(unsigned char) * numRows * numCols);
	

}

void Blurrer::makeFilter(int fWidth)
{
	const int blurKernelWidth = 9;
	const float blurKernelSigma = 2.;
	filterWidth = blurKernelWidth;

	//create and fill the filter we will convolve with
	h_filter = new float[blurKernelWidth * blurKernelWidth];
	float filterSum = 0.f; //for normalization
	for (int r = -blurKernelWidth/2; r <= blurKernelWidth/2; ++r) 
	{
		for (int c = -blurKernelWidth/2; c <= blurKernelWidth/2; ++c) 
		{
			float filterValue = expf( -(float)(c * c + r * r) / (2.f * blurKernelSigma * blurKernelSigma));
      			h_filter[(r + blurKernelWidth/2) * blurKernelWidth + c + blurKernelWidth/2] = filterValue;
      			filterSum += filterValue;
    		}	
  	}

  	float normalizationFactor = 1.f / filterSum;

  	for (int r = -blurKernelWidth/2; r <= blurKernelWidth/2; ++r) 
	{
    	for (int c = -blurKernelWidth/2; c <= blurKernelWidth/2; ++c) 
		{
			h_filter[(r + blurKernelWidth/2) * blurKernelWidth + c + blurKernelWidth/2] *= normalizationFactor;
    	}
  	}	
	
	cudaMalloc(&d_filter , filterWidth * filterWidth * sizeof(float));
	cudaMemcpy(d_filter , h_filter , filterWidth * filterWidth * sizeof(float) , cudaMemcpyHostToDevice);
}

/* Wrapper to call the blur kernel.
 * Initializes the block and grid dimensions and calls a series of 3 kernels  : 
 * seperate Channels , gaussianBlur : once for each colour channel.
 * recombineChannels
 */
void Blurrer::wrapperBlurrer()
{
	const int BLOCK_WIDTH =  32;
	const dim3 blockSize(BLOCK_WIDTH , BLOCK_WIDTH);
	const dim3 gridSize((numCols/BLOCK_WIDTH) + 1 , (numRows/BLOCK_WIDTH) + 1 );
	
	separateChannels<<<gridSize,blockSize>>>(d_inputImageRGBA , numRows , numCols, d_red, d_green , d_blue);
        cudaDeviceSynchronize(); cudaGetLastError();
  
	gaussianBlur<<<gridSize , blockSize>>>(d_red , d_redBlurred, numRows, numCols, d_filter, filterWidth);
	gaussianBlur<<<gridSize , blockSize>>>(d_green , d_greenBlurred, numRows, numCols, d_filter, filterWidth);
	gaussianBlur<<<gridSize , blockSize>>>(d_blue , d_blueBlurred, numRows, numCols, d_filter, filterWidth);
	cudaDeviceSynchronize(); cudaGetLastError();
  
	recombineChannels<<<gridSize, blockSize>>>(d_redBlurred , d_greenBlurred , d_blueBlurred , d_outputImageRGBA , numRows, numCols);
	cudaDeviceSynchronize(); cudaGetLastError();
}

void Blurrer::filterBlur (unsigned char* input_image, unsigned char* output_image, int width, int height, int filterSize) {

    unsigned char* dev_input;
    unsigned char* dev_output;
    cudaMalloc( (void**) &dev_input, width*height*3*sizeof(unsigned char));
    cudaMemcpy( dev_input, input_image, width*height*3*sizeof(unsigned char), cudaMemcpyHostToDevice );
 
    cudaMalloc( (void**) &dev_output, width*height*3*sizeof(unsigned char));

    dim3 blockDims(512,1,1);
    dim3 gridDims((unsigned int) ceil((double)(width*height*3/blockDims.x)), 1, 1 );


    boxBlur<<<gridDims, blockDims>>>(dev_input, dev_output, width, height, filterSize);
           

    cudaMemcpy(output_image, dev_output, width*height*3*sizeof(unsigned char), cudaMemcpyDeviceToHost );

    cudaFree(dev_input);
    cudaFree(dev_output);

}

void Blurrer::resetBlurrer()
{
	cudaFree(d_red);
	cudaFree(d_green);
	cudaFree(d_blue);
	cudaFree(d_filter);
	cudaFree(d_redBlurred);	
	cudaFree(d_greenBlurred);
	cudaFree(d_blueBlurred);
	cudaFree(d_inputImageRGBA);
	cudaFree(d_outputImageRGBA);
	delete [] h_filter;
	h_inputImageRGBA = NULL;
	//h_outputImageRGBA = NULL;
	d_inputImageRGBA = NULL;
	d_outputImageRGBA = NULL;
	d_red = NULL;
	d_green = NULL;
	d_blue = NULL;
	h_filter = NULL;
	d_filter = NULL;
	d_redBlurred = NULL;
	d_greenBlurred = NULL; 	
	d_blueBlurred = NULL;
	filterWidth = 0;
	numRows = 0;
	numCols = 0;
}

Blurrer::~Blurrer()
{
	
}

__global__
void gaussianBlur(const unsigned char* const inputChannel , unsigned char* const outputChannel, int numRows , int numCols, const float* const filter, int filterWidth)
{
	int half_width = filterWidth/2;
	float image_value = 0.0f;
	float blur_value = 0.0f;
	float computed_value = 0.0f;
	int row = 0;
	int column = 0;
	const int2 thread_2D_pos = make_int2( blockIdx.x * blockDim.x + threadIdx.x,
                                        blockIdx.y * blockDim.y + threadIdx.y);
	const int thread_1D_pos = thread_2D_pos.y * numCols + thread_2D_pos.x;

	if (thread_2D_pos.x >= numCols || thread_2D_pos.y >= numRows) {
		return;
	}

	for(row = -half_width ; row <= half_width ; ++row) {
		for(column = -half_width; column <= half_width ; ++column) {
			int image_r = min(max(thread_2D_pos.y + row, 0), (numRows - 1));
			int image_c = min(max(thread_2D_pos.x + column, 0), (numCols - 1));
			image_value = static_cast<float>(inputChannel[(image_r) * numCols + (image_c)]);
			blur_value = filter[(row + half_width) * filterWidth + (column + half_width)];
			computed_value += image_value * blur_value;
		}       
	}
    
	outputChannel[thread_1D_pos] = static_cast<char>(computed_value);	
}

/* This kernel takes in an image where each pixel is represented as a uchar4 and splits
 * it into three color channels . 
 */
__global__ void separateChannels(const uchar4* const inputImageRGBA , int numRows , int numCols , unsigned char* const redChannel , 
                      unsigned char* const greenChannel , unsigned char* const blueChannel)
{
	const int2 thread_2D_pos = make_int2( blockIdx.x * blockDim.x + threadIdx.x,
                                        blockIdx.y * blockDim.y + threadIdx.y);

	const int thread_1D_pos = thread_2D_pos.y * numCols + thread_2D_pos.x;

	if (thread_2D_pos.x >= numCols || thread_2D_pos.y >= numRows)	
	{
		return;
	}

	redChannel[thread_1D_pos] = inputImageRGBA[thread_1D_pos].x;
	greenChannel[thread_1D_pos] = inputImageRGBA[thread_1D_pos].y;
	blueChannel[thread_1D_pos] = inputImageRGBA[thread_1D_pos].z;

}

/* Recombines the three colour channels to form a single output coloured image */
__global__
void recombineChannels(const unsigned char* const redChannel , const unsigned char* const greenChannel,
                       const unsigned char* const blueChannel,
                       uchar4* const outputImageRGBA,
                       int numRows,
                       int numCols)
{
	const int2 thread_2D_pos = make_int2( blockIdx.x * blockDim.x + threadIdx.x,
                                        blockIdx.y * blockDim.y + threadIdx.y);
	const int thread_1D_pos = thread_2D_pos.y * numCols + thread_2D_pos.x;

	if (thread_2D_pos.x >= numCols || thread_2D_pos.y >= numRows)
	{
		return;
	}

	unsigned char red   = redChannel[thread_1D_pos];
	unsigned char green = greenChannel[thread_1D_pos];
	unsigned char blue  = blueChannel[thread_1D_pos];
	uchar4 outputPixel = make_uchar4(red, green, blue, 255);
	outputImageRGBA[thread_1D_pos] = outputPixel;
}

__global__
void boxBlur(unsigned char* input_image, unsigned char* output_image, int width, int height, int fsize) {

    const unsigned int offset = blockIdx.x*blockDim.x + threadIdx.x;
    int x = offset % width;
    int y = (offset-x)/width;
    if(offset < width*height) {

        float output_red = 0;
        float output_green = 0;
        float output_blue = 0;
        int hits = 0;
        for(int ox = -fsize; ox < fsize+1; ++ox) {
            for(int oy = -fsize; oy < fsize+1; ++oy) {
                if((x+ox) > -1 && (x+ox) < width && (y+oy) > -1 && (y+oy) < height) {
                    const int currentoffset = (offset+ox+oy*width)*3;
                    output_red += input_image[currentoffset]; 
                    output_green += input_image[currentoffset+1];
                    output_blue += input_image[currentoffset+2];
                    hits++;
                }
            }
        }
        output_image[offset*3] = output_red/hits;
        output_image[offset*3+1] = output_green/hits;
        output_image[offset*3+2] = output_blue/hits;
    }
}