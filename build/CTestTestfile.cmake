# CMake generated Testfile for 
# Source directory: /home/artem/CProjects/Para_CL_Language
# Build directory: /home/artem/CProjects/Para_CL_Language/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(lexer "/home/artem/CProjects/Para_CL_Language/build/lexer_test" "/home/artem/CProjects/Para_CL_Language/build/lexer_input.pcl")
set_tests_properties(lexer PROPERTIES  _BACKTRACE_TRIPLES "/home/artem/CProjects/Para_CL_Language/CMakeLists.txt;26;add_test;/home/artem/CProjects/Para_CL_Language/CMakeLists.txt;0;")
add_test(parser "/home/artem/CProjects/Para_CL_Language/build/parser_test" "/home/artem/CProjects/Para_CL_Language/build/parser_input.pcl")
set_tests_properties(parser PROPERTIES  _BACKTRACE_TRIPLES "/home/artem/CProjects/Para_CL_Language/CMakeLists.txt;30;add_test;/home/artem/CProjects/Para_CL_Language/CMakeLists.txt;0;")
add_test(semantic_analyzer "/home/artem/CProjects/Para_CL_Language/build/semantic_analyzer_test" "/home/artem/CProjects/Para_CL_Language/build/semantic_input.pcl")
set_tests_properties(semantic_analyzer PROPERTIES  _BACKTRACE_TRIPLES "/home/artem/CProjects/Para_CL_Language/CMakeLists.txt;34;add_test;/home/artem/CProjects/Para_CL_Language/CMakeLists.txt;0;")
