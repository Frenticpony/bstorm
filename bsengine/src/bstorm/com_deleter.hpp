#pragma once

#include <unknwn.h>

namespace bstorm
{
struct com_deleter
{
    void operator()(IUnknown* p)
    {
        p->Release();
    }
};
}
