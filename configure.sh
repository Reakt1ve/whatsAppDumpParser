#!/bin/bash

# Сконфигурировать исполняемый файл
result=$(gcc -v 2>&1)
if echo $result | grep ": gcc:" >/dev/null; then
	echo "Install gcc"
else
	gcc arrayList.c bool.h extend.c parser.c utilities.c -o parser
fi

# Записать файл в папку bin
if ls /bin | grep parser > /dev/null; then
	cp -a ./parser /bin
else
	:
fi
