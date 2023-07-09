#!/bin/ruby

require 'json'

sols = JSON.parse(File.read('current_best.json'))['solutions']

sols.each do |sol|
  s3path = sol['solutionPath']
  puts "Downloading #{s3path}"
  `aws s3 cp s3://icfpc2023-manarimo-3mrzsd/#{s3path} tmp/#{sol['problemId']}.json`
end

pwd = `pwd`
`deno task paint-solutions #{pwd}`
