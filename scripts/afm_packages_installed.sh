#!/bin/bash

function list_afm_packaged() {
    local fields=()
    [[ ! -d /var/local/lib/afm/applications ]] && { error "Unable to find /var/local/lib/afm/applications/"; return 1;}
    for package in $(ls /var/local/lib/afm/applications/); do
        fields=()
        for field in $(sudo yum list installed | grep redis-tsdb-binding); do
            fields+=($field)
        done
        echo "{\"${package}\": \"${fields[1]}\"}"
    done

}

list_afm_packaged