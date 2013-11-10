md5me
=====

A utility that renames files to their md5 value.

Depends on openssl to calculate the md5 chacksums of files. (Link against ssl when compiling).

Usage

    md5me (-l | -s) (-r) <files>

    -l Lazy (default) will skip filenames that already look like md5 checksums.
    -s Strict will process all files even if they already look like md5 checksums.
    -r Recursive. 

There is no file clobbering protection since cryptographic checksums are unique to the file contents by definition; it makes no sense to keep two copies of the same file inside the same directory.

Although the md5 algorithm is cryptographically broken it is still in wide use for the task of finding duplicate files. To this end it is still a very viable solution.
