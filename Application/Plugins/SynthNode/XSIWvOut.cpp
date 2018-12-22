/***************************************************/
/*! \class XSIWvOut
    \brief STK realtime audio (blocking) XSI class.
*/
/***************************************************/
#include "XSIWvOut.h"
#include <cstring>


namespace stk {

// Streaming status states.
enum { RUNNING, EMPTYING, FINISHED };

// This function does not block.  If the user does not write output
// data to the buffer fast enough, previous data will be re-output
// (data underrun).
int XSIWvOut::readBuffer( void *buffer, unsigned int frameCount )
{
  unsigned int nSamples;
  unsigned int nFrames = frameCount;
  StkFloat *input = (StkFloat *) &data_[ readIndex_];
  StkFloat *output = (StkFloat *) buffer;
  long counter;

  while ( nFrames > 0 ) {

    // I'm assuming that both the RtAudio and StkFrames buffers
    // contain interleaved data.
    counter = nFrames;

    // Pre-increment read pointer and check bounds.
    readIndex_ += nFrames;
    if ( readIndex_ >= data_.frames() ) {
      counter -= readIndex_ - data_.frames();
      readIndex_ = 0;
    }

    // Copy data from the StkFrames container.
    if ( status_ == EMPTYING && framesFilled_ <= counter ) {
      nSamples = framesFilled_ ;
      unsigned int i;
      for ( i=0; i<nSamples; i++ ) *output++ = *input++;
      nSamples = (counter - framesFilled_) ;
      for ( i=0; i<nSamples; i++ ) *output++ = 0.0;
      status_ = FINISHED;
      return 1;
    }
    else {
      nSamples = counter ;
      for ( unsigned int i=0; i<nSamples; i++ )
        *output++ = *input++;
    }

    nFrames -= counter;
  }

  mutex_.lock();
  framesFilled_ -= frameCount;
  mutex_.unlock();
  if ( framesFilled_ < 0 ) {
    framesFilled_ = 0;
    //    writeIndex_ = readIndex_;
    errorString_ << "RtWvOut: audio buffer underrun!";
    handleError( StkError::WARNING );
  }

  return 0;
}


XSIWvOut::XSIWvOut(StkFloat sampleRate, int bufferFrames, int nBuffers )
  : stopped_( true ), readIndex_( 0 ), writeIndex_( 0 ), framesFilled_( 0 ), status_(0)
{
	unsigned int size = bufferFrames;
	data_.resize( size * nBuffers, 1 );

	// Start writing half-way into buffer.
	writeIndex_ = (unsigned int ) (data_.frames() / 2.0);
	framesFilled_ = writeIndex_;
}

XSIWvOut :: ~XSIWvOut( void )
{
  // Change status flag to signal callback to clear the buffer and close.
  status_ = EMPTYING;
  while ( status_ != FINISHED) Stk::sleep( 100 );
}


void XSIWvOut :: tick( const StkFloat sample )
{
  // Block until we have room for at least one frame of output data.
  while ( framesFilled_ == (long) data_.frames() ) Stk::sleep( 1 );

  StkFloat input = sample;
  clipTest( input );
  unsigned long index = writeIndex_ ;
   data_[index++] = input;

  mutex_.lock();
  framesFilled_++;
  mutex_.unlock();
  frameCounter_++;
  writeIndex_++;
  if ( writeIndex_ == data_.frames() )
    writeIndex_ = 0;
}

void XSIWvOut :: tick( const StkFrames& frames )
{
#if defined(_STK_DEBUG_)
  if ( data_.channels() != frames.channels() ) {
    errorString_ << "RtWvOut::tick(): incompatible channel value in StkFrames argument!";
    handleError( StkError::FUNCTION_ARGUMENT );
  }
#endif

  // See how much space we have and fill as much as we can ... if we
  // still have samples left in the frames object, then wait and
  // repeat.
  unsigned int framesEmpty, nFrames, bytes, framesWritten = 0;
  unsigned int nChannels = 1;
  while ( framesWritten < frames.frames() ) {

    // Block until we have some room for output data.
    while ( framesFilled_ == (long) data_.frames() ) Stk::sleep( 1 );
    framesEmpty = data_.frames() - framesFilled_;

    // Copy data in one chunk up to the end of the data buffer.
    nFrames = framesEmpty;
    if ( writeIndex_ + nFrames > data_.frames() )
      nFrames = data_.frames() - writeIndex_;
    if ( nFrames > frames.frames() - framesWritten )
      nFrames = frames.frames() - framesWritten;
    bytes = nFrames * nChannels * sizeof( StkFloat );
    StkFloat *samples = &data_[writeIndex_ * nChannels];
    StkFrames *ins = (StkFrames *) &frames;
    memcpy( samples, &(*ins)[framesWritten * nChannels], bytes );
    for ( unsigned int i=0; i<nFrames * nChannels; i++ ) clipTest( *samples++ );

    writeIndex_ += nFrames;
    if ( writeIndex_ == data_.frames() ) writeIndex_ = 0;

    framesWritten += nFrames;
    mutex_.lock();
    framesFilled_ += nFrames;
    mutex_.unlock();
    frameCounter_ += nFrames;
  }
}

} // stk namespace
