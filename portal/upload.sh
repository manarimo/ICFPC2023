#!/bin/bash

set -x
set -eo pipefail

aws --region=ap-northeast-1 lambda update-function-code \
    --function-name icfpc2023-portal \
    --zip-file fileb://lambda.zip
