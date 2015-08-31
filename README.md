# Fork-Lang
Compiler and Runtime development repo for the Fork Programming Language.

Not to be confused with any extant language, Fork is an experimental langauge (under development) emphasizing 
concurrency in the language grammar.

For more information, contact a contributor and inquire about a formal 
specification. This will be migrated here in time.

Depends: flex, GNU bison, make, BDW collector (tree structures are hard to manage correctly always)

For the collector:

wget http://hboehm.info/gc/gc_source/gc-7.2f.tar.gz

tar xvf gc-7.2

cd gc-7.2

./configure --prefix=/usr/local/ --enable-threads=posix --enable-thread-local-alloc --enable-parallel-mark --enable-cplusplus
  
make; make check; sudo make install

Use your package management system to obtain flex and bison. For Ubuntu this is:

sudo apt-get install aptitude;
sudo aptitude update;
sudo aptitude install flex bison build-essential;

add path fix file 
touch /usr/local/include/gc_allocator.h
echo "#include <gc/gc_allocator.h>" > /usr/local/include/gc_allocator.h

In order to test:
make

In order to store error logs:

make log

logs are stored in fork_log