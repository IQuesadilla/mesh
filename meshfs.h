#ifndef MESHFS_H
#define MESHFS_H
#pragma once

#include <string>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <bits/struct_stat.h>
#include <memory>
#include <thread>

#define DEFAULT_CONFIG_PATH "./config.xml";
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

template <class DEVID>
class meshfs
{
public:
    meshfs();
    ~meshfs();

    int initfs(std::string config);
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

    int stat(std::string path, struct stat *info);

    int reqblks   (std::vector<BLKID> *req, std::map<BLKID, std::vector<uint8_t*> > *blks);
    int (*getblks)(std::vector<BLKID> *req, std::map<BLKID, std::vector<uint8_t*> > *blks, void *userptr);
    int updatecache  (std::pair<DEVID, std::vector<BLKID, size_t> > *blks);
    int (*updatemesh)(std::pair<DEVID, std::vector<BLKID, size_t> > *blks, void *userptr);

    void setuserptr(void *ptr) {userptr = ptr;}
    void *getuserptr(){return userptr;}

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
    void *userptr;

    std::map<std::string,Inode> open_files;
    std::map<Inode,file_meta> dict; 
    std::map<BLKID,std::vector<uint8_t*> > raw;
    std::map<DEVID,std::pair<BLKID,size_t> > online;
};

#endif