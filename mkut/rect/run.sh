seq -f %g 45 | parallel -j 6 -k --no-notice './a.out < ../../problems/{}.json > out/{}.json'
