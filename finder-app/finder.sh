#! /bin/bash

files_dir=$1
search_str=$2

if [ $# -lt 1 ]; then
	exit 1
fi
if [[ -d $files_dir ]]; then
	files_count=$(find "$files_dir" -type f | wc -l)
	lines_count=$( grep -rc "$files_dir" -e "search_str" | wc -l)
	echo "The number of files are " $files_count " and the  number of matching lines are " $files_count
else 
	exit 1
fi
