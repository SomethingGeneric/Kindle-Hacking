## Setup (This way is for build.sh)
* `sudo apt-get install libc6-armel-cross libc6-dev-armel-cross binutils-arm-linux-gnueabi libncurses5-dev -y`
* `sudo apt-get install gcc-arm-linux-gnueabi g++-arm-linux-gnueabi -y`

## Caveat
I've only used for C, but those libs should include the g++ version too
(Tested on Ubuntu 20.04 as build machine)

## Use
`./build.sh <source file> <exec fn>`