#include "Ifpack_ConfigDefs.h"
#ifdef HAVE_IFPACK_TEUCHOS
#include <iomanip>
#include "Epetra_Operator.h"
#include "Epetra_RowMatrix.h"
#include "Epetra_Comm.h"
#include "Epetra_Map.h"
#include "Epetra_MultiVector.h"
#include "Epetra_Vector.h"
#include "Epetra_Time.h"
#include "Ifpack_Chebyshev.h"
#include "Ifpack_Utils.h"
#include "Ifpack_Condest.h"


//==============================================================================
Ifpack_Chebyshev::
Ifpack_Chebyshev(const Epetra_RowMatrix* Matrix) :
  IsInitialized_(false),
  IsComputed_(false),
  NumInitialize_(0),
  NumCompute_(0),
  NumApplyInverse_(0),
  InitializeTime_(0.0),
  ComputeTime_(0.0),
  ApplyInverseTime_(0.0),
  ComputeFlops_(0.0),
  ApplyInverseFlops_(0.0),
  PolyDegree_(1),
  UseTranspose_(false),
  Condest_(-1.0),
  ComputeCondest_(false),
  EigRatio_(30.0),
  Label_(),
  LambdaMin_(0.0),
  LambdaMax_(100.0),
  MinDiagonalValue_(0.0),
  NumMyRows_(0),
  NumMyNonzeros_(0),
  NumGlobalRows_(0),
  NumGlobalNonzeros_(0),
  Matrix_(Matrix),
  InvDiagonal_(0),
  Time_(0),
  ZeroStartingSolution_(true)
{

}

//==============================================================================
Ifpack_Chebyshev::~Ifpack_Chebyshev()
{
  if (InvDiagonal_)
    delete InvDiagonal_;
  if (Time_)
    delete Time_;
}

//==============================================================================
int Ifpack_Chebyshev::SetParameters(Teuchos::ParameterList& List)
{

  EigRatio_             = List.get("chebyshev: ratio eigenvalue", EigRatio_);
  LambdaMin_            = List.get("chebyshev: min eigenvalue", LambdaMin_);
  LambdaMax_            = List.get("chebyshev: max eigenvalue", LambdaMax_);
  PolyDegree_           = List.get("chebyshev: degree",PolyDegree_);
  MinDiagonalValue_     = List.get("chebyshev: min diagonal value", 
                                   MinDiagonalValue_);
  ZeroStartingSolution_ = List.get("chebyshev: zero starting solution", 
                                   ZeroStartingSolution_);

  SetLabel();

  return(0);
}

//==============================================================================
const Epetra_Comm& Ifpack_Chebyshev::Comm() const
{
  return(Matrix_->Comm());
}

//==============================================================================
const Epetra_Map& Ifpack_Chebyshev::OperatorDomainMap() const
{
  return(Matrix_->OperatorDomainMap());
}

//==============================================================================
const Epetra_Map& Ifpack_Chebyshev::OperatorRangeMap() const
{
  return(Matrix_->OperatorRangeMap());
}

//==============================================================================
int Ifpack_Chebyshev::
Apply(const Epetra_MultiVector& X, Epetra_MultiVector& Y) const
{
  if (IsComputed() == false)
    IFPACK_CHK_ERR(-3);

  if (X.NumVectors() != Y.NumVectors())
    IFPACK_CHK_ERR(-2);

  IFPACK_CHK_ERR(Matrix_->Multiply(UseTranspose(),X,Y));
  return(0);
}

//==============================================================================
int Ifpack_Chebyshev::Initialize()
{
  IsInitialized_ = false;

  if (Matrix_ == 0)
    IFPACK_CHK_ERR(-2);

  if (Time_ == 0)
    Time_ = new Epetra_Time(Comm());

  if (Matrix().NumGlobalRows() != Matrix().NumGlobalCols())
    IFPACK_CHK_ERR(-2); // only square matrices

  NumMyRows_ = Matrix_->NumMyRows();
  NumMyNonzeros_ = Matrix_->NumMyNonzeros();
  NumGlobalRows_ = Matrix_->NumGlobalRows();
  NumGlobalNonzeros_ = Matrix_->NumGlobalNonzeros();

  ++NumInitialize_;
  InitializeTime_ += Time_->ElapsedTime();
  IsInitialized_ = true;
  return(0);
}

//==============================================================================
int Ifpack_Chebyshev::Compute()
{
  if (!IsInitialized())
    IFPACK_CHK_ERR(Initialize());

  Time_->ResetStartTime();

  // reset values
  IsComputed_ = false;
  Condest_ = -1.0;

  if (PolyDegree_ <= 0)
    IFPACK_CHK_ERR(-2); // at least one application
  
  if (InvDiagonal_)
    delete InvDiagonal_;
  InvDiagonal_ = new Epetra_Vector(Matrix().RowMatrixRowMap());

  if (InvDiagonal_ == 0)
    IFPACK_CHK_ERR(-5);

  IFPACK_CHK_ERR(Matrix().ExtractDiagonalCopy(*InvDiagonal_));

  // Inverse diagonal elements
  // Replace zeros with 1.0
  for (int i = 0 ; i < NumMyRows_ ; ++i) {
    double diag = (*InvDiagonal_)[i];
    if (IFPACK_ABS(diag) < MinDiagonalValue_)
      (*InvDiagonal_)[i] = MinDiagonalValue_;
    else
      (*InvDiagonal_)[i] = 1.0 / diag;
  }
  ComputeFlops_ += NumMyRows_;

  ++NumCompute_;
  ComputeTime_ += Time_->ElapsedTime();
  IsComputed_ = true;

  return(0);
}

//==============================================================================
ostream& Ifpack_Chebyshev::Print(ostream & os) const
{

  double MyMinVal, MyMaxVal;
  double MinVal, MaxVal;

  if (IsComputed_) {
    InvDiagonal_->MinValue(&MyMinVal);
    InvDiagonal_->MaxValue(&MyMaxVal);
    Comm().MinAll(&MyMinVal,&MinVal,1);
    Comm().MinAll(&MyMaxVal,&MaxVal,1);
  }

  if (!Comm().MyPID()) {
    os << endl;
    os << "================================================================================" << endl;
    os << "Ifpack_Chebyshev" << endl;
    os << "Degree of polynomial = " << PolyDegree_ << endl;
    if (ZeroStartingSolution_) 
      os << "Using zero starting solution" << endl;
    else
      os << "Using input starting solution" << endl;
    os << "Condition number estimate = " << Condest() << endl;
    os << "Global number of rows            = " << Matrix_->NumGlobalRows() << endl;
    if (IsComputed_) {
      os << "Minimum value on stored inverse diagonal = " << MinVal << endl;
      os << "Maximum value on stored inverse diagonal = " << MaxVal << endl;
    }
    os << endl;
    os << "Phase           # calls   Total Time (s)       Total MFlops     MFlops/s" << endl;
    os << "-----           -------   --------------       ------------     --------" << endl;
    os << "Initialize()    "   << std::setw(5) << NumInitialize_ 
       << "  " << std::setw(15) << InitializeTime_ 
       << "              0.0              0.0" << endl;
    os << "Compute()       "   << std::setw(5) << NumCompute_ 
       << "  " << std::setw(15) << ComputeTime_
       << "  " << std::setw(15) << 1.0e-6 * ComputeFlops_;
    if (ComputeTime_ != 0.0)
      os << "  " << std::setw(15) << 1.0e-6 * ComputeFlops_ / ComputeTime_ << endl;
    else
      os << "  " << std::setw(15) << 0.0 << endl;
    os << "ApplyInverse()  "   << std::setw(5) << NumApplyInverse_ 
       << "  " << std::setw(15) << ApplyInverseTime_
       << "  " << std::setw(15) << 1.0e-6 * ApplyInverseFlops_;
    if (ApplyInverseTime_ != 0.0)
      os << "  " << std::setw(15) << 1.0e-6 * ApplyInverseFlops_ / ApplyInverseTime_ << endl;
    else
      os << "  " << std::setw(15) << 0.0 << endl;
    os << "================================================================================" << endl;
    os << endl;
  }

  return(os);
}

//==============================================================================
double Ifpack_Chebyshev::
Condest(const Ifpack_CondestType CT, 
        const int MaxIters, const double Tol,
	Epetra_RowMatrix* Matrix)
{
  if (!IsComputed()) // cannot compute right now
    return(-1.0);

  // always computes it. Call Condest() with no parameters to get
  // the previous estimate.
  Condest_ = Ifpack_Condest(*this, CT, MaxIters, Tol, Matrix);

  return(Condest_);
}

//==============================================================================
void Ifpack_Chebyshev::SetLabel()
{
  Label_ = "IFPACK (Chebyshev polynomial), degree=" + Ifpack_toString(PolyDegree_);

}

//==============================================================================
int Ifpack_Chebyshev::
ApplyInverse(const Epetra_MultiVector& X, Epetra_MultiVector& Y) const
{
  if (!IsComputed())
    IFPACK_CHK_ERR(-3);

  if (PolyDegree_ == 0)
    return 0;

  int nVec = X.NumVectors();
  int len = X.MyLength();
  if (nVec != Y.NumVectors())
    IFPACK_CHK_ERR(-2);

  Time_->ResetStartTime();

  // AztecOO gives X and Y pointing to the same memory location,
  // need to create an auxiliary vector, Xcopy
  const Epetra_MultiVector* Xcopy;
  if (X.Pointers()[0] == Y.Pointers()[0])
    Xcopy = new Epetra_MultiVector(X);
  else
    Xcopy = &X;

  double **xPtr = 0, **yPtr = 0;
  Xcopy->ExtractView(&xPtr);
  Y.ExtractView(&yPtr);

  //--- Do a quick solve when the matrix is identity
  double *invDiag = InvDiagonal_->Values();
  if ((LambdaMin_ == 1.0) && (LambdaMax_ == LambdaMin_)) {
    if (nVec == 1) {
      double *yPointer = yPtr[0], *xPointer = xPtr[0];
      for (int i = 0; i < len; ++i)
        yPointer[i] = xPointer[i]*invDiag[i];
    }
    else {
      int i, k;
      for (i = 0; i < len; ++i) {
        double coeff = invDiag[i];
        for (k = 0; k < nVec; ++k)
          yPtr[k][i] = xPtr[k][i] * coeff;
      }
    } // if (nVec == 1)
    return 0;
  } // if ((LambdaMin_ == 1.0) && (LambdaMax_ == LambdaMin_))

  //--- Initialize coefficients
  // Note that delta stores the inverse of ML_Cheby::delta
  double alpha = LambdaMax_ / EigRatio_;
  double beta = 1.1 * LambdaMax_;
  double delta = 2.0 / (beta - alpha);
  double theta = 0.5 * (beta + alpha);
  double s1 = theta * delta;

  //--- Define vectors
  // In ML_Cheby, V corresponds to pAux and W to dk
  Epetra_MultiVector V(X);
  Epetra_MultiVector W(X);

  double *vPointer = V.Values(), *wPointer = W.Values();

  double oneOverTheta = 1.0/theta;
  int i, j, k;

  // Do the smoothing when block scaling is turned OFF
  // --- Treat the initial guess
  if (ZeroStartingSolution_ == false) {
    Matrix_->Apply(Y, V);
    // Compute W = invDiag * ( X - V )/ Theta
    if (nVec == 1) {
      double *xPointer = xPtr[0];
      for (i = 0; i < len; ++i)
        wPointer[i] = (xPointer[i] - vPointer[i])*invDiag[i]*oneOverTheta;
    }
    else {
      for (i = 0; i < len; ++i) {
        double coeff = invDiag[i]*oneOverTheta;
        double *wi = wPointer + i, *vi = vPointer + i;
        for (k = 0; k < nVec; ++k) {
          *wi = (xPtr[k][i] - (*vi)) * coeff;
          wi = wi + len; vi = vi + len;
        }
      }
    } // if (nVec == 1)
    // Update the vector Y
    Y.Update(1.0, W, 1.0);
  }
  else {
    // Compute W = invDiag * X / Theta
    if (nVec == 1) {
      double *xPointer = xPtr[0];
      for (i = 0; i < len; ++i)
        wPointer[i] = xPointer[i]*invDiag[i]*oneOverTheta;
      memcpy(yPtr[0], wPointer, len*sizeof(double));
    }
    else {
      for (i = 0; i < len; ++i) {
        double coeff = invDiag[i]*oneOverTheta;
        double *wi = wPointer + i;
        for (k = 0; k < nVec; ++k) {
          *wi = xPtr[k][i] * coeff;
          wi = wi + len;
        }
      }
      for (k = 0; k < nVec; ++k)
        memcpy(yPtr[k], wPointer + k*len, len*sizeof(double));
    } // if (nVec == 1)
  } // if (ZeroStartingSolution_ == false)

  //--- Apply the polynomial
  double rhok = 1.0/s1, rhokp1;
  double dtemp1, dtemp2;
  int degreeMinusOne = PolyDegree_ - 1;
  if (nVec == 1) {
    double *xPointer = xPtr[0];
    for (k = 0; k < degreeMinusOne; ++k) {
      Matrix_->Apply(Y, V);
      rhokp1 = 1.0 / (2.0*s1 - rhok);
      dtemp1 = rhokp1 * rhok;
      dtemp2 = 2.0 * rhokp1 * delta;
      rhok = rhokp1;
      // Compute W = dtemp1 * W
      W.Scale(dtemp1);
      // Compute W = W + dtemp2 * invDiag * ( X - V )
      for (i = 0; i < len; ++i)
        wPointer[i] += dtemp2*(xPointer[i] - vPointer[i])*invDiag[i];
      // Update the vector Y
      Y.Update(1.0, W, 1.0);
    } // for (k = 0; k < degreeMinusOne; ++k)
  }
  else {
    for (k = 0; k < degreeMinusOne; ++k) {
      Matrix_->Apply(Y, V);
      rhokp1 = 1.0 / (2.0*s1 - rhok);
      dtemp1 = rhokp1 * rhok;
      dtemp2 = 2.0 * rhokp1 * delta;
      rhok = rhokp1;
      // Compute W = dtemp1 * W
      W.Scale(dtemp1);
      // Compute W = W + dtemp2 * invDiag * ( X - V )
      for (i = 0; i < len; ++i) {
        double coeff = invDiag[i]*dtemp2;
        double *wi = wPointer + i, *vi = vPointer + i;
        for (j = 0; j < nVec; ++j) {
          *wi += (xPtr[j][i] - (*vi)) * coeff;
          wi = wi + len; vi = vi + len;
        }
      }
      // Update the vector Y
      Y.Update(1.0, W, 1.0);
    } // for (k = 0; k < degreeMinusOne; ++k)
  } // if (nVec == 1)

  // Flops are updated in each of the following. 

  if (Xcopy != &X)
    delete Xcopy;

  ++NumApplyInverse_;
  ApplyInverseTime_ += Time_->ElapsedTime();
  return(0);

}

#endif
