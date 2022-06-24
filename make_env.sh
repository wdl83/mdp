#! /bin/bash

echo "USR="$USER  > .env
echo "UID="`id -u` >> .env
echo "GID="`id -g` >> .env
echo "DST="`mktemp -d` >> .env
