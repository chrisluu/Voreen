/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/core/vis/processors/image/depthoffield.h"

#include "tgt/assert.h"
#include "tgt/glmath.h"
#include "tgt/vector.h"

#include "voreen/core/vis/voreenpainter.h"

namespace voreen {

const std::string DepthOfField::setDepthOfFieldThreshold_("depthOfFieldThreshold");

DepthOfField::DepthOfField()
    : ImageProcessorDepth("pp_depthoffield")
    , depthThreshold_(setDepthOfFieldThreshold_, "Depth Threshold")
    , inport_(Port::INPORT, "image.inport")
    , outport_(Port::OUTPORT, "image.outport", true)
{
    addProperty(depthThreshold_);

    addPort(inport_);
    addPort(outport_);
}

const std::string DepthOfField::getProcessorInfo() const {
    return "Performs a depth of field rendering.";
}

Processor* DepthOfField::create() const {
    return new DepthOfField();
}

void DepthOfField::setDepthThreshold(float depthThreshold) {
    depthThreshold_.set(depthThreshold);
}

void DepthOfField::process() {
    outport_.activateTarget();

    analyzeDepthBuffer(&inport_);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind shading result from previous ray cast
    inport_.bindTextures(tm_.getGLTexUnit(shadeTexUnit_), tm_.getGLTexUnit(depthTexUnit_));

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("depthTex_", tm_.getTexUnit(depthTexUnit_));
    program_->setUniform("minDepth_", minDepth_.get());
    program_->setUniform("maxDepth_", maxDepth_.get());
    program_->setUniform("depthThreshold_", depthThreshold_.get());

    renderQuad();

    program_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

} // voreen namespace