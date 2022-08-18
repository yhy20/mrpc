#ifndef __MRPC_NET_ZLISTREAM_H__
#define __MRPC_NET_ZLISTREAM_H__ 

#include "Buffer.h"
#include "noncopyable.h"

#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <zlib.h>

namespace mrpc
{
namespace net
{

class ZlibInputStream : noncopyable
{
public:
    explicit ZlibInputStream(Buffer* output)
        : m_output(output),
          m_zerror(Z_OK)
    {
        memZero(&m_zstream, sizeof(m_zstream));
        m_zerror = inflateInit(&m_zstream);
    }

    ~ZlibInputStream()
    {
        
    }


private:
    int decompress(int flush);

private:
    Buffer* m_output;
    z_stream m_zstream;
    int m_zerror;
};

class ZlibOutputStream : noncopyable
{

};

}  // namespace net
}  // namespace mrpck

#endif  // __MRPC_NET_ZLISTREAM_H__ 