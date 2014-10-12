#!/bin/bash

cat $1| sed 's/^.*\t.*\t//' | while read transcript; do
  read -p "About to send: $transcript" < /dev/tty
  qdbus at.tugraz.Spencer /Spencer local.Spencer.SpencerAdapter.simulateInput "$transcript";
done
