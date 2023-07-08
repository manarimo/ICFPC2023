#!/bin/bash

npm run build
aws s3 sync out s3://icfpc2023-manarimo-3mrzsd/
