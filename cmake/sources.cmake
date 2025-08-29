set(sources
	src/bureau_codec.cpp
	src/bureau_store.cpp
	src/mram_mr25h40.cpp
)

set(exe_sources
	src/main.cpp
	${sources}
)

set(headers
	include/
)

set(test_src
	test/src/main_test.cpp
	test/src/bureau_codec_test.cpp
	test/src/bureau_store_test.cpp
	test/src/e2e_test.cpp
	test/src/mram_test.cpp
	${sources}
)
