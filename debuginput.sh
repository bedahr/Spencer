#!/bin/bash
while true; do
  read command;
  qdbus at.tugraz.Spencer /Spencer local.Spencer.SpencerAdapter.simulateInput "$command";
done
