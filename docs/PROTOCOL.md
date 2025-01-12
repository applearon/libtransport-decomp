# NOTICE: I only have experience with vid 5500 (Streamdock 293), your milage may vary

The StreamDock operates through apparent "reverse C-Strings", as all packets sent to the device start with the string terminator `\0`. For the sake of consistency, this fact will be ignored, and packets will be defined in the order that is sent to the device.
The Protocol of the Streamdock (generally) consists of packets of size 3, with spacers made of 2 string terminators.
As an example, the "BEGIN" packet, sent before every command, are the 3 unsigned characters `CRT`.
For the definitions of each packet, see packets.hpp.

Large binary data is sent in chunks of 513 (`\0` followed by 512 bytes of data). This appears to be an arbitrary choice, as other packet chunk sizes are also effective, however I noticed that the data transfer speed is slower at other sizes.


## Images
There are 2 ways that image data is sent to the device. You can send the entire background as raw image data, or you can send induvidual (100x100) jpgs for each button (does not fill the entire cell, usually centered).
Naturally, sending the jpg is significantly faster than the raw image data(<1sec vs ~5sec), but cannot fill the entire screen.

