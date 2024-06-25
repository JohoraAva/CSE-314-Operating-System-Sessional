#!/bin/bash


mkdir -p $2
target_dir=$2
echo "RollNo,Language,Matched,Not_matched"> $target_dir/result.csv

tests=$3
answers=$4
location=""
type=""


#function
FindTheFile()
{
    if [ -d $1 ]
    then
        if [ -f $1/*.c ] ; then
            mkdir -p "$target_dir/C/$2"
            mv $1/*.c "$target_dir/C/$2/main.c"
            location="$target_dir/C/$2"
            type="C"
#             echo $location

        elif [ -f $1/*.java ] ; then
            mkdir -p "$target_dir/Java/$2"
            mv $1/*.java "$target_dir/Java/$2/Main.java"
            location="$target_dir/Java/$2"
            type="Java"

        elif [ -f $1/*.py ] ; then
            mkdir -p "$target_dir/Python/$2"
            mv $1/*.py "$target_dir/Python/$2/main.py"
            location="$target_dir/Python/$2"
            type="Python"

        else
            for i in $1/* ; do
                FindTheFile $i $2
            done
        fi

    fi


}

#check argument no.
if [[ $4 == "" ]];then
    echo Usage
    echo "./1905022.sh <submission folder> <target folder> <test folder> <answer folder> [-v] [-noexecute]"
    echo -v: verbose
    echo -noexecute: do not execute code files
    exit
fi


#find -v -noexecute
isNoExecute=0
isV=0

for commands in $*
do
    if [ $commands = "-v" ]
    then
        isV=1
    fi

    if [ $commands = "-noexecute" ]
    then
        isNoExecute=1
    fi
done

#difference
FindDiff()
{
    isDiff=$(diff $1 $2)
    if [[ $isDiff == "" ]]
    then
        Mcount=`expr $3 + 1`
    else
        NMcount=`expr $4 + 1`
    fi

}



writeToCSV()
{
    if [ $isV = 1 ]
    then
        echo executing files of $1
    fi
    echo "$1,$2,$3,$4">> ./../../result.csv

    cd "./../../../"
}



executeC()
{
     gcc "main.c"
    for testNo in {1..3}
    do
     "./a.out" < ./../../../$tests/test$testNo.txt > "out$testNo.txt"
     FindDiff "out$testNo.txt"  ./../../../$answers/ans$testNo.txt $Mcount $NMcount
    done
}

executeJava()
{
    javac Main.java
    for testNo in {1..3}
    do
     java Main < ./../../../$tests/test$testNo.txt > "out$testNo.txt"
     FindDiff "out$testNo.txt"  ./../../../$answers/ans$testNo.txt $Mcount $NMcount $files "Java"
    done

}

executePython()
{

    for testNo in {1..3}
    do
     python3 main.py < ./../../../$tests/test$testNo.txt > "out$testNo.txt"
     FindDiff "out$testNo.txt"  ./../../../$answers/ans$testNo.txt $Mcount $NMcount $files "Python"
    done
}

#compiling and executing
executeCode()
{
if [ $1 == "C" ];then
    Mcount=0
    NMcount=0
    executeC
elif [ $1 == "Java" ];then
    Mcount=0
    NMcount=0
    executeJava
elif [ $1 == "Python" ];then
    Mcount=0
    NMcount=0
    executePython

fi

 writeToCSV $2 $1 $Mcount $NMcount


}

# Loop through all the zipped folders
for zipped_folder in "$1"/*".zip"
do

    fileName=${zipped_folder:(-11):-4} #roll
    mkdir -p "$target_dir/tmp/$fileName"
    unzip -qq -d "$target_dir/tmp/$fileName" "$zipped_folder"

    if [ $isV = 1 ]
    then
        echo organizing files of $fileName
    fi

    FindTheFile "$target_dir/tmp/$fileName" $fileName


    if [ $isNoExecute = 0 ]
    then
        cd $location
        executeCode $type $fileName
    fi

done
rm -r $target_dir/tmp

