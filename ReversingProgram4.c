//reverse audio program
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <portsf.h>


int open_output(char* name, PSF_PROPS *props);
// float* allocate_buffer(long bytes);
// float* allocate_envbuffer(long bytes);

int open_input(char* name, PSF_PROPS *pProps)
	{
		int infile;
    
    	/* Read the properties of the wav file */
    	infile = psf_sndOpen(name, pProps, 0);
   	 if(infile < 0)
   		{
			printf("Error: unable to read %s\n", name);
		}
		return infile;
	}

int open_output(char* name, PSF_PROPS *props)
{
	int outfile;
	
	outfile = psf_sndCreate(name, props, 0, 0, PSF_CREATE_RDWR); 	
	if(outfile < 0)
	{
		printf("Error: unable to create %s\n", name); 
	}
    return outfile;
}

float* allocate_inbuffer(long samples)
{
    float *inbuffer;
	inbuffer= (float*)malloc(sizeof(float) * samples);
	if(inbuffer == 0)
    {
		printf("Error: unable to allocate buffer\n");
	}
    return inbuffer;
}

int allocate_mega_buffer(long samples, int num_channels, float** mega_buffer)
{
	int i;
	mega_buffer = (float**)malloc(sizeof(float*) * num_channels);
	for (i = 0; i < num_channels; i++)
	{
		mega_buffer[i] = (float*)malloc(sizeof(float) * samples);
	}
	if (mega_buffer)
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}

int allocate_envbuffer(long samples, int num_channels, float** envbuffer)
{
	int i;
	envbuffer = (float**)malloc(sizeof(float*) * num_channels);
	for (i = 0; i < num_channels; i++)
	{
		envbuffer[i] = (float*)malloc(sizeof(float) * samples);
	}
	if (envbuffer)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int allocate_rev_buffer(long samples, int num_channels, float** rev_buffer)
{
	int i;
	rev_buffer = (float**)malloc(sizeof(float*) * num_channels);
	for (i = 0; i < num_channels; i++)
	{
		rev_buffer[i] = (float*)malloc(sizeof(float) * samples);
	}
	if (rev_buffer)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

float* allocate_bufferFinal(long samples)
{
    float *bufferFinal;
	bufferFinal = (float*)malloc(sizeof(float*) * samples);
	if(bufferFinal == 0)
    {
		printf("Error: unable to allocate bufferFinal\n");
	}
    return bufferFinal;
}

long copyin(int infile, float* inbuffer, long num_frames)
{
	long frames;
	
    /* Read the input file */
	frames = psf_sndReadFloatFrames(infile, inbuffer, num_frames);
	if(frames != num_frames)
	{
		printf("Error reading input\n");
		return -1;
	}
}

void channel_split(float* inbuffer, float** mega_buffer, int num_channels, int samples)
{
    int i;
    
    for (i = 0; i < samples; i++)
    {
    printf("test2\t%i\n\n", i);
    	mega_buffer[(i % num_channels)][(i / num_channels)] = inbuffer[i]; //THIS LINE IS THE ERROR
    printf("mega_buffer = %i", mega_buffer[(i % num_channels)][(i / num_channels)]);
    }
}

float envelope(float *envbuffer, float *mega_buffer, int size, int filtersize, int skipfactor)
{
	int e;
	for (e = 0; e < (size - filtersize); e += skipfactor)
	{
		for (int i = 0; i < filtersize; i++)
		{
			envbuffer[e] += fabs(mega_buffer[e + i]);
		}
		envbuffer[e] /= filtersize;
// 		fprintf(fp, "%f\n", envbuffer[e]);
	}
	for ( e = (size - filtersize); e < size; e++)
	{
		envbuffer[e] = 0;
// 		fprintf(fp, "%f\n", envbuffer[e]);
	}
}


void channel_join(float** mega_buffer, int num_channels, float* bufferFinal, int num_frames, int samples)
{
    int i;
    for (i = 0; i < samples; i++)
    {
        bufferFinal[i] = mega_buffer[i % num_channels][i / num_channels];
    }
}

long OutputReversed(int outfile, float* bufferFinal, long num_frames)
{
	long frames;
	
	frames = psf_sndWriteFloatFrames(outfile, bufferFinal, num_frames);
	if(frames != num_frames)
	{
		printf("Error writing to output\n");
		return -1;
	}	
    return frames;
}


void clean_up(int infile, int outfile, float* buffer, float* envbuffer)
{
    /* Close files */
    if(infile >= 0)
    {
		if(psf_sndClose(infile))
      	{
			printf("Warning: error closing input\n");
		}
	}
    if(outfile >= 0)
    {
		if (psf_sndClose(outfile))
		{
			printf("Warning: error closing output\n");
		}
	}
    
  //   Free the buffer */
// 	if(buffer)
// 	{
//       free(buffer);
//     }
//     if(envbuffer)
// 	{
//       free(envbuffer);
//     }
}

int main(int argc, char *argv[])
{
	enum{ARG_NAME, ARG_INFILE, ARG_OUTFILE,/* ARG_CHORD_NUM,*/ ARGC};
	if(argc != ARGC)
	{
		printf("error, after program name please enter: Input file name, Output file name and the number of Chords inputted");
		return -1;
	}
	
	float samplingRate;
	int infile; //click and drag would be better than typing
	int outfile;
	int chord_num = 0;
	int num_channels;
	int samples;
// 	char confirm;
	
	long bytes;
	long num_frames;
	long size;
	long pos;
	int rc = 0;
	int skipfactor;
	
	int filtersize;
	
	int e, i, y, h, x, t, b;
	
	
	
	PSF_CHPEAK* peaks = NULL;
	
	PSF_PROPS props;
	props.srate = samplingRate;
// 	props.chans = 2;
	props.samptype = PSF_SAMP_16;
	props.format = PSF_STDWAVE;
	props.chformat = MC_STEREO;
 	props.format = PSF_AIFF;	
	
infile = open_input(argv[ARG_INFILE], &props); 
	if(infile < 0)
  	{
  		return -5;
  	}

	
	num_channels = props.chans;
	printf("%i channels", num_channels);
	
	float* inbuffer;
	float** mega_buffer;
	float** envbuffer;
	float** rev_buffer;
	float* bufferFinal;
	float** chan_buffers;
	
	
	long transient_position[256];
	
	int psf_init(void);

	
  	// FIX ME! num_channels
	//int samples = num_frames * num_channels;

	printf("Sample rate = %d\n", props.srate);
	printf("number of channels = %d\n", props.chans);
	
			
	
	outfile = open_output(argv[ARG_OUTFILE], &props);
	if(outfile < 0)
	{
		return -10;
	}
	
	/* Calculate number of bytes and allocate buffer */
	num_frames = (long)psf_sndSize(infile); /* number of samples in file */ 
	samples = num_frames * num_channels;
	//bytes = num_frames * props.chans * sizeof(float);/* convert to bytes */
	inbuffer = allocate_inbuffer(samples); /* allocate buffer */
	allocate_mega_buffer(samples, num_channels, mega_buffer);
	allocate_envbuffer(samples, num_channels, envbuffer);
	allocate_rev_buffer(samples, num_channels, rev_buffer);
	bufferFinal = allocate_bufferFinal(samples);
// 	if(buffer[num_channels] == NULL)
// 	{
//   		clean_up(infile, outfile, buffer[num_channels], envbuffer[num_channels]);
// 		return -15;
// 	}


	
	copyin(infile, inbuffer, num_frames);
	
 	size = psf_sndSize(infile);
	

	printf("File size = %ld frames\n", size);
	
	printf("test\n\n");
	
	channel_split(inbuffer, mega_buffer, num_channels, samples);

	
	FILE *fp;
	fp = fopen("waveform.txt", "w");
	if (fp == NULL)
	{
		printf("error fp");
	}

	for(b = 0; b < (num_channels); b++)
 	{
 	
		envelope(envbuffer[b], mega_buffer[b], size, 1000, 10);

		envelope(envbuffer[b], envbuffer[b], size, 1000, 1);
	
		envelope(envbuffer[b], envbuffer[b], size, 1000, 1);
	
	
		for (t = 0; t < 256; t++)
		{
			transient_position[t] = 0;
		}
	
		t = 0;
	
		for (h = 0; h < (size - 2000); h++) 
		{
			if(envbuffer[b][h + 2000] > (envbuffer[b][h] + 0.01)) 
			{
				transient_position[t] = h;
				chord_num += 1;
 				printf("position = %d\n", h);
				t ++;
				h += 2500;
			}
		}
	
		for( t = 0; t < 256; t++)
		{	
			if(transient_position[t] != 0)
			{
				printf("position test = %d\n", transient_position[t]);
			}	
		}
	
	
		printf("Number of chords = %d\n", chord_num);

		
		for(t = 0; t < 256; t++)
		{
			if(transient_position[t] != 0)
			{
				if (t+1 < chord_num)
				{
					y = transient_position[t+1];
				}
				else {
					y = size;
				}
				
				printf("%d \t %d\n", transient_position[t], y);
				
				for (x = 0; x < (y-transient_position[t]); x++)
				{
					rev_buffer[b][x+transient_position[t]] = mega_buffer[b][y-x];
				}
				
				for (x = 0; x < 100; x++)
				{
					rev_buffer[b][x+transient_position[t]] *= (x/100.0);
					rev_buffer[b][y-x] *= (x/100.0);
				}	
			}
		}
	}
	
	for( e = 0; e < size; e ++)
	{
	fprintf(fp, "%f\n", bufferFinal[e]);
	}
	
	fclose(fp);	

	channel_join(rev_buffer, num_channels, bufferFinal, num_frames, samples);

	
 	OutputReversed(outfile, bufferFinal, num_frames);
	
	/* Clean up */
  //  clean_up(infile, outfile, /*buffer*/, envbuffer);

	int psf_finish(void);
}