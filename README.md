Enabling permissions:
In `/etc/udev/rules.d/10-streamdeck.rules`:
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



THANK YOU TO https://github.com/nothings/stb
