#pragma once

#include <vector>

#include <libwebsockets.h>


class MessageBuffer
{
public:
    typedef std::vector<char> Buffer;

    MessageBuffer();
    MessageBuffer(MessageBuffer&&);

    const Buffer::value_type* data() const;
    Buffer::value_type* data();

    size_t size() const;

    void append(const char*, size_t);
    void assign(const char*, size_t);
    void assign(const char*);

    bool onReceive(lws*, void* chunk, size_t);
    bool writeAsText(lws*);

    void swap(MessageBuffer& other);

    void clear();

private:
    Buffer _buffer;
};