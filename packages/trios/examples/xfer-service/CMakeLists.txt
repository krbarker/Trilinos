
# @HEADER
#  ************************************************************************
#
#                    Trios: Trilinos I/O Support
#                  Copyright 2011 Sandia Corporation
#
#  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
#  the U.S. Government retains certain rights in this software.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#  1. Redistributions of source code must retain the above copyright
#  notice, this list of conditions and the following disclaimer.
#
#  2. Redistributions in binary form must reproduce the above copyright
#  notice, this list of conditions and the following disclaimer in the
#  documentation and/or other materials provided with the distribution.
#
#  3. Neither the name of the Corporation nor the names of the
#  contributors may be used to endorse or promote products derived from
#  this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
#  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Questions? Contact Ron A. Oldfield (raoldfi@sandia.gov)
#
#  *************************************************************************
# @HEADER


INCLUDE(TriosProcessXDR)



INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Generate the XDR header and source file for the service arguments
TriosProcessXDR(${CMAKE_CURRENT_SOURCE_DIR}/xfer_service_args.x)


TRIBITS_ADD_EXECUTABLE(
  XferMPITest
  SOURCES xfer_service_args.c xfer_util.cpp xfer_mpi_server.cpp xfer_mpi_test.cpp xfer_mpi_client.cpp
  DEPLIBS ${DEPLIBS}
  NOEXEPREFIX
)


TRIBITS_ADD_EXECUTABLE(
  XferServiceTest
  SOURCES xfer_service_args.c xfer_util.cpp xfer_server.cpp xfer_threads.cpp xfer_service_test.cpp xfer_client.cpp
  DEPLIBS ${DEPLIBS}
  NOEXEPREFIX
)

SET(EXTRA_TEST_ARGS "")

# Gemini network transport takes a few seconds to initialize
IF (${PACKAGE_NAME}_ENABLE_Gemini)
  APPEND_SET(EXTRA_TEST_ARGS "--delay=3")
ENDIF()


SET(TESTNAMES
   write-encode-sync
   write-encode-async
   write-rdma-sync
   write-rdma-async
   read-encode-sync
   read-encode-async
   read-rdma-sync
   read-rdma-async)


foreach (testname ${TESTNAMES})

  SET(DEFAULT_ARGS "--io-method=${testname} --validate ")
  SET(DEFAULT_ARGS "${DEFAULT_ARGS} --result-file=${testname}-result.out ")
  SET(DEFAULT_ARGS "${DEFAULT_ARGS} --result-file-mode=a ")
  SET(DEFAULT_ARGS "${DEFAULT_ARGS} --num-trials=5 ")

  TRIBITS_ADD_TEST(
    XferServiceTest
    NOEXEPREFIX
    NAME XferService_${testname}
    ARGS "${DEFAULT_ARGS} ${EXTRA_TEST_ARGS}"
    COMM serial mpi
    NUM_MPI_PROCS 2)

  #SET_TESTS_PROPERTIES(${PACKAGE_NAME}_XferService_${testname}_MPI_2 PROPERTIES TIMEOUT 10)
endforeach()


