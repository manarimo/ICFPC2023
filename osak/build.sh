#!/bin/bash

cat <<EOF > build.json
{
  "path": "",
  "solverName": "$1"
}
EOF

curl -X POST -H 'Content-Type: application/json' --data @build.json https://vwbqm1f1u5.execute-api.ap-northeast-1.amazonaws.com/prod/build
