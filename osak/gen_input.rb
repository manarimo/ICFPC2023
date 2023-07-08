#!/usr/bin/ruby

require 'json'

res = []
(1..90).each do |i|
    res << { problemId: i, solverPath: "solver/kawatea_block_pillar", solverName: "kawatea_block_pillar" }
end

puts JSON.dump(res)