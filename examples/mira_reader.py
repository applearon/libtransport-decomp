#!/usr/bin/env python
from pydbus import SessionBus
from gi.repository import GLib

# Connect to the session bus
bus = SessionBus()

# Define the interface, object path, and signal
interface_name = "ca.applism.miradock"
object_path = "/ca/applism/MiraDock"  # Guessing typical object path naming
signal_name = "KeyPress"

# Signal handler
def on_key_press(*args, **kwargs):
    key_arr = args[4][0].split(':')
    key = int(key_arr[0])
    press_down = True if key_arr[1] == "1" else False
    print("Key", key, "pressed", "down" if press_down else "up")
    #print("KeyPress signal received!")
    #print("Arguments:", args)
    #print("Kwargs:", kwargs)

# Subscribe to the signal
bus.subscribe(
    sender=interface_name,
    iface=interface_name,
    signal=signal_name,
    object=object_path,
    signal_fired=on_key_press
)

print(f"Listening for '{signal_name}' on '{interface_name}'...")
GLib.MainLoop().run()
