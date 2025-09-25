# qt-rgb-led

RUN: sudo build/qt-rgb-led -platform offscreen

sudo systemctl stop qt-rgb-led.service
sudo systemctl disable qt-rgb-led.service


#uruchomienie z coredumpem
sudo bash -c "ulimit -c unlimited && /home/erwinek/qt-rgb-led/build/qt-rgb-led -platform offscreen"

sudo dd if=/dev/mmcblk0 of=/dev/sda bs=4M status=progress
