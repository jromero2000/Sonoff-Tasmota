/*
  support_buffer.ino - Static binary buffer for Zigbee

  Copyright (C) 2019  Theo Arends and Stephan Hadinger

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

typedef struct SBuffer_impl {
  uint16_t size;                // size in bytes of the buffer
  uint16_t len;                 // current size of the data in buffer. Invariant: len <= size
  uint8_t buf[];                     // the actual data
} SBuffer_impl;

typedef class SBuffer {

protected:
  SBuffer(void) {
    // unused empty constructor except from subclass
  }

public:
  SBuffer(const size_t size) {
    _buf = (SBuffer_impl*) new char[size+4];   // add 4 bytes for size and len
    _buf->size = size;
    _buf->len = 0;
    //*((uint32_t*)_buf) = size;   // writing both size and len=0 in a single 32 bits write
  }

  inline size_t getSize(void) const { return _buf->size; }
  inline size_t size(void) const { return _buf->size; }
  inline size_t getLen(void) const { return _buf->len; }
  inline size_t len(void) const { return _buf->len; }
  inline uint8_t *getBuffer(void) const { return _buf->buf; }
  inline uint8_t *buf(void) const { return _buf->buf; }

  virtual ~SBuffer(void) {
    delete[] _buf;
  }

  inline void setLen(const size_t len) {
    uint16_t old_len = _buf->len;
    _buf->len = (len <= _buf->size) ? len : _buf->size;
    if (old_len < _buf->len) {
      memset((void*) &_buf->buf[old_len], 0, _buf->len - old_len);
    }
  }

  size_t add8(const uint8_t data) {           // append 8 bits value
    if (_buf->len < _buf->size) {       // do we have room for 1 byte
      _buf->buf[_buf->len++] = data;
    }
    return _buf->len;
  }
  size_t add16(const uint16_t data) {           // append 16 bits value
    if (_buf->len < _buf->size - 1) {    // do we have room for 2 bytes
      _buf->buf[_buf->len++] = data;
      _buf->buf[_buf->len++] = data >> 8;
    }
    return _buf->len;
  }
  size_t add32(const uint32_t data) {           // append 32 bits value
    if (_buf->len < _buf->size - 3) {     // do we have room for 2 bytes
      _buf->buf[_buf->len++] = data;
      _buf->buf[_buf->len++] = data >> 8;
      _buf->buf[_buf->len++] = data >> 16;
      _buf->buf[_buf->len++] = data >> 24;
    }
    return _buf->len;
  }

  size_t addBuffer(const SBuffer &buf2) {
    if (len() + buf2.len() <= size()) {
      for (uint32_t i = 0; i < buf2.len(); i++) {
        _buf->buf[_buf->len++] = buf2.buf()[i];
      }
    }
    return _buf->len;
  }

  size_t addBuffer(const char *buf2, size_t len2) {
    if (len() + len2 <= size()) {
      for (uint32_t i = 0; i < len2; i++) {
        _buf->buf[_buf->len++] = pgm_read_byte(&buf2[i]);
      }
    }
    return _buf->len;
  }

  uint8_t get8(size_t offset) const {
    if (offset < _buf->len) {
      return _buf->buf[offset];
    } else {
      return 0;
    }
  }
  uint8_t read8(const size_t offset) const {
    if (offset < len()) {
      return _buf->buf[offset];
    }
    return 0;
  }
  uint16_t get16(const size_t offset) const {
    if (offset < len() - 1) {
      return _buf->buf[offset] | (_buf->buf[offset+1] << 8);
    }
    return 0;
  }
  uint32_t get32(const size_t offset) const {
    if (offset < len() - 3) {
      return _buf->buf[offset] | (_buf->buf[offset+1] << 8) |
            (_buf->buf[offset+2] << 16) | (_buf->buf[offset+3] << 24);
    }
    return 0;
  }

  SBuffer subBuffer(const size_t start, size_t len) const {
    if (start >= _buf->len) {
      len = 0;
    } else if (start + len > _buf->len) {
      len = _buf->len - start;
    }

    SBuffer buf2(len);
    memcpy(buf2.buf(), buf()+start, len);
    buf2._buf->len = len;
    return buf2;
  }

protected:
  SBuffer_impl * _buf;

} SBuffer;

typedef class PreAllocatedSBuffer : public SBuffer {

public:
  PreAllocatedSBuffer(const size_t size, void * buffer) {
    _buf = (SBuffer_impl*) buffer;
    _buf->size = size - 4;
    _buf->len = 0;
  }

  ~PreAllocatedSBuffer(void) {
    // don't deallocate
    _buf = nullptr;
  }
} PreAllocatedSBuffer;
