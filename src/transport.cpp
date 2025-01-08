#include <hidapi/hidapi.h>
#include <hidapi/hidapi_libusb.h>
#include<iostream>
#include "stream_dock.hpp"
#include"packets.hpp"
#include<chrono>
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

#define MAX_STR 255
int main(void) {
    StreamDock *dock = new StreamDock();
    dock->refresh();
    dock->set_brightness(100);
    int x,y,n;
    unsigned char *data = stbi_load("flower_power.png", &x, &y, &n, 3);
    if (data == NULL) {
        std::cout << "Failed to load image." <<std::endl;
        return 1;
    }
    std::cout << "Setting the background image..." << std::endl;
    const auto start = std::chrono::system_clock::now();
    dock->refresh();
    dock->set_full_background(data);
    const auto end = std::chrono::system_clock::now();
    stbi_image_free(data);
    const std::chrono::duration<double> diff = end - start;
    std::cout << "Loading the image took " << diff.count() << " seconds." << std::endl;
    dock->refresh();
    std::cout << "Starting Loop" << std::endl;
    while (true) {
        struct key_input key = dock->read();
        std::cout << "Key " << key.key << " pressed";
        if (key.down) {
            std::cout << " down." << std::endl;
        } else {
            std::cout << " up." << std::endl;
        }
        if (key.key == 15) {
            dock->toggle_screen();
         } else if (key.key == 14 && !key.down) {
            dock->clear_cell_background(key.key);
        }
    }
    return 0;
}
