#include "meshfs.h"
#include <cmath>
#include <cstring>

template class meshfs<std::string>;
template class meshfs<uint>;
//template class meshfs<void*>;

template <class DEVID>
meshfs<DEVID>::meshfs(DEVID id)
{
    if(!exists)
        initfs(id);
}

template <class DEVID>
meshfs<DEVID>::~meshfs()
{
    if (exists)
        killfs();
}
/*
template <class DEVID>
int meshfs<DEVID>::initfs(std::string config)
{
    initfs("null");

    return 0;
}
*/
template <class DEVID>
int meshfs<DEVID>::initfs(DEVID id)
{
    exists = true;

    create("/",FILE_TYPES::DIRECTORY);

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
    open_files.insert(std::make_pair(path,inodeatpath(path)));
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::close(std::string path)
{
    open_files.erase(path);
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::read(std::string path, uint64_t beg, uint64_t count, uint8_t *databuf)
{
    file_meta fm = dict[open_files[path]];
    BLKID nblkid = fm.blkid + (int)floor(beg / fs_meta.BLKSIZE);
    for (int i = 1; i < (int)ceil(count / fs_meta.BLKSIZE) - 1; ++i)
        // was working here
        memcpy(databuf[],raw[nblkid + i].data(),fs_meta.BLKSIZE);
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
    file_meta metadata;
    std::map<Inode,file_meta> ndict;
    std::map<BLKID, size_t> nraw;

    metadata.gid = 1000;
    metadata.uid = 1000;
    metadata.mod = 644;
    metadata.type = flags;
    metadata.blkid = newblkid();
    metadata.links = 1;

    switch (flags)
    {
        case FILE_TYPES::DIRECTORY:
            break;
    }

    Inode ni = newinode();
    dict.insert(std::make_pair(ni,metadata));

    ndict.insert(std::make_pair(ni,metadata));
    updatemesh(getmyid(),false,&ndict,&nraw,userptr);

    return 0;
}

template <class DEVID>
int meshfs<DEVID>::link(std::string path, std::string oldfile)
{
    Inode oi = inodeatpath(oldfile);
    Inode ni = newinode();

    file_meta nfm = dict[ni];
    dict[oi].links += 1;

    nfm.links += 1;
    nfm.path = path;

    dict.insert(std::make_pair(ni,nfm));
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::unlink(std::string path)
{
    dict.erase(inodeatpath(path));
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::stat(std::string path, struct stat *info)
{
    Inode node = inodeatpath(path);
    file_meta fm = dict[node];
    info->st_uid = fm.uid;
    info->st_gid = fm.gid;

    info->st_ctim = {0,0};
    info->st_atim = {0,0};
    info->st_mtim = {0,0};

    info->st_nlink = fm.links;
    info->st_size = fm.size;
    info->st_blksize = fs_meta.BLKSIZE;
    info->st_blocks = (int)ceil((float)fm.size / (float)fs_meta.BLKSIZE);

    info->st_ino = node;
    info->st_mode = fm.mod;
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::reqblks(std::vector<BLKID> *req, std::map<BLKID, std::vector<uint8_t*> > *blks)
{
    for (auto &x : *req)
        blks->insert(std::make_pair(x,raw[x]));
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::updatecache(DEVID id, bool remove, std::map<Inode,file_meta> *thedict, std::map<BLKID, size_t> *blks)
{
    if (remove)
    {
        if (thedict->size() > 0)
            for (auto &x : *thedict)
                dict.erase(x.first);
        if (blks->size() > 0)
            for (auto &x : *blks)
                raw.erase(x.first);
    }
    else
    {
        if (thedict->size() > 0)
            dict.insert(thedict->begin(),thedict->end());
        if (blks->size() > 0)
            raw.insert(blks->begin(),blks->end());
    }
    return 0;
}

template <class DEVID>
Inode meshfs<DEVID>::newinode()
{
    Inode i = 0;
    for (; i < dict.size(); ++i)
        if (dict.find(i) == dict.end())
            return i;
    return i+1;
}

template <class DEVID>
BLKID meshfs<DEVID>::newblkid()
{
    BLKID i = 0;
    for (; i < raw.size(); ++i)
        if (raw.find(i) == raw.end())
            return i;
    return i+1;
}

template <class DEVID>
Inode meshfs<DEVID>::inodeatpath(std::string path)
{
    for (auto &x : dict)
        if (x.second.path == path)
            return x.first;
    return dict.size()+1;
}