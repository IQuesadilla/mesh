#include "meshfs.h"

template class meshfs<std::string>;
template class meshfs<uint>;
template class meshfs<void*>;

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
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::killfs()
{
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::importfs(std::string importpath)
{
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::exportfs(std::string exportfs)
{
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::open(std::string path, OPEN_FLAGS flags)
{
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::close(std::string path)
{
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::read(std::string path, uint64_t beg, uint64_t count, uint8_t *databuf)
{
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::write(std::string path, uint64_t beg, uint64_t count, uint8_t *databuf)
{
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::create(std::string path, FILE_TYPES flags)
{
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::link(std::string path, std::string oldfile)
{
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::unlink(std::string path)
{
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::stat(std::string path, struct stat *info)
{
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::reqblks(std::vector<BLKID> *req, std::map<BLKID, std::vector<uint8_t*> > *blks)
{
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::updatecache(std::pair<DEVID, std::vector<BLKID, size_t> > *blks)
{
    return 0;
}