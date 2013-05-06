#!/bin/sh
# modify yac source code , and deploy it !
# pytonic@foxmail.com

cd $(pwd)/yac/

make clean ;
/usr/local/Cellar/php5.4/bin/phpize
./configure --enable-yac --with-php-config=/usr/local/Cellar/php5.4/bin/php-config
make 
make install;

re='^[^;].*yac\.so';
yac_config=`grep $re /usr/local/Cellar/php5.4/lib/php.ini`
echo $yac_config;

if [ -z "$yac_config" ]; then
    echo "need to enable yac.so in php.ini...\n";
    echo "[yac] \nextension = yac.so" >> /usr/local/Cellar/php5.4/lib/php.ini
else
    echo "already enabled yac.so in php.ini...\r\nDone";
fi

