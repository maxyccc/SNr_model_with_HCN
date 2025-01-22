#!/bin/bash

for script in ./sh_commands/*.sh; do
    echo "Running $script..."
    bash "$script"
done