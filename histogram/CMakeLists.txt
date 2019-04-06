# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

set_gflags_lib_name ()

# Declare micro-libs:
add_library(quickstep_histogram_HTree ../empty_src.cpp HTree.hpp)

# Module all-in-one library:
add_library(quickstep_histogram ../empty_src.cpp HistogramModule.hpp)
target_link_libraries(quickstep_histogram
                      quickstep_histogram_HTree)
# Link dependencies:
#target_link_libraries(quickstep_utility_Alignment
#                      glog
#                      quickstep_utility_Macros)

# Tests:
add_executable(htree_test "${CMAKE_CURRENT_SOURCE_DIR}/htree_test.cpp")
target_link_libraries(htree_test
					  glog
					  gtest
					  gtest_main
					  quickstep_histogram_HTree)
add_test(htree_test htree_test)
