#ifndef ITER_UTF8_HPP_
#define ITER_UTF8_HPP_

#include <cstdint>
#include <iterator>

class utf8_iterator;

class UTF8
{
  friend class utf8_iterator;
public:
  UTF8(const void *s, size_t n) { s_ = reinterpret_cast<const uint8_t *>(s); end_ = s_ + n; }
  inline utf8_iterator begin() const;
  utf8_iterator end() const;
private:
  const uint8_t *s_;
  const uint8_t *end_;
};

/* TODO:
 * Instead of pointer to container, just store the pointer to end (-4).
 * In dereference operator, don't check whether payload bytes can be read:
 *   if we're past the end pointer by less than 4:
 *     clear a 4 byte buffer
 *     copy needed bytes into buffer
 *     parse data in buffer
 * In case of two payload bytes, ifdef unaligned check of: *p & 0xc0c0 == 0x8080
 * Add check code point is in minimal length encoding.
 */
class utf8_iterator: public std::iterator<std::forward_iterator_tag, int32_t>
{
  friend class UTF8;
private:
  const UTF8 *utf8_;
  const uint8_t *p_;
  int curr_seq_len_;
  utf8_iterator(const UTF8 *utf8, const uint8_t *p) : utf8_(utf8), p_(p) { curr_seq_len_ = 0; }
public:
  utf8_iterator(const utf8_iterator& other) { utf8_ = other.utf8_; p_ = other.p_; }
  utf8_iterator& operator++();
  bool operator!=(const utf8_iterator& other) { return p_ != other.p_; }
  int32_t operator*();
};

inline utf8_iterator UTF8::begin() const { return utf8_iterator(this, s_); }
inline utf8_iterator UTF8::end() const { return utf8_iterator(this, end_); }

#endif /* defined(ITER_UTF8_HPP_) */
