#include "../../meshfs.h"
#include <thread>
#include <fstream>
#include <cstdlib>

#define FUSE_USE_VERSION 30

#include <errno.h>
#include <fuse3/fuse.h>
#include <unistd.h>
#include <iostream>
#if __has_include (<sys/xattr.h>)
    #include <sys/xattr.h>
    #define HAVE_SYS_XATTR_H
#endif

#include <sys/ipc.h>
#include <sys/shm.h>
#include <mutex>

#define MYDEVID std::string
bool firstname = true;
#define SHM_KEY 0x1234

#define TEST_DATA ((test_fuse::test_data *) fuse_get_context()->private_data)

struct shared_data
{
    std::mutex reqmtx;
    std::mutex respmtx;

    bool is_Inode;
    bool is_blkid;
    bool is_reqblks;

    BLKID req;
    uint8_t blks[512];

    MYDEVID id;
    bool remove;
    Inode thedict_Inode;
    file_meta thedict_file_meta;
    BLKID blks_blkid;
    size_t blks_size_t;

    bool loop_again;
};

int my_getblks(std::vector<BLKID> *req, std::map<BLKID, std::vector<uint8_t*> > *blks, void *userptr)
{
    std::cout << "Log: (my_getblks)" << std::endl;
    std::cout << "Value: Size of req: " << req->size() << std::endl;
    int ret = ((meshfs<MYDEVID>*)userptr)->reqblks(req,blks);
    std::cout << "Value: Size of blks: " << blks->size() << std::endl;
    return ret;
}

template <class DEVID>
int my_updatemesh(DEVID id, bool remove, std::map<Inode,file_meta> *thedict, std::map<BLKID, size_t> *blks, void *userptr)
{
    std::cout << "Log: (my_updatemesh)" << std::endl;
    std::cout << "Values: id=" << id << std::endl;

    auto it_thedict = thedict->cbegin(), end_thedict = thedict->cend();
    auto it_blks = blks->cbegin(), end_blks = blks->cend();
    for (; it_blks != end_blks && it_thedict != end_thedict;)
    {
        if (it_thedict != end_thedict)
        {
            it_thedict++;
        }

        if (it_blks != end_blks)
        {
            it_blks++;
        }
    }

    return ((meshfs<MYDEVID>*)userptr)->updatecache(id, remove, thedict, blks);
}

namespace test_fuse
{
    int getattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi);
    int readlink(const char *path, char *link, size_t size);
    int mknod(const char *path, mode_t mode, dev_t dev);
    int mkdir(const char *path, mode_t mode);
    int unlink(const char *path);
    int rmdir(const char *path);
    int symlink(const char *path, const char *link);
    int rename(const char *path, const char *newpath, unsigned int);
    int link(const char *path, const char *newpath);
    int chmod(const char *path, mode_t mode, struct fuse_file_info *fi);
    int chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi);
    int truncate(const char *path, off_t newsize, struct fuse_file_info *fi);
    int utime(const char *path, struct utimbuf *ubuf);
    int open(const char *path, fuse_file_info *fi);
    int read(const char *path, char *buf, size_t size, off_t offset, fuse_file_info *fi);
    int write(const char *path, const char *buf, size_t size, off_t offset, fuse_file_info *fi);
    int statfs(const char *path, struct statvfs *statv);
    int flush(const char *path, fuse_file_info *fi);
    int release(const char *path, fuse_file_info *fi);
    int fsync(const char *path, int datasync, fuse_file_info *fi);

    #ifdef HAVE_SYS_XATTR_H
    int setxattr(const char *path, const char *name, const char *value, size_t size, int flags);
    int getxattr(const char *path, const char *name, char *value, size_t);
    int listxattr(const char *path, char *list, size_t size);
    int removexattr(const char *path, const char *name);
    #endif

    int opendir(const char *path, fuse_file_info *fi);
    int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, fuse_file_info *fi, fuse_readdir_flags flags);
    int fsyncdir(const char *path, int datasync, fuse_file_info *fi);
    int releasedir(const char *path, fuse_file_info *fi);
    void *init(struct fuse_conn_info *conn, struct fuse_config *cfg);
    void destroy(void *userdata);
    int access(const char *path, int mask);
    int ftruncate(const char *path, off_t offset, fuse_file_info *fi);
    int fgetattr(const char *path, struct stat *statbuf, fuse_file_info *fi);

    static fuse_operations mfs_ops = {
        .getattr = test_fuse::getattr,
        .readlink = test_fuse::readlink,
        .mknod = test_fuse::mknod,
        .mkdir = test_fuse::mkdir,
        .unlink = test_fuse::unlink,
        .rmdir = test_fuse::rmdir,
        .symlink = test_fuse::symlink,
        .rename = test_fuse::rename,
        .link = test_fuse::link,
        .chmod = test_fuse::chmod,
        .chown = test_fuse::chown,
        .truncate = test_fuse::truncate,
//        .utime = nullptr,//test_fuse::utime,
        .open = test_fuse::open,
        .read = test_fuse::read,
        .write = test_fuse::write,
        /** Just a placeholder, don't set */ // huh???
        .statfs = test_fuse::statfs,
        .flush = test_fuse::flush,
        .release = test_fuse::release,
        .fsync = test_fuse::fsync,
        
        #ifdef HAVE_SYS_XATTR_H
        .setxattr = test_fuse::setxattr,
        .getxattr = test_fuse::getxattr,
        .listxattr = test_fuse::listxattr,
        .removexattr = test_fuse::removexattr,
        #endif
        
        .opendir = test_fuse::opendir,
        .readdir = test_fuse::readdir,
        .releasedir = test_fuse::releasedir,
        .fsyncdir = test_fuse::fsyncdir,
        .init = test_fuse::init,
        .destroy = test_fuse::destroy,
        .access = test_fuse::access,
        //.ftruncate = nullptr,//test_fuse::ftruncate,
        //.fgetattr = nullptr,//test_fuse::fgetattr

        //Fuse3
        .create = nullptr,
        .lock = nullptr,
        .utimens = nullptr,
        .bmap = nullptr,
        .ioctl = nullptr,
        .poll = nullptr,
        .write_buf = nullptr,
        .read_buf = nullptr,
        .flock = nullptr,
        .fallocate = nullptr,
        .copy_file_range = nullptr,
        .lseek = nullptr
    };

    struct test_data
    {
        std::shared_ptr<meshfs<MYDEVID> > mfs;
        std::string name;
        std::fstream tlog;
        std::shared_ptr<std::thread> thr;
        shared_data *shmp;
    };

    static void mfs_file_info_default(fuse_file_info* fi)
    {
        fi->direct_io = true;
        fi->keep_cache = false;
        fi->nonseekable = true;
    };
}

void the_thread(struct test_fuse::test_data *mydata)
{
    mydata->tlog << "Log: (thread)" << std::endl;

    int shm_id = shmget(SHM_KEY, sizeof(struct shared_data), 0644|IPC_CREAT);
    if (shm_id < 0)
    {
        mydata->tlog << "Error: Failed opening shared memory" << std::endl;
    }

    mydata->shmp = (shared_data*)shmat(shm_id, NULL, 0);

    while(1)
    {
        std::vector<BLKID> req;
        std::map<BLKID, std::vector<uint8_t*> > blks;

        MYDEVID id;
        bool remove;
        std::map<Inode,file_meta> thedict;
        std::map<BLKID, size_t> theblks;

        bool doloop;

        //Read data into req
        do
        {
            mydata->shmp->respmtx.unlock();
            mydata->shmp->reqmtx.lock();

            if (mydata->shmp->is_reqblks)
            {
                req.push_back(mydata->shmp->blks_blkid);
            }

            if (mydata->shmp->is_Inode)
            {
                thedict[mydata->shmp->thedict_Inode] = mydata->shmp->thedict_file_meta;
            }

            if (mydata->shmp->is_blkid)
            {
                theblks[mydata->shmp->blks_blkid] = mydata->shmp->blks_size_t;
            }

            doloop = mydata->shmp->loop_again;

            mydata->shmp->respmtx.lock();
            mydata->shmp->reqmtx.unlock();
        }
        while (doloop);

        if (mydata->shmp->is_reqblks)
        {
            mydata->mfs->reqblks(&req,&blks);
        }

        if (mydata->shmp->is_Inode || mydata->shmp->is_blkid)
        {
            mydata->mfs->updatecache(id, remove, &thedict, &theblks);
        }

        //Write data into blks
    }
}

int test_fuse::getattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (getattr)" << std::endl;
    statbuf->st_atim = {0};
    statbuf->st_blksize = 512;
    statbuf->st_blocks = 1;
    statbuf->st_ctim = {0};
    statbuf->st_dev = 1;
    statbuf->st_gid = 1000;
    statbuf->st_ino = 0;
    statbuf->st_mode = 0;
    statbuf->st_mtim = {0};
    statbuf->st_nlink = 1;
    statbuf->st_rdev = 0;
    statbuf->st_size = 512;
    statbuf->st_uid = 1000;
    return 0;
}

int test_fuse::readlink(const char *path, char *link, size_t size)
{
    TEST_DATA->tlog << "Log: Fuse: (readlink)" << std::endl;
    return ENOTSUP;
}

int test_fuse::mknod(const char *path, mode_t mode, dev_t dev)
{
    TEST_DATA->tlog << "Log: Fuse: (mknod)" << std::endl;
    // TODO
    return ENOTSUP;
}

int test_fuse::mkdir(const char *path, mode_t mode)
{
    TEST_DATA->tlog << "Log: Fuse: (mkdir)" << std::endl;
    return ENOTSUP;
}

int test_fuse::unlink(const char *path)
{
    TEST_DATA->tlog << "Log: Fuse: (unlink)" << std::endl;
    // TODO
    return ENOTSUP;
}

int test_fuse::rmdir(const char *path)
{
    TEST_DATA->tlog << "Log: Fuse: (rmdir)" << std::endl;
    return ENOTSUP;
}

int test_fuse::symlink(const char *path, const char *link)
{
    TEST_DATA->tlog << "Log: Fuse: (symlink)" << std::endl;
    return ENOTSUP;
}

int test_fuse::rename(const char *path, const char *newpath, unsigned int)
{
    TEST_DATA->tlog << "Log: Fuse: (rename)" << std::endl;
    return ENOTSUP;
}

int test_fuse::link(const char *path, const char *newpath)
{
    TEST_DATA->tlog << "Log: Fuse: (link)" << std::endl;
    return ENOTSUP;
}

int test_fuse::chmod(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (chmod)" << std::endl;
    return ENOTSUP;
}

int test_fuse::chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (chown)" << std::endl;
    return ENOTSUP;
}

int test_fuse::truncate(const char *path, off_t newsize, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (truncate)" << std::endl;
    return ENOTSUP;
}

int test_fuse::utime(const char *path, struct utimbuf *ubuf)
{
    TEST_DATA->tlog << "Log: Fuse: (utime)" << std::endl;
    return ENOTSUP;
}

int test_fuse::open(const char *path, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (open)" << std::endl;
    // TODO
    return ENOTSUP;
}

int test_fuse::read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (read)" << std::endl;
    // TODO
    return ENOTSUP;
}

int test_fuse::write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (write)" << std::endl;
    // TODO
    return ENOTSUP;
}

int test_fuse::statfs(const char *path, struct statvfs *statv)
{
    TEST_DATA->tlog << "Log: Fuse: (statfs)" << std::endl;
    // TODO
    return ENOTSUP;
}

int test_fuse::flush(const char *path, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (flush)" << std::endl;
    return ENOTSUP;
}

int test_fuse::release(const char *path, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (release)" << std::endl;
    // TODO
    return ENOTSUP;
}

int test_fuse::fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (fsync)" << std::endl;
    return ENOTSUP;
}

#ifdef HAVE_SYS_XATTR_H
int test_fuse::setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
    TEST_DATA->tlog << "Log: Fuse: (setxattr)" << std::endl;
    return ENOTSUP;
}

int test_fuse::getxattr(const char *path, const char *name, char *value, size_t)
{
    TEST_DATA->tlog << "Log: Fuse: (getxattr)" << std::endl;
    return ENOTSUP;
}

int test_fuse::listxattr(const char *path, char *list, size_t size)
{
    TEST_DATA->tlog << "Log: Fuse: (listxattr)" << std::endl;
    return ENOTSUP;
}

int test_fuse::removexattr(const char *path, const char *name)
{
    TEST_DATA->tlog << "Log: Fuse: (removexattr)" << std::endl;
    return ENOTSUP;
}
#endif

int test_fuse::opendir(const char *path, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (opendir)" << std::endl;
    // TODO
    return ENOTSUP;
}

int test_fuse::readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, fuse_readdir_flags flags)
{
    TEST_DATA->tlog << "Log: Fuse: (readdir)" << std::endl;
    // TODO
    return ENOTSUP;
}

int test_fuse::fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (fsyncdir)" << std::endl;
    return ENOTSUP;
}

int test_fuse::releasedir(const char *path, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (releasedir)" << std::endl;
    // TODO
    return ENOTSUP;
}

void *test_fuse::init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    TEST_DATA->tlog << "Log: (init)" << std::endl;
    TEST_DATA->mfs.reset(new meshfs<MYDEVID>(TEST_DATA->name));

    TEST_DATA->mfs->getblks= my_getblks;
    TEST_DATA->mfs->updatemesh = my_updatemesh;

    TEST_DATA->thr.reset(new std::thread(the_thread,TEST_DATA));

    return TEST_DATA;
}

void test_fuse::destroy(void *userdata)
{
    TEST_DATA->tlog << "Log: Fuse: (destroy)" << std::endl;
    delete TEST_DATA;
}

int test_fuse::access(const char *path, int mask)
{
    TEST_DATA->tlog << "Log: Fuse: (access)" << std::endl;
    return ENOTSUP;
}

int test_fuse::ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (ftruncate)" << std::endl;
    return ENOTSUP;
}

int test_fuse::fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    TEST_DATA->tlog << "Log: Fuse: (fgetattr)" << std::endl;
    return ENOTSUP;
}

int main(int argc, char *argv[])
{
    if ((getuid() == 0) || (geteuid() == 0)) {
    	std::cerr << "Running this test as root opens unnacceptable security holes" << std::endl;
    	return 1;
    }

    char *newpath[2] = {argv[1],argv[1]};

    test_fuse::test_data *data = new test_fuse::test_data;
    data->name = newpath[0];

    data->tlog.open ("mfslog.txt", std::fstream::out | std::fstream::app);
    data->tlog << std::endl << "Log: (main)" << std::endl;
    data->tlog << "Path: " << std::string(newpath[1]) << " Name: " << newpath[0] << std::endl;

    int mfs_stat = fuse_main(2, newpath, &test_fuse::mfs_ops, data);
    return 0;
}