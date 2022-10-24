#include "meshfs.h"

template <class DEVID>
meshfs<DEVID>::meshfs()
{
    //
}

template <class DEVID>
meshfs<DEVID>::~meshfs()
{
    if (exists)
        killfs();
}

template <class DEVID>
int meshfs<DEVID>::initfs(std::string config)
{
    exists = true;
}