#!/bin/ruby

require 'set'
require 'json'

include_list = Set.new([8, 2, 1, 12, 30])
exclude_list = (1..90).select {|n| !include_list.include?(n)}

(1..20).each do |i|
  payload = JSON.dump({
    "label": "top-seed-#{i}",
    "count": 1,
    "limit": 20,
    "lightningSolver": "kawatea_block",
    "pillarSolver": "kawatea_pillar_iterate",
    "exclude": exclude_list,
    "env": {
      "MAX_DISTANCE_POW_MIN": 0,
      "MAX_DISTANCE_POW_MAX": 3.5
    },
    "tag": "top-seed-#{i}"
  })

  cmdline = "aws stepfunctions start-execution --state-machine-arn arn:aws:states:ap-northeast-1:145924489351:stateMachine:ICFPC2023-step-functions --name top-seed-#{i} --input '#{payload}'"
  puts cmdline
  `#{cmdline}`
end
