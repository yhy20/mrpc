#include "Util.h"
#include "Exception.h"

namespace mrpc
{

Exception::Exception(std::string msg)
    : m_message(std::move(msg)),
      m_stack(Util::StackTrace())
{ }


}  // namespace mrpc

