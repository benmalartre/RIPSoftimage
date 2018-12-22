#ifndef STK_XSIWVOUT_H
#define STK_XSIWVOUT_H

#include "WvOut.h"
#include "Mutex.h"

namespace stk {

class XSIWvOut : public WvOut
{
 public:

  //! Default constructor.
  XSIWvOut( StkFloat sampleRate = Stk::sampleRate(),int bufferFrames = RT_BUFFER_SIZE, int nBuffers = 20 );

  //! Class destructor.
  ~XSIWvOut();

  //! Output a single sample to all channels in a sample frame.
  void tick( const StkFloat sample );

  //! Output the StkFrames data.
  void tick( const StkFrames& frames );

  // This function is not intended for general use but must be
  // public for access from the audio callback function.
  int readBuffer( void *buffer, unsigned int frameCount );

 protected:
  Mutex mutex_;
  bool stopped_;
  unsigned int readIndex_;
  unsigned int writeIndex_;
  long framesFilled_;
  unsigned int status_; // running = 0, emptying buffer = 1, finished = 2

};

} // stk namespace

#endif
