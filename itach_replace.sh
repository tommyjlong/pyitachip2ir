#!/bin/bash
# Shell Script for HomeAssistant that will replace
# the native itachip2ir dynamic linked library with a custom one.
# When pyitach runs, it looks for a file with a name containing
#   the string *itachip2ir* + ext, where ext is 'so','dylib','dll','pyd'.
#   This script assumes its it ".so"
# It will look for the native version based on a key search word and if found will
# rename its filename extension from ".so" to ".sow"
# and then will copy the custom version (named itachip2ir.so) in its place.
# For HA Docker on HAOS, it should be located 
#  at /usr/local/lib/python3.XX/site-packages.

SourceFile=/config/shell_commands/itachip2ir.so
SitePath=/usr/local/lib/python3.10/site-packages
SearchString="cpython"
echo "Looking for original itach binary file"
FullFileName=$(ls $SitePath/itachip2ir.* | grep -E $SearchString )
#echo FullFileName $FullFileName

if [ -z $FullFileName ]; then
 #echo "FullFileName is null"
  echo "Could not find native itach binary file"
else
    if [ -e $FullFileName ]; then
      echo "Found file" $FullFileName
      if [ "${FullFileName: -4}" == ".sow" ]; then
          echo "File ending in .sow already exists, so nothing to do"
      else
          echo "File does not end in .sow, so rename it and copy over custom version"
          mv $FullFileName "${FullFileName}w"
          cp $SourceFile "${SitePath}/itachip2ir.so"
      fi
    else
      echo "File Not Found"
    fi
fi
