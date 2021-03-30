#!/bin/bash

numbers=$(rpm -qa | wc --lines)

echo "{\"numbers\" : $numbers}"