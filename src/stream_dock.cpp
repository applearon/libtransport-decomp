#include <hidapi/hidapi.h>
#include <wchar.h>
#include <hidapi/hidapi_libusb.h>
#include<iostream>
#include"stream_dock.hpp"
#include "packets.hpp"
#include<memory.h>
//#include<vector>
#include<fstream>
#include<string>

void swap_bytes(unsigned char *data, int size) {
    for (int i = 0; i < size/2; ++i) {
        std::swap(data[i], data[size-i-1]);
    }
}

StreamDock::StreamDock() {
        hid_init();
        hid = hid_open(vid, pid, NULL);
        if(hid == nullptr) {
            std::cout << "Failed to open device" << std::endl;
hid_exit();
            exit(1);
        };
        hid_set_nonblocking(hid, 0);
};

bool StreamDock::set_brightness(int brightness) {
    if (0 > brightness || brightness > 100) {
        std::cout << "Invalid Brightness" << std::endl;
        return false;
    }
    unsigned char out[13] = GEN_PACKET(BRIGHTNESS_PACKET SPACER_PACKET (unsigned char) brightness);
    hid_write(this->hid,out, PACKET_SIZE + 1);
    return true;
}

bool StreamDock::refresh() {
    unsigned char out[13] = GEN_PACKET(REFRESH_PACKET);
    hid_write(this->hid, out, PACKET_SIZE + 1);
    return true;
}

bool StreamDock::toggle_screen(bool on) {
    this->screen_on = on;
    if (on) {
        unsigned char out[13] = GEN_PACKET(SCREEN_TOGGLE_ON_PACKET);
        hid_write(this->hid, out, PACKET_SIZE + 1);
    } else {
        unsigned char out[13] = GEN_PACKET(SCREEN_TOGGLE_OFF_PACKET);
        hid_write(this->hid, out, PACKET_SIZE + 1);
    }
    return true;
}

bool StreamDock::toggle_screen() {
    return toggle_screen(!this->screen_on);
}

bool StreamDock::send_wakeup() {
    return toggle_screen(true);
}

bool StreamDock::set_full_background(unsigned char *img_buf) {
    unsigned char out[PACKET_SIZE + 1] = {BEGIN_PACKET SPACER_PACKET BACKGROUND_IMG_PACKET 0, 17, 148, 0, 1};
    hid_write(this->hid, out, PACKET_SIZE + 1);
    int total_size = 480*800*3;
    unsigned char img_out[PACKET_SIZE + 1] = {'\0'};
    swap_bytes(img_buf, total_size);
    for (int i = 0; i <= total_size; i += PACKET_SIZE) {
        memcpy(img_out+1, img_buf + i, PACKET_SIZE);
        hid_write(this->hid, img_out, PACKET_SIZE + 1);
    }
    return true;
}

bool StreamDock::set_cell_background(enum key key, std::string path) {
        unsigned char real_key;
    if (key == 0xff) {
        real_key = key;
    } else {
        real_key = ((key - 1) % 5) + 5 * (2 - (key - 1) / 5) + 1;
    }
    unsigned char out[PACKET_SIZE + 1] = GEN_PACKET(SET_CELL_IMG SPACER_PACKET 14, 149, real_key);
    hid_write(this->hid, out, PACKET_SIZE + 1);
    std::ifstream file_stream;
    file_stream.open(path);
    unsigned char img_out[PACKET_SIZE + 1] = {'\0'};
    while (file_stream.good()) {
        memset(img_out+1, 0, PACKET_SIZE);
        file_stream.read((char*)(&img_out[1]), PACKET_SIZE);
        std::cout << "uwu: " << img_out << (int) img_out[PACKET_SIZE] << std::endl;
        hid_write(this->hid, img_out, PACKET_SIZE + 1);
    }
    return true;
}


bool StreamDock::clear_cell_background(enum key key) {
    // this really shouldn't be neccesarily but for some reason
    // the key is vertically flipped
    //
    // also this sucks generally since it doesn't fully clear the clear_cell_background
    unsigned char real_key;
    if (key == 0xff) {
        real_key = key;
    } else {
        real_key = ((key - 1) % 5) + 5 * (2 - (key - 1) / 5) + 1;
    }
    unsigned char out[PACKET_SIZE + 1] =  GEN_PACKET(CLEAR_PACKET SPACER_PACKET 0, (unsigned char) real_key);
    hid_write(this->hid, out, PACKET_SIZE + 1);
    return true;
}

bool StreamDock::clear_full_background() {
    return clear_cell_background(ALL_KEYS);
}

struct key_input StreamDock::read() {
    struct key_input key;
    hid_read(this->hid, this->buf, PACKET_SIZE + 1);
    key.key = static_cast<enum key>(buf[9]);
    key.down = buf[10] == 0x01;
    return key;
}
