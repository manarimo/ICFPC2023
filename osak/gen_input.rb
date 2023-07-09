#!/usr/bin/ruby

require 'json'

res = []
(1..55).each do |i|
    res << { problemId: i, solverPath: "solver/charibert", solverName: "charibert_07-09-1510", seed: "__best__" }
end
(56..90).each do |i|
    res << { problemId: i, solverPath: "solver/charibert_pillar", solverName: "charibert_07-09-1510", seed: " synced-bests-07091459" }
end


puts JSON.dump(res)
