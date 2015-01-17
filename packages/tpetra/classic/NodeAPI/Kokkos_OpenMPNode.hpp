//@HEADER
// ************************************************************************
//
//          Kokkos: Node API and Parallel Node Kernels
//              Copyright (2008) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

#ifndef KOKKOS_OPENMPNODE_HPP_
#define KOKKOS_OPENMPNODE_HPP_

/// \file Kokkos_OpenMPNode.hpp
/// \brief Declaration and definition of the (now DEPRECATED)
///   KokkosClassic::OpenMPNode Node type.
/// \warning KokkosClassic::OpenMPNode has been DEPRECATED.  For a
///   Node that uses OpenMP for thread-level parallelism, please use
///   Kokkos::Compat::KokkosOpenMPWrapperNode instead.

#include "Kokkos_ConfigDefs.hpp"

// mfh 08 Jan 2015: Don't enable the contents of this file unless the
// appropriate CMake option is enabled.  This avoids deprecation
// warnings once we deprecate this Node type.
#ifdef HAVE_TPETRACLASSIC_OPENMP
#include "Kokkos_StandardNodeMemoryModel.hpp"
#include "Kokkos_NodeHelpers.hpp"
#include <omp.h>

#ifdef HAVE_TPETRACLASSIC_TEUCHOSKOKKOSCOMPAT
#  include "Kokkos_Core.hpp"
#endif


namespace Teuchos {
  //forward declarations
  class ParameterList;
}

namespace KokkosClassic {

  /// \class OpenMPNode
  /// \brief Kokkos Node using OpenMP for parallelization.
  ///
  /// \warning This Node type is DEPRECATED.  Please use
  ///   Kokkos::Compat::KokkosOpenMPWrapperNode instead.  That Node
  ///   type lives in
  ///   <tt>Trilinos/packages/teuchos/kokkoscompat/src/KokkosCompat_ClassicNodeAPI_Wrapper.hpp</tt>.
  ///   You should not normally have to include that header file, as
  ///   Kokkos_DefaultNode.hpp normally includes header files for all
  ///   the enabled Nodes.
  class TPETRA_DEPRECATED OpenMPNode : public StandardNodeMemoryModel {
  public:
    /// \brief This is a "classic" Node type.
    ///
    /// That means we plan to deprecate it with the 11.14 release of
    /// Trilinos, and remove it entirely with the 12.0 release.
    static const bool classic = true;

    /// \brief Constructor.
    ///
    /// We accept the following (optional) parameters:
    /// - "Num Threads" (int): The number of threads that OpenMP
    ///   should use.  If not provided, or if -1 or 0, OpenMP will
    ///   pick the number of threads in the usual way.
    /// - "Verbose" (int): If nonzero, the Kokkos Node will print
    ///    status output to std::cout.
    ///
    /// \warning If you set the "Num Threads" parameter to a positive
    ///   value, this will set the number of threads that <i>all</i>
    ///   OpenMPNode instances will use, not just this one.
    OpenMPNode(Teuchos::ParameterList &pl);

    /// \brief Default constructor (sets default parameter values).
    ///
    /// For parameters and their default values, see the documentation
    /// of the constructor that accepts a ParameterList input / output
    /// argument.
    OpenMPNode();

    //! Get default parameters for this class.
    static Teuchos::ParameterList getDefaultParameters ();

    //! Destructor.
    ~OpenMPNode ();

    /// \brief Set the number of threads that OpenMP should use.
    ///
    /// It's not necessary to call this method unless you want to
    /// change the number of threads that OpenMP should use, after
    /// constructing the Kokkos Node instance.
    ///
    /// \warning This method will affect the number of threads used by
    ///   all OpenMPNode instances.
    ///
    /// \param numThreads [in] The number of threads that OpenMP
    ///   should use.  Ignored if -1 or 0.
    void init (const int numThreads);

    /// \brief Return the human-readable name of this Node.
    ///
    /// See \ref kokkos_node_api "Kokkos Node API"
    static std::string name ();

    //! Perform a parallel for loop on the given half-exclusive index range.
    template <class WDP>
    static void parallel_for (const int beg, const int end, WDP wd) {
#pragma omp parallel for schedule(guided) default(shared)
      for (int i = beg; i < end; ++i) {
        wd.execute(i);
      }
    }

    //! Perform a parallel reduction on the given half-exclusive index range.
    template <class WDP>
    static typename WDP::ReductionType
    parallel_reduce (const int beg, const int end, WDP wd) {
      typedef typename WDP::ReductionType ReductionType;
      ReductionType threadResult = wd.identity();
      ReductionType globalResult = wd.identity();
#pragma omp parallel default(shared) firstprivate(threadResult)
      {
#pragma omp for
        for (int i = beg; i < end; ++i) {
          threadResult = wd.reduce(threadResult, wd.generate(i));
        }
#pragma omp critical
        globalResult = wd.reduce(globalResult, threadResult);
      }
      return globalResult;
    }

    //! Synchronize threads; this is a no-op for OpenMPNode.
    inline void sync () const {}

  private:
    /// \brief "Num Threads" parameter value.
    ///
    /// If <= 0, OpenMPNode ignores this and lets OpenMP pick its own
    /// number of threads.
    int curNumThreads_;

    //! Whether to print verbose status output to std::cout.
    bool verbose_;
  };

#ifdef _MSC_VER
#pragma warning(push)
// destructor could not be generated because a base class destructor is inaccessible
#pragma warning(disable : 4624)
#endif

  template <>
  class ArrayOfViewsHelper<OpenMPNode> :
    public ArrayOfViewsHelperTrivialImpl<OpenMPNode>
  {};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace KokkosClassic

#if defined(HAVE_TPETRACLASSIC_TEUCHOSKOKKOSCOMPAT) && defined(KOKKOS_HAVE_OPENMP)
namespace Kokkos {
  namespace Compat {
    template <>
    struct NodeDevice<KokkosClassic::OpenMPNode> {
      typedef Kokkos::OpenMP type;
    };
  } // namespace Compat
} // namespace Kokkos
#endif

#endif // HAVE_TPETRACLASSIC_OPENMP
#endif // KOKKOS_OPENMPNODE_HPP_