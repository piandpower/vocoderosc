#include <stdio.h>
#include <math.h>
#include "portaudio.h"
#include "synth.h"
#define NUM_VOICES    (12)
#define NUM_SECONDS   (4)
#define SAMPLE_RATE   (44100)

/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int patestCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{

    /* Cast data passed through stream to Synth. */
    Synth *synth = (Synth*)userData;
    float *out = (float*)outputBuffer;
    (void) inputBuffer; /* Prevent unused variable warning. */

    float* synthBuffer = synth_getBuffer(framesPerBuffer, synth);
    unsigned int i;
    for( i=0; i<framesPerBuffer; i++ ) {
        *out++ = synthBuffer[i];
        *out++ = synthBuffer[i];
        // printf("out: %f\n\n", synthBuffer[i]);
    }
    return 0;
}

/*******************************************************************/
int main(void);
int main(void) {
    PaStream *stream;
    PaError err;

    printf("PortAudio Test: output sawtooth wave.\n");
    /* Initialize our data for use by callback. */
    Synth* synth = synth_new(SAMPLE_RATE, NUM_VOICES);

    /* Initialize library before making any other calls. */
    err = Pa_Initialize();
    if( err != paNoError ) goto error;
    
    /* Open an audio I/O stream. */
    err = Pa_OpenDefaultStream( &stream,
                                0,          /* no input channels */
                                2,          /* stereo output */
                                paFloat32,  /* 32 bit floating point output */
                                SAMPLE_RATE,
                                256,        /* frames per buffer */
                                patestCallback,
                                synth );
    if( err != paNoError ) goto error;

    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;

    /* Start listening for TouchOSC messages */
    runServer(synth);

    /* Sleep for several seconds. */
    Pa_Sleep(NUM_SECONDS*1000);

    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;
    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;
    Pa_Terminate();
    printf("Test finished.\n");
    return err;
error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}
