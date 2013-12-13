#!/bin/bash

mkdir -p ~/Desarrollo/Workspace/2012-2c-bashenato/

cp -r ../2012-2c-bashenato/PROCER/  ~/Desarrollo/Workspace/2012-2c-bashenato/
cp -r ../2012-2c-bashenato/SHIELD/ ~/Desarrollo/Workspace/2012-2c-bashenato/
cp tests.zip ~/
cd ~/
unzip tests.zip
cp tests/* ~/
rm -f -r tests
rm tests.zip

cd ~/Desarrollo/Workspace/2012-2c-bashenato/PROCER/commons/Debug/
make all
cp libcommons.so ~/

cd ~/Desarrollo/Workspace/2012-2c-bashenato/PROCER/PI/Debug/
make all
cp PI ~/pi
cp pi.config ~/

cd ~/Desarrollo/Workspace/2012-2c-bashenato/PROCER/PP/Debug/
make all
cp PP ~/pp
cp pp.config ~/

cp -r ~/Desarrollo/Workspace/2012-2c-bashenato/SHIELD/ ~/

rm -r -f ~/Desarrollo

echo '#!/bin/bash' > ~/exportLDlib.sh
chmod +x ~/exportLDlib.sh
echo 'export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:~/' >> ~/exportLDlib.sh

