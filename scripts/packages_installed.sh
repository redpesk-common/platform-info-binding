#!/bin/bash

function list_packaged() {
    COUNT=0
    VERSION=""
    PACKAGE=""
    local JO_ALL=()
    for package in $(yum list installed); do
        let COUNT=${COUNT}+1
        if [[ ${COUNT} == 1 ]] || [[ ${COUNT} == 2 ]]; then 
            continue 
        fi
        case $(expr ${COUNT} % 3) in
            0)
                # name
                PACKAGE="${package}"
                ;;

            1)
                # version
                VERSION="${package}"
                ;;

            2)
                # from - no need
                # Create the json
                # jq  -n \
                #     --arg pn "$PACKAGE" \
                #     --arg pv "$VERSION" \
                #     '{package: $pn, version: $pv}';
                echo "${PACKAGE}/${VERSION}";
                ;;
            *)
                ;;
        esac
    done
}

list_packaged