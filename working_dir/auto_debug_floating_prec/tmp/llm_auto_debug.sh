
mkdir -p tmp
../auto_debug 2.5 > tmp/output1.txt
../auto_debug -3.75 > tmp/output2.txt
diff tmp/output1.txt tmp/output2.txt > tmp/diff.txt
            