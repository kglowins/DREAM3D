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

#ifndef _Symmetric6x6WidgetCodeGenerator_H_
#define _Symmetric6x6WidgetCodeGenerator_H_

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/Common/SIMPLibSetGetMacros.h"

#include "DevHelper/CodeGenerators/FPCodeGenerator.h"

class Symmetric6x6WidgetCodeGenerator : public FPCodeGenerator
{
  public:
    SIMPL_SHARED_POINTERS(Symmetric6x6WidgetCodeGenerator)

    static Pointer New(QString humanLabel, QString propertyName, QString category, QString initValue)
    {
      Pointer sharedPtr(new Symmetric6x6WidgetCodeGenerator(humanLabel, propertyName, category, initValue));
      return sharedPtr;
    }

    virtual ~Symmetric6x6WidgetCodeGenerator();

    virtual QString generateSetupFilterParameters();

    virtual QString generateReadFilterParameters();

    virtual QString generateDataCheck();

    virtual QString generateFilterParameters();

    virtual QString generateHIncludes();

  protected:
    Symmetric6x6WidgetCodeGenerator(QString humanLabel, QString propertyName, QString category, QString initValue);

  private:

    Symmetric6x6WidgetCodeGenerator(const Symmetric6x6WidgetCodeGenerator&); // Copy Constructor Not Implemented
    void operator=(const Symmetric6x6WidgetCodeGenerator&); // Operator '=' Not Implemented
};

#endif /* Symmetric6x6WidgetCodeGenerator_H_ */
