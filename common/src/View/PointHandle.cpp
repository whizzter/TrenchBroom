/*
 Copyright (C) 2010-2014 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "PointHandle.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"

namespace TrenchBroom {
    namespace View {
        PointHandle::PointHandle(const Vec3& position, const Color& color) :
        m_position(position),
        m_color(color) {}
        
        FloatType PointHandle::pick(const Ray3& pickRay, const Renderer::Camera& camera) const {
            const FloatType radius = static_cast<FloatType>(pref(Preferences::HandleRadius));
            const FloatType scaling = static_cast<FloatType>(camera.perspectiveScalingFactor(m_position));
            return pickRay.intersectWithSphere(m_position, 2.0 * radius * scaling);
        }
        
        void PointHandle::render(const Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch, const bool highlight) const {
        }
    }
}