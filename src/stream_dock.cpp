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
#include<limits>

void swap_bytes(unsigned char *data, int size) {
    for (int i = 0; i < size/2; ++i) {
        std::swap(data[i], data[size-i-1]);
    }
}

void dump_str(unsigned char *data, int size) {
    for (int i=0; i < size; ++i) {
        std::cout << (int) data[i] << " ";
    }
    std::cout << std::endl;
}

StreamDock::StreamDock() {
        hid_init();
        hid = hid_open(vid, pid, NULL);
        if (hid == nullptr) {
            good = false;
        } else {
            hid_set_nonblocking(hid, 1);
        }
};

bool StreamDock::set_brightness(int brightness) {
    if (0 > brightness || brightness > 100) {
        std::cout << "Invalid Brightness" << std::endl;
        return false;
    }
    unsigned char out[13] = GEN_PACKET(BRIGHTNESS_PACKET SPACER_PACKET (unsigned char) brightness);
    hid_write(this->hid,out, PACKET_SIZE + 1);
    this->brightness = brightness;
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
    unsigned char out[13] = GEN_PACKET(SET_CELL_IMG SPACER_PACKET 1, 0, KEY_1);

    hid_write(this->hid, out, PACKET_SIZE + 1);
    unsigned char out2[PACKET_SIZE + 1];
    memset(out2, 0, PACKET_SIZE+1);
    hid_write(this->hid, out2, PACKET_SIZE + 1);
    return true;
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
        // thank you to my friend for find this extremely cursed code
        // all it does is turns 1-5 -> 11-15, 6-10 -> 6-10, 11-15 -> 1-5
        // ie "vertically flipping" the keys
        real_key = ((key - 1) % 5) + 5 * (2 - (key - 1) / 5) + 1;
    }
    //std::cout << "awa" << (int) key << std::endl;
    std::ifstream file_stream;
    file_stream.open(path);
    file_stream.ignore( std::numeric_limits<std::streamsize>::max() );
    std::streamsize jpg_length = file_stream.gcount();
    file_stream.clear();   //  Since ignore will have set eof.
    file_stream.seekg( 0, std::ios_base::beg );
    unsigned char size_bigger = jpg_length >> 8;
    unsigned char size_smaller = jpg_length % 256; 
    unsigned char out[PACKET_SIZE + 1] = GEN_PACKET(SET_CELL_IMG SPACER_PACKET size_bigger, size_smaller, real_key);
    hid_write(this->hid, out, PACKET_SIZE + 1);
    //std::cout << "uwu: " << length << std::endl;
    unsigned char img_out[PACKET_SIZE + 1] = {'\0'};
    while (file_stream.good()) {
        memset(img_out+1, 0, PACKET_SIZE);
        file_stream.read((char*)(&img_out[1]), PACKET_SIZE);
        //dump_str(img_out, PACKET_SIZE);
        hid_write(this->hid, img_out, PACKET_SIZE + 1);
    }
    return true;
}


bool StreamDock::clear_cell_background(enum key key) {
    // this really shouldn't be neccesary but for some reason
    // the key is vertically flipped
    //
    // also this sucks generally since it doesn't fully clear the clear_cell_background
    unsigned char real_key;
    if (key == 0xff) {
        real_key = key;
    } else {
        real_key = ((key - 1) % 5) + 5 * (2 - (key - 1) / 5) + 1;
    }
    unsigned char out[PACKET_SIZE + 1] =  GEN_PACKET(CLEAR_PACKET SPACER_PACKET 0, real_key);
    hid_write(this->hid, out, PACKET_SIZE + 1);
    return true;
}

bool StreamDock::clear_full_background() {
    return clear_cell_background(ALL_KEYS);
}

struct key_input StreamDock::read() {
    struct key_input key = {ALL_KEYS, true};
    if (!good) return key;
    int out = hid_read(this->hid, this->buf, PACKET_SIZE + 1);
    good = out != -1;
    //dump_str(this->buf, PACKET_SIZE + 1);
    if (out == 0) { // no key_input
        key.key = ALL_KEYS;
    } else {
        key.key = static_cast<enum key>(buf[9]);
        key.down = buf[10] == 0x01;
    }
    //if (key.key != ALL_KEYS) {
    //    dump_str(buf, PACKET_SIZE + 1);
    //}
    return key;
}

bool StreamDock::is_good() {
    return good;
}

bool StreamDock::is_screen_on() {
    return screen_on;
}

int StreamDock::get_brightness() {
    return brightness;
}
