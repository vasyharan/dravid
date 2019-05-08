#/bin/sh
root=$(realpath "$(dirname $0)/..")
build=($realpath "${root}/build")
fixtures=($realpath "${root}/test/snapshot/_fixtures")
"$build/test/snapshot/test-snapshot" $@ -- "$root/test/snapshot/_fixtures" | \
    sed -e 's,^FAIL: "\(.*\)\.vd\.\(.*\)"$,icdiff '"$fixtures"'/\1.\2.snap '"$fixtures"'/\1.\2.out,' | \
    sed -e 's,^\(PASS:.*$\),echo \"\1\",' | sh
