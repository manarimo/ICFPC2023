#!/bin/bash

npm --prefix next-portal run build

# TODO next-portal/out にファイルが生成されているので、それを S3 にアップロードする
ls -l next-portal/out
