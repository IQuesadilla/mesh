#include "../../meshfs.h"
#include <iostream>

#define MYDEVID std::string

int my_getblks(std::vector<BLKID> *req, std::map<BLKID, std::vector<uint8_t*> > *blks, void *userptr)
{
    return ((meshfs<std::string>*)userptr)->reqblks(req,blks);
}

int my_updatemesh(MYDEVID id, std::vector<BLKID, size_t> *blks, void *userptr)
{
    return ((meshfs<std::string>*)userptr)->updatecache(id, blks);
}

int main()
{
    std::shared_ptr<meshfs<MYDEVID> > mfs1;
    std::shared_ptr<meshfs<MYDEVID> > mfs2;

    mfs1.reset(new meshfs<MYDEVID>());
    mfs2.reset(new meshfs<MYDEVID>());

    mfs1->setuserptr(mfs2.get());
    mfs2->setuserptr(mfs1.get());

    mfs1->updatemesh = my_updatemesh;
    mfs2->updatemesh = my_updatemesh;
    mfs1->getblks = my_getblks;
    mfs2->getblks = my_getblks;

    mfs1->create("test.txt",FILE_TYPES::NORMAL);

    struct stat statbuf1;
    mfs1->stat("test.txt", &statbuf1);

    mfs1->open("test.txt", OPEN_FLAGS::WRITE);
    mfs1->write("test.txt", 0, statbuf1.st_size, (uint8_t*)"this is a test\n");
    mfs1->close("test.txt");

    struct stat statbuf2;
    mfs2->stat("test.txt", &statbuf2);

    char *buffer = (char*)malloc(statbuf2.st_size);
    mfs2->open("test.txt", OPEN_FLAGS::READ);
    mfs2->read("test.txt", 0, statbuf2.st_size, (uint8_t*)buffer);
    mfs2->close("test.txt");

    std::cout << "File contents - [" << buffer << "]" << std::endl;
    free(buffer);

    return 0;
}