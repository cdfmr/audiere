/*
  MP3 input for Audiere by cdfmr, base on minimp3.
*/

#include <string.h>
#include "input_mp3.h"
#include "utility.h"
#include "debug.h"


namespace audiere {

  MP3InputStream::MP3InputStream() {
    m_mp3ex = 0;
    m_mp3io.read = mp3Read;
    m_mp3io.read_data = this;
    m_mp3io.seek = mp3Seek;
    m_mp3io.seek_data = this;
  }

  MP3InputStream::~MP3InputStream() {
    if (m_mp3ex) {
      mp3dec_ex_close(m_mp3ex);
      delete m_mp3ex;
    }
  }


  bool
  MP3InputStream::initialize(FilePtr file) {
    m_file = file;
    readID3v1Tags();
    readID3v2Tags();
    m_file->seek(0, File::BEGIN);

    m_mp3ex = new mp3dec_ex_t;
    if (!m_mp3ex)
      return false;

    if (mp3dec_ex_open_cb(m_mp3ex, &m_mp3io, MP3D_SEEK_TO_SAMPLE))
    {
      delete m_mp3ex;
      m_mp3ex = 0;
      return false;
    }

    return true;
  }

  void
    MP3InputStream::getFormat(
    int& channel_count,
    int& sample_rate,
    SampleFormat& sample_format)
  {
    channel_count = m_mp3ex->info.channels;
    sample_rate = m_mp3ex->info.hz;
    sample_format = SF_S16;
  }

  bool
  MP3InputStream::isSeekable() {
    return true;
  }

  int
    MP3InputStream::getLength() {
    return m_mp3ex->samples / m_mp3ex->info.channels;
  }

  int
  MP3InputStream::getPosition() {
     return m_mp3ex->cur_sample / m_mp3ex->info.channels;
  }

  void
  MP3InputStream::setPosition(int position) {
    mp3dec_ex_seek(m_mp3ex, position * m_mp3ex->info.channels);
  }

  int
  MP3InputStream::doRead(int frame_count, void* samples) {
    ADR_GUARD("MP3InputStream::doRead");
    return mp3dec_ex_read(m_mp3ex, (mp3d_sample_t*)samples, frame_count * m_mp3ex->info.channels) / m_mp3ex->info.channels;
  }

  void
  MP3InputStream::reset() {
    ADR_GUARD("MP3InputStream::reset");
    mp3dec_ex_seek(m_mp3ex, 0);
  }


  size_t
  MP3InputStream::mp3Read(void *buf, size_t size, void *user_data) {
    MP3InputStream* stream = reinterpret_cast<MP3InputStream*>(user_data);
    return stream->m_file->read(buf, size);
  }

  int
  MP3InputStream::mp3Seek(uint64_t position, void *user_data) {
    MP3InputStream* stream = reinterpret_cast<MP3InputStream*>(user_data);
    return stream->m_file->seek(position, File::BEGIN) ? 0 : 1;
  }


  const char* getGenre(u8 code) {
    const char* genres[] = {
      // From Appendix A.3 at http://www.id3.org/id3v2-00.txt and

      "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk",
      "Grunge", "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies", "Other",
      "Pop", "R&B", "Rap", "Reggae", "Rock", "Techno", "Industrial",
      "Alternative", "Ska", "Death Metal", "Pranks", "Soundtrack",
      "Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk",
      "Fusion", "Trance", "Classical", "Instrumental", "Acid", "House",
      "Game", "Sound Clip", "Gospel", "Noise", "AlternRock", "Bass",
      "Soul", "Punk", "Space", "Meditative", "Instrumental Pop",
      "Instrumental Rock", "Ethnic", "Gothic", "Darkwave",
      "Techno-Industrial", "Electronic", "Pop-Folk", "Eurodance",
      "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta", "Top 40",
      "Christian Rap", "Pop/Funk", "Jungle", "Native American",
      "Cabaret", "New Wave", "Psychadelic", "Rave", "Showtunes",
      "Trailer", "Lo-Fi", "Tribal", "Acid Punk", "Acid Jazz", "Polka",
      "Retro", "Musical", "Rock & Roll", "Hard Rock", "Folk", "Folk-Rock",
      "National Folk", "Swing", "Fast Fusion", "Bebob", "Latin", "Revival",
      "Celtic", "Bluegrass", "Avantgarde", "Gothic Rock",
      "Progressive Rock", "Psychedelic Rock", "Symphonic Rock",
      "Slow Rock", "Big Band", "Chorus", "Easy Listening", "Acoustic",
      "Humour", "Speech", "Chanson", "Opera", "Chamber Music", "Sonata",
      "Symphony", "Booty Bass", "Primus", "Porn Groove", "Satire",
      "Slow Jam", "Club", "Tango", "Samba", "Folklore", "Ballad",
      "Power Ballad", "Rhythmic Soul", "Freestyle", "Duet", "Punk Rock",
      "Drum Solo", "Acapella", "Euro-House", "Dance Hall",

      // http://lame.sourceforge.net/doc/html/id3.html

      "Goa", "Drum & Bass", "Club-House", "Hardcore", "Terror", "Indie",
      "BritPop", "Negerpunk", "Polsk Punk", "Beat", "Christian Gangsta",
      "Heavy Metal", "Black Metal", "Crossover", "Contemporary C",
      "Christian Rock", "Merengue", "Salsa", "Thrash Metal", "Anime",
      "JPop", "SynthPop",
    };
    const int genre_count = sizeof(genres) / sizeof(*genres);

    return (code < genre_count ? genres[code] : "");
  }


  // Return a null-terminated std::string from the beginning of 'buffer'
  // up to 'maxlen' chars in length.
  std::string getString(u8* buffer, int maxlen) {
    char* begin = reinterpret_cast<char*>(buffer);
    int end = 0;
    for (; end < maxlen && begin[end]; ++end) {
    }
    return std::string(begin, begin + end);
  }


  void
  MP3InputStream::readID3v1Tags() {
    // Actually, this function reads both ID3v1 and ID3v1.1.

    if (!m_file->seek(-128, File::END)) {
      return;
    }

    u8 buffer[128];
    if (m_file->read(buffer, 128) != 128) {
      return;
    }

    // Verify that it's really an ID3 tag.
    if (memcmp(buffer + 0, "TAG", 3) != 0) {
      return;
    }

    std::string title   = getString(buffer + 3,  30);
    std::string artist  = getString(buffer + 33, 30);
    std::string album   = getString(buffer + 63, 30);
    std::string year    = getString(buffer + 93, 4);
    std::string comment = getString(buffer + 97, 30);
    std::string genre   = getGenre(buffer[127]);

    addTag("title",   title,   "ID3v1");
    addTag("artist",  artist,  "ID3v1");
    addTag("album",   album,   "ID3v1");
    addTag("year",    year,    "ID3v1");
    addTag("comment", comment, "ID3v1");
    addTag("genre",   genre,   "ID3v1");

    // This is the ID3v1.1 part.
    if (buffer[97 + 28] == 0 && buffer[97 + 29] != 0) {
      char track[20];
      sprintf(track, "%d", int(buffer[97 + 29]));
      addTag("track", track, "ID3v1.1");
    }
  }


  void
  MP3InputStream::readID3v2Tags() {
    // ID3v2 is super complicated.
  }

}
