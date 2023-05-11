#
# Copyright 2013-2022 iSignal Research Labs Pvt Ltd.
#
# This file is part of isrRAN
#
# isrRAN is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# isrRAN is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# A copy of the GNU Affero General Public License can be found in
# the LICENSE file in the top-level directory of this distribution
# and at http://www.gnu.org/licenses/.
#

add_executable(isrlog_frontend_latency benchmarks/frontend_latency.cpp)
target_link_libraries(isrlog_frontend_latency isrlog)

add_executable(isrlog_test isrlog_test.cpp)
target_link_libraries(isrlog_test isrlog)
add_test(isrlog_test isrlog_test)

add_executable(log_channel_test log_channel_test.cpp)
target_link_libraries(log_channel_test isrlog)
add_test(log_channel_test log_channel_test)

add_executable(log_backend_test log_backend_test.cpp)
target_include_directories(log_backend_test PUBLIC ../../)
target_link_libraries(log_backend_test isrlog)
add_test(log_backend_test log_backend_test)

add_executable(logger_test logger_test.cpp)
target_link_libraries(logger_test isrlog)
add_test(logger_test logger_test)

add_executable(any_test any_test.cpp)
target_link_libraries(any_test isrlog)
add_test(any_test any_test)

add_executable(file_sink_test file_sink_test.cpp)
target_include_directories(file_sink_test PUBLIC ../../)
target_link_libraries(file_sink_test isrlog)
add_test(file_sink_test file_sink_test)

add_executable(syslog_sink_test syslog_sink_test.cpp)
target_include_directories(syslog_sink_test PUBLIC ../../)
target_link_libraries(syslog_sink_test isrlog)

add_executable(file_utils_test file_utils_test.cpp)
target_include_directories(file_utils_test PUBLIC ../../)
target_link_libraries(file_utils_test isrlog)
add_test(file_utils_test file_utils_test)

add_executable(tracer_test event_trace_test.cpp)
add_definitions(-DENABLE_ISRLOG_EVENT_TRACE)
target_link_libraries(tracer_test isrlog)
add_test(tracer_test tracer_test)

add_executable(text_formatter_test text_formatter_test.cpp)
target_include_directories(text_formatter_test PUBLIC ../../)
target_link_libraries(text_formatter_test isrlog)
add_test(text_formatter_test text_formatter_test)

add_executable(json_formatter_test json_formatter_test.cpp)
target_include_directories(json_formatter_test PUBLIC ../../)
target_link_libraries(json_formatter_test isrlog)
add_test(json_formatter_test json_formatter_test)

add_executable(context_test context_test.cpp)
target_link_libraries(context_test isrlog)
add_test(context_test context_test)
