#!/bin/bash
echo "Searching bluetooth module"
#gnome-terminal --working-directory=/path/to/dir
hcitool scan > temp.txt
input="temp.txt"
module=HC-05
found="no"
while read -r line
do
  # check for HC-05 module
  if grep -q "$module" <<< "$line"; then
     
     sudo rfcomm connect hci0 $(echo "$line" | awk '{print $1}')
     found="yes"
     break
  fi
  
done < "$input"

rm temp.txt

# check if successfully find module
if [ $found == 'no' ] 
then
   echo "module not found, retry"
fi
