#qupgrade

##Description

Firmware flashing tool

##Building

###Ubuntu
sudo add-apt-repository ppa:canonical-qt5-edgers/qt5-proper
sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get install build-essential git-core
sudo apt-get install qt5-default qt5-qmake
sudo apt-get install libudev-dev libqt5webkit5-dev

cd ~
mkdir src
cd src
git clone https://github.com/LorenzMeier/qupgrade
cd qupgrade
qmake
make
