#! /bin/bash 

gcc -o app.exe src/buffer.c src/consumer.c src/producer.c main.c
./app.exe