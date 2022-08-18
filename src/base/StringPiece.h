#ifndef __MRPC_BASE_STRINGPIECE_H__
#define __MRPC_BASE_STRINGPIECE_H__

#include <string.h>

#include <iosfwd>
#include <string>

namespace mrpc
{

class StringArg
{
public:
    StringArg(const char* str)
        : m_str(str) { }

    StringArg(const std::string& str)
        : m_str(str.c_str()) { }

    const char* c_str() const { return m_str; }

private:
    const char* m_str;
};

class StringPiece
{
public:
    StringPiece()
        : m_ptr(nullptr), m_length(0) { }
    StringPiece(const char* str)
        : m_ptr(str), m_length(static_cast<int>(strlen(m_ptr))) { }
    StringPiece(const unsigned char* str)
        : m_ptr(reinterpret_cast<const char*>(str)),
          m_length(static_cast<int>(strlen(m_ptr))) { }
    StringPiece(const std::string& str)
        : m_ptr(str.data()), m_length(static_cast<int>(str.size())) { }
    StringPiece(const char* offset, int len)
        : m_ptr(offset), m_length(len) { }

public:
    const char* data() const { return m_ptr; }
    int size() const { return m_length; }
    bool empty() const { return m_length == 0; }
    const char* begin() const { return m_ptr; }
    const char* end() const { return m_ptr + m_length; }

    void clear() { m_ptr = nullptr; m_length = 0; }
    void set(const char* buffer, int len) { m_ptr = buffer; m_length = len; }
    void set(const char* buffer)
    {
        m_ptr = buffer;
        m_length = static_cast<int>(strlen(buffer));
    }
    void set(const void* buffer, int len)
    {
        m_ptr = reinterpret_cast<const char*>(buffer);
        m_length = len;
    }
    char operator[](int i) const { return m_ptr[i]; }

    void removePrefix(int n)
    {
        m_ptr += n;
        m_length -= n;
    }

    void removeSuffix(int n)
    {
        m_length -= n;
    }

    bool operator==(const StringPiece& rhs)
    {
        return ((m_length == rhs.m_length) &&
                (memcmp(m_ptr, rhs.m_ptr, m_length)));
    }

    bool operator!=(const StringPiece& rhs)
    {
        return !(*this == rhs);
    }

#define STRINGPIECE_BINARY_PREDICATE(cmp, auxcmp)                   \
    bool operator cmp (const StringPiece& rhs) const                \
    {                                                               \
        int r = memcmp(m_ptr, rhs.m_ptr,                            \
            m_length < rhs.m_length ? m_length : rhs.m_length);     \
        return ((r auxcmp 0) ||                                     \
            ((r == 0) && (m_length cmp rhs.m_length)));             \
    }
    STRINGPIECE_BINARY_PREDICATE(<, <);
    STRINGPIECE_BINARY_PREDICATE(<=, <);
    STRINGPIECE_BINARY_PREDICATE(>=, >);
    STRINGPIECE_BINARY_PREDICATE(>, >);
#undef STRINGPIECE_BINARY_PREDICATE

    int compare(const StringPiece& rhs) const{
        int r = memcmp(m_ptr, rhs.m_ptr, \
            m_length < rhs.m_length ? m_length : rhs.m_length);
        if(r == 0)
        {
            if(m_length < rhs.m_length) r = -1;
            else if(m_length > rhs.m_length) r = +1;
        }
        return r;
    }

    std::string as_string() const 
    {
        return std::string(data(), size());
    }

    void copyToString(std::string* target) const 
    {
        target->assign(m_ptr, m_length);
    }

    bool start_with(const StringPiece& rhs) const
    {
        return ((m_length >= rhs.m_length) && \
                (memcmp(m_ptr, rhs.m_ptr, rhs.m_length)));
    }

private:
    const char* m_ptr;
    int         m_length;
};

}  // namespace mrpc

#ifdef HAVE_TYPE_TRAITS
// This makes vector<StringPiece> really fast for some STL implementations
template<> struct __type_traits<mrpc::StringPiece> {
  typedef __true_type    has_trivial_default_constructor;
  typedef __true_type    has_trivial_copy_constructor;
  typedef __true_type    has_trivial_assignment_operator;
  typedef __true_type    has_trivial_destructor;
  typedef __true_type    is_POD_type;
};
#endif  // HAVE_TYPE_TRAITSz

std::ostream& operator<<(std::ostream&o, const mrpc::StringPiece& piece);

#endif  // __MRPC_BASE_STRINGPIECE_H__