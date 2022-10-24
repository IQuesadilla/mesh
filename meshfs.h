#ifndef MESHFS_H
#define MESHFS_H
#pragma once

#include <string>
#include <vector>
#include <map>
#include <bits/struct_stat.h>
#include <memory>
#include <thread>

#define DEFAULT_CONFIG_PATH "./config.xml"

template <class DEVID>
class meshfs
{
public:
    typedef unsigned int BLKID;
    enum OPEN_FLAGS {
        READ,
        WRITE
    };

    enum FILE_TYPES {
        NORMAL,
        DIRECTORY,
        FIFO
    };

    meshfs();
    ~meshfs();

    int initfs(std::string config = DEFAULT_CONFIG_PATH);
    int killfs();

    int importfs(std::string importpath);
    int exportfs(std::string exportpath);

    int open (std::string path, OPEN_FLAGS flags);
    int close(std::string path);
    int read (std::string path, uint64_t beg, uint64_t count, uint8_t *databuf);
    int write(std::string path, uint64_t beg, uint64_t count, uint8_t *databuf);

    int create(std::string path, FILE_TYPES flags);
    int link  (std::string path, std::string oldfile);
    int unlink(std::string path);

    int stat(std::string path, stat *info);

    int hasblks(std::vector<BLKID> req, std::map<DEVID, std::vector<BLKID,size_t> > whohas);
    int getblks(std::vector<BLKID> req, std::map<BLKID,std::vector<uint8_t*> > *blks);
    int updateonline(std::map<DEVID, std::vector<BLKID,size_t> > blks);

private:
    typedef unsigned int Inode;
    struct file_meta {
        std::string path;
        uid_t uid;
        gid_t gid;
        mode_t mod;
        BLKID blkid;
    };

    struct {
        int BLKSIZE;
    } fs_meta;

    bool exists = false;

    std::map<std::string,Inode> open_files;
    std::map<Inode,file_meta> dict; 
    std::map<BLKID,std::vector<uint8_t*> > raw;
    std::map<DEVID,std::pair<BLKID,size_t> > online;
};

#endif