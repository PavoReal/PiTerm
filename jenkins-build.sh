#! /bin/bash 

time ./build.sh -r
if [ ! $? -eq 0 ]
then
	echo -e "\e[7mRelease build failed...\e[0m"
	exit 1
else
	echo -e "\e[7mGood release build\e[0m"
fi

cp build/PiTerm build/PiTerm-Release
objdump -S --disassemble build/PiTerm > build/PiTerm-Release.list

time ./build.sh -d
if [ ! $? -eq 0 ]
then
	echo -e "\e[7mDebug build failed...\e[0m"
	exit 1
else
	echo -e "\e[7mGood debug build\e[0m"
fi
objdump -S --disassemble build/PiTerm > build/PiTerm-Debug.list
