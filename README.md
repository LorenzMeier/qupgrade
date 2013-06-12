#qupgrade

##Description

Firmware flashing tool

##Building

###Ubuntu 12.04 / 12.10

    sudo add-apt-repository ppa:canonical-qt5-edgers/qt5-proper
    sudo apt-get update
    sudo apt-get dist-upgrade
    sudo apt-get install build-essential git-core
    sudo apt-get install qt5-default qt5-qmake
    sudo apt-get install libudev-dev libqt5webkit5-dev qtlocation5-dev qtsensors5-dev
    
    sudo add-apt-repository ppa:ubuntu-sdk-team/ppa
    sudo apt-get update
    sudo sudo apt-get install qtcreator
   
    sudo apt-get install libxslt1-dev libglib2.0 libgstreamer-plugins-base0.10-dev libsqlite3-dev
    
    cd ~
    mkdir src
    cd src
    git clone https://github.com/LorenzMeier/qupgrade
    cd qupgrade
    qmake
    make

###Ubuntu 13.04

    sudo apt-get install build-essential git-core
    sudo apt-get install qt5-default qt5-qmake
    sudo apt-get install libudev-dev libqt5webkit5-dev qtlocation5-dev qtsensors5-dev qtdeclarative5-dev
   
    sudo apt-get install libxslt1-dev libglib2.0 libgstreamer-plugins-base0.10-dev libsqlite3-dev
    
    cd ~
    mkdir src
    cd src
    git clone https://github.com/LorenzMeier/qupgrade
    cd qupgrade
    qmake
    make
