#include "Buffer.h"
#include "Endian.h"
#include "Logging.h"
#include "TcpConnection.h"

class LengthHeaderCodec : mrpc::noncopyable
{
public:
    typedef std::function<void(const mrpc::net::TcpConnectionPtr&,
                                const std::string&,
                                mrpc::TimeStamp)> StringMessageCallback;

    explicit LengthHeaderCodec(const StringMessageCallback& cb)
        : m_messageCallback(cb) { }
    
    void onMessage(const mrpc::net::TcpConnectionPtr& conn,
                   mrpc::net::Buffer* buf,
                   mrpc::TimeStamp receiveTime)
    {
        while(buf->readableBytes() >= kHeaderLen)
        {
            int32_t len = buf->peekInt32();
            if(len > 65536 || len < 0)
            {
                LOG_ERROR << "Invalid lenght " << len;
                conn->shutdown();
                break; 
            } 
            else if(buf->readableBytes() >= len + kHeaderLen)
            {
                buf->retrieve(kHeaderLen);
                std::string message(buf->readBegin(), len);
                m_messageCallback(conn, message, receiveTime);
                buf->retrieve(len);
            }
            else
            {
                break;
            }
        }
    }

void send(mrpc::net::TcpConnection* conn,
          const mrpc::StringPiece& message)
{
    mrpc::net::Buffer buf;
    buf.append(message.data(), message.size());
    int32_t len = static_cast<int32_t>(message.size());
    int32_t be32 = mrpc::net::sockets::HostToNetwork32(len);
    buf.prepend(&be32, sizeof(be32));
    conn->send(&buf);
}

private:
    StringMessageCallback m_messageCallback;
    const static size_t kHeaderLen = sizeof(int32_t);
};