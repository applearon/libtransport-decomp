#include <hidapi/hidapi.h>
#include <hidapi/hidapi_libusb.h>
#include<iostream>
#include "stream_dock.hpp"
#include"packets.hpp"
#include<chrono>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"
#include<thread>

#include <zmq.hpp>


#define MAX_STR 255

void bad_apple(StreamDock *dock) {
    std::cout << "Starting bad apple..." << std::endl;
    const size_t total_frames = 6572;
    constexpr size_t fps = 30;
    size_t time_per_frame = 1000000000 / 30;
    const auto ms_per_frame = std::chrono::nanoseconds(time_per_frame); // 30fps
    const auto begin = std::chrono::system_clock::now();
    std::chrono::duration<double> diff;
    size_t next_frame = 1;
    size_t last_frame = next_frame;
    while (true) {
            const auto start = std::chrono::system_clock::now();
            diff = start - begin;
            last_frame = next_frame;
            next_frame = diff / ms_per_frame;
            if (next_frame == 0) {
                next_frame = 1;
            }
            if (last_frame == next_frame) {
                continue;
            }
            if (next_frame - last_frame == 2) {
                //std::cout << "Skipping frame " << last_frame + 1 << std::endl;
            } else if (next_frame - last_frame != 1) {
                std::cout << "nf: " << next_frame << ", diff: " << diff.count() << std::endl;
                std::cout << "Skipping frame(s) " << last_frame + 1 << " to " << next_frame - 1 << std::endl;
            }
            if (next_frame > total_frames) {
                break;
            }
            std::string path = "bad_apple/out" + std::to_string(next_frame) + ".jpg";
            dock->set_cell_background(KEY_8, path);
            dock->refresh();
    }
}

int main(void) {
    std::thread apple;
    zmq::context_t context (1);
    zmq::socket_t publisher (context, zmq::socket_type::pub);
    publisher.bind("tcp://*:5556");
    zmq::message_t message(2);

    StreamDock *dock = new StreamDock();
    if (!dock->is_good()) {
        std::cout << "Failed to open device" << std::endl;
        return 1;
    }
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
    bool running = true;
    while (true) {
        struct key_input key = dock->read();
        if (!dock->is_good()) {
            std::cout << "Device Disconnected" << std::endl;
            return 1;
        }
        if (key.key == ALL_KEYS) continue; // no input detected
        message.rebuild(2);
        message.data<unsigned char>()[0] = key.key;
        message.data<unsigned char>()[1] = key.down;
        //snprintf((char *) message.data(), 4, "%02d%d", key.key, key.down);
        publisher.send(message, zmq::send_flags::none);
        //strcpy(message.data<char>(), out.data());
        std::cout << "Key " << key.key << " pressed";
        if (key.down) {
            std::cout << " down." << std::endl;
        } else {
            std::cout << " up." << std::endl;
        }
        //if (key.key == 15) {
        //    dock->toggle_screen();
        // } else if (key.key == 14 && !key.down) {
        //    dock->clear_cell_background(key.key);
        //} else if (key.key == 13 && !key.down) {
        //    apple = std::thread(bad_apple, dock);
        //    //running = false;
        //    //apple.join();
        //    //bad_apple(dock);
        //}
    }
    return 0;
}
