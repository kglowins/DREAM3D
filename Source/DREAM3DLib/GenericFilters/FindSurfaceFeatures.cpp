/* ============================================================================
 * Copyright (c) 2011 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2011 Dr. Michael A. Groeber (US Air Force Research Laboratories)
 * All rights reserved.
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
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
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
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "FindSurfaceFeatures.h"

#include "DREAM3DLib/Math/DREAM3DMath.h"
#include "DREAM3DLib/Common/Constants.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindSurfaceFeatures::FindSurfaceFeatures() :
  AbstractFilter(),
  m_DataContainerName(DREAM3D::HDF5::VolumeDataContainerName),
  m_FeatureIdsArrayName(DREAM3D::CellData::FeatureIds),
  m_SurfaceFeaturesArrayName(DREAM3D::FeatureData::SurfaceFeatures),
  m_FeatureIds(NULL),
  m_SurfaceFeatures(NULL)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindSurfaceFeatures::~FindSurfaceFeatures()
{
}
// -----------------------------------------------------------------------------
void FindSurfaceFeatures::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  /* Code to read the values goes between these statements */
  /* FILTER_WIDGETCODEGEN_AUTO_GENERATED_CODE BEGIN*/
  /* FILTER_WIDGETCODEGEN_AUTO_GENERATED_CODE END*/
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int FindSurfaceFeatures::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindSurfaceFeatures::dataCheck(bool preflight, size_t voxels, size_t features, size_t ensembles)
{
  VolumeDataContainer* m = getDataContainerArray()->getDataContainerAs<VolumeDataContainer>(getDataContainerName());

  QVector<int> dims(1, 1);
  // Cell Data
  m_FeatureIdsPtr = m->getPrereqArray<int32_t, AbstractFilter>(this, m_CellAttributeMatrixName,  m_FeatureIdsArrayName, -300, voxels, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0); /* Assigns the actual data pointer to our instance variable m_FeatureIds */

  // Feature Data
  m_SurfaceFeaturesPtr = m->createNonPrereqArray<bool, AbstractFilter>(this, m_CellFeatureAttributeMatrixName,  m_SurfaceFeaturesArrayName, false, features, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  m_SurfaceFeatures = m_SurfaceFeaturesPtr.lock()->getPointer(0); /* Assigns the actual data pointer to our instance variable m_SurfaceFeatures */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindSurfaceFeatures::preflight()
{
  VolumeDataContainer* m = getDataContainerArray()->getDataContainerAs<VolumeDataContainer>(getDataContainerName());
  if(NULL == m)
  {
    setErrorCondition(-999);
    addErrorMessage(getHumanLabel(), "The VolumeDataContainer Object with the specific name " + getDataContainerName() + " was not available.", getErrorCondition());
    return;
  }

  dataCheck(true, 1, 1, 1);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindSurfaceFeatures::execute()
{
  QString ss;
  VolumeDataContainer* m = getDataContainerArray()->getDataContainerAs<VolumeDataContainer>(getDataContainerName());
  if (NULL == m)
  {
    setErrorCondition(-999);
    notifyErrorMessage(QObject::tr("VolumeDataContainer was NULL. Returning from Execute Method for filter %1").arg(getHumanLabel()), getErrorCondition());
    return;
  }
  setErrorCondition(0);

  int64_t totalPoints = m->getTotalPoints();
  size_t totalFeatures = m->getNumCellFeatureTuples();
  ss = QObject::tr("FSG Points - %1, Features - %2").arg(totalPoints).arg(totalFeatures);
  notifyStatusMessage(ss);
  dataCheck(false, totalPoints, totalFeatures, 1);
  if (getErrorCondition() < 0)
  {
    return;
  }

  if(m->getXPoints() > 1 && m->getYPoints() > 1 && m->getZPoints() > 1) { find_surfacefeatures(); }
  if(m->getXPoints() == 1 || m->getYPoints() == 1 || m->getZPoints() == 1) { find_surfacefeatures2D(); }


  notifyStatusMessage("FindSurfaceFeatures Completed");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindSurfaceFeatures::find_surfacefeatures()
{
  VolumeDataContainer* m = getDataContainerArray()->getDataContainerAs<VolumeDataContainer>(getDataContainerName());

  size_t xPoints = m->getXPoints();
  size_t yPoints = m->getYPoints();
  size_t zPoints = m->getZPoints();

  int zStride, yStride;
  for(size_t i = 0; i < zPoints; i++)
  {
    zStride = i * xPoints * yPoints;
    for (size_t j = 0; j < yPoints; j++)
    {
      yStride = j * xPoints;
      for(size_t k = 0; k < xPoints; k++)
      {
        int gnum = m_FeatureIds[zStride + yStride + k];
        if(m_SurfaceFeatures[gnum] == false)
        {
          if(k <= 0) { m_SurfaceFeatures[gnum] = true; }
          if(k >= xPoints - 1) { m_SurfaceFeatures[gnum] = true; }
          if(j <= 0) { m_SurfaceFeatures[gnum] = true; }
          if(j >= yPoints - 1) { m_SurfaceFeatures[gnum] = true; }
          if(i <= 0) { m_SurfaceFeatures[gnum] = true; }
          if(i >= zPoints - 1) { m_SurfaceFeatures[gnum] = true; }
          if(m_SurfaceFeatures[gnum] == false)
          {
            if(m_FeatureIds[zStride + yStride + k - 1] == 0) { m_SurfaceFeatures[gnum] = true; }
            if(m_FeatureIds[zStride + yStride + k + 1] == 0) { m_SurfaceFeatures[gnum] = true; }
            if(m_FeatureIds[zStride + yStride + k - xPoints] == 0) { m_SurfaceFeatures[gnum] = true; }
            if(m_FeatureIds[zStride + yStride + k + xPoints] == 0) { m_SurfaceFeatures[gnum] = true; }
            if(m_FeatureIds[zStride + yStride + k - (xPoints * yPoints)] == 0) { m_SurfaceFeatures[gnum] = true; }
            if(m_FeatureIds[zStride + yStride + k + (xPoints * yPoints)] == 0) { m_SurfaceFeatures[gnum] = true; }
          }
        }
      }
    }
  }
}
void FindSurfaceFeatures::find_surfacefeatures2D()
{
  VolumeDataContainer* m = getDataContainerArray()->getDataContainerAs<VolumeDataContainer>(getDataContainerName());

  int xPoints = 0, yPoints = 0;
  float xRes = 0.0f, yRes = 0.0f;

  if(m->getXPoints() == 1)
  {
    xPoints = m->getYPoints();
    xRes = m->getYRes();
    yPoints = m->getZPoints();
    yRes = m->getZRes();
  }
  if(m->getYPoints() == 1)
  {
    xPoints = m->getXPoints();
    xRes = m->getXRes();
    yPoints = m->getZPoints();
    yRes = m->getZRes();
  }
  if(m->getZPoints() == 1)
  {
    xPoints = m->getXPoints();
    xRes = m->getXRes();
    yPoints = m->getYPoints();
    yRes = m->getYRes();
  }

  int yStride;
  for (int j = 0; j < yPoints; j++)
  {
    yStride = j * xPoints;
    for(int k = 0; k < xPoints; k++)
    {
      int gnum = m_FeatureIds[yStride + k];
      if(m_SurfaceFeatures[gnum] == false)
      {
        if(k <= 0) { m_SurfaceFeatures[gnum] = true; }
        if(k >= xPoints - 1) { m_SurfaceFeatures[gnum] = true; }
        if(j <= 0) { m_SurfaceFeatures[gnum] = true; }
        if(j >= yPoints - 1) { m_SurfaceFeatures[gnum] = true; }
        if(m_SurfaceFeatures[gnum] == false)
        {
          if(m_FeatureIds[yStride + k - 1] == 0) { m_SurfaceFeatures[gnum] = true; }
          if(m_FeatureIds[yStride + k + 1] == 0) { m_SurfaceFeatures[gnum] = true; }
          if(m_FeatureIds[yStride + k - m->getXPoints()] == 0) { m_SurfaceFeatures[gnum] = true; }
          if(m_FeatureIds[yStride + k + m->getXPoints()] == 0) { m_SurfaceFeatures[gnum] = true; }
        }
      }
    }
  }
}