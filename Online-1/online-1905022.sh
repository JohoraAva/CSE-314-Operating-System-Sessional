#!/bin/bash


input=$1
output=$2


mkdir -p "$2"

lineCount()
{
    c=0
    while read -d line
    do
    c=$(`expr c + 1 `)
    echo $c 
    done
}

FindTheFile()
{
    if [ -d $1 ]
    then
        if [ -f $1/*.txt ] ; then
        # echo name
        # echo $1/*
        mv $1/*.txt "$output/$1.txt"
        # lineCount $1
       else
            for i in $1/* ; do
                FindTheFile $i
            done
        fi

    fi


}

echo c
# cd $1
FindTheFile  $1



