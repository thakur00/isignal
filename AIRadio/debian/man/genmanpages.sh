#!/bin/bash

txt2man -d "${CHANGELOG_DATE}" -t ISRENB -s 8 isrenb.txt > isrenb.8
txt2man -d "${CHANGELOG_DATE}" -t ISREPC -s 8 isrepc.txt > isrepc.8
txt2man -d "${CHANGELOG_DATE}" -t ISRUE  -s 8 isrue.txt > isrue.8
