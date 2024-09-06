#!/bin/sh

for dir in libnslog libwapcaplet libparserutils libcss libhubbub libdom libnsbmp libnsgif librosprite libnsutils libutf8proc libnspsl libsvgtiny libnsfb nsgenbind netsurf
do
  if [ -d $dir ]; then
    echo "--------------------------- BEGIN MAKE $dir"
    cd $dir
    make -f Makefile.$dir $1
    cd ..
    echo "--------------------------- END MAKE $dir"
  fi
done

cd pumpkin
make $1
cd ..

exit 0
