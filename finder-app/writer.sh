file=$1
str=$2

if [ $# -ne 2 ]; then
	exit 1
elif [ $# -lt 1 ]; then
	exit 1
else 
	dir=$(dirname "$file")
	if [ ! -d "$dir" ]; then
		mkdir -p "$dir"
	fi
	echo "$str" >> "$file"
fi

