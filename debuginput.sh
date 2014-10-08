#!/bin/bash
while true; do
  read command;
  qdbus at.tugraz.Spencer /Spencer local.Spencer.Spencer.userInput "$command";
done
