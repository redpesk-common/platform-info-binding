#!/bin/bash

numbers=$(yum list installed | wc --lines)

echo "{\"numbers\" : $numbers}"