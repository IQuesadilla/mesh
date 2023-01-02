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
typedef unsigned int Inode;
enum OPEN_FLAGS {
    READ,
    WRITE
};

enum FILE_TYPES {
    NORMAL,
    DIRECTORY,
    FIFO
};

struct file_meta {
    std::string path;
    uid_t uid;
    gid_t gid;
    mode_t mod;
    BLKID blkid;
    FILE_TYPES type;
    size_t size;
    size_t links;
};

template <class DEVID>
class meshfs
{
public:
    meshfs(DEVID id);
    ~meshfs();

    //int initfs(std::string config);
    int initfs();
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
    int updatecache  (DEVID id, bool remove, std::map<Inode,file_meta> *thedict, std::map<BLKID, size_t> *blks);
    int (*updatemesh)(DEVID id, bool remove, std::map<Inode,file_meta> *thedict, std::map<BLKID, size_t> *blks, void *userptr);

    void setuserptr(void *ptr) {userptr = ptr;}
    void *getuserptr(){return userptr;}

    void setmyid(DEVID id){myid = id;}
    DEVID getmyid(){return myid;}

    void dumptostdout();

private:
    struct {
        int BLKSIZE = 512;
    } fs_meta;

    bool exists = false;
    void *userptr;
    DEVID myid;

    Inode newinode();
    BLKID newblkid();
    Inode inodeatpath(std::string path);

    bool islocal(BLKID id);
    bool isonline(BLKID id);
    int blkcount(size_t count);
    int newblksize(int i, size_t count);

    std::map<std::string,Inode> open_files;
    std::map<Inode,file_meta> dict; 
    std::map<BLKID,std::vector<uint8_t*> > raw;
    std::map<DEVID,std::map<BLKID,size_t> > online;
};

#endif