#!/bin/bash

set -x
set -eo pipefail

if [ -d .deno_dir/LAMBDA_TASK_ROOT ]; then
  rm -r .deno_dir/LAMBDA_TASK_ROOT
fi
DENO_DIR=.deno_dir deno cache lambda.ts
cp -R .deno_dir/gen/file/$PWD/ .deno_dir/LAMBDA_TASK_ROOT
zip lambda.zip -x '.deno_dir/gen/file/*' -r .deno_dir *.ts
