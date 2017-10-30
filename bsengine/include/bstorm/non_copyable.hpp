#pragma once

namespace bstorm {
  class NonCopyable {
  protected:
    NonCopyable() {}
    ~NonCopyable() {}
  private:
    void operator =(const NonCopyable& src);
    NonCopyable(const NonCopyable& src);
  };
}
