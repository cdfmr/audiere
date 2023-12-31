#include "memory_file.h"
#include "internal.h"
#include "utility.h"


namespace audiere {

  ADR_EXPORT(File*) AdrCreateMemoryFile(const void* buffer, int size) {
    if (size && !buffer) {
      return 0;
    }
    if (size < 0) {
      return 0;
    }

    return new MemoryFile(buffer, size);
  }

  ADR_EXPORT(File*) AdrCreateMemoryFileInPlace(const void* buffer, int size) {
    if (size && !buffer) {
      return 0;
    }
    if (size < 0) {
      return 0;
    }

    return new MemoryFile(buffer, size, false);
  }


  int getNextPowerOfTwo(int value) {
    int i = 1;
    while (i < value) {
      i *= 2;
    }
    return i;
  }

  MemoryFile::MemoryFile(const void* buffer, int size, bool copydata) {
    m_size = size;
    m_owndata = copydata;
    if (copydata) {
      m_capacity = getNextPowerOfTwo(size);
      m_buffer = new u8[m_capacity];
      memcpy(m_buffer, buffer, size);
    } else {
      m_capacity = size;
      m_buffer = (u8*)buffer;
    }

    m_position = 0;
  }

  MemoryFile::~MemoryFile() {
    if (m_owndata) {
      delete[] m_buffer;
    }
  }

  int ADR_CALL MemoryFile::read(void* buffer, int size) {
    int real_read = std::min((m_size - m_position), size);
    memcpy(buffer, m_buffer + m_position, real_read);
    m_position += real_read;
    return real_read;
  }

  int ADR_CALL MemoryFile::write(const void* buffer, int size) {
    ensureSize(m_position + size);
    memcpy(m_buffer + m_position, buffer, size);
    m_position += size;
    return size;
  }

  bool ADR_CALL MemoryFile::seek(int position, SeekMode mode) {
    int real_pos;
    switch (mode) {
      case BEGIN:   real_pos = position;              break;
      case CURRENT: real_pos = m_position + position; break;
      case END:     real_pos = m_size + position;     break;
      default:      return false;
    }

    if (real_pos < 0 || real_pos > m_size) {
      m_position = 0;
      return false;
    } else {
      m_position = real_pos;
      return true;
    }
  }

  int ADR_CALL MemoryFile::tell() {
    return m_position;
  }

  void MemoryFile::ensureSize(int min_size) {
    bool realloc_needed = false;
    if (m_capacity < min_size) {
      realloc_needed = true;
      if (m_owndata) {
        while (m_capacity < min_size) {
          m_capacity *= 2;
        }
      } else {
        m_capacity = getNextPowerOfTwo(min_size);
      }
    }

    if (realloc_needed) {
      u8* new_buffer = new u8[m_capacity];
      memcpy(new_buffer, m_buffer, m_size);
      if (m_owndata) {
        delete[] m_buffer;
      }
      m_buffer = new_buffer;
      m_owndata = true;
    }

    m_size = min_size;
  }

};
