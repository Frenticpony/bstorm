#pragma once

namespace bstorm
{
template <class T>
void safe_delete(T*& p)
{
    delete p;
    p = nullptr;
};

template <class T>
void safe_delete_array(T*& p)
{
    delete[] p;
    p = nullptr;
};

template <class T>
void safe_release(T*& p)
{
    if (p)
    {
        p->Release();
        p = nullptr;
    }
};
}