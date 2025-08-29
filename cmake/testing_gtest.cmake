add_executable(mram_driver_test
	${test_src})
	
target_link_libraries(mram_driver_test
PRIVATE
	gtest
	gmock
	pthread
)
