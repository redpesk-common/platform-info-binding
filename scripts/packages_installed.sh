#!/bin/bash

function list_packaged() {
    for packages in $(rpm -qa); do
	    package=""
	    for tmp in $(echo $packages | awk -F'-' '{for(i=1; i<=NF-2; ++i) print $i}'); do
	    	if [[ $package == "" ]]
		then
		    package="$tmp"	
	    	else
			package="$package-$tmp"
		fi
	    done
	    version=$(echo $packages | awk -F'-' '{print $(NF-1)}')
	    echo "{\"$package\": \"$version\"}"
    done
}

list_packaged
