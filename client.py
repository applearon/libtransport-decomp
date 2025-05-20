#!/usr/bin/env python
import zmq
from evdev import uinput, ecodes as e
from enum import Enum
import io
from PIL import Image

img = Image.open("flower_power.png", mode='r').convert("RGB")


packet = {
    "SEND_GET_SCREEN_ON": b'\x00',
    "SEND_REFRESH": b'\x01',
    "SEND_SET_BRIGHTNESS": b'\x02',
    "SEND_TOGGLE_SCREEN": b'\x03',
    "SEND_SET_FULL_BACKGROUND": b'\x04',
    "SEND_SET_CELL_BACKGROUND": b'\x05',
    "SEND_CLEAR_CELL_BACKGROUND": b'\x06',
    "SEND_WAKEUP": b'\x07',
    "SEND_STATUS": b'\x08'

}

context = zmq.Context()
socket = context.socket(zmq.SUB)
sender = context.socket(zmq.REQ)

socket.connect("tcp://localhost:40289")
socket.setsockopt_string(zmq.SUBSCRIBE, "")
sender.connect("tcp://localhost:40389")

#sender.send(packet["SEND_SET_FULL_BACKGROUND"] + img.tobytes())
#status = "succeeded" if sender.recv()[0] == 1 else "failed"
#sender.send(packet["SEND_REFRESH"])
#print(sender.recv())

with uinput.UInput() as ui:
    while True:
        string = socket.recv()
        down = "down" if string[1] == 1 else "up"
        key = string[0]
        print("Key " + str(key) + " was pressed " + down)
        if down == "up":
            continue
        if key == 5:
            sender.send(packet["SEND_GET_SCREEN_ON"])
            status = "on" if sender.recv()[0] == 1 else "off"
            print("The screen is " + status)
        if key == 6:
            sender.send(packet["SEND_REFRESH"])
            status = "succeeded" if sender.recv()[0] == 1 else "failed"
            print("The screen refresh " + status)
        if key == 7:
            sender.send(packet["SEND_SET_BRIGHTNESS"] + bytes([100]))
            status = "succeeded" if sender.recv()[0] == 1 else "failed"
            print("set brightness " + status)
        if key == 15:
            ui.write(e.EV_KEY, e.KEY_M, 1)
            ui.write(e.EV_KEY, e.KEY_M, 0)
            ui.write(e.EV_KEY, e.KEY_E, 1)
            ui.write(e.EV_KEY, e.KEY_E, 0)
            ui.write(e.EV_KEY, e.KEY_A, 1)
            ui.write(e.EV_KEY, e.KEY_A, 0)
            ui.write(e.EV_KEY, e.KEY_T, 1)
            ui.write(e.EV_KEY, e.KEY_T, 0)
            ui.write(e.EV_KEY, e.KEY_CAPSLOCK, 1)
            ui.write(e.EV_KEY, e.KEY_CAPSLOCK, 0)
            ui.write(e.EV_KEY, e.KEY_LEFTALT, 1)
            ui.write(e.EV_KEY, e.KEY_TAB, 1)
            ui.write(e.EV_KEY, e.KEY_LEFTALT, 0)
            ui.write(e.EV_KEY, e.KEY_TAB, 0) 
            ui.write(e.EV_KEY, e.KEY_SYSRQ, 1)
            ui.write(e.EV_KEY, e.KEY_SYSRQ, 0)
            ui.syn()

