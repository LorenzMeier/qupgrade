#!/bin/sh
cp -r ../../../qupgrade-build-Desktop_Qt_5_0_1_clang_64bit-Release/ .

echo -e '\n\nStarting to create disk image. This may take a while..\n'
macdeployqt qupgrade.app -dmg
rm -rf qupgrade.app
echo -e '\n\n qupgrade .DMG file is now ready for publishing\n'
