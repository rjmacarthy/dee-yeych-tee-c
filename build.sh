#!/bin/bash

gcc -c dht.c && gcc -c dht_test.c && gcc -o dht_test dht.o dht_test.o -lcunit -lcrypto -lpthread -lm