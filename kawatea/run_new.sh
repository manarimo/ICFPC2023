seq 56 90 | parallel -j 6 -k --no-notice './a.exe < ../problems/{}.json > out/{}.json'
