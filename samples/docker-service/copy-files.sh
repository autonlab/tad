#!/bin/bash

if [[ ! -d service ]]; then
    mkdir service
fi

cp -v ../../build/samples/cpp-server/cpp-server service/.

rm -rf service/SRL
cp -vR ../../python/SRL service/.

if [[ ! -d service/eventdetector ]]; then
    mkdir -p service/eventdetector
fi
rm -rf service/eventdetector/*
cp -vR ../python-eventdetector-service/* service/eventdetector/.
