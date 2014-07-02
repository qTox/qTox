#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP

#include <atomic>
#include <cstddef>
#include <stdlib.h>
#include <QIODevice>

template<size_t SIZE>
class Ringbuffer : public QIODevice
{
public:
    enum {capacity = SIZE};
    Ringbuffer(QObject* parent = 0) : QIODevice(parent), tail(0), head(0)
    {
        static_assert(SIZE > 0, "the size of the buffer must be > 0");
    }

    qint64 writeData(const char *data, qint64 maxSize)
    {
        if (maxSize < 0) {
            return -1;
        }

        size_t currentTail = tail.load();
        const size_t currentHead = head.load();
        const qint64 available = currentTail >= currentHead ?
            (capacity - (currentTail - currentHead)) - 1 :
            (currentHead - currentTail) - 1;

        maxSize = std::min(maxSize, available);
        qint64 bytesToWrite = maxSize;
        qint64 bytesAlreadyWritten = 0;
        size_t nextTail = currentTail + maxSize;

        if (nextTail < currentTail) {
            return -1;
        } else if (nextTail >= capacity) {
            bytesAlreadyWritten = capacity - currentTail;
            bytesToWrite -= bytesAlreadyWritten;
            memcpy(buffer + currentTail, data, bytesAlreadyWritten * sizeof(char));
            currentTail = 0;
            nextTail = nextTail % capacity;
        }
        memcpy(buffer + currentTail, data + bytesAlreadyWritten, bytesToWrite * sizeof(char));
        tail.store(nextTail);

        emit bytesWritten(maxSize);
        return maxSize;
    }

    qint64 readData(char *data, qint64 maxSize)
    {
        if (maxSize < 0) {
            return -1;
        }

        const size_t currentTail = tail.load();
        size_t currentHead = head.load();
        const qint64 contains =  currentHead <= currentTail ?
            currentTail - currentHead :
            capacity - (currentHead - currentTail);

        maxSize = std::min(maxSize, contains);
        qint64 bytesToRead = maxSize;
        qint64 bytesAlreadyRead = 0;
        size_t nextHead = currentHead + maxSize;

        if (nextHead < currentHead) {
            return -1;
        } else if (nextHead >= capacity) {
            bytesAlreadyRead = capacity - currentHead;
            bytesToRead -= bytesAlreadyRead;
            memcpy(data, buffer + currentHead, bytesAlreadyRead * sizeof(char));
            currentHead = 0;
            nextHead = nextHead % capacity;
        }
        memcpy(data + bytesAlreadyRead, buffer + currentHead, bytesToRead * sizeof(char));
        head.store(nextHead);

        return maxSize;
    }

    bool isSequential()
    {
        return true;
    }

private:
    char buffer[capacity];
    std::atomic<size_t> tail;
    std::atomic<size_t> head;
};

#endif // RINGBUFFER_HPP
