#!/usr/bin/env python
import zmq
from evdev import uinput, ecodes as e
from enum import Enum

class packet_names(Enum):
    SEND_GET_SCREEN_ON  = 0
    SEND_REFRESH = 1
    SEND_SET_BRIGHTNESS = 2
    SEND_TOGGLE_SCREEN = 3
    SEND_SET_FULL_BACKGROUND = 4
    SEND_SET_CELL_BACKGROUND = 5
    SEND_CLEAR_CELL_BACKGROUND = 6
    SEND_WAKEUP = 7
    SEND_STATUS = 8

context = zmq.Context()
socket = context.socket(zmq.SUB)
sender = context.socket(zmq.REQ)

socket.connect("tcp://localhost:40289")
socket.setsockopt_string(zmq.SUBSCRIBE, "")
sender.connect("tcp://localhost:40389")

with uinput.UInput() as ui:
    while True:
        string = socket.recv()
        down = "down" if string[1] == 1 else "up"
        key = string[0]
        print("Key " + str(key) + " was pressed " + down)
        if down == "up":
            continue
        if key == 1:
            sender.send(bytes([packet_names.SEND_WAKEUP.value]))
            status = "succeeded" if sender.recv()[0] == 1 else "failed"
            print("wake up screen " + status)
        if key == 5:
            sender.send(bytes([packet_names.SEND_GET_SCREEN_ON.value]))
            status = "on" if sender.recv()[0] == 1 else "off"
            print("The screen is " + status)
        if key == 6:
            sender.send(bytes([packet_names.SEND_REFRESH.value]))
            status = "succeeded" if sender.recv()[0] == 1 else "failed"
            print("The screen refresh " + status)
        if key == 7:
            sender.send(bytes([packet_names.SEND_SET_BRIGHTNESS.value, 100]))
            status = "succeeded" if sender.recv()[0] == 1 else "failed"
            print("set brightness " + status)
