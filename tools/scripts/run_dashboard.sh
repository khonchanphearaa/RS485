#!/bin/bash

# run streamlist dashboard

cd "$(dirname "$0")/.."

streamlist run tools/dashboard/app.py --server.address=0.0.0.0 --server.port=8501

