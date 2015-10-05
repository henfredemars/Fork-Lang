# Fork-Lang
Compiler and Runtime development repo for the Fork Programming Language.

Not to be confused with any extant language, Fork is an experimental langauge (under development) emphasizing 
concurrency in the language grammar to enable statement level parallelism.

For more information, contact a contributor and inquire about a formal 
specification. This will be migrated here in time.

Depends: flex, GNU bison, make, BDW collector (tree structures are hard to manage correctly always)


###Dependencies and Alternatives

Use your package management system to obtain flex and bison. For Ubuntu this is:

	sudo apt-get install aptitude;
	sudo aptitude update;
	sudo aptitude upgrade;
	sudo aptitude install flex bison build-essential libtinfo-dev;

bwd gc is included inside this repo, but if you desire to reinstall or update the version of bwd gc feel free to use/edit the following commands:

	wget http://hboehm.info/gc/gc_source/gc-some-later-version.tar.gz

	tar xvf gc-some-later-version

	mv gc-some-later-version gc

	cd ./gc

	git clone https://github.com/ivmai/libatomic_ops.git


Building the compiler will take a long time your first build due to llvm compilation, do not use -j in make because you will most likely run out of memory

###Building Fork

To Build The Compiler(lexer, parser, code generator):

	make

To Build only BWD GC:

	make .gc_built_marker

To Build only LLVM:

	make .llvm_built_marker

In order to store error logs(stored in fork_log):

	make log

###After Building Fork

At this point, Fork programs can be parsed into an AST representation with the following command:

	./fc.py program.fk

--CODE GENERATION TO BE IMPLEMENTED

###Additional Information

A test binary tree program is available in ./Bench/C++ and ./Bench/Fork to provide examples for statement parallelism.

Test Documentation is available at ./Testing/Docs/ which tests programs in ./Testing/Programs.

