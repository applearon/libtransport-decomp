#include <hidapi/hidapi.h>
#include <hidapi/hidapi_libusb.h>
#include<iostream>
#include "stream_dock.hpp"
#include"packets.hpp"
#include<chrono>
//#include<thread>
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

#define MAX_STR 255

void bad_apple(StreamDock *dock) {
    const size_t total_frames = 6572;
    const auto ms_per_frame = std::chrono::nanoseconds(33333333);
    const auto begin = std::chrono::system_clock::now();
    std::chrono::duration<double> diff;
    size_t next_frame = 1;
    size_t last_frame = next_frame;
    while (true) {
            const auto start = std::chrono::system_clock::now();
            diff = start - begin;
            last_frame = next_frame;
            next_frame = diff / ms_per_frame;
            if (last_frame == next_frame) {
                continue;
            }
            std::cout << next_frame << std::endl;
            if (next_frame > total_frames) {
                break;
            }
            std::string path = "bad_apple/out" + std::to_string(next_frame) + ".jpg";
            dock->set_cell_background(KEY_8, path);
            dock->refresh();
            //const auto end = std::chrono::system_clock::now();
            //diff = end - start;
            //std::cout << diff.count() << std::endl;
            //std::this_thread::sleep_for(std::chrono::milliseconds(33) - diff);
    }
}

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
    dock->send_wakeup();
    //dock->set_full_background(data);
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
        } else if (key.key == 13 && !key.down) {
            const auto start = std::chrono::system_clock::now();
            //for (int x = 1; x < 16; ++x) {
            //    dock->set_cell_background(static_cast<enum key>(x), "SonicMiku-test.jpg");
            //}
            //dock->set_cell_background(KEY_11, "tall.jpg");
            //dock->set_cell_background(KEY_12, "tall.jpg");
            //dock->set_cell_background(KEY_13, "tall.jpg");
            //dock->set_cell_background(KEY_14, "tall.jpg");
            //dock->set_cell_background(KEY_15, "tall.jpg");
            //dock->set_cell_background(KEY_8,  "bad_apple/out113.jpg");
            //dock->refresh();
            bad_apple(dock);
            const auto end = std::chrono::system_clock::now();
            const std::chrono::duration<double> diff = end - start;
            std::cout << "Loading the image took " << diff.count() << " seconds." << std::endl;
            //dock->set_cell_background(KEY_8, "ibuki_rocking.jpg");
            //dock->refresh();
        }
    }
    return 0;
}
