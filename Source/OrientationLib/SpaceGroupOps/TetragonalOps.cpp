/* ============================================================================
* Copyright (c) 2009-2015 BlueQuartz Software, LLC
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


#include "TetragonalOps.h"

#ifdef SIMPLib_USE_PARALLEL_ALGORITHMS
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/partitioner.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/task_group.h>
#include <tbb/task.h>
#endif

// Include this FIRST because there is a needed define for some compiles
// to expose some of the constants needed below
#include "SIMPLib/Math/SIMPLibMath.h"
#include "SIMPLib/Utilities/ColorTable.h"

#include "OrientationLib/OrientationMath/OrientationArray.hpp"
#include "OrientationLib/OrientationMath/OrientationTransforms.hpp"
#include "OrientationLib/Utilities/ModifiedLambertProjection.h"
#include "OrientationLib/Utilities/PoleFigureUtilities.h"


namespace Detail
{

  static const float TetraDim1InitValue = powf((0.75f * ((SIMPLib::Constants::k_Pi / 2.0f) - sinf((SIMPLib::Constants::k_Pi / 2.0f)))), (1.0f / 3.0f));
  static const float TetraDim2InitValue = powf((0.75f * ((SIMPLib::Constants::k_Pi / 2.0f) - sinf((SIMPLib::Constants::k_Pi / 2.0f)))), (1.0f / 3.0f));
  static const float TetraDim3InitValue = powf((0.75f * ((SIMPLib::Constants::k_Pi / 4.0f) - sinf((SIMPLib::Constants::k_Pi / 4.0f)))), (1.0f / 3.0f));
  static const float TetraDim1StepValue = TetraDim1InitValue / 18.0f;
  static const float TetraDim2StepValue = TetraDim2InitValue / 18.0f;
  static const float TetraDim3StepValue = TetraDim3InitValue / 9.0f;

  static const float TetraRodSym[8][3] = {{0.0f, 0.0f, 0.0f},
    {10000000000.0f, 0.0f, 0.0f},
    {0.0f, 10000000000.0f, 0.0f},
    {0.0f, 0.0f, 10000000000.0f},
    {0.0f, 0.0f, -1.0f},
    {0.0f, 0.0f, 1.0f},
    {10000000000.0f, 10000000000.0f, 0.0},
    { -10000000000.0f, 10000000000.0f, 0.0}
  };

  namespace TetragonalHigh
  {
    static const int symSize0 = 2;
    static const int symSize1 = 4;
    static const int symSize2 = 4;
  }
}

static const QuatF TetraQuatSym[8] = {QuaternionMathF::New(0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f),
                                      QuaternionMathF::New(1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f),
                                      QuaternionMathF::New(0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f),
                                      QuaternionMathF::New(0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f),
                                      QuaternionMathF::New(0.000000000f, 0.000000000f, SIMPLib::Constants::k_1OverRoot2, -SIMPLib::Constants::k_1OverRoot2),
                                      QuaternionMathF::New(0.000000000f, 0.000000000f, SIMPLib::Constants::k_1OverRoot2, SIMPLib::Constants::k_1OverRoot2),
                                      QuaternionMathF::New(SIMPLib::Constants::k_1OverRoot2, SIMPLib::Constants::k_1OverRoot2, 0.000000000f, 0.000000000f),
                                      QuaternionMathF::New(-SIMPLib::Constants::k_1OverRoot2, SIMPLib::Constants::k_1OverRoot2, 0.000000000f, 0.000000000f)
                                     };


static const float TetraMatSym[8][3][3] =
{
  { {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}
  },

  { {1.0, 0.0,  0.0},
    {0.0, -1.0,  0.0},
    {0.0, 0.0, -1.0}
  },

  { { -1.0,  0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0,  0.0, -1.0}
  },

  { { -1.0, 0.0, 0.0},
    {0.0, -1.0, 0.0},
    {0.0, 0.0, 1.0}
  },

  { {0.0, 1.0, 0.0},
    { -1.0, 0.0, 0.0},
    {0.0, 0.0, 1.0}
  },

  { {0.0, -1.0, 0.0},
    {1.0,  0.0, 0.0},
    {0.0,  0.0, 1.0}
  },

  { {0.0, 1.0, 0.0},
    {1.0,  0.0, 0.0},
    {0.0,  0.0, -1.0}
  },

  { {0.0, -1.0,  0.0},
    { -1.0,  0.0,  0.0},
    {0.0,  0.0, -1.0}
  }
};


using namespace Detail;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
TetragonalOps::TetragonalOps()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
TetragonalOps::~TetragonalOps()
{
  // TODO Auto-generated destructor stub
}

float TetragonalOps::_calcMisoQuat(const QuatF quatsym[8], int numsym,
                                   QuatF& q1, QuatF& q2,
                                   float& n1, float& n2, float& n3)
{
  float wmin = 9999999.0f; //,na,nb,nc;
  float w = 0;
  float n1min = 0.0f;
  float n2min = 0.0f;
  float n3min = 0.0f;
  QuatF qr;
  QuatF qc;
  QuatF q2inv;

  QuaternionMathF::Copy(q2, q2inv);
  QuaternionMathF::Conjugate(q2inv);

  QuaternionMathF::Multiply(q1, q2inv, qr);
  for (int i = 0; i < numsym; i++)
  {
    QuaternionMathF::Multiply(quatsym[i], qr, qc);
    if (qc.w < -1)
    {
      qc.w = -1;
    }
    else if (qc.w > 1)
    {
      qc.w = 1;
    }

    FOrientArrayType ax(4, 0.0f);
    FOrientTransformsType::qu2ax(FOrientArrayType(qc.x, qc.y, qc.z, qc.w), ax);
    ax.toAxisAngle(n1, n2, n3, w);

    if (w > SIMPLib::Constants::k_Pi)
    {
      w = SIMPLib::Constants::k_2Pi - w;
    }
    if (w < wmin)
    {
      wmin = w;
      n1min = n1;
      n2min = n2;
      n3min = n3;
    }
  }
  float denom = sqrt((n1min * n1min + n2min * n2min + n3min * n3min));
  n1 = n1min / denom;
  n2 = n2min / denom;
  n3 = n3min / denom;
  if(denom == 0)
  {
    n1 = 0.0, n2 = 0.0, n3 = 1.0;
  }
  if(wmin == 0)
  {
    n1 = 0.0, n2 = 0.0, n3 = 1.0;
  }
  return wmin;
}

float TetragonalOps::getMisoQuat(QuatF& q1, QuatF& q2, float& n1, float& n2, float& n3)
{
  int numsym = 8;

  return _calcMisoQuat(TetraQuatSym, numsym, q1, q2, n1, n2, n3);
}

void TetragonalOps::getQuatSymOp(int i, QuatF& q)
{
  QuaternionMathF::Copy(TetraQuatSym[i], q);
  //  q.x = TetraQuatSym[i][0];
  //  q.y = TetraQuatSym[i][1];
  //  q.z = TetraQuatSym[i][2];
  //  q.w = TetraQuatSym[i][3];
}

void TetragonalOps::getRodSymOp(int i, float* r)
{
  r[0] = TetraRodSym[i][0];
  r[1] = TetraRodSym[i][1];
  r[2] = TetraRodSym[i][2];
}

void TetragonalOps::getMatSymOp(int i, float g[3][3])
{
  g[0][0] = TetraMatSym[i][0][0];
  g[0][1] = TetraMatSym[i][0][1];
  g[0][2] = TetraMatSym[i][0][2];
  g[1][0] = TetraMatSym[i][1][0];
  g[1][1] = TetraMatSym[i][1][1];
  g[1][2] = TetraMatSym[i][1][2];
  g[2][0] = TetraMatSym[i][2][0];
  g[2][1] = TetraMatSym[i][2][1];
  g[2][2] = TetraMatSym[i][2][2];
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FOrientArrayType TetragonalOps::getODFFZRod(FOrientArrayType rod)
{
  int  numsym = 8;

  return _calcRodNearestOrigin(TetraRodSym, numsym, rod);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FOrientArrayType TetragonalOps::getMDFFZRod(FOrientArrayType rod)
{
  float w, n1, n2, n3;
  float FZn1, FZn2, FZn3, FZw;

  FOrientArrayType ax(4, 0.0f);
  OrientationTransforms<FOrientArrayType, float>::ro2ax(rod, ax);
  n1 = ax[0];
  n2 = ax[1], n3 = ax[2], w = ax[3];

  FZn1 = fabs(n1);
  FZn2 = fabs(n2);
  FZn3 = fabs(n3);
  FZw = w;

  ax.fromAxisAngle(FZn1, FZn2, FZn3, FZw);
  OrientationTransforms<FOrientArrayType, float>::ax2ro(ax, rod);
  return rod;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void TetragonalOps::getNearestQuat(QuatF& q1, QuatF& q2)
{
  int numsym = 8;

  _calcNearestQuat(TetraQuatSym, numsym, q1, q2);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int TetragonalOps::getMisoBin(FOrientArrayType rod)
{
  float dim[3];
  float bins[3];
  float step[3];

  FOrientArrayType ho(3);
  OrientationTransforms<FOrientArrayType, float>::ro2ho(rod, ho);

  dim[0] = TetraDim1InitValue;
  dim[1] = TetraDim2InitValue;
  dim[2] = TetraDim3InitValue;
  step[0] = TetraDim1StepValue;
  step[1] = TetraDim2StepValue;
  step[2] = TetraDim3StepValue;
  bins[0] = 36.0;
  bins[1] = 36.0;
  bins[2] = 18.0;

  return _calcMisoBin(dim, bins, step, ho);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FOrientArrayType TetragonalOps::determineEulerAngles(uint64_t seed, int choose)
{
  float init[3];
  float step[3];
  int32_t phi[3];
  float h1, h2, h3;

  init[0] = TetraDim1InitValue;
  init[1] = TetraDim2InitValue;
  init[2] = TetraDim3InitValue;
  step[0] = TetraDim1StepValue;
  step[1] = TetraDim2StepValue;
  step[2] = TetraDim3StepValue;
  phi[0] = static_cast<int32_t>(choose % 36);
  phi[1] = static_cast<int32_t>((choose / 36) % 36);
  phi[2] = static_cast<int32_t>(choose / (36 * 36));

  _calcDetermineHomochoricValues(seed, init, step, phi, choose, h1, h2, h3);

  FOrientArrayType ho(h1, h2, h3);
  FOrientArrayType ro(4);
  OrientationTransforms<FOrientArrayType, float>::ho2ro(ho, ro);

  ro = getODFFZRod(ro);
  FOrientArrayType eu(4);
  OrientationTransforms<FOrientArrayType, float>::ro2eu(ro, eu);
  return eu;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FOrientArrayType TetragonalOps::randomizeEulerAngles(FOrientArrayType synea)
{
  QuatF q;
  QuatF qc;
  size_t symOp = getRandomSymmetryOperatorIndex(k_NumSymQuats);

  FOrientArrayType quat(4, 0.0f);
  OrientationTransforms<FOrientArrayType, float>::eu2qu(synea, quat);
  q = quat.toQuaternion();
  QuaternionMathF::Multiply(TetraQuatSym[symOp], q, qc);

  quat.fromQuaternion(qc);
  OrientationTransforms<FOrientArrayType, float>::qu2eu(quat, synea);
  return synea;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FOrientArrayType TetragonalOps::determineRodriguesVector(uint64_t seed, int choose)
{
  float init[3];
  float step[3];
  int32_t phi[3];
  float h1, h2, h3;

  init[0] = TetraDim1InitValue;
  init[1] = TetraDim2InitValue;
  init[2] = TetraDim3InitValue;
  step[0] = TetraDim1StepValue;
  step[1] = TetraDim2StepValue;
  step[2] = TetraDim3StepValue;
  phi[0] = static_cast<int32_t>(choose % 36);
  phi[1] = static_cast<int32_t>((choose / 36) % 36);
  phi[2] = static_cast<int32_t>(choose / (36 * 36));

  _calcDetermineHomochoricValues(seed, init, step, phi, choose, h1, h2, h3);
  FOrientArrayType ho(h1, h2, h3);
  FOrientArrayType ro(4);
  OrientationTransforms<FOrientArrayType, float>::ho2ro(ho, ro);
  ro = getMDFFZRod(ro);
  return ro;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int TetragonalOps::getOdfBin(FOrientArrayType rod)
{
  float dim[3];
  float bins[3];
  float step[3];

  FOrientArrayType ho(3);
  OrientationTransforms<FOrientArrayType, float>::ro2ho(rod, ho);

  dim[0] = TetraDim1InitValue;
  dim[1] = TetraDim2InitValue;
  dim[2] = TetraDim3InitValue;
  step[0] = TetraDim1StepValue;
  step[1] = TetraDim2StepValue;
  step[2] = TetraDim3StepValue;
  bins[0] = 36.0f;
  bins[1] = 36.0f;
  bins[2] = 18.0f;

  return _calcODFBin(dim, bins, step, ho);
}

void TetragonalOps::getSchmidFactorAndSS(float load[3], float& schmidfactor, float angleComps[2], int& slipsys)
{
  schmidfactor = 0;
  slipsys = 0;
}

void TetragonalOps::getSchmidFactorAndSS(float load[3], float plane[3], float direction[3], float& schmidfactor, float angleComps[2], int& slipsys)
{
  schmidfactor = 0;
  slipsys = 0;
  angleComps[0] = 0;
  angleComps[1] = 0;

  //compute mags
  float loadMag = sqrt( load[0] * load[0] + load[1] * load[1] + load[2] * load[2] );
  float planeMag = sqrt( plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2] );
  float directionMag = sqrt( direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2] );
  planeMag *= loadMag;
  directionMag *= loadMag;

  //loop over symmetry operators finding highest schmid factor
  for(int i = 0; i < k_NumSymQuats; i++)
  {
    //compute slip system
    float slipPlane[3] = {0};
    slipPlane[2] = TetraMatSym[i][2][0] * plane[0] + TetraMatSym[i][2][1] * plane[1] + TetraMatSym[i][2][2] * plane[2];

    //dont consider negative z planes (to avoid duplicates)
    if( slipPlane[2] >= 0)
    {
      slipPlane[0] = TetraMatSym[i][0][0] * plane[0] + TetraMatSym[i][0][1] * plane[1] + TetraMatSym[i][0][2] * plane[2];
      slipPlane[1] = TetraMatSym[i][1][0] * plane[0] + TetraMatSym[i][1][1] * plane[1] + TetraMatSym[i][1][2] * plane[2];

      float slipDirection[3] = {0};
      slipDirection[0] = TetraMatSym[i][0][0] * direction[0] + TetraMatSym[i][0][1] * direction[1] + TetraMatSym[i][0][2] * direction[2];
      slipDirection[1] = TetraMatSym[i][1][0] * direction[0] + TetraMatSym[i][1][1] * direction[1] + TetraMatSym[i][1][2] * direction[2];
      slipDirection[2] = TetraMatSym[i][2][0] * direction[0] + TetraMatSym[i][2][1] * direction[1] + TetraMatSym[i][2][2] * direction[2];

      float cosPhi = fabs( load[0] * slipPlane[0] + load[1] * slipPlane[1] + load[2] * slipPlane[2] ) / planeMag;
      float cosLambda = fabs( load[0] * slipDirection[0] + load[1] * slipDirection[1] + load[2] * slipDirection[2] ) / directionMag;

      float schmid = cosPhi * cosLambda;
      if(schmid > schmidfactor)
      {
        schmidfactor = schmid;
        slipsys = i;
        angleComps[0] = acos(cosPhi);
        angleComps[1] = acos(cosLambda);
      }
    }
  }
}

void TetragonalOps::getmPrime(QuatF& q1, QuatF& q2, float LD[3], float& mPrime)
{
  mPrime = 0;
}

void TetragonalOps::getF1(QuatF& q1, QuatF& q2, float LD[3], bool maxSF, float& F1)
{
  F1 = 0;
}

void TetragonalOps::getF1spt(QuatF& q1, QuatF& q2, float LD[3], bool maxSF, float& F1spt)
{
  F1spt = 0;
}

void TetragonalOps::getF7(QuatF& q1, QuatF& q2, float LD[3], bool maxSF, float& F7)
{
  F7 = 0;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
namespace Detail
{
  namespace TetragonalHigh
  {
    class GenerateSphereCoordsImpl
    {
        FloatArrayType* m_Eulers;
        FloatArrayType* m_xyz001;
        FloatArrayType* m_xyz011;
        FloatArrayType* m_xyz111;

      public:
        GenerateSphereCoordsImpl(FloatArrayType* eulerAngles, FloatArrayType* xyz001Coords, FloatArrayType* xyz011Coords, FloatArrayType* xyz111Coords) :
          m_Eulers(eulerAngles),
          m_xyz001(xyz001Coords),
          m_xyz011(xyz011Coords),
          m_xyz111(xyz111Coords)
        {}
        virtual ~GenerateSphereCoordsImpl() {}

        void generate(size_t start, size_t end) const
        {
          float g[3][3];
          float gTranpose[3][3];
          float direction[3] = {0.0, 0.0, 0.0};

          // Geneate all the Coordinates
          for(size_t i = start; i < end; ++i)
          {
            FOrientArrayType eu(m_Eulers->getPointer(i * 3), 3);
            FOrientArrayType om(9, 0.0);
            OrientationTransforms<FOrientArrayType, float>::eu2om(eu, om);
            om.toGMatrix(g);
            MatrixMath::Transpose3x3(g, gTranpose);

            // -----------------------------------------------------------------------------
            // 001 Family
            direction[0] = 0.0;
            direction[1] = 0.0;
            direction[2] = 1.0;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz001->getPointer(i * 6));
            MatrixMath::Copy3x1(m_xyz001->getPointer(i * 6), m_xyz001->getPointer(i * 6 + 3));
            MatrixMath::Multiply3x1withConstant(m_xyz001->getPointer(i * 6 + 3), -1);


            // -----------------------------------------------------------------------------
            // 011 Family
            direction[0] = 1.0;
            direction[1] = 0.0;
            direction[2] = 0.0;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz011->getPointer(i * 12));
            MatrixMath::Copy3x1(m_xyz011->getPointer(i * 12), m_xyz011->getPointer(i * 12 + 3));
            MatrixMath::Multiply3x1withConstant(m_xyz011->getPointer(i * 12 + 3), -1);
            direction[0] = 0.0;
            direction[1] = 1.0;
            direction[2] = 0.0;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz011->getPointer(i * 12 + 6));
            MatrixMath::Copy3x1(m_xyz011->getPointer(i * 12 + 6), m_xyz011->getPointer(i * 12 + 9));
            MatrixMath::Multiply3x1withConstant(m_xyz011->getPointer(i * 12 + 9), -1);

            // -----------------------------------------------------------------------------
            // 111 Family
            direction[0] = SIMPLib::Constants::k_1OverRoot2;
            direction[1] = SIMPLib::Constants::k_1OverRoot2;
            direction[2] = 0;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz111->getPointer(i * 12));
            MatrixMath::Copy3x1(m_xyz111->getPointer(i * 12), m_xyz111->getPointer(i * 12 + 3));
            MatrixMath::Multiply3x1withConstant(m_xyz111->getPointer(i * 12 + 3), -1);
            direction[0] = -SIMPLib::Constants::k_1OverRoot2;
            direction[1] = SIMPLib::Constants::k_1OverRoot2;
            direction[2] = 0.0;
            MatrixMath::Multiply3x3with3x1(gTranpose, direction, m_xyz111->getPointer(i * 12 + 6));
            MatrixMath::Copy3x1(m_xyz111->getPointer(i * 12 + 6), m_xyz111->getPointer(i * 12 + 9));
            MatrixMath::Multiply3x1withConstant(m_xyz111->getPointer(i * 12 + 9), -1);
          }

        }

#ifdef SIMPLib_USE_PARALLEL_ALGORITHMS
        void operator()(const tbb::blocked_range<size_t>& r) const
        {
          generate(r.begin(), r.end());
        }
#endif
    };
  }
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void TetragonalOps::generateSphereCoordsFromEulers(FloatArrayType* eulers, FloatArrayType* xyz001, FloatArrayType* xyz011, FloatArrayType* xyz111)
{
  BOOST_ASSERT(false);
  size_t nOrientations = eulers->getNumberOfTuples();

  // Sanity Check the size of the arrays
  if (xyz001->getNumberOfTuples() < nOrientations * Detail::TetragonalHigh::symSize0)
  {
    xyz001->resize(nOrientations * Detail::TetragonalHigh::symSize0 * 3);
  }
  if (xyz011->getNumberOfTuples() < nOrientations * Detail::TetragonalHigh::symSize1)
  {
    xyz011->resize(nOrientations * Detail::TetragonalHigh::symSize1 * 3);
  }
  if (xyz111->getNumberOfTuples() < nOrientations * Detail::TetragonalHigh::symSize2)
  {
    xyz111->resize(nOrientations * Detail::TetragonalHigh::symSize2 * 3);
  }

#ifdef SIMPLib_USE_PARALLEL_ALGORITHMS
  tbb::task_scheduler_init init;
  bool doParallel = true;
#endif

#ifdef SIMPLib_USE_PARALLEL_ALGORITHMS
  if (doParallel == true)
  {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, nOrientations),
                      Detail::TetragonalHigh::GenerateSphereCoordsImpl(eulers, xyz001, xyz011, xyz111), tbb::auto_partitioner());
  }
  else
#endif
  {
    Detail::TetragonalHigh::GenerateSphereCoordsImpl serial(eulers, xyz001, xyz011, xyz111);
    serial.generate(0, nOrientations);
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool TetragonalOps::inUnitTriangle(float eta, float chi)
{
  if( eta < 0 || eta > (45.0 * SIMPLib::Constants::k_PiOver180) || chi < 0 || chi > (90.0 * SIMPLib::Constants::k_PiOver180) )
  {
    return false;
  }
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DREAM3D::Rgb TetragonalOps::generateIPFColor(double* eulers, double* refDir, bool convertDegrees)
{
  return generateIPFColor(eulers[0], eulers[1], eulers[2], refDir[0], refDir[1], refDir[2], convertDegrees);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DREAM3D::Rgb TetragonalOps::generateIPFColor(double phi1, double phi, double phi2, double refDir0, double refDir1, double refDir2, bool degToRad)
{
  if (degToRad == true)
  {
    phi1 = phi1 * SIMPLib::Constants::k_DegToRad;
    phi = phi * SIMPLib::Constants::k_DegToRad;
    phi2 = phi2 * SIMPLib::Constants::k_DegToRad;
  }
  QuatF qc;
  QuatF q2;
  QuatF q1;
  float g[3][3];
  float p[3];
  float refDirection[3];
  float eta, chi;
  float _rgb[3] = { 0.0, 0.0, 0.0 };

  FOrientArrayType eu(phi1, phi, phi2);
  FOrientArrayType qu(4);
  FOrientArrayType om(9); // Reusable for the loop
  OrientationTransforms<FOrientArrayType, float>::eu2qu(eu, qu);
  q1 = qu.toQuaternion();

  for (int j = 0; j < 8; j++)
  {
    getQuatSymOp(j, q2);
    QuaternionMathF::Multiply(q2, q1, qc);

    qu.fromQuaternion(qc);
    OrientationTransforms<FOrientArrayType, float>::qu2om(qu, om);
    om.toGMatrix(g);

    refDirection[0] = refDir0;
    refDirection[1] = refDir1;
    refDirection[2] = refDir2;
    MatrixMath::Multiply3x3with3x1(g, refDirection, p);
    MatrixMath::Normalize3x1(p);

    if(getHasInversion() == false && p[2] < 0)
    {
      continue;
    }
    else if(getHasInversion() == true && p[2] < 0)
    {
      p[0] = -p[0], p[1] = -p[1], p[2] = -p[2];
    }
    chi = acos(p[2]);
    eta = atan2(p[1], p[0]);
    if(inUnitTriangle(eta, chi) == false)
    {
      continue;
    }
    else
    {
      break;
    }
  }

  float etaMin = 0.0;
  float etaMax = 45.0;
  float chiMax = 90.0;
  float etaDeg = eta * SIMPLib::Constants::k_180OverPi;
  float chiDeg = chi * SIMPLib::Constants::k_180OverPi;

  _rgb[0] = 1.0 - chiDeg / chiMax;
  _rgb[2] = fabs(etaDeg - etaMin) / (etaMax - etaMin);
  _rgb[1] = 1 - _rgb[2];
  _rgb[1] *= chiDeg / chiMax;
  _rgb[2] *= chiDeg / chiMax;
  _rgb[0] = sqrt(_rgb[0]);
  _rgb[1] = sqrt(_rgb[1]);
  _rgb[2] = sqrt(_rgb[2]);

  float max = _rgb[0];
  if (_rgb[1] > max)
  {
    max = _rgb[1];
  }
  if (_rgb[2] > max)
  {
    max = _rgb[2];
  }

  _rgb[0] = _rgb[0] / max;
  _rgb[1] = _rgb[1] / max;
  _rgb[2] = _rgb[2] / max;

  return RgbColor::dRgb(_rgb[0] * 255, _rgb[1] * 255, _rgb[2] * 255, 255);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DREAM3D::Rgb TetragonalOps::generateRodriguesColor(float r1, float r2, float r3)
{
  float range1 = 2.0f * TetraDim1InitValue;
  float range2 = 2.0f * TetraDim2InitValue;
  float range3 = 2.0f * TetraDim3InitValue;
  float max1 = range1 / 2.0f;
  float max2 = range2 / 2.0f;
  float max3 = range3 / 2.0f;
  float red = (r1 + max1) / range1;
  float green = (r2 + max2) / range2;
  float blue = (r3 + max3) / range3;

  // Scale values from 0 to 1.0
  red = red / max1;
  green = green / max1;
  blue = blue / max2;

  return RgbColor::dRgb(red * 255, green * 255, blue * 255, 255);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<UInt8ArrayType::Pointer> TetragonalOps::generatePoleFigure(PoleFigureConfiguration_t& config)
{
  QString label0("Tetragonal <001>");
  QString label1("Tetragonal <100>");
  QString label2("Tetragonal <110>");
  if(config.labels.size() > 0) { label0 = config.labels.at(0); }
  if(config.labels.size() > 1) { label1 = config.labels.at(1); }
  if(config.labels.size() > 2) { label2 = config.labels.at(2); }

  int numOrientations = config.eulers->getNumberOfTuples();

  // Create an Array to hold the XYZ Coordinates which are the coords on the sphere.
  // this is size for CUBIC ONLY, <001> Family
  QVector<size_t> dims(1, 3);
  FloatArrayType::Pointer xyz001 = FloatArrayType::CreateArray(numOrientations * Detail::TetragonalHigh::symSize0, dims, label0 + QString("xyzCoords"));
  // this is size for CUBIC ONLY, <011> Family
  FloatArrayType::Pointer xyz011 = FloatArrayType::CreateArray(numOrientations * Detail::TetragonalHigh::symSize1, dims, label1 + QString("xyzCoords"));
  // this is size for CUBIC ONLY, <111> Family
  FloatArrayType::Pointer xyz111 = FloatArrayType::CreateArray(numOrientations * Detail::TetragonalHigh::symSize2, dims, label2 + QString("xyzCoords"));

  config.sphereRadius = 1.0f;

  // Generate the coords on the sphere **** Parallelized
  generateSphereCoordsFromEulers(config.eulers, xyz001.get(), xyz011.get(), xyz111.get());


  // These arrays hold the "intensity" images which eventually get converted to an actual Color RGB image
  // Generate the modified Lambert projection images (Squares, 2 of them, 1 for northern hemisphere, 1 for southern hemisphere
  DoubleArrayType::Pointer intensity001 = DoubleArrayType::CreateArray(config.imageDim * config.imageDim, label0 + "_Intensity_Image");
  DoubleArrayType::Pointer intensity011 = DoubleArrayType::CreateArray(config.imageDim * config.imageDim, label1 + "_Intensity_Image");
  DoubleArrayType::Pointer intensity111 = DoubleArrayType::CreateArray(config.imageDim * config.imageDim, label2 + "_Intensity_Image");
#ifdef SIMPLib_USE_PARALLEL_ALGORITHMS
  tbb::task_scheduler_init init;
  bool doParallel = true;
  tbb::task_group* g = new tbb::task_group;

  if(doParallel == true)
  {
    g->run(GenerateIntensityMapImpl(xyz001.get(), &config, intensity001.get()));
    g->run(GenerateIntensityMapImpl(xyz011.get(), &config, intensity011.get()));
    g->run(GenerateIntensityMapImpl(xyz111.get(), &config, intensity111.get()));
    g->wait(); // Wait for all the threads to complete before moving on.
    delete g;
    g = NULL;
  }
  else
#endif
  {
    GenerateIntensityMapImpl m001(xyz001.get(), &config, intensity001.get());
    m001();
    GenerateIntensityMapImpl m011(xyz011.get(), &config, intensity011.get());
    m011();
    GenerateIntensityMapImpl m111(xyz111.get(), &config, intensity111.get());
    m111();
  }

  // Find the Max and Min values based on ALL 3 arrays so we can color scale them all the same
  double max = std::numeric_limits<double>::min();
  double min = std::numeric_limits<double>::max();

  double* dPtr = intensity001->getPointer(0);
  size_t count = intensity001->getNumberOfTuples();
  for(size_t i = 0; i < count; ++i)
  {
    if (dPtr[i] > max)
    {
      max = dPtr[i];
    }
    if (dPtr[i] < min)
    {
      min = dPtr[i];
    }
  }


  dPtr = intensity011->getPointer(0);
  count = intensity011->getNumberOfTuples();
  for(size_t i = 0; i < count; ++i)
  {
    if (dPtr[i] > max)
    {
      max = dPtr[i];
    }
    if (dPtr[i] < min)
    {
      min = dPtr[i];
    }
  }

  dPtr = intensity111->getPointer(0);
  count = intensity111->getNumberOfTuples();
  for(size_t i = 0; i < count; ++i)
  {
    if (dPtr[i] > max)
    {
      max = dPtr[i];
    }
    if (dPtr[i] < min)
    {
      min = dPtr[i];
    }
  }

  config.minScale = min;
  config.maxScale = max;

  dims[0] = 4;
  UInt8ArrayType::Pointer image001 = UInt8ArrayType::CreateArray(config.imageDim * config.imageDim, dims, label0);
  UInt8ArrayType::Pointer image011 = UInt8ArrayType::CreateArray(config.imageDim * config.imageDim, dims, label1);
  UInt8ArrayType::Pointer image111 = UInt8ArrayType::CreateArray(config.imageDim * config.imageDim, dims, label2);

  QVector<UInt8ArrayType::Pointer> poleFigures(3);
  if(config.order.size() == 3)
  {
    poleFigures[config.order[0]] = image001;
    poleFigures[config.order[1]] = image011;
    poleFigures[config.order[2]] = image111;
  }
  else
  {
    poleFigures[0] = image001;
    poleFigures[1] = image011;
    poleFigures[2] = image111;
  }

#ifdef SIMPLib_USE_PARALLEL_ALGORITHMS

  if(doParallel == true)
  {
    g = new tbb::task_group;
    g->run(GeneratePoleFigureRgbaImageImpl(intensity001.get(), &config, image001.get()));
    g->run(GeneratePoleFigureRgbaImageImpl(intensity011.get(), &config, image011.get()));
    g->run(GeneratePoleFigureRgbaImageImpl(intensity111.get(), &config, image111.get()));
    g->wait(); // Wait for all the threads to complete before moving on.
    delete g;
    g = NULL;
  }
  else
#endif
  {
    GeneratePoleFigureRgbaImageImpl m001(intensity001.get(), &config, image001.get());
    m001();
    GeneratePoleFigureRgbaImageImpl m011(intensity011.get(), &config, image011.get());
    m011();
    GeneratePoleFigureRgbaImageImpl m111(intensity111.get(), &config, image111.get());
    m111();
  }

  return poleFigures;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UInt8ArrayType::Pointer TetragonalOps::generateIPFTriangleLegend(int imageDim)
{

  QVector<size_t> dims(1, 4);
  UInt8ArrayType::Pointer image =  UInt8ArrayType::CreateArray( static_cast<size_t>(imageDim * imageDim), dims, "Orthorhombic Triangle Legend");
  uint32_t* pixelPtr = reinterpret_cast<uint32_t*>(image->getPointer(0));

  static const float xInc = 1.0 / imageDim;
  static const float yInc = 1.0 / imageDim;
  static const float rad = 1.0f;

  float x = 0.0f;
  float y = 0.0f;
  float a = 0.0f;
  float b = 0.0f;
  float c = 0.0f;

  float val = 0.0f;
  float x1 = 0.0f;
  float y1 = 0.0f;
  float z1 = 0.0f;
  float denom = 0.0f;

  DREAM3D::Rgb color;
  size_t idx = 0;
  size_t yScanLineIndex = 0; // We use this to control where the data is drawn. Otherwise the image will come out flipped vertically
  // Loop over every pixel in the image and project up to the sphere to get the angle and then figure out the RGB from
  // there.
  for (int32_t yIndex = 0; yIndex < imageDim; ++yIndex)
  {

    for (int32_t xIndex = 0; xIndex < imageDim; ++xIndex)
    {
      idx = (imageDim * yScanLineIndex) + xIndex;

      x = xIndex * xInc;
      y = yIndex * yInc;

      float sumSquares = (x * x) + (y * y);
      if( x > y || sumSquares > 1.0f) // Outside unit circle
      {
        color = 0xFFFFFFFF;
      }
      else if ( sumSquares > (rad-2*xInc) && sumSquares < (rad+2*xInc)) // Black border on the edges
      {
        color = 0xFF000000;
      }
      else if (xIndex == 0 || yIndex == 0 || xIndex == yIndex) // Black border on the edges
      {
        color = 0xFF000000;
      }
      else
      {
        a = (x * x + y * y + 1);
        b = (2 * x * x + 2 * y * y);
        c = (x * x + y * y - 1);

        val = (-b + sqrtf(b * b - 4.0 * a * c)) / (2.0 * a);
        x1 = (1 + val) * x;
        y1 = (1 + val) * y;
        z1 = val;
        denom = (x1 * x1) + (y1 * y1) + (z1 * z1);
        denom = sqrtf(denom);
        x1 = x1 / denom;
        y1 = y1 / denom;
        z1 = z1 / denom;

        color = generateIPFColor(0.0, 0.0, 0.0, x1, y1, z1, false);
      }

      pixelPtr[idx] = color;
    }
    yScanLineIndex++;
  }
  return image;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DREAM3D::Rgb TetragonalOps::generateMisorientationColor(const QuatF& q, const QuatF& refFrame)
{
  BOOST_ASSERT(false);

  float n1, n2, n3, w;
  float xo, xo1, xo2, xo3, x, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11;
  float yo, yo1, yo2, yo3, y, y1, y2, y3, y4, y5, y6, y7, y8, y9, y10, y11;
  float zo, zo1, zo2, zo3, z, z1, z2, z3, z4, z5, z6, z7, z8, z9, z10, z11;
  float k, h, s, v, c, r, g, b;

  QuatF q1, q2;
  QuaternionMathF::Copy(q, q1);
  QuaternionMathF::Copy(refFrame, q2);

  //get misorientation
  w = getMisoQuat(q1, q2, n1, n2, n3);
  n1 = fabs(n1);
  n2 = fabs(n2);
  n3 = fabs(n3);

  //eq c5.1
  k = tan(w / 2.0f);
  xo = n1;
  yo = n2;
  zo = n3;

  FOrientArrayType rod(xo, yo, zo, k);
  rod = getMDFFZRod(rod);
  xo = rod[0];
  yo = rod[1];
  zo = rod[2];
  k = rod[3];

  //eq c3.2
  k = atan2(yo, xo);
  if(k <= M_PI / 8.0f)
  {
    k = sqrtf(xo * xo + yo * yo);
    if(k == 0)
    {
      k = xo;
    }
    else
    {
      k = xo / k;
    }
  }
  else
  {
    k = (2.0f * sqrtf(xo * xo + yo * yo));
    if(k == 0)
    {
      k = (xo + yo);
    }
    else
    {
      k = (xo + yo) / k;
    }
  }
  xo1 = xo * k;
  yo1 = yo * k;
  zo1 = zo / tan(M_PI / 8);

  //eq c3.3
  k = 2.0f * atan2(yo1, xo1);
  xo2 = sqrtf(xo1 * xo1 + yo1 * yo1) * cos(k);
  yo2 = sqrtf(xo1 * xo1 + yo1 * yo1) * sin(k);
  zo2 = zo1;

  //eq c3.4
  k = sqrtf(xo2 * xo2 + yo2 * yo2) / std::max(xo2, yo2);
  xo3 = xo2 * k;
  yo3 = yo2 * k;
  zo3 = zo2;

  //substitute c5.4 results into c1.1
  x = xo3;
  y = yo3;
  z = zo3;

  //eq c1.2
  k = std::max(x, y);
  k = std::max(k, z);
  k = (k * sqrt(3.0f)) / (x + y + z);
  x1 = x * k;
  y1 = y * k;
  z1 = z * k;

  //eq c1.3
  //3 rotation matricies (in paper) can be multiplied into one (here) for simplicity / speed
  //g1*g2*g3 = {{sqrt(2/3), 0, 1/sqrt(3)},{-1/sqrt(6), 1/sqrt(2), 1/sqrt(3)},{-1/sqrt(6), 1/sqrt(2), 1/sqrt(3)}}
  x2 = x1 * sqrt(2.0f / 3.0f) - (y1 + z1) / sqrt(6.0f);
  y2 = (y1 - z1) / sqrt(2.0f);
  z2 = (x1 + y1 + z1) / sqrt(3.0f);

  //eq c1.4
  k = fmodf(atan2f(y2, x2) + 2.0f * SIMPLib::Constants::k_Pi, 2.0f * SIMPLib::Constants::k_Pi);
  x3 = cos(k) * sqrt((x2 * x2 + y2 * y2) / 2.0f) * sin(SIMPLib::Constants::k_Pi / 6.0f + fmodf(k, 2.0f * SIMPLib::Constants::k_Pi / 3.0f)) / 0.5f;
  y3 = sin(k) * sqrt((x2 * x2 + y2 * y2) / 2.0f) * sin(SIMPLib::Constants::k_Pi / 6.0f + fmodf(k, 2.0f * SIMPLib::Constants::k_Pi / 3.0f)) / 0.5f;
  z3 = z2 - 1.0f;

  //eq c1.5
  k = (sqrt(x3 * x3 + y3 * y3) - z3) / sqrt(x3 * x3 + y3 * y3 + z3 * z3);
  x4 = x3 * k;
  y4 = y3 * k;
  z4 = z3 * k;

  //eq c1.6, 7, and 8 (from matlab code not paper)
  k = fmod(atan2(y4, x4) + 2 * M_PI, 2 * M_PI);

  int type;
  if(k >= 0.0f && k < 2.0f * M_PI / 3.0f)
  {
    type = 1;
    x5 = (x4 + y4 * sqrt(3.0f)) / 2.0f;
    y5 = (-x4 * sqrt(3.0f) + y4) / 2.0f;
  }
  else if(k >= 2.0f * M_PI / 3.0f && k < 4.0f * M_PI / 3.0f)
  {
    type = 2;
    x5 = x4;
    y5 = y4;
  }
  else//k>=4*pi/3 && <2*pi
  {
    type = 3;
    x5 = (x4 - y4 * sqrt(3.0f)) / 2.0f;
    y5 = (x4 * sqrt(3.0f) + y4) / 2.0f;
  }
  z5 = z4;

  k = 1.5f * atan2(y5, x5);
  x6 = sqrt(x5 * x5 + y5 * y5) * cos(k);
  y6 = sqrt(x5 * x5 + y5 * y5) * sin(k);
  z6 = z5;

  k = 2.0f * atan2(x6, -z6);
  x7 = sqrt(x6 * x6 + z6 * z6) * sin(k);
  y7 = y6;
  z7 = -sqrt(x6 * x6 + z6 * z6) * cos(k);

  k = (2.0f / 3.0f) * atan2(y7, x7);
  x8 = sqrt(x7 * x7 + y7 * y7) * cos(k);
  y8 = sqrt(x7 * x7 + y7 * y7) * sin(k);
  z8 = z7;

  if(type == 1)
  {
    x9 = (x8 - y8 * sqrt(3.0f)) / 2.0f;
    y9 = (x8 * sqrt(3.0f) + y8) / 2.0f;
  }
  else if(type == 2)
  {
    x9 = x8;
    y9 = y8;
  }
  else//type==3;
  {
    x9 = (x8 + y8 * sqrt(3.0f)) / 2.0f;
    y9 = (-x8 * sqrt(3.0f) + y8) / 2.0f;
  }
  z9 = z8;

  //c1.9
  x10 = (x9 - y9 * sqrt(3.0f)) / 2.0f;
  y10 = (x9 * sqrt(3.0f) + y9) / 2.0f;
  z10 = z9;

  //cartesian to traditional hsv
  x11 = sqrt(x10 * x10 + y10 * y10 + z10 * z10); //r
  y11 = acos(z10 / x11) / M_PI; //theta
  z11 = fmod(fmod(atan2(y10, x10) + 2.0f * M_PI, 2.0f * M_PI) + 4.0f * M_PI / 3.0f, 2.0f * M_PI) / (2.0f * M_PI); //rho

  if(x11 == 0)
  {
    y11 = 0;
    z11 = 0;
  }

  h = z11;
  if(y11 >= 0.5f)
  {
    s = (1.0f - x11);
    v = 2.0f * x11 * (1.0f - y11) + (1.0f - x11) / 2.0f;
    if(v > 0)
    {
      s = s / (2.0f * v);
    }
    s = 1.0f - s;
  }
  else
  {
    s = (4.0f * x11 * y11) / (1.0f + x11);
    v = 0.5f + x11 / 2;
  }

  //hsv to rgb (from wikipedia hsv/hsl page)
  c = v * s;
  k = c * (1 - fabs(fmod(h * 6, 2) - 1)); //x in wiki article
  h = h * 6;
  r = 0;
  g = 0;
  b = 0;

  if(h >= 0)
  {
    if(h < 1)
    {
      r = c;
      g = k;
    }
    else if(h < 2)
    {
      r = k;
      g = c;
    }
    else if(h < 3)
    {
      g = c;
      b = k;
    }
    else if(h < 4)
    {
      g = k;
      b = c;
    }
    else if (h < 5)
    {
      r = k;
      b = c;
    }
    else if(h < 6)
    {
      r = c;
      b = k;
    }
  }

  //adjust lumosity and invert
  r = 1 - (r + (v - c));
  g = 1 - (g + (v - c));
  b = 1 - (b + (v - c));

  DREAM3D::Rgb rgb = RgbColor::dRgb(r * 255, g * 255, b * 255, 0);

  return rgb;
}

