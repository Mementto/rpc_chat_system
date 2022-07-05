#!/bin/bash

set -e

cd `pwd`/client &&
    sh autobuild.sh
cd ..

cd `pwd`/friend_rpc &&
    sh autobuild.sh
cd ..

cd `pwd`/group_rpc &&
    sh autobuild.sh
cd ..

cd `pwd`/user_rpc &&
    sh autobuild.sh
cd ..