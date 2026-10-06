#!/bin/bash

# run realtime monitoring service

cd "$(dirname "$0")/.."

python3 -m tools.services.log_service
