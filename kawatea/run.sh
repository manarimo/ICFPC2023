seq -f %g 45 | parallel -j 6 -k --no-notice './a.exe < ../problems/{}.json > out/{}.json'
