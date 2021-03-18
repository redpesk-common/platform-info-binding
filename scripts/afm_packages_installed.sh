#!/bin/bash

function list_afm_packaged() {
    local fields=()
    [[ ! -d /var/local/lib/afm/applications ]] && { error "Unable to find /var/local/lib/afm/applications/"; return 1;}
    for package in $(ls /var/local/lib/afm/applications/); do
        fields=()
	if [[ "$package" != "include" ]]; then
            for field in $(sudo yum list installed | grep ${package}); do
                fields+=($field)
            done
	    echo "{\"${package}\": \"${fields[1]}\"}"
        fi
    done
}

list_afm_packaged
