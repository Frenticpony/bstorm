#pragma once

struct com_deleter {
  template <class T>
  void operator()(T* p) {
    if (p) p->Release();
  }
};