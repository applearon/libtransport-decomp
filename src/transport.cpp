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
        "/ca/applism/MiraDock", // object path
        "ca.applism.miradock",  // interface
        "KeyPress"              // signal name
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

void handle_dbus_events(DBusConnection *dconn, DBusMessage *msg, StreamDock *dock) {
    if (msg == nullptr) return;
    std::string interface = dbus_message_get_interface(msg);
    DBusMessage *reply = dbus_message_new_method_return(msg);
    DBusError error;
    dbus_error_init(&error);
    if (interface == "org.freedesktop.DBus.Introspectable") {
        const char* introspection_xml =
            R"(<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-Bus Object Introspection 1.0//EN"
             "http://www.freedesktop.org/standards/dbus/2.0/introspect.dtd">
            <node>
              <interface name="ca.applism.miradock">
                <method name="Refresh">
                    <arg direction="out" type="b" name="success" />
                </method>
                <method name="SetCellBackgroundPath">
                    <arg direction="in" type="q" name="cell" />
                    <arg direction="in" type="s" name="path" />
                    <arg direction="out" type="b" name="success" />
                </method>
                <method name="SetCellBackground">
                    <arg direction="in" type="q" name="cell" />
                    <arg direction="in" type="ay" name="jpgdata" />
                    <arg direction="out" type="b" name="success" />
                </method>
                <method name="ToggleScreen">
                    <arg direction="in" type="b" name="on" />
                    <arg direction="out" type="b" name="success" />
                </method>
                <method name="SetFullBackground">
                    <arg direction="in" type="ay" name="photobuffer" />
                    <arg direction="out" type="b" name="success" />
                </method>
                <method name="SetBrightness">
                    <arg direction="in" type="q" name="brightness" />
                    <arg direction="out" type="b" name="success" />
                </method>
                <method name="ClearFullBackground">
                    <arg direction="out" type="b" name="success" />
                </method>
                <method name="ClearCellBackground">
                    <arg direction="in" type="i" name="cell" />
                    <arg direction="out" type="b" name="success" />
                </method>
                <method name="SendWakeup">
                    <arg direction="out" type="b" name="success" />
                </method>
                <method name="Status">
                    <arg direction="out" type="b" name="device_on" />
                    <arg direction="out" type="b" name="screen_on" />
                    <arg direction="out" type="i" name="brightness" />
                </method>
              </interface>
              <interface name="org.freedesktop.DBus.Introspectable">
                <method name="Introspect">
                  <arg direction="out" type="s" name="data"/>
                </method>
              </interface>
            </node>
            )";
        dbus_message_append_args(reply, DBUS_TYPE_STRING, &introspection_xml, DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(msg, "ca.applism.miradock", "Refresh")) {
        int response = dock->refresh();
        dbus_message_append_args(reply, DBUS_TYPE_BOOLEAN, &response, DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(msg, "ca.applism.miradock", "SetCellBackgroundPath")) {
        const char* path;
        uint16_t key;
        int response = false; // if invalid arguments
        if (dbus_message_get_args(msg, &error, DBUS_TYPE_UINT16, &key, DBUS_TYPE_STRING, &path, DBUS_TYPE_INVALID)) {
            // Send reply
            response = dock->set_cell_background(static_cast<enum key>(key), path);
        }
        dbus_message_append_args(reply, DBUS_TYPE_BOOLEAN, &response, DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(msg, "ca.applism.miradock", "SetCellBackground")) {
        int response = false;
        unsigned char *bytes = nullptr;
        int length = 0;
        uint16_t key = ALL_KEYS;
        if (dbus_message_get_args(msg, &error, DBUS_TYPE_UINT16, &key, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &bytes, &length, DBUS_TYPE_INVALID)) {
            response = dock->set_cell_background(static_cast<enum key>(key), bytes, length);
        }
        dbus_message_append_args(reply, DBUS_TYPE_BOOLEAN, &response, DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(msg, "ca.applism.miradock", "ToggleScreen")) {
        int on;
        int response = false;
        if (dbus_message_get_args(msg, &error, DBUS_TYPE_BOOLEAN, &on, DBUS_TYPE_INVALID)) {
            // Send reply
            response = dock->toggle_screen(on);
        }
        dbus_message_append_args(reply, DBUS_TYPE_BOOLEAN, &response, DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(msg, "ca.applism.miradock", "SetFullBackground")) { // uh uhm ill do this later
        unsigned char *bytes = nullptr;
        int length = 0;
        int response = false;
        if (dbus_message_get_args(msg, &error, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &bytes, &length, DBUS_TYPE_INVALID)){
            std::cout << length << std::endl;
            response = dock->set_full_background(bytes);
            std::cout << "owo" << std::endl;
            dock->refresh();
        }
        dbus_message_append_args(reply, DBUS_TYPE_BOOLEAN, &response, DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(msg, "ca.applism.miradock", "SetBrightness")) {
        uint16_t brightness;
        int response = false;
        if (dbus_message_get_args(msg, &error, DBUS_TYPE_UINT16, &brightness, DBUS_TYPE_INVALID)) {
            response = true && dock->set_brightness(brightness);
        }
        dbus_message_append_args(reply, DBUS_TYPE_BOOLEAN, &response, DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(msg, "ca.applism.miradock", "ClearFullBackground")) {
        int response = dock->clear_full_background();
        dbus_message_append_args(reply, DBUS_TYPE_BOOLEAN, &response, DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(msg, "ca.applism.miradock", "ClearCellBackground")){
        int key;
        int response = false;
        if (dbus_message_get_args(msg, &error, DBUS_TYPE_INT64, &key, DBUS_TYPE_INVALID)) {
            response = true && dock->clear_cell_background(static_cast<enum key>(key));
        }
        dbus_message_append_args(reply, DBUS_TYPE_BOOLEAN, &response, DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(msg, "ca.applism.miradock", "SendWakeup")) {
        int response = dock->send_wakeup();
        dbus_message_append_args(reply, DBUS_TYPE_BOOLEAN, &response, DBUS_TYPE_INVALID);
    } else if (dbus_message_is_method_call(msg, "ca.applism.miradock", "Status")) { // send brightness and if screen is on and if the device is on ig
        int status = dock->is_good();
        int screen = dock->is_screen_on();
        int64_t brightness = dock->get_brightness();
        dbus_message_append_args(reply,DBUS_TYPE_BOOLEAN, &status, DBUS_TYPE_BOOLEAN, &screen, DBUS_TYPE_INT64, &brightness, DBUS_TYPE_INVALID);
    }
    dbus_connection_send(dconn, reply, nullptr);
    dbus_connection_flush(dconn);
    dbus_message_unref(reply);
    dbus_message_unref(msg);
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
    //std::thread apple;
    StreamDock *dock = new StreamDock();
    DBusError derr;
    DBusConnection *dconn;
    
    dbus_error_init(&derr);

    if (!dock->is_good()) {
        std::cout << "Failed to open device" << std::endl;
        return 1;
    }

    dock->refresh(); // Im like pretty sure we need this to start it up
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

    //int x,y,n;
    //unsigned char *data = stbi_load("flower_power.png", &x, &y, &n, 3);
    //if (data == NULL) {
    //    std::cout << "Failed to load image." <<std::endl;
    //    return 1;
    //}
    //std::cout << "Setting the background image..." << std::endl;
    //const auto start = std::chrono::system_clock::now();
    std::cout << "Initializing..." << std::endl;
    //dock->refresh();
    dock->send_wakeup();
    //dock->set_full_background(data);
    //const auto end = std::chrono::system_clock::now();
    //stbi_image_free(data);
    //const std::chrono::duration<double> diff = end - start;
    //std::cout << "Loading the image took " << diff.count() << " seconds." << std::endl;
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
        dbus_connection_read_write(dconn, 0);
        DBusMessage* msg = dbus_connection_pop_message(dconn);
        //if (key.key == ALL_KEYS) continue; // no input detected
        if (key.key != ALL_KEYS) {
            std::stringstream dbus_out;
            dbus_out << std::setw(2) << std::setfill('0') << key.key << ":" << key.down;
            emit_ping_signal(dconn, dbus_out.str());
            std::cout << "Key " << key.key << " pressed";
            if (key.down) {
                std::cout << " down." << std::endl;
            } else {
                std::cout << " up." << std::endl;
            }
        }
        handle_dbus_events(dconn, msg, dock);
        //if (key.key == 15) {
        //    dock->toggle_screen();
        // } else if (key.key == 14 && !key.down) {
        //    dock->clear_cell_background(key.key);
        //} else if (key.key == 13 && !key.down) {
        //    //apple = std::thread(bad_apple, dock);
        //    //std::cout << "uuw" << std::endl;
        //    //running = false;
        //    //apple.join();
        //    //bad_apple(dock);
        //}
    }
    return 0;
}
