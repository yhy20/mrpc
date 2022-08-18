#ifndef __MRPC_NET_ENDIAN_H__
#define __MRPC_NET_ENDIAN_H__

#include <stdint.h>
#include <endian.h>

namespace mrpc
{
namespace net
{
namespace sockets
{
 /**
 * #include <endian.h>
 * uint16_t htobe16(uint16_t host_16bits);
 * uint16_t htole16(uint16_t host_16bits);
 * uint16_t be16toh(uint16_t big_endian_16bits);
 * uint16_t le16toh(uint16_t little_endian_16bits);
 *
 * uint32_t htobe32(uint32_t host_32bits);
 * uint32_t htole32(uint32_t host_32bits);
 * uint32_t be32toh(uint32_t big_endian_32bits);
 * uint32_t le32toh(uint32_t little_endian_32bits);
 *
 * uint64_t htobe64(uint64_t host_64bits);
 * uint64_t htole64(uint64_t host_64bits);
 * uint64_t be64toh(uint64_t big_endian_64bits);
 * uint64_t le64toh(uint64_t little_endian_64bits);
 * 
 * 这些函数将整数值的字节编码从当前CPU（"主机"）使用的字节顺序转换为 little-endian 和 big-endian 字节顺序。
 * 每个函数名称中的数字 nn 表示该函数处理的整数的大小，可以是 16 位，32 位或 64 位。  
 * 
 * 名称格式为" htobe nn"的函数将从主机字节顺序转换为大端顺序。
 * 名称格式为" htole nn"的函数将从主机字节顺序转换为小端顺序。
 * 名称形式为" be nn toh"的函数将从big-endian顺序转换为主机字节顺序。
 * 名称形式为" le nn toh"的函数会从little-endian顺序转换为主机字节顺序。
 */

/// TODO: 测试错误
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"

inline uint64_t HostToNetwork64(uint64_t host64)
{
    return htobe64(host64);
}

inline uint32_t HostToNetwork32(uint32_t host32)
{
    return htobe32(host32);
}

inline uint16_t HostToNetwork16(uint16_t host16)
{
    return htobe16(host16);
}

inline uint64_t NetworkToHost64(uint64_t net64)
{
    return be64toh(net64);
}

inline uint32_t NetworkToHost32(uint32_t net32)
{
    return be32toh(net32);
}

inline uint16_t NetworkToHost16(uint16_t net16)
{
    return be16toh(net16);
}

#pragma GCC diagnostic pop

}  // namespace sockets
}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_ENDIAN_H__