//reverse audio program
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <portsf.h>


int open_output(char* name, PSF_PROPS *props);
float* allocate_buffer(long bytes);
float* allocate_envbuffer(long bytes);

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


float* allocate_buffer(long bytes)
{
    float *buffer;
	buffer = (float*)malloc(bytes);
	if(buffer == 0)
    {
		printf("Error: unable to allocate buffer\n");
	}
    return buffer;
}

float* allocate_envbuffer(long bytes)
{
    float *envbuffer;
	envbuffer = (float*)malloc(bytes);
	if(envbuffer == 0)
    {
		printf("Error: unable to allocate envbuffer\n");
	}
    return envbuffer;
}

float* allocate_bufferFinal(long bytes)
{
    float *buffer;
	buffer = (float*)malloc(bytes);
	if(buffer == 0)
    {
		printf("Error: unable to allocate buffer\n");
	}
    return buffer;
}

long copyin(int infile, float* buffer, long num_frames)
{
	long frames;
	
    /* Read the input file */
	frames = psf_sndReadFloatFrames(infile, buffer, num_frames);
	if(frames != num_frames)
	{
		printf("Error reading input\n");
		return -1;
	}
}

float envelope(float *envbuffer, float *buffer, int size, int filtersize, int skipfactor)
{

int e;
for (e = 0; e < (size - filtersize); e += skipfactor)
	{
		for (int i = 0; i < filtersize; i++)
		{
			envbuffer[e] += fabs(buffer[e + i]);
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

int read_frame_and_reverse(float *frame,int sfd)
{
	long got;

	got = psf_sndReadFloatFrames(sfd,frame,1);
	if(got==0)
		return 1;	/* at EOF */
	if(got <0) {
		printf("got = %ld\n",got);
		return 1;		   /* this would be a true error */
	}
	/* step back two frames! */
	if(psf_sndSeek(sfd,-2,PSF_SEEK_CUR)) {
		/* we have reached frame 1, having just read it, so can't step back 2! */
		printf("reached reversed EOF!\n");	
		return -1;
	}
	return 0;
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
    
    /* Free the buffer */
	if(buffer)
	{
      free(buffer);
    }
    if(envbuffer)
	{
      free(envbuffer);
    }
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
	int temp_file_num;
// 	char confirm;
	float* buffer;
	float* envbuffer;
	float* bufferFinal;
	long bytes;
	long num_frames;
	long size;
	long pos;
	int rc = 0;
	int skipfactor;
	
	int filtersize;
	
	int e, i, y, h, x, t;
	

	PSF_CHPEAK* peaks = NULL;
	
	PSF_PROPS props;
	props.srate = samplingRate;
	props.chans = 2;
	props.samptype = PSF_SAMP_16;
	props.format = PSF_STDWAVE;
	props.chformat = MC_STEREO;
 	props.format = PSF_AIFF;	
	
	long transient_position[256];
	
	int psf_init(void);

	infile = open_input(argv[ARG_INFILE], &props); 
	if(infile < 0)
  	{
  		return -5;
  	}
  	

	printf("Sample rate = %d\n", props.srate);
	printf("number of channels = %d\n", props.chans);
	
			
	
	outfile = open_output(argv[ARG_OUTFILE], &props);
	if(outfile < 0)
	{
		return -10;
	}
	
	/* Calculate number of bytes and allocate buffer */
	num_frames = (long)psf_sndSize(infile); /* number of samples in file */ 
	bytes = num_frames * props.chans * sizeof(float);/* convert to bytes */
	buffer = allocate_buffer(bytes); /* allocate buffer */
	envbuffer = allocate_envbuffer(bytes);
	bufferFinal = allocate_bufferFinal(bytes);
	if(buffer == NULL)
	{
  		clean_up(infile, outfile, buffer, envbuffer);
		return -15;
	}


	
	copyin(infile, buffer, num_frames);
	
	
	
	size = psf_sndSize(infile);
	printf("File size = %ld frames\n", size);
	
	
	
//if( (psf_sndSeek(infile, 0, PSF_SEEK_SET) ) < 0)
	
	
	rc = psf_sndSeek(infile,-1,PSF_SEEK_END);
	
	if(rc)
		{
			printf("error seeking to last frame\n");
		}
	
	pos = psf_sndTell(infile);
	printf("starting at frame %ld\n",pos);
	
	
	double maxval = 0.0;
	unsigned long posx = 0;
	int w;
	for(w=0; w < size; w++)
	{	
		if(buffer[w] > maxval)
		{
			maxval = buffer[w];
			posx = w;
		}
	}
	printf("the maximum sample is %f, at position %d\n", maxval, posx);
	
	FILE *fp;
	fp = fopen("waveform.txt", "w");
	if (fp == NULL)
	{
		printf("error fp");
	}
	
//// 	filtersize = 5000;
// 	filtersize = 1000;
// 	skipfactor = 10;
// 	
// 	for (e = 0; e < (size - filtersize); e++)
// 	for (e = 0; e < (size - filtersize); e += skipfactor)
// 	{
// 		for (int i = 0; i < filtersize; i++)
// 		{
// 			envbuffer[e] += fabs(buffer[e + i]);
// 		}
// 		envbuffer[e] /= filtersize;
// 		fprintf(fp, "%f\n", envbuffer[e]);
// 	}
// 	for ( e = (size - filtersize); e < size; e++)
// 	{
// 		envbuffer[e] = 0;
// 		fprintf(fp, "%f\n", envbuffer[e]);
// 	}
// 		
// envelope(envbuffer, buffer, filtersize, skipfactor)

	
	envelope(envbuffer, buffer, size, 1000, 10);

	envelope(envbuffer, envbuffer, size, 1000, 1);
	
	envelope(envbuffer, envbuffer, size, 1000, 1);



	
	
	for (t = 0; t < 256; t++)
	{
		transient_position[t] = 0;
	}
	
	t = 0;
	
	for (h = 0; h < (size - 2000); h++) 
	{
		if(envbuffer[h + 2000] > (envbuffer[h] + 0.01)) 
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
	
//	chord_num += 1;
	
	printf("Number of chords = %d\n", chord_num);
	
// 	for(t = 0; t < 256; t++)
// 	{
// 		if(transient_position[t] != 0)
// 		{
// 			y = (transient_position[t]);
// 			for(x = 0; x < y; x++)
// 			{
// 				buffer[x] = bufferFinal[y];
// 				y--;
// 			}
// 		}
// 	}
		
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

				bufferFinal[x+transient_position[t]] = buffer[y-x];
			} 
			for (x = 0; x < 100; x++)
			{
				bufferFinal[x+transient_position[t]] *= (x/100.0);
				bufferFinal[y-x] *= (x/100.0);
			}	
		}
	}
	
	for( e = 0; e < size; e ++)
	{
	fprintf(fp, "%f\n", bufferFinal[e]);
	}
	
	fclose(fp);	

	// 	
// 	if(read_frame_and_reverse(buffer,infile))
// 	{
// 		printf("Error reading initial frame\n");
// 
// 	}
// 	while(size-- >= 0) 
// 	{							
// 		if(psf_sndWriteFloatFrames(outfile,buffer,1)<1){
// 			printf("error writing frame\n");
// 		
// 		}
// 		if(size % 100000 == 0)
// 			printf("%ld\r",size);
// 		if(read_frame_and_reverse(buffer,infile) <0){
// 			/* we read the first sample, but failed to reverse; write final sample and break */
// 			if(psf_sndWriteFloatFrames(outfile,buffer,1)<1){
// 				printf("error writing frame\n");
// 			
// 			}
// 			break;
// 		}
// 	} 
	

	
 	OutputReversed(outfile, bufferFinal, num_frames);
	
	/* Clean up */
    clean_up(infile, outfile, buffer, envbuffer);

	int psf_finish(void);
}