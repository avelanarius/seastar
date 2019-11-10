#!/bin/bash

usage() {
    echo "Usage: $0 [-i INPUT_PATH] [-c CLIENT_IPS]" 1>&2; exit 1;
}

while getopts ":i:c:" o; do
    case "${o}" in
        i)
            INPUT_PATH=${OPTARG}
            ;;
        c)
            CLIENT_IPS=${OPTARG}
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

if [ -z "${INPUT_PATH}" ] || [ -z "${CLIENT_IPS}" ]; then
    usage
fi

INPUT_PATH=$(readlink -f "${INPUT_PATH}")
cd "${0%/*}" || exit 1

./cmake-build-debug/kafkasnoop "${INPUT_PATH}" "${CLIENT_IPS}"
xdg-open "${INPUT_PATH}/two_column.html"