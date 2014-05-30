/* ============================================================================
 * Copyright (c) 2012 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2012 Dr. Michael A. Groeber (US Air Force Research Laboratories)
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

#include "RenameFaceArray.h"


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RenameFaceArray::RenameFaceArray() :
  AbstractFilter(),
  m_SelectedFaceArrayName(""),
  m_NewFaceArrayName("")
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RenameFaceArray::~RenameFaceArray()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RenameFaceArray::setupFilterParameters()
{
  std::vector<FilterParameter::Pointer> parameters;
  {
    FilterParameter::Pointer option = FilterParameter::New();
    option->setHumanLabel("Current Face Array Name");
    option->setPropertyName("SelectedFaceArrayName");
    option->setWidgetType(FilterParameter::SurfaceMeshFaceArrayNameSelectionWidget);
    option->setValueType("string");
    option->setUnits("");
    parameters.push_back(option);
  }
  {
    FilterParameter::Pointer parameter = FilterParameter::New();
    parameter->setHumanLabel("New Cell Array Name");
    parameter->setPropertyName("NewFaceArrayName");
    parameter->setWidgetType(FilterParameter::StringWidget);
    parameter->setValueType("string");
    parameters.push_back(parameter);
  }

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RenameFaceArray::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  /* Code to read the values goes between these statements */
/* FILTER_WIDGETCODEGEN_AUTO_GENERATED_CODE BEGIN*/
  setSelectedFaceArrayName( reader->readValue( "SelectedFaceArrayName", getSelectedFaceArrayName() ) );
  setNewFaceArrayName( reader->readValue( "NewFaceArrayName", getNewFaceArrayName() ) );
/* FILTER_WIDGETCODEGEN_AUTO_GENERATED_CODE END*/
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int RenameFaceArray::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  writer->writeValue("SelectedFaceArrayName", getSelectedFaceArrayName() );
  writer->writeValue("NewFaceArrayName", getNewFaceArrayName() );
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RenameFaceArray::dataCheck(bool preflight, size_t voxels, size_t fields, size_t ensembles)
{
  SurfaceMeshDataContainer* sm = getSurfaceMeshDataContainer();
  if(NULL == sm)
  {
    setErrorCondition(-999);
    addErrorMessage(getHumanLabel(), "The Surface Mesh DataContainer Object was NULL", -999);
    return;
  }
  setErrorCondition(0);
  std::stringstream ss;

  if(m_SelectedFaceArrayName.empty() == true)
  {
    setErrorCondition(-11000);
    ss << "An array from the Surface Mesh Data Container must be selected.";
    addErrorMessage(getHumanLabel(),ss.str(),getErrorCondition());
  }
  else
  {
    bool check = sm->renameFaceData(m_SelectedFaceArrayName, m_NewFaceArrayName);
    if(check == false)
    {
      ss << "Array to be renamed could not be found in DataContainer";
      addErrorMessage(getHumanLabel(),ss.str(),getErrorCondition());
    }
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RenameFaceArray::preflight()
{
  /* Place code here that sanity checks input arrays and input values. Look at some
  * of the other DREAM3DLib/Filters/.cpp files for sample codes */
  dataCheck(true, 1, 1, 1);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RenameFaceArray::execute()
{
  SurfaceMeshDataContainer* sm = getSurfaceMeshDataContainer();
  if(NULL == sm)
  {
    setErrorCondition(-999);
    notifyErrorMessage("The DataContainer Object was NULL", -999);
    return;
  }
  setErrorCondition(0);
  std::stringstream ss;

  bool check = sm->renameFaceData(m_SelectedFaceArrayName, m_NewFaceArrayName);

  if(check == false)
  {
  ss << "Array to be renamed could not be found in DataContainer";
  setErrorCondition(-11000);
  notifyErrorMessage(ss.str(), getErrorCondition());
  }

  notifyStatusMessage("Complete");
}
