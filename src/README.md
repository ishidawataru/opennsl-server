# opennsl-man
Opennsl-man is a simple management interface for OpenNSL compatible switches.
The goal is that it should implement most of the features that you'd expect in a regular L2 switch.

## Motivation
Inspired by opennsl-server by Ishida Wataru, I wanted to build a tools that exposes the essential feature set of the opennsl library. Further more as an avid fan of quagga and vtysh I wanted to borrow some of the look and feel from that interface so that users would feel more at home.
Eventually I would like to convert this experiment into a quagga plugin so that it's all just inside one interface for convenience; I would aim to make it similar to how previous versions of openswitch worked, but without breaking fundamental BGP features (next-hop-self anyone?).

## Build
    make

## Use
    1. Activate the lasers
    i2cset -y 1 0x61 0x0c 0x00 b
    i2cset -y 1 0x61 0x0d 0x00 b
    i2cset -y 1 0x61 0x0e 0x00 b

    i2cset -y 1 0x62 0x0c 0x00 b
    i2cset -y 1 0x62 0x0d 0x00 b
    i2cset -y 1 0x62 0x0e 0x00 b
    
    #initialise the kernel modules
    rmmod linux-user-bde
    rmmod linux-bcm-knet
    rmmod linux-kernel-bde
    rm -f /dev/linux-kernel-bde
    rm -f /dev/linux-user-bde
    rm -f /dev/linux-bcm-knet

    mknod /dev/linux-kernel-bde c 127 0
    mknod /dev/linux-user-bde c 126 0
    mknod /dev/linux-bcm-knet c 122 0

    insmod linux-kernel-bde.ko
    insmod linux-user-bde.ko
    insmod linux-bcm-knet.ko
    
    3. Run OSH
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<OPENNSL_DIR>
    export OPENNSL_CONFIG_FILE=config.as5712
    ./osh
    osh> conf t
    
## Contributions
Contributions to opennsl-man are definitely welcome, if you'd like to get involved please get intouch through github.

## Feature requests
If you want me to add support for a specific OpenNSL compatible switch send me one, If I already have that model and you want me to spend more time on it feel free to contact me anyway.
Current switches I have access to:

 - Accton / EdgeCore AS5712-54X

### DISCLAIMER
I write this software because I need it, I use EdgeCore switches at work, if you want me to support something else I need a reason to do so. If you send me a switch, you wont get it back, it will be mine to do with as I wish and it does not constitute my acceptance of any form of contract.
### END OF DISCLAIMER

Having said all that, if I have the hardware I will most likely use it, and if I use it, I will need it to work.
    
## License
This software is licensed under the MIT License. You can find a copy in the source tree.
