#!/bin/bash
make

sudo insmod stego_module.ko 
sudo ./chat_bi server
