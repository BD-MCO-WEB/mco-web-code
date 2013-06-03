#!/bin/sh

# 安装软件: git svn
# 下载代码库: 几个git托管的代码库
# 配置环境：bashrc vim screen

home_dir="/home/map/pytonic/"
home_layout="pkg tool project repertory test"

pkg_url_git="http://download.chinaunix.net/down.php?id=44365&ResourceID=3744&site=1"
pkg_url_svn=""
pkg_url_vim=""
pkg_url_ack=""

git_url_phpsrc= 'https://github.com/pytonic/php-src.git'
git_url_apue2e= 'https://github.com/pytonic/apue2e.git'
git_url_redis= 'https://github.com/pytonic/redis.git'
git_url_yac= 'https://github.com/pytonic/yac.git'
git_url_yaf= 'https://github.com/laruence/php-yaf.git'
git_url_yar= 'https://github.com/laruence/php-yar.git'

if [ -z  ]

#使Git忽略ssl验证
git_config_cmd="
pgit config --global user.name 'pytonic'
pgit config --global user.email 'pytonic@foxmail.com'
pgit config --global credential.helper cache
pgit config --global credential.helper 'cache --timeout=3600'
pgit config --global http.sslVerify false
"


