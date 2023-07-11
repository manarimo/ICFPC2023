require 'json'

tags = JSON.parse(File.read('tags.json'))
tags['tags'].each do |tag|
  puts tag
  `curl https://vwbqm1f1u5.execute-api.ap-northeast-1.amazonaws.com/prod/solutions?tag=#{tag} > solutions_#{tag}.json`
end
