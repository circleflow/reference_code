# script used to workaround with opmock's bug: function prototype can not return a reference
# this script take 2 arguments:
# $1: reference type to be fixed(used as function return type)
# $2: file 

echo $1 $2

sed -i "s/($1 \&)\&/\&($1 \&)/g" $2

