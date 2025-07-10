#include <hidapi/hidapi.h>
#include <hidapi/hidapi_libusb.h>
#include<iostream>
#include <iomanip>
#include <sstream>
#include "stream_dock.hpp"
#include"packets.hpp"
#include<chrono>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"
#include<thread>

#include<dbus/dbus.h>


#define MAX_STR 255

void emit_ping_signal(DBusConnection* conn, std::string msg) {
    DBusMessage* signal = dbus_message_new_signal(
        "/ca/applism/KeyObj",          // object path
        "ca.applism.miradock",        // interface
        "KeyPress"                        // signal name
    );

    if (!signal) {
        std::cerr << "Failed to create signal\n";
        return;
    }

    DBusMessageIter args;
    dbus_message_iter_init_append(signal, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &msg)) {
        std::cerr << "Out of memory when appending\n";
        return;
    }

    if (!dbus_connection_send(conn, signal, nullptr)) {
        std::cerr << "Out of memory when sending\n";
    }
    dbus_connection_flush(conn);
    dbus_message_unref(signal);
}

void bad_apple(StreamDock *dock) {
    std::cout << "Starting bad apple..." << std::endl;
    const size_t total_frames = 6572;
    constexpr size_t fps = 30;
    size_t time_per_frame = 1000000000 / fps;
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
    StreamDock *dock = new StreamDock();
    DBusError derr;
    DBusConnection *dconn;
    
    dbus_error_init(&derr);

    if (!dock->is_good()) {
        std::cout << "Failed to open device" << std::endl;
        return 1;
    }

    dock->refresh();
    dock->set_brightness(100);

    dconn = dbus_bus_get(DBUS_BUS_SESSION, &derr);
    if (dbus_error_is_set(&derr)) {
        std::cerr << "Connection Error: " << derr.message << std::endl;
        return 1;
    }
    int dbus_req_res = dbus_bus_request_name(dconn, "ca.applism.miradock", DBUS_NAME_FLAG_REPLACE_EXISTING, &derr);
    if (dbus_error_is_set(&derr) || dbus_req_res != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        std::cerr << "Failed to claim dbus name: " << derr.message << std::endl;
        return 1;
    }

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
        if (!dock->is_good()) {
            std::cout << "Device Disconnected. Reconnecting..." << std::endl;
            while (!dock->is_good()) {
                hid_exit();
                delete dock;
                dock = new StreamDock();
            }
            std::cout << "Reconnected!" << std::endl;
        }
        if (key.key == ALL_KEYS) continue; // no input detected
        std::stringstream dbus_out;
        dbus_out << std::setw(2) << std::setfill('0') << key.key << ":" << key.down;
        emit_ping_signal(dconn, dbus_out.str());
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
