// @HEADER
// ************************************************************************
//
//               Rapid Optimization Library (ROL) Package
//                 Copyright (2014) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
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
// Questions? Contact lead developers:
//              Drew Kouri   (dpkouri@sandia.gov) and
//              Denis Ridzal (dridzal@sandia.gov)
//
// ************************************************************************
// @HEADER

/*! \file  test_01.cpp
    \brief Test line search.
*/

#define USE_HESSVEC 0

#include "ROL_TestObjectives.hpp"
#include "ROL_Algorithm.hpp"
#include "ROL_LineSearchStep.hpp"
#include "ROL_StatusTest.hpp"
#include "Teuchos_oblackholestream.hpp"
#include "Teuchos_GlobalMPISession.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"

#include <iostream>

typedef double RealT;

int main(int argc, char *argv[]) {

  Teuchos::GlobalMPISession mpiSession(&argc, &argv);

  // This little trick lets us print to std::cout only if a (dummy) command-line argument is provided.
  int iprint     = argc - 1;
  ROL::Ptr<std::ostream> outStream;
  Teuchos::oblackholestream bhs; // outputs nothing
  if (iprint > 0)
    outStream = ROL::makePtrFromRef(std::cout);
  else
    outStream = ROL::makePtrFromRef(bhs);

  int errorFlag  = 0;

  // *** Test body.

  try {

    std::string filename = "input.xml";
    Teuchos::RCP<Teuchos::ParameterList> parlist = Teuchos::rcp( new Teuchos::ParameterList() );
    Teuchos::updateParametersFromXmlFile( filename, parlist.ptr() );
    parlist->sublist("General").set("Inexact Hessian-Times-A-Vector",true);
#if USE_HESSVEC
    parlist->sublist("General").set("Inexact Hessian-Times-A-Vector",false);
#endif

    for ( ROL::ETestObjectives objFunc = ROL::TESTOBJECTIVES_ROSENBROCK; objFunc < ROL::TESTOBJECTIVES_LAST; objFunc++ ) {
      *outStream << std::endl << std::endl << ROL::ETestObjectivesToString(objFunc) << std::endl << std::endl;

      // Set up optimization problem
      ROL::Ptr<ROL::Vector<RealT> > x0, z;
      ROL::Ptr<ROL::Objective<RealT> > obj;
      ROL::getTestObjectives<RealT>(obj,x0,z,objFunc);
      ROL::Ptr<ROL::Vector<RealT> > x = x0->clone();

      // Get Dimension of Problem
      int dim = x0->dimension(); 
      parlist->sublist("General").sublist("Krylov").set("Iteration Limit", 2*dim);

      // Error Vector
      ROL::Ptr<ROL::Vector<RealT> > e = x0->clone();
      e->zero();

      for ( ROL::EDescent desc = ROL::DESCENT_STEEPEST; desc < ROL::DESCENT_LAST; desc++ ) {
        parlist->sublist("Step").sublist("Line Search").sublist("Descent Method").set("Type", ROL::EDescentToString(desc));
        if ( desc == ROL::DESCENT_NEWTON && 
             ((objFunc == ROL::TESTOBJECTIVES_LEASTSQUARES)   || 
              (objFunc == ROL::TESTOBJECTIVES_POISSONCONTROL) ||
              (objFunc == ROL::TESTOBJECTIVES_POISSONINVERSION)) ) {
          parlist->sublist("Step").sublist("Line Search").sublist("Descent Method").set("Type", ROL::EDescentToString(ROL::DESCENT_NEWTONKRYLOV));
        }
        else {
          *outStream << std::endl << std::endl << ROL::EDescentToString(desc) << std::endl << std::endl;
      
          // Define Algorithm
          ROL::Algorithm<RealT> algo("Line Search",*parlist,false);

          // Run Algorithm
          x->set(*x0);
          algo.run(*x, *obj, true, *outStream);

          // Compute Error
          e->set(*x);
          e->axpy(-1.0,*z);
          *outStream << std::endl << "Norm of Error: " << e->norm() << std::endl;
          //errorFlag += (int)(e.norm() < std::sqrt(ROL::ROL_EPSILON<RealT>())); 
        }
      }
    }
  }
  catch (std::logic_error err) {
    *outStream << err.what() << std::endl;
    errorFlag = -1000;
  }; // end try

  if (errorFlag != 0)
    std::cout << "End Result: TEST FAILED" << std::endl;
  else
    std::cout << "End Result: TEST PASSED" << std::endl;

  return 0;

}

