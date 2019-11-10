#!/bin/bash

DIR="$(cd "$(dirname "$0")" && pwd)"

usage() {
    echo "Usage: $0 [-o OUTPUT_PATH] [-c CLIENT_IPS]" 1>&2; exit 1;
}

while getopts ":o:c:" o; do
    case "${o}" in
        o)
            OUTPUT_PATH=${OPTARG}
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

if [ -z "${OUTPUT_PATH}" ] || [ -z "${CLIENT_IPS}" ]; then
    usage
fi

if [ -d "${OUTPUT_PATH}" ]; then
  tput setaf 1
  echo "$OUTPUT_PATH already exists! Exitting."
  tput sgr0
  exit 1
fi

mkdir -p "${OUTPUT_PATH}/tcpflow"

tput setaf 2
echo "Collecting packets into $OUTPUT_PATH. Press Ctrl-C to stop collection."
tput sgr0
sudo tcpflow -T %A-%a-%B-%b-%# -I -o "${OUTPUT_PATH}/tcpflow" -i any "port 9092" 2>/dev/null

tput setaf 3
echo "Generating reports and opening them in the browser."
tput sgr0
"$DIR"/kafkasnoop_generate.sh -i "${OUTPUT_PATH}" -c "${CLIENT_IPS}"