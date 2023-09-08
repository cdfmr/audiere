#ifndef INPUT_MP3_H
#define INPUT_MP3_H

#include "minimp3/minimp3_ex.h"
#include "audiere.h"
#include "basic_source.h"
#include "types.h"
#include "utility.h"

namespace audiere {

  class MP3InputStream : public BasicSource {
  public:
    MP3InputStream();
    ~MP3InputStream();

    bool initialize(FilePtr file);

    void ADR_CALL getFormat(
      int& channel_count,
      int& sample_rate,
      SampleFormat& sample_format);
    int doRead(int frame_count, void* samples);
    void ADR_CALL reset();

    bool ADR_CALL isSeekable();
    int  ADR_CALL getLength();
    void ADR_CALL setPosition(int position);
    int  ADR_CALL getPosition();


  private:
    void readID3v1Tags();
    void readID3v2Tags();

    FilePtr m_file;

    mp3dec_ex_t* m_mp3ex;
    mp3dec_io_t  m_mp3io;

    static size_t mp3Read(void *buf, size_t size, void *user_data);
    static int mp3Seek(uint64_t position, void *user_data);
  };

}

#endif
