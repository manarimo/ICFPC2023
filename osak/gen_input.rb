#!/usr/bin/ruby

require 'json'

res = []
(1..2).each do |i|
    res << { problemId: i, solverPath: "solver/charibert", solverName: "charibert", seed: "__best__" }
end

puts JSON.dump(res)
