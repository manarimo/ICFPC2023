#!/bin/bash

set -x
set -eo pipefail

aws --region=ap-northeast-1 lambda update-function-code \
    --function-name icfpc2023-looper \
    --zip-file fileb://dist/index_looper.zip
