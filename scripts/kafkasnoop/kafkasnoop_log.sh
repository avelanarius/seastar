#!/bin/bash

usage() {
    echo "Usage: $0 [-o OUTPUT_PATH]" 1>&2; exit 1;
}

while getopts ":o:" o; do
    case "${o}" in
        o)
            OUTPUT_PATH=${OPTARG}
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

if [ -z "${OUTPUT_PATH}" ]; then
    usage
fi

if [ ! -d "${OUTPUT_PATH}" ]; then
  tput setaf 1
  echo "$OUTPUT_PATH does not exist! Exitting."
  tput sgr0
  exit 1
fi

while IFS= read -r line; do
  TIMESTAMP=$(date +%s.%N)
  LOG_LINE=$(printf '%s|%s' "$TIMESTAMP" "$line")
  echo "$LOG_LINE" >> "$OUTPUT_PATH/log_info.txt"
done