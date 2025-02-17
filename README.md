# Open Libtransport
An open source reimplementation of the MiraBox Stream Dock Protocol (Reverse-Engineered from the proprietary libtransport.so)

See `docs/` for information on hardware quirks and an explaination of the protocol.

# Usage

Enabling permissions:

In `/etc/udev/rules.d/10-streamdock.rules`:
```
SUBSYSTEMS=="usb", ATTRS{idVendor}=="VID", GROUP="users", TAG+="uaccess"
```
Where `VID` is replaced with your stream dock's VID, which can be found with lsusb:
```
$ lsusb
...
Bus 001 Device 041: ID 5500:1001 355 35549
...
```
(In the above example, you would replace `VID` with `5500`)

Then run:
```
sudo udevadm control --reload-rules 
```

# Compiling

```
mkdir build && cd build
cmake ..
make -j4
```
Requires [cppzmq](https://github.com/zeromq/cppzmq)


THANK YOU TO https://github.com/nothings/stb FOR THE EXAMPLE
