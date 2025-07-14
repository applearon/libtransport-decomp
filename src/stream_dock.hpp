#ifndef STREAM_DOCK_H
#define STREAM_DOCK_H
#include<hidapi/hidapi_libusb.h>
#include"packets.hpp"
#include<string>
constexpr size_t PACKET_SIZE = 512;

class StreamDock {
    private:
        int vid = 0x5500;
        int pid = 0x1001;
        unsigned char buf[513];
        bool screen_on = true;
        bool good = true;
        int brightness = 100;
    public:
        hid_device *hid = nullptr;
        StreamDock();
        bool refresh();
        bool set_brightness(int brightness); // between 0-100?
        struct key_input read();
        bool toggle_screen(bool on);
        bool toggle_screen();
        bool set_full_background(unsigned char *img_buf);
        bool set_cell_background(enum key key, std::string path);
        bool set_cell_background(enum key key, unsigned char *img_buf, unsigned int length);
        bool clear_full_background();
        bool clear_cell_background(enum key key);
        bool send_wakeup();
        bool is_good();
        bool is_screen_on();
        int get_brightness();
};

#endif
