#include "meshfs.h"
#include <cmath>
#include <cstring>
#include <iostream>

template class meshfs<std::string>;
template class meshfs<uint>;
//template class meshfs<void*>;

template <class DEVID>
meshfs<DEVID>::meshfs(DEVID id)
{
    myid = id;
    //if(!exists)
    //    initfs(id);
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
int meshfs<DEVID>::initfs()
{
    exists = true;

    for (auto &x : dict)
        if (x.second.path == "/")
            return 0;

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
    std::cout << "Log: (read)" << std::endl;
    if (open_files.find(path) == open_files.end())
    {
        std::cout << "Error: file requested was not opened" << std::endl;
        return -1;
    }

    file_meta fm = dict[open_files[path]];

    if (islocal(fm.blkid))
    {
        std::cout << "Log: Found the file locally" << std::endl;
        BLKID nblkid = fm.blkid + (int)floor((float)beg / (float)fs_meta.BLKSIZE);
        for (int i = 0; i < blkcount(count); ++i)
        {
            memcpy(&databuf[i * fs_meta.BLKSIZE],raw[nblkid + i].data(),newblksize(i,count));
        }
    }
    else if (isonline(fm.blkid))
    {
        std::cout << "Log: Found the file online" << std::endl;
        std::vector<BLKID> req;
        std::map<BLKID, std::vector<uint8_t*> > resp;
        BLKID nblkid = fm.blkid + (int)floor((float)beg / (float)fs_meta.BLKSIZE);
        for (int i = 0; i < blkcount(count); ++i)
        {
            req.push_back(nblkid + i);
        }

        std::cout << "Log: Requesting files from online" << std::endl;
        std::cout << "Value: Count: " << req.size() << std::endl;
        getblks(&req,&resp,userptr);

        for (int i = 0; i < blkcount(count); ++i)
        {
            memcpy(&databuf[i * fs_meta.BLKSIZE],resp[nblkid + i].data(),newblksize(i,count));
        }
    }
    else
    {
        std::cout << "Warning: Failed to find file locally or online" << std::endl;
        return -1;
    }
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::write(std::string path, uint64_t beg, uint64_t count, uint8_t *databuf)
{
    std::cout << "Log: (write)" << std::endl;
    if (open_files.find(path) == open_files.end())
    {
        std::cout << "Error: file requested was not opened" << std::endl;
        return -1;
    }

    std::map<Inode, file_meta> ndict;
    std::map<BLKID, size_t> nblks;

    file_meta fm = dict[open_files[path]];
    BLKID nblkid = fm.blkid + (int)floor(beg / fs_meta.BLKSIZE);
    for (int i = 0; i < blkcount(count); ++i)
    {
        // only works locally
        int newcount = newblksize(i,count);
        std::cout << "Values: nblkid: " << nblkid << " i: " << i << " nc: " << newcount << std::endl;
        raw[nblkid + i].resize(newcount);
        memcpy(raw[nblkid + i].data(),&databuf[i * fs_meta.BLKSIZE],newcount);
        nblks[nblkid + i] = newcount;
    }

    dict[open_files[path]].size = count;
    ndict[open_files[path]] = dict[open_files[path]];

    updatemesh(getmyid(),false,&ndict,&nblks,userptr);
    return 0;
}

template <class DEVID>
int meshfs<DEVID>::create(std::string path, FILE_TYPES flags)
{
    std::cout << "Log: (create)" << std::endl;
    std::cout << "Value: Path: " << path << std::endl;
    file_meta metadata;
    std::map<Inode,file_meta> ndict;
    std::map<BLKID, size_t> nblks;

    std::cout << "Log: Generating default metadata values" << std::endl;
    metadata.path = path;
    metadata.gid = 1000;
    metadata.uid = 1000;
    metadata.mod = 644;
    metadata.type = flags;
    metadata.blkid = newblkid();
    metadata.links = 1;
    metadata.size = fs_meta.BLKSIZE;

    nblks[metadata.blkid] = metadata.size;

    std::cout << "Log: Checking file type" << std::endl;
    switch (flags)
    {
        case FILE_TYPES::DIRECTORY:
            std::cout << "Log: Is a directory" << std::endl;
            break;
        case FILE_TYPES::NORMAL:
            std::cout << "Log: Generating new raw block" << std::endl;
            raw[metadata.blkid] = {};
    }

    std::cout << "Log: Generating new inode" << std::endl;
    Inode ni = newinode();

    std::cout << "Log: Inserting new inode into dictionary" << std::endl;
    dict.insert(std::make_pair(ni,metadata));

    std::cout << "Log: Insering new inode into dicionary to upload" << std::endl;
    ndict.insert(std::make_pair(ni,metadata));

    std::cout << "Log: Updating the mesh of the changes" << std::endl;
    updatemesh(getmyid(),false,&ndict,&nblks,userptr);

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
    info->st_blocks = blkcount(fm.size);

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
    std::cout << "Log: (updatecache)" << std::endl;
    std::cout << "Value: Size of blks: " << (*blks).size() << std::endl;
    std::cout << "Values: Size of thedict: " << (*thedict).size() << std::endl;
    if (remove)
    {
        std::cout << "Log: Remove Flag True" << std::endl;
        if (thedict->size() > 0)
        {
            std::cout << "Log: Altering dict" << std::endl;
            for (auto &x : *thedict)
                dict.erase(x.first);
        }
        if (blks->size() > 0)
        {
            std::cout << "Log: Altering online" << std::endl;
            for (auto &x : *blks)
                online[id].erase(x.first);
        }
    }
    else
    {
        std::cout << "Log: Remove Flag Not True" << std::endl;
        if (thedict->size() > 0)
        {
            std::cout << "Log: Altering dict" << std::endl;
            for (auto &x : *thedict)
                dict[x.first] = x.second;
        }
        if (blks->size() > 0)
        {
            std::cout << "Log: Altering online" << std::endl;
            for (auto &x : *blks)
                online[id][x.first] = x.second;
        }
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
    return dict.size();
}

template <class DEVID>
BLKID meshfs<DEVID>::newblkid()
{
    BLKID i = 0;
    for (; i < raw.size(); ++i)
        if (raw.find(i) == raw.end())
            return i;
    return raw.size();
}

template <class DEVID>
Inode meshfs<DEVID>::inodeatpath(std::string path)
{
    std::cout << "Log: (inodeatpath)" << std::endl;
    std::cout << "Value: Path: [" << path << "]" << std::endl;
    for (auto &x : dict)
        if (x.second.path == path)
        {
            std::cout << "Value: Inode: " << x.first << std::endl;
            return x.first;
        }

    std::cout << "Error: Failed to find Inode at given path, possible corruption" << std::endl;
    return dict.size()+1;
}

template <class DEVID>
bool meshfs<DEVID>::islocal(BLKID id)
{
    bool is = false;
    if (raw.find(id) != raw.end())
        is = true;

    std::cout << "Log: (islocal)" << std::endl;
    std::cout << "  Value: " << is << std::endl;
    
    return is;
}

template <class DEVID>
bool meshfs<DEVID>::isonline(BLKID id)
{
    bool is = false;
    for (auto &x : online)
        if (x.second.find(id) != x.second.end())
            is = true;

    std::cout << "Log: (isonline)" << std::endl;
    std::cout << "  Value: " << is << std::endl;
    
    return is;
}

template <class DEVID>
int meshfs<DEVID>::blkcount(size_t count)
{
    return (int)ceil((float)count / (float)fs_meta.BLKSIZE);
}

template <class DEVID>
int meshfs<DEVID>::newblksize(int i, size_t count)
{
    std::cout << "Log: (newblksize)" << std::endl;
    int temp = count - (i * fs_meta.BLKSIZE);

    if (temp <= 0)
    {
        std::cout << "Log: <= 0" << std::endl;
        return -1;
    }
    else if (temp >= fs_meta.BLKSIZE)
    {
        std::cout << "Log: >= BLKSIZE" << std::endl;
        return fs_meta.BLKSIZE;
    }
    else
    {
        std::cout << "Log: > BLKSIZE" << std::endl;
        return temp;
    }
}

template <class DEVID>
void meshfs<DEVID>::dumptostdout()
{
    std::cout << "Log: Start DUMP" << std::endl;
    std::cout << "Dump: Currently open files" << std::endl;
    for (auto &x : open_files)
        std::cout << 
        "  Path: [" << x.first << "]" << std::endl <<
        "    Inode: [" << x.second << "]" << std::endl;

    std::cout << "Dump: Entire Dictionary" << std::endl;
    for (auto &x : dict)
        std::cout <<
        "  Inode: [" << x.first << "]" << std::endl <<
        "    Path: [" << x.second.path << "]" << std::endl <<
        "    BLKID: [" << x.second.blkid << "]" << std::endl <<
        "    Size: [" << x.second.size << "]" << std::endl <<
        "    Links: [" << x.second.links << "]" << std::endl;

    std::cout << "Dump: Raw" << std::endl;
    for (auto &x : raw)
        std::cout <<
        "  BLKID: [" << x.first << "]" << std::endl;

    std::cout << "Dump: Online" << std::endl;
    for (auto &x : online)
    {
        std::cout << 
        "  DEVID: [" << x.first << "]" << std::endl;
        for (auto &y : x.second)
            std::cout << 
            "    BLKID: [" << y.first << "]" << std::endl <<
            "      Size: [" << y.second << "]" << std::endl;
    }

    std::cout << "Log: End DUMP" << std::endl;
}