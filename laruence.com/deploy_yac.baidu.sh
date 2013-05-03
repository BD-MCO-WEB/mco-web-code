#!/bin/sh
# modify yac source code , and deploy it !
# pytonic@foxmail.com

cd $(pwd)/yac/

make clean ;
./configure --enable-yac --with-php-config=/home/work/php/bin/php-config 
make 
make install;

mv /home/work/php/lib/php/extensions/no-debug-non-zts-20090626/yac.so /home/work/php/lib;

re='^[^;].*yac\.so';
yac_config=`grep $re /home/work/php/lib/php.ini`
echo $yac_config;

if [ -z "$yac_config" ]; then
    echo "need to enable yac.so in php.ini...\n";
    echo -e "[yac] \nextension = yac.so" >> /home/work/php/lib/php.ini
else
    echo -e "already enabled yac.so in php.ini...\r\nDone";
fi

