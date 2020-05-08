# https://stackoverflow.com/questions/59391695/cross-compiling-python-for-arm-from-source

read -p "Press enter to confirm you have installed deps in ../My-Cross/README.md"
read -p "Download Python source, and extract. Press enter when done."
read -p "Directory: " dir

cd $dir
mkdir -p /home/$USER/CROSS/python3_exec
mkdir -p /home/$USER/CROSS/python3_pre

make clean
./configure CC=arm-linux-gnueabi-gcc CXX=arm-linux-gnueabi-g++ AR=arm-linux-gnueabi-ar LD=arm-linux-gnueabi-ld RANLIB=arm-linux-gnueabi-ranlib --host=arm-linux-gnueabihf --target=arm --exec-prefix=/home/$USER/CROSS/python3_exec --prefix=/home/$USER/CROSS/python3_pre --build=x86_64-linux-gnu

make
read -p "Done with make. Press enter"
clear

cd ../
p="/home/$USER/CROSS"
python3 rep.py $p $dir/Makefile
cd $dir

read -p "Tried to reconfigure. Press enter"
clear

make install
echo "Things should be in /home/$USER/CROSS/python3_install"
cd ..