#!/bin/bash
/Applications/xlDo -A -v -c "{\"cmd\":\"checkSequence\", \"seq\":\"$1\"}" -s result.sh

if [ $? -eq 0 ]
then
  chmod 755 result.sh
  source result.sh
  open -t $output
else
  echo "Error level: $?"
fi
