md5me
=====

A utility that renames files to their md5 value.

Depends on openssl to calculate the md5 chacksums of files. (Link against ssl when compiling).

Usage

    md5me (-l | -s) (-r) <files>

    -l Lazy (default) will skip filenames that already look like md5 checksums.
    -s Strict will process all files even if they already look like md5 checksums.
    -r Recursive. 
