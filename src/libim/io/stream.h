#ifndef INPUTSTREAM_H
#define INPUTSTREAM_H
#include "common.h"

#include <iostream>

#include "assert.h"
#include <climits>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

class Stream;

template<typename T>
constexpr bool IsStreamType = std::is_base_of<Stream, T>::value || std::is_same<T, Stream>::value;

template<class T, typename std::enable_if_t<IsStreamType<T>, int> = 0>
using StreamPtr = std::shared_ptr<T>;

template<class T, class... Args, typename std::enable_if_t<IsStreamType<T>, int> = 0>
StreamPtr<T> MakeStreamPtr(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename U,
         typename std::enable_if_t<std::is_same<T, Stream>::value && IsStreamType<U>, int> = 0>
std::shared_ptr<T> StreamPointerCast(const std::shared_ptr<U>& r) {
    return std::static_pointer_cast<Stream>(r);
}

template<typename T, typename U,
         typename std::enable_if_t<IsStreamType<T> && IsStreamType<U> && !std::is_same<Stream, T>::value, int> = 0>
std::shared_ptr<T> StreamPointerCast(const std::shared_ptr<U>& r) {
    return std::static_pointer_cast<T>(StreamPointerCast<Stream>(r));
}

struct StreamError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class Stream
{
public:
    virtual ~Stream() = default;

    template<class T>
    T read() const
    {
        return _read(tag<T>{});
    }

    /*template<class T>
    T read(std::size_t lenHint) const
    {
        return _read(lenHint, tag<T>{});
    }*/

    template<typename T, typename... Args>
    T read(Args... args) const
    {
        return _read(std::forward<Args>(args)..., tag<T>{});
    }

    ByteArray read(const std::size_t size) const
    {
        ByteArray data(size);
        const std::size_t nRead = read(&data[0], size);

        if(nRead != size) {
            throw StreamError("Error while reading stream!");
        }

        return data;
    }

    std::size_t read(byte_t* data, const std::size_t length) const
    {
        if(this->tell() + length > this->size()) {
            throw StreamError("End of stream");
        }

        return  readsome(data, length);
    }


//    template<class T>
//    Stream& write(T&& data)
//    {
//        return _write(std::forward<T>(data), tag<T>{});
//    }

    Stream& write(char c);
    Stream& write(int8_t i8);
    Stream& write(uint8_t ui8);
    Stream& write(int16_t i16);
    Stream& write(uint16_t ui16);
    Stream& write(int32_t i32);
    Stream& write(uint32_t ui32);
    Stream& write(int64_t i64);
    Stream& write(uint64_t ui64);
    Stream& write(float f);
    Stream& write(double d);

    template<class T, typename std::enable_if_t<!IsStreamType<T>, int> = 0>
    Stream& write(const T& data)
    {
        return _write(data, tag<T>{});
    }

    template<class T, typename std::enable_if_t<IsStreamType<T>, int> = 0>
    Stream& write(const T& istream)
    {
        return write(istream, 0, istream.size());
    }

    virtual Stream& write(const ByteArray& data)
    {
        auto nWritten = write(&data[0], data.size());
        if(nWritten != data.size()){
            throw StreamError("Failed to write data to stream!");
        }

        return *this;
    }

    /* Write from stream. read stream from offset to the ennd */
    virtual Stream& write(const Stream& istream, std::size_t offset)
    {
        return write(istream, offset, istream.size() - 1);
    }
    
    virtual Stream& write(const Stream& istream, std::size_t offsetBegin, std::size_t offsetEnd)
    {
        if(!istream.canRead() || offsetBegin >= istream.size()) 
        {
            // TODO: log!
            assert(false &&  "!istream.canRead()");
            return *this;
        }

        if((offsetBegin + offsetEnd) > istream.size()) {
            //TODO: log
            offsetEnd = (istream.size()) - offsetBegin; // write to the end of istream
        }

        //auto cur = istream.tell();
        istream.seek(offsetBegin);
        
        // TODO: read/write in batch
        write(istream.read(offsetEnd));
        
        //istream.seek(cur);
        return *this;
    }

    virtual std::size_t write(const byte_t* data, const std::size_t length)
    {
        return writesome(data, length);
    }

    virtual void seekBegin() const
    {
        return this->seek(0);
    }

    virtual void seekEnd() const
    {
        return this->seek(this->size() -1);
    }

    bool eos() const // End of Stream
    {
        return tell() >= size();
    }

    virtual void seek(std::size_t position) const = 0; // Set new absolute position relative to beginning of the stream
    virtual std::size_t size() const  = 0;
    virtual std::size_t tell() const  = 0;
    virtual bool canRead() const  = 0;
    virtual bool canWrite() const = 0;

    void setName(std::string name)
    {
        m_name = std::move(name);
    }

    const std::string& name() const
    {
        return m_name;
    }

protected:
    virtual std::size_t readsome(byte_t* data, std::size_t length) const = 0;
    virtual std::size_t writesome(const byte_t* data, std::size_t length) = 0;

private:
    template <typename T> struct tag {};

    template <typename T> T _read(std::size_t lenHint, tag<T>&&) const = delete;
    template <typename T, typename ...Args> T _read(Args&& ..., tag<T>&&) const = delete;

    /* Delete non-POD type version */
    template <typename T, typename std::enable_if<!std::is_pod<T>::value, int>::type = 0>
    T _read(tag<T>&&) const = delete;

    template <typename T>
    typename std::enable_if<!std::is_pod<T>::value, Stream>::type&
    _write(const T&, tag<T>&&) = delete;

    /* POD type sepcialization */
    template <typename T, typename std::enable_if<std::is_pod<T>::value, int>::type = 0>
    T _read(tag<T>&&) const;

    template <typename T, typename std::enable_if<std::is_pod<T>::value, int>::type = 0>
    Stream& _write(const T&, tag<T>&&);

    /* std::unique_ptr specialization */
    template <typename T>
    std::unique_ptr<T> _read(tag<std::unique_ptr<T>>&&) const;

    template <typename T>
    std::unique_ptr<T> _read(std::size_t lenHint, tag<std::unique_ptr<T>>&&) const;

    template <typename T>
    Stream& _write(const std::unique_ptr<T>& ptr, tag<std::unique_ptr<T>>&&);

     /* std::shared_ptr specialization */
    template <typename T>
    std::shared_ptr<T> _read(tag<std::shared_ptr<T>>&&) const ;

    template <typename T>
    std::shared_ptr<T> _read(std::size_t lenHint, tag<std::shared_ptr<T>>&&) const;

    template <typename T>
    Stream& _write(const std::shared_ptr<T>& ptr, tag<std::shared_ptr<T>>&&);

     /* std::vector specialization */
    template<typename T, typename A, typename std::enable_if_t<std::is_pod<T>::value, int> = 0>
    std::vector<T, A> _read(std::size_t lenHint, tag<std::vector<T, A>>&&) const;

    template<typename T, typename A, typename std::enable_if_t<!std::is_pod<T>::value, int> = 0>
    std::vector<T, A> _read(std::size_t lenHint, tag<std::vector<T, A>>&&) const;

    template<typename T, typename A, typename std::enable_if_t<std::is_pod<T>::value, int> = 0>
    Stream& _write(const std::vector<T, A>& vec, tag<std::vector<T, A>>&&);

    template<typename T, typename A, typename std::enable_if_t<!std::is_pod<T>::value, int> = 0>
    Stream& _write(const std::vector<T, A>& vec, tag<std::vector<T, A>>&&);

private:
    std::string m_name;
};



class InputStream : public virtual Stream
{
private:
    using Stream::write;
};

class OutputStream : public virtual Stream
{
private:
    using Stream::read;
};




//template<> inline Stream& Stream::write(const InputStream& stream)
//{
////    stream.
////    const auto nWritten = this->write(reinterpret_cast<const byte_t*>(&c), CHAR_BYTE);
////    if(nWritten != CHAR_BYTE) {
////        throw StreamError("Could not write char to stream!");
////    }

//    return *this;
//}



template <typename T, typename std::enable_if<std::is_pod<T>::value, int>::type>
T Stream::_read(tag<T>&&) const
{
    T pod;
    auto nRead = this->readsome(reinterpret_cast<byte_t*>(&pod), sizeof(T));
    if(nRead != sizeof(T)) {
          throw StreamError("Error reading POD from stream!");
    }

    return pod;
}

template <typename T, typename std::enable_if<std::is_pod<T>::value, int>::type>
Stream& Stream::_write(const T& pod, tag<T>&&)
{
    auto nWritten = this->writesome(reinterpret_cast<const byte_t*>(&pod), sizeof(pod));
    if(nWritten != sizeof(pod)) {
        throw StreamError("Error writing POD to stream!");
    }

    return *this;
}

template<> inline char Stream::read<char>() const
{
    char c;
    const auto nRead = this->read(reinterpret_cast<byte_t*>(&c), CHAR_BYTE);
    if(nRead != CHAR_BYTE) {
        throw StreamError("Could not read char from stream!");
    }

    return c;
}

inline Stream& Stream::write(char c)
{
    const auto nWritten = this->write(reinterpret_cast<const byte_t*>(&c), CHAR_BYTE);
    if(nWritten != CHAR_BYTE) {
        throw StreamError("Could not write char to stream!");
    }

    return *this;
}


template<> inline int8_t Stream::read<int8_t>() const
{
    int8_t i8;
    const auto nRead = this->read(reinterpret_cast<byte_t*>(&i8), INT8_BYTE);
    if(nRead != INT8_BYTE) {
        throw StreamError("Could not read int8_t from stream");
    }

    return i8;
}

inline Stream& Stream::write(int8_t i8)
{
    const auto nWritten = this->write(reinterpret_cast<const byte_t*>(&i8), INT8_BYTE);
    if(nWritten != INT8_BYTE) {
        throw StreamError("Could not write int8_t to stream");
    }

    return *this;
}

template<> inline uint8_t Stream::read<uint8_t>() const
{
    uint8_t ui8;
    const auto nRead = this->read(reinterpret_cast<byte_t*>(&ui8), INT8_BYTE);
    if(nRead != INT8_BYTE) {
        throw StreamError("Could not read uint8_t from stream");
    }

    return ui8;
}

inline Stream& Stream::write(uint8_t ui8)
{
    const auto nWritten = this->write(reinterpret_cast<const byte_t*>(&ui8), INT8_BYTE);
    if(nWritten != INT8_BYTE) {
        throw StreamError("Could not write uint8_t to stream");
    }

    return *this;
}

template<> inline int16_t Stream::read<int16_t>() const
{
    int16_t i16;
    const auto nRead = this->read(reinterpret_cast<byte_t*>(&i16), INT16_BYTE);
    if(nRead != INT16_BYTE) {
        throw StreamError("Could not read int16_t from stream");
    }

    return i16;
}

inline Stream& Stream::write(int16_t i16)
{
    const auto nWritten = this->write(reinterpret_cast<const byte_t*>(&i16), INT16_BYTE);
    if(nWritten != INT16_BYTE) {
        throw StreamError("Could not write int16_t to stream");
    }

    return *this;
}

template<> inline uint16_t Stream::read<uint16_t>() const
{
    uint16_t ui16;
    const auto nRead = this->read(reinterpret_cast<byte_t*>(&ui16), INT16_BYTE);
    if(nRead != INT16_BYTE) {
        throw StreamError("Could not read uint16_t from stream");
    }

    return ui16;
}

inline Stream& Stream::write(uint16_t ui16)
{
    const auto nWritten = this->write(reinterpret_cast<const byte_t*>(&ui16), INT16_BYTE);
    if(nWritten != INT16_BYTE) {
        throw StreamError("Could not write uint16_t to stream");
    }

    return *this;
}

template<> inline int32_t Stream::read<int32_t>() const
{
    int32_t i32;
    const auto nRead = this->read(reinterpret_cast<byte_t*>(&i32), INT32_BYTE);
    if(nRead != INT32_BYTE) {
        throw StreamError("Could not read int32_t from stream");
    }

    return i32;
}

inline Stream& Stream::write(int32_t i32)
{
    const auto nWritten = this->write(reinterpret_cast<const byte_t*>(&i32), INT32_BYTE);
    if(nWritten != INT32_BYTE) {
        throw StreamError("Could not write int32_t to stream");
    }

    return *this;
}

template<> inline uint32_t Stream::read<uint32_t>() const
{
    uint32_t ui32;
    const auto nRead = this->read(reinterpret_cast<byte_t*>(&ui32), INT32_BYTE);
    if(nRead != INT32_BYTE) {
        throw StreamError("Could not read uint32_t from stream");
    }

    return ui32;
}

inline Stream& Stream::write(uint32_t ui32)
{
    const auto nWritten = this->write(reinterpret_cast<const byte_t*>(&ui32), INT32_BYTE);
    if(nWritten != INT32_BYTE) {
        throw StreamError("Could not write uint32_t to stream");
    }

    return *this;
}

template<> inline int64_t Stream::read<int64_t>() const
{
    int64_t i64;
    const auto nRead = this->read(reinterpret_cast<byte_t*>(&i64), INT64_BYTE);
    if(nRead != INT64_BYTE) {
        throw StreamError("Could not read int64_t from stream");
    }

    return i64;
}

inline Stream& Stream::write(int64_t i64)
{
    const auto nWritten = this->write(reinterpret_cast<const byte_t*>(&i64), INT64_BYTE);
    if(nWritten != INT64_BYTE) {
        throw StreamError("Could not write int64_t to stream");
    }

    return *this;
}

template<> inline uint64_t Stream::read<uint64_t>() const
{
    uint64_t ui64;
    const auto nRead = this->read(reinterpret_cast<byte_t*>(&ui64), INT64_BYTE);
    if(nRead != INT64_BYTE) {
        throw StreamError("Could not read uint64_t from stream");
    }

    return ui64;
}

inline Stream& Stream::write(uint64_t ui64)
{
    const auto nWritten = this->write(reinterpret_cast<const byte_t*>(&ui64), INT64_BYTE);
    if(nWritten != INT64_BYTE) {
        throw StreamError("Could not write uint64_t to stream");
    }

    return *this;
}

template<> inline float Stream::read<float>() const
{
    float f;
    const auto nRead = this->read(reinterpret_cast<byte_t*>(&f), FLOAT_BYTE);
    if(nRead != FLOAT_BYTE) {
        throw StreamError("Could not read float from stream");
    }

    return f;
}

inline Stream& Stream::write(float f)
{
    const auto nWritten = this->write(reinterpret_cast<const byte_t*>(&f), FLOAT_BYTE);
    if(nWritten != FLOAT_BYTE) {
        throw StreamError("Could not write float to stream");
    }

    return *this;
}

template<> inline double Stream::read<double>() const
{
    double d;
    const auto nRead = this->read(reinterpret_cast<byte_t*>(&d), DOUBLE_BYTE);
    if(nRead != DOUBLE_BYTE) {
        throw StreamError("Could not read double from stream");
    }

    return d;
}

inline Stream& Stream::write(double d)
{
    const auto nWritten = this->write(reinterpret_cast<const byte_t*>(&d), DOUBLE_BYTE);
    if(nWritten != DOUBLE_BYTE) {
        throw StreamError("Could not write double to stream");
    }

    return *this;
}

template<> inline bool Stream::read<bool>() const
{
    return read<uint8_t>() != 0;
}

// std::uniqe_ptr
template <typename T>
std::unique_ptr<T> Stream::_read(tag<std::unique_ptr<T>>&&) const
{
    return std::unique_ptr<T>(new T(this->read<T>()));
}

template <typename T>
std::unique_ptr<T> Stream::_read(std::size_t lenHint, tag<std::unique_ptr<T>>&&) const
{
    return std::unique_ptr<T>(new T(this->read<T>(lenHint)));
}

template <typename T>
Stream& Stream::_write(const std::unique_ptr<T>& ptr, tag<std::unique_ptr<T>>&&)
{
    return this->write<T>(*ptr);
}

// std::shared_ptr
template <typename T>
std::shared_ptr<T> Stream::_read(tag<std::shared_ptr<T>>&&) const
{
    return std::shared_ptr<T>(new T(this->read<T>()));
}

template <typename T>
std::shared_ptr<T> Stream::_read(std::size_t lenHint, tag<std::shared_ptr<T>>&&) const
{
    return std::shared_ptr<T>(new T(this->read<T>(lenHint)));
}

template <typename T>
Stream& Stream::_write(const std::shared_ptr<T>& ptr, tag<std::shared_ptr<T>>&&)
{
    return this->write<T>(*ptr);
}

// std::vector
template<typename T, typename A, typename std::enable_if_t<std::is_pod<T>::value, int>>
std::vector<T, A> Stream::_read(std::size_t lenHint, tag<std::vector<T, A>>&&) const
{
    std::vector<T, A> vec(lenHint);
    const auto nRead = this->read(reinterpret_cast<byte_t*>(vec.data()), lenHint * sizeof(T));
    if(nRead != lenHint * sizeof(T)) {
        throw StreamError(std::string("Could not read std::vector of type ") + typeid(T).name() + " from stream");
    }

    return vec;
}

template<typename T, typename A, typename std::enable_if_t<!std::is_pod<T>::value, int>>
std::vector<T, A> Stream::_read(std::size_t lenHint, tag<std::vector<T, A>>&&) const
{
    std::vector<T, A> vec;
    vec.reserve(lenHint);
    for(std::size_t i = 0; i < lenHint; i++) {
        vec.push_back(this->read<T>());
    }

    return vec;
}

template<typename T, typename A, typename std::enable_if_t<std::is_pod<T>::value, int>>
Stream& Stream::_write(const std::vector<T, A>& vec, tag<std::vector<T, A>>&&)
{
    const std::size_t nWrite = vec.size() * sizeof(T);
    const auto nWritten = this->write(reinterpret_cast<const byte_t*>(vec.data()), nWrite);
    if(nWritten != nWrite) {
        throw StreamError(std::string("Could not write std::vector of type ") + typeid(T).name() + " to stream: " + name());
    }

    return *this;
}

template<typename T, typename A, typename std::enable_if_t<!std::is_pod<T>::value, int>>
Stream& Stream::_write(const std::vector<T, A>& vec, tag<std::vector<T, A>>&&)
{
    for(const auto& e : vec) {
        this->write<T>(e);
    }

    return *this;
}

// std::string
template<> inline std::string Stream::read<std::string>(std::size_t lenHint) const
{
    std::string res(lenHint, '0');
    const auto nRead = this->read(reinterpret_cast<byte_t*>(&res[0]), lenHint * sizeof(std::string::value_type));
    if(nRead != lenHint){
        throw StreamError("Could not read std::string from stream");
    }

    auto nPos = res.find('\0');
    if(nPos != std::string::npos && nPos != res.size() - 1)
        res.resize(nPos);
    return res;
}

#endif // INPUTSTREAM_H
