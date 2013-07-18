/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__Picker__
#define __TrenchBroom__Picker__

#include "VecMath.h"
#include "Holder.h"
#include "Model/Octree.h"
#include "Model/Pickable.h"

#include <limits>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Hit {
        public:
            typedef unsigned long HitType;
            static const HitType NoType = 0;
            static const HitType AnyType = 0xFFFFFFFF;
            static HitType freeHitType();
            
            typedef std::vector<Hit> List;
            static const Hit NoHit;
        private:
            HitType m_type;
            float m_distance;
            Vec3f m_hitPoint;
            BaseHolder::Ptr m_holder;
        public:
            template <typename T>
            Hit(const HitType type, const float distance, const Vec3f& hitPoint, T object) :
            m_type(type),
            m_distance(distance),
            m_hitPoint(hitPoint),
            m_holder(Holder<T>::newHolder(object)) {}

            HitType type() const;
            bool hasType(const HitType typeMask) const;
            float distance() const;
            const Vec3f& hitPoint() const;
            
            template <typename T>
            inline T object() const {
                return static_cast<Holder<T>*>(m_holder.get())->object();
            }
        };
        
        class HitFilter {
        public:
            virtual bool matches(const Hit& hit) const = 0;
        };
        
        class PickResult {
        public:
            struct FirstHit {
                bool matches;
                Hit hit;
                FirstHit(const bool i_matches, const Hit& i_hit);
            };
        private:
            Hit::List m_hits;
            
            struct CompareHits {
                bool operator() (const Hit& left, const Hit& right) const;
            };
        public:
            FirstHit firstHit(const HitFilter& filter, const bool ignoreOccluders) const;
            Hit::List hits(const HitFilter& filter) const;
            Hit::List allHits() const;
            
            void addHit(const Hit& hit);
            void sortHits();
        };
        
        class Picker {
        private:
            Octree<FloatType, Pickable::Ptr> m_octree;
        public:
            Picker(const BBox<FloatType, 3>& worldBounds);
            
            void addObject(Pickable::Ptr object);
            void addObjects(const Pickable::List& objects);
            void removeObject(Pickable::Ptr object);
            void removeObjects(const Pickable::List& objects);
            
            PickResult pick(const Ray3& ray);
        };
    }
}

#endif /* defined(__TrenchBroom__Picker__) */