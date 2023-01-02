#include "../../meshfs.h"
#include <iostream>
#include <cstring>

#define MYDEVID std::string

const char *myrandomdata = "\
This is a bunch of random data used to test the existence of many blks. \n\
Thus, I will use the \"common\" random data: \n\
  Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Viverra suspendisse potenti nullam ac. Praesent tristique magna sit amet purus gravida quis blandit. Magna sit amet purus gravida. Augue ut lectus arcu bibendum at varius vel pharetra. Ultricies leo integer malesuada nunc vel risus commodo viverra maecenas. Quis risus sed vulputate odio ut enim. Quis varius quam quisque id diam vel quam elementum pulvinar. Non consectetur a erat nam at lectus urna duis. Aliquam ultrices sagittis orci a scelerisque. Vehicula ipsum a arcu cursus vitae congue. Condimentum id venenatis a condimentum vitae sapien pellentesque. Nunc aliquet bibendum enim facilisis.\n\
  Urna nec tincidunt praesent semper feugiat. Et netus et malesuada fames ac turpis egestas. Id velit ut tortor pretium viverra suspendisse potenti nullam ac. Sed cras ornare arcu dui vivamus arcu. Consequat ac felis donec et odio pellentesque. Enim nunc faucibus a pellentesque sit amet porttitor eget. Egestas sed sed risus pretium quam vulputate dignissim suspendisse in. Blandit massa enim nec dui nunc mattis enim ut. Est lorem ipsum dolor sit amet consectetur adipiscing elit pellentesque. Viverra suspendisse potenti nullam ac tortor vitae purus faucibus ornare.\n\
  Nisl vel pretium lectus quam id leo. Commodo quis imperdiet massa tincidunt nunc. Tempus quam pellentesque nec nam aliquam sem et tortor consequat. In mollis nunc sed id. Auctor augue mauris augue neque gravida in fermentum. Aliquet eget sit amet tellus cras adipiscing enim eu turpis. Viverra justo nec ultrices dui sapien eget. Velit aliquet sagittis id consectetur purus. Duis convallis convallis tellus id interdum. At urna condimentum mattis pellentesque. Habitant morbi tristique senectus et netus et malesuada fames ac. Aliquam sem et tortor consequat id porta nibh venenatis. Etiam erat velit scelerisque in dictum non consectetur a erat. Neque gravida in fermentum et sollicitudin ac orci. At volutpat diam ut venenatis tellus in metus vulputate eu. Elit ut aliquam purus sit amet luctus venenatis lectus.\n\
  Magna fermentum iaculis eu non diam phasellus vestibulum lorem sed. Libero enim sed faucibus turpis in. Velit euismod in pellentesque massa placerat duis ultricies lacus. Integer quis auctor elit sed vulputate mi. Mi quis hendrerit dolor magna. Pellentesque adipiscing commodo elit at imperdiet dui accumsan sit amet. Vitae justo eget magna fermentum iaculis eu non. Lorem mollis aliquam ut porttitor leo a diam sollicitudin tempor. Adipiscing commodo elit at imperdiet dui accumsan sit. Convallis posuere morbi leo urna molestie at elementum.\n\
  Cras sed felis eget velit. Neque ornare aenean euismod elementum nisi quis eleifend quam adipiscing. Dictumst vestibulum rhoncus est pellentesque elit. Dictum varius duis at consectetur lorem donec massa sapien faucibus. Est ante in nibh mauris cursus. Praesent elementum facilisis leo vel fringilla est ullamcorper. Elit duis tristique sollicitudin nibh sit amet. Consectetur purus ut faucibus pulvinar elementum. Sollicitudin ac orci phasellus egestas tellus rutrum tellus pellentesque. Eget mi proin sed libero enim sed faucibus turpis in.\n\
  Tellus molestie nunc non blandit massa enim nec dui nunc. Tempor id eu nisl nunc mi ipsum. Id diam vel quam elementum. Urna nunc id cursus metus aliquam eleifend mi. Duis convallis convallis tellus id interdum velit laoreet id donec. Ut consequat semper viverra nam. Molestie a iaculis at erat pellentesque adipiscing. Faucibus et molestie ac feugiat sed lectus vestibulum. Elit duis tristique sollicitudin nibh sit amet commodo. Enim sit amet venenatis urna cursus eget nunc. Amet nisl purus in mollis nunc sed. In mollis nunc sed id semper risus in hendrerit gravida. Odio pellentesque diam volutpat commodo. Eget gravida cum sociis natoque penatibus et magnis dis.\n\
  Cras ornare arcu dui vivamus arcu felis. Orci eu lobortis elementum nibh tellus molestie. Leo vel fringilla est ullamcorper eget nulla facilisi. Fames ac turpis egestas sed tempus urna et pharetra pharetra. Ultrices tincidunt arcu non sodales neque sodales. Vulputate dignissim suspendisse in est ante in nibh. Sed euismod nisi porta lorem mollis aliquam ut porttitor leo. Enim sit amet venenatis urna cursus. Risus viverra adipiscing at in tellus integer feugiat scelerisque. Viverra ipsum nunc aliquet bibendum enim facilisis gravida neque convallis. Ultrices vitae auctor eu augue. Ante in nibh mauris cursus mattis molestie a. Risus ultricies tristique nulla aliquet enim tortor at auctor urna. Vivamus arcu felis bibendum ut tristique et egestas quis ipsum. Nibh mauris cursus mattis molestie. Vitae nunc sed velit dignissim sodales. Sodales neque sodales ut etiam sit amet. Hac habitasse platea dictumst quisque.\n\
  Cursus vitae congue mauris rhoncus aenean vel elit. Vulputate dignissim suspendisse in est ante in nibh. Ut sem viverra aliquet eget. Laoreet non curabitur gravida arcu ac tortor dignissim. Eget lorem dolor sed viverra ipsum nunc aliquet. Proin libero nunc consequat interdum. Id venenatis a condimentum vitae sapien. Fermentum dui faucibus in ornare quam viverra orci sagittis. Pellentesque massa placerat duis ultricies lacus sed turpis. Sem viverra aliquet eget sit amet tellus. Nisl nunc mi ipsum faucibus vitae aliquet nec. Arcu non odio euismod lacinia at quis risus sed. Nibh venenatis cras sed felis eget velit aliquet sagittis. Sociis natoque penatibus et magnis dis parturient.\n\
  Nisi vitae suscipit tellus mauris a diam. Ullamcorper dignissim cras tincidunt lobortis feugiat vivamus at. Fusce id velit ut tortor pretium viverra. Libero nunc consequat interdum varius sit amet mattis vulputate enim. Pellentesque elit ullamcorper dignissim cras tincidunt lobortis feugiat. Est velit egestas dui id ornare arcu. Habitasse platea dictumst quisque sagittis purus sit amet volutpat consequat. Non sodales neque sodales ut etiam sit amet nisl purus. Aliquet risus feugiat in ante metus dictum at tempor. Mattis pellentesque id nibh tortor. Nulla facilisi etiam dignissim diam. Ac ut consequat semper viverra nam libero. Pretium viverra suspendisse potenti nullam ac tortor vitae. Amet tellus cras adipiscing enim eu turpis egestas pretium aenean. Duis tristique sollicitudin nibh sit amet commodo nulla facilisi nullam. Id faucibus nisl tincidunt eget nullam. Amet risus nullam eget felis eget. Tincidunt tortor aliquam nulla facilisi.\n\
  Amet consectetur adipiscing elit ut aliquam purus sit amet. Eleifend donec pretium vulputate sapien nec sagittis aliquam malesuada. Morbi enim nunc faucibus a pellentesque sit amet porttitor. Commodo odio aenean sed adipiscing diam donec adipiscing. Placerat duis ultricies lacus sed turpis tincidunt id aliquet. Donec et odio pellentesque diam volutpat commodo sed egestas. At volutpat diam ut venenatis. Aliquet sagittis id consectetur purus ut faucibus pulvinar elementum integer. Eget lorem dolor sed viverra. Id eu nisl nunc mi ipsum. Facilisi cras fermentum odio eu.";

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
    return ((meshfs<MYDEVID>*)userptr)->updatecache(id, remove, thedict, blks);
}

int main()
{
    std::cout << "Log: Creating the pointers for the devices" << std::endl;
    std::shared_ptr<meshfs<MYDEVID> > mfs1;
    std::shared_ptr<meshfs<MYDEVID> > mfs2;

    std::cout << "Log: Creating the devices" << std::endl;
    mfs1.reset(new meshfs<MYDEVID>("mfs1"));
    mfs2.reset(new meshfs<MYDEVID>("mfs2"));

    std::cout << "Log: Setting userptr on each to the other" << std::endl;
    mfs1->setuserptr(mfs2.get());
    mfs2->setuserptr(mfs1.get());

    std::cout << "Log: Setting update functions" << std::endl;
    mfs1->updatemesh = my_updatemesh;
    mfs2->updatemesh = my_updatemesh;
    mfs1->getblks = my_getblks;
    mfs2->getblks = my_getblks;

    std::cout << "Log: Running initfs on both" << std::endl;
    mfs1->initfs();
    mfs2->initfs();

    std::cout << "Log: Creating the first file" << std::endl;
    mfs1->create("test.txt",FILE_TYPES::NORMAL);

    std::cout << "Log: Attempting a stat on the new file" << std::endl;
    struct stat statbuf1;
    mfs1->stat("test.txt", &statbuf1);

    mfs1->dumptostdout();

    mfs1->open("test.txt", OPEN_FLAGS::WRITE);
    mfs1->write("test.txt", 0, strlen(myrandomdata), (uint8_t*)myrandomdata);
    mfs1->close("test.txt");

    struct stat statbuf2;
    mfs2->stat("test.txt", &statbuf2);

    std::cout << "Log: Ready to open and read the new file" << std::endl;
    std::cout << "Log: Length: " << statbuf2.st_size << std::endl;

    char *buffer = (char*)malloc(statbuf2.st_size);
    memset(buffer,(int)',',statbuf2.st_size);
    mfs2->open("test.txt", OPEN_FLAGS::READ);
    mfs2->read("test.txt", 0, statbuf2.st_size, (uint8_t*)buffer);
    mfs2->close("test.txt");
    buffer[statbuf2.st_size] = '\0';

    mfs1->dumptostdout();
    mfs2->dumptostdout();

    std::cout << "Value: File Size: " << statbuf2.st_size << std::endl;
    std::cout << "Raw Data: [" << buffer << "]" << std::endl;
    free(buffer);

    return 0;
}