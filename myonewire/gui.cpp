#include <ncurses.h>
#include <cstdio>
#include "wire.h"
#include <string>
#include <thread>
#include <list>

class myowm : public owm
{
public:
    int send_callback( char *data, size_t len )
    {
        int ret_len = send_buffer.length();
        if ( ret_len > len ) ret_len = len;

        int y, x;
        getmaxyx(stdscr,y,x);
        move(y-1, 0);
        clrtoeol();
        mvwprintw( stdscr, y-1, 0, "    Sending: %s", send_buffer.c_str());
        refresh();

        memcpy (data, send_buffer.c_str(), ret_len);
        send_buffer.erase( 0, ret_len );

        return ret_len; 
    }

    void recv_callback( char *data, size_t len )
    {
        recv_buffer.append( data, len );
    }

    std::string send_buffer;
    std::string recv_buffer;

    WINDOW *msgpanel;
    std::list<std::string> msglist;
    int selected;
    bool isSelectionMode;
};

void draw_msglist(myowm *mesh)
{
    int cur_y, cur_x, max_x, max_y;
    getyx(stdscr,cur_y,cur_x);
    getmaxyx(mesh->msgpanel, max_y, max_x);

    wclear(mesh->msgpanel);

    int line = 1;
    for ( auto &x : mesh->msglist )
    {
        wmove(mesh->msgpanel, line, 1);
        if ( mesh->isSelectionMode && mesh->msglist.size() - line == mesh->selected )
            wprintw(mesh->msgpanel, "  ");
        wprintw(mesh->msgpanel, "Size: %.4d, Recived line: %s", x.size(), x.c_str() );
        ++line;
    }

    wborder(mesh->msgpanel,0,0,0,0,0,0,0,0);
    wrefresh(mesh->msgpanel);

    move(cur_y,cur_x);
    refresh();
}

void recvfunc(myowm *mesh)
{
    while (1)
    {
        int msgid = mesh->recvmsg();
        if ( msgid < 0 ) continue;

        mesh->msglist.push_front(mesh->recv_buffer);
        mesh->recv_buffer = "";

        draw_msglist(mesh);
    }
}

int main()
{
    myowm mesh;

    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    int y, x;
    getmaxyx(stdscr, y, x);

    refresh();

    WINDOW *msgpanel = newwin(y - 1, x, 0, 0);
    wborder(msgpanel,0,0,0,0,0,0,0,0);
    wrefresh(msgpanel);
    //scrollok(msgpanel, true);

    mesh.selected = 0;
    mesh.isSelectionMode = false;

    mesh.msgpanel = msgpanel;

    std::thread recv_thread(recvfunc, &mesh);

    std::string input_buffer;

    int ch = 0;
    do
    {
        if (ch >= 32 && ch <= 126) // All typing characters
            input_buffer += ch;

        switch (ch)
        {
            case 127: // Delete
                input_buffer.pop_back();
            break;
            case 10: // Enter
                mesh.send_buffer = input_buffer;
                mesh.sendmsg();
                input_buffer.clear();
            break;
            case KEY_UP:
                if ( mesh.isSelectionMode == true )
                {
                    mesh.selected += 1;
                    if ( mesh.selected == mesh.msglist.size() )
                        mesh.isSelectionMode = false;
                    draw_msglist(&mesh);
                }
            break;
            case KEY_DOWN:
                if ( mesh.isSelectionMode == false )
                {
                    mesh.selected = mesh.msglist.size();
                    mesh.isSelectionMode = true;
                }

                mesh.selected -= 1;

                if (mesh.selected < 0)
                {
                    mesh.selected = 0;
                }
                draw_msglist(&mesh);
            break;
        }
        
        move(y-1, 0);
        clrtoeol();
        mvwprintw( stdscr, y-1, 0, "Enter value: %s", input_buffer.c_str());
        refresh();
    }
    while((ch = getch()) != 27); // Escape

    endwin();
}