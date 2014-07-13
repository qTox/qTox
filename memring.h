#ifndef MEMRING_H
#define MEMRING_H

#include <exception>
#include <string.h>
#include <stddef.h>

template<typename T> class MemRing {
public:
    explicit MemRing(size_t size):
        _buff(0), _w_ptr(0), _r_ptr(0),
        _size(0), _size_mask(0)
    {
        if (!size)
            throw std::exception();

        size += 1; // reserve at least 1 element for internal purposes
        size_t power_of_two = 1;
        for (; (1u << power_of_two) < size; power_of_two++) ;

        _size = _size_mask = 1 << power_of_two;
        _size_mask -= 1;

        _buff = reinterpret_cast<T*>(malloc(_size * sizeof(T)));

        if (!_buff)
            throw std::exception();
    }

    ~MemRing() {
        if (_buff)
            free(_buff);
    }

    size_t readSpace()
    {
        size_t w, r;

        w = _w_ptr;
        r = _r_ptr;

        if (w > r) {
            return w - r;
        } else {
            return (w - r + _size) & _size_mask;
        }
    }

    size_t writeSpace()
    {
        size_t w, r;

        w = _w_ptr;
        r = _r_ptr;

        if (w > r) {
            return ((r - w + _size) & _size_mask) - 1;
        } else if (w < r) {
            return (r - w) - 1;
        } else {
            return _size - 1;
        }
    }

    inline void reset() { _r_ptr = _w_ptr = 0; }

    size_t pull(T* data, size_t len)
    {
        size_t read_space = readSpace();

        if (!read_space)
            return 0;

        size_t to_read = len > read_space ? read_space : len;
        size_t n0, n1, n2, r_ptr = _r_ptr;

        n0 = r_ptr + to_read;

        if (n0 > _size) {
            n1 = _size - r_ptr;
            n2 = n0 & _size_mask;
        } else {
            n1 = to_read;
            n2 = 0;
        }

        memcpy (data, _buff + r_ptr, n1 * sizeof(T));
        r_ptr = (r_ptr + n1) & _size_mask;

        if (n2) {
            memcpy (data + n1, &(_buff[r_ptr]), n2 * sizeof(T));
            r_ptr = (r_ptr + n2) & _size_mask;
        }

        _r_ptr = r_ptr;

        return to_read;
    }

    size_t push(T* data, size_t len)
    {
        size_t write_space = writeSpace();

        if (!write_space)
            return 0;

        size_t to_write;
        size_t n0, n1, n2, w_ptr = _w_ptr;

        to_write = len > write_space ? write_space : len;

        n0 = w_ptr + to_write;

        if (n0 > _size) {
            n1 = _size - w_ptr;
            n2 = n0 & _size_mask;
        } else {
            n1 = to_write;
            n2 = 0;
        }

        memcpy(_buff + w_ptr, data, n1 * sizeof(T));
        w_ptr = (w_ptr + n1) & _size_mask;

        if (n2) {
            memcpy(_buff + w_ptr, data + n1, n2 * sizeof(T));
            w_ptr = (w_ptr + n2) & _size_mask;
        }

        _w_ptr = w_ptr;

        return to_write;
    }

private:
    T* _buff;
    volatile size_t _w_ptr;
    volatile size_t _r_ptr;
    size_t _size, _size_mask;
};

#endif // MEMRING_H
