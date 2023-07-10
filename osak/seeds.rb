#!/bin/ruby

require 'set'
require 'json'

#include_list = Set.new([29, 6, 12, 30, 22, 23, 24, 25, 26, 27, 28])
include_list = Set.new([1,2,8])
exclude_list = (1..90).select {|n| !include_list.include?(n)}
base_name = "boring"

(1..20).each do |i|
  payload = JSON.dump({
    "label": "#{base_name}-#{i}",
    "count": 1,
    "limit": 20,
    "lightningSolver": "kawatea_block",
    "pillarSolver": "kawatea_pillar_iterate",
    "exclude": exclude_list,
    "env": {
      "MAX_DISTANCE_POW_MIN": 0,
      "MAX_DISTANCE_POW_MAX": 3.5
    },
    "tag": "#{base_name}-#{i}"
  })

  cmdline = "aws stepfunctions start-execution --state-machine-arn arn:aws:states:ap-northeast-1:145924489351:stateMachine:ICFPC2023-step-functions --name #{base_name}-#{i} --input '#{payload}'"
  puts cmdline
  `#{cmdline}`
end
