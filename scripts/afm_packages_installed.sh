#!/bin/bash

function list_afm_packaged() {
    local fields=()
    [[ ! -d /var/local/lib/afm/applications ]] && { error "Unable to find /var/local/lib/afm/applications/"; return 1;}
    for packages in $(ls /var/local/lib/afm/applications/); do
        fields=()
        if [[ "$packages" != "include" ]] && [[  "$packages" != "recovery" ]]; then
            FullNamePackage=$(rpm -qf /var/local/lib/afm/applications/$packages)
            package=""
            for tmp in $(echo $FullNamePackage | awk -F'-' '{for(i=1; i<=NF-2; ++i) print $i}'); do
                if [[ $package == "" ]]
                then
                        package="$tmp"
                else
                        package="$package-$tmp"
                fi
            done
            Version=$(rpm --info -q ${packages} | grep Version | awk -F' ' '{print $3}')
            echo "{\"${package}\": \"${Version}\"}"

            # Check if plugins
            if [[ -d /var/local/lib/afm/applications/$packages/lib/plugins ]]; then
                for plugins in $(ls /var/local/lib/afm/applications/$packages/lib/plugins); do
                    FullNamePlugin=$(rpm -qf /var/local/lib/afm/applications/$packages/lib/plugins/$plugins)
                    plugin=""
                    for tmp in $(echo $FullNamePlugin | awk -F'-' '{for(i=1; i<=NF-2; ++i) print $i}'); do
                            if [[ $plugin == "" ]]; then
                                    plugin="$tmp"
                            else
                                    plugin="$plugin-$tmp"
                            fi
                    done
                    Version=$(rpm --info -q ${FullNamePlugin} | grep Version | awk -F' ' '{print $3}')
                    [[ $plugin != $package ]] && echo "{\"${plugin}\": \"${Version}\"}"
                done
            fi
        fi
    done
}

list_afm_packaged
